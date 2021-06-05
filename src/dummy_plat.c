#include "ezp_platform.h"



EZP_RESULT ezp_platform_getc(uint8_t* byte){
 *byte = 0;
 return EZP_OK;
}




EZP_RESULT ezp_platform_putc(uint8_t byte){
    (void)byte;
    return EZP_OK;
}




void ezp_platform_flush(){}

void runTests();

int main(int argc, const char** vargs){
    runTests();
}