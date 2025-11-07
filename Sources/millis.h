/**
 ******************************************************************************
 * @file     millis.h
 * @brief    Millisecond timing library implementation for AVR
 * 
 * @author   Hossein Bagheri
 * @github   https://github.com/aKaReZa75
 * 
 * @note     This library provides Arduino-style millis() functionality for
 *           AVR microcontrollers using Timer0 in CTC mode with interrupt.
 *
 * @note     FUNCTION SUMMARY:
 *           - millis_Init : Initialize millisecond timer using SysTick interrupt
 * 
 * @note     Features:
 *           - Non-blocking interval timing using millis_T structure
 *           - Previous timestamp tracking for delta time calculation
 *           - Configurable interval timing for periodic tasks
 *           - Compatible with STM32 HAL timer infrastructure
 * 
 * @note     Usage:
 *           1. Call millis_Init() once during system initialization
 *           2. Create millis_T structure for each timing task
 *           3. Use System_millis to get current millisecond count
 *           4. Calculate elapsed time using delta between timestamps
 * 
 * @note     Example:
 *           extern volatile uint32_t System_millis;
 *           millis_T ledTimer = {.Delta = 0, .Previous = 0, .Interval = 1000};  // 1 second interval
 *           ledTimer.Delta = System_millis - ledTimer.Previous;
 *           if (ledTimer.Delta >= ledTimer.Interval) {
 *               ledTimer.Previous = currentMillis;
 *               // Execute periodic task
 *           }
 * 
 * @note     For detailed documentation with examples, visit:
 *           https://github.com/aKaReZa75/AVR_millis
 ******************************************************************************
 */
#ifndef _millis_H_
#define _millis_H_

#include "aKaReZa.h"


/* ============================================================================
 *                    CRITICAL DEPENDENCY CHECK
 * ============================================================================
 *  This library requires the aKaReZa.h base library to compile correctly.
 *  If the file is missing, please download it or contact for support.
 * ============================================================================ */
#ifndef _aKaReZa_H_
    #warning "============================================================"
    #warning " [WARNING] Missing required dependency: aKaReZa.h"
    #warning "------------------------------------------------------------"
    #warning "  This module depends on the aKaReZa.h base library."
    #warning "  Please download it from: https://github.com/aKaReZa75/AVR_RawProject"
    #warning "  Or contact for support: akaReza75@gmail.com"
    #warning "------------------------------------------------------------"
    #error   "Compilation aborted: Required file 'aKaReZa.h' not found!"
    #warning "============================================================"
#endif


/* ============================================================================
 *                         TYPE DEFINITIONS
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Millisecond timing structure for non-blocking delays
 * @note This structure tracks timing intervals for periodic tasks
 * ------------------------------------------------------- */
typedef struct
{
    uint32_t Previous;    /**< Previous timestamp in milliseconds - stores last event time */
    uint32_t Delta;       /**< Elapsed time since previous timestamp in milliseconds */
    uint32_t Interval;    /**< Desired interval duration in milliseconds for periodic events */
} millis_T;


/* ============================================================================
 *                         FUNCTION PROTOTYPES
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Initialize millisecond timing system
 * @retval None
 * @note Sets up SysTick timer for 1ms interrupt generation
 *       Must be called once before using timing functions
 *       Uses System_millis for millisecond counter access
 * ------------------------------------------------------- */
void millis_Init(void);

#endif /* _millis_H_ */
