#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "ezp_types.h"
#include "ezp_msg.h"



// EZP_RESULT ezp_platform_read_byte(uint8_t*);

// should either block (which will cause process to block) or buffer
// the bytes to avoid having to return EZP_EAGAIN
typedef struct{
    EZP_RESULT (*write_byte)(void*, uint8_t);
    EZP_RESULT (*flush)(void*);
    EZP_RESULT (*on_recv_msg)(void*, ezp_msg_t*);
    void *usr_data;
} ezp_platform_t;


#if defined(__cplusplus)
}
#endif

