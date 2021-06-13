#include "catch.hpp"
#include "ezp_byte_buffer_fake.h"
#include "ezp_master.h" // buffer recomended size

extern "C" {
    #include "ezp_msg_reader.h"
    #include "ezp_msg_sender.h"
}


std::vector<int> pops;


extern "C"{
    void on_byte_buff_popped(int count){
        pops.emplace_back(count);
    }
}

msg_reader_t reader;
msgSender_t sender;


uint8_t buff[EZP_RECV_BUFFER_MIN_SIZE * 2];

int flush_calls = 0;



EZP_RESULT write_byte(void* usr_data, uint8_t b) {
    REQUIRE(msgReader_push_byte(&reader, b) == EZP_OK);
    return EZP_OK;
}
EZP_RESULT flush(void* usr_data) {
    ++flush_calls;
    return EZP_OK;
}

EZP_RESULT on_recv_msg(void* usr_data, ezp_msg_t *m) {
    return EZP_OK;
}


TEST_CASE("round up pow2 marco test"){ // TODO move
    REQUIRE(EZP_ROUND_UP_POW2(1) == 1);
    REQUIRE(EZP_ROUND_UP_POW2(2) == 2);
    REQUIRE(EZP_ROUND_UP_POW2(3) == 4);
    REQUIRE(EZP_ROUND_UP_POW2(4) == 4);
    REQUIRE(EZP_ROUND_UP_POW2(5) == 8);
    REQUIRE(EZP_ROUND_UP_POW2(128) == 128);
    REQUIRE(EZP_ROUND_UP_POW2(255) == 256);
    REQUIRE(EZP_ROUND_UP_POW2(256) == 256);

}


TEST_CASE("reader sender loopback"){

    ezp_platform_t platform;
    platform.write_byte = write_byte;
    platform.flush = flush;
    platform.on_recv_msg = on_recv_msg;


    msgReader_init(&reader, buff, sizeof(buff));
    msgSender_init(&sender, platform);

    flush_calls = 0;
    pops.clear();

    // ezp_foo_t foo;
    // foo.a = 123;
    // foo.b = 221;

    SECTION("sending foo.a, foo.a"){
        ezp_msg_t msg;
        msg.typeID = ezp_msgID_foo;
        msg.foo.a = 0xAA;
        msg.foo.b = 0xBB;

        SECTION("flush called once"){
            flush_calls = 0;
            REQUIRE(msgSender_send(&sender, &msg) == EZP_OK);
            REQUIRE(flush_calls == 1);

        }

        SECTION("send then read works"){
            ezp_msg_t rmsg;
            REQUIRE(msgSender_send(&sender, &msg) == EZP_OK);
            REQUIRE(msgReader_read_msg(&reader, & rmsg) == EZP_OK);
            REQUIRE(rmsg.typeID == ezp_msgID_foo);
            REQUIRE(rmsg.foo.a == 0xAA);
            REQUIRE(rmsg.foo.b == 0xBB);

            SECTION("pop called after msg reported good") {
                REQUIRE(pops.size() == 0);
                REQUIRE(msgReader_on_msg_valid(&reader, &msg) == EZP_OK);
                REQUIRE(pops.size() == 1);
                REQUIRE(pops[0] > 1);
            }

            SECTION("pop called after msg reported bad") {
                REQUIRE(pops.size() == 0);
                REQUIRE(msgReader_on_msg_invalid(&reader) == EZP_OK);
                REQUIRE(pops.size() == 1);
                REQUIRE(pops[0] == 1);
            }

            REQUIRE(msgReader_on_msg_valid(&reader, &msg) == EZP_OK);



            SECTION("send then read works multiple times") {

                msg.typeID = ezp_msgID_bar;
                msg.bar.c = 0xCC;
                msg.bar.d = 0xDD;

                msgSender_send(&sender, &msg);
                REQUIRE(msgReader_read_msg(&reader, &rmsg) == EZP_OK);
                REQUIRE(msgReader_on_msg_valid(&reader, &rmsg) == EZP_OK);

                REQUIRE(rmsg.typeID == ezp_msgID_bar);
                REQUIRE(rmsg.bar.c == 0xCC);
                REQUIRE(rmsg.bar.d == 0xDD);

                SECTION("pop called twice") {
                    REQUIRE(pops.size() == 2);
                }
            }

        }

        SECTION("send then read works with garbage sent first") {

            platform.write_byte(platform.usr_data, 1);
            platform.write_byte(platform.usr_data, 2);


            ezp_msg_t rmsg;
            msgSender_send(&sender, &msg);
            REQUIRE(msgReader_read_msg(&reader, &rmsg) == EZP_OK);
            REQUIRE(msgReader_on_msg_valid(&reader, &rmsg) == EZP_OK);
            REQUIRE(rmsg.typeID == ezp_msgID_foo);
            REQUIRE(rmsg.foo.a == 0xAA);
            REQUIRE(rmsg.foo.b == 0xBB);

            SECTION("pop called three times") {
                REQUIRE(pops.size() == 3);
            }
        }

        SECTION("send then read works with out of range garbage sent first") {

            platform.write_byte(platform.usr_data, 1);
            platform.write_byte(platform.usr_data, 234);


            ezp_msg_t rmsg;
            msgSender_send(&sender, &msg);
            REQUIRE(msgReader_read_msg(&reader, &rmsg) == EZP_OK);
            REQUIRE(msgReader_on_msg_valid(&reader, &rmsg) == EZP_OK);
            REQUIRE(rmsg.typeID == ezp_msgID_foo);
            REQUIRE(rmsg.foo.a == 0xAA);
            REQUIRE(rmsg.foo.b == 0xBB);

            SECTION("pop called three times") {
                REQUIRE(pops.size() == 3);
            }
        }

    }


    SECTION("sending ack (zero payload"){
        ezp_msg_t msg;
        msg.typeID = ezp_msgID_ack;

        REQUIRE(msgSender_send(&sender, &msg) == EZP_OK);
        ezp_msg_t rmsg;
        REQUIRE(msgReader_read_msg(&reader, &rmsg) == EZP_OK);
        REQUIRE(msgReader_on_msg_valid(&reader, &rmsg) == EZP_OK);

        REQUIRE(rmsg.typeID == ezp_msgID_ack);

    }


}