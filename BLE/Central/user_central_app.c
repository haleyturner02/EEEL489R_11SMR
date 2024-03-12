// Parent/Central Main Application

/****************************************************************************************
 *
 * @file user_central_app.c
 *
 * @brief Cental application project source code.
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
#include "user_central_app.h"
#include "user_ble_gap.h"
#include "user_ble_common.h"
#include "user_ble_gatt.h"
#include "user_config.h"
#include "user_central_config.h"
#include "gap.h"
#include "ke_task.h"
#include "wkupct_quadec.h"
#include "user_periph_setup.h"
#include "spi_flash.h"

// UART definitions/configurations
#include "uart.h"
#include "uart_utils.h"

#define SCAN_INTVL_MS		(50)
#define SCAN_WINDOW_MS	(50)

typedef struct{
	struct bd_addr addr;
}connecting_device_t;

typedef struct{
	periph_device_t 	periph_devices[CFG_MAX_CONNECTIONS];
	
	bool 							connect_to_periph;
	struct bd_addr 		connect_to_addr;
	uint8_t 					connect_to_addr_type;
	uint8_t						num_connections;
	timer_hnd					connection_timer;
	
}central_app_env_t;

central_app_env_t central_app_env;

/****************************************************************************************
 * FUNCTION DEFINITIONS
 ****************************************************************************************/

static const char *format_uuid(att_uuid_t *uuid){
        static char buf[37];

        if (uuid->type == ATT_UUID_16) {
                sprintf(buf, "0x%04x", uuid->uuid.uuid16);
        } else {
                int i;
                int idx = 0;

                for (i = ATT_UUID_LENGTH; i > 0; i--) {
                        if (i == 12 || i == 10 || i == 8 || i == 6) {
                                buf[idx++] = '-';
                        }

                        idx += sprintf(&buf[idx], "%02x", uuid->uuid.uuid128[i - 1]);
                }
        }

        return buf;
}

/* return static buffer with characteristics properties mask */
static const char *format_properties(uint8_t properties){
        static const char props_str[] = "BRXWNISE"; // each letter corresponds to single property
        static char buf[9];
        int i;

        // copy full properties mask
        memcpy(buf, props_str, sizeof(props_str));

        for (i = 0; i < 8; i++) {
                // clear letter from mask if property not present
                if ((properties & (1 << i)) == 0) {
                        buf[i] = '-';
                }
        }

        return buf;
}

#ifdef ENABLE_IAS
static void configure_alert_button(void);
#endif

#if (SCAN_FILTER == (SCAN_FILTER_NAME) || SCAN_FILTER == SCAN_FILTER_NONE) 
static inline void format_adv_string(uint8_t * data, uint8_t len, char *out_string){
	memcpy(out_string, data, len);
	out_string[len] = '\0';	
}
#endif
/* return true if UUIDs match */
static bool match_uuid(uint8_t * uuid1, uint8_t *uuid2, uint8_t len){
	return !memcmp(uuid1, uuid2, len);
}

#ifdef ENABLE_IAS
/****************************************************************************************
 * @brief callback from alert button
 * @return void
 ****************************************************************************************/
static void app_button_press_cb(){
	uint8_t i;
	for(i = 0; i < CFG_MAX_CONNECTIONS ; i++){
		
		if(central_app_env.periph_devices[i].con_valid){
			if(central_app_env.periph_devices[i].serv_disc.ias_alert_counter > 1){
				central_app_env.periph_devices[i].serv_disc.ias_alert_counter = 0;
			} else {
				central_app_env.periph_devices[i].serv_disc.ias_alert_counter++;
			}
			
			user_ble_gatt_write(central_app_env.periph_devices[i].serv_disc.ias_write_op,
														central_app_env.periph_devices[i].con_idx, 
														central_app_env.periph_devices[i].serv_disc.ias_char.c.value_handle, 
														&central_app_env.periph_devices[i].serv_disc.ias_alert_counter, 
														sizeof(uint8_t));
		
			arch_puts("Write Alert Level \r\n");
		}
		
	}
	
	configure_alert_button();
}

/****************************************************************************************
 * @brief configure alert button
 * @return void
 ****************************************************************************************/
static void configure_alert_button(){
	wkupct_register_callback(app_button_press_cb);
}
#endif 

/****************************************************************************************
 * @brief After full discovery finished
 * @param[in] con_idx - connection identifier
 * @return void
 ****************************************************************************************/
static void handle_service_disc_finished(uint8_t con_idx){
#ifdef ENABLE_BAS
	if(central_app_env.periph_devices[con_idx].serv_disc.bas_handle_valid){
		user_gatt_read_simple(con_idx, 
						central_app_env.periph_devices[con_idx].serv_disc.bas_char.c.value_handle);
	}
#endif 

#ifdef ENABLE_IAS
	if(central_app_env.periph_devices[con_idx].serv_disc.ias_handle_valid){
			configure_alert_button();
	}
#endif 
	
}

/****************************************************************************************
 * @brief Scan for devices
 * @return void
 ****************************************************************************************/
static void ble_scan_for_devices(){
	
	arch_puts("Scanning...\r\n");
	
	ke_state_set(TASK_APP, APP_CONNECTABLE);
	ble_gap_error_t err = user_ble_gap_start_scan(true, SCAN_INTVL_MS, SCAN_WINDOW_MS, false);
  ASSERT_ERROR(err == BLE_GAP_ERROR_NO_ERROR);

}

/****************************************************************************************
 * @brief Callback from SDK when disconnect occurs.
 * @param[in] report  advertising report information
 * @return void
 ****************************************************************************************/
void user_on_adv_report_ind(struct gapm_adv_report_ind const * adv_ind){
	//bool dev_found = false;
	
#if CONNECT_TO_PERIPHERAL == 1
	bool conn_to_device = false;
	struct bd_addr dev_addr = adv_ind->report.adv_addr;
#endif 

	uint8_t num_ad_elements = user_ble_gap_get_adv_num_elements(adv_ind->report.data, 
																																		adv_ind->report.data_len);
	ble_gap_adv_struct_t adv_data_structs[num_ad_elements]; 
	
	user_ble_gap_parse_adv_data(adv_ind->report.data_len, adv_ind->report.data, adv_data_structs);
	
	uint8_t i;
	for(i = 0 ; i < num_ad_elements; i++){
		
		switch(adv_data_structs[i].type){

#if (SCAN_FILTER == (SCAN_FILTER_NAME) || SCAN_FILTER == SCAN_FILTER_NONE) 

			case GAP_AD_TYPE_COMPLETE_NAME:{
				//dev_found = true;
				
				char local_name[adv_data_structs[i].len + 1];
				format_adv_string(adv_data_structs[i].data,adv_data_structs[i].len, local_name);
				//arch_printf("Central Device Name: Parent\r\n");
				//arch_printf("Peripheral Device Name: %s\r\n", local_name);
	#if CONNECT_TO_PERIPHERAL == 1
				conn_to_device = memcmp(PERIPH_MATCH_DATA, adv_data_structs[i].data, PERIPH_MATCH_DATA_LEN) ? false : true;
	#endif 
				break;
			}
#endif 
			default:{
				break;
			}
			
		}
		
	}

	#if CONNECT_TO_PERIPHERAL
	if(conn_to_device){
		arch_printf("Central Device Name: Parent\r\n");
		arch_printf("Peripheral Device Name: Child #1\r\n");
		arch_puts("Connecting to Device...\r\n");
		central_app_env.connect_to_periph = true;
		memcpy(&central_app_env.connect_to_addr, &dev_addr, sizeof(struct bd_addr));
		central_app_env.connect_to_addr_type = adv_ind->report.adv_addr_type;
		
		user_ble_gap_stop_scan();
		
	}
	#endif

}

/****************************************************************************************
 * @brief Callback from SDK when  connection occurs.
 * @param[in] connection_idx  connection identifier
 * @param[in] param  peer connection info
 * @return void
 ****************************************************************************************/
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param){
	
		if(central_app_env.connection_timer != EASY_TIMER_INVALID_TIMER){
			app_easy_timer_cancel(central_app_env.connection_timer);
			central_app_env.connection_timer = EASY_TIMER_INVALID_TIMER;
		}
	
    default_app_on_connection(connection_idx, param);
		arch_puts("Connected\r\n");
		
		central_app_env.periph_devices[connection_idx].addr = param->peer_addr;
		central_app_env.periph_devices[connection_idx].con_idx = connection_idx;
		central_app_env.periph_devices[connection_idx].con_valid = true;
		central_app_env.num_connections++;
	
		if(central_app_env.num_connections < CFG_MAX_CONNECTIONS){
			ble_scan_for_devices();
		}

#ifdef ENABLE_IAS		
		central_app_env.periph_devices[connection_idx].serv_disc.ias_handle_valid = false;
#endif
#ifdef ENABLE_BAS
		central_app_env.periph_devices[connection_idx].serv_disc.bas_handle_valid = false;
#endif
		
		user_gatt_discover_all_services(connection_idx, 1);
		
		/*struct gattc_sdp_svc_ind const *disc_svc = (struct gattc_sdp_svc_ind const *)(param);
		service_info_t *svc = ke_malloc(user_ble_get_svc_size(disc_svc), KE_MEM_NON_RETENTION);
		arch_printf("%s: conn_idx=%04x start_h=%04x end_h=%04x\r\n", __func__, connection_idx, disc_svc->start_hdl, disc_svc->end_hdl);		
		arch_printf("UUID: %s\r\n", format_uuid(&svc->svc_uuid));
		*/
		
		
		// Write Value to Peripheral then Read Value 
		
		//arch_printf("Connection Index: %04x\r\n", connection_idx);
		//arch_printf("Handle: %04x\r\n", handle);
		
		// Handle: 002e
    // UUID: 772ae377-b3d2-4f8e-4042-5481d1e0098c

		uint8_t data = 0x05;		
		uint8_t* ptr = (uint8_t*)(sizeof(uint8_t));
		ptr[0] = data;
		
		arch_printf("Write Value: %04x\r\n", data);

		user_ble_gatt_write(GATTC_WRITE, connection_idx, 0x002E, ptr, sizeof(uint8_t));	

		for(int i= 0; i < 10000; i++){}

		user_gatt_read_simple(connection_idx, 0x002E);		

		/*arch_puts("UART Transmit Measurement\r\n");
		
		char measurement[] = "789";
		
		static char* v1 = (char *)(sizeof(char));
		v1[0] = measurement[0];
		uart_send(UART1, (uint8_t *)v1, 1, UART_OP_BLOCKING);
		v1[0] = measurement[1];
		uart_send(UART1, (uint8_t *)v1, 1, UART_OP_BLOCKING);
		v1[0] = measurement[2];
		uart_send(UART1, (uint8_t *)v1, 1, UART_OP_BLOCKING);
		
		*/
		
}

/****************************************************************************************
 * @brief Callback from SDK when disconnect occurs.
 * @param[in] param  peer connection info
 * @return void
 ****************************************************************************************/
void user_on_disconnect( struct gapc_disconnect_ind const *param ){
    default_app_on_disconnect(param);
		central_app_env.periph_devices[param->conhdl].con_valid = false;
	
		if(central_app_env.connection_timer != EASY_TIMER_INVALID_TIMER){
			app_easy_timer_cancel(central_app_env.connection_timer);
			central_app_env.connection_timer = EASY_TIMER_INVALID_TIMER;
		}
		if( central_app_env.num_connections == CFG_MAX_CONNECTIONS){
			ble_scan_for_devices();
		}
		central_app_env.num_connections--;
		
		arch_puts("Disconnected\r\n");
		
#ifdef ENABLE_IAS		
		if(central_app_env.num_connections < 1){
			wkupct_disable_irq();
		}
#endif
			
}

/****************************************************************************************
 * @brief Callback from SDK when disconnect occurs.
 * @return void
 ****************************************************************************************/
static void connection_timeout_cb(){
	
	central_app_env.connection_timer = EASY_TIMER_INVALID_TIMER;
	user_ble_cancel_connect();
}

/****************************************************************************************
 * @brief Callback from SDK when disconnect occurs.
 * @param[in] reason reason for disconnect BT SIG reason defined in co_error.h
 * @return void
 ****************************************************************************************/
void user_on_scanning_completed(uint8_t reason){
	
	if(reason == GAP_ERR_CANCELED){
		user_ble_gap_connect(central_app_env.connect_to_addr.addr, central_app_env.connect_to_addr_type);
		central_app_env.connection_timer = app_easy_timer(CONNECTION_TIMEOUT_10MS, connection_timeout_cb);
	} else {		
		arch_puts("Error\r\n");
	}	
}

/****************************************************************************************
 * @brief Callback from SDK once stack is configured
 * @return void
 ****************************************************************************************/
void user_app_on_set_dev_config_complete(void){
		/*Advertising starts within this default handler, but we have made the callback NULL to avoid advertising. 
		* alternate method, would be to copy the default handler code into here, removing the advertising call
		*/
		default_app_on_set_dev_config_complete();
	
		// Wait for UART receive here before scanning for devices
	
		arch_puts("Wait for UART alert\r\n");
		static char buffer[2];
	
		while(buffer[0] != '#') {
			//uart_receive(UART1, (uint8_t *)buffer, 1, UART_OP_BLOCKING);
			//uint8_t value = uart_read_byte(UART1);
			uint8_t value = '#';
			buffer[0] = value;
			arch_printf("Received Character: %c\r\n", value);
		}
	
		ble_scan_for_devices();
	
		//spi_flash_power_down(); //power down flash before entering sleep
}

static void handle_svc_ind(uint8_t con_idx, struct gattc_sdp_svc_ind const *disc_svc){
	service_info_t *svc = ke_malloc(user_ble_get_svc_size(disc_svc), KE_MEM_NON_RETENTION);

	arch_printf("%s: conn_idx=%04x start_h=%04x end_h=%04x\r\n", __func__, con_idx, disc_svc->start_hdl, disc_svc->end_hdl);
	
	user_gatt_parse_service(disc_svc, svc, con_idx);
	
	uint16_t i;
	
	/*bool ctrl_uuid_match;
	uint16_t ctrl_uuid = 0x686A;
	if(svc->svc_uuid.type == ATT_UUID_16) {
		ctrl_uuid_match = match_uuid((uint8_t *)&ctrl_uuid, (uint8_t *)&svc->svc_uuid.uuid.uuid16, sizeof(uint16_t));
	}
	
	if(ctrl_uuid_match) {
		arch_puts("Control Point Service\r\n");
	}*/
	
#ifdef ENABLE_IAS
	bool ias_uuid_match;
	uint16_t ias_uuid = ATT_SVC_IMMEDIATE_ALERT;
	uint16_t ias_alert_uuid = ATT_CHAR_ALERT_LEVEL;
	if(svc->svc_uuid.type == ATT_UUID_16){
		ias_uuid_match = match_uuid((uint8_t *)&ias_uuid, (uint8_t *)&svc->svc_uuid.uuid.uuid16, sizeof(uint16_t));
	}
	
	if(ias_uuid_match){
		arch_puts("Immediate Alert Service\r\n");
	}
#endif 
	
#ifdef ENABLE_BAS
	bool bas_uuid_match;
	uint16_t bas_uuid = ATT_SVC_BATTERY_SERVICE;
	uint16_t bas_level_uuid= ATT_CHAR_BATTERY_LEVEL;
	
	if(svc->svc_uuid.type == ATT_UUID_16){
		bas_uuid_match = match_uuid((uint8_t *)&bas_uuid, (uint8_t *)&svc->svc_uuid.uuid.uuid16, sizeof(uint16_t));
	}
	if(bas_uuid_match){
		arch_puts("Battery Service\r\n");
	}
#endif
	for(i = 0 ; i<svc->num_chars; i++){
		gattc_chars_t *gatt_char = &svc->items[i];
		
		//arch_printf("\t%04x char %s prop=%02x (%s)\r\n", gatt_char->handle, format_uuid(&gatt_char->uuid), gatt_char->c.properties, format_properties(gatt_char->c.properties));
		
		arch_printf("\tHandle: %04x\r\n", gatt_char->handle);
		arch_printf("\tUUID: %s\r\n", format_uuid(&gatt_char->uuid));
		
		
#ifdef ENABLE_IAS
		if(ias_uuid_match){
			if( match_uuid( (uint8_t *)&ias_alert_uuid , (uint8_t *)&gatt_char->uuid.uuid, 2) ){
				memcpy(&central_app_env.periph_devices[con_idx].serv_disc.ias_char, 
																									gatt_char, sizeof(gattc_chars_t) );
				
				central_app_env.periph_devices[con_idx].serv_disc.ias_handle_valid = true;
				arch_puts("\tAlert Level Char\r\n");
				
				if(gatt_char->c.properties & GATT_PROP_WRITE){
					central_app_env.periph_devices[con_idx].serv_disc.ias_write_op = GATTC_WRITE;
				} else if(gatt_char->c.properties & GATT_PROP_WRITE_NO_RESP) {
					central_app_env.periph_devices[con_idx].serv_disc.ias_write_op = GATTC_WRITE_NO_RESPONSE;	
				} else if(gatt_char->c.properties & GATT_PROP_WRITE_SIGNED) {
					central_app_env.periph_devices[con_idx].serv_disc.ias_write_op = GATTC_WRITE_SIGNED;	
				} else {		
					central_app_env.periph_devices[con_idx].serv_disc.ias_handle_valid = false;
				}
			}
		}
#endif 
#ifdef ENABLE_BAS
		if(bas_uuid_match){
			if( match_uuid( (uint8_t *)&bas_level_uuid , (uint8_t *)&gatt_char->uuid.uuid, 2) ){
				memcpy(&central_app_env.periph_devices[con_idx].serv_disc.bas_char, 
																										gatt_char, sizeof(gattc_chars_t));
				
				central_app_env.periph_devices[con_idx].serv_disc.bas_handle_valid = true;
				arch_puts("\tBattery Level Char\r\n");
			}
		}
#endif 
	}
	
	ke_free(svc);
	
	central_app_env.periph_devices[con_idx].serv_disc.last_handle = disc_svc->end_hdl;
}

/****************************************************************************************
 * @brief Callback from SDK when an unhandled stack event occurs
 * @param[in] msgid ID of message from stack
 * @param[in] param - message data
 * @param[in] dest_id - destination task of message, e.g. TASK_APP
 * @param[in] src_id - task originator of message
 * @return void
 ****************************************************************************************/
void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id){
	
	
	uint8_t conn_idx = KE_IDX_GET(src_id);
	switch(msgid){
		case GATTC_CMP_EVT:{
			
			struct gattc_cmp_evt const* evt = (struct gattc_cmp_evt const *)(param);
			switch(evt->operation){
				case GATTC_SDP_DISC_SVC_ALL:{
					
					arch_puts("Completion Event: GATTC_DISC_ALL_SVC\r\n");
					if(evt->status != CO_ERROR_NO_ERROR){

							user_gatt_discover_all_services(conn_idx, central_app_env.periph_devices[conn_idx].serv_disc.last_handle);
					} else {
							handle_service_disc_finished(conn_idx);
					}
								
				}break;
				case GATTC_WRITE:{
					arch_puts("Completion Event: GATTC_WRITE\r\n");
					break;
				}
				case GATTC_READ:
					arch_printf("GATTC_READ: status: %04x\r\n", evt->status);				
					break;
				
				default:
					break;
			}
			
		}break;
		
		case GATTC_EVENT_IND:{
			struct gattc_event_ind const *ind = (struct gattc_event_ind const *)param;
#ifdef ENABLE_BAS
			if(ind->handle == central_app_env.periph_devices[conn_idx].serv_disc.bas_char.c.value_handle){
				arch_puts("Battery Level Changed\r\n");			
			}
#endif	
		}break;
		
		case GAPM_CMP_EVT:{
			struct gapm_cmp_evt const* evt = (struct gapm_cmp_evt const *)(param);
			
			if(evt->operation == GAPM_CANCEL){
				arch_puts("Connection Timeout\r\n");
				ble_scan_for_devices();
			}
			break;
			
		}
		case GATTC_SDP_SVC_IND:{
			struct gattc_sdp_svc_ind const *disc_svc = (struct gattc_sdp_svc_ind const *)(param);
			
			uint8_t con_idx = KE_IDX_GET(src_id);
			//handle_svc_ind(con_idx, disc_svc);
		}break;
		
		case GATTC_READ_IND:{
			
			struct gattc_read_ind const *ind = (struct gattc_read_ind const*)param;		
			arch_printf("GATTC_READ: handle: %04x\r\n", ind->handle);
			arch_printf("GATTC_READ: value: %d\r\n", ind->value[0]);
			
			
		}break;	
		
		default:
			break;
		
	}	
	
}
 
/// @} APP
