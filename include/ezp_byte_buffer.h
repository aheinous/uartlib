#pragma once

#include "ezp_types.h"


#define EZP_BYTE_BUFFER_CAPACITY (32)

typedef struct {
    uint8_t m_data[EZP_BYTE_BUFFER_CAPACITY];
	uint8_t m_writeIndex;
	uint8_t m_readIndex;
} byteBuff_t;

// max size 256
void byteBuff_init(byteBuff_t *self);

ezp_bool_t byteBuff_isFull(byteBuff_t *self);

ezp_bool_t byteBuff_isEmpty(byteBuff_t *self);

uint8_t byteBuff_size(byteBuff_t *self);

EZP_RESULT byteBuff_push(byteBuff_t *self, uint8_t byte);

EZP_RESULT byteBuff_peek(byteBuff_t *self, uint8_t idx, uint8_t *pbyte);

EZP_RESULT byteBuff_pop(byteBuff_t *self, uint8_t count);

