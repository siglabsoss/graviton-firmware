#include "mbed.h"

#include "BufferedSerial.h"

#include "amc7891.h"
#include "graviton.h"
#include "lmk04826b.h"
#include "lmk04133.h"
#include "ads42lb69.h"
#include "dac3484.h"
#include "RiscvInterface.hpp"
#include "FirmwareUtils.hpp"


void set_led(const unsigned idx, const bool val);
void go_safe_and_reset(const char* reason, const char* reason2 = 0, const int32_t reason3 = 0);


// order is  tx,   rx
RawSerial pc(PA_2, NC);

// 9 is tx
// 10 is rx
RawSerial fpga(PA_9, PA_10); // works

RiscvInterface<RawSerial> riscv(&fpga);

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
    out->printf("E_DAC_ALARM   = %x\r\n", ((E_DAC)&(~0x0040)) );
    dac3484.clear_alarms();
}


void go_safe_and_reset(const char* reason, const char* reason2, const int32_t reason3)
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

    if( reason2 == 0 ) {
        pc.printf("ERROR: POWERED DOWN. %s\r\n", reason);
    } else {
        pc.puts(reason2);
        pc.printf(" value: %d\r\n", reason3);
        pc.printf("ERROR: POWERED DOWN. %s\r\n", reason);
    }

    dump_telemetry(&pc);

    pc.printf("LOG: halted. resetting in 5 seconds.\r\n");

    for( int i = 0; i < 5; i++) {
        riscv.notifyError();
        wait_ms(1000);
    }

    NVIC_SystemReset();
}

typedef struct {
    const int32_t lower_hard;
    const int32_t lower_tol;
    const int32_t upper_hard;
    const int32_t repeat_tol;
    int32_t violations;
    const char* name;       // short name which we print, and print 'value: ' afterwards
    const char* msg;        // full message to print, like previous code
} soft_bottom_tolerance_t;


// This has a soft bottom and a hard bottom, with a hard top
// the value is only allowed to be below the soft bottom for a certain amount of time
// (the units are iterations).  If it is below the soft bottom for more than N
// iterations, it triggers a fault
// if it is below the hard bottom for above the hard top we trigger fault immedaitly
soft_bottom_tolerance_t v3v8v_soft =
{  3100,   3150,   4000,      100,       0,  "3.8", "3.8V rail out of spec"};

soft_bottom_tolerance_t v5v5v_soft =
{  4250,   4300,   5750,      100,       0,  "5.5", "5.5V rail out of spec"};

soft_bottom_tolerance_t v5v5vneg_soft =
{ -7600,  -7500,  -5350,      100,       0,  "-5.5", "negative 5.5V rail out of spec"};


// returns true if error / we did reset
bool go_safe_and_reset_tolerance(soft_bottom_tolerance_t* tolerance, const int32_t value) {

    // tolerance_t* tolerance = &(saftey_tolerance[test_item]);


    //    if( (3500 > value) || (4000 < value) ) {
    // pc.printf("3.8 value: %d", value);
    // go_safe_and_reset("3.8V rail out of spec");
     //    }

    // check against hard lower and hard upper first
    if( (tolerance->lower_hard > value) || (tolerance->upper_hard < value) ) {
        go_safe_and_reset(tolerance->msg, tolerance->name, value);
        return true;
    }


    // check against soft lower
    if( (tolerance->lower_tol > value) ) {
        if( tolerance->violations >= tolerance->repeat_tol ) {
            go_safe_and_reset(tolerance->msg, tolerance->name, value);
            return true;
        }
        
        // increment now
        tolerance->violations++;

        pc.printf("%s r: %d v: %d\r\n", tolerance->name, tolerance->violations, value);

        return false;
    } else {

        tolerance->violations = 0;
        return false;
    }

    return false;
}

uint16_t disable_saftey_vector[32] = {};


///
/// Run a specific safety check and return the next safety check in the list.
/// Do not re-number these tests without first considering the broken_boards() function
/// maybe these checks could be moved to #defines

uint16_t safety_check(const uint16_t test_item)
{
    int32_t value;

    ///
    /// This is an array of values which will disable checks if the array entry is set
    /// When the array entry is set to 0, the test is enabled
    /// see init_broken_boards()
    /// 
    const uint16_t test_or_disabled = test_item | disable_saftey_vector[test_item];

    switch( test_or_disabled )
    {
    case 0:
        value = V2V5;
        if( (1500 > value) || (2600 < value) ) go_safe_and_reset("2.5V rail out of spec", "2.5", value);
        break;
    case 1:
        value = V1V8;
        if( (1700 > value) || (1900 < value) ) go_safe_and_reset("1.8V rail out of spec", "1.8", value);
        break;
    case 2:
        value = V3V8;
        go_safe_and_reset_tolerance(&v3v8v_soft, value);
     //    if( (3500 > value) || (4000 < value) ) {
        // pc.printf("3.8 value: %d", value);
        // go_safe_and_reset("3.8V rail out of spec");
     //    }
        break;
    case 3:
        value = V5V5;
        // if( (5000 > value) || (5750 < value) ) go_safe_and_reset("5.5V rail out of spec", "5.5", value);
        go_safe_and_reset_tolerance(&v5v5v_soft, value);
        break;
    case 4:
        value = V5V5N;
        /*telemetry is wrong so this value is wrong to compensate */
        // if( (-6900  > value) || (-5350 < value) )
        // go_safe_and_reset("negative 5.5V rail out of spec", "negative 5.5", value);
        go_safe_and_reset_tolerance(&v5v5vneg_soft, value);
        break;
    case 5:
        value = V29;
        if( (28000 > value) || (29500 < value) ) go_safe_and_reset("29V rail out of spec", "29", value);
        break;
    case 6:
        value = AFE_CH_A_TEMP;
        if( 60 < value ) go_safe_and_reset("AFE #A overheated", "AFE #A", value);
        break;
    case 7:
        value = AFE_CH_B_TEMP;
        if( 60 < value ) go_safe_and_reset("AFE #B overheated", "AFE #B", value);
        break;
    case 8:
        value = AFE_MAIN_TEMP;
        if( 60 < value ) go_safe_and_reset("Main AFE overheated", "Main AFE", value);
        break;


    case 9:
        value = ADC_LM20;
        if( 70 < value ) go_safe_and_reset("ADC overheated", "ADC", value);
        break;
    case 10:
        value = T_PRE_RX_A;
        if( 70 < value ) go_safe_and_reset("RX preamp #A overheated", "RX preamp #A", value);
        break;
    case 11:
        value = T_LDO_A;
        if( 70 < value ) go_safe_and_reset("LDO #A overheated", "LDO #A", value);
        break;
    case 12:
        value = T_PA_A;
        if( 90 < value ) go_safe_and_reset("PA #A overheated", "PA #A", value);
        break;
    case 13:
        value = T_PRE_TX2_A;
        if( 70 < value ) go_safe_and_reset("TX preamp 2 #A overheated", "TX preamp 2 #A", value);
        break;
    case 14:
        value = T_PRE_TX3_A;
        if( 70 < value ) go_safe_and_reset("TX preamp 3 #A overheated", "TX preamp 3 #A", value);
        break;
    case 15:
        //if( 33 < P_IN_A ) go_safe_and_reset("PA #A return power > 2 Watts");
        break;


    case 16:
        value = T_PRE_RX_B;
        if( 70 < value ) go_safe_and_reset("RX preamp #B overheated", "RX preamp #B", value);
        break;
    case 17:
        value = T_LDO_B;
        if( 70 < value ) go_safe_and_reset("LDO #B overheated", "LDO #B", value);
        break;
    case 18:
        value = T_PA_B;
        if( 90 < value ) go_safe_and_reset("PA #B overheated", "PA #B", value);
        break;
    case 19:
        value = T_PRE_TX2_B;
        if( 70 < value ) go_safe_and_reset("TX preamp 2 #B overheated", "TX preamp 2 #B", value);
        break;
    case 20:
        value = T_PRE_TX3_B;
        if( 70 < value ) go_safe_and_reset("TX preamp 3 #B overheated", "TX preamp 3 #B", value);
        break;
    case 21:
        //if( 33 < P_IN_B ) go_safe_and_reset("PA #B return power > 2 Watts");
        break;
    ///
    /// Make sure to increase disable_saftey_vector if we add more than 32 cases here
    ///
    default:
        return 0;
    }

    return test_item+1;
}


/*
 * Safety check all items
 */
// void safety_check_all()
// {
//     uint16_t item = 0;
//     while( 0 < safety_check(item++) ) { }
// }

///
/// Broken Boards
/// 

void zero_broken_boards() {
    const unsigned len = ARRAY_SIZE(disable_saftey_vector);

    for(unsigned i = 0; i < len; i++) {
        disable_saftey_vector[i] = 0;
    }
}

void disable_saftey_check(const uint16_t test_item) {
    const unsigned len = ARRAY_SIZE(disable_saftey_vector);

    if( test_item >= len ) {
        return;
    }

    disable_saftey_vector[test_item] = 0x8000;
}

void enable_saftey_check(const uint16_t test_item) {
    const unsigned len = ARRAY_SIZE(disable_saftey_vector);

    if( test_item >= len ) {
        return;
    }

    disable_saftey_vector[test_item] = 0;
}

void init_broken_boards() {

    // reset array, enabling all checks
    zero_broken_boards();

    const uint32_t a = DEV_ID0;
    const uint32_t b = DEV_ID1;
    const uint32_t c = DEV_ID2;

    // Board 008
    // 8000e 524b4303 20323035

    
    // Board 012
    // 150016 524b4303 20323035
    if( a == 0x150016 && b == 0x524b4303 && c == 0x20323035 ) {
        // Board 012 has a broken PA #A temp sensor
        // note that 12 here is the check number and not the board number
        disable_saftey_check(12);

        pc.printf("Board 012 has broken PA #A sensor!!\r\n");
    }

    // Board 021
    // 800f0011    524b4303    20323035
    if( a == 0x800f0011 && b == 0x524b4303 && c == 0x20323035 ) {
        disable_saftey_check(11);
        disable_saftey_check(10);

        pc.printf("Board 021 has broken LDO #A sensor!!\r\n");
        pc.printf("Board 021 has HOT RX preamp #A!!\r\n");
    }

    // Board 007
    if( a == 0x140013 && b == 0x524b4303 && c == 0x20323035 ) {
        disable_saftey_check(16);
        // disable_saftey_check(10);
        pc.printf("Board 007 has broken RX preamp #B sensor!!\r\n");
        // pc.printf("Board 007 has zero value preamp #A!!\r\n");
    }

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
    if( (V2V5 < 300) && (V1V8 < 400) ) {
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
    if( (CS_RX == (CS_RX_FLAG_MASK & msg)) && (CS_AFE_A == (CS_AFE_MASK & msg)) ) {
        switch_to_rx(&afe_a, CS_OP_CODE_MASK & msg);
    }
    if( (CS_RX == (CS_RX_FLAG_MASK & msg)) && (CS_AFE_B == (CS_AFE_MASK & msg)) ) {
        switch_to_rx(&afe_b, CS_OP_CODE_MASK & msg);
    }
    if( CS_RX != (CS_RX_FLAG_MASK & msg) )
    switch( CS_OP_CODE_MASK & msg )
        {
        case CS_OP_CODE_0:
            switch_to_safe(&afe_a);
            switch_to_safe(&afe_b);
            break;
        case CS_OP_CODE_1:
            dump_telemetry(&fpga);
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
            dac3484.set_current(&pc, (CS_OP_CODE_MASK & msg) - CS_OP_CODE_4);
            break;
        case CS_OP_CODE_20:
            pc.printf("\033[0;0H");
            pc.printf("P_IN_A        = %i    \r\n", P_IN_A);
            pc.printf("P_OUT_A       = %i    \r\n", P_OUT_A);
            break;
        case CS_OP_CODE_21:
            dac3484.read_config(&pc);
            break;
        case CS_OP_CODE_22:
            if(CS_AFE_A == (CS_AFE_MASK & msg))
                switch_to_safe(&afe_a);
            if(CS_AFE_B == (CS_AFE_MASK & msg))
                switch_to_safe(&afe_b);
            break;
        case CS_OP_CODE_63:
        default:
            break;
        }
}


int main()
{
    int initial_wait=0;
    while(initial_wait != 5000000) {
    initial_wait++;
    }

    uint16_t check = 0;                   // safety checklist item number

    // initialize entire board
    init();

    init_broken_boards();

    int slow = 1;

    int mode_switched = 0;

    int dac_current = 0;
    (void) dac_current;

    pc.printf("Board ID Booted:   %x    %x    %x\r\n", DEV_ID0, DEV_ID1, DEV_ID2);

    while(1)
    {
        if(slow % 10000 == 0 ) {
            //          pc.printf("x\n");
            //slow = 0;
            if(!mode_switched) {
                // pc.printf("switched to tx a\n");
                mode_switched = 1;
                // changeState(CS_OP_CODE_3 | CS_AFE_A);
                //              changeState(CS_OP_CODE_19 | 0x400);
                //changeState(CS_OP_CODE_7); // dac current
                changeState(CS_OP_CODE_21);
                //changeState(CS_OP_CODE_18); // dac current
                // configure_grav_on_with_tx_on(&afe_0);
            }
            changeState(CS_OP_CODE_2);
            // fpga.printf("hello\r\n");
                riscv.alive();
        }
        slow++;

        if(fpga.readable()) {
            uint8_t val = fpga.getc();
            changeState(val);
            pc.printf("received char: %d\r\n", (int) val);
            riscv.echoChar(val);


            // either one of the channels is set to transmit then enable DAC output
            if( (AMC7891_MODE_TX == afe_a.mode) | (AMC7891_MODE_TX == afe_b.mode) )
                configure_grav_on_with_tx_on(&afe_0);
            else
                configure_grav_on_with_tx_off(&afe_0);
        }
        //pc.printf("BEFORE safety check\r\n");
        check = safety_check(check);
        //pc.printf("AFTER safety check\r\n");
        /*if(slow % 5000000 == 0 && mode_switched) {
          changeState(CS_OP_CODE_4 + dac_current);
          pc.printf("DAC_CURRENT: %d\n", dac_current);
          dac_current++;
          if(dac_current == 15) {
          dac_current = 0;
          }
          }*/

        /*if( true || interrupt )
          {
          //
          // it takes up to 200us to guarantee entry into this block so
          // make sure user pulses interrupt pin for 200us. Note that
          // no new safety checks are called when interrupt is high
          // so dont pulse it for more than 200us.
          //

          // pc.printf("before\r\n");

          // while( interrupt ) { }

          // process commands first-in-first-out
          while( fpga.readable() ) {
          chan
          pc.printf("char: %d\r\n", (int)fpga.getc());
          }
          // if either one of the channels is set to transmit then enable DAC output
          // if( (AMC7891_MODE_TX == afe_a.mode) | (AMC7891_MODE_TX == afe_b.mode) )
          //    configure_grav_on_with_tx_on(&afe_0);
          // else
          //    configure_grav_on_with_tx_off(&afe_0);
          }
        */
        //check = safety_check(check);
    }
}


