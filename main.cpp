#include "mbed.h"
 
#include "amc7891.h"
#include "graviton.h"


Serial pc(PA_2, NC);

Serial fpga(PA_9, PA_10);
DigitalOut buzzer(PA_0);
DigitalOut osc_en(PA_11);
SPI spi_bus(PB_5, PB_4, PB_3);

AMC7891 afe_a(&spi_bus, PA_15);
AMC7891 afe_b(&spi_bus, PA_12);
AMC7891 afe_0(&spi_bus, PB_0);

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








void init()
{
	uint8_t ret;

	buzzer = 0;       // buzzer doesn't actually work
	osc_en = 0;       // not yet ready to setup oscillator

	dac_cs = 1;       // DAC Chip Select
	adc_cs = 1;       // ADC Chip Select
	clk_cs = 1;       // Clock Distribution IC Chip Select
	synth_cs = 1;     // Clock Synthesizer IC Chip Select

	dac_rst = 0;      // Active Low DAC Reset (hold in reset)
	adc_rst = 0;      // Active Low ADC Reset (hold in reset)
	synth_rst = 0;    // Active Low Synthesizer Reset (hold in reset)

	// Set serial port baud
	pc.baud(115200);

	pc.printf("LOG: attempting to change to external oscillator\r\n");

	osc_en = 1;       // Turn on External Oscillator

	wait_ms(10);      // Wait for it to stabilize

	// Change internal clock source to external oscillator
	ret = switch_to_ext_osc();

	// Set serial port baud again (it got messed up when we changed the clock)
	pc.baud(115200);

	if( 1 == ret )
		pc.printf("LOG: external oscillator configured successfully\r\n");
	else
	{
		pc.printf("ERROR: EXTERNAL OSCILLATOR FAILED TO CONFIGURE. HALTING\r\n");
		while(1) { }
	}

	// Set SPI bus format
	spi_bus.format(8,1);
	spi_bus.frequency(18000000);

	afe_a.init();
	afe_b.init();
	afe_0.init();

	// Configure main AFE
	afe_0.config_gpio(GRAV_MAIN_ENABLES, GRAV_MAIN_DEFAULTS);

	// Configure RF AFE
	afe_a.config_gpio(GRAV_CH_ENABLES, GRAV_CH_DEFAULTS);
	afe_b.config_gpio(GRAV_CH_ENABLES, GRAV_CH_DEFAULTS);

	// Wait for Copper Suicide to come up
	wait_ms(500);

	if( (get_v2v5(&afe_0) < 100) &&
		(get_v1v8(&afe_0) < 100) ) {
		afe_0.set_gpio(GRAV_MAIN_POWER_ON_TX_OFF);
	}
	else {
		pc.printf("ERROR: BUS IS POWERED. HALTING\r\n");
		while(1) { }
	}

	// Wait for power supplies to stabilize
	wait_ms(100);
	make_afe_dac_safe(&afe_a);
	make_afe_dac_safe(&afe_b);

	pc.printf("Ready!\n\r");
}




int main()
{
	init();

	uint16_t temp_ch_b, temp_ch_a, temp_main, val;

	pc.printf("\033[2J");
	while(1)
	{

		temp_ch_b = afe_b.read_reg(AMC_TEMP_DATA) / 8;
		temp_ch_a = afe_a.read_reg(AMC_TEMP_DATA) / 8;
		temp_main = afe_0.read_reg(AMC_TEMP_DATA) / 8;


		pc.printf("\033[0;0H");
		pc.printf("V2V5 = %u\r\n", get_v2v5(&afe_0));
		pc.printf("V1V8 = %u\r\n", get_v1v8(&afe_0));
		pc.printf("V3V8 = %u\r\n", get_v3v8(&afe_0));
		pc.printf("V5V5N_V = %u\r\n", get_v5v5n(&afe_0));
		pc.printf("V5V5_V = %u\r\n", get_v5v5(&afe_0));
		pc.printf("V29_V = %u\r\n", get_v29(&afe_0));
		pc.printf("ADC_TEMP = %i\r\n", get_adc_temp(&afe_0));

		pc.printf("AFE Temperature   A: %i   B: %i   MAIN: %i\n\r", temp_ch_a, temp_ch_b, temp_main);

		wait(1);
	}

}


