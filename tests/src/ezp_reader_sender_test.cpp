#include "catch.hpp"
#include "ezp_byte_buffer_fake.h"


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

msgReader_t reader;
msgSender_t sender;

int flush_calls = 0;

extern "C"{

    EZP_RESULT ezp_platform_write_byte(uint8_t b) {
        REQUIRE(msgReader_push_byte(&reader, b) == EZP_OK);
        return EZP_OK;
    }
    EZP_RESULT ezp_platform_flush() {
        ++flush_calls;
        return EZP_OK;
    }

    EZP_RESULT ezp_platform_on_recv_msg(ezp_msg_t *m) {
        return EZP_OK;
    }
}


TEST_CASE("reader sender loopback"){
    msgReader_init(&reader);
    msgSender_init(&sender);

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

            SECTION("pop called once") {
                REQUIRE(pops.size() == 1);
            }


            SECTION("send then read works multiple times") {

                msg.typeID = ezp_msgID_bar;
                msg.bar.c = 0xCC;
                msg.bar.d = 0xDD;

                msgSender_send(&sender, &msg);
                REQUIRE(msgReader_read_msg(&reader, &rmsg) == EZP_OK);

                REQUIRE(rmsg.typeID == ezp_msgID_bar);
                REQUIRE(rmsg.bar.c == 0xCC);
                REQUIRE(rmsg.bar.d == 0xDD);

                SECTION("pop called twice") {
                    REQUIRE(pops.size() == 2);
                }
            }

        }

        SECTION("send then read works with garbage sent first") {

            ezp_platform_write_byte(1);
            ezp_platform_write_byte(2);


            ezp_msg_t rmsg;
            msgSender_send(&sender, &msg);
            REQUIRE(msgReader_read_msg(&reader, &rmsg) == EZP_OK);
            REQUIRE(rmsg.typeID == ezp_msgID_foo);
            REQUIRE(rmsg.foo.a == 0xAA);
            REQUIRE(rmsg.foo.b == 0xBB);

            SECTION("pop called three times") {
                REQUIRE(pops.size() == 3);
            }
        }

        SECTION("send then read works with out of range garbage sent first") {

            ezp_platform_write_byte(1);
            ezp_platform_write_byte(234);


            ezp_msg_t rmsg;
            msgSender_send(&sender, &msg);
            REQUIRE(msgReader_read_msg(&reader, &rmsg) == EZP_OK);
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

        REQUIRE(rmsg.typeID == ezp_msgID_ack);

    }


}