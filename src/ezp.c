#if 0

#include "ezp.h"
#include "ezp_msg_buffer.h"
// #include "ezp_byte_buffer.h"
#include "ezp_csum.h"

#define SIZEOF_CSUM 2
#define SIZEOF_TYPEID 1
#define SIZEOF_SEQNUM 1
#if 1
// ISSUES:
// 	- Non-blocking UART read vs. separate UART interrupt
//	- Retry timer.
//


// TODO static all static-able functions
#include <stdio.h>


void ezp_printMsg(ezp_msg_t *msg);

void print_uint8_t(const char*label, uint8_t val){
	printf("%s %d\n", label, (int)val);
}


void printBuff(const uint8_t *buff, int len){
	EZP_VLOG("buff: [");
	for(int i=0; i<len; i++){
		EZP_VLOG("%d ", (int) buff[i]);
	}
	EZP_VLOG("]\n");
}


// EZP_RESULT ezp_recv(ezp_msg_t* msg);

// EZP_RESULT ezp_send(ezp_msg_t *msg);

// TODO ifdef ENABLE_TESTS

#define EZP_TEST_ASSERT(cond) do{ \
	int res = (cond); \
	printf("Test: " #cond " ... %s\n", (res) ? "SUCCEEDED" : "FAILED" );\
	if(!res) while(1) {} ;\
}while(0)



#define EZP_CHECK_OK(expn) do{ \
	EZP_RESULT res = (expn); \
	EZP_ASSERT(res == EZP_OK); \
} while(0)





// ----- global EZP instance -------------------------------

#define RETRY_INTERVAL 10

// TODO put into struct

#define NONE (-10) 	// Use -10 instead of -1 to stop NONE from getting
					// incremented into the valid seqNum range of 0->255

// static int initted = 0;
// static int16_t lastSeqNumInSendBuffer = 0;
// static int16_t lastSeqNumAckRecvd = NONE;

// static int16_t lastSeqNumFullyRecvd = NONE;
// // static int fullyRecvdAtLeastOneMsg = 0;

// static uint16_t timeTillRetry = 0;

// static msgRingbuff_t msgSendQueue;
// static msgRingbuff_t msgRecvQueue;



	// ezp_bool_t initted = EZP_FALSE;
	// int16_t lastSeqNumInSendBuffer = 0;
	// int16_t lastSeqNumAckRecvd = NONE;

	// int16_t lastSeqNumFullyRecvd = NONE;

	// uint16_t timeTillRetry = 0;

	// msgRingbuff_t msgSendQueue;
	// msgRingbuff_t msgRecvQueue;



// checksum -------------------------------------------------------------------------





// msgsender ----------------------------------------------------------------------------

// typedef struct{
// 	csum_calc_t csum;
// } msgSender_t;


// void initMsgSender(msgSender_t *sender){
// 	csumCalculator_init(&sender->csum);
// }

// EZP_RESULT send_uint8_t(msgSender_t *sender, uint8_t data){
// 	if(ezp_platform_putc(data) == EZP_OK){
// 		csumCalculator_update(&sender->csum, data);
// 		return EZP_OK;
// 	}
// 	return EZP_EAGAIN;
// }

// EZP_RESULT finish_send(msgSender_t *sender){
// 	uint16_t csum = csumCalculator_getCsum(&sender->csum);
// 	uint8_t low = csum & 0xff;
// 	uint8_t high = (csum>>8) & 0xff;
// 	if(ezp_platform_putc( low ) != EZP_OK){
// 		return EZP_EAGAIN;
// 	}
// 	if(ezp_platform_putc( high ) != EZP_OK){
// 		return EZP_EAGAIN;
// 	}
// 	return EZP_OK;
// }


// msgreader ----------------------------------------------------------------------------
typedef struct{
	uint8_t *buff;
	int index;
	int size;

} msgReader_t;

void initMsgReader(msgReader_t *reader, uint8_t *buff, int size){
	EZP_VLOG("initMsgReader %p:", reader);
	printBuff(buff, size);
	reader->buff = buff;
	reader->index = 0;
	reader->size = size;
}


EZP_RESULT read_uint8_t(msgReader_t *reader, uint8_t*data){
	EZP_ASSERT(reader != NULL);
	EZP_ASSERT(data != NULL);

	EZP_ASSERT(reader->index < reader->size);

	uint8_t val = reader->buff[ (reader->index)++ ];
	*data = val;

	return EZP_OK;
}



// -----------------------------------------------------------------------------------------
// ---------------------  X Macro Msg Table Function Definitions ---------------------------
// -----------------------------------------------------------------------------------------




EZP_RESULT deserialize(ezp_msg_t *msg, msgReader_t *reader);




// size of function ---------------------------------------------


#define START_MSG(msgName) 			case (ezp_msgID_ ## msgName): *size = 0
#define FIELD(type, fieldName) 		+ sizeof(type)
#define END_MSG(msgName) 			; break;

EZP_RESULT sizeOfMsgPayload(EZP_MSG_TYPE_ID typeID, int *size){
	EZP_ASSERT(size != NULL);

	switch(typeID){
		#include EZP_MSG_TABLE

		case ezp_msgID_ack:
			*size = 0;
			break;

		default:
			//EZP_ELOG("sizeOfMsgPayload(): unexpected typeID: %d\n", (int) typeID);
			return EZP_EINVALID;
	}
	return EZP_OK;
}

#undef START_MSG
#undef FIELD
#undef END_MSG


EZP_RESULT sizeOfWholeMsg(EZP_MSG_TYPE_ID typeID, int *size){
	EZP_ASSERT(size != NULL);

	EZP_RESULT res = sizeOfMsgPayload(typeID, size) ;
	*size += SIZEOF_SEQNUM + SIZEOF_TYPEID + SIZEOF_CSUM;

	return res;
}

// Send/Serialize Funtion ---------------------------------------------

#define START_MSG(msgName) 		case (ezp_msgID_ ## msgName):{ \
									ezp_ ## msgName ## _t *innerMsg = &(msg->msgName);
#define FIELD(type, fieldName) 		if(send_ ## type( sender, innerMsg->fieldName ) != EZP_OK) { return EZP_EAGAIN; }
#define END_MSG(msgName) 		} break;


EZP_RESULT serialize(ezp_msg_t *msg, msgSender_t* sender){


	// msgSender_t sender;

	// initMsgSender(&sender);

	if(send_uint8_t(sender, msg->typeID) != EZP_OK){
		return EZP_EAGAIN;
	}
	if(send_uint8_t(sender, msg->_seqNum) != EZP_OK){
		return EZP_EAGAIN;
	}


	switch(msg->typeID){

		#include EZP_MSG_TABLE

		case ezp_msgID_ack:
			// no payload
			break;

		default:
			EZP_ELOG("Unexpected msg type ID: %d\n", msg->typeID);
			//return EZP_EARG;
			EZP_ASSERT(0);
	}


	return EZP_OK;
	// // TODO rename finish send to have "object" name
	// if(finish_send(&sender) == EZP_OK){
	// 	return EZP_OK;
	// }
	// return EZP_EAGAIN;
}


#undef START_MSG
#undef FIELD
#undef END_MSG





// Deserialize Funtion ---------------------------------------------

#define START_MSG(msgName) 		case (ezp_msgID_ ## msgName):{ \
									ezp_ ## msgName ## _t *innerMsg = &(msg->msgName);
#define FIELD(type, fieldName) 		if(read_ ## type( reader, &(innerMsg->fieldName) ) != EZP_OK) { return EZP_EAGAIN; }
#define END_MSG(msgName) 		} break;

EZP_RESULT deserialize(ezp_msg_t *msg, msgReader_t *reader){
	// EZP_VLOG("reading typeID\n");

	if(read_uint8_t(reader, &(msg->typeID)) != EZP_OK){
		return EZP_EAGAIN;
	}
	if(read_uint8_t(reader, &(msg->_seqNum)) != EZP_OK){
		return EZP_EAGAIN;
	}

	switch(msg->typeID){

		#include EZP_MSG_TABLE

		case ezp_msgID_ack:
			// no payload
			break;

		default:
			EZP_ELOG("Unexpected deser msg type ID: %d\n", msg->typeID);
			return EZP_EINVALID;
	}

	return EZP_OK;
}

#undef START_MSG
#undef FIELD
#undef END_MSG

// Print Funtion ------------------------------------------------

#define START_MSG(msgName) 		case (ezp_msgID_ ## msgName):{ \
									ezp_ ## msgName ## _t *innerMsg = &(msg->msgName);\
									EZP_LOG("A " #msgName " Msg:\n" );
#define FIELD(type, fieldName) 		print_ ## type("\t" #fieldName, innerMsg->fieldName);
#define END_MSG(msgName) 		} break;


void ezp_printMsg(ezp_msg_t *msg){
	switch(msg->typeID){

		#include EZP_MSG_TABLE

		case (ezp_msgID_ack):
			EZP_LOG("An Ack msg:\n");
			break;

		default:
			EZP_ELOG("Unexpected msg type ID: %d\n", msg->typeID);
			return;
	}

	print_uint8_t("\t_seqNum:", msg->_seqNum);
}


#undef START_MSG
#undef FIELD
#undef END_MSG


// -----------------------------------------------------------------------------------------
// --------------------- End X Macro Msg Table Function Definitions ------------------------
// -----------------------------------------------------------------------------------------


void ezp_recvAndPrint(){
	// printf("-- Enter ezp_recvAndPrint\n");
	ezp_msg_t msg;
	EZP_RESULT res = ezp_recv(&msg);
	EZP_VLOG("ezp_recvAndPrint() ezp_recv gave: %s\n", ezp_res_str(res));
	if(res == EZP_OK){
		EZP_LOG("received: ");
		ezp_printMsg(&msg);
	}else{
	}
}


/******************************************************************************/
/* UTILITY                                                                    */
/******************************************************************************/

// static EZP_RESULT _sanitize_result = EZP_EVERSION;

// static EZP_RESULT ezp_message_sanitize(ezp_msg_t* msg, int send)
// {
// 	switch(msg->type) {
// 	case EZP_MSG_INIT:
// 		// if(!send) {
// 		// 	// If the reported version is less than the minimum compatible version
// 		// 	// then all messages are considered invalid.
// 		// 	if(msg->version >= EZP_VERSION_COMPAT) {
// 		// 		_sanitize_result = EZP_OK;
// 		// 	} else {
// 		// 		_sanitize_result = EZP_EVERSION;
// 		// 	}
// 		// } else {
// 			return EZP_OK;
// 		// }
// 		break;
// 	case EZP_MSG_RESET:
// 		for(int i=0; i<EZP_MSG_SIZE; ++i) {
// 			msg->data[i] = 0;
// 		}
// 		break;
// 	case EZP_MSG_VOLUME:
// 		if(msg->volume > 100) {
// 			msg->volume = 100;
// 		}
// 		if(msg->volume < 0) {
// 			msg->volume = 0;
// 		}
// 		break;
// 	case EZP_MSG_VOLUME_CHANGE:
// 		if(msg->volume > 100) {
// 			msg->volume = 100;
// 		}
// 		if(msg->volume < -100) {
// 			msg->volume = -100;
// 		}
// 		break;
// 	case EZP_MSG_MUTE:
// 		msg->mute_state._unused = 0;
// 		break;
// 	case EZP_MSG_PLAY_STATE:
// 		msg->play_state._unused = 0;
// 		break;
// 	case EZP_MSG_NETWORK_STATE:
// 		switch(msg->network_state.interface) {
// 		case EZP_NETWORK_NONE:
// 		case EZP_NETWORK_SOFTAP:
// 		case EZP_NETWORK_STA:
// 			break;
// 		default:
// 			// Not a valid network ID.
// 			return EZP_EINVALID;
// 		}
// 		switch(msg->network_state.state) {
// 		case EZP_CONNECTION_DISCONNECTED:
// 		case EZP_CONNECTION_CONNECTING:
// 		case EZP_CONNECTION_CONNECTED:
// 			break;
// 		default:
// 			// Not a valid connection ID.
// 			return EZP_EINVALID;
// 		}
// 		msg->network_state._unused = 0;
// 		break;
// 	case EZP_MSG_TRACK_SKIP:
// 		break;
// 	case EZP_MSG_AUDIO_CONFIG:
// 		switch(msg->audio_config.rate) {
// 			case 44100:
// 			case 48000:
// 			case 88200:
// 			case 96000:
// 				break;
// 			default:
// 				return EZP_EINVALID;
// 		}
// 		switch(msg->audio_config.bits) {
// 			case 16:
// 			case 24:
// 			case 32:
// 				break;
// 			default:
// 				return EZP_EINVALID;
// 		}
// 		break;
// 	default:
// 		return EZP_EUNKNOWN;
// 		break;
// 	}
// 	// return _sanitize_result;
// 	return EZP_OK;
// }


static EZP_RESULT Msgr_sendAckOf(ezp_master_t *msgr, ezp_msg_t *msg){
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


static void Msgr_considerSend(ezp_master_t *msgr){
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


static void Msgr_handleAck(ezp_master_t *msgr, ezp_msg_t *ack){
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

static void Msgr_handleNonAck(ezp_master_t *msgr, ezp_msg_t *msg){

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

static void Msgr_handleAnyRecvdMsgs(ezp_master_t *msgr){
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


EZP_RESULT Msgr_Process(ezp_master_t *msgr){
	EZP_VLOG(">> %s\n", __func__);
	EZP_ASSERT(msgr->initted);

	Msgr_considerSend(msgr);
	Msgr_handleAnyRecvdMsgs(msgr);

	return EZP_OK;
}

EZP_RESULT ezp_EZPProcess(){
	return Msgr_Process(&g_msgr);
}

EZP_RESULT ezp_onRecvChar(unsigned char ch){
	return EZP_OK;
}

/******************************************************************************/
/* PRIVATE BUFFERED INPUT                                                     */
/******************************************************************************/

#define EZP_BUFFER_SIZE 16
#define EZP_BUFFER_MASK (EZP_BUFFER_SIZE - 1)
#define EZP_BUFFER_INC(x, by) (((x) + by) & EZP_BUFFER_MASK)

static unsigned char _buffer[EZP_BUFFER_SIZE];
static int _buffer_head = 0;
static int _buffer_tail = 0;
static int _buffer_pos = 0;

static EZP_RESULT _ezp_buffer_getc(unsigned char* byte)
{
	EZP_ASSERT(byte != NULL);

	if(_buffer_pos == _buffer_head) {
		int next = EZP_BUFFER_INC(_buffer_head, 1);
		if(next == _buffer_tail) return EZP_EAGAIN;
		if(ezp_platform_getc(&_buffer[_buffer_head]) != EZP_OK) return EZP_EAGAIN;
		_buffer_head = next;
	}

	*byte = _buffer[_buffer_pos];
	_buffer_pos = EZP_BUFFER_INC(_buffer_pos, 1);

	// EZP_VLOG("_ezp_buffer_getc: %d\n", (int)*byte);
	return EZP_OK;
}

static EZP_RESULT _ezp_buffer_advance(int by)
{
	// EZP_VLOG("_ezp_buffer_advance by %d\n", by);
	int used = (_buffer_head - _buffer_tail) & EZP_BUFFER_MASK;
	if(by > used) {
		by = used;
	}
	_buffer_tail = EZP_BUFFER_INC(_buffer_tail, by);
	_buffer_pos = _buffer_tail;
	return EZP_OK;
}

static void _ezp_buffer_reset()
{
	// EZP_VLOG("_ezp_buffer_reset\n");
	_buffer_pos = _buffer_tail;
}

/******************************************************************************/
/* COMMUNICATION INTERFACE                                                    */
/******************************************************************************/

EZP_RESULT Msgr_init(ezp_master_t *msgr){
	// EZP_ASSERT(!initted);

	// msgr->initted = EZP_TRUE;
	msgr->lastSeqNumInSendBuffer = 0;
	msgr->lastSeqNumAckRecvd = NONE;
	msgr->lastSeqNumFullyRecvd = NONE;
	msgr->timeTillRetry = 0;

	// msgRingbuff_t msgSendQueue;
	// msgRingbuff_t msgRecvQueue;


	msgRingbuff_init( & msgr->msgSendQueue );

	msgr->initted = EZP_TRUE;
	return EZP_OK;
}


EZP_RESULT ezp_initEZP()
{
	EZP_LOG(">> %s)\n",__func__);


	Msgr_init(&g_msgr);

	return EZP_OK;
}


EZP_RESULT checkChecksum(uint8_t *buff, int size){
	csumCalculator_t calc;
	csumCalculator_init(&calc);
	for(int i=0; i < size-SIZEOF_CSUM; i++){
		csumCalculator_update(&calc, buff[i]);
	}

	if( csumCalculator_getCsum(&calc) == (buff[size-2] | (buff[size-1]<<8)) ){
		return EZP_OK;
	}else{
		return EZP_EINVALID;
	}
}

EZP_RESULT Msgr_tryReadMsg(ezp_master_t *msgr, ezp_msg_t *msg){
	uint8_t buff[32]; // TODO
	int buffIndex = 0;

	_ezp_buffer_reset();

	if( _ezp_buffer_getc(&buff[0]) != EZP_OK) return EZP_EAGAIN;

	//int totalSize = 1 + sizeOfMsgPayload(buff[0]) + 1;
	int totalSize;

	if(sizeOfWholeMsg(buff[0], &totalSize) != EZP_OK){
		// invalid type ID
		return EZP_EINVALID;
	}

	EZP_ASSERT(totalSize <= sizeof(buff));

	for(buffIndex=1; buffIndex<totalSize; buffIndex++){
		if( _ezp_buffer_getc(&buff[ buffIndex ]) != EZP_OK) return EZP_EAGAIN;
	}


	if(checkChecksum(buff, totalSize) == EZP_EINVALID){
		EZP_LOG("Checksum failure.\n");
		return EZP_EINVALID;
	}

	msgReader_t reader;
	initMsgReader(&reader, buff, totalSize);

	if(deserialize(msg, &reader) == EZP_OK){
		return EZP_OK;
	}
	EZP_LOG("Deserialization failure.\n");
	return EZP_EINVALID;
}


static EZP_RESULT Msgr_actualRecv(ezp_master_t *msgr, ezp_msg_t* msg)
{
	if(msg == 0) return EZP_EARG;


	while(1){

		// EZP_VLOG(">> Msgr_tryReadMsg\n");
		EZP_RESULT res = Msgr_tryReadMsg(msgr, msg);
		// EZP_VLOG("<< Msgr_tryReadMsg\n");

		if(res == EZP_EINVALID){
			// EZP_VLOG("res == EZP_EINVALID\n");
			_ezp_buffer_advance(1);
		}else if(res == EZP_OK){
			// EZP_VLOG("res == EZP_OK\n");
			// 1+sizeOfMsgPayload(msg->typeID)+1
			int msgSize;
			EZP_CHECK_OK( sizeOfWholeMsg(msg->typeID, &msgSize) );
			_ezp_buffer_advance(msgSize);
			return EZP_OK;
		}else{
			// EZP_VLOG("res == EZP_EAGAIN\n");
			_ezp_buffer_reset();
			return EZP_EAGAIN;
		}
	}

}


EZP_RESULT Msgr_recv(ezp_master_t *msgr, ezp_msg_t *msg){
	EZP_ASSERT(msgr != NULL);
	EZP_ASSERT(msg != NULL);

	return msgRingbuff_pop( & msgr->msgRecvQueue, msg);
}



EZP_RESULT Msgr_send(ezp_master_t *msgr, ezp_msg_t *msg){
	EZP_ASSERT(msgr != NULL);
	EZP_ASSERT(msg != NULL);

	msg->_seqNum = ++(msgr->lastSeqNumInSendBuffer);
	EZP_RESULT res = msgRingbuff_push( &(msgr->msgSendQueue), msg);
	if(res != EZP_OK){
		// TODO test this code path
		-- (msgr->lastSeqNumInSendBuffer);
	}

	return res;
}


EZP_RESULT ezp_recv(ezp_msg_t* msg){
	// EZP_RESULT res = msgRingbuff_pop( &msgRecvQueue, msg);

	// return res;

	return Msgr_recv( &g_msgr, msg);
}

EZP_RESULT ezp_send(ezp_msg_t *msg){
	// msg->_seqNum = ++lastSeqNumInSendBuffer;
	// EZP_RESULT res = msgRingbuff_push( &msgSendQueue, msg);
	// if(res != EZP_OK){
	// 	// TODO test this code path
	// 	--lastSeqNumInSendBuffer;
	// }

	// return res;

	return Msgr_send(&g_msgr, msg);
}


static inline void ezp_send_fooAB(int a, int b)
{
	printf(">> ezp_send_fooAB(%d,%d)\n",a,b);
	ezp_msg_t msg;
	msg.typeID = ezp_msgID_foo;
	msg.foo.a = a;
	msg.foo.b = b;
	ezp_send(&msg);

}

// /******************************************************************************/
// /* MESSAGE SENDING                                                            */
// /******************************************************************************/

// // static EZP_RESULT ezp_send_command(unsigned char id)
// // {
// // 	ezp_msg_t msg;
// // 	msg.type = id;
// // 	return ezp_send(&msg);
// // }

// EZP_RESULT ezp_send_reset()
// {
// 	// return ezp_send_command(EZP_MSG_RESET);
// 	return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_network_state(EZP_NETWORK interface, EZP_CONNECTION state)
// {
// 	// ezp_msg_t msg;
// 	// msg.type = EZP_MSG_NETWORK_STATE;
// 	// msg.network_state.interface = interface;
// 	// msg.network_state.state = state;
// 	// return ezp_send(&msg);
// 	return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_start_pairing()
// {
// 	return EZP_OK; //TODO return ezp_send_network_state(EZP_NETWORK_SOFTAP, EZP_CONNECTION_CONNECTED);
// }

// EZP_RESULT ezp_send_stop_pairing()
// {
// 	return EZP_OK; //TODOreturn ezp_send_network_state(EZP_NETWORK_STA, EZP_CONNECTION_CONNECTED);
// }

// EZP_RESULT ezp_send_volume(int volume)
// {
// 	// printf()
// 	ezp_msg_t msg;
// 	msg.typeID = EZP_msgID_volume;
// 	msg.volume.value = volume;
// 	return ezp_send(&msg);
// 	// return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_volume_change(int volume)
// {
// // 	ezp_msg_t msg;
// // 	msg.type = EZP_MSG_VOLUME_CHANGE;
// // 	msg.volume = volume;
// // 	return ezp_send(&msg);
// return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_mute(int on)
// {
// 	// ezp_msg_t msg;
// 	// msg.type = EZP_MSG_MUTE;
// 	// msg.mute_state.on = on;
// 	// msg.mute_state.toggle = 0;
// 	// return ezp_send(&msg);
// 	return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_mute_toggle()
// {
// 	// ezp_msg_t msg;
// 	// msg.type = EZP_MSG_MUTE;
// 	// msg.mute_state.on = 1;
// 	// msg.mute_state.toggle = 1;
// 	// return ezp_send(&msg);
// 	return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_play()
// {
// // 	ezp_msg_t msg;
// // 	msg.type = EZP_MSG_PLAY_STATE;
// // 	msg.play_state.playing = 1;
// // 	msg.play_state.toggle = 0;
// // 	return ezp_send(&msg);
// 	return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_play_toggle()
// {
// 	// ezp_msg_t msg;
// 	// msg.type = EZP_MSG_PLAY_STATE;
// 	// msg.play_state.playing = 1;
// 	// msg.play_state.toggle = 1;
// 	// return ezp_send(&msg);
// 	return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_stop()
// {
// 	// ezp_msg_t msg;
// 	// msg.type = EZP_MSG_PLAY_STATE;
// 	// msg.play_state.playing = 0;
// 	// msg.play_state.toggle = 0;
// 	// return ezp_send(&msg);
// 	return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_track_skip(int direction)
// {
// 	// ezp_msg_t msg;
// 	// msg.type = EZP_MSG_TRACK_SKIP;
// 	// msg.direction = direction;
// 	// return ezp_send(&msg);
// 	return EZP_OK; //TODO
// }

// EZP_RESULT ezp_send_audio_config(int master, int rate, int bits, int external_codec, int soft_volume)
// {
// 	// ezp_msg_t msg;
// 	// msg.type = EZP_MSG_AUDIO_CONFIG;
// 	// msg.audio_config.master = master;
// 	// msg.audio_config.rate = rate;
// 	// msg.audio_config.bits = bits;
// 	// msg.audio_config.external_codec = external_codec;
// 	// msg.audio_config.soft_volume = soft_volume;
// 	// return ezp_send(&msg);
// 	return EZP_OK; //TODO
// }


// void sendFooABTest(){
// 	printf("--------foo send ab test -------------\n");
// 	ezp_recvAndPrint();
// 	printf("--------------------------------------\n");
// 	ezp_send_fooAB(10,20);
// 	printf("--------------------------------------\n");
// 	ezp_recvAndPrint();
// 	printf("--------------------------------------\n");
// }

// void sendVolumeTest(){
// 	printf("--------foo send volume -------------\n");
// 	ezp_recvAndPrint();
// 	printf("--------------------------------------\n");
// 	ezp_send_volume(54);
// 	printf("--------------------------------------\n");
// 	ezp_recvAndPrint();
// 	printf("--------------------------------------\n");
// }



void sendFooABTwiceTest(){
	printf("--------foo send ab twice test -------------\n");
	ezp_recvAndPrint();
	ezp_recvAndPrint();
	printf("--------------------------------------\n");
	ezp_send_fooAB(10,20);
	ezp_send_fooAB(10,20);
	printf("--------------------------------------\n");
	ezp_recvAndPrint();
	ezp_recvAndPrint();
	ezp_recvAndPrint();
	printf("--------------------------------------\n");
}

void sendFooABTwiceWithDummyValueTest1(){
	printf("-------- %s ----------\n", __func__);
	ezp_recvAndPrint();
	ezp_recvAndPrint();
	printf("--------------------------------------\n");
	ezp_send_fooAB(10,20);
	uint8_t x = 0;
	ezp_platform_putc(x);
	ezp_send_fooAB(10,20);
	printf("--------------------------------------\n");
	ezp_recvAndPrint();
	ezp_recvAndPrint();
	ezp_recvAndPrint();
	printf("--------------------------------------\n");
}

void sendFooABTwiceWithDummyValueTest2(){
	printf("-------- %s ----------\n", __func__);
	ezp_recvAndPrint();
	ezp_recvAndPrint();
	printf("--------------------------------------\n");
	ezp_send_fooAB(10,20);
	uint8_t x = 17;
	ezp_platform_putc(x);
	ezp_send_fooAB(10,20);
	printf("--------------------------------------\n");
	ezp_recvAndPrint();
	ezp_recvAndPrint();
	ezp_recvAndPrint();
	printf("--------------------------------------\n");
}

void sendFooABPartialTest(){
	printf("-------- %s ----------\n", __func__);

	uint8_t fooAB_bytes[] = {0,10,20,28};

	ezp_platform_putc(fooAB_bytes[0]);
	ezp_platform_putc(fooAB_bytes[1]);
	ezp_recvAndPrint();
	printf("--------------------------------------\n");
	ezp_platform_putc(fooAB_bytes[2]);
	ezp_recvAndPrint();


	printf("--------------------------------------\n");
	ezp_platform_putc(fooAB_bytes[3]);
	ezp_recvAndPrint();

	printf("--------------------------------------\n");
}



void loopbackTest();

void runTests(){
	printf("--------------------------------------\n");
	printf("running tests.\n");
	// checksumTest();
	// loopbackTest();
	// sendFooABTest();
	// sendFooABTwiceTest();
	// sendFooABTwiceWithDummyValueTest1();
	// sendFooABTwiceWithDummyValueTest2();
	// sendFooABPartialTest();
	// sendVolumeTest();

	msgRingbuff_tests();

	printf("--------------------------------------\n");
}
#endif
#endif