#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "ezp_byte_buffer.h"
#include "ezp_msg.h"

typedef struct{
    byteBuff_t byteBuff;
    uint8_t pos;
}msg_reader_t;


EZP_RESULT msgReader_init(msg_reader_t *self, uint8_t *buff, uint8_t len);

EZP_RESULT msgReader_push_byte(msg_reader_t *, uint8_t );

EZP_RESULT msgReader_read_msg(msg_reader_t*, ezp_msg_t*);

#ifdef __cplusplus
}
#endif
