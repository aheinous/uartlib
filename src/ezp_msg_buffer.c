#include "ezp_msg_buffer.h"

#if 1

#define INCED(index) ((index)+1)
#define WRAPPED(index) ((index) & (self->m_capacity-1))




EZP_RESULT msgRingbuff_init(msg_buff_t *self, ezp_msg_t *buff, uint8_t len) {
	self->m_writeIndex = 0;
	self->m_readIndex = 0;
	self->m_data = buff;
	self->m_capacity = len;

	return EZP_OK;
}

ezp_bool_t msgRingbuff_isFull(msg_buff_t *self){
	// separating volatile access to two lines suppresses UB warning in IAR
	uint8_t writeIndex = self->m_writeIndex;
	return ((uint8_t)(writeIndex - self->m_readIndex)) == self->m_capacity;
}

ezp_bool_t msgRingbuff_isEmpty(msg_buff_t *self){
	// separating volatile access to two lines suppresses UB warning in IAR
	uint8_t writeIndex = self->m_writeIndex;
	return (writeIndex == self->m_readIndex);
}

uint8_t msgRingbuff_size(msg_buff_t *self){
	uint8_t writeIndex = self->m_writeIndex;
	return writeIndex - self->m_readIndex;
}


EZP_RESULT msgRingbuff_push(msg_buff_t *self, ezp_msg_t *msg){
	if(msgRingbuff_isFull(self)){
		return EZP_EAGAIN;
	}
	self->m_data[ WRAPPED(self->m_writeIndex) ] = *msg;
	self->m_writeIndex = INCED(self->m_writeIndex);
	return EZP_OK;
}

EZP_RESULT msgRingbuff_peek(msg_buff_t *self, ezp_msg_t **msg){
	if(msgRingbuff_isEmpty(self)){
		return EZP_EAGAIN;
	}
	*msg = & (self->m_data[ WRAPPED(self->m_readIndex) ]);
	return EZP_OK;
}


EZP_RESULT msgRingbuff_pop(msg_buff_t *self, ezp_msg_t *msg){
	if(msgRingbuff_isEmpty(self)){
		return EZP_EAGAIN;
	}
	if(msg != NULL){
		*msg = self->m_data[ WRAPPED(self->m_readIndex) ];
	}

	self->m_readIndex = INCED( self->m_readIndex );
	return EZP_OK;
}



#endif
