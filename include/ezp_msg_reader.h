#pragma once

#ifdef __cplusplus
extern "C" {
#endif






#include "ezp_byte_buffer.h"
#include "ezp_msg.h"

typedef struct{
    byteBuff_t byteBuff;
    uint8_t pos;
}msgReader_t;


void msgReader_init(msgReader_t*);

EZP_RESULT msgReader_push_byte(msgReader_t *, uint8_t );

EZP_RESULT msgReader_read_msg(msgReader_t*, ezp_msg_t*);

#ifdef __cplusplus
}
#endif
