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
#include "General.h"

// Features:
//
// 1 - IIR filters of any order (n) are supported:
//
//   A) Poles/zeros canonical form (G = dc gain):
//
//            G (s/wz1+1) (s/wz2+1) ... (s/wzn+1)
//     H(s)= --------------------------------------
//              (s/wp1+1) (s/wp2+1) ... (s/wpn+1)
//
//   B) Continuous-time canonical form (b0 normalized to 1):
//
//            a0 + a1*s + a2*s^2 + ... an*s^n
//     H(s)= ----------------------------------
//            b0 + b1*s + b2*s^2 + ... bn*s^n
//
//   C) Discrete-time canonical form (n0 normalized to 1):
//
//            m0 + m1*z^-1 + m2*z^-2 + ... mn*z^-n
//     H(s)= ----------------------------------
//            n0 + n1*z^-1 + n2*z^-2 + ... nn*z^-n
//
// 2-  Different types of initializations are supported:
//   A) Specify poles and zeros (wz's and wp's). Poles and zeros at infinity are supported!
//   B) Specify continuous-time coefficients (a's & b's).
//   C) Specify discrete-time coefficients (m's & n's).
//
// 3 - For IIR filter orders 1-3, use the order-specific APIs.
//   These are optimized for speed and memory.
//
// 4 - For IIR filter orders >3, use the generic API.
//   A) The maximum supported order can be set by adjusting FILT_IIR_ORDER_MAX.
//   B) Increasing FILT_IIR_ORDER_MAX will increase the memory consumption.
//   C) Use higher maximum orders only if needed.


#define FILT_IIR_ORDER_MAX  (3U)
#define FILT_IIR_LEN_MAX    (FILT_IIR_ORDER_MAX+1U)

typedef struct
{
    uint8_t order;
    float32_t wz[FILT_IIR_ORDER_MAX];
    float32_t wp[FILT_IIR_ORDER_MAX];
    float32_t a[FILT_IIR_LEN_MAX];
    float32_t b[FILT_IIR_LEN_MAX];
    float32_t m[FILT_IIR_LEN_MAX];
    float32_t n[FILT_IIR_LEN_MAX];
    float32_t d[FILT_IIR_LEN_MAX];
} FILT_IIR_t;

// Initialization & Reset APIs
void FILT_IIR_PoleZeroInit(FILT_IIR_t* filt, uint8_t order, float32_t f, float32_t G,
                           const float32_t wz[], const float32_t wp[]);
void FILT_IIR_ContinuousInit(FILT_IIR_t* filt, uint8_t order, float32_t f, const float32_t a[], const float32_t b[]);
void FILT_IIR_DiscreteInit(FILT_IIR_t* filt, uint8_t order, const float32_t m[], const float32_t n[]);
void FILT_IIR_Reset(FILT_IIR_t* filt, float32_t y);

// IIR-On (n-th order)
float32_t FILT_IIR_Run(FILT_IIR_t* filt, float32_t x);

// IIR-O3 (3rd order)
float32_t FILT_IIR_O3_Run(FILT_IIR_t* filt, float32_t x);

// IIR-O2 (2nd order)
float32_t FILT_IIR_O2_Run(FILT_IIR_t* filt, float32_t x);

// IIR-O1 (1st order)
float32_t FILT_IIR_O1_Run(FILT_IIR_t* filt, float32_t x);
