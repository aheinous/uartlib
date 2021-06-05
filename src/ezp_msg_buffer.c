#include "ezp_msg_buffer.h"



#define INCED(index) ((index)+1)
#define WRAPPED(index) ((index) & (self->m_size-1))




void msgRingbuff_init(msgRingbuff_t *self, ezp_msg_t *buff, uint8_t size){
    self->m_buff = buff;
    self->m_size = size;
	self->m_writeIndex = 0;
	self->m_readIndex = 0;
}

ezp_bool_t msgRingbuff_isFull(msgRingbuff_t *self){
	// separating volatile access to two lines suppresses UB warning in IAR
	uint8_t writeIndex = self->m_writeIndex;
	return self->m_writeIndex - self->m_readIndex == self->m_size;
}

ezp_bool_t msgRingbuff_isEmpty(msgRingbuff_t *self){
	// separating volatile access to two lines suppresses UB warning in IAR
	uint8_t writeIndex = self->m_writeIndex;
	return (writeIndex == self->m_readIndex);
}

EZP_RESULT msgRingbuff_push(msgRingbuff_t *self, ezp_msg_t *msg){
	EZP_VLOG(">> %s\n", __func__);

	if(msgRingbuff_isFull(self)){
		return EZP_EAGAIN;
	}
	self->m_buff[ WRAPPED(self->m_writeIndex) ] = *msg;
	self->m_writeIndex = INCED(self->m_writeIndex);
	return EZP_OK;
}

EZP_RESULT msgRingbuff_peek(msgRingbuff_t *self, ezp_msg_t **msg){
	EZP_VLOG(">> %s\n", __func__);

	if(msgRingbuff_isEmpty(self)){
		return EZP_EAGAIN;
	}
	*msg = & (self->m_buff[ WRAPPED(self->m_readIndex) ]);
	return EZP_OK;
}


EZP_RESULT msgRingbuff_pop(msgRingbuff_t *self, ezp_msg_t *msg){
	EZP_VLOG(">> %s\n", __func__);

	if(msgRingbuff_isEmpty(self)){
		return EZP_EAGAIN;
	}
	if(msg != NULL){
		*msg = self->m_buff[ WRAPPED(self->m_readIndex) ];
	}

	self->m_readIndex = INCED( self->m_readIndex );
	return EZP_OK;
}




void msgRingbuff_tests(){

	msgRingbuff_t self;
	msgRingbuff_init(&self);
	printf("---------------------------------------------------------------\n");
	printf("running msgRingbuff_tests\n");
	ezp_msg_t msg;
	EZP_TEST_ASSERT( msgRingbuff_isEmpty(&self));

	for(int i=0; i<3; i++){
		printf("i: %d\n", i);
		EZP_TEST_ASSERT( ! msgRingbuff_isFull(&self));
		msg.typeID = i;
		EZP_TEST_ASSERT(msgRingbuff_push(&self, &msg) == EZP_OK);
		EZP_TEST_ASSERT( ! msgRingbuff_isEmpty(&self));
	}

	EZP_TEST_ASSERT( msgRingbuff_isFull(&self));
	EZP_TEST_ASSERT(msgRingbuff_push(&self, &msg) == EZP_EAGAIN);

	ezp_msg_t *pmsg;
	for(int i=0; i<3; i++){
		printf("i: %d\n", i);
		EZP_TEST_ASSERT( ! msgRingbuff_isEmpty(&self));
		EZP_TEST_ASSERT( msgRingbuff_peek(&self,&pmsg) == EZP_OK);
		EZP_TEST_ASSERT(pmsg->typeID == i);
		EZP_TEST_ASSERT(msgRingbuff_pop(&self, NULL) == EZP_OK);

		EZP_TEST_ASSERT( ! msgRingbuff_isFull(&self));
	}

	EZP_TEST_ASSERT( msgRingbuff_isEmpty(&self));
	EZP_TEST_ASSERT( msgRingbuff_peek(&self,&pmsg) == EZP_EAGAIN);
	printf("---------------------------------------------------------------\n");
}


