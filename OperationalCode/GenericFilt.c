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


#include "GenericFilt.h"

// ...........................................................................................................
// IIR-On (nth order)

void FILT_IIR_PoleZeroInit(FILT_IIR_t* filt, uint8_t order, float32_t fs, float32_t G,
                           const float32_t wz[], const float32_t wp[])
{
    filt->order = order;
    float32_t wz_inv[FILT_IIR_ORDER_MAX] = { 0.0f }, wp_inv[FILT_IIR_ORDER_MAX] = { 0.0f };
    for (uint8_t idx = 0; idx < filt->order; ++idx)
    {
        filt->wz[idx] = wz[idx];
        filt->wp[idx] = wp[idx];
        wz_inv[idx] = isinf(wz[idx]) ? 0.0f : 1.0f / wz[idx];
        wp_inv[idx] = isinf(wp[idx]) ? 0.0f : 1.0f / wp[idx];
    }
    filt->a[0] = 1.0f; filt->b[0] = 1.0f;
    for (uint8_t idx = 1; idx <= filt->order; ++idx)
    {
        filt->a[idx] = 0.0f;
        filt->b[idx] = 0.0f;
    }
    for (uint8_t iter = 0; iter < filt->order; ++iter)
    {
        for (uint8_t idx = iter + 1; idx > 0; --idx)
        {
            filt->a[idx] += wz_inv[iter] * filt->a[idx - 1];
            filt->b[idx] += wp_inv[iter] * filt->b[idx - 1];
        }
    }
    for (uint8_t idx = 0; idx <= filt->order; ++idx)
    {
        filt->a[idx] *= G;
    }
    FILT_IIR_ContinuousInit(filt, order, fs, filt->a, filt->b);
}


void FILT_IIR_ContinuousInit(FILT_IIR_t* filt, uint8_t order, float32_t fs, const float32_t a[], const float32_t b[])
{
    filt->order = order;
    float32_t b0_inv = 1.0f / b[0];
    for (uint8_t idx = 0U; idx <= filt->order; ++idx)
    {
        filt->a[idx] = a[idx] * b0_inv;
        filt->b[idx] = b[idx] * b0_inv;
        filt->m[idx] = 0.0f;
        filt->n[idx] = 0.0f;
    }
    float32_t comb, pol; // combination and polarity factors for binomial expansion
    float32_t f = 1.0f;  // powers of sampling frequency
    for (uint8_t iter = 0U; iter <= filt->order; ++iter)
    {
        comb = 1.0f;
        pol = 1.0f;
        for (uint8_t idx = 0U; idx <= iter; ++idx)
        {   // I'm trying to free your mind Neo
            filt->m[idx] += a[iter] * f * comb * pol;
            filt->n[idx] += b[iter] * f * comb * pol;
            comb *= (float32_t)(iter - idx) / (float32_t)(idx + 1);
            pol *= -1.0f;
        }
        f *= fs;
    }
    FILT_IIR_DiscreteInit(filt, order, filt->m, filt->n);
}


void FILT_IIR_DiscreteInit(FILT_IIR_t* filt, uint8_t order, const float32_t m[], const float32_t n[])
{
    filt->order = order;
    float32_t n0_inv = 1.0f / n[0];
    for (uint8_t idx = 0U; idx <= filt->order; ++idx)
    {
        filt->m[idx] = m[idx] * n0_inv;
        filt->n[idx] = n[idx] * n0_inv;
    }
}


void FILT_IIR_Reset(FILT_IIR_t* filt, float32_t y)
{
    float32_t m_sum = 0.0f;
    for (uint8_t idx = 0U; idx <= filt->order; ++idx)
    {
        m_sum += filt->m[idx];
    }
    float32_t d = y / m_sum;
    for (uint8_t idx = 0U; idx <= filt->order; ++idx)
    {
        filt->d[idx] = d;
    }
}


RAMFUNC_BEGIN
float32_t FILT_IIR_Run(FILT_IIR_t* filt, float32_t x)
{
    float32_t nd_sum = 0.0f, md_sum = 0.0f;
    for (uint8_t idx = filt->order; idx > 0U; --idx)
    {
        filt->d[idx] = filt->d[idx - 1];
        nd_sum += filt->n[idx] * filt->d[idx];
        md_sum += filt->m[idx] * filt->d[idx];
    }
    filt->d[0] = x - nd_sum;
    md_sum += filt->m[0] * filt->d[0];

    return md_sum;
}


RAMFUNC_END


// ...........................................................................................................
// IIR 3rd Order

RAMFUNC_BEGIN
float32_t FILT_IIR_O3_Run(FILT_IIR_t* filt, float32_t x)
{
    filt->d[3] = filt->d[2]; filt->d[2] = filt->d[1]; filt->d[1] = filt->d[0];
    filt->d[0] = x - filt->n[1] * filt->d[1] - filt->n[2] * filt->d[2] - filt->n[3] * filt->d[3];
    return (filt->m[0] * filt->d[0] + filt->m[1] * filt->d[1] + filt->m[2] * filt->d[2] + filt->m[3] * filt->d[3]);
}


RAMFUNC_END


// ...........................................................................................................
// IIR 2nd Order

RAMFUNC_BEGIN
float32_t FILT_IIR_O2_Run(FILT_IIR_t* filt, float32_t x)
{
    filt->d[2] = filt->d[1]; filt->d[1] = filt->d[0];
    filt->d[0] = x - filt->n[1] * filt->d[1] - filt->n[2] * filt->d[2];
    return (filt->m[0] * filt->d[0] + filt->m[1] * filt->d[1] + filt->m[2] * filt->d[2]);
}


RAMFUNC_END


// ...........................................................................................................
// IIR 1st Order

RAMFUNC_BEGIN
float32_t FILT_IIR_O1_Run(FILT_IIR_t* filt, float32_t x)
{
    filt->d[1] = filt->d[0];
    filt->d[0] = x - filt->n[1] * filt->d[1];
    return (filt->m[0] * filt->d[0] + filt->m[1] * filt->d[1]);
}


RAMFUNC_END
