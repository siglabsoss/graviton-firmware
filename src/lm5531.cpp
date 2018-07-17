/*
 * lm5331.cpp
 *
 *  Created on: Jul 16, 2018
 *      Author: karthik
 */

#include "lm5531.h"

int32_t lm5331_conv_mv_db_out(uint32_t in_mv, uint32_t out_mv)
{
	// for now just output mv
	return (int32_t)out_mv;
}

int32_t lm5331_conv_mv_db_in(uint32_t in_mv, uint32_t out_mv)
{
	// for now just output mv
	return (int32_t)in_mv;
}


