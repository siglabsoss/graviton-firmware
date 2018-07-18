/*
 * graviton.h
 *
 *  Created on: Jul 15, 2018
 *      Author: karthik
 */

#ifndef INCLUDES_GRAVITON_H_
#define INCLUDES_GRAVITON_H_

#include "amc7891.h"
#include "lm20.h"
#include "lm5531.h"


#define GRAV_EN_CLK         0x001 // this is broken on Graviton Rev. 3
#define GRAV_EN_ETH         0x002
#define GRAV_EN_1V8         0x004
#define GRAV_EN_2V5         0x008
#define GRAV_EN_TX          0x040 // Active high DAC transmit enable (internal pull-down). Be sure to set sif_txenable to 0b to use this
#define GRAV_EN_ADC         0x200
#define GRAV_EN_DAC         0x400 // this is broken on Graviton Rev. 3
#define GRAV_EN_SYN         0x800 // this is broken on Graviton Rev. 3

#define GRAV_EN_DSA         0x07F
#define GRAV_EN_LNA_DIS     0x080
#define GRAV_EN_PRE_RX_EN   0x100
#define GRAV_EN_PRE_TX_EN   0x200
#define GRAV_EN_TX_EN       0x400
#define GRAV_EN_RX_EN       0x800

#define GRAV_ADC_V2V5       AMC_ADC0
#define GRAV_ADC_V1V8       AMC_ADC1
#define GRAV_ADC_V3V8       AMC_ADC2
#define GRAV_ADC_V5V5N      AMC_ADC3
#define GRAV_ADC_V5V5       AMC_ADC4
#define GRAV_ADC_V29        AMC_ADC5
#define GRAV_ADC_I3I8       AMC_ADC6 // this is broken on Graviton Rev. 3
#define GRAV_ADC_ADC_TEMP   AMC_ADC7

#define GRAV_ADC_P_OUT      AMC_ADC0
#define GRAV_ADC_I_PA       AMC_ADC1
#define GRAV_ADC_T_PRE_RX   AMC_ADC2
#define GRAV_ADC_T_LDO      AMC_ADC3
#define GRAV_ADC_P_IN       AMC_ADC4
#define GRAV_ADC_T_PA       AMC_ADC5
#define GRAV_ADC_T_PRE_TX2  AMC_ADC6
#define GRAV_ADC_T_PRE_TX3  AMC_ADC7

#define GRAV_DAC_PA_EN      AMC_DAC0
#define GRAV_DAC_VGSET      AMC_DAC1
#define GRAV_DAC_TX_SW      AMC_DAC2
#define GRAV_TX_LNA_DIS     AMC_DAC3


enum GAIN
{
	GAIN_1X = 2,
	GAIN_2X = 1
};

/********************************************************
 * Standard settings
 */

#define GRAV_MAIN_ENABLES (GRAV_EN_ETH | GRAV_EN_1V8 | GRAV_EN_2V5 | GRAV_EN_TX | GRAV_EN_ADC)
#define GRAV_MAIN_DEFAULTS (0)

#define GRAV_MAIN_POWER_ON_TX_OFF (GRAV_EN_ETH | GRAV_EN_1V8 | GRAV_EN_2V5 | GRAV_EN_ADC)
#define GRAV_MAIN_POWER_ON_TX_ON (GRAV_EN_ETH | GRAV_EN_1V8 | GRAV_EN_2V5 | GRAV_EN_TX | GRAV_EN_ADC)


// AFE GPO that are enabled

#define GRAV_AFE_ENABLES (GRAV_EN_DSA | GRAV_EN_LNA_DIS | GRAV_EN_PRE_RX_EN | GRAV_EN_PRE_TX_EN | GRAV_EN_TX_EN | GRAV_EN_RX_EN)

/*
 * All the different modes that the AFE can be in
 */

#define GRAV_AFE_SAFE (GRAV_EN_LNA_DIS | GRAV_EN_DSA)





#define GRAV_DAC_PA_EN_SAFE       (0x0000) // turn off PA
#define GRAV_DAC_VGSET_SAFE       (0x03FF) // set PA gate voltage to minimum (remember its inverted)
#define GRAV_DAC_TX_SW_SAFE       (0x02A3) // set TX/RX switch to RX (3.6V)
#define GRAV_TX_LNA_DIS_SAFE      (0x03FF) // disable the TX LNA


int32_t get_adc_temp(AMC7891 *afe);

uint32_t get_mv(AMC7891 *afe, ADC_CHANNEL channel, GAIN gain);
int32_t conv_mv_v5v5n(uint32_t in_mv);
uint32_t conv_mv_v5v5(uint32_t in_mv);
uint32_t conv_mv_v29(uint32_t in_mv);
uint32_t conv_mv_i_pa(uint32_t in_mv);
int32_t conv_dac_counts_to_mv_for_vgg_circuit(uint32_t counts);
uint32_t conv_mv_to_dac_counts_for_vgg_circuit(int32_t in_mv);
uint8_t switch_to_ext_osc();
uint8_t switch_to_int_osc();

void make_afe_dac_safe(AMC7891 *afe);

/*
 * SHORTCUTS
 */

#define AFE_CH_A_TEMP afe_a.read_temperature()
#define AFE_CH_B_TEMP afe_b.read_temperature()
#define AFE_MAIN_TEMP afe_0.read_temperature()
#define V2V5          get_mv(&afe_0, GRAV_ADC_V2V5, GAIN_1X)
#define V1V8          get_mv(&afe_0, GRAV_ADC_V1V8, GAIN_1X)
#define V3V8          get_mv(&afe_0, GRAV_ADC_V3V8, GAIN_2X)
#define V5V5          conv_mv_v5v5(get_mv(&afe_0, GRAV_ADC_V5V5, GAIN_1X))
#define V5V5N         conv_mv_v5v5n(get_mv(&afe_0, GRAV_ADC_V5V5N, GAIN_1X))
#define V29           conv_mv_v29(get_mv(&afe_0, GRAV_ADC_V29, GAIN_1X))
#define ADC_LM20      lm20_conv_mv_degc(get_mv(&afe_0, GRAV_ADC_ADC_TEMP, GAIN_1X))
#define MCU_TEMP      (int32_t)(((1.43 - (3 * mcu_temp))/4.3)+25)

#define P_OUT_A       lm5331_conv_mv_db_out(get_mv(&afe_a, GRAV_ADC_P_IN, GAIN_1X),get_mv(&afe_a, GRAV_ADC_P_OUT, GAIN_1X))
#define I_PA_A        conv_mv_i_pa(get_mv(&afe_a, GRAV_ADC_I_PA, GAIN_1X))
#define T_PRE_RX_A    lm20_conv_mv_degc(get_mv(&afe_a, GRAV_ADC_T_PRE_RX, GAIN_1X))
#define T_LDO_A       lm20_conv_mv_degc(get_mv(&afe_a, GRAV_ADC_T_LDO, GAIN_1X))
#define P_IN_A        lm5331_conv_mv_db_in(get_mv(&afe_a, GRAV_ADC_P_IN, GAIN_1X),get_mv(&afe_a, GRAV_ADC_P_OUT, GAIN_1X))
#define T_PA_A        lm20_conv_mv_degc(get_mv(&afe_a, GRAV_ADC_T_PA, GAIN_1X))
#define T_PRE_TX2_A   lm20_conv_mv_degc(get_mv(&afe_a, GRAV_ADC_T_PRE_TX2, GAIN_1X))
#define T_PRE_TX3_A   lm20_conv_mv_degc(get_mv(&afe_a, GRAV_ADC_T_PRE_TX3, GAIN_1X))

#define P_OUT_B       lm5331_conv_mv_db_out(get_mv(&afe_b, GRAV_ADC_P_IN, GAIN_1X),get_mv(&afe_b, GRAV_ADC_P_OUT, GAIN_1X))
#define I_PA_B        conv_mv_i_pa(get_mv(&afe_b, GRAV_ADC_I_PA, GAIN_1X))
#define T_PRE_RX_B    lm20_conv_mv_degc(get_mv(&afe_b, GRAV_ADC_T_PRE_RX, GAIN_1X))
#define T_LDO_B       lm20_conv_mv_degc(get_mv(&afe_b, GRAV_ADC_T_LDO, GAIN_1X))
#define P_IN_B        lm5331_conv_mv_db_in(get_mv(&afe_b, GRAV_ADC_P_IN, GAIN_1X),get_mv(&afe_b, GRAV_ADC_P_OUT, GAIN_1X))
#define T_PA_B        lm20_conv_mv_degc(get_mv(&afe_b, GRAV_ADC_T_PA, GAIN_1X))
#define T_PRE_TX2_B   lm20_conv_mv_degc(get_mv(&afe_b, GRAV_ADC_T_PRE_TX2, GAIN_1X))
#define T_PRE_TX3_B   lm20_conv_mv_degc(get_mv(&afe_b, GRAV_ADC_T_PRE_TX3, GAIN_1X))


#endif /* INCLUDES_GRAVITON_H_ */
