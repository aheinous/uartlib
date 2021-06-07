#pragma once

#include "ezp_types.h"
#include "ezp_msg.h"



typedef struct {
	ezp_msg_t * m_data;
	uint8_t m_capacity;
	volatile uint8_t m_writeIndex;
	volatile uint8_t m_readIndex;
} msgRingbuff_t;



EZP_RESULT msgRingbuff_init(msgRingbuff_t *self, ezp_msg_t *buff, uint8_t len);

ezp_bool_t msgRingbuff_isFull(msgRingbuff_t *self);

ezp_bool_t msgRingbuff_isEmpty(msgRingbuff_t *self);

EZP_RESULT msgRingbuff_push(msgRingbuff_t *self, ezp_msg_t *msg);

EZP_RESULT msgRingbuff_peek(msgRingbuff_t *self, ezp_msg_t **msg);

// invalidates result of peek
EZP_RESULT msgRingbuff_pop(msgRingbuff_t *self, ezp_msg_t *msg);

