/****************************************************************************************
 *
 * @file user_empty_peripheral_template.c
 *
 * @brief Empty peripheral template project source code.
 *
 * Copyright (C) 2012-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************/

/****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************/
#include "rwip_config.h"             // SW configuration
#include "arch_console.h"

/****************************************************************************************
 * INCLUDE FILES
 ****************************************************************************************/

#include "app_api.h"
#include "user_empty_peripheral_template.h"

#include "gap.h"
#include "app_easy_timer.h"
#include "user_custs1_impl.h"
#include "user_custs1_def.h"
#include "co_bt.h"

// UART definitions/configurations
#include "uart.h"
#include "uart_utils.h"

uint8_t app_connection_idx                      __SECTION_ZERO("retention_mem_area0");
timer_hnd app_adv_data_update_timer_used        __SECTION_ZERO("retention_mem_area0");
timer_hnd app_param_update_request_timer_used   __SECTION_ZERO("retention_mem_area0");

// Index of manufacturer data in advertising data or scan response data (when MSB is 1)
uint8_t mnf_data_index                          __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t stored_adv_data_len                     __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t stored_scan_rsp_data_len                __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t stored_adv_data[ADV_DATA_LEN]           __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t stored_scan_rsp_data[SCAN_RSP_DATA_LEN] __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

// Memory retention for measurement
uint16_t measurement __SECTION_ZERO("retention_mem_area0");

/****************************************************************************************
 * FUNCTION DEFINITIONS
 ****************************************************************************************/

void request_measurement() {
	
	static const char OUTPUT_STRING[] = "#";
		
  uart_send(UART1, (uint8_t *)OUTPUT_STRING, sizeof(OUTPUT_STRING) - 1, UART_OP_BLOCKING); 
	
	// Need to receive measurement via UART
	// received = 0;
	// while(received == 0) {
	// 		uart_receive(UART1, &m, sizeof(m), UART_OP_BLOCKING);
	// 	

	// Send 1 char at a time, respond?
	
	
	arch_puts("Received Measurement: \r\n");
	
	// Need to set measurement
	
	measurement = 0x01;
	
	
}

void user_app_init() {
	
		app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;

   // Initialize Manufacturer Specific Data
   // mnf_data_init();

    // Initialize Advertising and Scan Response Data
    memcpy(stored_adv_data, USER_ADVERTISE_DATA, USER_ADVERTISE_DATA_LEN);
    stored_adv_data_len = USER_ADVERTISE_DATA_LEN;
    memcpy(stored_scan_rsp_data, USER_ADVERTISE_SCAN_RESPONSE_DATA, USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN);
    stored_scan_rsp_data_len = USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN;

    default_app_on_init();
	
}

void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param){
    default_app_on_connection(connection_idx, param);
		arch_puts("Connected\r\n");
		//arch_printf("%s: conn_idx=%04x\r\n", __func__, connection_idx);
		measurement = 0xFFFF;
		request_measurement();
	
}

void user_on_disconnect( struct gapc_disconnect_ind const *param ){
    default_app_on_disconnect(param);
		arch_puts("Disconnected\r\n");
}

void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id){													
	
    switch(msgid){
	
        case CUSTS1_VAL_WRITE_IND:{
					
            struct custs1_val_write_ind const *msg_param = (struct custs1_val_write_ind const *)(param);

						struct custs1_val_set_req const *msg_param1 = (struct custs1_val_set_req const *)(param);
					
						//arch_printf("Write Value: %u\r\n", (unsigned long) msg_param->value[0]);
					
            switch (msg_param->handle){
							
                case SVC1_IDX_CONTROL_POINT_VAL:																					// Call when Control Point is asserted
                    user_svc1_ctrl_wr_ind_handler(msgid, msg_param, dest_id, src_id);
                    break;
								
								case SVC1_IDX_MEASUREMENT_VAL:
										//user_svc1_measurement_wr_ind_handler(msgid, msg_param, dest_id, src_id, measurement);
										user_svc1_measurement_update_ind_handler(msgid, msg_param1, dest_id, src_id, measurement);
								
                    break;
								
                default:
                    break;
            }
        } break;

        default:
            break;
    }
}

/// @} APP
