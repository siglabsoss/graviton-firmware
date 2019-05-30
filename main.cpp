#include "mbed.h"

#include "BufferedSerial.h"

#include "amc7891.h"
#include "graviton.h"
#include "lmk04826b.h"
#include "lmk04133.h"
#include "ads42lb69.h"
#include "dac3484.h"


RawSerial pc(PA_2, NC);
BufferedSerial fpga(PA_9, PA_10, 8, 2);

DigitalIn interrupt(PA_8);

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


void dump_telemetry(RawSerial *out)
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
/*		if( (2400 > value) || (2600 < value) ) go_safe_and_reset("2.5V rail out of spec");*/
		break;
	case 1:
		value = V1V8;
//		if( (1700 > value) || (1900 < value) ) go_safe_and_reset("1.8V rail out of spec");
		break;
	case 2:
		value = V3V8;
//		if( (3550 > value) || (4000 < value) ) go_safe_and_reset("3.8V rail out of spec");
		break;
	case 3:
		value = V5V5;
//		if( (1 > value) || (5750 < value) ) go_safe_and_reset("5.5V rail out of spec");
		break;
	case 4:
		value = V5V5N;
//		if( (-6100 /*telemetry is wrong so this value is wrong to compensagte*/ > value) ||
//			(-5350 < value) )
//		        go_safe_and_reset("negative 5.5V rail out of spec");
		break;
	case 5:
		value = V29;
//		if( (28750 > value) || (29250 < value) ) go_safe_and_reset("29V rail out of spec");
		break;
	case 6:
		if( 60 < AFE_CH_A_TEMP ) go_safe_and_reset("AFE #A overheated");
		break;
	case 7:
		if( 60 < AFE_CH_B_TEMP ) go_safe_and_reset("AFE #B overheated");
		break;
	case 8:
		if( 60 < AFE_MAIN_TEMP ) go_safe_and_reset("Main AFE overheated");
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
	case 15:
//		if( 33 < P_IN_A ) go_safe_and_reset("PA #A return power > 2 Watts");
		break;


	case 16:
		if( 70 < T_PRE_RX_B ) go_safe_and_reset("RX preamp #B overheated");
		break;
	case 17:
		if( 70 < T_LDO_B ) go_safe_and_reset("LDO #B overheated");
		break;
	case 18:
		if( 90 < T_PA_B ) go_safe_and_reset("PA #B overheated");
		break;
	case 19:
		if( 70 < T_PRE_TX2_B ) go_safe_and_reset("TX preamp 2 #B overheated");
		break;
	case 20:
		if( 70 < T_PRE_TX3_B ) go_safe_and_reset("TX preamp 3 #B overheated");
		break;
	case 21:
//		if( 33 < P_IN_B ) go_safe_and_reset("PA #B return power > 2 Watts");
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

	// Enable that scary Power Amplfier
	afe->write_dac(GRAV_DAC_PA_EN, GRAV_DAC_PA_EN_TX);

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

	// Disable Power Amplfier
	afe->write_dac(GRAV_DAC_PA_EN, GRAV_DAC_PA_EN_RX);

	pc.printf("\r\nLOG: Done with power amplifier(Vgg = %imV, I = %imA)\r\n",
			conv_dac_counts_to_mv_for_vgg_circuit(counts),
			conv_mv_i_pa(get_mv(afe, GRAV_ADC_I_PA, GAIN_1X)));

	afe->vg_setpoint = counts;
	afe->vg_quiescent = conv_mv_to_dac_counts_for_vgg_circuit(conv_dac_counts_to_mv_for_vgg_circuit(counts) - 100 /*mV*/ );
}


void init()
{
	uint8_t ret;

	set_time(0);

	buzzer = 0;       // buzzer doesn't actually work
	osc_en = 0;       // not yet ready to setup oscillator

	wait_ms(100);      // Wait for power supplies to come up

	// Set serial port baud
	pc.baud(115200);

	//pc.printf("\033[2J\033[1;1H");
	pc.printf("\r\n\r\n\r\nGRAVITON HARDWARE VERSION 3.0\r\n");
	pc.printf("FIRMWARE VERSION 0.1\r\n");
	pc.printf("SIGNAL LABORATORIES, INC.\r\n");

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
	if( (V2V5 < 100) && (V1V8 < 100) ) {
		pc.printf("V2V5: %d V1V8: %d\r\n", V2V5, V1V8);
		configure_grav_on_with_tx_off(&afe_0);
	} else {
		pc.printf("V2V5: %d V1V8: %d\r\n", V2V5, V1V8);
		pc.printf("ERROR: BUS IS POWERED. HALTING\r\n");
		//while(1) { }
	}

	// Wait for power supplies to stabilize
	wait_ms(100);

	pc.printf("LOG: checking everything...\r\n");

	//safety_check();

	dump_telemetry(&pc);

	// bring up clocks
	lmk04826b.init();
	lmk04133.init();
	ads42lb69.init();
	dac3484.init(&pc);

	// once ADC, DAC, CLOCK, and SYNTH programmed then increase SPI bus speed
	spi_bus.frequency(500000);

	// calibrating power amplifiers

	//safety_check();

	calibrate_power_amplifier(&afe_a);
	calibrate_power_amplifier(&afe_b);

	//safety_check();

	// Configure RF AFE
	switch_to_safe(&afe_a);
	switch_to_safe(&afe_b);

	dump_telemetry(&pc);

	pc.printf("LOG: Ready!\n\r");
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
void changeState(uint8_t msg)
{
	if( (CS_RX == (CS_RX_FLAG_MASK & msg)) && (CS_AFE_A == (CS_AFE_MASK & msg)) )
		switch_to_rx(&afe_a, CS_OP_CODE_MASK & msg);
	if( (CS_RX == (CS_RX_FLAG_MASK & msg)) && (CS_AFE_B == (CS_AFE_MASK & msg)) )
		switch_to_rx(&afe_b, CS_OP_CODE_MASK & msg);
	if( CS_RX != (CS_RX_FLAG_MASK & msg) )
		switch( CS_OP_CODE_MASK & msg )
		{
		case CS_OP_CODE_0:
			switch_to_safe(&afe_a);
			switch_to_safe(&afe_b);
			break;
		case CS_OP_CODE_1:
			//dump_telemetry(&fpga);
			break;
		case CS_OP_CODE_2:
			dump_telemetry(&pc);
			break;
		case CS_OP_CODE_3:
			if( CS_AFE_A == (CS_AFE_MASK & msg) )
				switch_to_tx(&afe_a);
			if( CS_AFE_B == (CS_AFE_MASK & msg) )
				switch_to_tx(&afe_b);
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
			dac3484.set_current((CS_OP_CODE_MASK & msg) - CS_OP_CODE_4);
			break;
		case CS_OP_CODE_20:
			pc.printf("\033[0;0H");
			pc.printf("P_IN_A        = %i    \r\n", P_IN_A);
			pc.printf("P_OUT_A       = %i    \r\n", P_OUT_A);
		case CS_OP_CODE_63:
		default:
			break;
		}
}


int main()
{
	uint16_t check = 0;                   // safety checklist item number

	// initialize entire board
	init();

	int slow = 1;

	int mode_switched = 0;

	while(1)
	{
		if(slow % 100000 == 0 ) {
//			pc.printf("x\n");
			slow = 0;
			if(!mode_switched) {
				pc.printf("switched to tx a\n");
				mode_switched = 1;
				//changeState(CS_OP_CODE_3 | CS_AFE_A);// required to enable tx
//				changeState(CS_OP_CODE_19 | 0x400);
//				changeState(CS_OP_CODE_7); // dac current
				//changeState(CS_OP_CODE_18); // dac current
				//configure_grav_on_with_tx_on(&afe_0);
				//switch_to_rx(&afe_b);
			changeState(CS_RX | CS_AFE_B | 0xf); //set CH B to rx and dsa to 2
			}
			changeState(CS_OP_CODE_2);
		}
		slow++;

		/*if( interrupt )
		{
			//
			// it takes up to 200us to guarantee entry into this block so
			// make sure user pulses interrupt pin for 200us. Note that
			// no new safety checks are called when interrupt is high
			// so dont pulse it for more than 200us.
			//
			while( interrupt ) { }

			// process commands first-in-first-out
			while( fpga.readable() )
				changeState(fpga.getc());

			// if either one of the channels is set to transmit then enable DAC output
			if( (AMC7891_MODE_TX == afe_a.mode) | (AMC7891_MODE_TX == afe_b.mode) )
				configure_grav_on_with_tx_on(&afe_0);
			else
				configure_grav_on_with_tx_off(&afe_0);
		}*/

		//check = safety_check(check);
	}
}


