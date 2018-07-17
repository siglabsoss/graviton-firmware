/*
 * lm20.cpp
 *
 *  Created on: Jul 16, 2018
 *      Author: karthik
 */


#include "lm20.h"

uint32_t lm20_conv_mv_degc(uint32_t in)
{
	return -(100*in - 186630)/1169;
}

