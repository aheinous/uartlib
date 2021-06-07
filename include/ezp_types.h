#pragma once



#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>


typedef uint8_t ezp_bool_t;

#define EZP_TRUE (1)
#define EZP_FALSE (0)

/**
@brief Return values.
*/
typedef enum {
	/** @brief Success */
	EZP_OK = 0,
	/** @brief Failure, bad argument */
	EZP_EARG = -1,
	/** @brief Failure, not ready, try again */
	EZP_EAGAIN = -2,
	/** @brief Failure, bad configuration */
	EZP_ECONFIG = -3,
	/** @brief Failure, incompatible protocol version. */
	EZP_EVERSION = -4,
	/** @brief Failure, invalid message. */
	EZP_EINVALID = -5,
	/** @brief Failure, unknown message type. */
	EZP_EUNKNOWN = -6
} EZP_RESULT;


#ifdef EZP_DEBUG

static inline const char * ezp_res_str(EZP_RESULT res){
	switch(res){
		case EZP_OK: return "EZP_OK";
		case EZP_EARG: return "EZP_EARG";
		case EZP_EAGAIN: return "EZP_EAGAIN";
		case EZP_ECONFIG: return "EZP_ECONFIG";
		case EZP_EVERSION: return "EZP_EVERSION";
		case EZP_EINVALID: return "EZP_EINVALID";
		case EZP_EUNKNOWN: return "EZP_EUNKNOWN";
		default: return "(error unknown EZP_RESULT)";
	}
}
#endif




#if defined(__cplusplus)
}
#endif