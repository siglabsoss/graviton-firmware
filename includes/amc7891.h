/*
 * amc7891.h
 *
 *  Created on: Jul 15, 2018
 *      Author: karthik
 */

#ifndef INCLUDES_AMC7891_H_
#define INCLUDES_AMC7891_H_

#include "mbed.h"

// AMC7891 Register Map
#define AMC_TEMP_DATA 0x00
#define AMC_TEMP_CONFIG 0x0A
#define AMC_TEMP_RATE 0x0B
#define AMC_ADC0_DATA 0x23
#define AMC_ADC1_DATA 0x24
#define AMC_ADC2_DATA 0x25
#define AMC_ADC3_DATA 0x26
#define AMC_ADC4_DATA 0x27
#define AMC_ADC5_DATA 0x28
#define AMC_ADC6_DATA 0x29
#define AMC_ADC7_DATA 0x2A
#define AMC_DAC0_DATA 0x2B
#define AMC_DAC1_DATA 0x2C
#define AMC_DAC2_DATA 0x2D
#define AMC_DAC3_DATA 0x2E
#define AMC_DAC0_CLEAR 0x2F
#define AMC_DAC1_CLEAR 0x30
#define AMC_DAC2_CLEAR 0x31
#define AMC_DAC3_CLEAR 0x32
#define AMC_GPIO_CONFIG 0x33
#define AMC_GPIO_OUT 0x34
#define AMC_GPIO_IN 0x35
#define AMC_AMC_CONFIG 0x36
#define AMC_ADC_ENABLE 0x37
#define AMC_ADC_GAIN 0x38
#define AMC_DAC_CLEAR 0x39
#define AMC_DAC_SYNC 0x3A
#define AMC_AMC_POWER 0x3B
#define AMC_AMC_RESET 0x3E
#define AMC_AMC_ID 0x40

enum ADC_CHANNEL
{
	AMC_ADC0 = 0,
	AMC_ADC1,
	AMC_ADC2,
	AMC_ADC3,
	AMC_ADC4,
	AMC_ADC5,
	AMC_ADC6,
	AMC_ADC7
};

enum DAC_CHANNEL
{
	AMC_DAC0 = 0,
	AMC_DAC1,
	AMC_DAC2,
	AMC_DAC3
};

class AMC7891
{
	SPI *spi;
	DigitalOut *cs;

	uint16_t read_reg(unsigned char addr);
	void write_reg(unsigned char addr, uint16_t val);

public:
	AMC7891(SPI *bus, PinName select);

	void init();

	void enable_dacs();
	void write_dac(DAC_CHANNEL channel, uint16_t val);

	void config_gpio(uint16_t gpios, uint16_t init);
	void set_gpio(uint16_t val);

	void config_adc(uint8_t val);
	uint16_t read_adc(ADC_CHANNEL channel);

	uint16_t read_temperature();

public:
	uint32_t gain;
};






#endif /* INCLUDES_AMC7891_H_ */
