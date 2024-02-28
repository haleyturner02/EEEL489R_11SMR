/**
 ****************************************************************************************
 *
 * @file user_custs1_impl.c
 *
 * @brief Peripheral project Custom1 Server implementation source code.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gpio.h"
#include "app_api.h"
#include "app.h"
#include "prf_utils.h"
#include "custs1.h"
#include "custs1_task.h"
#include "user_custs1_def.h"
#include "user_custs1_impl.h"
#include "user_peripheral.h"
#include "user_periph_setup.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
 
 //static const char REQUEST [] = "<<1>>";
 static const char REQUEST [] = "#";
 //static char received = 0x00;
 static char m1 [] = "<789>";								// Transmitting each char 3 times???
 static char measurement [] = "<XXX>";

ke_msg_id_t timer_used      __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint16_t indication_counter __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint16_t non_db_val_counter __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void user_svc1_ctrl_wr_ind_handler(ke_msg_id_t const msgid,
                                      struct custs1_val_write_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id){
    uint8_t val = 0;
    memcpy(&val, &param->value[0], param->length);
																				
		if(val == 0x01) {
			// Transmit measurement request via UART1 to Parent
			//uart_send(UART1, (uint8_t *)REQUEST, sizeof(REQUEST) - 1, UART_OP_BLOCKING);							// Send request

			//uart_receive(UART1, (uint8_t *)measurement, sizeof(measurement) - 1, UART_OP_INTR);		// Receive measurement
			// try UART_OP_INTR?
			
			uart_send(UART1, (uint8_t *)m1, sizeof(m1), UART_OP_BLOCKING);			// Send measurement back (to verify)
			val = 0;
		}

}
