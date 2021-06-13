#include "ezp_msg_reader.h"
#include "ezp_csum.h"
#include "ezp_util.h"
#include "ezp_msg.h"


static inline EZP_RESULT msgReader_deserialize(msg_reader_t *self, ezp_msg_t *msg);
static inline EZP_RESULT msgReader_try_one_read(msg_reader_t *self, ezp_msg_t *msg);


static inline EZP_RESULT msgReader_read_uint8_t(msg_reader_t * self, uint8_t * b){
    EZP_CHECK_OK(byteBuff_peek(&self->byteBuff, self->pos, b));
    ++(self->pos);
    return EZP_OK;
}
#define msgReader_read_char(s, d) msgReader_read_uint8_t(s, (uint8_t*)d)
#define msgReader_read_int8_t(s, d) msgReader_read_uint8_t(s, (uint8_t*)d)

static inline EZP_RESULT msgReader_read_uint16_t(msg_reader_t * self, uint16_t * d){
    uint8_t b;
    *d = 0;
    for(int8_t i=0; i<2; i++){
        EZP_CHECK_OK(byteBuff_peek(&self->byteBuff, self->pos, &b));
        *d |= ((uint16_t)b << (i*8));
        ++(self->pos);
    }
    return EZP_OK;
}
#define msgReader_read_int16_t(s, d) msgReader_read_uint16_t(s, (uint16_t*)d)

static inline EZP_RESULT msgReader_read_uint32_t(msg_reader_t * self, uint32_t * d){
    uint8_t b;
    *d = 0;
    for(int8_t i=0; i<4; i++){
        EZP_CHECK_OK(byteBuff_peek(&self->byteBuff, self->pos, &b));
        *d |= ((uint32_t) b << (i * 8));
        ++(self->pos);
    }
    return EZP_OK;
}
#define msgReader_read_int32_t(s, d) msgReader_read_uint32_t(s, (uint32_t*)d)

static inline EZP_RESULT msgReader_read_uint64_t(msg_reader_t * self, uint64_t * d){
    uint8_t b;
    *d = 0;
    for(int8_t i=0; i<8; i++){
        EZP_CHECK_OK(byteBuff_peek(&self->byteBuff, self->pos, &b));
        *d |= ((uint64_t) b << (i * 8));
        ++(self->pos);
    }
    return EZP_OK;
}
#define msgReader_read_int64_t(s, d) msgReader_read_uint64_t(s, (uint64_t*)d)



static inline EZP_RESULT msgReader_checkChecksum(msg_reader_t *self, int size) {
    csum_calc_t calc;
    csumCalc_init(&calc);
    for(int i = 0; i < size - EZP_SIZEOF_CSUM; i++) {
        uint8_t b;
        byteBuff_peek(&self->byteBuff, i, &b);
        csumCalc_update(&calc, b);
    }

    uint8_t low, high;
    uint8_t bufflow, buffhigh;

    csumCalc_getCsum(&calc, &low, &high);
    byteBuff_peek(&self->byteBuff, size - 2, &bufflow);
    byteBuff_peek(&self->byteBuff, size - 1, &buffhigh);

    if( low == bufflow && high == buffhigh ) {
        return EZP_OK;
    }
    else {
        return EZP_EINVALID;
    }
}


EZP_RESULT msgReader_init(msg_reader_t *self, uint8_t *buff, uint8_t len){
    self->pos = 0;
    return byteBuff_init(&self->byteBuff, buff, len);
}

EZP_RESULT msgReader_read_msg(msg_reader_t *self, ezp_msg_t *msg){
    EZP_ASSERT(msg);

    while(1){
        // EZP_VLOG(">> master_tryReadMsg\n");
        EZP_RESULT res = msgReader_try_one_read(self, msg);
        // EZP_VLOG("<< master_tryReadMsg\n");

        if(res == EZP_EINVALID) {
            byteBuff_pop(&self->byteBuff, 1);
        }
        else if(res == EZP_OK) {
            return EZP_OK;
        }
        else {
            return EZP_EAGAIN;
        }
    }
}


EZP_RESULT msgReader_on_msg_valid(msg_reader_t *self, ezp_msg_t *msg){
    int msgSize;
    EZP_CHECK_OK(sizeOfWholeMsg(msg->typeID, &msgSize));
    byteBuff_pop(&self->byteBuff, msgSize);
    return EZP_OK;
}

EZP_RESULT msgReader_on_msg_invalid(msg_reader_t *self){
    byteBuff_pop(&self->byteBuff, 1);
    return EZP_OK;
}


EZP_RESULT msgReader_push_byte(msg_reader_t *self, uint8_t b){
    return byteBuff_push(&self->byteBuff, b);
}


static inline EZP_RESULT msgReader_try_one_read(msg_reader_t *self, ezp_msg_t *msg){
    if(byteBuff_isEmpty(&self->byteBuff)){
        return EZP_EAGAIN;
    }

    uint8_t id;
    byteBuff_peek(&self->byteBuff, 0, &id);

    int totalSize;
    if(sizeOfWholeMsg(id, &totalSize) != EZP_OK) {
        // invalid type ID
        return EZP_EINVALID;
    }

    if(byteBuff_size(&self->byteBuff) < totalSize){
        return EZP_EAGAIN;
    }

    if(msgReader_checkChecksum(self, totalSize) == EZP_EINVALID) {
        EZP_LOG("Checksum failure.\n");
        return EZP_EINVALID;
    }

    return msgReader_deserialize(self, msg);
}



// Deserialize Funtion ---------------------------------------------

#define START_MSG(msgName) 		case (ezp_msgID_ ## msgName):{ \
									ezp_ ## msgName ## _t *innerMsg = &(msg->msgName); \
                                    (void) innerMsg ; /* might be unused */
#define FIELD(type, fieldName) 		if(msgReader_read_ ## type( self, &(innerMsg->fieldName) ) != EZP_OK) { return EZP_EAGAIN; }
#define END_MSG(msgName) 		} break;

static inline EZP_RESULT msgReader_deserialize(msg_reader_t *self, ezp_msg_t *msg) {

    self->pos = 0;
    if(msgReader_read_uint8_t(self, &(msg->typeID)) != EZP_OK) {
        return EZP_EAGAIN;
    }
    if(msgReader_read_uint8_t(self, &(msg->_seqNum)) != EZP_OK) {
        return EZP_EAGAIN;
    }

    switch(msg->typeID) {

        #include EZP_MSG_TABLE

        case ezp_msgID_ack:
            // no payload
            break;

        default:
            EZP_ELOG("Unexpected deser msg type ID: %d\n", msg->typeID);
            return EZP_EINVALID;
    }

    return EZP_OK;
}

#undef START_MSG
#undef FIELD
#undef END_MSG
