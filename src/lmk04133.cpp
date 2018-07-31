/*
 * lmk04133.cpp
 *
 *  Created on: Jul 18, 2018
 *      Author: joel
 */


#include "lmk04133.h"

uint8_t _lmk04133_rom[] = {
		0x00,0x00,0x00,0x17,
		0x01,0x01,0x01,0x00,
		0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x02,
		0x01,0x01,0x01,0x03,
		0x01,0x01,0x01,0x04,
		0x00,0x00,0x00,0x07,
		0x01,0x50,0x00,0x0A,
		0x00,0x65,0x01,0x6B,
		0xE0,0x78,0x07,0x8C,
		0x0A,0x04,0x40,0x0D,
		0x0F,0xA3,0x00,0x5E,
		0x1C,0x80,0x02,0x6F };



LMK04133::LMK04133(SPI *bus, PinName select, PinName rst_pin)
{
	spi = bus;
	rst = new DigitalOut(rst_pin,0);
	cs = new DigitalOut(select,1);
}

void LMK04133::init()
{
	unsigned i;

	spi->format(8,0);

	wait_ms(1);

	rst->write(1);

	wait_ms(1);

	for( i = 0; i < sizeof(_lmk04133_rom) / 4; i++)
	{
		wait_us(1);

		cs->write(0);

		wait_us(1);

		spi->write((const char*)(_lmk04133_rom + 4 * i), 4, 0, 0);

		wait_us(1);

		cs->write(1);
	}


	spi->format(8,1);
}

