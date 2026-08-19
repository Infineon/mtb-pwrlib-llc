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

// HW --> Fault Detection --> SW
//                            |
//                            | Processing
//                            V
// HW <-- Fault Reaction <-- SW
//
// Some fault detections happen in SW, some happen in HW.
// HW only reports the faults and SW decides what reaction is needed.


typedef union
{
    struct
    {
        union    // 32 bits, software faults space
        {
            struct
            {
                uint32_t ov_vin : 1;        // over-voltage, vin, slow, adc
                uint32_t uv_vin : 1;        // under-voltage, vin, slow, adc
                uint32_t ov_vout : 1;       // over-voltage, vout, slow, adc
                uint32_t oc_iout : 1;       // over-current, iout, slow, adc
                uint32_t ot : 1;            // over-temperature, slow, adc
                uint32_t err_vout : 1;      // vout regulation error exceeds the limit
                uint32_t fan : 1;           // fan fault
                uint32_t params : 1;        // parameter fault (incompatibilities between parameters)
                uint32_t em_stop : 1;       // emergency stop
                uint32_t reserved : 23;     // reserved, DO NOT LEAVE UNDEFINED BITS
            };

            uint32_t reg;
        } sw;
        union   // 32 bits, hardware faults space
        {
            struct
            {
                uint32_t ov_vin : 1;        // over-voltage, vin, fast
                uint32_t oc_ires : 1;       // over-current, ires, fast
                uint32_t ov_vout : 1;       // over-voltage, vout, fast
                uint32_t oc_iout : 1;       // over-current, iout, fast
                uint32_t reserved : 28;     // reserved, DO NOT LEAVE UNDEFINED BITS
            };

            uint32_t reg;
        } hw;
    };

    uint64_t all;
} FAULT_FLAGS_t;

typedef enum
{
    No_React = 0U,              // no reaction to fault
    HiZ_Clr_On_Req_Cnt = 1U,    // high-z, clear fault on request (e.g. GUI), count number of clear attempts
    HiZ_Clr_On_Req_No_Cnt = 2U, // high-z, clear fault on request (e.g. GUI), don't count number of clear attempts
    HiZ_Clr_Auto_Try = 3U,      // high-z, try to clear fault automatically when fault condition is cleared
    HiZ_Perm = 4U,              // high-z, fault is permanent and cannot be cleared
    Num_React = 5U              // number of reactions
} FAULT_REACTION_t;

typedef struct
{   // for sw faults
    TIMER_t ov_vin;
    TIMER_t uv_vin;
    TIMER_t ov_vout;
    TIMER_t oc_iout;
    TIMER_t ot;
    TIMER_t err_vout;
    TIMER_t fan;
} SW_FAULT_TIMER_t;

typedef struct
{   // for hw faults
    TIMER_t ov_vin;
    TIMER_t oc_ires;
    TIMER_t ov_vout;
    TIMER_t oc_iout;
} HW_FAULT_TIMER_t;

typedef struct
{
    SW_FAULT_TIMER_t sw;
    HW_FAULT_TIMER_t hw;
} FAULT_TIMER_t;


typedef struct
{
    FAULT_FLAGS_t flags;
    FAULT_FLAGS_t flags_latched;
    FAULT_FLAGS_t react_mask[Num_React];
    FAULT_REACTION_t reaction;
    FAULT_TIMER_t timers;
} FAULTS_t;

extern MULT_CORE_VLT FAULTS_t faults;

void FAULT_PROTECT_Init();
void FAULT_PROTECT_Reset();
void FAULT_PROTECT_ClearFaults();
void FAULT_PROTECT_RunISR0();
void FAULT_PROTECT_RunISR1();
