/****************************************************************************************
 *
 * @file user_custs1_impl.c
 *
 * @brief Peripheral project Custom1 Server implementation source code.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************/

/****************************************************************************************
 * INCLUDE FILES
 ****************************************************************************************/

#include "gpio.h"
#include "app_api.h"
#include "app.h"
#include "prf_utils.h"
#include "custs1.h"
#include "custs1_task.h"
#include "user_custs1_def.h"
#include "user_custs1_impl.h"
#include "user_empty_peripheral_template.h"
#include "user_periph_setup.h"
#include "arch_console.h"

/****************************************************************************************
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************/
 
 //static const char REQUEST [] = "#";				// Measurement request indicator 
 //static char m1 [] = "<456>";								// Transmitting each char 3 times???
 //static char measurement [] = "<789>";			// Measurement storage

ke_msg_id_t timer_used      __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint16_t indication_counter __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint16_t non_db_val_counter __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

/****************************************************************************************
 * FUNCTION DEFINITIONS
 ****************************************************************************************/
 
 // Reading from register
 // memcpy(sys_addr, (void *)(em_addr + EM_BASE_ADDR), len);
 
 // Writing to register
 // memset((void *)(em_addr + REG_COMMON_EM_ET_BASE_ADDR), value, len);

void user_svc1_ctrl_wr_ind_handler(ke_msg_id_t const msgid,
                                      struct custs1_val_write_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id){
    uint8_t val = 0;
    memcpy(&val, &param->value[0], param->length);																						// Get value written to Control Point
								
		arch_printf("Received Value: %d\r\n", val);
																				
		/*if(val == 0x01) {																																					// If '1', request measurement from MCU
			uart_send(UART1, (uint8_t *)REQUEST, sizeof(REQUEST) - 1, UART_OP_BLOCKING);							// Send request to MCU

			uart_receive(UART1, (uint8_t *)measurement, sizeof(measurement) - 1, UART_OP_INTR);		// Receive measurement from MCU
			// try UART_OP_INTR?
			
			uart_send(UART1, (uint8_t *)measurement, sizeof(measurement) - 1, UART_OP_BLOCKING);			// Send measurement back (to verify)
			//val = 0;																																								// Clear value
		}*/

}
																			
void user_svc1_led_wr_ind_handler(ke_msg_id_t const msgid, 
																	struct custs1_val_write_ind const *param,
																	ke_task_id_t const dest_id,
																	ke_task_id_t const src_id){

		uint8_t val = 0;
		memcpy(&val, &param->value[0], param->length);
		
		if(val == CUSTS1_LED_ON) {
			arch_puts("LED On\r\n");
		} else if (val == CUSTS1_LED_OFF) {
			arch_puts("LED Off\r\n");
		}
																		
}
																			
void user_svc2_measurement_ind_handler(ke_msg_id_t const msgid,
                                   struct custs1_val_write_ind const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id){

		uint16_t val = 0x31;
		memset(&param, val, sizeof(val));
																					
																				
};
																	 
void user_svc1_long_val_cfg_ind_handler(ke_msg_id_t const msgid,
                                           struct custs1_val_write_ind const *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id){
    // Generate indication when the central subscribes to it
    if (param->value[0]){
			
				arch_puts("Notifications\r\n");
			
        uint8_t conidx = KE_IDX_GET(src_id);

        struct custs1_val_ind_req* req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_IND_REQ,
                                                          prf_get_task_from_id(TASK_ID_CUSTS1),
                                                          TASK_APP,
                                                          custs1_val_ind_req,
                                                          sizeof(indication_counter));

        req->conidx = app_env[conidx].conidx;
        req->handle = SVC1_IDX_INDICATEABLE_VAL;
        req->length = sizeof(indication_counter);
        req->value[0] = (indication_counter >> 8) & 0xFF;
        req->value[1] = indication_counter & 0xFF;

        indication_counter++;

        ke_msg_send(req);
    }
}
																					 
void user_svc1_long_val_wr_ind_handler(ke_msg_id_t const msgid,
                                          struct custs1_val_write_ind const *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id){
			
		uint8_t val;
		memcpy(&val, &param->value[0], param->length);																			
	  arch_printf("Set Value: %u\r\n", (unsigned long) val);
																						
}
	
																					 
