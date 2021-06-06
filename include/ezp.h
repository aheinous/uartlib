#pragma once
#if 0
#if defined(__cplusplus)
extern "C" {
#endif


#include "ezp_types.h"
#include "ezp_util.h"
#include "ezp_platform.h"

// for debugging
#define EZP_VLOG_INT(expn) EZP_VLOG( #expn ": %d\n", (int) expn)
#define EZP_LOG_INT(expn) EZP_LOG( #expn ": %d\n", (int) expn)
#define EZP_ELOG_INT(expn) EZP_ELOG( #expn ": %d\n", (int) expn)
#define EZP_WLOG_INT(expn) EZP_WLOG( #expn ": %d\n", (int) expn)


EZP_RESULT ezp_init();
EZP_RESULT ezp_send(ezp_msg_t* msg);
EZP_RESULT ezp_on_recv_byte(uint8_t b);
EZP_RESULT ezp_process();


#if defined(__cplusplus)
}
#endif
#endif