#pragma once

#include "ezp_types.h"
#include "ezp_msg_buffer.h"
#include "ezp_byte_buffer.h"
#include "ezp_msg_reader.h"


typedef struct {
    ezp_bool_t initted;
    int16_t lastSeqNumInSendBuffer;
    int16_t lastSeqNumAckRecvd;

    int16_t lastSeqNumFullyRecvd;

    uint16_t timeTillRetry;

    msgRingbuff_t msgSendQueue;
    msgReader_t msg_reader;
} ezp_master_t;


EZP_RESULT master_init(ezp_master_t *self);
EZP_RESULT master_process(ezp_master_t *self);
EZP_RESULT master_enqueue(ezp_master_t *self, ezp_msg_t *msg);
EZP_RESULT master_onRecvByte(ezp_master_t *self, uint8_t b);



