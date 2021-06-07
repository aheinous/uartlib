
#include "ezp_master.h"
#include "ezp_msg_sender.h"
#include "ezp_msg_reader.h"

#include "ezp_platform.h"
#include "ezp_util.h"
#include "ezp_msg.h"

#define RETRY_INTERVAL 10


#define NONE (-10) 	// Use -10 instead of -1 to stop NONE from getting
					// incremented into the valid seqNum range of 0->255
					// TODO ???
#if 1





// static EZP_RESULT master_actualRecv(ezp_master_t *self, ezp_msg_t* msg);


EZP_RESULT master_init(	ezp_master_t *self,
						uint8_t *recv_buffer_data, uint8_t recv_buff_len,
						ezp_msg_t *send_buffer_data, uint8_t send_buffer_len,
                        ezp_platform_t platform)
{
	self->lastSeqNumInSendBuffer = 0;
	self->lastSeqNumAckRecvd = NONE;
	self->lastSeqNumFullyRecvd = NONE;
	self->timeTillRetry = 0;
    self->m_platform = platform;

	msgRingbuff_init(&self->msgSendQueue, send_buffer_data, send_buffer_len);
	msgReader_init(&self->msg_reader, recv_buffer_data, recv_buff_len);

	self->initted = EZP_TRUE;
	return EZP_OK;
}




static EZP_RESULT master_actualSend(ezp_master_t *self, ezp_msg_t *msg){
	EZP_ASSERT(self->initted);

	EZP_LOG("Sending:");
	ezp_printMsg(msg);

	msgSender_t sender;

    msgSender_init(&sender, self->m_platform);
	return msgSender_send(&sender, msg);



}


static EZP_RESULT master_sendAckOf(ezp_master_t *self, ezp_msg_t *msg){
	EZP_ASSERT(msg != NULL);
	EZP_ASSERT(msg->typeID != ezp_msgID_ack);

	ezp_msg_t ack;

	ack.typeID = ezp_msgID_ack;
	ack._seqNum = msg->_seqNum;


	EZP_RESULT res = master_actualSend(self, & ack);
	if(res==EZP_OK){
		return EZP_OK;
	}
	EZP_WLOG("master_sendAckOf(): master_actualSend() gave %s, retrying ...\n",
				ezp_res_str(res));

	return res;
}


static void master_considerSend(ezp_master_t *self){
	if(  msgRingbuff_isEmpty(& self->msgSendQueue) ){
		return;
	}

	if(self->timeTillRetry > 0) {
		EZP_LOG_INT(self->timeTillRetry);
		--(self->timeTillRetry);
		return;
	}

	// EZP_VLOG(">> %s -- sending msg\n", __func__);
	ezp_msg_t *msgToSend;
	EZP_CHECK_OK(msgRingbuff_peek(& self->msgSendQueue, &msgToSend));



	master_actualSend(self, msgToSend);
	self->timeTillRetry = RETRY_INTERVAL;


	// TODO wait for ack
	//msgRingbuff_pop(&msgSendQueue, NULL);
}


static void master_handleAck(ezp_master_t *self, ezp_msg_t *ack){
	EZP_LOG("got ack\n");

	if(ack->_seqNum == self->lastSeqNumAckRecvd){
		EZP_LOG("Recv'd duplicate ACK: %d\n", (int) ack->_seqNum);
		return;
	}
	if( self->lastSeqNumAckRecvd != NONE
		&& ack->_seqNum != self->lastSeqNumAckRecvd+1 )
	{
		EZP_ELOG("Recv'd unexpected ACK %d.\n",
					(int) ack->_seqNum);
		return;
	}

	if(msgRingbuff_isEmpty(& self->msgSendQueue)){
		// Presumably we've recv'd this ACK before.
		// Either that or we've reset and are recving
		// acks for msgs sent before reset.
		EZP_WLOG("Recv'd Ack with empty msgSendQueue !\n");
		return;
	}

	ezp_msg_t *possiblyAckedMsg;
	EZP_CHECK_OK(msgRingbuff_peek(& self->msgSendQueue, &possiblyAckedMsg));

	if(possiblyAckedMsg->_seqNum != ack->_seqNum){
		EZP_ELOG("Ack %d does not match msg in send queue %d\n",
			ack->_seqNum,
			possiblyAckedMsg->_seqNum);
		return;
	}

	// seqNum matchs msg in queue
	EZP_CHECK_OK(msgRingbuff_pop( & self->msgSendQueue, NULL));
	// possiblyAckedMsg = NULL; // invalid after pop
	// EZP_ASSERT(ack->_seqNum == lastSeqNumAckRecvd+1);
	self->lastSeqNumAckRecvd = ack->_seqNum;
	self->timeTillRetry = 0;
}


// static uint8_t lastSeqNumFullyRecvd = 0; // TODO
// static int fullyRecvdAtLeastOneMsg = 0;

static void master_handleNonAck(ezp_master_t *self, ezp_msg_t *msg){

	if(self->lastSeqNumFullyRecvd == msg->_seqNum )
	{
		// drop duplicate
		EZP_LOG("Dropping duplicate message, but resending ACK. SeqNum: %d\n",
				 msg->_seqNum);
		master_sendAckOf(self, msg);
		return;
	}

	// EZP_RESULT recv_res = ezp_platform_on_recv_msg(msg);
    EZP_RESULT recv_res = self->m_platform.on_recv_msg(msg);
	if(recv_res == EZP_OK){
		EZP_VLOG("Recv'd msg, successfully pushed to recvQueue.\n");
		master_sendAckOf(self, msg);

		self->lastSeqNumFullyRecvd = msg->_seqNum;
	}else{
		EZP_WLOG("Recv'd msg, but ezp_platform_on_recv_msg gave res %s, dropping.\n",
						ezp_res_str(recv_res));
	}


	// EZP_RESULT pushRes = msgRingbuff_push( & self->msgRecvQueue, msg);
	// if(pushRes == EZP_OK){
	// 	EZP_VLOG("Recv'd msg, successfully pushed to recvQueue.\n");
	// 	master_sendAckOf(self, msg);

	// 	self->lastSeqNumFullyRecvd = msg->_seqNum;
	// }else{
	// 	EZP_WLOG("Recv'd msg, but no room in recvQueue (got res %s), dropping.\n",
	// 				ezp_res_str(pushRes));
	// }
}

static void master_handleAnyRecvdMsgs(ezp_master_t *self){
	ezp_msg_t recvdMsg;
	EZP_RESULT recvRes = msgReader_read_msg(&self->msg_reader, &recvdMsg);
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


EZP_RESULT master_process(ezp_master_t *self){
	EZP_VLOG(">> %s\n", __func__);
	EZP_ASSERT(self->initted);

	master_considerSend(self);
	master_handleAnyRecvdMsgs(self);

	return EZP_OK;
}



// static EZP_RESULT checkChecksum(uint8_t *buff, int size) {
// 	csum_calc_t calc;
// 	csumCalc_init(&calc);
// 	for(int i = 0; i < size - EZP_SIZEOF_CSUM; i++) {
// 		csumCalc_update(&calc, buff[i]);
// 	}

// 	// csumCalc_getCsum(&calc) == (buff[size - 2] | (buff[size - 1] << 8))
// 	uint8_t low, high;
// 	csumCalc_getCsum(&calc, &low, &high);

// 	if(buff[size-2] == low && buff[size-1] == high) {
// 		return EZP_OK;
// 	}
// 	else {
// 		return EZP_EINVALID;
// 	}
// }


EZP_RESULT master_onRecvByte(ezp_master_t *self, uint8_t b){
	return msgReader_push_byte(&self->msg_reader, b);
}





// EZP_RESULT master_recv(ezp_master_t *self, ezp_msg_t *msg) {
// 	EZP_ASSERT(self != NULL);
// 	EZP_ASSERT(msg != NULL);

// 	return msgRingbuff_pop(&self->msgRecvQueue, msg);
// }



EZP_RESULT master_enqueue(ezp_master_t *self, ezp_msg_t *msg) {
	EZP_ASSERT(self != NULL);
	EZP_ASSERT(msg != NULL);

	msg->_seqNum = self->lastSeqNumInSendBuffer;
	EZP_RESULT res = msgRingbuff_push(&(self->msgSendQueue), msg);
	if(res == EZP_OK) {
		++(self->lastSeqNumInSendBuffer);
	}
	return res;
}


#endif