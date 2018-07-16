/*
 * graviton.h
 *
 *  Created on: Jul 15, 2018
 *      Author: karthik
 */

#ifndef INCLUDES_GRAVITON_H_
#define INCLUDES_GRAVITON_H_

#include "amc7891.h"


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

#define GRAV_DAC_PA_EN      0x1
#define GRAV_DAC_VGSET      0x2
#define GRAV_DAC_TX_SW      0x4
#define GRAV_DAC_TX_LNA_DIS 0x8

#define GRAV_ADC_V2V5       AMC_ADC0
#define GRAV_ADC_V1V8       AMC_ADC1
#define GRAV_ADC_V3V8       AMC_ADC2
#define GRAV_ADC_V5V5N_V    AMC_ADC3
#define GRAV_ADC_V5V5_V     AMC_ADC4
#define GRAV_ADC_V29_V      AMC_ADC5
#define GRAV_ADC_V3V8_I     AMC_ADC6 // this is broken on Graviton Rev. 3
#define GRAV_ADC_ADC_TEMP   AMC_ADC7

#define GRAV_ADC_P_OUT      0x01
#define GRAV_ADC_I_PA       0x02
#define GRAV_ADC_T_PRE_RX   0x04
#define GRAV_ADC_T_LDO      0x08
#define GRAV_ADC_P_IN       0x10
#define GRAV_ADC_T_PA       0x20
#define GRAV_ADC_T_PRE_TX2  0x40
#define GRAV_ADC_T_PRE_TX3  0x80

/********************************************************
 * Standard settings
 */

#define GRAV_CH_ENABLES (GRAV_EN_DSA | GRAV_EN_LNA_DIS | GRAV_EN_PRE_RX_EN | GRAV_EN_PRE_TX_EN | GRAV_EN_TX_EN | GRAV_EN_RX_EN)
#define GRAV_CH_DEFAULTS (GRAV_EN_DSA | GRAV_EN_LNA_DIS)

#define GRAV_MAIN_ENABLES (GRAV_EN_ETH | GRAV_EN_1V8 | GRAV_EN_2V5 | GRAV_EN_TX | GRAV_EN_ADC)
#define GRAV_MAIN_DEFAULTS (0)

#define GRAV_MAIN_POWER_ON_TX_OFF (GRAV_EN_ETH | GRAV_EN_1V8 | GRAV_EN_2V5 | GRAV_EN_ADC)
#define GRAV_MAIN_POWER_ON_TX_ON (GRAV_EN_ETH | GRAV_EN_1V8 | GRAV_EN_2V5 | GRAV_EN_TX | GRAV_EN_ADC)



#define GRAV_AFE_ENABLES (GRAV_EN_DSA | GRAV_EN_LNA_DIS | GRAV_EN_PRE_RX_EN | GRAV_EN_PRE_TX_EN | GRAV_EN_TX_EN | GRAV_EN_RX_EN)
#define GRAV_AFE_SAFE (GRAV_EN_LNA_DIS | GRAV_EN_DSA)

#define GRAV_AFE_DAC0_SAFE (0x0000) // turn off PA
#define GRAV_AFE_DAC1_SAFE (0x03FF) // set PA gate voltage to minimum (remember its inverted)
#define GRAV_AFE_DAC2_SAFE (0x02A3) // set TX/RX switch to RX (3.6V)
#define GRAV_AFE_DAC3_SAFE (0x03FF) // disable the TX LNA


uint32_t get_v2v5(AMC7891 *afe);
uint32_t get_v1v8(AMC7891 *afe);
uint32_t get_v3v8(AMC7891 *afe);
uint32_t get_v5v5(AMC7891 *afe);
uint32_t get_v5v5n(AMC7891 *afe);
uint32_t get_v29(AMC7891 *afe);
int32_t get_adc_temp(AMC7891 *afe);

uint8_t switch_to_ext_osc();
uint8_t switch_to_internal_clock();

void make_afe_dac_safe(AMC7891 *afe);





#endif /* INCLUDES_GRAVITON_H_ */
