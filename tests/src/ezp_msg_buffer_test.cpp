#include "catch.hpp"

extern "C"{
    #include "ezp_msg_buffer.h"
}


SCENARIO("Using msgBuffer"){
    GIVEN("just initted"){
        msgRingbuff_t self;
        ezp_msg_t msg;

        msgRingbuff_init(&self);

        THEN("empty"){
            REQUIRE(msgRingbuff_isEmpty(&self));
        }

        WHEN("has 3/4 elems"){
            for(int i=0; i<3; i++){
                REQUIRE_FALSE(  msgRingbuff_isFull(&self));
                msg.typeID = i;
                REQUIRE(msgRingbuff_push(&self, &msg) == EZP_OK);
                REQUIRE_FALSE(msgRingbuff_isEmpty(&self));
            }

            THEN("not full"){
                REQUIRE_FALSE(msgRingbuff_isFull(&self));
            }
        }

        WHEN("has 4/4 elems"){
            for(int i = 0; i < 4; i++) {
                REQUIRE_FALSE(msgRingbuff_isFull(&self));
                msg.typeID = i;
                REQUIRE(msgRingbuff_push(&self, &msg) == EZP_OK);
                REQUIRE_FALSE(msgRingbuff_isEmpty(&self));
            }

            THEN("is full"){
                REQUIRE(msgRingbuff_isFull(&self));
            }

            THEN("can pop 4"){
                ezp_msg_t *pmsg;
                for(int i=0; i<4; i++){
                    REQUIRE_FALSE( msgRingbuff_isEmpty(&self));
                    REQUIRE( msgRingbuff_peek(&self,&pmsg) == EZP_OK);
                    REQUIRE(pmsg->typeID == i);
                    REQUIRE(msgRingbuff_pop(&self, &msg) == EZP_OK);
                    REQUIRE(msg.typeID == i);
                    REQUIRE_FALSE( msgRingbuff_isFull(&self));
                }
                THEN("is empty"){
                    REQUIRE( msgRingbuff_isEmpty(&self));
                    REQUIRE( msgRingbuff_peek(&self,&pmsg) == EZP_EAGAIN);
                }
            }

        }
    }
}