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
@ingroup group_comm
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



// -----------------------------------------------------------------------------------------
// ------------------------- X Macro Msg Table Definitions ---------------------------------
// -----------------------------------------------------------------------------------------

//  This section uses the long lost x-macros technique.
//	https://en.wikipedia.org/wiki/X_Macro


#if 0
#ifndef EZP_MSG_TABLE
#define EZP_MSG_TABLE \
	START_MSG(foo) \
	FIELD(uint8_t, a) \
	FIELD(uint8_t, b) \
	END_MSG(foo) \
	\
	START_MSG(bar) \
	FIELD(uint8_t, c) \
	FIELD(uint8_t, d) \
	END_MSG(bar) \
	\
	START_MSG(ping) \
	FIELD(uint8_t, val) \
	END_MSG(ping) \
	\
	START_MSG(pong) \
	FIELD(uint8_t, val) \
	END_MSG(pong) \
	\
	START_MSG(volume) \
	FIELD(uint8_t, value) \
	END_MSG(volume)
#endif


#endif
// Enum definition -------------------------------------------------

#define START_MSG(msgName) 	ezp_msgID_ ## msgName,
#define FIELD(type, fieldName)
#define END_MSG(msgName)

typedef enum {

	ezp_msgID_ack, // ACK special case

	#include EZP_MSG_TABLE
} EZP_MSG_TYPE_ID;

#undef START_MSG
#undef FIELD
#undef END_MSG


// Struct definitions ---------------------------------------------
#define START_MSG(msgName) 	typedef struct {
#define FIELD(type, fieldName) 		type fieldName;
#define END_MSG(msgName) 	} ezp_ ## msgName ## _t;

#include EZP_MSG_TABLE


#undef START_MSG
#undef FIELD
#undef END_MSG


// Big Conglomerate Struct Union definition -----------------------

#define START_MSG(msgName) ezp_ ## msgName ## _t msgName;
#define FIELD(type, fieldName)
#define END_MSG(msgName)

typedef struct{
	// EZP_MSG_TYPE_ID typeID;// TODO switch back
	uint8_t typeID;
	uint8_t _seqNum; // for internal use only
	union{
		#include EZP_MSG_TABLE
	};
} ezp_msg_t;

#undef START_MSG
#undef END_MSG
#undef FIELD

// -----------------------------------------------------------------------------------------
// ------------------------- End X Macro Msg Table Definitions -----------------------------
// -----------------------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif