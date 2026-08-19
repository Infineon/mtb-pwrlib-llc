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

UVW_t UVW_Zero = { 0.0f, 0.0f, 0.0f };
UVW_t UVW_One = { 1.0f, 1.0f, 1.0f };
UVW_t UVW_Half = { 0.5f, 0.5f, 0.5f };
AB_t AB_Zero = { 0.0f, 0.0f };
QD_t QD_Zero = { 0.0f, 0.0f };
MINMAX_t MinMax_Zero = { 0.0f, 0.0f };
PARK_t Park_Zero = { 0.0f, 1.0f };
POLAR_t Polar_Zero = { 0.0f, 0.0f };

RAMFUNC_BEGIN
void EmptyFcn()
{
}


RAMFUNC_END

RAMFUNC_BEGIN
bool AlwaysTrue()
{
    return true;
}


RAMFUNC_END

void PI_Reset(MULT_CORE_VLT PI_t* pi)
{
    pi->integ = 0.0f;

    pi->ff = 0.0f;
    pi->error = 0.0f;
    pi->output = 0.0f;
}


void PI_UpdateParams(MULT_CORE_VLT PI_t* pi, const float32_t kp, const float32_t ki,
                     const float32_t output_min, const float32_t output_max)
{
    pi->kp = kp;
    pi->ki = ki;
    pi->output_min = output_min;
    pi->output_max = output_max;
}


RAMFUNC_BEGIN
void PI_Run(MULT_CORE_VLT PI_t* pi, const float32_t cmd, const float32_t fb, const float32_t ff)
{
    pi->error = cmd - fb;
    pi->ff = ff;
    pi->integ += pi->ki * pi->error;
    float32_t prop_ff = pi->error * pi->kp + pi->ff;
    float32_t output_raw = pi->integ + prop_ff;
    if (output_raw <= pi->output_min)
    {
        pi->output = pi->output_min;
        pi->integ = pi->output_min - prop_ff;
    }
    else if (output_raw >= pi->output_max)
    {
        pi->output = pi->output_max;
        pi->integ = pi->output_max - prop_ff;
    }
    else
    {
        pi->output = output_raw;
    }
}


RAMFUNC_END

void PI_IntegBackCalc(MULT_CORE_VLT PI_t* pi, const float32_t output, const float32_t error, const float32_t ff)
{
    pi->error = error;
    pi->ff = ff;
    pi->output = output;
    pi->integ = output - (error * pi->kp + ff);
}


void BILINEAR_INTEG_Reset(MULT_CORE_VLT BILINEAR_INTEG_t* bilinear, const float32_t integ_val)
{
    bilinear->integ = integ_val;
    bilinear->prev_input = 0.0f;
}


RAMFUNC_BEGIN
float32_t BILINEAR_INTEG_Run(MULT_CORE_VLT BILINEAR_INTEG_t* bilinear, const float32_t input)
{
    bilinear->integ += (bilinear->prev_input + input) * 0.5f;
    bilinear->prev_input = input;
    return bilinear->integ;
}


RAMFUNC_END

RAMFUNC_BEGIN
void ClarkeTransform(const UVW_t* input, MULT_CORE_VLT AB_t* output)
{
    output->alpha = input->u;
    output->beta = (input->w - input->v) * ONE_OVER_SQRT_THREE;
}


RAMFUNC_END

RAMFUNC_BEGIN
void ClarkeTransformInv(const AB_t* input, MULT_CORE_VLT UVW_t* output)
{
    output->u = input->alpha;
    output->v = -0.5f * input->alpha - SQRT_THREE_OVER_TWO * input->beta;
    output->w = -0.5f * input->alpha + SQRT_THREE_OVER_TWO * input->beta;
}


RAMFUNC_END

RAMFUNC_BEGIN
void ParkInit(const float32_t angle, MULT_CORE_VLT PARK_t* park)
{
    int32_t sector = (int32_t)((angle * TWO_OVER_PI) + (angle >= 0.0f ? 0.0f : -1.0f));
    float32_t th = angle - (sector * PI_OVER_TWO);
    uint32_t index_s = (uint32_t)(th * params.sys.lut.sin.th_step_inv);
    uint32_t index_c = (TRIG_LUT_WIDTH - 1U) - index_s;
    float32_t d_th = th - (index_s * params.sys.lut.sin.th_step);

    switch (sector & 0x3)
    {
        default:
        case 0x0: // sector 0
            park->sine = params.sys.lut.sin.val[index_s] + params.sys.lut.sin.val[index_c] * d_th;
            park->cosine = params.sys.lut.sin.val[index_c] - params.sys.lut.sin.val[index_s] * d_th;
            break;

        case 0x1: // sector 1
            park->sine = params.sys.lut.sin.val[index_c] - params.sys.lut.sin.val[index_s] * d_th;
            park->cosine = -params.sys.lut.sin.val[index_s] - params.sys.lut.sin.val[index_c] * d_th;
            break;

        case 0x2: // sector 2
            park->sine = -params.sys.lut.sin.val[index_s] - params.sys.lut.sin.val[index_c] * d_th;
            park->cosine = -params.sys.lut.sin.val[index_c] + params.sys.lut.sin.val[index_s] * d_th;
            break;

        case 0x3: // sector 3
            park->sine = -params.sys.lut.sin.val[index_c] + params.sys.lut.sin.val[index_s] * d_th;
            park->cosine = params.sys.lut.sin.val[index_s] + params.sys.lut.sin.val[index_c] * d_th;
            break;
    }
}


RAMFUNC_END

RAMFUNC_BEGIN
void ParkTransform(const AB_t* input, const PARK_t* park, MULT_CORE_VLT QD_t* output)
{
    output->q = input->alpha * park->cosine - input->beta * park->sine;
    output->d = input->alpha * park->sine + input->beta * park->cosine;
}


RAMFUNC_END

RAMFUNC_BEGIN
void ParkTransformInv(const QD_t* input, const PARK_t* park, MULT_CORE_VLT AB_t* output)
{
    output->alpha = input->q * park->cosine + input->d * park->sine;
    output->beta = -input->q * park->sine + input->d * park->cosine;
}


RAMFUNC_END

RAMFUNC_BEGIN
// Do not use outside of this source file
static inline float32_t InvTrigLUT(const float32_t input, INV_TRIG_LUT_t* lut)
{
    uint32_t index = SAT(0, INV_TRIG_LUT_WIDTH - 2, (int32_t)(input * lut->step_inv));
    float32_t result = lut->val[index] + (input - index * lut->step) * lut->step_inv *
                       (lut->val[index + 1] - lut->val[index]);
    return result;
}


RAMFUNC_END

RAMFUNC_BEGIN
float32_t ATan2(const float32_t y, const float32_t x)
{
    INV_TRIG_LUT_t* lut = (INV_TRIG_LUT_t*)&params.sys.lut.atan;
    uint8_t sector = (IS_POS(y) << 1) | IS_POS(x);
    float32_t theta;

    switch (sector)
    {
        default:
        case 0b11:
            theta = IS_POS(x - y) ? InvTrigLUT(y / x, lut) : PI_OVER_TWO - InvTrigLUT(x / y, lut);
            break;

        case 0b10:
            theta = IS_POS(x + y) ? PI_OVER_TWO + InvTrigLUT((-x) / y, lut) : PI - InvTrigLUT(y / (-x), lut);
            break;

        case 0b00:
            theta = IS_NEG(x - y) ? -PI + InvTrigLUT((-y) / (-x), lut) : -PI_OVER_TWO - InvTrigLUT((-x) / (-y), lut);
            break;

        case 0b01:
            theta = IS_NEG(x + y) ? -PI_OVER_TWO + InvTrigLUT(x / (-y), lut) : -InvTrigLUT((-y) / x, lut);
            break;
    }

    return theta;
}


RAMFUNC_END

RAMFUNC_BEGIN
float32_t ASin(const float32_t y)
{
    INV_TRIG_LUT_t* lut = (INV_TRIG_LUT_t*)&params.sys.lut.asin;

    float32_t theta = IS_NEG(y) ? -InvTrigLUT(-y, lut) : +InvTrigLUT(y, lut);

    return theta;
}


RAMFUNC_END

RAMFUNC_BEGIN
float32_t ACos(const float32_t x)
{
    INV_TRIG_LUT_t* lut = (INV_TRIG_LUT_t*)&params.sys.lut.asin;

    float32_t theta = IS_NEG(x) ? PI_OVER_TWO + InvTrigLUT(-x, lut) : PI_OVER_TWO - InvTrigLUT(x, lut);

    return theta;
}


RAMFUNC_END


RAMFUNC_BEGIN
void ToPolar(const float32_t x, const float32_t y, MULT_CORE_VLT POLAR_t* polar)
{
    float32_t rad_squared = x * x + y * y;
    polar->rad = sqrtf(rad_squared);
    polar->theta = ATan2(y, x);
}


RAMFUNC_END

RAMFUNC_BEGIN
float32_t LUT1DInterp(const LUT_1D_t* lut, const float32_t input)
{
    float32_t result;
    if (input < lut->x_min)
    {
        result = lut->y[0];
    }
    else if (input >= lut->x_max)
    {
        result = lut->y[LUT_1D_WIDTH - 1U];
    }
    else
    {
        uint32_t index = (uint32_t)((input - lut->x_min) * lut->x_step_inv);
        float32_t x_index = lut->x_min + index * lut->x_step;
        result = lut->y[index] + (input - x_index) * lut->x_step_inv * (lut->y[index + 1] - lut->y[index]);
    }
    return result;
}


RAMFUNC_END

RAMFUNC_BEGIN
static inline float32_t LUT2DGet(const LUT_2D_t* lut, const uint32_t x_idx, const uint32_t y_idx)
{   // no safety checks to speed up the execution, index arguments must be in the safe range
    return lut->z[y_idx * LUT_2D_WIDTH_Y + x_idx];
}


RAMFUNC_END

void LUT2DInit(MULT_CORE_VLT LUT_2D_t* lut, const MINMAX_t x_lim, const MINMAX_t y_lim,
               const float32_t z[LUT_2D_WIDTH_X* LUT_2D_WIDTH_Y])
{
    lut->x_min = x_lim.min;
    lut->x_max = x_lim.max;
    lut->x_step = (x_lim.max - x_lim.min) / (LUT_2D_WIDTH_X - 1U);
    lut->x_step_inv = 1.0f / lut->x_step;

    lut->y_min = y_lim.min;
    lut->y_max = y_lim.max;
    lut->y_step = (y_lim.max - y_lim.min) / (LUT_2D_WIDTH_Y - 1U);
    lut->y_step_inv = 1.0f / lut->y_step;

    for (uint32_t idx = 0; idx < LUT_2D_WIDTH_X * LUT_2D_WIDTH_Y; ++idx)
    {
        lut->z[idx] = z[idx];
    }
}


RAMFUNC_BEGIN
float32_t LUT2DInterp(const LUT_2D_t* lut, const float32_t x, const float32_t y)
{
    float32_t x_idxf = SAT(0.0f, (LUT_2D_WIDTH_X - 1.0f - EPSILON), (x - lut->x_min) * lut->x_step_inv);
    float32_t y_idxf = SAT(0.0f, (LUT_2D_WIDTH_Y - 1.0f - EPSILON), (y - lut->y_min) * lut->y_step_inv);

    uint32_t x_idx0 = (uint32_t)(x_idxf);
    uint32_t y_idx0 = (uint32_t)(y_idxf);

    uint32_t x_idx1 = x_idx0 + 1U;
    uint32_t y_idx1 = y_idx0 + 1U;

    float32_t dx_idx = x_idxf - x_idx0;
    float32_t dy_idx = y_idxf - y_idx0;

    float32_t z00 = LUT2DGet(lut, x_idx0, y_idx0);
    float32_t z10 = LUT2DGet(lut, x_idx1, y_idx0);
    float32_t z01 = LUT2DGet(lut, x_idx0, y_idx1);
    float32_t z11 = LUT2DGet(lut, x_idx1, y_idx1);

    float32_t zx0 = z00 + dx_idx * (z10 - z00);
    float32_t zx1 = z01 + dx_idx * (z11 - z01);
    float32_t zxy = zx0 + dy_idx * (zx1 - zx0);

    return zxy;
}


RAMFUNC_END

RAMFUNC_BEGIN
float32_t SlopeIntercept(const float32_t slope, const float32_t intercept, const float32_t x)
{
    return (slope * x + intercept);
}


RAMFUNC_END

void ScalarBlend(const float32_t ratio, const float32_t x1, const float32_t x2, MULT_CORE_VLT float32_t* x)
{
    *x = ratio * x1 + (1.0f - ratio) * x2;
}


void AngleBlend(const float32_t ratio, const float32_t th1, const float32_t th2, MULT_CORE_VLT float32_t* th)
{
    float32_t th1_uw, th2_uw;   // unwrapped
    if ((th1 >= PI_OVER_TWO) && (th2 < -PI_OVER_TWO))
    {
        th1_uw = th1;
        th2_uw = th2 + TWO_PI;
    }
    else if ((th1 < -PI_OVER_TWO) && (th2 >= PI_OVER_TWO))
    {
        th1_uw = th1 + TWO_PI;
        th2_uw = th2;
    }
    else
    {
        th1_uw = th1;
        th2_uw = th2;
    }

    float32_t th_uw = ratio * th1_uw + (1.0f - ratio) * th2_uw;
    *th = Wrap2Pi(th_uw);
}


void PolarBlend(const float32_t ratio, const POLAR_t* polar1, const POLAR_t* polar2, MULT_CORE_VLT POLAR_t* result)
{
    ScalarBlend(ratio, polar1->rad, polar2->rad, &result->rad);
    AngleBlend(ratio, polar1->theta, polar2->theta, &result->theta);
}


RAMFUNC_BEGIN
float32_t Wrap2Pi(const float32_t th)
{
    int32_t n = (int32_t)((th * ONE_OVER_TWO_PI) + (th >= 0.0f ? 0.5f : -0.5f));
    return (th - n * TWO_PI);
}


RAMFUNC_END

RAMFUNC_BEGIN
float32_t RateLimit(const float32_t rate, const float32_t target, const float32_t current)
{
    if (current >= target)
    {
        return MAX(current - rate, target);
    }
    else
    {
        return MIN(current + rate, target);
    }
}


RAMFUNC_END

void StopWatchInit(MULT_CORE_VLT TIMER_t* timer, const float32_t time_thresh, const float32_t run_period)
{
    timer->time_thresh = time_thresh;
    timer->run_period = run_period;
    timer->time_thresh_ticks = MAX(1UL, (uint32_t)(time_thresh / run_period));
    StopWatchReset(timer);
}


void StopWatchReset(MULT_CORE_VLT TIMER_t* timer)
{
    timer->time_ticks = 0U;
}


RAMFUNC_BEGIN
void StopWatchRun(MULT_CORE_VLT TIMER_t* timer)
{
    timer->time_ticks += (timer->time_ticks < timer->time_thresh_ticks);
}


RAMFUNC_END

RAMFUNC_BEGIN
bool StopWatchIsDone(MULT_CORE_VLT TIMER_t* timer)
{
    return (timer->time_ticks >= timer->time_thresh_ticks);
}


RAMFUNC_END

float32_t StopWatchGetTime(MULT_CORE_VLT TIMER_t* timer)
{
    return (timer->time_ticks * timer->run_period);
}


void(*const DebounceFiltInit)(MULT_CORE_VLT TIMER_t* timer, const float32_t time_thresh,
                              const float32_t run_period) = &StopWatchInit;
void(*const DebounceFiltReset)(MULT_CORE_VLT TIMER_t* timer) = &StopWatchReset;
void(*const DebounceFiltInc)(MULT_CORE_VLT TIMER_t* timer) = &StopWatchRun;
bool(*const DebounceFiltIsSet)(MULT_CORE_VLT TIMER_t* timer) = &StopWatchIsDone;
float32_t(*const DebounceFiltGetTime)(MULT_CORE_VLT TIMER_t* timer) = &StopWatchGetTime;

RAMFUNC_BEGIN
void DebounceFiltDec(MULT_CORE_VLT TIMER_t* timer)
{
    timer->time_ticks -= (timer->time_ticks > 0U);
}


RAMFUNC_END

RAMFUNC_BEGIN
bool DebounceFiltIsClear(MULT_CORE_VLT TIMER_t* timer)
{
    return (timer->time_ticks == 0U);
}


RAMFUNC_END

RAMFUNC_BEGIN
bool DebounceFiltIncDec(const bool condition, MULT_CORE_VLT TIMER_t* timer)
{
    if (condition)
    {
        DebounceFiltInc(timer);
    }
    else
    {
        DebounceFiltDec(timer);
    }
    return DebounceFiltIsSet(timer);
}


RAMFUNC_END

RAMFUNC_BEGIN
void LinearRegressionReset(MULT_CORE_VLT LIN_REG_t* lin_reg)
{
    lin_reg->cnt = 0.0f;
    lin_reg->sigma_x = 0.0f;
    lin_reg->sigma_xx = 0.0f;
    lin_reg->sigma_xy = 0.0f;
    lin_reg->sigma_y = 0.0f;
    lin_reg->sigma_yy = 0.0f;
}


RAMFUNC_END

RAMFUNC_BEGIN
void LinearRegressionAddDataPoint(MULT_CORE_VLT LIN_REG_t* lin_reg, const float32_t x, const float32_t y)
{
    lin_reg->cnt++;
    lin_reg->sigma_x += x;
    lin_reg->sigma_xx += POW_TWO(x);
    lin_reg->sigma_xy += x * y;
    lin_reg->sigma_y += y;
    lin_reg->sigma_yy += POW_TWO(y);
}


RAMFUNC_END

RAMFUNC_BEGIN
void LinearRegressionProcessData(MULT_CORE_VLT LIN_REG_t* lin_reg)
{
    lin_reg->var_xx = lin_reg->cnt * lin_reg->sigma_xx - lin_reg->sigma_x * lin_reg->sigma_x;
    lin_reg->var_yy = lin_reg->cnt * lin_reg->sigma_yy - lin_reg->sigma_y * lin_reg->sigma_y;
    lin_reg->cov_xy = lin_reg->cnt * lin_reg->sigma_xy - lin_reg->sigma_x * lin_reg->sigma_y;

    lin_reg->m = lin_reg->cov_xy / lin_reg->var_xx;
    lin_reg->r = lin_reg->cov_xy / sqrtf(lin_reg->var_xx * lin_reg->var_yy);
    lin_reg->c = (lin_reg->sigma_y - lin_reg->m * lin_reg->sigma_x) / lin_reg->cnt;
}


RAMFUNC_END

// Pseudo Random Binary Sequence (PRBS) Look Up Table (LUT)
static const uint8_t PRBS_LUT[15U][4U] =
{
    // Polynomial Terms,    Polynomial Order
    { 2U,  1U,  1U,  1U   }, // 02U
    { 3U,  2U,  2U,  2U   }, // 03U
    { 4U,  3U,  3U,  3U   }, // 04U
    { 5U,  4U,  3U,  2U   }, // 05U
    { 6U,  5U,  3U,  2U   }, // 06U
    { 7U,  6U,  5U,  4U   }, // 07U
    { 8U,  6U,  5U,  4U   }, // 08U
    { 9U,  8U,  6U,  5U   }, // 09U
    { 10U, 9U,  7U,  6U   }, // 10U
    { 11U, 10U, 9U,  7U   }, // 11U
    { 12U, 11U, 8U,  6U   }, // 12U
    { 13U, 12U, 10U, 9U   }, // 13U
    { 14U, 13U, 11U, 9U   }, // 14U
    { 15U, 14U, 13U, 11U  }, // 15U
    { 16U, 14U, 13U, 11U  }, // 16U
};

void PseudoRandBinaryInit(MULT_CORE_VLT PRBS_t* prbs, const uint8_t order)
{
    prbs->order = SAT(2U, 16U, order);
    prbs->period = ((uint32_t)(1U) << prbs->order) - 1U;
    for (uint8_t index = 0U; index < 4U; ++index)
    {
        prbs->term[index] = PRBS_LUT[prbs->order - 2U][index];
        prbs->shift[index] = prbs->order - prbs->term[index];
        prbs->mask[index] = (uint32_t)(1U) << prbs->shift[index];
    }
    PseudoRandBinaryReset(prbs);
}


void PseudoRandBinaryReset(MULT_CORE_VLT PRBS_t* prbs)
{
    prbs->lfsr = prbs->period; // seed value, 0b111...1
}


bool PseudoRandBinaryGen(MULT_CORE_VLT PRBS_t* prbs)
{
    uint32_t msb =
        (((prbs->lfsr) & (prbs->mask[0U])) >> (prbs->shift[0U])) ^
        (((prbs->lfsr) & (prbs->mask[1U])) >> (prbs->shift[1U])) ^
        (((prbs->lfsr) & (prbs->mask[2U])) >> (prbs->shift[2U])) ^
        (((prbs->lfsr) & (prbs->mask[3U])) >> (prbs->shift[3U]));
    (prbs->lfsr) = ((prbs->lfsr) >> 1U) | (msb << ((prbs->order) - 1U));
    return msb;
}
