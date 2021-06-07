#include "ezp_msg_sender.h"
#include "ezp_types.h"
#include "ezp_platform.h"
#include "ezp_util.h"


static EZP_RESULT msgSender_serialize(msgSender_t *self, ezp_msg_t *msg);
static EZP_RESULT msgSender_send_csum(msgSender_t *self);

void msgSender_init(msgSender_t *self, ezp_platform_t platform) {
    csumCalc_init(&self->m_csum);
    self->m_platform = platform;
}

EZP_RESULT msgSender_send(msgSender_t *self, ezp_msg_t *msg){
    csumCalc_init(&self->m_csum);
    if(msgSender_serialize(self, msg) == EZP_EAGAIN) {
        return EZP_EAGAIN;
    }

    if(msgSender_send_csum(self) == EZP_EAGAIN) {
        return EZP_EAGAIN;
    }
    return self->m_platform.flush();
}

static EZP_RESULT msgSender_send_uint8_t(msgSender_t *self, uint8_t data) {
    if(self->m_platform.write_byte(data) == EZP_OK){
        csumCalc_update(&self->m_csum, data);
        return EZP_OK;
    }
    return EZP_EAGAIN;
}

static EZP_RESULT msgSender_send_csum(msgSender_t *self) {
    uint8_t low;
    uint8_t high;

    csumCalc_getCsum(&self->m_csum, &low, &high);

    if(self->m_platform.write_byte(low) != EZP_OK) {
        return EZP_EAGAIN;
    }
    if(self->m_platform.write_byte(high) != EZP_OK) {
        return EZP_EAGAIN;
    }
    return EZP_OK;
}


// Send/Serialize Funtion ---------------------------------------------

#define START_MSG(msgName) 		case (ezp_msgID_ ## msgName):{ \
									ezp_ ## msgName ## _t *innerMsg = &(msg->msgName);
#define FIELD(type, fieldName) 		if(msgSender_send_ ## type( self, innerMsg->fieldName ) != EZP_OK) { return EZP_EAGAIN; }
#define END_MSG(msgName) 		} break;


static EZP_RESULT msgSender_serialize(msgSender_t *self,ezp_msg_t *msg) {


    // msgSender_t self;

    // initMsgSender(&self);

    if(msgSender_send_uint8_t(self, msg->typeID) != EZP_OK) {
        return EZP_EAGAIN;
    }
    if(msgSender_send_uint8_t(self, msg->_seqNum) != EZP_OK) {
        return EZP_EAGAIN;
    }


    switch(msg->typeID) {

        #include EZP_MSG_TABLE

        case ezp_msgID_ack:
            // no payload
            break;

        default:
            EZP_ELOG("Unexpected msg type ID: %d\n", msg->typeID);
            //return EZP_EARG;
            EZP_ASSERT(0);
    }


    return EZP_OK;
    // // TODO rename finish send to have "object" name
    // if(finish_send(&self) == EZP_OK){
    // 	return EZP_OK;
    // }
    // return EZP_EAGAIN;
}