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

MULT_CORE_SHM_SUB(".params") MULT_CORE_VLT PARAMS_t params;
MULT_CORE_SHM_SUB(".kit_info") MULT_CORE_VLT KIT_INFO_t kit_info;
PARAMS_FCN_t params_fcn;
HW_FCN_t hw_fcn;

static const float32_t Td_sr[LUT_2D_WIDTH_X*LUT_2D_WIDTH_Y] =
{
    0.0E-9f, 0.0E-9f, 0.0E-9f,  0.0E-9f,  0.0E-9f,  0.0E-9f,  0.0E-9f,  0.0E-9f,
    0.3E-9f, 1.3E-9f, 2.3E-9f,  3.1E-9f,  3.9E-9f,  4.5E-9f,  5.2E-9f,  5.7E-9f,
    0.5E-9f, 2.6E-9f, 4.5E-9f,  6.2E-9f,  7.6E-9f,  8.9E-9f,  10.1E-9f, 11.1E-9f,
    0.8E-9f, 3.9E-9f, 6.7E-9f,  9.2E-9f,  11.3E-9f, 13.1E-9f, 14.7E-9f, 15.9E-9f,
    1.1E-9f, 5.3E-9f, 8.9E-9f,  12.1E-9f, 14.8E-9f, 17.0E-9f, 18.8E-9f, 20.1E-9f,
    1.3E-9f, 6.6E-9f, 11.1E-9f, 15.0E-9f, 18.1E-9f, 20.6E-9f, 22.4E-9f, 23.6E-9f,
    1.6E-9f, 7.9E-9f, 13.2E-9f, 17.7E-9f, 21.2E-9f, 23.8E-9f, 25.6E-9f, 26.6E-9f,
    1.9E-9f, 9.1E-9f, 15.3E-9f, 20.3E-9f, 24.1E-9f, 26.7E-9f, 28.3E-9f, 29.1E-9f,
};

static const float32_t T_dt[LUT_2D_WIDTH_X * LUT_2D_WIDTH_Y] =
{
    34.0E-9f, 34.0E-9f, 34.0E-9f, 34.0E-9f, 34.0E-9f, 34.0E-9f, 34.0E-9f, 34.0E-9f,
    32.0E-9f, 32.0E-9f, 32.0E-9f, 32.0E-9f, 32.0E-9f, 32.0E-9f, 32.0E-9f, 32.0E-9f,
    30.0E-9f, 30.0E-9f, 30.0E-9f, 30.0E-9f, 30.0E-9f, 30.0E-9f, 30.0E-9f, 30.0E-9f,
    28.0E-9f, 28.0E-9f, 28.0E-9f, 28.0E-9f, 28.0E-9f, 28.0E-9f, 28.0E-9f, 28.0E-9f,
    26.0E-9f, 26.0E-9f, 26.0E-9f, 26.0E-9f, 26.0E-9f, 26.0E-9f, 26.0E-9f, 26.0E-9f,
    24.0E-9f, 24.0E-9f, 24.0E-9f, 24.0E-9f, 24.0E-9f, 24.0E-9f, 24.0E-9f, 24.0E-9f,
    22.0E-9f, 22.0E-9f, 22.0E-9f, 22.0E-9f, 22.0E-9f, 22.0E-9f, 22.0E-9f, 22.0E-9f,
    20.0E-9f, 20.0E-9f, 20.0E-9f, 20.0E-9f, 20.0E-9f, 20.0E-9f, 20.0E-9f, 20.0E-9f,
};

// Example initialization of fan control LUTs:
// temp_lut[] =              40C          50C          60C
// duty_lut[] =       40 %    |    60 %    |     80%    |    100%
// spd_lut[]  =     9000 rpm  |  12000 rpm |  15000 rpm |  18000 rpm

// *SUSPEND-FORMATTING*
static const float32_t Temp_LUT[FAN_CTRL_LUT_WIDTH - 1U] = {      40.0f,    50.0f,    60.0f        };
static const float32_t Duty_LUT[FAN_CTRL_LUT_WIDTH]      = {    0.4f,     0.6f,     0.8f,     1.0f };
static const float32_t Spd_LUT[FAN_CTRL_LUT_WIDTH]       = { 9000.0f, 12000.0f, 15000.0f, 18000.0f };
// *RESUME-FORMATTING*

void PARAMS_Init()
{
    // Parameters are initialized and saved in flash if
    // - Flash read is not successful
    // - First time running the FW
    // - FW update causes parameter incompatibility
    bool params_id_check = hw_fcn.FlashRead((PARAMS_ID_t) { PARAMS_CODE, BUILD_CONFIG_ID, PARAMS_VER },
                                            (PARAMS_t*)&params);
    if (!params_id_check || !PARAMS_LOAD_FLASH)
    {
        params_fcn.InitManual();
        params_fcn.InitAutoCalc();
        params_fcn.InitOverWrite();
        hw_fcn.FlashWrite((PARAMS_t*)&params);
    }

    // kit_info.chip_id is read by hardware interface from the chip
    kit_info.parameter_version = params.id.ver;
    kit_info.firmware_version = FIRMWARE_VER;
    kit_info.kit_id = KIT_ID;
    kit_info.build_config_id = BUILD_CONFIG_ID;
}


void PARAMS_InitManual()
{
    // LLC Parameters:
    // -> nominal ratings
    params.llc.nom.type = Full_Bridge; // [#]
    params.llc.nom.vin = 40.0f; // [V], dc
    params.llc.nom.vout = 5.0f; // [V], dc
    params.llc.nom.pout = 50.0f; // [W], dc
    params.llc.nom.ires = 2.4f; // [A], ac peak
    // -> circuit parameters
    params.llc.circuit.Lr = 2.6E-6f; // [H]
    params.llc.circuit.Cr = 36.0E-9f; // [F]
    params.llc.circuit.Lm = 12.5E-6f; // [H]
    params.llc.circuit.n = 7.0; // [#]
    params.llc.circuit.C = 1690.0E-6f; // [F]
    // -> operating range parameters
    params.llc.range.fsw = (MINMAX_t) { 524.0E3f, 635.0E3f }; // [Hz]
    params.llc.range.vout = (MINMAX_t) { 4.825f, 5.242f }; // [Hz]
    // -> synchronous rectification parameters
    params.llc.sr.en = Dis; // [], disabled by default, LUTs must be initialized by HW tests before enabling
    // -> dead-time modulation parameters
    params.llc.dt.method = DT_Perc; // []
    params.llc.dt.perc = PERC_TO_NORM(1.0f); // [%]

    // System Parameters:
    // -> command
    params.sys.cmd.source = Internal;
    params.sys.cmd.vout.tgt = 5.0f; // [V]
    // -> rate limiter
    params.sys.rate_lim.vout_cmd = params.sys.cmd.vout.tgt / 4E-3f; // [V/sec]
    // -> sampling
    params.sys.samp.fs0 = 50.0E3f; // [Hz]
    params.sys.samp.fs0_fs1_ratio = 5U; // []
    #if defined(PC_TEST)
    params.sys.samp.fsim_fs0_ratio = 10000U; // []
    #endif

    // Control Parameters:
    // --> general
    params.ctrl.id.mode = Voltage_Mode; // []
    params.ctrl.auto_start = En;        // [] Enable auto-start by default
    params.ctrl.soft_start.f = 635.0E3f;  // [Hz]
    params.ctrl.soft_start.t = 500.0E-6f; // [sec]
    params.ctrl.burst.f = 10.0E3f;      // [Hz]
    // --> output-voltage control
    params.ctrl.vout.bw = HZ_TO_RADSEC(500.0f); // [Ra/sec]
    // --> system identification
    params.ctrl.sys_id.en = true;
    // --> fan control
    params.ctrl.fan.en = false;
    params.ctrl.fan.f_pwm = 25.0E3f; // [Hz]
    params.ctrl.fan.cpr = 2.0f; // [#]
}


void PARAMS_InitAutoCalc()
{
    // Parameter IDs:
    params.id.code = PARAMS_CODE;
    params.id.build_config = BUILD_CONFIG_ID;
    params.id.ver = PARAMS_VER;

    // LLC Parameters:
    // -> nominal ratings
    params.llc.nom.iout = params.llc.nom.pout / params.llc.nom.vout; // [A]
    // -> circuit parameters
    params.llc.circuit.R = POW_TWO(params.llc.nom.vout) / params.llc.nom.pout; // [Ohm]
    // -> operating range parameters
    params.llc.range.Tsw = (MINMAX_t) { 1.0f / params.llc.range.fsw.max, 1.0f / params.llc.range.fsw.min }; // [sec]
    // -> synchronous rectification parameters
    const MINMAX_t G_Range = { 0.0f, PERC_TO_NORM(100.0f) / params.llc.circuit.R };
    LUT2DInit(&params.llc.sr.Td_on, params.llc.range.fsw, G_Range, Td_sr);
    LUT2DInit(&params.llc.sr.Td_off, params.llc.range.fsw, G_Range, Td_sr);
    // -> dead-time modulation parameters
    LUT2DInit(&params.llc.dt.lut, params.llc.range.fsw, G_Range, T_dt);

    // System Parameters:
    // -> command limits
    params.sys.cmd.vout.lim.max = params.llc.nom.vout * PERC_TO_NORM(120.0f); // [V], dc
    params.sys.cmd.vout.lim.min = 0.0f; // [V], dc
    // -> rate limiter
    params.sys.rate_lim.Tsw_cmd = (params.llc.range.Tsw.max - params.llc.range.Tsw.min) /
                                  AVE(params.llc.range.Tsw.max, params.llc.range.Tsw.min) / 20.0f;  // [sec/sec]
    params.sys.rate_lim.d_cmd = PERC_TO_NORM(100.0f) / params.llc.range.Tsw.min / 20.0f; // [%/sec]
    // --> sampling
    params.sys.samp.ts0 = 1.0f / params.sys.samp.fs0; // [sec]
    params.sys.samp.fs1 = params.sys.samp.fs0 / params.sys.samp.fs0_fs1_ratio; // [Hz]
    params.sys.samp.ts1 = 1.0f / params.sys.samp.fs1; // [sec]
    #if defined(PC_TEST)
    params.sys.samp.fsim = params.sys.samp.fs0 * params.sys.samp.fsim_fs0_ratio; // [Hz]
    params.sys.samp.tsim = 1.0f / params.sys.samp.fsim; // [sec]
    #endif
    // -> analog sensor
    params.sys.analog.filt.w0_vin = HZ_TO_RADSEC(1.0E3f); // [Ra/sec]
    params.sys.analog.filt.w0_vout = HZ_TO_RADSEC(5.0E3f); // [Ra/sec]
    params.sys.analog.filt.w0_iout = HZ_TO_RADSEC(1.0E3f); // [Ra/sec]
    params.sys.analog.filt.w0_cmd = HZ_TO_RADSEC(1.0E3f);   // [Ra/sec]
    params.sys.analog.filt.w0_temp = TAU_TO_RADSEC(0.500f); // [Ra/sec]
    // -> faults
    params.sys.faults.sw.vin.thresh.max = params.llc.nom.vin * PERC_TO_NORM(125.0f); // [V], dc
    params.sys.faults.sw.vin.thresh.min = params.llc.nom.vin * PERC_TO_NORM(75.0f); // [V], dc
    params.sys.faults.hw.vin.thresh.max = params.llc.nom.vin * PERC_TO_NORM(150.0f); // [V], dc
    params.sys.faults.hw.ires.thresh.max = params.llc.nom.ires * PERC_TO_NORM(150.0f); // [A], ac peak
    params.sys.faults.sw.vout.thresh.max = params.llc.nom.vout * PERC_TO_NORM(125.0f); // [V], dc
    params.sys.faults.hw.vout.thresh.max = params.llc.nom.vout * PERC_TO_NORM(150.0f); // [V], dc
    params.sys.faults.sw.iout.thresh.max = params.llc.nom.iout * PERC_TO_NORM(125.0f); // [A], dc
    params.sys.faults.hw.iout.thresh.max = params.llc.nom.iout * PERC_TO_NORM(150.0f); // [A], dc
    params.sys.faults.sw.temp.thresh.max = 110.0f; // [Celsius], dc
    params.sys.faults.sw.vout_err.thresh.max = params.sys.cmd.vout.tgt * PERC_TO_NORM(10.0f); // [V], dc
    params.sys.faults.sw.fan_err.thresh.max = 1000.0f; // [rpm]
    params.sys.faults.sw.vin.time = 0.002f; // [sec]
    params.sys.faults.hw.vin.time = 0.001f; // [sec]
    params.sys.faults.hw.ires.time = 0.001f; // [sec]
    params.sys.faults.sw.vout.time = 0.010f; // [sec]
    params.sys.faults.hw.vout.time = 0.001f; // [sec]
    params.sys.faults.sw.iout.time = 0.010f; // [sec]
    params.sys.faults.hw.iout.time = 0.001f; // [sec]
    params.sys.faults.sw.temp.time = 0.400f; // [sec]
    params.sys.faults.sw.vout_err.time = 5.0f; // [sec]
    params.sys.faults.sw.fan_err.time = 0.400f; // [sec]
    params.sys.faults.auto_clr_per = 1.0E-3f; // [sec]
    params.sys.faults.max_clr_tries = 10U; // []
    // --> LUTs
    params.sys.lut.sin.th_step = PI_OVER_TWO / (TRIG_LUT_WIDTH - 1); // [Ra]
    params.sys.lut.sin.th_step_inv = 1.0f / params.sys.lut.sin.th_step; // [1/Ra]
    for (uint32_t index = 0; index < TRIG_LUT_WIDTH; ++index)
    {
        params.sys.lut.sin.val[index] = sinf(index * params.sys.lut.sin.th_step); // [#]
    }
    float32_t inv_trig_lut_step = 1.0f / (INV_TRIG_LUT_WIDTH - 1); // [#]
    float32_t inv_trig_lut_step_inv = (INV_TRIG_LUT_WIDTH - 1); // [#]
    params.sys.lut.atan.step = inv_trig_lut_step; // [#]
    params.sys.lut.asin.step = inv_trig_lut_step; // [#]
    params.sys.lut.atan.step_inv = inv_trig_lut_step_inv; // [#]
    params.sys.lut.asin.step_inv = inv_trig_lut_step_inv; // [#]
    for (uint32_t index = 0; index < INV_TRIG_LUT_WIDTH; ++index)
    {
        float32_t inv_trig_lut_input = index * inv_trig_lut_step;
        params.sys.lut.asin.val[index] = asinf(inv_trig_lut_input); // [Ra]
        params.sys.lut.atan.val[index] = asinf(inv_trig_lut_input / sqrtf(1.0f + POW_TWO(inv_trig_lut_input))); // [Ra]
    }

    // Fitler Parameters:
    params.filt.w0_fsw = HZ_TO_RADSEC(1.0E3f);
    params.filt.w0_G_est = HZ_TO_RADSEC(1.0E3f);

    // Control Parameters:
    // --> general
    params.ctrl.boot_time = QUANTIZE_FLOAT(1.5f * params.sys.samp.ts1, params.sys.samp.ts1); // [sec]
    params.ctrl.burst.t = HZ_TO_PERIOD(params.ctrl.burst.f); // [sec]
    // --> output-voltage control
    params.ctrl.vout.kp = params.ctrl.vout.bw * 0.5f * params.llc.circuit.R * params.llc.circuit.C; // [V/V]=[]
    params.ctrl.vout.ki = params.ctrl.vout.bw; // [(Ra/sec).(V/V)]=[Ra/sec]
    params.ctrl.vout.kf = PERC_TO_NORM(0.0f); // [%]
    params.ctrl.vout.cmd_sat.max = params.llc.nom.vout * PERC_TO_NORM(125.0f); // [V]
    params.ctrl.vout.cmd_sat.min = 0.0f; // [sec]
    params.ctrl.vout.out_sat.max = params.llc.range.vout.max; // [sec]
    params.ctrl.vout.out_sat.min = 0.0f; // [sec]
    // --> system identification
    params.ctrl.sys_id.settle_time = 100.0f * (0.5f * params.llc.circuit.R * params.llc.circuit.C); // [sec]
    params.ctrl.sys_id.record_time = params.ctrl.sys_id.settle_time; // [sec]
    params.ctrl.sys_id.record_w0 = TAU_TO_RADSEC(0.1f * params.ctrl.sys_id.record_time); // [Ra/sec]
    // --> fan control
    params.ctrl.fan.temp_lut = Temp_LUT; // [C]
    params.ctrl.fan.duty_lut = Duty_LUT; // [%]
    params.ctrl.fan.spd_lut = Spd_LUT; // [rpm]
    params.ctrl.fan.temp_hyst = 0.5f;
    params.ctrl.fan.w0_spd = TAU_TO_RADSEC(0.500f); // [Ra/sec]
}
