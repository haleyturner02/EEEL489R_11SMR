/****************************************************************************************
 *
 * @file user_custs1_def.h
 *
 * @brief Custom Server 1 (CUSTS1) profile database definitions.
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************/

#ifndef _USER_CUSTS1_DEF_H_
#define _USER_CUSTS1_DEF_H_

/****************************************************************************************
 * @defgroup USER_CONFIG
 * @ingroup USER
 * @brief Custom Server 1 (CUSTS1) profile database definitions.
 *
 * @{
 ****************************************************************************************/

/****************************************************************************************
 * INCLUDE FILES
 ****************************************************************************************/

#include "attm_db_128.h"

/****************************************************************************************
 * DEFINES
 ****************************************************************************************/

// Service 1 of the custom server 1
#define DEF_SVC1_UUID_128                {0x59, 0x5a, 0x08, 0xe4, 0x86, 0x2a, 0x9e, 0x8f, 0xe9, 0x11, 0xbc, 0x7c, 0x98, 0x43, 0x42, 0x18}

#define DEF_SVC1_CTRL_POINT_UUID_128     {0x20, 0xEE, 0x8D, 0x0C, 0xE1, 0xF0, 0x4A, 0x0C, 0xB3, 0x25, 0xDC, 0x53, 0x6A, 0x68, 0x86, 0x2D}
#define DEF_SVC1_MEASUREMENT_UUID_128     {0x8C, 0x09, 0xE0, 0xD1, 0x81, 0x54, 0x42, 0x40, 0x8E, 0x4F, 0xD2, 0xB3, 0x77, 0xE3, 0x2A, 0x77}


#define DEF_SVC1_CTRL_POINT_CHAR_LEN     1
#define DEF_SVC1_MEASUREMENT_CHAR_LEN    10

#define DEF_SVC1_CONTROL_POINT_USER_DESC     "Control Point"
#define DEF_SVC1_MEASUREMENT_CHAR_USER_DESC   "Measurement"

/// Custom1 Service Data Base Characteristic enum
enum{
    // Custom Service 1
    SVC1_IDX_SVC = 0,

    SVC1_IDX_CONTROL_POINT_CHAR,
    SVC1_IDX_CONTROL_POINT_VAL,
    SVC1_IDX_CONTROL_POINT_USER_DESC,
		
		SVC1_IDX_MEASUREMENT_CHAR,
    SVC1_IDX_MEASUREMENT_VAL,
    SVC1_IDX_MEASUREMENT_NTF_CFG,
    SVC1_IDX_MEASUREMENT_USER_DESC,

    CUSTS1_IDX_NB
};

/// @} USER_CONFIG

#endif // _USER_CUSTS1_DEF_H_
