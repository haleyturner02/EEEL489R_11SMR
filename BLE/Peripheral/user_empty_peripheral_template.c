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

const int CHAR_COUNT = 1;
static char buffer[CHAR_COUNT + 1];
static int receiving = 0;
static const char request[] = "###";

/****************************************************************************************
 * @brief UART print string function
 * @param[in] uart              Identifies which UART to use
 * @param[in] str               String to print
 ****************************************************************************************/
void printf_string(uart_t *uart, char *str);

/****************************************************************************************
 * FUNCTION DEFINITIONS
 ****************************************************************************************/

void printf_string(uart_t *uart, char *str){
    uart_send(uart, (uint8_t *) str, strlen(str), UART_OP_BLOCKING);
}

bool integer_value(){
	
	bool int_value;
	
	switch(buffer[0]){
				case 0x30:{
					int_value = true;
				} break;
				case 0x31:{
					int_value = true;
				} break;
				case 0x32:{
					int_value = true;
				} break;
				case 0x33:{
					int_value = true;
				} break;
				case 0x34:{
					int_value = true;
				} break;
				case 0x35:{
					int_value = true;
				} break;
				case 0x36:{
					int_value = true;
				} break;
				case 0x37:{
					int_value = true;
				} break;
				case 0x38:{
					int_value = true;
				} break;
				case 0x39:{
					int_value = true;
				} break;
				default:{
					int_value = false;
				}break;
	}
	
	return int_value;
	
}

void request_measurement() {
	
	//arch_printf("Request Measurement: %c\r\n", request[0]);
	//printf_string(UART1, "Request Measurement: ");
  uart_send(UART1, (uint8_t *)request, sizeof(request) - 1, UART_OP_BLOCKING);
	//printf_string(UART1, "\r\n");
	
}

void receive_measurement() {
	
	int m;	
	bool v;
	
	while(receiving < 9){
		uart_receive(UART1, (uint8_t *)buffer, CHAR_COUNT, UART_OP_BLOCKING);
		
		if(receiving == 0 && buffer[0] == 0x28){
			receiving = 1;
		} else if(receiving == 1) {	
			v = integer_value();			
			if(v){
				m = (((int) buffer[0]) - 48)*100;
				receiving = 2;
			}
		} else if(receiving == 2 && buffer[0] == 0x29){
			receiving = 3;
		} else if(receiving == 3 && buffer[0] == 0x5B){
			receiving = 4;
		} else if(receiving == 4){
			v = integer_value();			
			if(v){
				m = m + ((((int) buffer[0]) - 48)*10);
				receiving = 5;
			}
		} else if(receiving == 5 && buffer[0] == 0x5D){
			receiving = 6;
		} else if(receiving == 6 && buffer[0] == 0x7B){
			receiving = 7;
		} else if(receiving == 7) {
			v = integer_value();			
			if(v){
				m = m + (((int) buffer[0]) - 48);
				receiving = 8;
			}
		} else if(receiving == 8 && buffer[0] == 0x7D){
			receiving = 9;
		}
			
	}
	receiving = 0;
	measurement = (uint16_t) m;	
	//printf_string(UART1, "\r\n");
	
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
		//arch_puts("Connected\r\n");
		//printf_string(UART1, "Connected\r\n");
		measurement = 0x00;
		request_measurement();
}

void user_on_disconnect( struct gapc_disconnect_ind const *param ){
    default_app_on_disconnect(param);
		//arch_puts("Disconnected\r\n");
		//printf_string(UART1, "Disconnected\r\n");
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
										if(msg_param->value[0] == 0x01) {
											receive_measurement();
										}
                    break;
								
								
								case SVC1_IDX_MEASUREMENT_VAL:
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
