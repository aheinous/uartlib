#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "ezp_types.h"
#include "ezp_msg.h"



// EZP_RESULT ezp_platform_read_byte(uint8_t*);

// should either block (which will cause process to block) or buffer
// the bytes to avoid having to return EZP_EAGAIN
EZP_RESULT ezp_platform_write_byte(uint8_t);
EZP_RESULT ezp_platform_flush();

EZP_RESULT ezp_platform_on_recv_msg(ezp_msg_t*);







// /**
// @brief Platform implementation to read a single character.

// The implementation may block, in which case all calls to ezp_recv will
// block until a full and correct message has been received.  The implementation
// may also return EZP_EAGAIN if no data is available, in which case ezp_recv
// will return EZP_EAGAIN until a full message is available and has been received.
// **/
// extern EZP_RESULT ezp_platform_getc(uint8_t* byte);



// /**

// @brief Platform implementation to write a single character.

// The implementation may block, in which case ezp_send will block until the
// message has been fully sent.  The implementation may also return EZP_EAGAIN, in
// which case ezp_send will return EZP_EAGAIN and must be called again on the
// same message.  If ezp_recv returns EZP_EAGAIN, the full message must be
// sent again.
// **/
// extern EZP_RESULT ezp_platform_putc(uint8_t byte);



// /**

// @brief Platform implementation to flush the write buffer.

// This will be called once an entire message has been sent via ezp_putc.

// This implementation may be left as a stub if flushing is not necessary
// for this platform.
// **/
// // TODO TODO TODO Call flush
// extern void ezp_platform_flush();

#if defined(__cplusplus)
}
#endif

