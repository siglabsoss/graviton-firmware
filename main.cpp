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
	spi_bus.frequency(1000000);

	afe_a.init();
	afe_b.init();
	afe_0.init();

	// Configure main AFE
	afe_0.config_gpio(GRAV_MAIN_ENABLES, GRAV_MAIN_DEFAULTS);

	// Configure RF AFE
	afe_a.config_gpio(GRAV_AFE_ENABLES, GRAV_AFE_SAFE);
	afe_b.config_gpio(GRAV_AFE_ENABLES, GRAV_AFE_SAFE);

	// Wait for Copper Suicide to come up
	wait_ms(500);

	// If I/O busses are unpowered then power them
	if( (V2V5 < 100) && (V1V8 < 100) )
		afe_0.set_gpio(GRAV_MAIN_POWER_ON_TX_OFF);
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

	pc.printf("\033[2J");
	while(1)
	{
		pc.printf("\033[0;0H");
		pc.printf("V2V5 = %i      \r\n", V2V5);
		pc.printf("V1V8 = %i      \r\n", V1V8);
		pc.printf("V3V8 = %i      \r\n", V3V8);
		pc.printf("V5V5 = %i      \r\n", V5V5);
		pc.printf("V5V5N = %i      \r\n", V5V5N);
		pc.printf("V29 = %i      \r\n", V29);
		pc.printf("ADC_TEMP = %i      \r\n", ADC_LM20);
		pc.printf("AFE Temperature   A: %i         B: %i         MAIN: %i      \n\r\n", AFE_CH_A_TEMP, AFE_CH_B_TEMP, AFE_MAIN_TEMP);

		pc.printf("P_OUT_A = %i      \r\n", P_OUT_A);
		pc.printf("I_PA_A = %i      \r\n", I_PA_A);
		pc.printf("T_PRE_RX_A = %i      \r\n", T_PRE_RX_A);
		pc.printf("T_LDO_A = %i      \r\n", T_LDO_A);
		pc.printf("P_IN_A = %i      \r\n", P_IN_A);
		pc.printf("T_PA_A = %i      \r\n", T_PA_A);
		pc.printf("T_PRE_TX2_A = %i      \r\n", T_PRE_TX2_A);
		pc.printf("T_PRE_TX3_A = %i      \r\n\n", T_PRE_TX3_A);

		pc.printf("P_OUT_B = %i      \r\n", P_OUT_B);
		pc.printf("I_PA_B = %i      \r\n", I_PA_B);
		pc.printf("T_PRE_RX_B = %i      \r\n", T_PRE_RX_B);
		pc.printf("T_LDO_B = %i      \r\n", T_LDO_B);
		pc.printf("P_IN_B = %i      \r\n", P_IN_B);
		pc.printf("T_PA_B = %i      \r\n", T_PA_B);
		pc.printf("T_PRE_TX2_B = %i      \r\n", T_PRE_TX2_B);
		pc.printf("T_PRE_TX3_B = %i      \r\n\n", T_PRE_TX3_B);

		wait_ms(100);
	}

}


