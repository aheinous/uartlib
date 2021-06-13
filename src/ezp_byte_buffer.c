#include "ezp_byte_buffer.h"


#define INCED(index) (((index)+1) & (self->m_capacity - 1))
#define WRAPPED(index) (((uint8_t)(index)) & (self->m_capacity- 1))


EZP_RESULT byteBuff_init(byteBuff_t *self, uint8_t *buff, uint8_t len) {
	self->m_writeIndex = 0;
	self->m_readIndex = 0;
	self->m_data = buff;
	self->m_capacity = len;
	return EZP_OK;
}

ezp_bool_t byteBuff_isFull(byteBuff_t *self){
	// separating volatile access to two lines suppresses UB warning in IAR
	uint8_t m_writeIndex = self->m_writeIndex;
	return (INCED(m_writeIndex) == self->m_readIndex);
}

ezp_bool_t byteBuff_isEmpty(byteBuff_t *self){
	// separating volatile access to two lines suppresses UB warning in IAR
	uint8_t m_writeIndex = self->m_writeIndex;
	return (m_writeIndex == self->m_readIndex);
}


uint8_t byteBuff_size(byteBuff_t *self){
	return WRAPPED(self->m_writeIndex - self->m_readIndex) ;
}

EZP_RESULT byteBuff_push(byteBuff_t *self, uint8_t byte){
	if(byteBuff_isFull(self)){
		return EZP_EAGAIN;
	}
	self->m_data[ self->m_writeIndex ] = byte;
	self->m_writeIndex = INCED(self->m_writeIndex);
	return EZP_OK;
}

EZP_RESULT byteBuff_peek(byteBuff_t *self,  uint8_t idx, uint8_t *pbyte){
	if(idx >= byteBuff_size(self)){
		return EZP_EAGAIN;
	}
	uint8_t read_idx = WRAPPED(self->m_readIndex + idx);
	*pbyte = (self->m_data[ read_idx ]);
	return EZP_OK;
}


EZP_RESULT byteBuff_pop(byteBuff_t *self, uint8_t count){
	// EZP_VLOG(">> %s\n", __func__);

	if(byteBuff_size(self) < count){
		return EZP_EARG;
	}

	self->m_readIndex = WRAPPED( self->m_readIndex + count );
	return EZP_OK;
}
