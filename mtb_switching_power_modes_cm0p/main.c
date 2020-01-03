/***************************************************************************//**
* \file main.c
* \version 1.30
*
* \brief
* Objective:
*  This is a CM0+ main() template. It starts the Cortex-M4 and enters deep-sleep.
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


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function of core0. Initializes core1 (CM4) and waits forever.
*
* Parameters:
*  None
* 
* Return:
*  None
*
*******************************************************************************/
int main(void)
{
    /* enable global interrupts */
    __enable_irq();

    /* start up M4 core */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR);

    for (;;)
    {
        Cy_SysPm_DeepSleep( CY_SYSPM_WAIT_FOR_INTERRUPT );
    }
}

/* [] END OF FILE */
