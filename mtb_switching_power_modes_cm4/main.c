/***************************************************************************//**
* \file main.c
* \version 1.30
*
* \brief
* Objective:
*    This example demonstrates how to transition PSoC 6 among the following
*    power modes - Low Power Active, Low Power Sleep, Ultra Low Power Active,
*    Ultra Low Power Sleep and Deep Sleep.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cycfg.h"


/*******************************************************************************
* Constants
*******************************************************************************/
typedef enum
{
    SWITCH_NO_EVENT     = 0u,
    SWITCH_QUICK_PRESS  = 1u,
    SWITCH_SHORT_PRESS  = 2u,
    SWITCH_LONG_PRESS   = 3u,
} SwitchEvent;

/* Constants to define LONG and SHORT presses on KIT_BTN1 */
#define QUICK_PRESS_COUNT     5000u /* < 200 milliseconds */
#define SHORT_PRESS_COUNT   100000u /* ~ 1 second */
#define LONG_PRESS_COUNT    500000u /* > 2 seconds */

/* PWM LED period constants (in cycles) */
#define LED_BLINK_FAST      50000u
#define LED_BLINK_SLOW      100000u
#define LED_DIM_CONTROL     100u

/* Time out for changing the FLL (in cycles) */
#define FLL_CLOCK_TIMEOUT   200000u

/* Clock frequency constants (in Hz) */
#define FLL_CLOCK_50_MHZ    50000000u
#define FLL_CLOCK_100_MHZ   100000000u
#define IMO_CLOCK           8000000u

/* Change the blinking pattern of the LED */
#define PWM_LED_ACTION(x)   Cy_TCPWM_PWM_SetPeriod0(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM, x); \
                            Cy_TCPWM_PWM_SetCompare0(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM, x/2); \
                            Cy_TCPWM_PWM_SetCounter(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM, 0);

/* Changes the brightness of the LED by changing the duty cycle */
#define PWM_LED_DIM(x)      Cy_TCPWM_PWM_SetPeriod0(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM, LED_DIM_CONTROL); \
                            Cy_TCPWM_PWM_SetCompare0(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM, x); \
                            Cy_TCPWM_PWM_SetCounter(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM, 0);

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
/* Auxiliary Prototype functions */
SwitchEvent GetSwitchEvent(void);
void WakeupInterruptHandler(void);

/* Callback Prototypes */
cy_en_syspm_status_t TCPWM_SleepCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode);
cy_en_syspm_status_t TCPWM_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode);
cy_en_syspm_status_t TCPWM_EnterUltraLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode);
cy_en_syspm_status_t TCPWM_ExitUltraLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode);
cy_en_syspm_status_t Clock_EnterUltraLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode);
cy_en_syspm_status_t Clock_ExitUltraLowPowerCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode);


/*******************************************************************************
* Function Name: main
****************************************************************************//**
*
*  Initialization:
*  - Register sleep callbacks.
*  - Initialize the PWM block that controls the LED brightness.
*  Do forever loop:
*  - Check if KIT_BTN1 was pressed and for how long.
*  - If quickly pressed, swap from LP to ULP (vice-versa).
*  - If short pressed, go to sleep.
*  - If long pressed, go to deep sleep.
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* SysPm callback params */
    cy_stc_syspm_callback_params_t callbackParams = {
        /*.base       =*/ NULL,
        /*.context    =*/ NULL
    };

    /* Wake-up Interrupt pin config structure (P0[4]) */
    cy_stc_sysint_t WakeupIsrPin =
    {
        /* Wake-up pin is located in Port 0 */
        .intrSrc = ioss_interrupts_gpio_0_IRQn,
        .intrPriority = 0,
    };

    /* Callback declaration for Power Modes */
    cy_stc_syspm_callback_t PwmSleepCb = {TCPWM_SleepCallback,      /* Callback function */
                                          CY_SYSPM_SLEEP,           /* Callback type */
                                          CY_SYSPM_SKIP_CHECK_READY |
                                          CY_SYSPM_SKIP_CHECK_FAIL, /* Skip mode */
                                          &callbackParams,          /* Callback params */
                                          NULL, NULL};              /* For internal usage */
    cy_stc_syspm_callback_t PwmDeepSleepCb = {TCPWM_DeepSleepCallback,  /* Callback function */
                                              CY_SYSPM_DEEPSLEEP,       /* Callback type */
                                              CY_SYSPM_SKIP_CHECK_READY |
                                              CY_SYSPM_SKIP_CHECK_FAIL, /* Skip mode */
                                              &callbackParams,          /* Callback params */
                                              NULL, NULL};              /* For internal usage */
    cy_stc_syspm_callback_t PwmEnterUlpCb = {TCPWM_EnterUltraLowPowerCallback, /* Callback function */
                                             CY_SYSPM_ULP,                     /* Callback type */
                                             CY_SYSPM_SKIP_CHECK_READY |
                                             CY_SYSPM_SKIP_CHECK_FAIL |
                                             CY_SYSPM_SKIP_BEFORE_TRANSITION,  /* Skip mode */
                                             &callbackParams,                  /* Callback params */
                                             NULL, NULL};                      /* For internal usage */
    cy_stc_syspm_callback_t PwmExitUlpCb = {TCPWM_ExitUltraLowPowerCallback, /* Callback function */
                                            CY_SYSPM_LP,                     /* Callback type */
                                            CY_SYSPM_SKIP_CHECK_READY |
                                            CY_SYSPM_SKIP_CHECK_FAIL |
                                            CY_SYSPM_SKIP_BEFORE_TRANSITION, /* Skip mode */
                                            &callbackParams,                 /* Callback params */
                                            NULL, NULL};                     /* For internal usage */
    cy_stc_syspm_callback_t ClkEnterUlpCb = {Clock_EnterUltraLowPowerCallback, /* Callback function */
                                             CY_SYSPM_ULP,                     /* Callback type */
                                             CY_SYSPM_SKIP_CHECK_READY |
                                             CY_SYSPM_SKIP_CHECK_FAIL |
                                             CY_SYSPM_SKIP_AFTER_TRANSITION,   /* Skip mode */
                                             &callbackParams,                  /* Callback params */
                                             NULL, NULL};
    cy_stc_syspm_callback_t ClkExitUlpCb = {Clock_ExitUltraLowPowerCallback, /* Callback function */
                                            CY_SYSPM_LP,                     /* Callback type */
                                            CY_SYSPM_SKIP_CHECK_READY |
                                            CY_SYSPM_SKIP_CHECK_FAIL |
                                            CY_SYSPM_SKIP_BEFORE_TRANSITION, /* Skip mode */
                                            &callbackParams,                 /* Callback params */
                                            NULL, NULL};                     /* For internal usage */

    /* enable interrupts */
    __enable_irq();

    /* Initialize the Wake-up Interrupt */
    Cy_SysInt_Init(&WakeupIsrPin, WakeupInterruptHandler);

    /* Configure pin interrupt */
    Cy_GPIO_SetInterruptMask(KIT_BTN1_PORT, KIT_BTN1_NUM, 0x01);

    /* Enable ISR to wake up pin */
    NVIC_EnableIRQ(WakeupIsrPin.intrSrc);

    /* Register SysPm callbacks */
    Cy_SysPm_RegisterCallback(&PwmSleepCb);
    Cy_SysPm_RegisterCallback(&PwmDeepSleepCb);
    Cy_SysPm_RegisterCallback(&PwmEnterUlpCb);
    Cy_SysPm_RegisterCallback(&PwmExitUlpCb);
    Cy_SysPm_RegisterCallback(&ClkEnterUlpCb);
    Cy_SysPm_RegisterCallback(&ClkExitUlpCb);

    /* Initialize the TCPWM blocks */
    Cy_TCPWM_PWM_Init(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM, &KIT_LED1_PWM_config);
    Cy_TCPWM_Counter_Init(APP_COUNTER_HW, APP_COUNTER_NUM, &APP_COUNTER_config);

    /* Enable the PWM LED */
    Cy_TCPWM_PWM_Enable(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM);
    Cy_TCPWM_TriggerStart(KIT_LED1_PWM_HW, KIT_LED1_PWM_MASK);

    for (;;)
    {
        switch (GetSwitchEvent())
        {
            case SWITCH_QUICK_PRESS:
                /* Check if the device is in System ULP mode */
                if (Cy_SysPm_IsSystemUlp())
                {
                    /* Switch to System LP mode */
                    Cy_SysPm_SystemEnterLp();
                }
                else
                {
                    /* Switch to ULP mode */
                    Cy_SysPm_SystemEnterUlp();
                }
                break;

            case SWITCH_SHORT_PRESS:
                /* Go to sleep */
                Cy_SysPm_CpuEnterSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
                /* Wait a bit to avoid glitches in the button press */
                Cy_SysLib_Delay(250);
                break;

            case SWITCH_LONG_PRESS:
                /* Go to deep sleep */
                Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
                /* Wait a bit to avoid glitches in the button press */
                Cy_SysLib_Delay(250);
                break;

            default:
                break;
        }
    }
}

/*******************************************************************************
* Function Name: GetSwitchEvent
****************************************************************************//**
*
* Returns how the KIT_BTN1 was pressed:
* - SWITCH_NO_EVENT: No press or very quick press
* - SWITCH_SHORT_PRESS: Short press was detected
* - SWITCH_LONG_PRESS: Long press was detected
*
*******************************************************************************/
SwitchEvent GetSwitchEvent(void)
{
    unsigned int pressCount;
    SwitchEvent event = SWITCH_NO_EVENT;

    /* Check if KIT_BTN1 is pressed */
    if (0u == Cy_GPIO_Read(KIT_BTN1_PORT, KIT_BTN1_NUM))
    {
        /* Check if the counter is not running */
        if (0u == ( Cy_TCPWM_Counter_GetStatus(APP_COUNTER_HW, APP_COUNTER_NUM) &
                   CY_TCPWM_COUNTER_STATUS_COUNTER_RUNNING))
        {
            /* Enable and trigger the counter */
            Cy_TCPWM_Counter_SetCounter(APP_COUNTER_HW, APP_COUNTER_NUM, 0u);
            Cy_TCPWM_Counter_Enable(APP_COUNTER_HW, APP_COUNTER_NUM);
            Cy_TCPWM_TriggerStart(APP_COUNTER_HW, APP_COUNTER_MASK);
        }
    }
    else
    {
        /* If button not pressed, check the counter value */
        pressCount = Cy_TCPWM_Counter_GetCounter(APP_COUNTER_HW, APP_COUNTER_NUM);

        /* Check if KIT_BTN1 was pressed for a long time */
        if (pressCount > LONG_PRESS_COUNT)
        {
            event = SWITCH_LONG_PRESS;
        }
        /* Check if KIT_BTN1 was pressed for a short time */
        else if (pressCount > SHORT_PRESS_COUNT)
        {
            event = SWITCH_SHORT_PRESS;
        }
        else if (pressCount > QUICK_PRESS_COUNT)
        {
            event = SWITCH_QUICK_PRESS;
        }

        /* Disable the switch counter */
        Cy_TCPWM_Counter_Disable(APP_COUNTER_HW, APP_COUNTER_NUM);

        /* Reset the switch counter */
        Cy_TCPWM_Counter_SetCounter(APP_COUNTER_HW, APP_COUNTER_NUM, 0u);

        /* Debounce the USER button */
        Cy_SysLib_Delay(10);
    }

    return event;
}

/*******************************************************************************
* Function Name: TCPWM_SleepCallback
****************************************************************************//**
*
* Sleep callback implementation. It changes the LED behavior based on the
* System Mode.
* - LP Mode CPU Sleep  : LED is turned ON
* - ULP Mode CPU Sleep : LED is dimmed.
* Note that the LED brightness is controlled using the PWM block.
*
*******************************************************************************/
cy_en_syspm_status_t TCPWM_SleepCallback(
    cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;

    switch (mode)
    {
        case CY_SYSPM_BEFORE_TRANSITION:

            /* Check if the device is in System ULP mode */
            if (Cy_SysPm_IsSystemUlp())
            {
                /* Before going to ULP sleep mode, dim the LED (10%) */
                PWM_LED_DIM(10);
            }
            else
            {
                /* Before going to LP sleep mode, turn on the LED (100%) */
                PWM_LED_DIM(100);
            }

            /* Disable switch Counter */
            Cy_TCPWM_Counter_Disable(APP_COUNTER_HW, APP_COUNTER_NUM);

            retVal = CY_SYSPM_SUCCESS;
            break;

        case CY_SYSPM_AFTER_TRANSITION:

            /* Check if the device is in System ULP mode */
            if (Cy_SysPm_IsSystemUlp())
            {
                /* After waking up, set the slow blink pattern */
                PWM_LED_ACTION(LED_BLINK_SLOW);
            }
            else
            {
                /* After waking up, set the fast blink pattern */
                PWM_LED_ACTION(LED_BLINK_FAST);
            }

            retVal = CY_SYSPM_SUCCESS;
            break;

        default:
            /* Don't do anything in the other modes */
            retVal = CY_SYSPM_SUCCESS;
            break;
    }

    return retVal;
}

/*******************************************************************************
* Function Name: TCPWM_DeepSleepCallback
****************************************************************************//**
*
* Deep Sleep callback implementation. It turns the LED off before going to deep
* sleep power mode. After waking up, it sets the LED to blink.
* Note that the PWM block needs to be re-enabled after waking up, since the
* clock feeding the PWM is disabled in deep sleep.
*
*******************************************************************************/
cy_en_syspm_status_t TCPWM_DeepSleepCallback(
    cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;

    switch (mode)
    {
        case CY_SYSPM_BEFORE_TRANSITION:
            /* Before going to sleep mode, turn off the LEDs */
            Cy_TCPWM_PWM_Disable(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM);

            /* Disable the switch counter */
            Cy_TCPWM_Counter_Disable(APP_COUNTER_HW, APP_COUNTER_NUM);

            retVal = CY_SYSPM_SUCCESS;
            break;

        case CY_SYSPM_AFTER_TRANSITION:
            /* Re-enable PWM */
            Cy_TCPWM_PWM_Enable(KIT_LED1_PWM_HW, KIT_LED1_PWM_NUM);
            Cy_TCPWM_TriggerStart(KIT_LED1_PWM_HW, KIT_LED1_PWM_MASK);

            /* Check if the device is in System ULP mode */
            if (Cy_SysPm_IsSystemUlp())
            {
                /* After waking up, set the slow blink pattern */
                PWM_LED_ACTION(LED_BLINK_SLOW);
            }
            else
            {
                /* After waking up, set the fast blink pattern */
                PWM_LED_ACTION(LED_BLINK_FAST);
            }

            retVal = CY_SYSPM_SUCCESS;
            break;

        default:
            /* Don't do anything in the other modes */
            retVal = CY_SYSPM_SUCCESS;
            break;
    }

    return retVal;
}

/*******************************************************************************
* Function Name: TCPWM_EnterUltraLowPowerCallback
****************************************************************************//**
*
* Enter System ULP Mode callback implementation. It changes the LED blinking
* pattern.
*
*******************************************************************************/
cy_en_syspm_status_t TCPWM_EnterUltraLowPowerCallback(
        cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;

    switch (mode)
    {
        case CY_SYSPM_AFTER_TRANSITION:
            /* Set slow blink LED pattern  */
            PWM_LED_ACTION(LED_BLINK_SLOW);

            retVal = CY_SYSPM_SUCCESS;
            break;

        default:
            /* Don't do anything in the other modes */
            retVal = CY_SYSPM_SUCCESS;
            break;
    }

    return retVal;
}

/*******************************************************************************
* Function Name: TCPWM_ExitUltraLowPowerCallback
****************************************************************************//**
*
* Exit System ULP Mode callback implementation. It changes the LED blinking
* pattern.
*
*******************************************************************************/
cy_en_syspm_status_t TCPWM_ExitUltraLowPowerCallback(
        cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;

    switch (mode)
    {
        case CY_SYSPM_AFTER_TRANSITION:
            /* Set fast blink LED pattern  */
            PWM_LED_ACTION(LED_BLINK_FAST);

            retVal = CY_SYSPM_SUCCESS;
            break;

        default:
            /* Don't do anything in the other modes */
            retVal = CY_SYSPM_SUCCESS;
            break;
    }

    return retVal;
}

/*******************************************************************************
* Function Name: Clock_EnterUltraLowPowerCallback
****************************************************************************//**
*
* Enter Low Power Mode callback implementation. It reduces the FLL frequency by
* half.
*
*******************************************************************************/
cy_en_syspm_status_t Clock_EnterUltraLowPowerCallback(
        cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;

    switch (mode)
    {
        case CY_SYSPM_BEFORE_TRANSITION:
            /* Disable the FLL */
            Cy_SysClk_FllDisable();

            /* Reconfigure the FLL to be half of the original frequency */
            Cy_SysClk_FllConfigure(IMO_CLOCK, FLL_CLOCK_50_MHZ, CY_SYSCLK_FLLPLL_OUTPUT_AUTO);

            /* Re-enable the FLL */
            Cy_SysClk_FllEnable(FLL_CLOCK_TIMEOUT);

            /* Set Peri Clock Divider to 0u, to keep at 50 MHz */
            Cy_SysClk_ClkPeriSetDivider(0u);

            retVal = CY_SYSPM_SUCCESS;
            break;

        default:
            /* Don't do anything in the other modes */
            retVal = CY_SYSPM_SUCCESS;
            break;
    }

    return retVal;
}

/*******************************************************************************
* Function Name: Clock_ExitUltraLowPowerCallback
****************************************************************************//**
*
* Exit Low Power Mode callback implementation. It sets the original FLL
* frequency for the device.
*
*******************************************************************************/
cy_en_syspm_status_t Clock_ExitUltraLowPowerCallback(
        cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;

    switch (mode)
    {
        case CY_SYSPM_AFTER_TRANSITION:
            /* Set Peri Clock Divider to 1u, to keep at 50 MHz */
            Cy_SysClk_ClkPeriSetDivider(1u);

            /* Disable the FLL */
            Cy_SysClk_FllDisable();

            /* Reconfigure the FLL to be the original frequency */
            Cy_SysClk_FllConfigure(IMO_CLOCK, FLL_CLOCK_100_MHZ, CY_SYSCLK_FLLPLL_OUTPUT_AUTO);

            /* Re-enable the FLL */
            Cy_SysClk_FllEnable(FLL_CLOCK_TIMEOUT);

            retVal = CY_SYSPM_SUCCESS;
            break;

        default:
            /* Don't do anything in the other modes */
            retVal = CY_SYSPM_SUCCESS;
            break;
    }

    return retVal;
}

/*******************************************************************************
* Function Name: WakeupInterruptHandler
****************************************************************************//**
*
* Wake-up pin interrupt handler. Clear the interrupt only.
*
*******************************************************************************/
void WakeupInterruptHandler(void)
{
    /* Clear any pending interrupt */
    if (0u != Cy_GPIO_GetInterruptStatusMasked(KIT_BTN1_PORT, KIT_BTN1_NUM))
    {
        Cy_GPIO_ClearInterrupt(KIT_BTN1_PORT, KIT_BTN1_NUM);
    }
}

/* [] END OF FILE */
