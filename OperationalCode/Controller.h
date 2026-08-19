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

#include "Biquad.h"
#include "CtrlVars.h"
#include "FaultProtect.h"
#include "FcnExeHandler.h"
#include "General.h"
#include "NotchFilt.h"
#include "Params.h"
#include "SensorIface.h"
#include "StateMachine.h"
#include "SysId.h"

typedef struct
{
    float32_t fsw;
    float32_t G_est;
} FILT_t;

extern FILT_t filt;

typedef struct
{
    float32_t kT;       // [sec/V]
    float32_t kd;       // [%/V]
    PI_t pi;            // []
} VOUT_CTRL_t;

typedef struct
{
    float32_t dT_fb;        // [sec], time delta (two consecutive edges of fan tachometer signal)
    float32_t spd_coeff;    // [rpm.sec], coefficient to convert from time delta to speed
    float32_t lpf_coeff;    // [], low pass filter coefficient
    int16_t lut_idx;
} FAN_CTRL_t;

typedef struct
{
    VOUT_CTRL_t vout;
    FAN_CTRL_t fan;
} CTRL_t;

extern CTRL_t ctrl;

typedef struct
{   // hardware function pointers
    void (* Init)();
    void (* EnterCriticalSection)();
    void (* ExitCriticalSection)();
    void (* GateDriverEnterHighZ)();
    void (* GateDriverExitHighZ)();
    void (* FaultResetEngage)();
    void (* FaultResetRelease)();
    void (* StartPeripherals)();         // PWMs, ADCs, DMA, ISRs
    void (* StopPeripherals)();          // PWMs, ADCs, DMA, ISRs
    void (* SyncRecEnableDisable)(bool en);
    bool (* FlashRead)(PARAMS_ID_t id, PARAMS_t* ram_data);
    bool (* FlashWrite)(PARAMS_t* ram_data);
} HW_FCN_t;

extern HW_FCN_t hw_fcn;

// Filter ...............................
void FILT_Reset();
void FILT_RunISR0();

// Vout control .........................
void VOUT_CTRL_Init();
void VOUT_CTRL_Reset();
void VOUT_CTRL_RunISR0();

// Tsw control ..........................
void TSW_CTRL_Reset();
void TSW_CTRL_RunISR0();

// Duty cycle control ...................
void D_CTRL_Reset();
void D_CTRL_RunISR0();

// Synchronous rectification control ....
void SR_CTRL_RunISR0();

// Dead-time control ....................
void DT_CTRL_Reset();
void DT_CTRL_RunISR0();

// Fan control ..........................
void FAN_CTRL_Init();
void FAN_CTRL_Reset();
void FAN_CTRL_RunISR0();
void FAN_CTRL_RunISR1();
