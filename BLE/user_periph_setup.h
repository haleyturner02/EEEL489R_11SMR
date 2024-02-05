/**
 ****************************************************************************************
 *
 * @file user_periph_setup.h
 *
 * @brief Peripherals setup header file.
 *
 * Copyright (c) 2015-2021 Renesas Electronics Corporation and/or its affiliates
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 ****************************************************************************************
 */

#ifndef _USER_PERIPH_SETUP_H_
#define _USER_PERIPH_SETUP_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "uart.h"
#include "gpio.h"

/****************************************************************************************/
/* UART1 & UART2 configuration to print messages                                        */
/****************************************************************************************/
#define UART                        UART2

// Definitions for UART1 & UART2 Ports and Pins
#if defined (__DA14531__)                                  
    #define UART1_TX_PORT           GPIO_PORT_X
    #define UART1_TX_PIN            GPIO_PIN_X
    #define UART1_TX_PORT           GPIO_PORT_X
    #define UART1_TX_PIN            GPIO_PIN_X
    #define UART2_TX_PORT           GPIO_PORT_0
    #define UART2_TX_PIN            GPIO_PIN_1
    #define UART2_RX_PORT           GPIO_PORT_X
    #define UART2_RX_PIN            GPIO_PIN_X
#endif

// Define UART1 Settings
#define UART1_BAUDRATE              UART_BAUDRATE_115200
#define UART1_DATABITS              UART_DATABITS_8
#define UART1_PARITY                UART_PARITY_NONE
#define UART1_STOPBITS              UART_STOPBITS_1
#define UART1_AFCE                  UART_AFCE_DIS
#define UART1_FIFO                  UART_FIFO_EN
#define UART1_TX_FIFO_LEVEL         UART_TX_FIFO_LEVEL_0
#define UART1_RX_FIFO_LEVEL         UART_RX_FIFO_LEVEL_0

// Define UART2 Settings
#define UART2_BAUDRATE              UART_BAUDRATE_115200
#define UART2_DATABITS              UART_DATABITS_8
#define UART2_PARITY                UART_PARITY_NONE
#define UART2_STOPBITS              UART_STOPBITS_1
#define UART2_AFCE                  UART_AFCE_DIS
#define UART2_FIFO                  UART_FIFO_EN
#define UART2_TX_FIFO_LEVEL         UART_TX_FIFO_LEVEL_0
#define UART2_RX_FIFO_LEVEL         UART_RX_FIFO_LEVEL_0

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief   Initializes application's peripherals and pins.
 * @return  void
 ****************************************************************************************
 */
void periph_init(void);

/**
****************************************************************************************
* @brief	Deinitializes application's peripherals and pins. For future use.
* @return	void
****************************************************************************************
*/
void periph_deinit(void);

#endif // _USER_PERIPH_SETUP_H_
