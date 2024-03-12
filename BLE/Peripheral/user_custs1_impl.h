/****************************************************************************************
 *
 * @file user_custs1_impl.h
 *
 * @brief Peripheral project Custom1 Server implementation header file.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************/

#ifndef _USER_CUSTS1_IMPL_H_
#define _USER_CUSTS1_IMPL_H_

/****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief
 *
 * @{
 ****************************************************************************************/

/****************************************************************************************
 * DEFINES
 ****************************************************************************************/

enum {
    CUSTS1_CP_ADC_VAL1_DISABLE = 0,
    CUSTS1_CP_ADC_VAL1_ENABLE,
};

enum {
    CUSTS1_LED_OFF = 0,
    CUSTS1_LED_ON,
};

/****************************************************************************************
 * INCLUDE FILES
 ****************************************************************************************/

#include "gapc_task.h"                 // gap functions and messages
#include "gapm_task.h"                 // gap functions and messages
#include "custs1_task.h"

/****************************************************************************************
 * FUNCTION DECLARATIONS
 ****************************************************************************************/

/****************************************************************************************
 * @brief Control point write indication handler.
 * @param[in] msgid   Id of the message received.
 * @param[in] param   Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id  ID of the sending task instance.
 ****************************************************************************************/
void user_svc1_ctrl_wr_ind_handler(ke_msg_id_t const msgid,
                                   struct custs1_val_write_ind const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id);																	 
																	 																			 
void user_svc1_measurement_wr_ind_handler(ke_msg_id_t const msgid,
                                          struct custs1_val_write_ind const *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id, uint16_t measurement);

void user_svc1_measurement_update_ind_handler(ke_msg_id_t const msgid,
                                          struct custs1_val_set_req const *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id, uint16_t measurement);																					

/// @} APP

#endif // _USER_CUSTS1_IMPL_H_
