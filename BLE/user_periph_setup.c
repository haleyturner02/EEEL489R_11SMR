/**
 ****************************************************************************************
 *
 * @file periph_setup.c
 *
 * @brief Peripherals initialization functions
 *
 * Copyright (c) 2012-2021 Renesas Electronics Corporation and/or its affiliates
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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "user_periph_setup.h"							// Peripheral Header File
#include "datasheet.h"								// Datasheet Header File

void GPIO_Reservations(void) {							// Reserve GPIO pins for UART1 & UART2

	RESERVE_GPIO(UART_TX, GPIO_PORT_0, GPIO_PIN_6, PID_UART2_TX);
   	RESERVE_GPIO(UART_RX, GPIO_PORT_0, GPIO_PIN_5, PID_UART2_RX);
	#if defined (CFG_PRINTF_UART2)
		RESERVE_GPIO(UART1_TX, UART1_TX_PORT, UART1_TX_PIN, PID_UART1_TX);
		RESERVE_GPIO(UART1_RX, UART1_RX_PORT, UART1_RX_PIN, PID_UART1_RX);
    		RESERVE_GPIO(UART2_TX, UART2_TX_PORT, UART2_TX_PIN, PID_UART2_TX);
		RESERVE_GPIO(UART2_RX, UART2_RX_PORT, UART2_RX_PIN, PID_UART2_RX);
	#endif
	
}

static void set_pad_functions(void) {						// Set GPIO pin configurations for DA14531MOD
	
	#if defined (__DA14531__)							
   	 	// Disallow spontaneous DA14531 SPI Flash wake-up
    		GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_3, OUTPUT, PID_GPIO, true);
	#endif

	GPIO_ConfigurePin(UART1_TX_PORT, UART1_TX_PIN, OUTPUT, PID_UART1_TX, false);	// UART1 used for communicating between DA14531MOD and MCU

	GPIO_ConfigurePin(UART1_RX_PORT, UART1_RX_PIN, INPUT, PID_UART0_RX, false);
	
	GPIO_ConfigurePin(UART2_TX_PORT, UART2_TX_PIN, OUTPUT, PID_UART2_TX, false);	// UART2 used for Programming/Debugging DA14531MOD

	GPIO_ConfigurePin(UART2_RX_PORT, UART2_RX_PIN, INPUT, PID_UART2_RX, false);	
	
}

// Configuration struct for UART1
static const uart_cfg1_t uart_cfg1 = {
    .baud_rate = UART1_BAUDRATE,
    .data_bits = UART1_DATABITS,
    .parity = UART1_PARITY,
    .stop_bits = UART1_STOPBITS,
    .auto_flow_control = UART1_AFCE,
    .use_fifo = UART1_FIFO,
    .tx_fifo_tr_lvl = UART1_TX_FIFO_LEVEL,
    .rx_fifo_tr_lvl = UART1_RX_FIFO_LEVEL,
    .intr_priority = 2,
};

// Configuration struct for UART2
static const uart_cfg2_t uart_cfg2 = {
    .baud_rate = UART2_BAUDRATE,
    .data_bits = UART2_DATABITS,
    .parity = UART2_PARITY,
    .stop_bits = UART2_STOPBITS,
    .auto_flow_control = UART2_AFCE,
    .use_fifo = UART2_FIFO,
    .tx_fifo_tr_lvl = UART2_TX_FIFO_LEVEL,
    .rx_fifo_tr_lvl = UART2_RX_FIFO_LEVEL,
    .intr_priority = 2,
};

void periph_init(void) {

   	uart_intialize(UART1, &uart_cfg1);
	
    	uart_initialize(UART2, &uart_cfg2);

    	set_pad_functions();

    	GPIO_set_pad_latch_en(true);
}
