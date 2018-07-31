#include "mbed.h"
 
#include "amc7891.h"
#include "graviton.h"
#include "lmk04826b.h"
#include "lmk04133.h"
#include "ads42lb69.h"
#include "dac3484.h"


Serial pc(PA_2, NC);
Serial fpga(PA_9, PA_10);

InterruptIn interrupt(PA_8);

DigitalOut buzzer(PA_0);
DigitalOut osc_en(PA_11);

SPI spi_bus(PB_5, PB_4, PB_3);

AMC7891 afe_a(&spi_bus, PA_15);
AMC7891 afe_b(&spi_bus, PA_12);
AMC7891 afe_0(&spi_bus, PB_0);

LMK04826B lmk04826b(&spi_bus, PA_4, PA_3);
LMK04133 lmk04133(&spi_bus, PA_5, PA_7);
ADS42LB69 ads42lb69(&spi_bus, PB_1, PA_6);
DAC3484 dac3484(&spi_bus, PB_6, PB_7);

int interrupt_detected = false;


void dump_telemetry(Serial *out)
{
	//out->printf("\033[0;0H");
	out->printf("LOG: telemetry dump\r\n");
	out->printf("V2V5          = %i\r\n", V2V5);
	out->printf("V1V8          = %i\r\n", V1V8);
	out->printf("V3V8          = %i\r\n", V3V8);
	out->printf("V5V5          = %i\r\n", V5V5);
	out->printf("V5V5N         = %i\r\n", V5V5N);
	out->printf("V29           = %i\r\n", V29);
	out->printf("ADC_TEMP      = %i\r\n", ADC_LM20);
	out->printf("AFE_CH_A_TEMP = %i\r\n", AFE_CH_A_TEMP);
	out->printf("AFE_CH_B_TEMP = %i\r\n", AFE_CH_B_TEMP);
	out->printf("AFE_MAIN_TEMP = %i\r\n", AFE_MAIN_TEMP);
	out->printf("P_OUT_A       = %i\r\n", P_OUT_A);
	out->printf("I_PA_A        = %i\r\n", I_PA_A);
	out->printf("T_PRE_RX_A    = %i\r\n", T_PRE_RX_A);
	out->printf("T_LDO_A       = %i\r\n", T_LDO_A);
	out->printf("P_IN_A        = %i\r\n", P_IN_A);
	out->printf("T_PA_A        = %i\r\n", T_PA_A);
	out->printf("T_PRE_TX2_A   = %i\r\n", T_PRE_TX2_A);
	out->printf("T_PRE_TX3_A   = %i\r\n", T_PRE_TX3_A);
	out->printf("P_OUT_B       = %i\r\n", P_OUT_B);
	out->printf("I_PA_B        = %i\r\n", I_PA_B);
	out->printf("T_PRE_RX_B    = %i\r\n", T_PRE_RX_B);
	out->printf("T_LDO_B       = %i\r\n", T_LDO_B);
	out->printf("P_IN_B        = %i\r\n", P_IN_B);
	out->printf("T_PA_B        = %i\r\n", T_PA_B);
	out->printf("T_PRE_TX2_B   = %i\r\n", T_PRE_TX2_B);
	out->printf("T_PRE_TX3_B   = %i\r\n", T_PRE_TX3_B);
	out->printf("DAC_TEMPDATA  = %i\r\n", dac3484.get_temp());
}


void go_safe_and_reset(const char* reason)
{
	switch_to_safe(&afe_a);
	switch_to_safe(&afe_b);

	// switch to internal clock source before mux'ing external clock sources
	switch_to_int_osc();

	// mux external clock source to external MCU oscillator
	osc_en = 1;

	// wait to stabilize
	wait_ms(10);

	// switch back to external clock source.
	switch_to_ext_osc();

	// shutdown everything else
	configure_grav_safe(&afe_0);

	pc.printf("ERROR: POWERED DOWN. %s\r\n", reason);

	dump_telemetry(&pc);

	pc.printf("LOG: halted. resetting in 5 seconds.\r\n");

	wait_ms(5000);

	NVIC_SystemReset();
}


/*
 * Run a specific safety check and return the next safety check in the list.
 */
uint16_t safety_check(uint16_t test_item)
{
	int32_t value;

	switch( test_item )
	{
	case 0:
		value = V2V5;
		if( (2400 > value) || (2600 < value) ) go_safe_and_reset("2.5V rail");
		break;
	case 1:
		value = V1V8;
		if( (1700 > value) || (1900 < value) ) go_safe_and_reset("1.8V rail");
		break;
	case 2:
		value = V3V8;
		if( (3550 > value) || (4000 < value) ) go_safe_and_reset("3.8V rail");
		break;
	case 3:
		value = V5V5;
		if( (5250 > value) || (5750 < value) ) go_safe_and_reset("5.5V rail");
		break;
	case 4:
		value = V5V5N;
		if( (-6100 /*telemetry is wrong so this value is wrong to compensagte*/ > value) ||
			(-5350 < value) )
		        go_safe_and_reset("negative 5.5V rail");
		break;
	case 5:
		value = V29;
		if( (28750 > value) || (29250 < value) ) go_safe_and_reset("29V rail");
		break;
	case 6:
		if( 50 < AFE_CH_A_TEMP ) go_safe_and_reset("AFE #A overheated");
		break;
	case 7:
		if( 50 < AFE_CH_B_TEMP ) go_safe_and_reset("AFE #B overheated");
		break;
	case 8:
		if( 50 < AFE_MAIN_TEMP ) go_safe_and_reset("Main AFE overheated");
		break;
	case 9:
		if( 70 < ADC_LM20 ) go_safe_and_reset("ADC overheated");
		break;
	case 10:
		if( 70 < T_PRE_RX_A ) go_safe_and_reset("RX preamp #A overheated");
		break;
	case 11:
		if( 70 < T_LDO_A ) go_safe_and_reset("LDO #A overheated");
		break;
	case 12:
		if( 90 < T_PA_A ) go_safe_and_reset("PA #A overheated");
		break;
	case 13:
		if( 70 < T_PRE_TX2_A ) go_safe_and_reset("TX preamp 2 #A overheated");
		break;
	case 14:
		if( 70 < T_PRE_TX3_A ) go_safe_and_reset("TX preamp 3 #A overheated");
		break;
	default:
		return 0;
	}

	return ++test_item;
}


/*
 * Safety check all items
 */
void safety_check()
{
	uint16_t item = 0;
	while( 0 < safety_check(item++) ) { }
}


/*
 * Calibrate gate voltage on power amplifier
 */
void calibrate_power_amplifier(AMC7891 *afe)
{
	uint32_t counts;
	uint32_t v29_bus;

	v29_bus = V29;

	pc.printf("LOG: Calibrating power amplifier\r\n");

	// Set VGG to -3800mV as a starting point (never seems to be lower than this)
	counts = conv_mv_to_dac_counts_for_vgg_circuit( -3800 /*mV*/ );
	afe->write_dac(GRAV_DAC_VGSET, counts);

	// Enable those scary Power Amplfiers
	afe->write_dac(GRAV_DAC_PA_EN, 0x3FF);

	// Bring it down until shoot-thru current is 100mA

	while( 100 /*mA*/ > conv_mv_i_pa(get_mv(afe, GRAV_ADC_I_PA, GAIN_1X)) )
	{
		afe->write_dac(GRAV_DAC_VGSET, --counts);
		wait_ms(1);
		// If the input rail drops by more than 100mV then something is wrong
		if( (v29_bus - 100 /*mV*/)  > V29 ) go_safe_and_reset("V29 drop during calibration");
		pc.printf(".");
		wait_ms(25); // no reason to do this fast
		if( conv_dac_counts_to_mv_for_vgg_circuit(counts) > -2000 /*mV*/ )
		{
			counts = conv_mv_to_dac_counts_for_vgg_circuit( -3800 /*mV*/ );
			pc.printf("\r\nfailed to calibrate power amplifier (Vg went above -2000mV)\r\n");
			break;
		}
	}
	counts += 2;
	afe->write_dac(GRAV_DAC_VGSET, counts);
	pc.printf("\r\nLOG: Done with power amplifier(Vgg = %imV, I = %imA)\r\n",
			conv_dac_counts_to_mv_for_vgg_circuit(counts),
			conv_mv_i_pa(get_mv(afe, GRAV_ADC_I_PA, GAIN_1X)));

	afe->vg_setpoint = counts;
}


void init()
{
	uint8_t ret;

	buzzer = 0;       // buzzer doesn't actually work
	osc_en = 0;       // not yet ready to setup oscillator

	wait_ms(100);      // Wait for power supplies to come up

	// Set serial port baud
	pc.baud(115200);

	pc.printf("\033[2J\033[1;1H");

	pc.printf("LOG: attempting to change to external oscillator\r\n");

	osc_en = 1;       // Turn on External Oscillator

	wait_ms(500);      // Wait for it to stabilize

	// Change internal clock source to external oscillator
	ret = switch_to_ext_osc();

	// Set serial port baud again (it got messed up when we changed the clock)
	pc.baud(115200);
	fpga.baud(115200);

	if( 1 == ret )
		pc.printf("LOG: external oscillator configured\r\n");
	else
	{
		pc.printf("ERROR: EXTERNAL OSCILLATOR FAILED TO CONFIGURE. HALTING\r\n");
		while(1) { }
	}

	// Set SPI bus format super slow to ensure proper
	// programming of ADC, DAC, CLOCK, and SYNTH
	spi_bus.format(8,1);
	spi_bus.frequency(500000);

	// Initialize all AFE chips
	afe_a.init();
	afe_b.init();
	afe_0.init();

	// Configure main AFE
	configure_grav_safe(&afe_0);

	// Configure RF AFE
	switch_to_safe(&afe_a);
	switch_to_safe(&afe_b);

	// Wait for Copper Suicide to come up
	wait_ms(500);

	// If I/O busses are unpowered then power them
	if( (V2V5 < 100) && (V1V8 < 100) )
		configure_grav_on_with_tx_off(&afe_0);
	else {
		pc.printf("ERROR: BUS IS POWERED. HALTING\r\n");
		while(1) { }
	}

	// Wait for power supplies to stabilize
	wait_ms(100);

	pc.printf("LOG: checking everything...\r\n");

	safety_check();

	dump_telemetry(&pc);

	// bring up clocks
	lmk04826b.init();
	lmk04133.init();
	ads42lb69.init();
	dac3484.init();

	// once ADC, DAC, CLOCK, and SYNTH programmed then increase SPI bus speed
	spi_bus.frequency(18000000);

	// calibrating power amplifiers

	safety_check();

	calibrate_power_amplifier(&afe_a);
	calibrate_power_amplifier(&afe_b);

	safety_check();

	// Configure RF AFE
	switch_to_safe(&afe_a);
	switch_to_safe(&afe_b);

	dump_telemetry(&pc);

	pc.printf("LOG: Ready!\n\r");
}


void handleInterruptHigh() {
	interrupt_detected = true;
}


/*
 * Change channel state
 *
 * OP Codes
 *
 * 0b x000 0000 : (CS_OP_CODE_0) go into safe mode
 * 0b x000 0001 : (CS_OP_CODE_1) dump telemetry to FPGA
 * 0b x000 0010 : (CS_OP_CODE_2) dump telemetry to debug port
 * 0b x000 0011 : (CS_OP_CODE_3) enable transmit
 * 0b x000 0100 : (CS_OP_CODE_4) do nothing
 *        .
 *        .
 *        .
 * 0b x011 1111 : (CS_OP_CODE_63) do nothing
 *
 * 0b x1yy yyyy : enable receive with gain = 0byy yyyy
 *     |
 *     |
 *      \
 *        ----- RX FLAG
 */
void changeState(AMC7891 *afe, uint8_t state)
{
	if( CS_RX == (CS_RX_FLAG_MASK & state) )
		switch_to_rx(afe, 0x3F & state);
	else
	{
		switch( CS_OP_CODE_MASK & state )
		{
		case CS_OP_CODE_0:
			switch_to_safe(afe);
			break;
		case CS_OP_CODE_1:
			dump_telemetry(&fpga);
			break;
		case CS_OP_CODE_2:
			dump_telemetry(&pc);
			break;
		case CS_OP_CODE_3:
			switch_to_tx(afe);
			break;
		case CS_OP_CODE_4:
		case CS_OP_CODE_5:
		case CS_OP_CODE_6:
		case CS_OP_CODE_7:
		case CS_OP_CODE_8:
		case CS_OP_CODE_9:
		case CS_OP_CODE_10:
		case CS_OP_CODE_11:
		case CS_OP_CODE_12:
		case CS_OP_CODE_13:
		case CS_OP_CODE_14:
		case CS_OP_CODE_15:
		case CS_OP_CODE_16:
		case CS_OP_CODE_17:
		case CS_OP_CODE_18:
		case CS_OP_CODE_19:
			dac3484.set_current(state - CS_OP_CODE_4);
			break;
		default:
			break;
		}
	}
}

int main()
{
	uint8_t msg;
	uint8_t ch_a_state = 0;
	uint8_t ch_b_state = 0;
	uint16_t check = 0;             // safety checklist item number

	// initialize entire board
	init();

	// handle interrupt event from Copper Suicide PCB
	interrupt.rise(&handleInterruptHigh);

	// flush FPGA buffer
	while( fpga.readable() ) msg = fpga.getc();

	while(1)
	{
		if( fpga.readable() )
		{
			msg = fpga.getc();

			if( CS_AFE_A == (CS_AFE_MASK & msg) )
				ch_a_state = 0x7F & msg;
			else
				ch_b_state = 0x7F & msg;
		}

		if( interrupt_detected )
		{
			changeState(&afe_a, ch_a_state);
			changeState(&afe_b, ch_b_state);

			// if either one of the channels is set to transmit then enable DAC output
			if( (AMC7891_MODE_TX == afe_a.mode) | (AMC7891_MODE_TX == afe_b.mode) )
				configure_grav_on_with_tx_on(&afe_0);
			else
				configure_grav_on_with_tx_off(&afe_0);

			ch_a_state = CS_OP_CODE_63; // NEXT TIME DO NOTHING
			ch_b_state = CS_OP_CODE_63; // NEXT TIME DO NOTHING

			interrupt_detected = false;
		}

		check = safety_check(check);
	}
}


