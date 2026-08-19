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

// Latest firmware version, 0xAABC==vAA.B.C, Example: 0x0140==v1.4.0.
#define FIRMWARE_VER            (0x0010UL)      // firmware version, parameters can be retained while upgrading the FW
#define PARAMS_CODE             (~0xBADC0DEUL)  // do not change this code
#define PARAMS_VER              (0x0001UL)      // parameters version, change when 'params struct' changes
#define BUILD_CONFIG_ID         (0x0000UL)      // indicating build configuration

#include "General.h"

typedef struct
{
    uint32_t code;          // [], for detecting corrupt nvm data or first time booting up
    uint16_t build_config;  // [], changing build configuration should overwrite parameters
    uint16_t ver;           // [], changing parameters version should overwrite parameters
} PARAMS_ID_t;

typedef enum
{
    Half_Bridge = 0b001 << 0U,    // [],
    Full_Bridge = 0b010 << 0U     // [],
} LLC_TYPE_t;

typedef struct
{
    // TODO: consider per struct directives e.g. __ attribute __((aligned(4))
    MULT_CORE_ALIGN LLC_TYPE_t type;        // [#], llc type
    float32_t vin;                          // [V], dc, nominal input voltage
    float32_t vout;                         // [V], dc, nominal output voltage
    float32_t pout;                         // [W], dc, nominal output power
    float32_t iout;                         // [A], dc, nominal output current
    float32_t ires;                         // [A], ac peak, nominal primary-side resonant current
} LLC_NOM_PARAMS_t;

typedef struct
{
    float32_t Lr;        // [H], series inductance
    float32_t Cr;        // [F], series capacitance
    float32_t Lm;        // [H], transformer's magnetizing inductance
    float32_t n;         // [], transformer's turn ratio = n:1:1 = primary:secondary_1:secondary_2
    float32_t C;         // [F], output capacitance
    float32_t R;         // [Ohm], output load's equivalent resistance
} LLC_CIRCUIT_PARAMS_t;

typedef struct
{
    MINMAX_t fsw;     // [Hz], switching frequency range
    MINMAX_t Tsw;     // [sec], switching period range corresponding to the switching frequency range
    MINMAX_t vout;    // [V], output voltage range corresonding to the switching frequency range
} LLC_RANGE_PARAMS_t;

typedef struct
{
    // LUT2D inputs (X/Y) and outputs (Z):
    // X = Tsw [sec],   switching period
    // Y = iout [A],    output current (load)
    // Z = td [sec],    on-time / off-time delay, positive or negative

    EN_DIS_t en;        // [], enable/disable
    LUT_2D_t Td_on;     // []
    LUT_2D_t Td_off;    // []
} LLC_SR_PARAMS_t;

typedef enum
{
    DT_Perc = 0,    // [], percentage based dead-time modulation method
    DT_LUT          // [], look-up-table based dead-time modulation method
} DT_METHOD_t;

typedef struct
{
    DT_METHOD_t method; // [], dead-time modulation method, either percentage-based or LUT-based
    float32_t perc;     // [%], dead-time modulation percentage, valid when method is DT_Perc
    LUT_2D_t lut;       // [], dead-time modulation LUT, valid when method is DT_LUT
} LLC_DT_PARAMS_t;

typedef struct
{
    LLC_NOM_PARAMS_t nom;           // [], nominal ratings
    LLC_CIRCUIT_PARAMS_t circuit;   // [], circuit parameters
    LLC_RANGE_PARAMS_t range;       // [], operating range
    LLC_SR_PARAMS_t sr;             // [], synchronous rectification parameters
    LLC_DT_PARAMS_t dt;             // [], dead-time modulation parameters
} LLC_PARAMS_t;

typedef enum
{
    Internal = 0,                   // [], From potentiometer
    External                        // [], From GUI, UART, etc.
} CMD_SOURCE_t;

typedef struct
{
    float32_t tgt;              // [], The power device output reference, e.g. output voltage for voltage control mode
    MINMAX_t  lim;              // [], The limits for the power device output, e.g. min/max output voltage for voltage
                                // control mode
} CMD_TARGET_t;

typedef struct
{
    // TODO: consider per struct directives e.g. __ attribute __((aligned(4))
    MULT_CORE_ALIGN CMD_SOURCE_t source;    // [], command source
    CMD_TARGET_t vout;                      // [V], voltage command, output-voltage target
} CMD_PARAMS_t;

typedef struct
{
    float32_t vout_cmd;             // [V/sec], can act as soft start in voltage control mode
    float32_t Tsw_cmd;              // [sec/sec], can act as soft start in frequency control mode
    float32_t d_cmd;                // [%/sec], can act as soft start in duty-cycle control mode
} RATE_LIM_PARAMS_t;

typedef struct
{
    float32_t fs0;                  // [Hz], sampling frequency, isr0
    float32_t ts0;                  // [sec], sampling time, isr0
    uint32_t fs0_fs1_ratio;         // [], ratio of isr0 to isr1 frequency
    float32_t fs1;                  // [Hz], sampling frequency, isr1
    float32_t ts1;                  // [sec], sampling time, isr1
    #if defined(PC_TEST)
    uint32_t fsim_fs0_ratio;        // [], ratio of sim to isr0 frequency
    float32_t fsim;                 // [Hz], sampling frequency, simulations
    float32_t tsim;                 // [sec], sampling time, simulations
    #endif
} SAMPLE_PARAMS_t;

typedef struct
{
    float32_t w0_vin;       // [Ra/sec]
    float32_t w0_vout;      // [Ra/sec]
    float32_t w0_iout;      // [Ra/sec]
    float32_t w0_cmd;       // [Ra/sec]
    float32_t w0_temp;      // [Ra/sec]
} ANALOG_FILT_PARAMS_t;

typedef struct
{
    ANALOG_FILT_PARAMS_t filt;
} ANALOG_SENS_PARAMS_t;

typedef struct
{
    MINMAX_t thresh;        // [V,A,Hz,etc.], fault detection threshold
    float32_t time;         // [sec], fault detection time, debouncing filter
} FAULT_DET_PARAMS_t;

typedef struct
{   // Software faults, slow detection, lower thresholds
    FAULT_DET_PARAMS_t vin;         // [V], dc, input voltage
    FAULT_DET_PARAMS_t vout;        // [V], dc, output voltage
    FAULT_DET_PARAMS_t iout;        // [A], dc, output current
    FAULT_DET_PARAMS_t temp;        // [Celsius], dc, temperature sensor
    FAULT_DET_PARAMS_t vout_err;    // [V], dc, output voltage error (= command - feedback)
    FAULT_DET_PARAMS_t fan_err;     // [rpm], fan speed error (= expected - feedback)
} SW_FAULT_PARAMS_t;

typedef struct
{   // Hardware faults, fast detection, higher thresholds
    FAULT_DET_PARAMS_t vin;   // [V], dc, input voltage
    FAULT_DET_PARAMS_t ires;  // [A], peak, resonant current
    FAULT_DET_PARAMS_t vout;  // [V], dc, output voltage
    FAULT_DET_PARAMS_t iout;  // [A], peak, output current
} HW_FAULT_PARAMS_t;

typedef struct
{
    SW_FAULT_PARAMS_t sw;           // [], software faults, slow detection, lower thresholds
    HW_FAULT_PARAMS_t hw;           // [], hardware faults, fast detection, higher thresholds
    float32_t auto_clr_per;         // [sec], check period for trying to clear faults that are auto-clearable
    uint32_t max_clr_tries;         // [], maximum tries to clear faults and restart, before system permanently
                                    // goes to the faulted state
} FAULT_PARAMS_t;

typedef struct
{
    TRIG_LUT_t sin;                 // [#], sin lut for park transforms
    INV_TRIG_LUT_t atan;            // [#], atan lut for control
    INV_TRIG_LUT_t asin;            // [#], asin lut for control
} LUT_PARAMS_t;

typedef struct
{
    CMD_PARAMS_t cmd;               // [], command parameters
    RATE_LIM_PARAMS_t rate_lim;     // [], rate limiter parameters
    SAMPLE_PARAMS_t samp;           // [], sampling parameters
    ANALOG_SENS_PARAMS_t analog;    // [], analog sensor parameters
    FAULT_PARAMS_t faults;          // [], fault parameters
    LUT_PARAMS_t lut;               // [#], luts
} SYS_PARAMS_t;

typedef struct
{
    float32_t w0_fsw;       // [Ra/sec], for switching frequency
    float32_t w0_G_est;     // [Ra/sec], for estimated output conductance
} FILTER_PARAMS_t;

typedef enum
{
    Voltage_Mode = 0b001,       // controlling output-voltage via switching-frequency
    Frequency_Mode = 0b010,     // controlling switching-frequency directly
    Duty_Cycle_Mode = 0b100     // controlling duty-cycle while using fixed switching-frequency and fixed
                                // burst-frequency
} CTRL_MODE_t;

typedef union
{
    struct
    {
        uint32_t mode : 3;
        uint32_t reserved : 29;
    };

    uint32_t reg;
} CTRL_ID_t;

typedef struct
{
    float32_t bw;       // [Ra/sec], bandwidth
    float32_t kp;       // [output-unit/input-unit]
    float32_t ki;       // [(Ra/sec)*(output-unit/input-unit)]
    float32_t kf;       // [%], feed-forward weight
    MINMAX_t cmd_sat;   // [input-unit], command saturation limits
    MINMAX_t out_sat;   // [output-unit], output saturation limits
} PI_CTRL_PARAMS_t;

typedef struct
{
    bool en;                    // [], enable / disable
    float32_t settle_time;      // [sec], settling time to wait out before recording results
    float32_t record_time;      // [sec], recording time for filtering out the results
    float32_t record_w0;        // [Ra/sec], filter's bandwidth during recording time
} SYS_ID_PARAMS_t;

typedef struct
{
    float32_t f;    // [Hz], frequency
    float32_t t;    // [sec], time
} FRQ_TIME_PARAMS_t;

typedef struct
{
    bool en;                    // [], enable/disable, set to false if no fan exists
    const float32_t* temp_lut;  // [C], temperature thresholds at which speed is increased
    const float32_t* duty_lut;  // [%], duty cycle commands for increasing the speed
    const float32_t* spd_lut;   // [rpm], expected speeds for health monitoring

    float32_t temp_hyst;    // [C], temperature hysteresis to avoid jitter at the temperature thresholds
    float32_t f_pwm;        // [Hz], PWM frequency for applying the duty cycle
    float32_t cpr;          // [#], counts per mechanical revolutions, used for speed monitoring
    float32_t w0_spd;       // [Ra/sec], bandwidth of spead estimator's low pass filter
} FAN_CTRL_PARAMS_t;

typedef struct
{
    CTRL_ID_t id;                                   // []
    // TODO: consider per struct directives e.g. __ attribute __((aligned(4))
    MULT_CORE_ALIGN EN_DIS_t  auto_start;           // []
    float32_t boot_time;                            // [sec]
    FRQ_TIME_PARAMS_t soft_start;                   // []
    FRQ_TIME_PARAMS_t burst;                        // []
    PI_CTRL_PARAMS_t vout;                          // []
    SYS_ID_PARAMS_t sys_id;                         // []
    FAN_CTRL_PARAMS_t fan;                          // []
} CTRL_PARAMS_t;

typedef struct
{
    PARAMS_ID_t id;
    LLC_PARAMS_t llc;
    SYS_PARAMS_t sys;
    FILTER_PARAMS_t filt;
    CTRL_PARAMS_t ctrl;
} PARAMS_t;

#if defined(MULTI_CORE_PRESENT)
    #include <stddef.h>
STATIC_ASSERT(offsetof(PARAMS_t, ctrl.auto_start) % 4 == 0, "PARAMS_t.ctrl.auto_start must be 4-byte aligned");
STATIC_ASSERT(offsetof(PARAMS_t, sys.cmd.source)  % 4 == 0, "PARAMS_t.sys.cmd.source must be 4-byte aligned");
STATIC_ASSERT(offsetof(PARAMS_t, llc.nom.type)    % 4 == 0, "PARAMS_t.llc.nom.type must be 4-byte aligned");
#endif

typedef struct
{
    void (* InitManual)();    // manual parameters
    void (* InitAutoCalc)();  // auto-calculated parameters
    void (* InitOverWrite)(); // overwrite auto-calcualted parameters
} PARAMS_FCN_t;

#pragma pack(push,1)
typedef struct  // for external identification (e.g. GUI)
{
    uint32_t  chip_id;                  // chip ID
    uint16_t  parameter_version;        // parameter version
    uint16_t  firmware_version;         // firmware version
    uint8_t   kit_id;                   // kit ID
    uint8_t   build_config_id;          // build configuration ID
} KIT_INFO_t;
#pragma pack(pop)

extern MULT_CORE_VLT PARAMS_t params;
extern PARAMS_FCN_t params_fcn;

extern MULT_CORE_VLT KIT_INFO_t kit_info;

void PARAMS_Init();
void PARAMS_InitManual();
void PARAMS_InitAutoCalc();
