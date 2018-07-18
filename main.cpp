#include "mbed.h"
 
#include "amc7891.h"
#include "graviton.h"
#include "lmk04826b.h"


Serial pc(PA_2, NC);
Serial fpga(PA_9, PA_10);

DigitalOut buzzer(PA_0);
DigitalOut osc_en(PA_11);
SPI spi_bus(PB_5, PB_4, PB_3);

AMC7891 afe_a(&spi_bus, PA_15);
AMC7891 afe_b(&spi_bus, PA_12);
AMC7891 afe_0(&spi_bus, PB_0);

LMK04826B lmk04826b(&spi_bus, PA_4);

DigitalOut dac_cs(PB_6);
DigitalOut adc_cs(PB_1);
DigitalOut synth_cs(PA_5);

DigitalOut dac_rst(PB_7);
DigitalOut adc_rst(PA_6);
DigitalOut synth_rst(PA_7);


void dump_telemetry()
{
	//pc.printf("\033[0;0H");
	pc.printf("LOG: telemetry dump\r\n");
	pc.printf("V2V5          = %i\r\n", V2V5);
	pc.printf("V1V8          = %i\r\n", V1V8);
	pc.printf("V3V8          = %i\r\n", V3V8);
	pc.printf("V5V5          = %i\r\n", V5V5);
	pc.printf("V5V5N         = %i\r\n", V5V5N);
	pc.printf("V29           = %i\r\n", V29);
	pc.printf("ADC_TEMP      = %i\r\n", ADC_LM20);
	pc.printf("AFE_CH_A_TEMP = %i\r\n", AFE_CH_A_TEMP);
	pc.printf("AFE_CH_B_TEMP = %i\r\n", AFE_CH_B_TEMP);
	pc.printf("AFE_MAIN_TEMP = %i\r\n", AFE_MAIN_TEMP);
	pc.printf("P_OUT_A       = %i\r\n", P_OUT_A);
	pc.printf("I_PA_A        = %i\r\n", I_PA_A);
	pc.printf("T_PRE_RX_A    = %i\r\n", T_PRE_RX_A);
	pc.printf("T_LDO_A       = %i\r\n", T_LDO_A);
	pc.printf("P_IN_A        = %i\r\n", P_IN_A);
	pc.printf("T_PA_A        = %i\r\n", T_PA_A);
	pc.printf("T_PRE_TX2_A   = %i\r\n", T_PRE_TX2_A);
	pc.printf("T_PRE_TX3_A   = %i\r\n", T_PRE_TX3_A);
	pc.printf("P_OUT_B       = %i\r\n", P_OUT_B);
	pc.printf("I_PA_B        = %i\r\n", I_PA_B);
	pc.printf("T_PRE_RX_B    = %i\r\n", T_PRE_RX_B);
	pc.printf("T_LDO_B       = %i\r\n", T_LDO_B);
	pc.printf("P_IN_B        = %i\r\n", P_IN_B);
	pc.printf("T_PA_B        = %i\r\n", T_PA_B);
	pc.printf("T_PRE_TX2_B   = %i\r\n", T_PRE_TX2_B);
	pc.printf("T_PRE_TX3_B   = %i\r\n", T_PRE_TX3_B);
}


void go_safe_and_shutdown(const char* reason)
{
	// Set RF AFE DACs to safe (big deal since they control power amplifiers)
	make_afe_dac_safe(&afe_a);
	make_afe_dac_safe(&afe_b);

	// Configure RF AFE
	afe_a.config_gpio(GRAV_AFE_ENABLES, GRAV_AFE_SAFE);
	afe_b.config_gpio(GRAV_AFE_ENABLES, GRAV_AFE_SAFE);

	// switch to internal clock before switching external clock sources
	switch_to_int_osc();

	osc_en = 1;       // Turn on External Oscillator

	wait_ms(10);

	switch_to_ext_osc();

	// Configure main AFE
	afe_0.config_gpio(GRAV_MAIN_ENABLES, GRAV_MAIN_DEFAULTS);


	pc.printf("ERROR: POWERED DOWN. %s\r\n", reason);

	dump_telemetry();

	pc.printf("LOG: halted\r\n");

	while(1) { }
}


void check_busses()
{
	int32_t value;

	value = V2V5;
	if( (2400 > value) || (2600 < value) ) go_safe_and_shutdown("2.5V rail");

	value = V1V8;
	if( (1700 > value) || (1900 < value) ) go_safe_and_shutdown("1.8V rail");

	value = V3V8;
	if( (3600 > value) || (4000 < value) ) go_safe_and_shutdown("3.8V rail");

	value = V5V5;
	if( (5350 > value) || (5650 < value) ) go_safe_and_shutdown("5.5V rail");

	value = V5V5N;
	if( (-5650 > value) || (-5350 < value) ) go_safe_and_shutdown("negative 5.5V rail");

	value = V29;
	if( (28750 > value) || (29250 < value) ) go_safe_and_shutdown("29V rail");

	if( 50 < AFE_CH_A_TEMP ) go_safe_and_shutdown("AFE #A overheated");
	if( 50 < AFE_CH_B_TEMP ) go_safe_and_shutdown("AFE #B overheated");
	if( 50 < AFE_MAIN_TEMP ) go_safe_and_shutdown("Main AFE overheated");
	if( 70 < ADC_LM20 ) go_safe_and_shutdown("ADC overheated");
	if( 70 < T_PRE_RX_A ) go_safe_and_shutdown("RX preamp #A overheated");
	if( 70 < T_LDO_A ) go_safe_and_shutdown("LDO #A overheated");
	if( 90 < T_PA_A ) go_safe_and_shutdown("PA #A overheated");
	if( 70 < T_PRE_TX2_A ) go_safe_and_shutdown("TX preamp 2 #A overheated");
	if( 70 < T_PRE_TX3_A ) go_safe_and_shutdown("TX preamp 3 #A overheated");
}


void calibrate_power_amplifier()
{
	uint32_t counts;
	uint32_t v29_bus;

	v29_bus = V29;

	/*
	 * Channel A
	 */
	pc.printf("LOG: Calibrating power amplifier A\r\n");

	// Set VGG to -5000mV
	counts = conv_mv_to_dac_counts_for_vgg_circuit( -5000 /*mV*/ );
	afe_a.write_dac(GRAV_DAC_VGSET, counts);

	// Enable those scary Power Amplfiers
	afe_a.write_dac(GRAV_DAC_PA_EN, 0x3FF);

	// Bring it down until shoot-thru current is 100mA

	while( 100 /*mA*/ > I_PA_A )
	{
		afe_a.write_dac(GRAV_DAC_VGSET, --counts);
		// If the input rail drops by more than 100mV then something is wrong
		if( (v29_bus - 100 /*mV*/)  > V29 ) go_safe_and_shutdown("V29 drop during calibration");
		wait_us(500);
		if( (v29_bus - 100 /*mV*/)  > V29 ) go_safe_and_shutdown("V29 drop during calibration");
		pc.printf(".");
		wait_ms(100); // no reason to do this fast
	}
	counts += 2;
	afe_a.write_dac(GRAV_DAC_VGSET, counts);
	pc.printf("\r\nLOG: Done with power amplifier A (Vgg = %imV, I = %imA)\r\n",
			conv_dac_counts_to_mv_for_vgg_circuit(counts),
			I_PA_A);


	/*
	 * Channel B
	 */
	pc.printf("LOG: Calibrating power amplifier B\r\n");

	// Set VGG to -5000mV
	counts = conv_mv_to_dac_counts_for_vgg_circuit( -5000 /*mV*/ );
	afe_b.write_dac(GRAV_DAC_VGSET, counts);

	// Enable those scary Power Amplfiers
	afe_b.write_dac(GRAV_DAC_PA_EN, 0x3FF);

	// Bring it down until shoot-thru current is 100mA

	while( 100 /*mA*/ > I_PA_B )
	{
		afe_b.write_dac(GRAV_DAC_VGSET, --counts);
		// If the input rail drops by more than 100mV then something is wrong
		if( (v29_bus - 100 /*mV*/)  > V29 ) go_safe_and_shutdown("V29 drop during calibration");
		wait_us(500);
		if( (v29_bus - 100 /*mV*/)  > V29 ) go_safe_and_shutdown("V29 drop during calibration");
		pc.printf(".");
		wait_ms(100); // no reason to do this fast
	}
	counts += 2;
	afe_b.write_dac(GRAV_DAC_VGSET, counts);
	pc.printf("\r\nLOG: Done with power amplifier B (Vgg = %imV, I = %imA)\r\n",
			conv_dac_counts_to_mv_for_vgg_circuit(counts),
			I_PA_B);
}


void init()
{
	uint8_t ret;

	buzzer = 0;       // buzzer doesn't actually work
	osc_en = 0;       // not yet ready to setup oscillator

	dac_cs = 1;       // DAC Chip Select
	adc_cs = 1;       // ADC Chip Select
	synth_cs = 1;     // Clock Synthesizer IC Chip Select

	dac_rst = 0;      // Active Low DAC Reset (hold in reset)
	adc_rst = 0;      // Active Low ADC Reset (hold in reset)
	synth_rst = 0;    // Active Low Synthesizer Reset (hold in reset)

	// Set serial port baud
	pc.baud(115200);

	pc.printf("\033[2J\033[1;1H");

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
	spi_bus.frequency(2000000);

	afe_a.init();
	afe_b.init();
	afe_0.init();

	// Configure main AFE
	afe_0.config_gpio(GRAV_MAIN_ENABLES, GRAV_MAIN_DEFAULTS);
	afe_0.config_adc(0x20);

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

	pc.printf("LOG: checking everything...\r\n");

	check_busses();

	dump_telemetry();


	// bring up clocks
	pc.printf("LOG: initializing LMK04826B clock jitter cleaner\r\n");
	lmk04826b.init();
	pc.printf("LOG: finished initializing LMK04826B\r\n");


	// calibrating power amplifiers

	calibrate_power_amplifier();

	pc.printf("LOG: checking everything again...\r\n");

	check_busses();

	dump_telemetry();

	pc.printf("LOG: Ready!\n\r");


	// TODO: configure clock distribution chip


	// TODO: configure synthesizer (gives us 950MHz for Local Oscillator)


	// TODO: configure ADC


	// TODO: configure DAC


	// calibrate the power amplifiers

	// set to lowest possible gain



	// we probably want a function call for CS/S-MODEM to calibrate TX chain
	// when its readdy to to so. This'll allow TX to opperate at max power.
	// before this just set the DAC current (i.e. DAC gain) to minimum for
	// safe operation.


	// LOOP HERE
	// IF MESSAGE_TX then start transition squence to TX
	// IF MESSAGE_RX when start transition sequence to RX
	// GOOD TIME TO CHECK TEHMPERATURES AND VOLTAGES
	// IF MESSAGE FROM ETH TO CALIBRATE DAC then calibrate DAC
	// IF MESSAGE FROM ETH TO SET DSA then set DSA
	// }

}




int main()
{
	init();

	while(1)
	{

	}

}


