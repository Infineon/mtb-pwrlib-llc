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

#include "General.h"

typedef struct
{
    // Controlled externally (e.g. GUI) .......................
    // TODO: consider per struct directives e.g. __attribute__((aligned(4))
    MULT_CORE_ALIGN bool en;                // [], Disable
    MULT_CORE_ALIGN bool clr_faults;        // [], Clear faults
    MULT_CORE_ALIGN bool em_stop;           // [], Emergency stop
    // ........................................................

    float32_t cmd_ext;          // [%], External (e.g. GUI)
    float32_t cmd_int;          // [%], Internal (e.g. potentiometer)
    float32_t cmd_final;        // [%], Final, either external or internal

    float32_t vout_cmd_ext;     // [V], External
    float32_t vout_cmd_int;     // [V], Internal, after applying caps & rate limiter

    float32_t Tsw_cmd_ext;      // [sec], External
    float32_t Tsw_cmd_volt;     // [sec], From voltage controller
    float32_t Tsw_cmd_int;      // [sec], Internal, after applying caps & rate limiter

    float32_t Td_on_sr;         // [sec], on-time delay, sync. rec.
    float32_t Td_off_sr;        // [sec], off-time delay, sync. rec.

    float32_t T_dt;             // [sec], dead-time

    float32_t d_cmd_ext;        // [%], External
    float32_t d_cmd_volt;       // [%], From voltage controller
    float32_t d_cmd_int;        // [%], Internal, after applying caps & rate limiter

    bool high_z;                // []
    bool sr_en;                 // []

    float32_t d_cmd_fan;        // [%], duty cycle command, fan
    float32_t spd_ref_fan;      // [rpm], speed reference, fan
    float32_t spd_fb_fan;       // [rpm], speed feedback, fan

    float32_t vin_fb;           // [V], Feedback
    float32_t vout_fb;          // [V], Feedback
    float32_t iout_fb;          // [V], Feedback, from hw low-pass-filter
    float32_t temp_fb;          // [Celsius]

    #if defined(PC_TEST)
    float32_t test[16];         // []
    #endif
} CTRL_VARS_t;

#ifdef MULTI_CORE_PRESENT
    #include <stddef.h>
STATIC_ASSERT(offsetof(CTRL_VARS_t, en)         % 4 == 0, "CTRL_VARS_t.en must be 4-byte aligned");
STATIC_ASSERT(offsetof(CTRL_VARS_t, em_stop)    % 4 == 0, "CTRL_VARS_t.em_stop must be 4-byte aligned");
STATIC_ASSERT(offsetof(CTRL_VARS_t, clr_faults) % 4 == 0, "CTRL_VARS_t.clr_faults must be 4-byte aligned");
#endif

extern MULT_CORE_VLT CTRL_VARS_t vars;
