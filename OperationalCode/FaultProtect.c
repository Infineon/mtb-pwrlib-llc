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

MULT_CORE_SHM_SUB(".faults") MULT_CORE_VLT FAULTS_t faults;

void FAULT_PROTECT_Init()
{
    // Faults:
    // Detection parameters:
    DebounceFiltInit(&faults.timers.sw.ov_vin, params.sys.faults.sw.vin.time, params.sys.samp.ts1);
    DebounceFiltInit(&faults.timers.sw.uv_vin, params.sys.faults.sw.vin.time, params.sys.samp.ts1);
    DebounceFiltInit(&faults.timers.hw.ov_vin, params.sys.faults.hw.vin.time, params.sys.samp.ts0);
    DebounceFiltInit(&faults.timers.hw.oc_ires, params.sys.faults.hw.ires.time, params.sys.samp.ts0);
    DebounceFiltInit(&faults.timers.sw.ov_vout, params.sys.faults.sw.vout.time, params.sys.samp.ts1);
    DebounceFiltInit(&faults.timers.hw.ov_vout, params.sys.faults.hw.vout.time, params.sys.samp.ts0);
    DebounceFiltInit(&faults.timers.sw.oc_iout, params.sys.faults.sw.iout.time, params.sys.samp.ts1);
    DebounceFiltInit(&faults.timers.hw.oc_iout, params.sys.faults.hw.iout.time, params.sys.samp.ts0);
    DebounceFiltInit(&faults.timers.sw.ot, params.sys.faults.sw.temp.time, params.sys.samp.ts1);
    DebounceFiltInit(&faults.timers.sw.err_vout, params.sys.faults.sw.vout_err.time, params.sys.samp.ts1);
    DebounceFiltInit(&faults.timers.sw.fan, params.sys.faults.sw.fan_err.time, params.sys.samp.ts1);

    // Fault reaction bitmasks:
    faults.react_mask[No_React] = (FAULT_FLAGS_t) { 0U };

    faults.react_mask[HiZ_Clr_On_Req_Cnt] = (FAULT_FLAGS_t) { 0U };
    faults.react_mask[HiZ_Clr_On_Req_Cnt].sw.ov_vout = 0b1;
    faults.react_mask[HiZ_Clr_On_Req_Cnt].sw.oc_iout = 0b1;
    faults.react_mask[HiZ_Clr_On_Req_Cnt].sw.ot = 0b1;
    faults.react_mask[HiZ_Clr_On_Req_Cnt].sw.err_vout = 0b1;
    faults.react_mask[HiZ_Clr_On_Req_Cnt].sw.fan = 0b1;
    faults.react_mask[HiZ_Clr_On_Req_Cnt].hw.ov_vin = 0b1;
    faults.react_mask[HiZ_Clr_On_Req_Cnt].hw.oc_ires = 0b1;
    faults.react_mask[HiZ_Clr_On_Req_Cnt].hw.ov_vout = 0b1;
    faults.react_mask[HiZ_Clr_On_Req_Cnt].hw.oc_iout = 0b1;

    faults.react_mask[HiZ_Clr_On_Req_No_Cnt] = (FAULT_FLAGS_t) { 0U };
    faults.react_mask[HiZ_Clr_On_Req_No_Cnt].sw.em_stop = 0b1;

    faults.react_mask[HiZ_Clr_Auto_Try] = (FAULT_FLAGS_t) { 0U };
    faults.react_mask[HiZ_Clr_Auto_Try].sw.uv_vin = 0b1;
    faults.react_mask[HiZ_Clr_Auto_Try].sw.ov_vin = 0b1;

    faults.react_mask[HiZ_Perm] = (FAULT_FLAGS_t) { 0U };
    faults.react_mask[HiZ_Perm].sw.params = 0b1;
}


void FAULT_PROTECT_Reset()
{
    // Clear all faults
    faults.flags.all = 0U;
    faults.flags_latched.all = 0U;
    faults.reaction = No_React;

    // Unclearable faults:
    // "Control Mode" must be "Voltage" for system identification
    // faults.flags.sw.params |= (params.ctrl.sys_id.en) && (params.ctrl.id.mode != Voltage_Mode);
}


void FAULT_PROTECT_ClearFaults()
{
    faults.flags.all &= faults.react_mask[HiZ_Perm].all;
    faults.flags_latched.all &= faults.react_mask[HiZ_Perm].all;
    faults.reaction = No_React;
}


RAMFUNC_BEGIN
void FAULT_PROTECT_RunISR0()
{
    // Fault Detections ........................................................................
    // Vin, OV
    faults.flags.hw.ov_vin = DebounceFiltIncDec(sensor_iface.digital.ov_vin, &faults.timers.hw.ov_vin);
    // Ires, OC
    faults.flags.hw.oc_ires = DebounceFiltIncDec(sensor_iface.digital.oc_ires, &faults.timers.hw.oc_ires);
    // Vout, OV
    faults.flags.hw.ov_vout = DebounceFiltIncDec(sensor_iface.digital.ov_vout, &faults.timers.hw.ov_vout);
    // Iout, OC
    faults.flags.hw.oc_iout = DebounceFiltIncDec(sensor_iface.digital.oc_iout, &faults.timers.hw.oc_iout);
}


RAMFUNC_END

void FAULT_PROTECT_RunISR1()
{
    // Fault Detections ........................................................................
    // Vin, OV and UV
    faults.flags.sw.ov_vin =
        DebounceFiltIncDec((vars.vin_fb >= params.sys.faults.sw.vin.thresh.max), &faults.timers.sw.ov_vin);
    faults.flags.sw.uv_vin =
        DebounceFiltIncDec((vars.vin_fb < params.sys.faults.sw.vin.thresh.min), &faults.timers.sw.uv_vin);
    // Vout, OV
    faults.flags.sw.ov_vout =
        DebounceFiltIncDec((vars.vout_fb >= params.sys.faults.sw.vout.thresh.max), &faults.timers.sw.ov_vout);
    // Iout, OC
    faults.flags.sw.oc_iout =
        DebounceFiltIncDec(vars.iout_fb >= params.sys.faults.sw.iout.thresh.max, &faults.timers.sw.oc_iout);
    // OT
    faults.flags.sw.ot =
        DebounceFiltIncDec((vars.temp_fb >= params.sys.faults.sw.temp.thresh.max), &faults.timers.sw.ot);
    // Vout, regulation error
    faults.flags.sw.err_vout = DebounceFiltIncDec((sm.current == Volt_Ctrl_State) &&
                                                  (ABS(ctrl.vout.pi.error) >= params.sys.faults.sw.vout_err.thresh.max),
                                                  &faults.timers.sw.err_vout);
    // Fan
    faults.flags.sw.fan = DebounceFiltIncDec(ABS(vars.spd_ref_fan - vars.spd_fb_fan) >=
                                             params.sys.faults.sw.fan_err.thresh.max,
                                             &faults.timers.sw.fan);
    // Emergency Stop
    faults.flags.sw.em_stop = vars.em_stop;

    // Fault Latching ..........................................................................
    faults.flags_latched.all =
        (sm.current != Init_State) ? (faults.flags_latched.all | faults.flags.all) : (faults.flags_latched.all);

    // Fault Reactions .........................................................................
    faults.reaction =
        (faults.flags_latched.all & faults.react_mask[HiZ_Perm].all) ? HiZ_Perm :
        (faults.flags_latched.all & faults.react_mask[HiZ_Clr_Auto_Try].all) ? HiZ_Clr_Auto_Try :
        (faults.flags_latched.all & faults.react_mask[HiZ_Clr_On_Req_No_Cnt].all) ? HiZ_Clr_On_Req_No_Cnt :
        (faults.flags_latched.all & faults.react_mask[HiZ_Clr_On_Req_Cnt].all) ? HiZ_Clr_On_Req_Cnt :
        faults.reaction;
}
