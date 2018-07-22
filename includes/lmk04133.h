/*
 * lmk04133.h
 *
 *  Created on: Jul 18, 2018
 *      Author: joel
 */

#ifndef INCLUDES_LMK04133_H_
#define INCLUDES_LMK04133_H_

#include "mbed.h"

class LMK04133
{
	SPI *spi;
	DigitalOut *cs;
	DigitalOut *rst;

public:
	LMK04133(SPI *bus, PinName select, PinName rst_pin);

	void init();
};




#endif /* INCLUDES_LMK04133_H_ */
