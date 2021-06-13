#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "ezp_csum.h"
#include "ezp_types.h"
#include "ezp_msg.h"
#include "ezp_platform.h"

typedef struct {
    csum_calc_t m_csum;
    ezp_platform_t m_platform;
} msgSender_t;


void msgSender_init(msgSender_t *self, ezp_platform_t platform);
EZP_RESULT msgSender_send(msgSender_t *self, ezp_msg_t*) ;

#ifdef __cplusplus
}
#endif