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

MULT_CORE_SHM_SUB(".sensor_iface") MULT_CORE_VLT SENSOR_IFACE_t sensor_iface = { 0 };

static inline void ApplyFilter(MULT_CORE_VLT ANALOG_SENSOR_t* analog_sens)
{
    analog_sens->filt += (analog_sens->raw - analog_sens->filt) * analog_sens->filt_coeff;
}


void SENSOR_IFACE_Init()
{
    // Analog sensors' initializations
    sensor_iface.vin.filt_coeff = params.sys.analog.filt.w0_vin * params.sys.samp.ts1;
    sensor_iface.vout.filt_coeff = params.sys.analog.filt.w0_vout * params.sys.samp.ts0;
    sensor_iface.iout.filt_coeff = params.sys.analog.filt.w0_iout * params.sys.samp.ts0;
    sensor_iface.cmd.filt_coeff = params.sys.analog.filt.w0_cmd * params.sys.samp.ts1;
    sensor_iface.temp.filt_coeff = params.sys.analog.filt.w0_temp * params.sys.samp.ts1;
}


void SENSOR_IFACE_Reset()
{
    // Analog sensors' filter reset
    sensor_iface.vin.filt = 0.0f;
    sensor_iface.vout.filt = 0.0f;
    sensor_iface.iout.filt = 0.0f;
    sensor_iface.cmd.filt = 0.0f;
    sensor_iface.temp.filt = 0.0f;
}


RAMFUNC_BEGIN
void SENSOR_IFACE_RunISR0()
{
    // Apply filters
    ApplyFilter(&sensor_iface.vout);
    ApplyFilter(&sensor_iface.iout);

    // Update variables
    vars.vout_fb = sensor_iface.vout.filt;
    vars.iout_fb = sensor_iface.iout.filt;
}


RAMFUNC_END

void SENSOR_IFACE_RunISR1()
{
    // Apply filters
    ApplyFilter(&sensor_iface.vin);
    ApplyFilter(&sensor_iface.cmd);
    ApplyFilter(&sensor_iface.temp);

    // Update variables
    vars.vin_fb = sensor_iface.vin.filt;
    vars.cmd_int = sensor_iface.cmd.filt;
    vars.temp_fb = sensor_iface.temp.filt;

    // Digital inputs
    sensor_iface.digital.user_sw ^= FALL_EDGE(sensor_iface.digital.user_btn_prev, sensor_iface.digital.user_btn);
    sensor_iface.digital.user_btn_prev = sensor_iface.digital.user_btn;

    // Command
    switch (params.sys.cmd.source)
    {
        default:
        case External:
            vars.cmd_final = vars.cmd_ext;
            break;

        case Internal:
            vars.cmd_final = vars.cmd_int;
            break;
    }

    #if defined(PC_TEST)
    vars.test[0] = sensor_iface.cmd.raw;
    vars.test[1] = vars.cmd_int;
    #endif

    switch (params.ctrl.id.mode)
    {
        default:
        case Voltage_Mode:
            vars.vout_cmd_ext = SAT(params.sys.cmd.vout.lim.min, params.sys.cmd.vout.lim.max,
                                    vars.cmd_final * params.sys.cmd.vout.tgt);
            break;

        case Frequency_Mode:
            ScalarBlend(vars.cmd_final, params.llc.range.Tsw.max, params.llc.range.Tsw.min, &vars.Tsw_cmd_ext);
            break;

        case Duty_Cycle_Mode:
            ScalarBlend(vars.cmd_final, PERC_TO_NORM(100.0f), PERC_TO_NORM(0.0f), &vars.d_cmd_ext);
            break;
    }
}
