#include "ezp_msg_reader.h"
#include "ezp_csum.h"
#include "ezp_util.h"
#include "ezp_msg.h"


static EZP_RESULT msgReader_deserialize(msgReader_t *self, ezp_msg_t *msg);
static EZP_RESULT msgReader_try_one_read(msgReader_t *self, ezp_msg_t *msg);


static EZP_RESULT msgReader_read_uint8_t(msgReader_t * self, uint8_t * b){
    EZP_CHECK_OK(byteBuff_peek(&self->byteBuff, self->pos, b));
    ++(self->pos);
    return EZP_OK;
}


static EZP_RESULT msgReader_checkChecksum(msgReader_t *self, int size) {
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


void msgReader_init(msgReader_t *self){
    byteBuff_init(&self->byteBuff);
    self->pos = 0;
}


EZP_RESULT msgReader_read_msg(msgReader_t *self, ezp_msg_t *msg){
    EZP_ASSERT(msg);

    while(1){
        // EZP_VLOG(">> master_tryReadMsg\n");
        EZP_RESULT res = msgReader_try_one_read(self, msg);
        // EZP_VLOG("<< master_tryReadMsg\n");

        if(res == EZP_EINVALID) {
            byteBuff_pop(&self->byteBuff, 1);
        }
        else if(res == EZP_OK) {
            int msgSize;
            EZP_CHECK_OK(sizeOfWholeMsg(msg->typeID, &msgSize));
            byteBuff_pop(&self->byteBuff, 1);
            return EZP_OK;
        }
        else {
            return EZP_EAGAIN;
        }
    }
}

// static EZP_RESULT master_actualRecv(ezp_master_t *self, ezp_msg_t *msg) {
//     if(msg == NULL) {
//         return EZP_EARG;
//     }


//     while(1) {

//         // EZP_VLOG(">> master_tryReadMsg\n");
//         EZP_RESULT res = master_tryReadMsg(self, msg);
//         // EZP_VLOG("<< master_tryReadMsg\n");

//         if(res == EZP_EINVALID) {
//             // EZP_VLOG("res == EZP_EINVALID\n");
//             _ezp_buffer_advance(1);
//         }
//         else if(res == EZP_OK) {
//             // EZP_VLOG("res == EZP_OK\n");
//             // 1+sizeOfMsgPayload(msg->typeID)+1
//             int msgSize;
//             EZP_CHECK_OK(sizeOfWholeMsg(msg->typeID, &msgSize));
//             _ezp_buffer_advance(msgSize);
//             return EZP_OK;
//         }
//         else {
//             // EZP_VLOG("res == EZP_EAGAIN\n");
//             _ezp_buffer_reset();
//             return EZP_EAGAIN;
//         }
//     }

// }



EZP_RESULT msgReader_push_byte(msgReader_t *self, uint8_t b){
    return byteBuff_push(&self->byteBuff, b);
}


static EZP_RESULT msgReader_try_one_read(msgReader_t *self, ezp_msg_t *msg){
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

// EZP_RESULT master_tryReadMsg(ezp_master_t *msgr, ezp_msg_t *msg) {
//     uint8_t buff[32]; // TODO
//     int buffIndex = 0;

//     _ezp_buffer_reset();

//     if(_ezp_buffer_getc(&buff[0]) != EZP_OK) return EZP_EAGAIN;

//     //int totalSize = 1 + sizeOfMsgPayload(buff[0]) + 1;
//     int totalSize;

//     if(sizeOfWholeMsg(buff[0], &totalSize) != EZP_OK) {
//         // invalid type ID
//         return EZP_EINVALID;
//     }

//     EZP_ASSERT(totalSize <= sizeof(buff));

//     for(buffIndex = 1; buffIndex < totalSize; buffIndex++) {
//         if(_ezp_buffer_getc(&buff[buffIndex]) != EZP_OK) return EZP_EAGAIN;
//     }


//     if(checkChecksum(buff, totalSize) == EZP_EINVALID) {
//         EZP_LOG("Checksum failure.\n");
//         return EZP_EINVALID;
//     }

//     msgReader_t self;
//     initMsgReader(&self, buff, totalSize);

//     if(deserialize(msg, &self) == EZP_OK) {
//         return EZP_OK;
//     }
//     EZP_LOG("Deserialization failure.\n");
//     return EZP_EINVALID;
// }








// Deserialize Funtion ---------------------------------------------

#define START_MSG(msgName) 		case (ezp_msgID_ ## msgName):{ \
									ezp_ ## msgName ## _t *innerMsg = &(msg->msgName);
#define FIELD(type, fieldName) 		if(msgReader_read_ ## type( self, &(innerMsg->fieldName) ) != EZP_OK) { return EZP_EAGAIN; }
#define END_MSG(msgName) 		} break;

static EZP_RESULT msgReader_deserialize(msgReader_t *self, ezp_msg_t *msg) {
    // EZP_VLOG("reading typeID\n");

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