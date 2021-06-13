#include "catch.hpp"

extern "C" {
    #include "ezp_csum.h"
}



SCENARIO("csum"){
    uint8_t simple;
    uint8_t extra;
    GIVEN("initted"){
        csum_calc_t csum;
        csumCalc_init(&csum);

        THEN("everything 0"){
            csumCalc_getCsum(&csum, &simple, &extra);
            REQUIRE(simple == 0);
            REQUIRE(extra == 0);
        }


        GIVEN("add test data"){
            csumCalc_update(&csum, 'a');
            csumCalc_update(&csum, 'b');
            csumCalc_update(&csum, 'c');
            csumCalc_update(&csum, 'd');
            csumCalc_update(&csum, 'e');


            THEN("get exprected result"){
                csumCalc_getCsum(&csum, &simple, &extra);

                REQUIRE((int)simple == 211);
                REQUIRE((int)extra == 109);

            }
        }
    }
}