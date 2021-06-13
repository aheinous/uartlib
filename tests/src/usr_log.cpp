#include "ezp_util.h"

#include <cstdio>
#include <cstdarg>

#ifdef  EZP_DEBUG





extern "C" {
    void ezp_usr_log(ezp_log_level level, const char *fmt, ...){
        if(level >= EZP_LOG_WARNING){
            va_list args;
            va_start(args, fmt);
            vprintf( fmt, args);
            va_end(args);
        }
    }



    void ezp_usr_error(ezp_error_type type){
        if(type == EZP_OTHER){
            printf("ERROR\n");
        }else{
            printf("ASSERT FAIL\n");
        }
    }
}
#endif  //EZP_DEBUG