
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "ezp_types.h"






// -----------------------------------------------------------------------------------------
// ------------------------- X Macro Msg Table Definitions ---------------------------------
// -----------------------------------------------------------------------------------------

//  This section uses the long lost x-macros technique.
//	https://en.wikipedia.org/wiki/X_Macro



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

typedef struct {
    // EZP_MSG_TYPE_ID typeID;// TODO switch back
    uint8_t typeID;
    uint8_t _seqNum; // for internal use only
    union {
        #include EZP_MSG_TABLE
    };
} ezp_msg_t;

#undef START_MSG
#undef END_MSG
#undef FIELD

// -----------------------------------------------------------------------------------------
// ------------------------- End X Macro Msg Table Definitions -----------------------------
// -----------------------------------------------------------------------------------------


EZP_RESULT sizeOfWholeMsg(EZP_MSG_TYPE_ID typeID, int *size);


#ifdef EZP_DEBUG
    void ezp_printMsg(ezp_msg_t *msg);
#else
    #define ezp_printMsg(m) ((void)sizeof(m))
#endif


#define EZP_ROUND_UP_POW2(x) \
            (   ((x) <= 1) ?  1 : \
                ((x) <= 2) ?  2 : \
                ((x) <= 4) ?  4 : \
                ((x) <= 8) ?  8 : \
                ((x) <= 16) ?  16 : \
                ((x) <= 32) ?  32 : \
                ((x) <= 64) ?  64 : \
                ((x) <= 128) ?  128 :  256)

#define EZP_RECV_BUFFER_MIN_SIZE EZP_ROUND_UP_POW2(sizeof(ezp_msg_t) + EZP_SIZEOF_CSUM)
#define EZP_RECV_BUFFER_RECOMENDED_SIZE EZP_ROUND_UP_POW2( 2* (sizeof(ezp_msg_t) + EZP_SIZEOF_CSUM) )


#if defined(__cplusplus)
}
#endif