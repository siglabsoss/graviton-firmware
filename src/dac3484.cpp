/*
 * dac3484.cpp
 *
 *  Created on: Jul 19, 2018
 *      Author: joel
 */

#include "dac3484.h"

uint8_t _dac3484_rom[] = {
		0x00,0x01,0x9d,
		0x02,0x80,0x82,
		0x07,0xff,0xff,
		0x1f,0x44,0x44,
		0x20,0x11,0x00,
		0x24,0x1c,0x00,
		0x1b,0x08,0x00,
		0x03,0x00,0x01 };

DAC3484::DAC3484(SPI *bus, PinName select, PinName rst_pin)
{
	spi = bus;
	rst = new DigitalOut(rst_pin,0);
	cs = new DigitalOut(select,1);
}

void DAC3484::set_current(uint8_t val)
{
	if( 16 <= val ) // this is invalid so return and don't do anything
	{
		return;
	}

	uint8_t config3[] = {03, 0x00, 0x01};

	spi->format(8,0);

	config3[1] = 0x00 | (val << 4);

	wait_us(1);

	cs->write(0);

	wait_us(1);

	spi->write((const char*)(config3), 3, 0, 0);

	wait_us(1);

	cs->write(1);

	spi->format(8,1);
}

int8_t DAC3484::get_temp()
{
	uint8_t config6[3] = {0x86, 0x00, 0x00};
	uint8_t val[3];

	spi->format(8,0);

	wait_us(1);

	cs->write(0);

	wait_us(1);

	spi->write((const char*)config6, 3, (char*)val, 3);

	wait_us(1);

	cs->write(1);

	spi->format(8,1);

	return (int8_t)val[1];
}

void DAC3484::init()
{
	unsigned i;

	spi->format(8,1);

	wait_ms(1);

	rst->write(1);

	wait_ms(1);

	for( i = 0; i < sizeof(_dac3484_rom) / 3; i++)
	{
		wait_us(1);

		cs->write(0);

		wait_us(1);

		spi->write((const char*)(_dac3484_rom + 3 * i), 3, 0, 0);

		wait_us(1);

		cs->write(1);
	}


	spi->format(8,1);
}





