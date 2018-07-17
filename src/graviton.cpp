/*
 * graviton.cpp
 *
 *  Created on: Jul 16, 2018
 *      Author: karthik
 */


#include "graviton.h"

void make_afe_dac_safe(AMC7891 *afe)
{
	afe->write_dac(AMC_DAC0, GRAV_AFE_DAC0_SAFE);
	afe->write_dac(AMC_DAC1, GRAV_AFE_DAC1_SAFE);
	afe->write_dac(AMC_DAC2, GRAV_AFE_DAC2_SAFE);
	afe->write_dac(AMC_DAC3, GRAV_AFE_DAC3_SAFE);
	afe->enable_dacs();
}

uint32_t get_mv(AMC7891 *afe, ADC_CHANNEL channel)
{
	uint32_t in;
	in = afe->read_adc(channel);

	return (in * 5 * 1000) / 1024;
}

int32_t conv_mv_v5v5n(uint32_t in_mv, uint32_t v5)
{
	int32_t in = (int32_t)in_mv;
	int32_t ref = (int32_t)v5;

	return (6800*ref - 2000*in - 6800*in)/(2*6800 + 2000);
}

uint32_t conv_mv_v5v5(uint32_t in_mv)
{
	return (in_mv * 880) / 200;
}

uint32_t conv_mv_v29(uint32_t in_mv)
{
	return (in_mv * 533) / 23;
}

uint32_t conv_mv_i_pa(uint32_t in_mv)
{
	return (in_mv*1000)/2820;
}

uint8_t switch_to_ext_osc()
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;


    /* Reset the RCC clock configuration to the default reset state ------------*/
    /* Set HSION bit */
    RCC->CR |= 0x00000001U;

    /* Reset CFGR register */
    RCC->CFGR &= 0xF87FC00CU;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= 0xFEF6FFFFU;

    /* Reset HSEBYP bit */
    RCC->CR &= 0xFFFBFFFFU;

    /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE bits */
    RCC->CFGR &= 0xFF80FFFFU;

    /* Reset PREDIV1[3:0] bits */
    RCC->CFGR2 &= 0xFFFFFFF0U;

    /* Reset USARTSW[1:0], I2CSW and TIMs bits */
    RCC->CFGR3 &= 0xFF00FCCCU;

    /* Disable all interrupts */
    RCC->CIR = 0x00000000U;

    /* Enable HSE oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;

    RCC_OscInitStruct.HSEState          = RCC_HSE_BYPASS; /* External 8 MHz clock on OSC_IN */

    RCC_OscInitStruct.HSEPredivValue      = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL9; // 72 MHz (8 MHz * 9)
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return 0; // FAIL
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType      = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK; // 72 MHz
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;         // 72 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;           // 36 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;           // 72 MHz
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        return 0; // FAIL
    }

    return 1; // OK
}


uint8_t switch_to_internal_clock()
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;


    /* Reset the RCC clock configuration to the default reset state ------------*/
    /* Set HSION bit */
    RCC->CR |= 0x00000001U;

    /* Reset CFGR register */
    RCC->CFGR &= 0xF87FC00CU;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= 0xFEF6FFFFU;

    /* Reset HSEBYP bit */
    RCC->CR &= 0xFFFBFFFFU;

    /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE bits */
    RCC->CFGR &= 0xFF80FFFFU;

    /* Reset PREDIV1[3:0] bits */
    RCC->CFGR2 &= 0xFFFFFFF0U;

    /* Reset USARTSW[1:0], I2CSW and TIMs bits */
    RCC->CFGR3 &= 0xFF00FCCCU;

    /* Disable all interrupts */
    RCC->CIR = 0x00000000U;

    /* Enable HSI oscillator and activate PLL with HSI as source */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSEState            = RCC_HSE_OFF;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL16; // 64 MHz (8 MHz/2 * 16)
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        return 0; // FAIL
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType      = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK; // 64 MHz
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;         // 64 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;           // 32 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;           // 64 MHz
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        return 0; // FAIL
    }

    /* Output clock on MCO1 pin(PA8) for debugging purpose */
    //HAL_RCC_MCOConfig(RCC_MCO, RCC_MCOSOURCE_HSI, RCC_MCO_DIV1); // 8 MHz

    return 1; // OK
}

