/*******************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

#pragma once

#include "Params.h"

typedef enum
{
    Sys_Id_State_Prep = 0x0,
    Sys_Id_State_Settle,
    Sys_Id_State_Record,
    Sys_Id_State_Finished
} SYS_ID_STATE_t;

typedef enum
{
    Sys_Id_Mode_Frequency = 0x0,
    Sys_Id_Mode_Duty_Cycle
} SYS_ID_MODE_t;

typedef struct
{
    // TODO: consider per struct directives e.g. __ attribute __((aligned(4))
    SYS_ID_STATE_t state;
    SYS_ID_MODE_t mode;

    TIMER_t settle_timer;
    TIMER_t record_timer;

    MULT_CORE_ALIGN float Tsw_cmd_step;
    MULT_CORE_ALIGN float d_cmd_step;
    float vout_filt;

    MULT_CORE_ALIGN float data_freq[SYSID_DATA_LEN];
    MULT_CORE_ALIGN float data_duty[SYSID_DATA_LEN];
    uint32_t data_idx;
} SYS_ID_t;

#if defined(MULTI_CORE_PRESENT)
    #include <stddef.h>
STATIC_ASSERT(offsetof(SYS_ID_t, Tsw_cmd_step) % 4 == 0, "SYS_ID_t.Tsw_cmd_step must be 4-byte aligned");
STATIC_ASSERT(offsetof(SYS_ID_t, d_cmd_step)   % 4 == 0, "SYS_ID_t.d_cmd_step must be 4-byte aligned");
STATIC_ASSERT(offsetof(SYS_ID_t, data_freq)    % 4 == 0, "SYS_ID_t.data_freq must be 4-byte aligned");
STATIC_ASSERT(offsetof(SYS_ID_t, data_duty)    % 4 == 0, "SYS_ID_t.data_duty must be 4-byte aligned");
#endif

extern MULT_CORE_VLT SYS_ID_t sys_id;

void SYS_ID_Init();
void SYS_ID_Reset(SYS_ID_STATE_t state);
void SYS_ID_RunISR0();
void SYS_ID_RunISR1();
