
#include "ezp_types.h"
#include "ezp_msg_buffer.h"


#define RETRY_INTERVAL 10

// TODO put into struct

#define NONE (-10) 	// Use -10 instead of -1 to stop NONE from getting 
					// incremented into the valid seqNum range of 0->255
#if 0

typedef struct {
	ezp_bool_t initted ;
	int16_t lastSeqNumInSendBuffer;
	int16_t lastSeqNumAckRecvd;

	int16_t lastSeqNumFullyRecvd; 

	uint16_t timeTillRetry;

	msgRingbuff_t msgSendQueue;
	msgRingbuff_t msgRecvQueue;
} ezp_msgr_t;

// ezp_msgr_t g_msgr;


static EZP_RESULT Msgr_actualRecv(ezp_msgr_t *msgr, ezp_msg_t* msg);


EZP_RESULT Msgr_actualSend(ezp_msgr_t *msgr, ezp_msg_t *msg){
	EZP_ASSERT(msgr->initted);

	EZP_LOG("Sending:"); ezp_printMsg(msg);

	msgSender_t sender;

	initMsgSender(&sender);


	if( serialize(msg, &sender) == EZP_EAGAIN){
		return EZP_EAGAIN;
	}

	// TODO rename finish send to have "object" name
	if(finish_send(&sender) == EZP_OK){
		return EZP_OK;
	}
	return EZP_EAGAIN;
}


static EZP_RESULT Msgr_sendAckOf(ezp_msgr_t *msgr, ezp_msg_t *msg){
	EZP_ASSERT(msg != NULL);
	EZP_ASSERT(msg->typeID != ezp_msgID_ack);

	ezp_msg_t ack;

	ack.typeID = ezp_msgID_ack;
	ack._seqNum = msg->_seqNum;

	// TODO no inf loop
	while(1){
		EZP_RESULT res = Msgr_actualSend(msgr, & ack);
		if(res==EZP_OK){
			return EZP_OK;
		}
		EZP_WLOG("Msgr_sendAckOf(): Msgr_actualSend() gave %s, retrying ...\n", 
					ezp_res_str(res));
	}
}


static void Msgr_considerSend(ezp_msgr_t *msgr){
	if(  msgRingbuff_isEmpty(& msgr->msgSendQueue) ){
		return;
	}

	// EZP_VLOG(">> %s -- sending msg\n", __func__);
	ezp_msg_t *msgToSend;
	EZP_CHECK_OK(msgRingbuff_peek(& msgr->msgSendQueue, &msgToSend));

	if(msgr->timeTillRetry > 0){
		EZP_LOG_INT(msgr->timeTillRetry);
		--(msgr->timeTillRetry);
		return;
	}
		
	Msgr_actualSend(msgr, msgToSend);
	msgr->timeTillRetry = RETRY_INTERVAL;


	// TODO wait for ack
	//msgRingbuff_pop(&msgSendQueue, NULL);
}


static void Msgr_handleAck(ezp_msgr_t *msgr, ezp_msg_t *ack){
	EZP_LOG("got ack\n");

	if(ack->_seqNum == msgr->lastSeqNumAckRecvd){
		EZP_LOG("Recv'd duplicate ACK: %d\n", (int) ack->_seqNum);
		return;
	}
	if( msgr->lastSeqNumAckRecvd != NONE
		&& ack->_seqNum != msgr->lastSeqNumAckRecvd+1 )
	{
		EZP_ELOG("Recv'd unexpected ACK %d.\n",
					(int) ack->_seqNum);
		return;
	}

	if(msgRingbuff_isEmpty(& msgr->msgSendQueue)){
		// Presumably we've recv'd this ACK before.
		// Either that or we've reset and are recving
		// acks for msgs sent before reset.
		EZP_WLOG("Recv'd Ack with empty msgSendQueue !\n");
		return;
	}

	ezp_msg_t *possiblyAckedMsg;
	EZP_CHECK_OK(msgRingbuff_peek(& msgr->msgSendQueue, &possiblyAckedMsg));

	if(possiblyAckedMsg->_seqNum != ack->_seqNum){
		EZP_ELOG("Ack %d does not match msg in send queue %d\n",
			ack->_seqNum,
			possiblyAckedMsg->_seqNum);
		return;
	}

	// seqNum matchs msg in queue
	EZP_CHECK_OK(msgRingbuff_pop( & msgr->msgSendQueue, NULL));
	// possiblyAckedMsg = NULL; // invalid after pop
	// EZP_ASSERT(ack->_seqNum == lastSeqNumAckRecvd+1);
	msgr->lastSeqNumAckRecvd = ack->_seqNum;
	msgr->timeTillRetry = 0;
}


// static uint8_t lastSeqNumFullyRecvd = 0; // TODO
// static int fullyRecvdAtLeastOneMsg = 0;

static void Msgr_handleNonAck(ezp_msgr_t *msgr, ezp_msg_t *msg){

	if(msgr->lastSeqNumFullyRecvd == msg->_seqNum )
	{
		// drop duplicate
		EZP_LOG("Dropping duplicate message, but resending ACK. SeqNum: %d\n", 
				 msg->_seqNum);
		Msgr_sendAckOf(msgr, msg);
		return;
	}


	EZP_RESULT pushRes = msgRingbuff_push( & msgr->msgRecvQueue, msg);
	if(pushRes == EZP_OK){
		EZP_VLOG("Recv'd msg, successfully pushed to recvQueue.\n");
		Msgr_sendAckOf(msgr, msg);

		msgr->lastSeqNumFullyRecvd = msg->_seqNum;
	}else{
		EZP_WLOG("Recv'd msg, but no room in recvQueue (got res %s), dropping.\n",
					ezp_res_str(pushRes));
	}
}

static void Msgr_handleAnyRecvdMsgs(ezp_msgr_t *msgr){
	ezp_msg_t recvdMsg;
	EZP_RESULT recvRes = Msgr_actualRecv( msgr, &recvdMsg );
	if(recvRes == EZP_OK){

		if(recvdMsg.typeID == ezp_msgID_ack){
			Msgr_handleAck(msgr, &recvdMsg);
		}else{
			Msgr_handleNonAck(msgr, &recvdMsg);		
		}

	}else{
		EZP_VLOG("Msgr_actualRecv() gave %s\n", ezp_res_str(recvRes));
	}
}


EZP_RESULT Msgr_Process(ezp_msgr_t *msgr){
	EZP_VLOG(">> %s\n", __func__);
	EZP_ASSERT(msgr->initted);

	Msgr_considerSend(msgr);
	Msgr_handleAnyRecvdMsgs(msgr);

	return EZP_OK;
}

#endif