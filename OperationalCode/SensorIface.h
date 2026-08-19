/*******************************************************************************
* Copyright 2021-2024, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#pragma once

#include "Params.h"

typedef struct
{
    float32_t filt_coeff;
    float32_t raw;
    float32_t filt;
} ANALOG_SENSOR_t;

typedef union
{
    struct
    {
        uint32_t ov_vin : 1;        // from hw peripherals
        uint32_t oc_ires : 1;       // from hw peripherals
        uint32_t ov_vout : 1;       // from hw peripherals
        uint32_t oc_iout : 1;       // from hw peripherals

        uint32_t user_btn : 1;      // user push button
        uint32_t user_btn_prev : 1; // user push button, previous value
        uint32_t user_sw : 1;       // user switch

        uint32_t padding;           // padding
    };

    uint32_t all;
} DIGITAL_INPUT_t;

typedef struct
{
    ANALOG_SENSOR_t vin;
    ANALOG_SENSOR_t vout;
    ANALOG_SENSOR_t iout;
    ANALOG_SENSOR_t cmd;
    ANALOG_SENSOR_t temp;

    DIGITAL_INPUT_t digital;
} SENSOR_IFACE_t;

extern MULT_CORE_VLT SENSOR_IFACE_t sensor_iface;

void SENSOR_IFACE_Init();
void SENSOR_IFACE_Reset();
void SENSOR_IFACE_RunISR0();    // for fast variables
void SENSOR_IFACE_RunISR1();    // for slow variables
