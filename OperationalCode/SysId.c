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

#include "Controller.h"

MULT_CORE_SHM_SUB(".sys_id") MULT_CORE_VLT SYS_ID_t sys_id;

void SYS_ID_Init()
{
    StopWatchInit(&sys_id.settle_timer, params.ctrl.sys_id.settle_time, params.sys.samp.ts1);
    StopWatchInit(&sys_id.record_timer, params.ctrl.sys_id.record_time, params.sys.samp.ts1);
    sys_id.Tsw_cmd_step = (params.llc.range.Tsw.max - params.llc.range.Tsw.min) / (SYSID_DATA_LEN - 1U);
    sys_id.d_cmd_step = SYSID_DUTY_MAX / (SYSID_DATA_LEN - 1U);
}


void SYS_ID_Reset(SYS_ID_STATE_t state)
{
    // can be reset to 'prep' or 'finished' states
    sys_id.state = (state == Sys_Id_State_Prep) ? Sys_Id_State_Prep : Sys_Id_State_Finished;
}


void SYS_ID_RunISR1()
{
    switch (sys_id.state)
    {
        default:
        case Sys_Id_State_Finished:
            break;

        case Sys_Id_State_Prep:
            vars.Tsw_cmd_int = params.llc.range.Tsw.max;
            vars.d_cmd_int = SYSID_DUTY_MAX;
            sys_id.data_idx = 0U;
            StopWatchReset(&sys_id.settle_timer);
            sys_id.mode = Sys_Id_Mode_Frequency;
            sys_id.state = Sys_Id_State_Settle;
            break;

        case Sys_Id_State_Settle:
            StopWatchRun(&sys_id.settle_timer);
            if (StopWatchIsDone(&sys_id.settle_timer))
            {
                sys_id.vout_filt = vars.vout_fb;
                StopWatchReset(&sys_id.record_timer);
                sys_id.state = Sys_Id_State_Record;
            }
            break;

        case Sys_Id_State_Record:
            StopWatchRun(&sys_id.record_timer);
            if (StopWatchIsDone(&sys_id.record_timer))
            {
                if (sys_id.mode == Sys_Id_Mode_Frequency)
                {
                    sys_id.data_freq[sys_id.data_idx] = sys_id.vout_filt;
                    if (++sys_id.data_idx < SYSID_DATA_LEN)
                    {
                        vars.Tsw_cmd_int -= sys_id.Tsw_cmd_step;
                    }
                    else
                    {
                        sys_id.data_idx = 0U;
                        sys_id.mode = Sys_Id_Mode_Duty_Cycle;
                    }
                    StopWatchReset(&sys_id.settle_timer);
                    sys_id.state = Sys_Id_State_Settle;
                }
                else // Sys_Id_Mode_Duty_Cycle
                {
                    sys_id.data_duty[sys_id.data_idx] = sys_id.vout_filt;
                    if (++sys_id.data_idx < SYSID_DATA_LEN)
                    {
                        vars.d_cmd_int -= sys_id.d_cmd_step;
                        StopWatchReset(&sys_id.settle_timer);
                        sys_id.state = Sys_Id_State_Settle;
                    }
                    else
                    {
                        fcn_exe_handler.done |= FCN_EXE_HANDLER_Mask(Run_Sys_Id);
                        sys_id.state = Sys_Id_State_Finished;
                    }
                }
            }
            break;
    }
}


void SYS_ID_RunISR0()
{
    switch (sys_id.state)
    {
        default:
        case Sys_Id_State_Finished:
        case Sys_Id_State_Prep:
            break;

        case Sys_Id_State_Record:
            sys_id.vout_filt += (vars.vout_fb - sys_id.vout_filt) * params.ctrl.sys_id.record_w0 * params.sys.samp.ts0;

        /* fall through */
        case Sys_Id_State_Settle:
            SR_CTRL_RunISR0();
            break;
    }
    #if defined(PC_TEST)
    vars.test[3] = sys_id.mode;
    vars.test[4] = sys_id.state;
    vars.test[5] = sys_id.vout_filt;
    #endif
}
