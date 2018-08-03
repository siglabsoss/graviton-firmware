/*
 * lm5331.cpp
 *
 *  Created on: Jul 16, 2018
 *      Author: karthik
 */

#include "lm5531.h"
#include <math.h>

int32_t lm5331_conv_mv_db_out(uint32_t in_mv, uint32_t out_mv)
{
	double a, b;

	a = out_mv;
	b = 10.544 * log(a) - 36.988; // dBm

	return (int32_t)b;
}

int32_t lm5331_conv_mv_db_in(uint32_t in_mv, uint32_t out_mv)
{
	double a, b;

	a = in_mv;
	b = 10.544 * log(a) - 36.988; // dBm

	return (int32_t)b;
}


