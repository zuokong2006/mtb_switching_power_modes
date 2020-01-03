/*******************************************************************************
* File Name: cycfg_peripherals.c
*
* Description:
* Peripheral Hardware Block configuration
* This file was automatically generated and should not be modified.
* Device Configurator: 2.0.0.1483
* Device Support Library (../../../libs/psoc6pdl): 1.4.0.1889
*
********************************************************************************
* Copyright 2017-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
********************************************************************************/

#include "cycfg_peripherals.h"

#define APP_COUNTER_INPUT_DISABLED 0x7U
#define KIT_LED1_PWM_INPUT_DISABLED 0x7U

cy_stc_csd_context_t cy_csd_0_context = 
{
	.lockKey = CY_CSD_NONE_KEY,
};
const cy_stc_tcpwm_counter_config_t APP_COUNTER_config = 
{
	.period = 100000000,
	.clockPrescaler = CY_TCPWM_COUNTER_PRESCALER_DIVBY_1,
	.runMode = CY_TCPWM_COUNTER_CONTINUOUS,
	.countDirection = CY_TCPWM_COUNTER_COUNT_UP,
	.compareOrCapture = CY_TCPWM_COUNTER_MODE_CAPTURE,
	.compare0 = 16384,
	.compare1 = 16384,
	.enableCompareSwap = false,
	.interruptSources = CY_TCPWM_INT_NONE,
	.captureInputMode = APP_COUNTER_INPUT_DISABLED & 0x3U,
	.captureInput = CY_TCPWM_INPUT_0,
	.reloadInputMode = APP_COUNTER_INPUT_DISABLED & 0x3U,
	.reloadInput = CY_TCPWM_INPUT_0,
	.startInputMode = APP_COUNTER_INPUT_DISABLED & 0x3U,
	.startInput = CY_TCPWM_INPUT_0,
	.stopInputMode = APP_COUNTER_INPUT_DISABLED & 0x3U,
	.stopInput = CY_TCPWM_INPUT_0,
	.countInputMode = APP_COUNTER_INPUT_DISABLED & 0x3U,
	.countInput = CY_TCPWM_INPUT_1,
};
#if defined (CY_USING_HAL)
	const cyhal_resource_inst_t APP_COUNTER_obj = 
	{
		.type = CYHAL_RSC_TCPWM,
		.block_num = 0U,
		.channel_num = 1U,
	};
#endif //defined (CY_USING_HAL)
const cy_stc_tcpwm_pwm_config_t KIT_LED1_PWM_config = 
{
	.pwmMode = CY_TCPWM_PWM_MODE_PWM,
	.clockPrescaler = CY_TCPWM_PWM_PRESCALER_DIVBY_1,
	.pwmAlignment = CY_TCPWM_PWM_LEFT_ALIGN,
	.deadTimeClocks = 0,
	.runMode = CY_TCPWM_PWM_CONTINUOUS,
	.period0 = 50000,
	.period1 = 32768,
	.enablePeriodSwap = false,
	.compare0 = 25000,
	.compare1 = 16384,
	.enableCompareSwap = false,
	.interruptSources = CY_TCPWM_INT_NONE,
	.invertPWMOut = CY_TCPWM_PWM_INVERT_DISABLE,
	.invertPWMOutN = CY_TCPWM_PWM_INVERT_DISABLE,
	.killMode = CY_TCPWM_PWM_STOP_ON_KILL,
	.swapInputMode = KIT_LED1_PWM_INPUT_DISABLED & 0x3U,
	.swapInput = CY_TCPWM_INPUT_0,
	.reloadInputMode = KIT_LED1_PWM_INPUT_DISABLED & 0x3U,
	.reloadInput = CY_TCPWM_INPUT_0,
	.startInputMode = KIT_LED1_PWM_INPUT_DISABLED & 0x3U,
	.startInput = CY_TCPWM_INPUT_0,
	.killInputMode = KIT_LED1_PWM_INPUT_DISABLED & 0x3U,
	.killInput = CY_TCPWM_INPUT_0,
	.countInputMode = KIT_LED1_PWM_INPUT_DISABLED & 0x3U,
	.countInput = CY_TCPWM_INPUT_1,
};
#if defined (CY_USING_HAL)
	const cyhal_resource_inst_t KIT_LED1_PWM_obj = 
	{
		.type = CYHAL_RSC_TCPWM,
		.block_num = 0U,
		.channel_num = 3U,
	};
#endif //defined (CY_USING_HAL)


void init_cycfg_peripherals(void)
{
	Cy_SysClk_PeriphAssignDivider(PCLK_CSD_CLOCK, CY_SYSCLK_DIV_8_BIT, 0U);

	Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM0_CLOCKS1, CY_SYSCLK_DIV_8_BIT, 1U);
#if defined (CY_USING_HAL)
	cyhal_hwmgr_reserve(&APP_COUNTER_obj);
#endif //defined (CY_USING_HAL)

	Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM0_CLOCKS3, CY_SYSCLK_DIV_8_BIT, 1U);
#if defined (CY_USING_HAL)
	cyhal_hwmgr_reserve(&KIT_LED1_PWM_obj);
#endif //defined (CY_USING_HAL)
}
