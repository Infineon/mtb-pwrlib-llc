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


#include "Controller.h"

MULT_CORE_SHM_SUB(".sm") MULT_CORE_VLT STATE_MACHINE_t sm;

RAMFUNC_BEGIN
static void CommonISR0()
{
    SENSOR_IFACE_RunISR0();
    FAULT_PROTECT_RunISR0();
    if (sm.current != Init_State)
    {
        FILT_RunISR0();
    }

    #if defined(PC_TEST)
    vars.test[6] = vars.T_dt;
    #endif
}


RAMFUNC_END

static void CommonISR1()
{
    SENSOR_IFACE_RunISR1();
    FAULT_PROTECT_RunISR1();
    FCN_EXE_HANDLER_RunISR1();
}


void STATE_MACHINE_ResetAllModules()
{
    SENSOR_IFACE_Init();
    FAULT_PROTECT_Init();
    VOUT_CTRL_Init();
    FAN_CTRL_Init();
    SYS_ID_Init();

    SENSOR_IFACE_Reset();
    FAULT_PROTECT_Reset();
    FILT_Reset();
    FAN_CTRL_Reset();
    SYS_ID_Reset(Sys_Id_State_Finished);
}


static void InitEntry()
{
    vars.high_z = true;
    hw_fcn.GateDriverEnterHighZ();
    vars.sr_en = false;
    hw_fcn.SyncRecEnableDisable(vars.sr_en);

    if (!sm.vars.init.param_init_done)  // only after starting up
    {
        hw_fcn.StopPeripherals();       // disable all ISRs, PWMs, ADCs, etc.

        PARAMS_Init();
        hw_fcn.Init();                  // all peripherals must stop before re-initializing
        STATE_MACHINE_ResetAllModules();

        sm.vars.init.param_init_done = true;
        vars.en = (En == params.ctrl.auto_start);

        hw_fcn.StartPeripherals();      // enable all ISRs, PWMs, ADCs, etc.
    }

    StopWatchInit(&sm.vars.init.timer, params.ctrl.boot_time, params.sys.samp.ts1);
}


static void InitISR1()
{
    StopWatchRun(&sm.vars.init.timer);
}


static void SoftStartEntry()
{
    vars.Tsw_cmd_int = HZ_TO_PERIOD(params.ctrl.soft_start.f);
    DT_CTRL_Reset();
    D_CTRL_Reset();
    StopWatchInit(&sm.vars.soft_start.timer, params.ctrl.soft_start.t, params.sys.samp.ts1);
    vars.high_z = false;
    hw_fcn.GateDriverExitHighZ();
}


static void SoftStartExit()
{
    vars.sr_en = params.llc.sr.en;
    hw_fcn.SyncRecEnableDisable(vars.sr_en);
}


static void SoftStartISR1()
{
    StopWatchRun(&sm.vars.soft_start.timer);
}


static void FreqCtrlEntry()
{
    TSW_CTRL_Reset();
    D_CTRL_Reset();
}


static void FreqCtrlISR0()
{
    TSW_CTRL_RunISR0();
    DT_CTRL_RunISR0();
    SR_CTRL_RunISR0();
}


static void DutyCtrlEntry()
{
    TSW_CTRL_Reset();
    D_CTRL_Reset();
}


static void DutyCtrlISR0()
{
    DT_CTRL_RunISR0();
    D_CTRL_RunISR0();
    SR_CTRL_RunISR0();
}


static void VoltCtrlEntry()
{
    VOUT_CTRL_Reset();
    TSW_CTRL_Reset();
    D_CTRL_Reset();
    vars.vout_cmd_int = vars.vout_fb;
}


static void VoltCtrlISR0()
{
    VOUT_CTRL_RunISR0();
    TSW_CTRL_RunISR0();
    DT_CTRL_RunISR0();
    D_CTRL_RunISR0();
    SR_CTRL_RunISR0();
}


static void FaultEntry()
{
    const FAULT_REACTION_t HiZ_Space_Start = HiZ_Clr_On_Req_Cnt, HiZ_Space_End = HiZ_Perm;

    if (IS_BETWEEN(faults.reaction, HiZ_Space_Start, HiZ_Space_End))
    {
        vars.high_z = true;
        hw_fcn.GateDriverEnterHighZ();
        vars.sr_en = false;
        hw_fcn.SyncRecEnableDisable(vars.sr_en);
    }

    StopWatchInit(&sm.vars.fault.auto_clr_timer, params.sys.faults.auto_clr_per, params.sys.samp.ts1);
    sm.vars.fault.clr_done = false;
    sm.vars.fault.clr_request = false;
}


static void FaultExit()
{
    // Reset all possibly frozen controllers
    DT_CTRL_Reset();
    VOUT_CTRL_Reset();
    TSW_CTRL_Reset();
    D_CTRL_Reset();
    // Soft start entry prep
    vars.Tsw_cmd_int = HZ_TO_PERIOD(params.ctrl.soft_start.f);
}


static void FaultISR1()
{
    if (sm.vars.fault.clr_request)
    {
        sm.vars.fault.clr_request = false;
        hw_fcn.FaultResetRelease();
        if (!faults.flags.all) // if non-latched values are cleared
        {
            FAULT_PROTECT_ClearFaults();
            sm.vars.fault.clr_done = true;
        }
    }
    else if (faults.reaction == HiZ_Clr_Auto_Try)
    {
        StopWatchRun(&sm.vars.fault.auto_clr_timer);
        if (StopWatchIsDone(&sm.vars.fault.auto_clr_timer))
        {
            StopWatchReset(&sm.vars.fault.auto_clr_timer);
            sm.vars.fault.clr_request = true;
            hw_fcn.FaultResetEngage();
        }
    }
    else if (faults.reaction == HiZ_Clr_On_Req_No_Cnt)
    {
        if (RISE_EDGE(sm.vars.fault.clr_faults_prev, vars.clr_faults))
        {
            sm.vars.fault.clr_request = true;
            hw_fcn.FaultResetEngage();
        }
    }
    else if (faults.reaction == HiZ_Clr_On_Req_Cnt)
    {
        if (RISE_EDGE(sm.vars.fault.clr_faults_prev, vars.clr_faults) &&
            (sm.vars.fault.clr_try_cnt < params.sys.faults.max_clr_tries))
        {
            ++sm.vars.fault.clr_try_cnt;
            sm.vars.fault.clr_request = true;
            hw_fcn.FaultResetEngage();
        }
    }

    sm.vars.fault.clr_faults_prev = vars.clr_faults;
}


static void StateTransitionCheck()
{
    bool fault_trigger = (faults.reaction != No_React);

    STATE_ID_t current = sm.current;
    STATE_ID_t next = current;
    switch (current)
    {
        default:
        case Init_State:
            if (vars.en && StopWatchIsDone(&sm.vars.init.timer))
            {
                next = Soft_Start_State;
            }
            break;

        case Soft_Start_State:
            if (!vars.en)
            {
                next = Init_State;
            }
            else if (fault_trigger)
            {
                next = Fault_State;
            }
            else if (StopWatchIsDone(&sm.vars.soft_start.timer))
            {
                if (params.ctrl.id.mode == Frequency_Mode)
                {
                    next = Freq_Ctrl_State;
                }
                else if (params.ctrl.id.mode == Voltage_Mode)
                {
                    next = Volt_Ctrl_State;
                }
                else if (params.ctrl.id.mode == Duty_Cycle_Mode)
                {
                    next = Duty_Ctrl_State;
                }
            }
            break;

        case Freq_Ctrl_State:
        case Volt_Ctrl_State:
        case Duty_Ctrl_State:
            if (!vars.en)
            {
                next = Init_State;
            }
            else if (fault_trigger)
            {
                next = Fault_State;
            }
            else if (fcn_exe_handler.ack & FCN_EXE_HANDLER_Mask(Run_Sys_Id))
            {
                if (params.ctrl.sys_id.en)
                {
                    next = Sys_Id_State;
                    SYS_ID_Reset(Sys_Id_State_Prep);
                }
                else
                {
                    fcn_exe_handler.done |= FCN_EXE_HANDLER_Mask(Run_Sys_Id);
                }
            }
            break;

        case Sys_Id_State:
            if (!vars.en)
            {
                next = Init_State;
            }
            else if (fault_trigger)
            {
                next = Fault_State;
            }
            else if (sys_id.state == Sys_Id_State_Finished)
            {
                next = Soft_Start_State;
            }
            break;

        case Fault_State:
            if ((!vars.en) || sm.vars.fault.clr_done)
            {
                next = Init_State;
            }
            break;
    }
    sm.previous = current;
    sm.next = next;
}


#if !defined(LLC_APP_PRESENT)
static void DummySyncRecEnableDisable(bool en)
{
}


static bool DummyFlashWrite(PARAMS_t* ram_data)
{
    return true;
}


static bool DummyFlashRead(PARAMS_ID_t id, PARAMS_t* ram_data)
{
    return false;
}


#endif // if !defined(LLC_APP_PRESENT)

void STATE_MACHINE_Init()
{
    #if !defined(LLC_APP_PRESENT)
    hw_fcn.Init = EmptyFcn;
    hw_fcn.EnterCriticalSection = EmptyFcn;
    hw_fcn.ExitCriticalSection = EmptyFcn;
    hw_fcn.GateDriverEnterHighZ = EmptyFcn;
    hw_fcn.GateDriverExitHighZ = EmptyFcn;
    hw_fcn.FaultResetEngage = EmptyFcn;
    hw_fcn.FaultResetRelease = EmptyFcn;
    hw_fcn.StartPeripherals = EmptyFcn;
    hw_fcn.StopPeripherals = EmptyFcn;
    hw_fcn.SyncRecEnableDisable = DummySyncRecEnableDisable;
    hw_fcn.FlashRead = DummyFlashRead;
    hw_fcn.FlashWrite = DummyFlashWrite;
    params_fcn.InitManual = PARAMS_InitManual;
    params_fcn.InitAutoCalc = PARAMS_InitAutoCalc;
    params_fcn.InitOverWrite = EmptyFcn;
    #endif // if !defined(LLC_APP_PRESENT)

    sm.extra_callback.RunISR0 = EmptyFcn;
    sm.extra_callback.RunISR1 = EmptyFcn;

    /* *SUSPEND-FORMATTING* */
    //                                         Entry(),         Exit(),         RunISR0(),       RunISR1()
    sm.states[Init_State]        = (STATE_t) { &InitEntry,      &EmptyFcn,      &EmptyFcn,       &InitISR1       };
    sm.states[Soft_Start_State]  = (STATE_t) { &SoftStartEntry, &SoftStartExit, &EmptyFcn,       &SoftStartISR1  };
    sm.states[Freq_Ctrl_State]   = (STATE_t) { &FreqCtrlEntry,  &EmptyFcn,      &FreqCtrlISR0,   &EmptyFcn       };
    sm.states[Volt_Ctrl_State]   = (STATE_t) { &VoltCtrlEntry,  &EmptyFcn,      &VoltCtrlISR0,   &EmptyFcn       };
    sm.states[Duty_Ctrl_State]   = (STATE_t) { &DutyCtrlEntry,  &EmptyFcn,      &DutyCtrlISR0,   &EmptyFcn       };
    sm.states[Sys_Id_State]      = (STATE_t) { &EmptyFcn,       &EmptyFcn,      &SYS_ID_RunISR0, &SYS_ID_RunISR1 };
    sm.states[Fault_State]       = (STATE_t) { &FaultEntry,     &FaultExit,     &EmptyFcn,       &FaultISR1      };
    /* *RESUME-FORMATTING* */

    // Must not be STATE_MACHINE_ResetAllModules because that is one of the functions requested by gui:
    FCN_EXE_HANDLER_Init();
    FCN_EXE_HANDLER_Reset();
    sm.vars.fault.clr_try_cnt = 0U;
    sm.vars.init.param_init_done = false;
    sm.previous = Init_State;
    sm.current = Init_State;
    sm.states[sm.current].Entry();

    #if defined(PC_TEST)
    for (uint32_t index = 0; index < sizeof(sm.vars.capture_vals) / sizeof(float32_t); ++index)
    {
        sm.vars.capture_vals[index] = 0.0f;
        sm.vars.capture_channels[index] = &sm.vars.capture_vals[index];
    }
    #endif
}


RAMFUNC_BEGIN
void STATE_MACHINE_RunISR0()
{
    CommonISR0();
    sm.states[sm.current].RunISR0();
    sm.extra_callback.RunISR0();
}


RAMFUNC_END

void STATE_MACHINE_RunISR1()
{
    CommonISR1();
    sm.states[sm.current].RunISR1();
    sm.extra_callback.RunISR1();

    StateTransitionCheck();
    if (sm.next != sm.current)
    {
        #if defined(PC_TEST)
        for (uint32_t index = 0; index < sizeof(sm.vars.capture_vals) / sizeof(float32_t); ++index)
        {
            sm.vars.capture_vals[index] = (*sm.vars.capture_channels[index]);
        }
        #endif
        sm.states[sm.current].Exit();
        sm.states[sm.next].Entry();
        sm.current = sm.next;
        // This instruction (sm.current update) should be the last one ue to data integrity reasons (between ISR0/ISR1)
    }
}
