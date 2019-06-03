/*
 * dac3484.cpp
 *
 *  Created on: Jul 19, 2018
 *      Author: joel
 */

#include "dac3484.h"

uint8_t _dac3484_rom[] = {
		0x00,0x01,0x9d,
		0x01,0x0C,0x00, // default 0x050E  --- even parity and word parity enabled
		0x02,0x80,0x82,
		0x07,0xff,0xE7, // disable mask on bit 4 and 3 (rising parity, falling parity)
		0x1f,0x44,0x44,
		0x20,0x11,0x00,
		0x24,0x1c,0x00,
		0x1b,0x08,0x00,
		0x03,0x40,0x01 }; // [15:12] COARSE_DAC, [0] SIF_TXENABLE (0 = use external TXENABLE pin, 1 = enable DAC output)

#define DAC3484_NUM_REGS 8


DAC3484::DAC3484(SPI *bus, PinName select, PinName rst_pin)
{
	spi = bus;
	rst = new DigitalOut(rst_pin,0);
	cs = new DigitalOut(select,1);
}

void DAC3484::set_current(uint8_t val)
{
	return;
	if( 16 <= val ) // this is invalid so return and don't do anything
	{
		return;
	}

	uint8_t config3[] = {0x03, 0x00, 0x00};

	spi->format(8,0);
	spi->frequency(500000);

	config3[1] = 0x00 | (0xF0 & (val << 4));

	wait_us(1);

	cs->write(0);

	wait_us(1);

	spi->write((const char*)(config3), 3, 0, 0);

	wait_us(1);

	cs->write(1);

	spi->format(8,1);
//	spi->frequency(18000000);
}

int8_t DAC3484::get_temp()
{
	uint8_t config6[3] = {0x86, 0x00, 0x00};
	uint8_t val[3];

	spi->format(8,0);
	spi->frequency(500000);

	wait_us(1);

	cs->write(0);

	wait_us(1);

	spi->write((const char*)config6, 3, (char*)val, 3);

	wait_us(1);

	cs->write(1);

	spi->format(8,1);
//	spi->frequency(18000000);

	return (int8_t)val[1];
}

uint16_t DAC3484::read_alarms() {
	uint8_t config5[3] = {0x85, 0x00, 0x00};
	uint8_t val[3];

	spi->format(8,0);
	spi->frequency(500000);

	wait_us(1);

	cs->write(0);

	wait_us(1);

	spi->write((const char*) config5, 3, (char*) val, 3);

	wait_us(1);

	cs->write(1);
	
	spi->format(8,1);

	return (uint16_t)((val[1] << 8) | val[2]);
}

/*int8_t DAC3484::data_pattern_checker()
{
	// Enable by asserting iotest_ena in register config1
	uint8_t config1[] = {0x01, 0x80, 0x00};
	spi->write((const char*)(config1), 1, 128, 0);

	wait_us(1);

	cs->write(1);

	spi->format(8,1);
	spi->frequency(18000000);

	return 0;
}*/

void DAC3484::init(RawSerial* serial)
{
	unsigned i;

	spi->format(8,1);
	spi->frequency(500000);

	wait_ms(1);

	rst->write(0);

	wait_ms(100);

	rst->write(1);

	wait_ms(100);

	serial->printf("programming DAC over SPI\r\n");

	for( i = 0; i < DAC3484_NUM_REGS; i++)
	{
		wait_ms(1);

		cs->write(0);

		wait_us(1);

		spi->write((const char*)(_dac3484_rom + 3 * i), 3, 0, 0);

		wait_us(1);

		cs->write(1);
	}

	uint8_t read[3];
	uint8_t write[3];

	serial->printf("reading back DAC registers\r\n");

	for( i = 0; i < DAC3484_NUM_REGS; i++)
	{
		write[0] = 0x80 | _dac3484_rom[3*i];
		write[1] = 0x00;
		write[2] = 0x00;

		read[0] = 0x00;
		read[1] = 0x00;
		read[2] = 0x00;

		wait_ms(1);

		cs->write(0);

		wait_us(1);

		spi->write((const char*)write, 3, (char*)read, 3);

		wait_us(1);

		cs->write(1);

		serial->printf("address 0x%02x, wrote 0x%02x 0x%02x, read 0x%02x 0x%02x 0x%02x\r\n",
				_dac3484_rom[3*i+0], _dac3484_rom[3*i+1], _dac3484_rom[3*i+2],
				read[0], read[1], read[2]);
	}


	spi->format(8,1);
	//spi->frequency(18000000);
}





