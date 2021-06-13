
#include "ezp_master.h"
#include "ezp_msg_sender.h"
#include "ezp_msg_reader.h"
#include "ezp_platform.h"
#include "ezp_util.h"
#include "ezp_msg.h"

#define NONE (-1)

#define WRAPPED(seq_num) ((int16_t)((uint16_t)(seq_num) & 255))

#define UNEXPECTED_SEQ_NUM_THRESHOLD 32

EZP_RESULT master_init(	ezp_master_t *self,
						uint8_t *recv_buffer_data, uint8_t recv_buff_len,
						ezp_msg_t *send_buffer_data, uint8_t send_buffer_len,
						ezp_platform_t platform, int32_t retry_intval)
{
	if(	!self
		|| !recv_buffer_data
		|| !send_buffer_data
		|| !IS_POW2(recv_buff_len)
		|| !IS_POW2(send_buffer_len)
		|| !platform.write_byte
		|| !platform.flush
		|| !platform.on_recv_msg )
	{
		EZP_ASSERT(0);
		return EZP_EARG;
	}

	self->m_next_send_seq_num = 0;
	self->m_last_seq_num_ack_recvd = NONE;
	self->m_last_seq_num_recvd_and_handled = NONE;
	self->m_time_till_retry = 0;
    self->m_platform = platform;
	self->m_retry_intval = retry_intval;
	self->m_consecutive_unexpected_seq_nums = 0;


	msgRingbuff_init(&self->m_send_queue, send_buffer_data, send_buffer_len);
	msgReader_init(&self->m_msg_reader, recv_buffer_data, recv_buff_len);

	self->m_initted = EZP_TRUE;
	return EZP_OK;
}



static inline EZP_RESULT master_actualSend(ezp_master_t *self, ezp_msg_t *msg){
	EZP_ASSERT(self->m_initted);

	EZP_LOG("Sending:");
	ezp_printMsg(msg);

	msgSender_t sender;

    msgSender_init(&sender, self->m_platform);
	return msgSender_send(&sender, msg);
}


static inline EZP_RESULT master_sendAckOf(ezp_master_t *self, ezp_msg_t *msg){
	EZP_ASSERT(msg != NULL);
	EZP_ASSERT(msg->typeID != ezp_msgID_ack);

	ezp_msg_t ack;

	ack.typeID = ezp_msgID_ack;
	ack._seqNum = msg->_seqNum;

	return master_actualSend(self, & ack);
}


static inline void master_considerSend(ezp_master_t *self, int32_t time_elapsed) {
	if(  msgRingbuff_isEmpty(& self->m_send_queue) ){
		return;
	}

	if(self->m_time_till_retry > time_elapsed) {
		EZP_LOG_INT(self->m_time_till_retry);
		self->m_time_till_retry -= time_elapsed;
		return;
	}

	self->m_time_till_retry = 0;

	ezp_msg_t *msg_to_send;
	EZP_CHECK_OK(msgRingbuff_peek(& self->m_send_queue, &msg_to_send));

	master_actualSend(self, msg_to_send);
	self->m_time_till_retry = self->m_retry_intval;
}


static inline void master_handleAck(ezp_master_t *self, ezp_msg_t *ack){
	EZP_LOG("got ack\n");

	if(ack->_seqNum == self->m_last_seq_num_ack_recvd){
		EZP_LOG("Recv'd duplicate ACK: %d\n", (int) ack->_seqNum);
		msgReader_on_msg_valid(&self->m_msg_reader, ack);
		return;
	}
	if( self->m_last_seq_num_ack_recvd != NONE
		&& ack->_seqNum != WRAPPED(self->m_last_seq_num_ack_recvd+1) )
	{
		EZP_WLOG("Recv'd unexpected ACK %d.\n",
					(int) ack->_seqNum);
		msgReader_on_msg_invalid(&self->m_msg_reader);
		return;
	}

	if(msgRingbuff_isEmpty(& self->m_send_queue)){
		// Presumably we've recv'd this ACK before.
		// Either that or we've reset and are recving
		// acks for msgs sent before reset.
		EZP_WLOG("Recv'd Ack with empty m_send_queue !\n");
		msgReader_on_msg_invalid(&self->m_msg_reader);
		return;
	}

	ezp_msg_t *possiblyAckedMsg;
	EZP_CHECK_OK(msgRingbuff_peek(& self->m_send_queue, &possiblyAckedMsg));

	if(possiblyAckedMsg->_seqNum != ack->_seqNum){
		EZP_ELOG("Ack %d does not match msg in send queue %d\n",
			ack->_seqNum,
			possiblyAckedMsg->_seqNum);
		msgReader_on_msg_invalid(&self->m_msg_reader);
		return;
	}

	// seqNum matchs msg in queue
	EZP_CHECK_OK(msgRingbuff_pop( & self->m_send_queue, NULL));
	msgReader_on_msg_valid(&self->m_msg_reader, ack);
	self->m_last_seq_num_ack_recvd = ack->_seqNum;
	self->m_time_till_retry = 0;
}



static inline void master_handleNonAck(ezp_master_t *self, ezp_msg_t *msg){
	if(self->m_last_seq_num_recvd_and_handled == msg->_seqNum )
	{
		// drop duplicate
		EZP_LOG("Dropping duplicate message, but resending ACK. SeqNum: %d\n",
				 msg->_seqNum);
		master_sendAckOf(self, msg);
		msgReader_on_msg_valid(&self->m_msg_reader, msg);

		return;
	}

	if (self->m_last_seq_num_recvd_and_handled != NONE
			&& msg->_seqNum != WRAPPED(1 + self->m_last_seq_num_recvd_and_handled))
	{
		++(self->m_consecutive_unexpected_seq_nums);
		EZP_WLOG("recvd msg with unexpected seq num. consec: %d\n", (int) self->m_consecutive_unexpected_seq_nums);
		if(self->m_consecutive_unexpected_seq_nums < UNEXPECTED_SEQ_NUM_THRESHOLD){
			msgReader_on_msg_invalid(&self->m_msg_reader);
			return;
		}
		// assume valid, and we were off somehow
	}
	self->m_consecutive_unexpected_seq_nums = 0;
	msgReader_on_msg_valid(&self->m_msg_reader, msg);

	EZP_RESULT recv_res = self->m_platform.on_recv_msg(self->m_platform.usr_data, msg);

	if(recv_res == EZP_OK){
		EZP_VLOG("Recv'd msg successfully.\n");
		master_sendAckOf(self, msg);

		self->m_last_seq_num_recvd_and_handled = msg->_seqNum;
	}else{
		EZP_LOG("Recv'd msg, but m_platform.on_recv_msg gave res %s, dropping.\n",
						ezp_res_str(recv_res));
	}
}

static inline void master_handleAnyRecvdMsgs(ezp_master_t *self) {
	ezp_msg_t recvdMsg;
	EZP_RESULT recvRes = msgReader_read_msg(&self->m_msg_reader, &recvdMsg);
	if(recvRes == EZP_OK){

		if(recvdMsg.typeID == ezp_msgID_ack){
			master_handleAck(self, &recvdMsg);
		}else{
			master_handleNonAck(self, &recvdMsg);
		}

	}else{
		EZP_VLOG("master_actualRecv() gave %s\n", ezp_res_str(recvRes));
	}
}


EZP_RESULT master_process(ezp_master_t *self, int32_t time_elapsed){
	EZP_VLOG(">> %s\n", __func__);
	EZP_ASSERT(self != NULL);
	EZP_ASSERT(self->m_initted);

	master_considerSend(self, time_elapsed);
	master_handleAnyRecvdMsgs(self);

	return EZP_OK;
}



EZP_RESULT master_on_recv_byte(ezp_master_t *self, uint8_t b){
	EZP_ASSERT(self != NULL);
	EZP_ASSERT(self->m_initted);
	return msgReader_push_byte(&self->m_msg_reader, b);
}


EZP_RESULT master_enqueue(ezp_master_t *self, ezp_msg_t *msg) {
	EZP_ASSERT(self != NULL);
	EZP_ASSERT(msg != NULL);
	EZP_ASSERT(self->m_initted);

	msg->_seqNum = self->m_next_send_seq_num;
	EZP_RESULT res = msgRingbuff_push(&(self->m_send_queue), msg);
	if(res == EZP_OK) {
		self->m_next_send_seq_num = WRAPPED(1 + self->m_next_send_seq_num);
	}
	return res;
}
