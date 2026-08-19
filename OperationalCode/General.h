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

#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>

#if !defined(LLC_APP_PRESENT)
// For testing only, "false" ensures parameters are always overwritten
#define PARAMS_LOAD_FLASH   (true)
#define KIT_ID              (0x0000U)   // Kit ID
/* MISRA rule 6.3 recommends using specific-length typedef for the basic
 * numerical types of signed and unsigned variants of char, float, and double.
 */
typedef char     char_t;    /**< Specific-length typedef for the basic numerical types of char */
typedef float    float32_t; /**< Specific-length typedef for the basic numerical types of float */
typedef double   float64_t; /**< Specific-length typedef for the basic numerical types of double */
#else
#include "HardwareConfig.h"
#include "cy_syslib.h"
#endif

#if !defined(MULTI_CORE_PRESENT)
#define MULT_CORE_ALIGN
#define MULT_CORE_VLT
#define MULT_CORE_SHM                            // multi core shared memory
#define MULT_CORE_SHM_SUB(subsection)
#else
#include "cy_ppca_shm.h"
#define MULT_CORE_ALIGN      CY_ALIGN(4)
#define MULT_CORE_VLT        volatile
#define MULT_CORE_SHM        CY_SECTION_PPCA_SHM
#define MULT_CORE_SHM_SUB(subsection)   CY_SECTION_PPCA_SHM_SUB(subsection)
#endif

#define PRBS_ORDER      (10U)
#define PRBS_DATA_LEN   (1U << PRBS_ORDER)      // first index is eliminated

#define SYSID_ORDER     (5U)
#define SYSID_DATA_LEN  (1U << SYSID_ORDER)
#define SYSID_DUTY_MAX  (PERC_TO_NORM(100.0f))

#define POW_TWO(x)      ((x)*(x))
#define POW_THREE(x)    ((x)*(x)*(x))
#define MIN(x, y)        (((x)<(y))?(x):(y))
#define MAX(x, y)        (((x)<(y))?(y):(x))
#define SAT(xl, xh, x)    (((x)<(xl))?(xl):(((x)>(xh))?(xh):(x)))
#define ABS(x)          (fabsf(x))  // always compiling to VABS.F32 in Cortex-M4
#define SIGN(x)         (((x)>=0.0f)?(+1.0f):(-1.0f))
#define IS_POS(x)       ((x)>0.0f)
#define IS_NEG(x)       ((x)<0.0f)
#define IS_BETWEEN(x, l, h)   (((x)>=l)&&((x)<=h))

#define RISE_EDGE(previous, current)     ((!(previous))&&(current))
#define FALL_EDGE(previous, current)     ((previous)&&(!(current)))
#define TRANS_EDGE(previous, current)    (RISE_EDGE(previous,current)||FALL_EDGE(previous,current))

#define AVE(x, y)                (((x)+(y))*(0.5f))
#define ABS_BELOW_LIM(x, lim)    (ABS(x)<(lim)) // lim must be positive
#define ABS_ABOVE_LIM(x, lim)    (ABS(x)>(lim)) // lim must be positive
#define ROUND_FLOAT_TO_INT(x)   ((int32_t)((x)+(((x)>=0.0f)?(+0.5f):(-0.5f))))
#define QUANTIZE_FLOAT(x, q)     ((float32_t)(ROUND_FLOAT_TO_INT((x)/(q)))*(q))

#define SQRT_TWO                (1.414213562373095f)
#define SQRT_THREE              (1.732050807568877f)
#define SQRT_THREE_OVER_TWO     (0.866025403784439f)
#define SQRT_TWO_OVER_THREE     (0.471404520791032f)
#define ONE_OVER_SQRT_TWO       (0.707106781186547f)
#define ONE_OVER_SQRT_THREE     (0.577350269189626f)
#define TWO_OVER_SQRT_THREE     (1.154700538379252f)
#define EXP_ONE                 (2.718281828459046f)
#define EXP_MINUS_ONE           (0.367879441171442f)
#define LOG_TWO_E               (1.442695040888963f)
#define LOG_TEN_E               (0.434294481903252f)
#define LN_TWO                  (0.693147180559945f)
#define LN_TEN                  (2.302585092994046f)
#define TWO_PI                  (6.283185307179586f)
#define TWO_PI_OVER_THREE       (2.094395102393195f)
#define FIVE_PI_OVER_SIX        (2.617993877991494f)
#define PI                      (3.141592653589793f)
#define PI_OVER_TWO             (1.570796326794897f)
#define PI_OVER_THREE           (1.047197551196598f)
#define PI_OVER_FOUR            (0.785398163397448f)
#define PI_OVER_SIX             (0.523598775598299f)
#define PI_OVER_TWELVE          (0.261799387799149f)
#define ONE_OVER_PI             (0.318309886183791f)
#define ONE_OVER_TWO_PI         (0.159154943091895f)
#define ONE_OVER_THREE_PI       (0.106103295394597f)
#define TWO_OVER_PI             (0.636619772367581f)
#define THREE_OVER_PI           (0.954929658551372f)
#define SIX_OVER_PI             (1.909859317102744f)
#define EPSILON                 (1.0E-10f)

#define SCALE_PI_TO_INT32   ((float32_t)(INT32_MAX) / PI)
#define SCALE_INT32_TO_DEG  (180.0f / (float32_t)(INT32_MAX))

#define LUT_1D_N            (7)
#define LUT_1D_WIDTH        (1<<LUT_1D_N)

#define LUT_2D_N_X          (3)
#define LUT_2D_N_Y          (3)
#define LUT_2D_WIDTH_X      (1<<LUT_2D_N_X)
#define LUT_2D_WIDTH_Y      (1<<LUT_2D_N_Y)

#define TRIG_LUT_N          (6)
#define TRIG_LUT_WIDTH      (1<<TRIG_LUT_N)

#define INV_TRIG_LUT_N      (5)
#define INV_TRIG_LUT_WIDTH  (1<<INV_TRIG_LUT_N)

#define TEMP_SENS_LUT_N     (4)
#define TEMP_SENS_LUT_WIDTH (1<<TEMP_SENS_LUT_N)

#define FAN_CTRL_LUT_N      (2)
#define FAN_CTRL_LUT_WIDTH  (1<<FAN_CTRL_LUT_N)

#define HZ_TO_RADSEC(x)     (TWO_PI*(x))
#define RADSEC_TO_HZ(x)     (ONE_OVER_TWO_PI*(x))
#define TAU_TO_RADSEC(x)    (1.0f/(x))
#define RADSEC_TO_TAU(x)    (1.0f/(x))
#define PERIOD_TO_RADSEC(x) (TWO_PI/(x))
#define RADSEC_TO_PERIOD(x) (TWO_PI/(x))
#define HZ_TO_PERIOD(x)     (1.0f/(x))
#define PERIOD_TO_HZ(x)     (1.0f/(x))
#define DISABLE_LPF_FS(x)   HZ_TO_RADSEC((x)*ONE_OVER_TWO_PI)   // y+=(x-y)*w0*Ts; w0=fs => y=x i.e. fmax=fs/TWO_PI
#define DISABLE_DBF_FS(x)   HZ_TO_PERIOD(x) // y=x i.e. debouncing is bypassed
#define RPM_TO_HZ(x)        ((x)*(1.0f/60.0f))
#define HZ_TO_RPM(x)        ((x)*(60.0f))
#define DEG_TO_RAD(x)       ((x)*(PI/180.0f))
#define RAD_TO_DEG(x)       ((x)*(180.0f/PI))
#define RMS_TO_PK(x)        ((x)*SQRT_TWO)
#define PK_TO_RMS(x)        ((x)*ONE_OVER_SQRT_TWO)
#define PHASE_TO_LINE(x)    ((x)*SQRT_THREE)
#define LINE_TO_PHASE(x)    ((x)*ONE_OVER_SQRT_THREE)
#define BIT_TO_FLOAT(word, bit_mask, one_val, zero_val)    (((word)&(bit_mask))?(one_val):(zero_val))
#define PERC_TO_NORM(x)     ((x)*0.01f)
#define NORM_TO_PERC(x)     ((x)*100.0f)

#define BYTE_TO_WORD(byte, index)    ((byte)<<((index)*8U))
#define WORD_TO_BYTE(word, index)    (((word)>>((index)*8U))&(0xFF))
#define THREE_BYTES_TO_WORD(byte0, byte1, byte2)  (BYTE_TO_WORD(byte0,0U)|BYTE_TO_WORD(byte1,1U)|BYTE_TO_WORD(byte2,2U))

#define STRUCT_TO_ARRAY(instance)   ((float32_t*)(&instance))   // use pointer casting carefully

#if defined(LLC_APP_PRESENT) && defined(RAMFUNC_ENABLE)
#include "cy_utils.h"
#define RAMFUNC_BEGIN   CY_RAMFUNC_BEGIN
#define RAMFUNC_END     CY_RAMFUNC_END
#else
#define RAMFUNC_BEGIN
#define RAMFUNC_END
#endif

#if defined(SIL_TEST) && !defined(__cplusplus)
#define STATIC_ASSERT(cond, msg) _Static_assert(cond,msg)
#else
#define STATIC_ASSERT(cond, msg) static_assert(cond,msg)
#endif

#pragma pack(push,4)

typedef struct
{
    float32_t u;
    float32_t v;
    float32_t w;
} UVW_t;

typedef struct
{
    float32_t x;
    float32_t y;
    float32_t z;
} XYZ_t;

typedef struct
{
    float32_t alpha;
    float32_t beta;
} AB_t;

typedef struct
{
    float32_t q;
    float32_t d;
} QD_t;

typedef struct
{
    float32_t min;
    float32_t max;
} MINMAX_t;

typedef struct
{
    float32_t sine;
    float32_t cosine;
} PARK_t;

typedef struct
{
    float32_t rad;
    float32_t theta;
} POLAR_t;

#pragma pack(pop)

typedef struct
{
    float32_t x_min;            // min x
    float32_t x_max;            // max x
    float32_t x_step;           // =(x_max-x_min)/(LUT_1D_WIDTH-1)
    float32_t x_step_inv;       // =1/x_step
    float32_t y[LUT_1D_WIDTH];  // y-axis, output
} LUT_1D_t;                     // look up table 1 dimension

typedef struct
{
    float32_t x_min;            // min x
    float32_t x_max;            // max x
    float32_t x_step;           // =(x_max-x_min)/(LUT_2D_WIDTH-1)
    float32_t x_step_inv;       // =1/x_step
    float32_t y_min;            // min y
    float32_t y_max;            // max y
    float32_t y_step;           // =(y_max-y_min)/(LUT_2D_WIDTH-1)
    float32_t y_step_inv;       // =1/y_step
    float32_t z[LUT_2D_WIDTH_X*LUT_2D_WIDTH_Y];     // z-axis, output
} LUT_2D_t;                     // look up table 2 dimensions

typedef struct
{
    // PI with back-calculation type anti-windup
    // H(s)=kp+ki/s
    float32_t kp;
    float32_t ki;
    float32_t output_min;
    float32_t output_max;

    float32_t integ;

    float32_t ff;
    float32_t error;
    float32_t output;
} PI_t;

typedef struct
{
    float32_t prev_input;
    float32_t integ;
} BILINEAR_INTEG_t;

typedef enum
{
    Dis = 0,
    En = 1
} EN_DIS_t;

typedef enum
{
    Task_Waiting = 0U,
    Task_Started,
    Task_Finished,
    Task_Error
} TASK_STATUS_t;

typedef enum
{
    Slow = 0U,
    Moderate,
    Fast
} SPEED_ATTRIB_t;

typedef struct
{
    // theta is in [0,pi/2]
    float32_t th_step;
    float32_t th_step_inv;
    float32_t val[TRIG_LUT_WIDTH];
} TRIG_LUT_t;

typedef struct
{
    // input is in [0,1]
    // output is in [0, pi/4] for atan and [0, pi/2] for asin
    float32_t step;
    float32_t step_inv;
    float32_t val[INV_TRIG_LUT_WIDTH];
} INV_TRIG_LUT_t;

typedef struct
{
    // input is in [step,1-step], excluding {0,1} asymptotic infinities
    float32_t step;
    float32_t step_inv;
    float32_t val[TEMP_SENS_LUT_WIDTH];
} TEMP_SENS_LUT_t;

typedef struct
{
    float32_t time_thresh;      // [sec]
    float32_t run_period;       // [sec]
    uint32_t time_thresh_ticks; // [#]
    uint32_t time_ticks;        // [#]
} TIMER_t;

typedef struct
{
    float32_t cnt;
    float32_t sigma_x;
    float32_t sigma_xx;
    float32_t sigma_xy;
    float32_t sigma_y;
    float32_t sigma_yy;
    float32_t var_xx;   // *N^2, variance
    float32_t var_yy;   // *N^2, variance
    float32_t cov_xy;   // *N^2, covariance
    float32_t m;        // slope, y = mx + c
    float32_t c;        // intercept, y = mx + c
    float32_t r;        // correlation
} LIN_REG_t;

typedef struct
{   // Pseudo Random Binary Sequence (PRBS)
    uint8_t order;      // Polynomial order (= number of bits)
    uint8_t term[4U];   // Polynomial terms (= feedback bits)
    uint8_t shift[4U];  // Bit shifts
    uint32_t mask[4U];  // Bit masks
    uint32_t period;    // PRBS period
    uint32_t lfsr;      // Linear Feedback Shift Register (LFSR)
} PRBS_t;

extern UVW_t UVW_Zero;
extern UVW_t UVW_One;
extern UVW_t UVW_Half;
extern AB_t AB_Zero;
extern QD_t QD_Zero;
extern MINMAX_t MinMax_Zero;
extern PARK_t Park_Zero;
extern POLAR_t Polar_Zero;

void EmptyFcn();
bool AlwaysTrue();

void PI_Reset(MULT_CORE_VLT PI_t* pi);
void PI_UpdateParams(MULT_CORE_VLT PI_t* pi, const float32_t kp, const float32_t ki,
                     const float32_t output_min, const float32_t output_max);
// With feed forward
void PI_Run(MULT_CORE_VLT PI_t* pi, const float32_t cmd, const float32_t fb, const float32_t ff);
void PI_IntegBackCalc(MULT_CORE_VLT PI_t* pi, const float32_t output, const float32_t error, const float32_t ff);

void BILINEAR_INTEG_Reset(MULT_CORE_VLT BILINEAR_INTEG_t* bilinear, const float32_t integ_val);
float32_t BILINEAR_INTEG_Run(MULT_CORE_VLT BILINEAR_INTEG_t* bilinear, const float32_t input);

void ClarkeTransform(const UVW_t* input, MULT_CORE_VLT AB_t* output);
void ClarkeTransformInv(const AB_t* input, MULT_CORE_VLT UVW_t* output);

void ParkInit(const float32_t angle, MULT_CORE_VLT PARK_t* park);
void ParkTransform(const AB_t* input, const PARK_t* park, MULT_CORE_VLT QD_t* output);
void ParkTransformInv(const QD_t* input, const PARK_t* park, MULT_CORE_VLT AB_t* output);

float32_t ATan2(const float32_t y, const float32_t x);
float32_t ASin(const float32_t y);  // output is in [-pi/2,+pi/2]
float32_t ACos(const float32_t x);  // output is in [0,+pi]

void ToPolar(const float32_t x, const float32_t y, MULT_CORE_VLT POLAR_t* polar);

float32_t LUT1DInterp(const LUT_1D_t* lut, const float32_t input);

void LUT2DInit(MULT_CORE_VLT LUT_2D_t* lut, const MINMAX_t x_lim, const MINMAX_t y_lim,
               const float32_t z[LUT_2D_WIDTH_X* LUT_2D_WIDTH_Y]);
float32_t LUT2DInterp(const LUT_2D_t* lut, const float32_t x, const float32_t y);

float32_t SlopeIntercept(const float32_t slope, const float32_t intercept, const float32_t x);

// ratio must be within [0-1]
void ScalarBlend(const float32_t ratio, const float32_t x1, const float32_t x2, MULT_CORE_VLT float32_t* x);
// angles must be within [-pi,pi)
void AngleBlend(const float32_t ratio, const float32_t th1, const float32_t th2, MULT_CORE_VLT float32_t* th);
void PolarBlend(const float32_t ratio, const POLAR_t* polar1, const POLAR_t* polar2, MULT_CORE_VLT POLAR_t* result);
float32_t Wrap2Pi(const float32_t th);  // in range [-pi,pi)

// rate must be positive, limits positive and negative slopes
float32_t RateLimit(const float32_t rate, const float32_t target, const float32_t current);

void StopWatchInit(MULT_CORE_VLT TIMER_t* timer, const float32_t time_thresh, const float32_t run_period);
void StopWatchReset(MULT_CORE_VLT TIMER_t* timer);          // resets the timer
void StopWatchRun(MULT_CORE_VLT TIMER_t* timer);
bool StopWatchIsDone(MULT_CORE_VLT TIMER_t* timer);         // returns true if it is time
float32_t StopWatchGetTime(MULT_CORE_VLT TIMER_t* timer);   // get time in [sec]

extern void(*const DebounceFiltInit)(MULT_CORE_VLT TIMER_t* timer, const float32_t time_thresh,
                                     const float32_t run_period);
extern void(*const DebounceFiltReset)(MULT_CORE_VLT TIMER_t* timer);
extern void(*const DebounceFiltInc)(MULT_CORE_VLT TIMER_t* timer);
extern bool(*const DebounceFiltIsSet)(MULT_CORE_VLT TIMER_t* timer);
extern float32_t(*const DebounceFiltGetTime)(MULT_CORE_VLT TIMER_t* timer);
void DebounceFiltDec(MULT_CORE_VLT TIMER_t* timer);
bool DebounceFiltIsClear(MULT_CORE_VLT TIMER_t* timer);
bool DebounceFiltIncDec(const bool condition, MULT_CORE_VLT TIMER_t* timer);

void LinearRegressionReset(MULT_CORE_VLT LIN_REG_t* lin_reg);
void LinearRegressionAddDataPoint(MULT_CORE_VLT LIN_REG_t* lin_reg, const float32_t x, const float32_t y);
void LinearRegressionProcessData(MULT_CORE_VLT LIN_REG_t* lin_reg);

// Gallois form (modular form) PRBS generation
void PseudoRandBinaryInit(MULT_CORE_VLT PRBS_t* prbs, const uint8_t order);
void PseudoRandBinaryReset(MULT_CORE_VLT PRBS_t* prbs);
bool PseudoRandBinaryGen(MULT_CORE_VLT PRBS_t* prbs);
