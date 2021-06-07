#include "catch.hpp"

extern "C" {
#include "ezp_byte_buffer.h"
}



SCENARIO("BYTE BUFF USAGE"){
    GIVEN("just initted"){

        uint8_t _data_for_buff[32];

        byteBuff_t bbuff;
        byteBuff_init(&bbuff, _data_for_buff, sizeof(_data_for_buff));

        uint8_t b;

        THEN("empty"){
            REQUIRE_FALSE(byteBuff_isFull(&bbuff));
            REQUIRE(byteBuff_isEmpty(&bbuff));
            REQUIRE(byteBuff_size(&bbuff) == 0);
            REQUIRE(byteBuff_peek(&bbuff, 0, &b) == EZP_EAGAIN);
            REQUIRE(byteBuff_pop(&bbuff, 1) == EZP_EARG);
            REQUIRE(byteBuff_push(&bbuff, 3) == EZP_OK);
        }
        WHEN("has 2 elems"){
            byteBuff_push(&bbuff, 5);
            byteBuff_push(&bbuff, 10);

            THEN("peek works"){
                REQUIRE(byteBuff_peek(&bbuff, 0, &b) == EZP_OK);
                REQUIRE(b == 5);
                REQUIRE(byteBuff_peek(&bbuff, 1, &b) == EZP_OK);
                REQUIRE(b == 10);
                REQUIRE(byteBuff_peek(&bbuff, 2, &b) == EZP_EAGAIN);
            }

            THEN("not empty"){
                REQUIRE_FALSE(byteBuff_isFull(&bbuff));
                REQUIRE_FALSE(byteBuff_isEmpty(&bbuff));
            }

            THEN("size: 2"){
                REQUIRE(byteBuff_size(&bbuff) == 2);
            }

            THEN("pop leaves 2nd"){
                REQUIRE(byteBuff_pop(&bbuff, 1) == EZP_OK);
                REQUIRE(byteBuff_peek(&bbuff, 0, &b) == EZP_OK);
                REQUIRE(b == 10);
            }

        }

        WHEN("full"){
            for( uint8_t i=0; i< sizeof(_data_for_buff)-1; i++){
                REQUIRE(byteBuff_push(&bbuff, 2*i) == EZP_OK);
            }
            THEN("acts full"){
                REQUIRE(byteBuff_push(&bbuff, 3) == EZP_EAGAIN);
                REQUIRE(byteBuff_isFull(&bbuff));
                REQUIRE_FALSE(byteBuff_isEmpty(&bbuff));
                REQUIRE(byteBuff_size(&bbuff) == sizeof(_data_for_buff)-1);
            }
            THEN("peeks work"){
                for( uint8_t i=0; i< sizeof(_data_for_buff)-1; i++){
                    REQUIRE(byteBuff_peek(&bbuff,  i, &b) == EZP_OK);
                    REQUIRE(b == 2*i);
                }
            }
            THEN("can pop wrap around"){
                REQUIRE(byteBuff_pop(&bbuff, 16) == EZP_OK);
                REQUIRE(byteBuff_size(&bbuff) == sizeof(_data_for_buff)-1-16);
                for( uint8_t i=sizeof(_data_for_buff)-1; i<sizeof(_data_for_buff)-1+16; i++){
                    REQUIRE(byteBuff_push(&bbuff, 2*i) == EZP_OK);
                }
                for(uint8_t i=0; i<sizeof(_data_for_buff)-1; i++){
                    REQUIRE(byteBuff_peek(&bbuff, i, &b) == EZP_OK);
                    REQUIRE(b == (i+16)*2);
                }
            }
        }

    }

}

