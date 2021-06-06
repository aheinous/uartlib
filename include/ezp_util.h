#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef EZP_DEBUG
    #include <stdio.h>
    #include <stdlib.h>

	#ifndef EZP_VLOG
    #define EZP_VLOG(...) 	/*printf(__VA_ARGS__)*/
    #endif

	#ifndef EZP_LOG
    #define EZP_LOG(...)	printf(__VA_ARGS__)
    #endif

	#ifndef EZP_WLOG
    #define EZP_WLOG(...)	printf("EZP Warning: " __VA_ARGS__ )
    #endif

	#ifndef EZP_ELOG
    #define EZP_ELOG(...)	printf("EZP Error: " __VA_ARGS__ )
    #endif

    #define EZP_ASSERT(cond) \
	do { if( !(cond) ){ \
		printf("Assertion failure: %s:%d\n", __FILE__,__LINE__);\
		exit(1);\
	} } while(0)


    #define EZP_VLOG_INT(expn) EZP_VLOG( #expn ": %d\n", (int) expn)
    #define EZP_LOG_INT(expn) EZP_LOG( #expn ": %d\n", (int) expn)
    #define EZP_ELOG_INT(expn) EZP_ELOG( #expn ": %d\n", (int) expn)
    #define EZP_WLOG_INT(expn) EZP_WLOG( #expn ": %d\n", (int) expn)


    //void print_uint8_t(const char *, uint8_t);

#else

#define EZP_VLOG(...)
#define EZP_LOG(...)
#define EZP_WLOG(...)
#define EZP_ELOG(...)

#define EZP_VLOG_INT(expn)
#define EZP_LOG_INT(expn)
#define EZP_ELOG_INT(expn)
#define EZP_WLOG_INT(expn)



#define EZP_ASSERT(cond) ((void)sizeof(cond))
#define print_uint8_t(a,b)




#endif


#define EZP_CHECK_OK(expn) do{ \
	EZP_RESULT res = (expn); \
	EZP_ASSERT(res == EZP_OK); \
} while(0)


#if defined(__cplusplus)
}
#endif
