// Fletcher-8 (called Fletcher-16 on wikipedia)

#include "ezp_csum.h"

void csumCalc_init(csum_calc_t *self){
	self->m_simple = 0;
	self->m_extra = 0;
}

void csumCalc_update(csum_calc_t *self, uint8_t byte){
	byte ^= 0xA3; // stop 0x00 from being treated like 0xFF
	self->m_simple = ((uint16_t)self->m_simple + byte) % 255;
	self->m_extra = ((uint16_t)self->m_extra + self->m_simple) % 255;

}

void csumCalc_getCsum(csum_calc_t *self, uint8_t *simple, uint8_t *extra){
    *simple = self->m_simple;
    *extra = self->m_extra;
}
