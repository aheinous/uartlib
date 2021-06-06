#pragma once

#include <stdint.h>


#define EZP_SIZEOF_CSUM 2

typedef struct
{
	uint8_t m_simple;
	uint8_t m_extra;

} csum_calc_t;


void csumCalc_init(csum_calc_t *self);

void csumCalc_update(csum_calc_t *self, uint8_t byte);

void csumCalc_getCsum(csum_calc_t *self, uint8_t *simple, uint8_t *extra);
