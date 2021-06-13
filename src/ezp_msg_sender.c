#include "ezp_msg_sender.h"
#include "ezp_types.h"
#include "ezp_platform.h"
#include "ezp_util.h"


static inline EZP_RESULT msgSender_serialize(msgSender_t *self, ezp_msg_t *msg);
static inline EZP_RESULT msgSender_send_csum(msgSender_t *self);


static inline EZP_RESULT msgSender_send_uint8_t(msgSender_t *self, uint8_t data) {
    if(self->m_platform.write_byte(self->m_platform.usr_data, data) == EZP_OK) {
        csumCalc_update(&self->m_csum, data);
        return EZP_OK;
    }
    return EZP_EAGAIN;
}
#define msgSender_send_char(s,c)   msgSender_send_uint8_t(s, (uint8_t) c)
#define msgSender_send_int8_t(s,c) msgSender_send_uint8_t(s, (uint8_t) c)


static inline EZP_RESULT msgSender_send_uint16_t(msgSender_t *self, uint16_t data) {

    for(int8_t i=0; i<2; i++){
        if(self->m_platform.write_byte(self->m_platform.usr_data, data & 0xFF) != EZP_OK) {
            return EZP_EAGAIN;
        }
        csumCalc_update(&self->m_csum, data & 0xFF);
        data >>= 8;
    }
    return EZP_OK;
}
#define msgSender_send_int16_t(s,c) msgSender_send_uint16_t(s, (uint16_t) c)


static inline EZP_RESULT msgSender_send_uint32_t(msgSender_t *self, uint32_t data) {

    for(int8_t i=0; i<4; i++){
        if(self->m_platform.write_byte(self->m_platform.usr_data, data & 0xFF) != EZP_OK) {
            return EZP_EAGAIN;
        }
        csumCalc_update(&self->m_csum, data & 0xFF);
        data >>= 8;
    }
    return EZP_OK;
}
#define msgSender_send_int32_t(s,c) msgSender_send_uint32_t(s, (uint32_t) c)


static inline EZP_RESULT msgSender_send_uint64_t(msgSender_t *self, uint64_t data) {

    for(int8_t i=0; i<8; i++){
        if(self->m_platform.write_byte(self->m_platform.usr_data, data & 0xFF) != EZP_OK) {
            return EZP_EAGAIN;
        }
        csumCalc_update(&self->m_csum, data & 0xFF);
        data >>= 8;
    }
    return EZP_OK;
}
#define msgSender_send_int64_t(s,c) msgSender_send_uint64_t(s, (uint64_t) c)




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
    return self->m_platform.flush(self->m_platform.usr_data);
}



static inline EZP_RESULT msgSender_send_csum(msgSender_t *self) {
    uint8_t low;
    uint8_t high;

    csumCalc_getCsum(&self->m_csum, &low, &high);

    if(self->m_platform.write_byte(self->m_platform.usr_data, low) != EZP_OK) {
        return EZP_EAGAIN;
    }
    if(self->m_platform.write_byte(self->m_platform.usr_data, high) != EZP_OK) {
        return EZP_EAGAIN;
    }
    return EZP_OK;
}


// Send/Serialize Funtion ---------------------------------------------

#define START_MSG(msgName) 		case (ezp_msgID_ ## msgName):{ \
									ezp_ ## msgName ## _t *innerMsg = &(msg->msgName);\
                                    (void)innerMsg; /* might be unused */
#define FIELD(type, fieldName) 		if(msgSender_send_ ## type( self, innerMsg->fieldName ) != EZP_OK) { return EZP_EAGAIN; }
#define END_MSG(msgName) 		} break;


static inline EZP_RESULT msgSender_serialize(msgSender_t *self,ezp_msg_t *msg) {

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
            EZP_ASSERT(0);
            return EZP_EARG;
    }


    return EZP_OK;
}