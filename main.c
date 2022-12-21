/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the RDK3_I2C_Scanner
*              Application for ModusToolbox.
*
* Related Document: See README.md
*
*
*  Created on: 2022-12-21
*  Company: Rutronik Elektronische Bauelemente GmbH
*  Address: Jonavos g. 30, Kaunas 44262, Lithuania
*  Author: GDR
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*
* Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
* including the software is for testing purposes only and,
* because it has limited functions and limited resilience, is not suitable
* for permanent use under real conditions. If the evaluation board is
* nevertheless used under real conditions, this is done at one’s responsibility;
* any liability of Rutronik is insofar excluded
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define I2C_TIMEOUT_MS	10

void handle_error(void);

int main(void)
{
    cy_rslt_t result;
    cy_en_scb_i2c_status_t ping;
    uint32_t address;
    uint32_t counter = 0;
    _Bool dev_online = false;
    cyhal_i2c_t I2C_scb3;
    cyhal_i2c_cfg_t I2C_cfg;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
    	handle_error();
    }

    /*Enable debug output via KitProg UART*/
    result = cy_retarget_io_init( KITPROG_TX, KITPROG_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
    printf("\x1b[2J\x1b[;H");

    /*Initialize LEDs*/
    result = cyhal_gpio_init( LED1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}
    result = cyhal_gpio_init( LED2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}

    /*Enable interrupts*/
    __enable_irq();

    /*I2C Peripheral Configuration*/
    I2C_cfg.is_slave = false;
    I2C_cfg.address = 0;
    I2C_cfg.frequencyhal_hz = 100000UL;
    result = cyhal_i2c_init(&I2C_scb3, ARDU_SDA, ARDU_SCL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
    	handle_error();
    }
    result = cyhal_i2c_configure(&I2C_scb3, &I2C_cfg);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
    printf("I2C peripheral ready.\r\n");
    CyDelay(1000);

    /*Look for the responsive I2C devices*/
    for (;;)
    {
    	printf("\x1b[2J\x1b[;H");
    	printf("\"RDK3 I2C Scanner\" is running.\r\n");
    	printf("Full scan completed %u times.\r\n", (unsigned int)counter);
    	for(address = 1; address < 127; address++)
    	{
    		ping = Cy_SCB_I2C_MasterSendStart(I2C_scb3.base, address, CY_SCB_I2C_WRITE_XFER, I2C_TIMEOUT_MS, &I2C_scb3.context);
    		Cy_SCB_I2C_MasterSendStop(I2C_scb3.base, I2C_TIMEOUT_MS, &I2C_scb3.context);
    		if(ping == CY_SCB_I2C_SUCCESS)
    		{
    			/*Left shift the address once and print it*/
    			printf("Responsive I2C address 8-bit: 0x%X, 7-bit: 0x%X\r\n", (unsigned int)(address << 1), (unsigned int)address);

    			/*Device found flag*/
    			dev_online = true;
    		}
    		/*Check for known protocol specific devices*/
    		else
    		{
    			/*CY8CMBR3xxx controller wake up*/
    			if(address == 0x37)
    			{
    				for(uint8_t i = 0; i < 3; i++)
    				{
    		    		ping = Cy_SCB_I2C_MasterSendStart(I2C_scb3.base, address, CY_SCB_I2C_WRITE_XFER, I2C_TIMEOUT_MS, &I2C_scb3.context);
    		    		Cy_SCB_I2C_MasterSendStop(I2C_scb3.base, I2C_TIMEOUT_MS, &I2C_scb3.context);
    		    		if(ping == CY_SCB_I2C_SUCCESS)
    		    		{
    		    			/*Left shift the address once and print it*/
    		    			printf("Responsive I2C address 8-bit: 0x%X, 7-bit: 0x%X\r\n", (unsigned int)(address << 1), (unsigned int)address);

    		    			/*Device found flag*/
    		    			dev_online = true;
    		    			break;
    		    		}
    				}
    			}
    		}
    	}

    	if(dev_online)
    	{
    		cyhal_gpio_write(LED1, false);
    		cyhal_gpio_write(LED2, true);
    	}
    	else
    	{
    		cyhal_gpio_write(LED1, true);
    		cyhal_gpio_write(LED2, false);
    	}
    	dev_online = false;
    	CyDelay(1000);
    	counter++;
    	printf("\r\n");
    }
}

void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}
/* [] END OF FILE */
