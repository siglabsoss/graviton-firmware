/*
 * ads42lb69.h
 *
 *  Created on: Jul 19, 2018
 *      Author: joel
 */

#ifndef INCLUDES_ADS42LB69_H_
#define INCLUDES_ADS42LB69_H_

#include "mbed.h"

class ADS42LB69
{
	SPI *spi;
	DigitalOut *cs;
	DigitalOut *rst;

public:
	ADS42LB69(SPI *bus, PinName select, PinName rst_pin);

	void init();

	void read_back();
};



#endif /* INCLUDES_ADS42LB69_H_ */
