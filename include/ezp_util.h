#pragma once
#if 0
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


#else

#define EZP_VLOG(...)
#define EZP_LOG(...)
#define EZP_WLOG(...)
#define EZP_ELOG(...)

#define EZP_ASSERT(cond) ((void)sizeof(cond))



#endif

#if defined(__cplusplus)
}
#endif
#endif