#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>


typedef uint8_t ezp_bool_t;

#define EZP_TRUE (1)
#define EZP_FALSE (0)


typedef enum {
	EZP_OK = 0,
	EZP_EARG = -1,
	EZP_EAGAIN = -2,
	EZP_EINVALID = -3,

} EZP_RESULT;


#ifdef EZP_DEBUG

static inline const char * ezp_res_str(EZP_RESULT res){
	switch(res){
		case EZP_OK: return "EZP_OK";
		case EZP_EARG: return "EZP_EARG";
		case EZP_EAGAIN: return "EZP_EAGAIN";
		case EZP_EINVALID: return "EZP_EINVALID";
		default: return "(error unknown EZP_RESULT)";
	}
}
#endif


#if defined(__cplusplus)
}
#endif
