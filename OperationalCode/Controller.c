/*******************************************************************************
* Copyright 2021-2024, Cypress Semiconductor Corporation (an Infineon company)
* or an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
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

CTRL_t ctrl;
FILT_t filt;

// Filters
// ...........................................................................................
void FILT_Reset()
{
    filt.fsw = 0.0f;
    filt.G_est = 0.0f;
}


void FILT_RunISR0()
{
    filt.fsw += (1.0f / MAX(vars.Tsw_cmd_int, EPSILON) - filt.fsw) * params.filt.w0_fsw * params.sys.samp.ts0;
    filt.G_est += (vars.iout_fb / MAX(vars.vout_fb, EPSILON) - filt.G_est) * params.filt.w0_G_est * params.sys.samp.ts0;
}


// Vout control via Tsw
// ...........................................................................................
void VOUT_CTRL_Init()
{
    ctrl.vout.kT = (params.llc.range.Tsw.max - params.llc.range.Tsw.min) /
                   (params.llc.range.vout.max - params.llc.range.vout.min);
    ctrl.vout.kd = 1.0f / POW_TWO(params.llc.range.vout.min);
    PI_UpdateParams(&ctrl.vout.pi, params.ctrl.vout.kp, params.ctrl.vout.ki * params.sys.samp.ts0,
                    params.ctrl.vout.out_sat.min, params.ctrl.vout.out_sat.max);
}


void VOUT_CTRL_Reset()
{
    PI_IntegBackCalc(&ctrl.vout.pi, params.llc.range.vout.min, 0.0f, 0.0f);
}


void VOUT_CTRL_RunISR0()
{
    vars.vout_cmd_int = RateLimit(params.sys.rate_lim.vout_cmd * params.sys.samp.ts0, vars.vout_cmd_ext,
                                  vars.vout_cmd_int);
    PI_Run(&ctrl.vout.pi, vars.vout_cmd_int, vars.vout_fb, 0.0f);
    if (ctrl.vout.pi.output >= params.llc.range.vout.min)
    {
        vars.d_cmd_volt = 1.0f;
        vars.Tsw_cmd_volt = (ctrl.vout.pi.output - params.llc.range.vout.min) * ctrl.vout.kT + params.llc.range.Tsw.min;
    }
    else // (ctrl.vout.pi.output < params.llc.range.vout.min)
    {
        vars.Tsw_cmd_volt = params.llc.range.Tsw.min;
        vars.d_cmd_volt = POW_TWO(ctrl.vout.pi.output) * ctrl.vout.kd;
    }
    #if defined(PC_TEST)
    vars.test[2] = ctrl.vout.pi.output;
    #endif
}


// Tsw control
// ...........................................................................................
void TSW_CTRL_Reset()
{
    vars.Tsw_cmd_int = params.llc.range.Tsw.min;
}


void TSW_CTRL_RunISR0()
{
    switch (params.ctrl.id.mode)
    {
        default:
        case Voltage_Mode:
            vars.Tsw_cmd_int = RateLimit(params.sys.rate_lim.Tsw_cmd * params.sys.samp.ts0, vars.Tsw_cmd_volt,
                                         vars.Tsw_cmd_int);
            break;

        case Frequency_Mode:
            vars.Tsw_cmd_int = RateLimit(params.sys.rate_lim.Tsw_cmd * params.sys.samp.ts0, vars.Tsw_cmd_ext,
                                         vars.Tsw_cmd_int);
            break;
    }
}


// Duty cycle control
// ...........................................................................................
void D_CTRL_Reset()
{
    vars.d_cmd_int = PERC_TO_NORM(100.0f);
}


void D_CTRL_RunISR0()
{
    switch (params.ctrl.id.mode)
    {
        default:
        case Voltage_Mode:
            vars.d_cmd_int =
                RateLimit(params.sys.rate_lim.d_cmd * params.sys.samp.ts0, vars.d_cmd_volt, vars.d_cmd_int);
            break;

        case Duty_Cycle_Mode:
            vars.d_cmd_int = RateLimit(params.sys.rate_lim.d_cmd * params.sys.samp.ts0, vars.d_cmd_ext, vars.d_cmd_int);
            break;
    }
}


// Synchronous rectification control
// ...........................................................................................
void SR_CTRL_RunISR0()
{
    if (vars.sr_en)
    {
        vars.Td_on_sr  = LUT2DInterp((const LUT_2D_t*)&params.llc.sr.Td_on, filt.fsw, filt.G_est);
        vars.Td_off_sr = LUT2DInterp((const LUT_2D_t*)&params.llc.sr.Td_off, filt.fsw, filt.G_est);
    }
    else
    {
        vars.Td_on_sr = 0.0f;
        vars.Td_off_sr = 0.0f;
    }
}


// Dead-time control
// ...........................................................................................
void DT_CTRL_Reset()
{
    // Corresponding to maximum frequency and load current
    const uint32_t Default_LUT_Index = LUT_2D_WIDTH_X * LUT_2D_WIDTH_Y - 1U;
    vars.T_dt = (params.llc.dt.method == DT_Perc) ? params.llc.range.Tsw.min * params.llc.dt.perc :
                (params.llc.dt.method == DT_LUT) ? params.llc.dt.lut.z[Default_LUT_Index] : 0.0f;
}


void DT_CTRL_RunISR0()
{
    switch (params.llc.dt.method)
    {
        default:
        case DT_Perc:
            vars.T_dt = vars.Tsw_cmd_int * params.llc.dt.perc;
            break;

        case DT_LUT:
            vars.T_dt = LUT2DInterp((const LUT_2D_t*)&params.llc.dt.lut, filt.fsw, filt.G_est);
            break;
    }
}


// Fan control
// ...........................................................................................
void FAN_CTRL_Init()
{
    if (params.ctrl.fan.en)
    {
        ctrl.fan.spd_coeff = HZ_TO_RPM(1.0f) * 0.5f / params.ctrl.fan.cpr;
        ctrl.fan.lpf_coeff = params.ctrl.fan.w0_spd * params.sys.samp.ts0;

        sm.extra_callback.RunISR0 = FAN_CTRL_RunISR0;
        sm.extra_callback.RunISR1 = FAN_CTRL_RunISR1;
    }
}


void FAN_CTRL_Reset()
{
    ctrl.fan.lut_idx = 0;
    vars.spd_fb_fan = params.ctrl.fan.spd_lut[0];
    vars.spd_ref_fan = params.ctrl.fan.spd_lut[0];
    ctrl.fan.dT_fb = ctrl.fan.spd_coeff / vars.spd_fb_fan;
}


void FAN_CTRL_RunISR0()
{
    vars.spd_fb_fan += ((ctrl.fan.spd_coeff / ctrl.fan.dT_fb) - vars.spd_fb_fan) * ctrl.fan.lpf_coeff;
    vars.spd_ref_fan += (params.ctrl.fan.spd_lut[ctrl.fan.lut_idx] - vars.spd_ref_fan) * ctrl.fan.lpf_coeff;
}


void FAN_CTRL_RunISR1()
{
    if (ctrl.fan.lut_idx > 0)
    {
        if (vars.temp_fb < (params.ctrl.fan.temp_lut[ctrl.fan.lut_idx - 1] - params.ctrl.fan.temp_hyst))
        {
            ctrl.fan.lut_idx--;
        }
    }
    if (ctrl.fan.lut_idx < FAN_CTRL_LUT_WIDTH - 1)
    {
        if (vars.temp_fb > params.ctrl.fan.temp_lut[ctrl.fan.lut_idx])
        {
            ctrl.fan.lut_idx++;
        }
    }
    vars.d_cmd_fan = params.ctrl.fan.duty_lut[ctrl.fan.lut_idx];
}
