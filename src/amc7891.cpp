/*
 * amc7891.cpp
 *
 *  Created on: Jul 15, 2018
 *      Author: karthik
 */


#include "amc7891.h"

void AMC7891::write_dac(DAC_CHANNEL channel, uint16_t val)
{
	write_reg(AMC_DAC0_DATA + (0x0F & (int)channel), 0x03FF & val);
}

void AMC7891::enable_dacs()
{
	write_reg(AMC_AMC_POWER, 0x7E00);
}

AMC7891::AMC7891(SPI *bus, PinName select)
{
	spi = bus;
	cs = new DigitalOut(select,1);
}

void AMC7891::init()
{
	// reset AFE controller
	write_reg(AMC_AMC_RESET, 0x6600);

	// enable temperature sensor
	write_reg(AMC_TEMP_CONFIG, 0x0008);

	// set temperature conversion time (15ms)
	write_reg(AMC_TEMP_RATE, 0x0007);

	// Enable ADC and internal reference
	write_reg(AMC_AMC_POWER, 0x6000);

	// Enable all ADCs
	write_reg(AMC_ADC_ENABLE, 0x6DE0);

	// set ADC gain
	write_reg(AMC_ADC_GAIN, 0x0000);

	// Trigger the ADC
	write_reg(AMC_AMC_CONFIG, 0x1300);
}


void AMC7891::set_gpio(uint16_t val)
{
	write_reg(AMC_GPIO_OUT, 0x0FFF & val);
}

void AMC7891::config_gpio(uint16_t gpios, uint16_t init)
{
	// set GPIOs
	set_gpio(init);

	// set all GPIO to output
	write_reg(AMC_GPIO_CONFIG, 0x0FFF & gpios);
}

void AMC7891::config_adc(uint8_t val)
{
	write_reg(AMC_ADC_GAIN, 0xFF00 & (((uint16_t)val)<<8));
}

uint16_t AMC7891::read_adc(ADC_CHANNEL channel)
{
	write_reg(AMC_AMC_CONFIG, 0x1300);
	wait_us(1000);
	return read_reg(AMC_ADC0_DATA + (0x0F & (int)channel));
}


void AMC7891::write_reg(unsigned char addr, uint16_t val)
{
	uint8_t out[3];

	out[0] = addr;
	out[1] = val >> 8;
	out[2] = val & 0xFF;

	wait_us(1);

	cs->write(0);

	spi->write((const char*)out, 3, 0, 0);

	cs->write(1);
}


uint16_t AMC7891::read_reg(unsigned char addr)
{
	uint8_t in[3] = {0};
	uint8_t out[3];

	in[0] = 0x80 | addr;

	wait_us(1);

	cs->write(0);

	spi->write((const char*)in, 3, 0, 0);

	cs->write(1);

	wait_us(1);

	in[0] = 0x80 | AMC_AMC_ID;

	cs->write(0);

	spi->write((const char*)in, 3, (char*)out, 3);

	cs->write(1);

	return (out[1] << 8) | (out[2]);
}

uint16_t AMC7891::read_temperature()
{
	return read_reg(AMC_TEMP_DATA) / 8;
}

