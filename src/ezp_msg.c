#include "ezp_msg.h"
#include "ezp_byte_buffer.h"
#include "ezp_util.h"

#include "ezp_msg_sender.h"
#include "ezp_msg_reader.h"

#define SIZEOF_SEQNUM 1
#define SIZEOF_TYPEID 1



// -----------------------------------------------------------------------------------------
// ---------------------  X Macro Msg Table Function Definitions ---------------------------
// -----------------------------------------------------------------------------------------




// static EZP_RESULT deserialize(ezp_msg_t *msg, msgReader_t *reader);




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
            //EZP_ELOG("sizeOfMsgPayload(): unexpected typeID: %d\n", (int) typeID);
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






// Print Funtion ------------------------------------------------

#ifdef EZP_DEBUG

static void print_uint8_t(const char*desc, uint8_t v){
    EZP_LOG("%s %d\n", desc, (int) v);
}

#define START_MSG(msgName) 		case (ezp_msgID_ ## msgName):{ \
									ezp_ ## msgName ## _t *innerMsg = &(msg->msgName);\
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

