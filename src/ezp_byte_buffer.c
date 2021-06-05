#include "ezp_byte_buffer.h"

#define INCED(index) (((index)+1) & (self->m_size - 1))




void byteBuff_init(byteBuff_t *self, uint8_t *m_data, uint8_t size){
    self->m_data = m_data;
    self->m_size = size;
	self->m_writeIndex = 0;
	self->m_readIndex = 0;
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


EZP_RESULT byteBuff_push(byteBuff_t *self, uint8_t byte){
	// EZP_VLOG(">> %s\n", __func__);
	// EZP_ASSERT(msg != NULL); // ??

	if(byteBuff_isFull(self)){
		return EZP_EAGAIN;
	}
	self->m_data[ self->m_writeIndex ] = byte;
	self->m_writeIndex = INCED(self->m_writeIndex);
	return EZP_OK;
}

EZP_RESULT byteBuff_peek(byteBuff_t *self, uint8_t *pbyte){
	// EZP_VLOG(">> %s\n", __func__);
	// EZP_ASSERT(pbyte != NULL);

	if(byteBuff_isEmpty(self)){
		return EZP_EAGAIN;
	}
	*pbyte = (self->m_data[ self->m_readIndex ]);
	return EZP_OK;
}


EZP_RESULT byteBuff_pop(byteBuff_t *self, uint8_t *pbyte){
	// EZP_VLOG(">> %s\n", __func__);

	if(byteBuff_isEmpty(self)){
		return EZP_EAGAIN;
	}
	if(pbyte != NULL){
		*pbyte = self->m_data[ self->m_readIndex ];
	}


	self->m_readIndex = INCED( self->m_readIndex );
	return EZP_OK;
}
