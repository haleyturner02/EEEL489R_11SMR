/**
 ****************************************************************************************
 *
 * @file user_custs1_def.h
 *
 * @brief Custom Server 1 (CUSTS1) profile database definitions.
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef _USER_CUSTS1_DEF_H_
#define _USER_CUSTS1_DEF_H_

/**
 ****************************************************************************************
 * @defgroup USER_CONFIG
 * @ingroup USER
 * @brief Custom Server 1 (CUSTS1) profile database definitions.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "attm_db_128.h"

/*
 * DEFINES
 ****************************************************************************************
 */

// Service 1 of the custom server 1
#define DEF_SVC1_UUID_128                {0x59, 0x5a, 0x08, 0xe4, 0x86, 0x2a, 0x9e, 0x8f, 0xe9, 0x11, 0xbc, 0x7c, 0x98, 0x43, 0x42, 0x18}

#define DEF_SVC1_CTRL_POINT_UUID_128     {0x20, 0xEE, 0x8D, 0x0C, 0xE1, 0xF0, 0x4A, 0x0C, 0xB3, 0x25, 0xDC, 0x53, 0x6A, 0x68, 0x86, 0x2D}

#define DEF_SVC1_CTRL_POINT_CHAR_LEN     1

#define DEF_SVC1_CONTROL_POINT_USER_DESC     "Control Point"

// Service 2 of the custom server 1
#define DEF_SVC2_UUID_128                {0x59, 0x5a, 0x08, 0xe4, 0x86, 0x2a, 0x9e, 0x8f, 0xe9, 0x11, 0xbc, 0x7c, 0x7c, 0x46, 0x42, 0x18}

#define DEF_SVC2_WRITE_VAL_1_UUID_128    {0x20, 0xEE, 0x8D, 0x0C, 0xE1, 0xF0, 0x4A, 0x0C, 0xB3, 0x25, 0xDC, 0x53, 0x6A, 0x68, 0x86, 0x2C}

#define DEF_SVC2_WRITE_VAL_1_CHAR_LEN    1

#define DEF_SVC2_WRITE_VAL_1_USER_DESC   "Write Data"

// Service 3 of the custom server 1
#define DEF_SVC3_UUID_128                {0x59, 0x5a, 0x08, 0xe4, 0x86, 0x2a, 0x9e, 0x8, 0xe9, 0x11, 0xbc, 0x7c, 0xd0, 0x47, 0x42, 0x18}

#define DEF_SVC3_READ_VAL_2_UUID_128     {0x23, 0x68, 0xEC, 0x52, 0x1E, 0x62, 0x44, 0x74, 0x9A, 0x1B, 0xD1, 0x8B, 0xAB, 0x75, 0xB6, 0x6D}

#define DEF_SVC3_READ_VAL_2_CHAR_LEN     2

#define DEF_SVC3_READ_VAL_2_USER_DESC    "Read Data"

/// Custom1 Service Data Base Characteristic enum
enum
{
    // Custom Service 1
    SVC1_IDX_SVC = 0,

    SVC1_IDX_CONTROL_POINT_CHAR,
    SVC1_IDX_CONTROL_POINT_VAL,
    SVC1_IDX_CONTROL_POINT_USER_DESC,

    // Custom Service 2
    SVC2_IDX_SVC,

    SVC2_WRITE_1_CHAR,
    SVC2_WRITE_1_VAL,
    SVC2_WRITE_1_USER_DESC,

    // Custom Service 3
    SVC3_IDX_SVC,

    SVC3_IDX_READ_2_CHAR,
    SVC3_IDX_READ_2_VAL,
    SVC3_IDX_READ_2_USER_DESC,

    CUSTS1_IDX_NB
};

/// @} USER_CONFIG

#endif // _USER_CUSTS1_DEF_H_
