#include "ezp_msg.h"
#include "ezp_byte_buffer.h"
#include "ezp_util.h"

#include "ezp_msg_sender.h"
#include "ezp_msg_reader.h"

#ifdef EZP_DEBUG
#include<inttypes.h>
#include <string.h>
#endif


#define SIZEOF_SEQNUM 1
#define SIZEOF_TYPEID 1



// -----------------------------------------------------------------------------------------
// ---------------------  X Macro Msg Table Function Definitions ---------------------------
// -----------------------------------------------------------------------------------------



// size of function ---------------------------------------------


#define START_MSG(msgName) 			case (ezp_msgID_ ## msgName): *size = 0
#define FIELD(type, fieldName) 		+ sizeof(type)
#define END_MSG(msgName) 			; break;

static EZP_RESULT sizeOfMsgPayload(EZP_MSG_TYPE_ID typeID, int *size) {
    EZP_ASSERT(size != NULL);

    switch(typeID) {
        #include EZP_MSG_TABLE

        case ezp_msgID_ack:
            *size = 0;
            break;

        default:
            return EZP_EINVALID;
    }
    return EZP_OK;
}

#undef START_MSG
#undef FIELD
#undef END_MSG


EZP_RESULT sizeOfWholeMsg(EZP_MSG_TYPE_ID typeID, int *size) {
    EZP_ASSERT(size != NULL);

    EZP_RESULT res = sizeOfMsgPayload(typeID, size);
    *size += SIZEOF_SEQNUM + SIZEOF_TYPEID + EZP_SIZEOF_CSUM;

    return res;
}

// Equals Funtion ------------------------------------------------


#if defined(EZP_DEBUG) && defined(__GNUC__)

#define START_MSG(msgName) 		            \
    case (ezp_msgID_ ## msgName):           \
        return 0 == memcmp(&left->msgName, &right->msgName, sz);

#define FIELD(type, fieldName)
#define END_MSG(msgName)

ezp_bool_t ezp_msg_equal(ezp_msg_t *left, ezp_msg_t *right){
    EZP_ASSERT(left);
    EZP_ASSERT(right);
    if(left->typeID != right->typeID){
        return EZP_FALSE;
    }

    int sz;
    sizeOfMsgPayload(left->typeID, &sz);

    switch(left->typeID){
        #include EZP_MSG_TABLE
        default:
            EZP_ASSERT(0);
            return EZP_FALSE;
    }
}

#undef START_MSG
#undef FIELD
#undef END_MSG

#endif

// Print Funtion ------------------------------------------------

#ifdef EZP_DEBUG


static inline void print_uint8_t(const char *desc, uint8_t v) {
    EZP_LOG("%s %" PRIu8 "\n", desc, v);
}

static inline void print_int8_t(const char *desc, int8_t v) {
    EZP_LOG("%s %" PRId8 "\n", desc, v);
}

static inline void print_char(const char *desc, char v) {
    EZP_LOG("%s %c\n", desc, v);
}

static inline void print_uint16_t(const char *desc, uint16_t v) {
    EZP_LOG("%s %" PRIu16 "\n", desc, v);
}

static inline void print_int16_t(const char *desc, int16_t v) {
    EZP_LOG("%s %" PRId16 "\n", desc, v);
}

static inline void print_uint32_t(const char *desc, uint32_t v) {
    EZP_LOG("%s %" PRIu32 "\n", desc, v);
}

static inline void print_int32_t(const char *desc, int32_t v) {
    EZP_LOG("%s %" PRId32 "\n", desc, v);
}

static inline void print_uint64_t(const char *desc, uint64_t v) {
    EZP_LOG("%s %" PRIu64 "\n", desc, v);
}

static inline void print_int64_t(const char *desc, int64_t v) {
    EZP_LOG("%s %" PRId64 "\n", desc, v);
}




#define START_MSG(msgName) 		case (ezp_msgID_ ## msgName):{ \
									ezp_ ## msgName ## _t *innerMsg = &(msg->msgName);\
                                    (void)innerMsg; /*possibly unused*/ \
									EZP_LOG("A " #msgName " Msg:\n" );
#define FIELD(type, fieldName) 		print_ ## type("\t" #fieldName, innerMsg->fieldName);
#define END_MSG(msgName) 		} break;


void ezp_printMsg(ezp_msg_t *msg) {
    switch(msg->typeID) {

        #include EZP_MSG_TABLE

        case (ezp_msgID_ack):
            EZP_LOG("An Ack msg:\n");
            break;

        default:
            EZP_ELOG("Unexpected msg type ID: %d\n", msg->typeID);
            return;
    }

    print_uint8_t("\t_seqNum:", msg->_seqNum);
}


#undef START_MSG
#undef FIELD
#undef END_MSG

#endif


// -----------------------------------------------------------------------------------------
// --------------------- End X Macro Msg Table Function Definitions ------------------------
// -----------------------------------------------------------------------------------------

