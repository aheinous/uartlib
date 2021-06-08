#pragma once

#if defined(__cplusplus)
extern "C" {
#endif


#include "ezp_types.h"
#include "ezp_msg.h"



typedef struct {
	ezp_msg_t * m_data;
	uint8_t m_capacity;
	volatile uint8_t m_writeIndex;
	volatile uint8_t m_readIndex;
} msg_buff_t;



EZP_RESULT msgRingbuff_init(msg_buff_t *self, ezp_msg_t *buff, uint8_t len);

ezp_bool_t msgRingbuff_isFull(msg_buff_t *self);

ezp_bool_t msgRingbuff_isEmpty(msg_buff_t *self);

uint8_t msgRingbuff_size(msg_buff_t *self);



EZP_RESULT msgRingbuff_push(msg_buff_t *self, ezp_msg_t *msg);

EZP_RESULT msgRingbuff_peek(msg_buff_t *self, ezp_msg_t **msg);

// invalidates result of peek
EZP_RESULT msgRingbuff_pop(msg_buff_t *self, ezp_msg_t *msg);


#if defined(__cplusplus)
}
#endif
