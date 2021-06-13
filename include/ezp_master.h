#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "ezp_types.h"
#include "ezp_msg_buffer.h"
#include "ezp_byte_buffer.h"
#include "ezp_msg_reader.h"
#include "ezp_csum.h"
#include "ezp_platform.h"
#include "ezp_util.h"


#define EZP_RECV_BUFFER_MIN_SIZE EZP_ROUND_UP_POW2(sizeof(ezp_msg_t) + EZP_SIZEOF_CSUM)



typedef struct {
    ezp_platform_t m_platform;

    int32_t m_time_till_retry;
    int32_t m_retry_intval;

    msg_buff_t m_send_queue;
    msg_reader_t m_msg_reader;

    int16_t m_next_send_seq_num;
    int16_t m_last_seq_num_ack_recvd;
    int16_t m_last_seq_num_recvd_and_handled;

    uint8_t m_consecutive_unexpected_seq_nums;

    ezp_bool_t m_initted;
} ezp_master_t;


EZP_RESULT master_init(ezp_master_t *self,
                        uint8_t *recv_buffer_data, uint8_t recv_buff_len,
                        ezp_msg_t *send_buffer_data, uint8_t send_buffer_len,
                        ezp_platform_t platform, int32_t retry_intval);
EZP_RESULT master_process(ezp_master_t *self, int32_t time_elapsed);
EZP_RESULT master_enqueue(ezp_master_t *self, ezp_msg_t *msg);
EZP_RESULT master_on_recv_byte(ezp_master_t *self, uint8_t b);



#ifdef __cplusplus
}
#endif
