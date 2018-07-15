#include "mbed.h"
 
DigitalOut LED_GRN(PB_1);
DigitalOut LED_YLW(PB_0);

DigitalOut SW_DIVERSITY(PB_5);
DigitalOut SW_RADIO(PA_15);
DigitalOut SW_ANTENNA(PB_4);

DigitalOut EN_LNA(PB_3);
DigitalOut EN_PS(PA_3);
DigitalOut SET_PS_FB_TRIM(PA_6);

AnalogIn I_SNS(PA_2);
AnalogIn V_SNS(PA_7);
AnalogIn V_PHANTOM(PA_4);

AnalogOut PA_BIAS(PA_5);

float BIAS_CAL = 0.0;


enum MODE_T
{
	TX,
	RX
};

#define FEEDBACK_GAIN 0.001        // Volts per Amp

#define SWITCH_THRESHOLD 20        // Volts

#define RF1 1                      // per PE42820 datasheet
#define RF2 0

#define ANT_J1 RF1                 // top antenna
#define ANT_J3 RF2                 // middle antenna

#define V6   1                     // power supply feedback trim
#define V28  0

#define ON   1
#define OFF  0


void isr_calibrate()
{
	/*
	 *  Per NPA1006 datasheet set quiescent current to 88mA.
	 *  Sense resistor = 300mOhm.
	 */

	BIAS_CAL += FEEDBACK_GAIN * (I_SNS * 0.3 - 0.088);

	PA_BIAS = BIAS_CAL;
}


float get_supply_voltage()
{
	return V_SNS * 3.3 * 208 / 11;
}


void mode(MODE_T mode)
{
	switch (mode)
	{
		case TX:
			EN_LNA = false;        // turn off PA
			SW_RADIO = RF1;
			SW_ANTENNA = RF2;

			wait_us(100);          // wait for switches

			SET_PS_FB_TRIM = V28;  // turn on PA
			PA_BIAS = BIAS_CAL;
			LED_YLW = ON;
			break;

		case RX:
			PA_BIAS = 0.0;         // turn off PA
			SET_PS_FB_TRIM = V6;

			wait_us(10);           // wait for bias to fall

			SW_RADIO = RF2;
			SW_ANTENNA = RF1;

			wait_us(100);          // wait for switches

			EN_LNA = true;         // turn on LNA
			LED_YLW = OFF;
			break;

		default:
			break;
	}
}


void init()
{
	DAC_ChannelConfTypeDef sConfig;
	DAC_HandleTypeDef hdac;

	wait_ms(100);                  // wait for supplies to stabilize

	PA_BIAS = 0.0;
	hdac.Instance = DAC1;
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);

	SET_PS_FB_TRIM = V6;

	EN_LNA = false;
	SW_DIVERSITY = ANT_J3;

	wait_us(100);                  // wait for bias to fall

	EN_PS = true;

	wait_ms(10);
}


void calibrate()
{
	/*
	 *  Turn on feedback loop for 500 ms
	 *  and hold calibration.
	 */

	mode(TX);

	SW_RADIO = RF2;                // disconnect PA input

	Ticker ticker;

	ticker.attach(&isr_calibrate, 0.001);

	wait_ms(500);

	ticker.detach();

	BIAS_CAL = PA_BIAS;
	LED_GRN = ON;
}


int main()
{
	init();

	calibrate();

    while(1)
    {
    	while( get_supply_voltage() > SWITCH_THRESHOLD ) { }
    	mode(TX);
    	while( get_supply_voltage() < SWITCH_THRESHOLD ) { }
    	mode(RX);
    }
}
