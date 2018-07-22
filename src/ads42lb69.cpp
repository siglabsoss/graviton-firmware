/*
 * ads42lb69.cpp
 *
 *  Created on: Jul 19, 2018
 *      Author: joel
 */

#include "ads42lb69.h"
#include <string.h>

uint8_t ads42lb69_rom[] =
{
		0x06,0x80, // reg6  - clock divider bypass
		0x08,0x08, // reg8  - disable CTRLx pins
		0x15,0x01, // reg15 - DDR LVDS output mode
		0x20,0x01, // reg20 - CTRL1 and CTRL2 function as overrange indicators for channels A & B respectively
		0x0F,0x00  // reg0f - CHA & CHB test patterns (see data sheet)
};

ADS42LB69::ADS42LB69(SPI *bus, PinName select, PinName rst_pin)
{
	spi = bus;
	cs = new DigitalOut(select,1);
	rst = new DigitalOut(rst_pin, 1);
}

extern Serial pc;

void ADS42LB69::read_back()
{
	unsigned i;
	unsigned vector[2];

	memcpy(vector, ads42lb69_rom, 10);

	spi->format(8,0);

	for(i = 0; i < 32; i++)
	{
		wait_us(1);

		cs->write(0);

		vector[0] = i | 0x80;
		vector[1] = 0;

		spi->write((const char*)vector, 2, 0, 0);

		wait_us(1);

		cs->write(1);
	}



	spi->format(8,1);
}

void ADS42LB69::init()
{
	unsigned i;

	spi->format(8,0);

	wait_ms(1);

	rst->write(0);

	wait_ms(1);

	rst->write(1);

	wait_ms(1);

	rst->write(0);

	wait_ms(1);

	for( i = 0; i < sizeof(ads42lb69_rom) / 2; i++)
	{
		wait_us(1);

		cs->write(0);

		spi->write((const char*)(ads42lb69_rom + 2 * i), 2, 0, 0);

		wait_us(1);

		cs->write(1);
	}

	spi->format(8,1);
}


