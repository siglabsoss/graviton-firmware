/*
 * dac3484.h
 *
 *  Created on: Jul 19, 2018
 *      Author: joel
 */

#ifndef INCLUDES_DAC3484_H_
#define INCLUDES_DAC3484_H_

#include "mbed.h"

class DAC3484
{
	SPI *spi;
	DigitalOut *cs;
	DigitalOut *rst;

public:
	DAC3484(SPI *bus, PinName select, PinName rst_pin);

	void init(RawSerial* serial);

	void set_current(uint8_t val);
	int8_t get_temp();
	int8_t data_pattern_checker();
};




#endif /* INCLUDES_DAC3484_H_ */
