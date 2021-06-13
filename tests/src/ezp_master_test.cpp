#include "ezp_master.h"
#include "ezp_util.h"
#include "catch.hpp"
#include "ezp_msg_buffer.h"


ezp_master_t a_master;
ezp_master_t b_master;

msg_buff_t a_recv_buff;
msg_buff_t b_recv_buff;


EZP_RESULT a_write_byte(void* usr_data, uint8_t b) {
    master_on_recv_byte(&b_master, b);
    return EZP_OK;
}

int a_flush_calls = 0;

EZP_RESULT a_flush(void* usr_data) {
    ++a_flush_calls;
    return EZP_OK;
}

EZP_RESULT a_on_recv_msg(void* usr_data, ezp_msg_t *msg) {
    return EZP_OK;
}

EZP_RESULT b_write_byte(void* usr_data, uint8_t b) {
    master_on_recv_byte(&a_master, b);
    return EZP_OK;
}

int b_flush_calls = 0;

EZP_RESULT b_flush(void* usr_data) {
    ++b_flush_calls;
    return EZP_OK;
}

EZP_RESULT b_on_recv_msg(void* usr_data, ezp_msg_t *msg) {
    msgRingbuff_push(&b_recv_buff, msg);
    return EZP_OK;
}




TEST_CASE("basic init") {

    SECTION("initted properly"){
        uint8_t a_bytes[EZP_RECV_BUFFER_MIN_SIZE * 2];
        ezp_msg_t a_msgs[4];

        ezp_platform_t a_platform;
        a_platform.write_byte = a_write_byte;
        a_platform.flush = a_flush;
        a_platform.on_recv_msg = a_on_recv_msg;

        EZP_RESULT res = master_init(&a_master, a_bytes, sizeof(a_bytes), a_msgs, countof(a_msgs), a_platform, 100);

        THEN("no error"){
            REQUIRE(res == EZP_OK);
        }

        SECTION("process called with nothing"){
            res = master_process(&a_master, 10);
            THEN("no error") {
                REQUIRE(res == EZP_OK);
            }
        }
    }
}


TEST_CASE("send and recv"){

    a_flush_calls = 0;
    b_flush_calls = 0;


    SECTION("init both + recv buffs"){

        uint8_t a_bytes[EZP_RECV_BUFFER_MIN_SIZE * 2];
        ezp_msg_t a_msgs[4];

        ezp_platform_t a_platform;
        a_platform.write_byte = a_write_byte;
        a_platform.flush = a_flush;
        a_platform.on_recv_msg = a_on_recv_msg;

        REQUIRE(master_init(&a_master, a_bytes, sizeof(a_bytes), a_msgs, countof(a_msgs), a_platform, 100) == EZP_OK);

        uint8_t b_bytes[EZP_RECV_BUFFER_MIN_SIZE * 2];
        ezp_msg_t b_msgs[4];

        ezp_platform_t b_platform;
        b_platform.write_byte = b_write_byte;
        b_platform.flush = b_flush;
        b_platform.on_recv_msg = b_on_recv_msg;

        REQUIRE(master_init(&b_master, b_bytes, sizeof(b_bytes), b_msgs, countof(b_msgs), b_platform, 100) == EZP_OK);

        ezp_msg_t a_recv_data[4];
        REQUIRE( msgRingbuff_init(&a_recv_buff, a_recv_data, countof(a_recv_data)) == EZP_OK);

        ezp_msg_t b_recv_data[4];
        REQUIRE( msgRingbuff_init(&b_recv_buff, b_recv_data, countof(b_recv_data)) == EZP_OK);





        SECTION("msg enqueued") {
            ezp_msg_t a_msg;
            a_msg.typeID = ezp_msgID_foo;
            a_msg.foo.a = 1;
            a_msg.foo.b = 2;

            ezp_msg_t b_msg;
            REQUIRE(EZP_OK == master_enqueue(&a_master, &a_msg));

            SECTION("b receives"){
                REQUIRE(EZP_OK == master_process(&a_master, 10));
                REQUIRE(EZP_OK == master_process(&b_master, 10));

                REQUIRE(msgRingbuff_size(& b_recv_buff) == 1);
                REQUIRE(msgRingbuff_pop(& b_recv_buff, &b_msg) == EZP_OK);

                REQUIRE(ezp_msgID_foo == b_msg.typeID);
                REQUIRE(1 == b_msg.foo.a);
                REQUIRE(2 == b_msg.foo.b);

                SECTION("a receives ack") {
                    REQUIRE(EZP_OK == master_process(&a_master, 10));
                    REQUIRE(a_master.m_last_seq_num_ack_recvd == 0);
                    REQUIRE(msgRingbuff_isEmpty(&a_master.m_send_queue));
                }
            }


            SECTION("a sends -- b does nothing -- a resends at right timeout -- b sends 2 acks"){
                REQUIRE(a_flush_calls == 0);
                REQUIRE(EZP_OK == master_process(&a_master, 10));
                REQUIRE(a_flush_calls == 1);
                REQUIRE(EZP_OK == master_process(&a_master, 99));
                REQUIRE(a_flush_calls == 1);
                REQUIRE(EZP_OK == master_process(&a_master, 1));
                REQUIRE(a_flush_calls == 2);

                REQUIRE(b_flush_calls == 0);
                REQUIRE(EZP_OK == master_process(&b_master, 1));
                REQUIRE(EZP_OK == master_process(&b_master, 1));
                REQUIRE(b_flush_calls == 2);

                REQUIRE(EZP_OK == master_process(&a_master, 10));
                REQUIRE(a_flush_calls == 2);
                REQUIRE(EZP_OK == master_process(&a_master, 110));
                REQUIRE(a_flush_calls == 2);

            }


        }
    }


}