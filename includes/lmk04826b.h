/*
 * lmk04826b.h
 *
 *  Created on: Jul 18, 2018
 *      Author: karthik
 */

#ifndef INCLUDES_LMK04826B_H_
#define INCLUDES_LMK04826B_H_

#include "mbed.h"

class LMK04826B
{
	SPI *spi;
	DigitalOut *cs;
	DigitalOut *rst;

public:
	LMK04826B(SPI *bus, PinName select, PinName rst_pin);

	void init();
};




#endif /* INCLUDES_LMK04826B_H_ */
