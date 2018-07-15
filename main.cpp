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

enum TELEM
{
	AFE_CH_B,
	AFE_CH_A,
	AFE_MAIN
};


Serial pc(PA_2, NC);

Serial fpga(PA_9, PA_10);
DigitalOut buzzer(PA_0);
DigitalOut osc_en(PA_11);
SPI spi_bus(PB_5, PB_4, PB_3);


DigitalOut afe_ch_b_cs(PA_12);
DigitalOut afe_ch_a_cs(PA_15);
DigitalOut afe_main_cs(PB_0);
DigitalOut dac_cs(PB_6);
DigitalOut adc_cs(PB_1);
DigitalOut clk_cs(PA_4);
DigitalOut synth_cs(PA_5);

DigitalOut dac_rst(PB_7);
DigitalOut adc_rst(PA_6);
DigitalOut synth_rst(PA_7);

AnalogIn adc_temp(ADC_TEMP);
AnalogIn adc_batt(ADC_VBAT);
AnalogIn adc_vref1(ADC_VREF1);
AnalogIn adc_vref2(ADC_VREF2);

void delay()
{
	unsigned long a = 1000;

	while(a--) { }
}

void ToggleAFE(TELEM telem, bool state)
{
	switch(telem)
	{
	case AFE_CH_B:
		afe_ch_b_cs = state;
		break;
	case AFE_CH_A:
		afe_ch_a_cs = state;
		break;
	case AFE_MAIN:
		afe_main_cs = state;
		break;
	default:
		break;
	}
}


void WriteAFE(TELEM telem, unsigned char addr, uint16_t val)
{
	uint8_t out[3];

	out[0] = addr;
	out[1] = val >> 8;
	out[2] = val & 0xFF;

	delay();

	ToggleAFE(telem, 0);
	afe_main_cs = 0;

	spi_bus.write((const char*)out, 3, 0, 0);

	ToggleAFE(telem, 1);
}


uint16_t ReadAFE(TELEM telem, unsigned char addr)
{
	uint8_t in[3] = {0};
	uint8_t out[3];

	in[0] = 0x80 | addr;

	delay();

	ToggleAFE(telem, 0);

	spi_bus.write((const char*)in, 3, 0, 0);

	ToggleAFE(telem, 1);

	delay();

	in[0] = 0x80 | AMC_AMC_ID;

	ToggleAFE(telem, 0);

	spi_bus.write((const char*)in, 3, (char*)out, 3);

	ToggleAFE(telem, 1);

	return (out[1] << 8) | (out[2]);
}

void InitAFE(TELEM telem)
{
	// reset AFE controller
	WriteAFE(telem, AMC_AMC_RESET, 0x6600);

	// set all GPIO to output
	WriteAFE(telem, AMC_GPIO_CONFIG, 0x0000);

	// enable temperature sensor
	WriteAFE(telem, AMC_TEMP_CONFIG, 0x0008);

	// set temperature conversion time (15ms)
	WriteAFE(telem, AMC_TEMP_RATE, 0x0007);

	// Enable ADC and internal reference
	WriteAFE(telem, AMC_AMC_POWER, 0x6000);

	 // set GPIOs
	WriteAFE(telem, AMC_GPIO_OUT, 0x0000);
}


void init()
{
	pc.baud(115200);

	buzzer = 0; // buzzer doesn't actually work
	osc_en = 0; // not yet ready to setup oscillator

	afe_ch_b_cs = 1;
	afe_ch_a_cs = 1;
	afe_main_cs = 1;
	dac_cs = 1;
	adc_cs = 1;
	clk_cs = 1;
	synth_cs = 1;

	dac_rst = 0;
	adc_rst = 0;
	synth_rst = 0;

	spi_bus.format(8,1);
	spi_bus.frequency(1000000);

	delay();

	InitAFE(AFE_CH_B);
	InitAFE(AFE_CH_A);
	InitAFE(AFE_MAIN);

	pc.printf("Hello World\n\r");
}




int main()
{
	init();

	uint16_t temp_ch_b, temp_ch_a, temp_main;



	while(1)
	{

		temp_ch_b = ReadAFE(AFE_CH_B, AMC_TEMP_DATA) / 8;
		temp_ch_a = ReadAFE(AFE_CH_A, AMC_TEMP_DATA) / 8;
		temp_main = ReadAFE(AFE_MAIN, AMC_TEMP_DATA) / 8;

	pc.printf("AFE Temperature   A: %i   B: %i   MAIN: %i\n\r", temp_ch_a, temp_ch_b, temp_main);
	}

}
