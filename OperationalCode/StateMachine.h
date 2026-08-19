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

/**
 * \defgroup group_state_machine State Machine
 * \{
 * The State Machine (SM) module manages the LLC converter through a set of operational
 * states defined by \ref STATE_ID_t. Each state processes the driven object through
 * four sub-phases:
 * * **Entry** — executed once when the state becomes active
 * * **RunISR0** — executed every fast ISR cycle (continuous state processing)
 * * **RunISR1** — executed every slow ISR cycle (continuous state processing)
 * * **Exit** — executed once before transitioning to the next state
 *
 * \section group_state_machine_execution Execution Model
 * The module operates in a dual-rate ISR architecture:
 * * **Fast ISR** — runs common processing (sensors, fault protection, filters),
 *   then the active state's RunISR0, then additional application callbacks.
 * * **Slow ISR** — runs common processing (sensors, fault protection,
 *   function-execution handler), then the active state's RunISR1, then additional
 *   application callbacks, and finally evaluates state transition conditions.
 *
 * State transitions occur only at the end of the slow ISR. When a transition is
 * triggered, the current state's Exit callback is invoked, followed by the next
 * state's Entry callback.
 *
 * \section group_state_machine_transitions State Flow
 * \verbatim
 *   Init ──► Soft-Start ──► Control (Freq / Volt / Duty) ◄──► Sys_Id
 *    ▲                             │
 *    └──────── Fault ◄─────────────┘
 * \endverbatim
 * * **Init** — system startup, parameter loading, peripheral initialization and start
 * * **Soft-Start** — gradual converter ramp-up with configurable duration
 * * **Control** — steady-state regulation (frequency, voltage, or duty-cycle mode)
 * * **Sys_Id** — automated plant parameter identification
 * * **Fault** — error handling with configurable recovery strategies
 *
 * \note The Init state Entry sub-phase is being executed not by the SM scheduler interrupts
 *       like all other state sub-phases, but by the \ref STATE_MACHINE_Init function itself.
 *
 * Any operating state transitions to Fault on a fault trigger; Fault and
 * disable conditions return to Init.
 *
 *   \defgroup group_state_machine_enums Enumerated Types
 *   \defgroup group_state_machine_data_structures Data Structures
 *   \defgroup group_state_machine_functions Functions
 * \}
 */

#pragma once

#include "General.h"

/** \addtogroup group_state_machine_enums
 * \{
 */

/** State IDs for state machine transitions */
typedef enum
{
    Init_State = 0U,    /**< Initialization state: system startup, parameter loading,
                         *   peripheral initialization and start */
    Soft_Start_State,   /**< Soft-start state: gradual converter ramp-up */
    Freq_Ctrl_State,    /**< Frequency control state: frequency-based regulation */
    Volt_Ctrl_State,    /**< Voltage control state: voltage regulation */
    Duty_Ctrl_State,    /**< Duty cycle control state: duty-cycle-based regulation */
    Sys_Id_State,       /**< System identification state: automated parameter discovery */
    Fault_State,        /**< Fault state: error handling and recovery */
    Max_State           /**< Sentinel: total number of states */
} STATE_ID_t;

/** \} group_state_machine_enums */

/** \addtogroup group_state_machine_data_structures
 * \{
 */

/** State callbacks structure */
typedef struct
{
    void (* Entry)();    /**< Configurable callback executed on Entry phase (execute once) */
    void (* Exit)();     /**< Configurable callback executed on Exit phase (execute once) */
    void (* RunISR0)();  /**< Configurable callback executed on fast-Run phase
                          * (execute every fast ISR cycle, can pre-empt ISR1) */
    void (* RunISR1)();  /**< Configurable callback executed on slow-Run phase
                          * (execute every slow ISR cycle, can be pre-empted by ISR0) */
} STATE_t;

/** Additional application-specific callbacks structure, provides more flexibility */
typedef struct
{
    void (* RunISR0)();   /**< Configurable callback additionally executed on fast-Run phase
                           * (execute every fast ISR cycle, can pre-empt ISR1) */
    void (* RunISR1)();   /**< Configurable callback additionally executed on slow-Run phase
                           * (execute every slow ISR cycle, can be pre-empted by ISR0) */
} STATE_ADD_CALLBACK_t;

/** Initialization state service structure */
typedef struct
{
    TIMER_t timer;              /**< Initialization duration timer */
    bool param_init_done;       /**< Initialization completion flag */
} STATE_VARS_INIT_t;

/** Soft-start state service structure */
typedef struct
{
    TIMER_t timer;              /**< Soft-start ramp-up timer */
} STATE_VARS_SOFT_START_t;

/** Fault state service structure */
typedef struct
{
    uint32_t clr_try_cnt;           /**< Clear attempt counter */
    TIMER_t auto_clr_timer;         /**< Automatic clear hold-time timer */
    bool clr_faults_prev;           /**< Previous clear request state */
    bool clr_done;                  /**< Fault clearing completion flag */
    bool clr_request;               /**< Active fault clear request flag */
} STATE_VARS_FAULT_t;

/** State-specific runtime variables container */
typedef struct
{
    STATE_VARS_INIT_t init;                 /**< Initialization state service structure (\ref STATE_VARS_INIT_t) */
    STATE_VARS_SOFT_START_t soft_start;     /**< Soft-start state service structure (\ref STATE_VARS_SOFT_START_t) */
    STATE_VARS_FAULT_t fault;               /**< Fault state service structure (\ref STATE_VARS_FAULT_t) */
    /** \cond INTERNAL */
    #if defined(PC_TEST)
    float32_t* capture_channels[8];         // for capturing variable in state transitions
    float32_t capture_vals[8];              // for capturing variable in state transitions
    #endif
    /** \endcond */
} STATE_VARS_t;

/** Top-level state machine instance structure */
typedef struct
{
    STATE_t states[Max_State];              /**< All state definitions from \ref STATE_ID_t enum (\ref STATE_t) */
    STATE_ID_t previous;                    /**< Previous state (\ref STATE_ID_t) */
    STATE_ID_t current;                     /**< Currently active state (\ref STATE_ID_t) */
    STATE_ID_t next;                        /**< Next state to transition to (\ref STATE_ID_t) */
    STATE_VARS_t vars;                      /**< State-specific variables (\ref STATE_VARS_t) */
    STATE_ADD_CALLBACK_t extra_callback;    /**< Additional ISR callbacks (\ref STATE_ADD_CALLBACK_t) */
} STATE_MACHINE_t;

/** \} group_state_machine_data_structures */

/**
 * Global state machine instance.
 */
extern MULT_CORE_VLT STATE_MACHINE_t sm;

/** \addtogroup group_state_machine_functions
 * \{
 */

/**
 ***********************************************************************
 * \brief   Initialize the state machine module. Populates the state
 *          table with Entry/Exit/RunISR0/RunISR1 callbacks for each
 *          \ref STATE_ID_t, initializes the function-execution handler,
 *          and enters \ref Init_State. On first entry into
 *          \ref Init_State, hardware interface and parameters are
 *          initialized and all control sub-modules are reset.
 *
 * \note    The extra ISR callbacks (\ref STATE_ADD_CALLBACK_t) are set
 *          to empty functions and must be reassigned by the application
 *          if needed.
 *
 * \note    After the boot timer expires, the state machine transitions
 *          to \ref Soft_Start_State automatically if auto-start is
 *          enabled (default). If auto-start is disabled, the state
 *          machine remains in \ref Init_State until an external enable
 *          command is received.
 **********************************************************************/
void STATE_MACHINE_Init(void);

/**
 ***********************************************************************
 * \brief   Execute the fast ISR task. Runs common fast-rate processing
 *          (sensor interface, fault protection, and filters when not in
 *          \ref Init_State), then invokes the current state's RunISR0
 *          handler and the extra fast callback. This function is placed
 *          in RAM for deterministic execution time.
 *
 * \note    Must be called periodically at the fast ISR rate.
 **********************************************************************/
void STATE_MACHINE_RunISR0(void);

/**
 ***********************************************************************
 * \brief   Execute the slow ISR task. Runs common slow-rate processing
 *          (sensor interface, fault protection, function-execution
 *          handler), then invokes the current state's RunISR1 handler
 *          and the extra slow callback. After execution, evaluates
 *          state transition conditions and performs Entry/Exit
 *          callbacks on state change.
 *
 * \note    Must be called periodically at the slow ISR rate.
 *          State transitions only occur at the end of this function.
 **********************************************************************/
void STATE_MACHINE_RunISR1(void);

/**
 ***********************************************************************
 * \brief   Reset all control sub-modules to their initial state.
 *          Re-initializes and resets sensor interface, fault protection,
 *          output voltage control, system identification, and filters.
 **********************************************************************/
void STATE_MACHINE_ResetAllModules(void);

/** \} group_state_machine_functions */
