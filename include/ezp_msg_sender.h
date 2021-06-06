#pragma once

#include "ezp_csum.h"
#include "ezp_types.h"
#include "ezp_msg.h"

typedef struct {
    csum_calc_t m_csum;
} msgSender_t;


void msgSender_init(msgSender_t *self);


EZP_RESULT msgSender_send(msgSender_t *self, ezp_msg_t*) ;

