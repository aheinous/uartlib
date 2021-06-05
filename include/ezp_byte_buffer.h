#pragma once

#include "ezp_types.h"

typedef struct {
    uint8_t *m_data;
    uint8_t m_size;
	volatile uint8_t m_writeIndex; 
	volatile uint8_t m_readIndex;
} byteBuff_t;

// max size 256
void byteBuff_init(byteBuff_t *self, uint8_t *m_data, uint8_t size);

ezp_bool_t byteBuff_isFull(byteBuff_t *self);

ezp_bool_t byteBuff_isEmpty(byteBuff_t *self);

EZP_RESULT byteBuff_push(byteBuff_t *self, uint8_t byte);

EZP_RESULT byteBuff_peek(byteBuff_t *self, uint8_t *pbyte);

EZP_RESULT byteBuff_pop(byteBuff_t *self, uint8_t *pbyte);

