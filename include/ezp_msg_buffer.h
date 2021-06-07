#pragma once

#include "ezp_types.h"
#include "ezp_msg.h"

#ifndef EZP_MSG_BUFF_SIZE
#define EZP_MSG_BUFF_SIZE 4
#endif
// max size 128

typedef struct {
	ezp_msg_t m_buff[EZP_MSG_BUFF_SIZE];
	volatile uint8_t m_writeIndex;
	volatile uint8_t m_readIndex;
} msgRingbuff_t;



void msgRingbuff_init(msgRingbuff_t *self);

ezp_bool_t msgRingbuff_isFull(msgRingbuff_t *self);

ezp_bool_t msgRingbuff_isEmpty(msgRingbuff_t *self);

EZP_RESULT msgRingbuff_push(msgRingbuff_t *self, ezp_msg_t *msg);

EZP_RESULT msgRingbuff_peek(msgRingbuff_t *self, ezp_msg_t **msg);

// invalidates result of peek
EZP_RESULT msgRingbuff_pop(msgRingbuff_t *self, ezp_msg_t *msg);

