#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef EZP_DEBUG


    typedef enum{
        EZP_LOG_VERBOSE,
        EZP_LOG_NORMAL,
        EZP_LOG_WARNING,
        EZP_LOG_ERROR,
        EZP_LOG_ASSERT,

    } ezp_log_level;

    typedef enum{
        EZP_OTHER,
        EZP_ASSERTION_FAILURE,
    } ezp_error_type;

    #ifdef EZP_USR_LOG_FUNC
        void ezp_usr_log(ezp_log_level level, const char *fmt, ...);
    #else
        static inline void ezp_usr_log(ezp_log_level level, const char *fmt, ...) { (void) fmt; (void) level; }
    #endif

    #ifdef EZP_USR_ERROR_FUNC
        void ezp_usr_error(ezp_error_type);
    #else
        static inline void ezp_usr_error(ezp_error_type) { }
    #endif


	#ifndef EZP_VLOG
    #define EZP_VLOG(...) 	 ezp_usr_log(EZP_LOG_VERBOSE, "EZP Verb: " __VA_ARGS__)
    #endif

	#ifndef EZP_LOG
    #define EZP_LOG(...)	ezp_usr_log(EZP_LOG_NORMAL,"EZP Log: " __VA_ARGS__)
    #endif

	#ifndef EZP_WLOG
    #define EZP_WLOG(...)	ezp_usr_log(EZP_LOG_WARNING,"EZP Warning: " __VA_ARGS__ )
    #endif

	#ifndef EZP_ELOG
    #define EZP_ELOG(...)	do{ ezp_usr_log(EZP_LOG_ERROR, "EZP Error: " __VA_ARGS__ ); ezp_usr_error(EZP_OTHER);} while(0)
    #endif

    #define EZP_ASSERT(cond) \
	do { if( !(cond) ){ \
		ezp_usr_log(EZP_LOG_ASSERT,"Assertion failure: %s:%d\n", __FILE__,__LINE__);\
        ezp_usr_error(EZP_ASSERTION_FAILURE); \
    }} while(0)


    #define EZP_VLOG_INT(expn) EZP_VLOG( #expn ": %d\n", (int) expn)
    #define EZP_LOG_INT(expn) EZP_LOG( #expn ": %d\n", (int) expn)
    #define EZP_ELOG_INT(expn) EZP_ELOG( #expn ": %d\n", (int) expn)
    #define EZP_WLOG_INT(expn) EZP_WLOG( #expn ": %d\n", (int) expn)

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
    #define ezp_on_error() do{}while(0)
#endif


#define IS_POW2(n) ( (n) && !((n) & (n-1) ))


#define EZP_ROUND_UP_POW2(x) \
        (   ((x) <= 1) ?  1 : \
            ((x) <= 2) ?  2 : \
            ((x) <= 4) ?  4 : \
            ((x) <= 8) ?  8 : \
            ((x) <= 16) ?  16 : \
            ((x) <= 32) ?  32 : \
            ((x) <= 64) ?  64 : \
            ((x) <= 128) ?  128 :  256)

#define countof(arr) (sizeof(arr) / sizeof(arr[0]))

#define EZP_CHECK_OK(expn) do{ \
	EZP_RESULT res = (expn); \
	EZP_ASSERT(res == EZP_OK); \
} while(0)


#if defined(__cplusplus)
}
#endif
