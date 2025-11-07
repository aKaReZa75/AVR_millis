/**
 ******************************************************************************
 * @file     millis.c
 * @brief    Millisecond timing library implementation for AVR
 * 
 * @author   Hossein Bagheri
 * @github   https://github.com/aKaReZa75
 * 
 * @note     This library provides Arduino-style millis() functionality for
 *           AVR microcontrollers using Timer0 in CTC mode with interrupt.
 * 
 * @note     FUNCTION SUMMARY:
 *           - millis_Init          : Initialize Timer0 for 1ms interrupt generation
 *           - TIMER0_COMPA_vect ISR: Interrupt service routine that increments millisecond counter
 * 
 * @note     Requirements:
 *           - Global interrupts must be enabled via globalInt_Enable() or sei()
 *           - Timer0 must not be used for other purposes (PWM, etc.)
 *           - CPU frequency assumed to be 16MHz (adjust OCR0A for different frequencies)
 * 
 * @note     Usage Example:
 *           millis_Init();              // Initialize timer
 *           globalInt_Enable();         // Enable global interrupts
 *           uint32_t start = System_millis;
 *           while ((System_millis - start) < 1000); // Wait 1 second
 * 
 * @note     For detailed documentation with examples, visit:
 *           https://github.com/aKaReZa75/AVR_millis
 ******************************************************************************
 */

#include "millis.h"


/* ============================================================================
 *                         GLOBAL VARIABLES
 * ============================================================================ */
volatile uint32_t System_millis = 0;     /**< System millisecond counter - incremented every 1ms by ISR */
                                         /**< volatile keyword ensures compiler doesn't optimize access */


/* ============================================================================
 *                         INTERRUPT SERVICE ROUTINES
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Timer0 Compare Match A Interrupt Service Routine
 * @retval None
 * @note This ISR is called every 1 millisecond when Timer0 matches OCR0A
 *       Increments the global millisecond counter
 * @note IMPORTANT: Global interrupts must be enabled for this ISR to execute
 *       Call globalInt_Enable() or manually set I-bit in SREG
 * @note ISR execution time should be minimal to avoid timing drift
 * ------------------------------------------------------- */
ISR(TIMER0_COMPA_vect)
{
    System_millis++;                     /**< Increment millisecond counter - atomic operation on AVR */
};


/* ============================================================================
 *                         INITIALIZATION FUNCTION
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Initialize Timer0 for millisecond timing
 * @retval None
 * @note Configuration details:
 *       - Mode: CTC (Clear Timer on Compare Match) - Mode 2
 *       - Prescaler: 64 (CS02:CS00 = 011)
 *       - Compare value: 249 (for 1ms at 16MHz)
 *       - Interrupt: Compare Match A enabled
 * @note Timer calculation for 16MHz clock:
 *       Timer_freq = F_CPU / Prescaler = 16MHz / 64 = 250kHz
 *       Tick_period = 1 / 250kHz = 4us
 *       Ticks_for_1ms = 1ms / 4us = 250 ticks
 *       OCR0A = 250 - 1 = 249 (counter starts from 0)
 * @note Global interrupts must be enabled separately after this function
 * ------------------------------------------------------- */
void millis_Init(void)
{
    /* ===== Configure Timer0 for CTC Mode (Mode 2) ===== */
    /* CTC Mode: WGM02:WGM00 = 010 */
    bitClear(TCCR0A, WGM00);             /**< WGM00 = 0 for CTC mode */
    bitSet  (TCCR0A, WGM01);             /**< WGM01 = 1 for CTC mode */
    bitClear(TCCR0B, WGM02);             /**< WGM02 = 0 for CTC mode */

    /* ===== Set Clock Prescaler to 64 ===== */
    /* Prescaler 64: CS02:CS00 = 011 */
    /* Timer frequency = 16MHz / 64 = 250kHz */
    bitSet  (TCCR0B, CS00);              /**< CS00 = 1 for prescaler 64 */
    bitSet  (TCCR0B, CS01);              /**< CS01 = 1 for prescaler 64 */
    bitClear(TCCR0B, CS02);              /**< CS02 = 0 for prescaler 64 */

    /* ===== Enable Compare Match A Interrupt ===== */
    bitSet(TIMSK0, OCIE0A);              /**< Enable interrupt on compare match with OCR0A */

    /* ===== Clear Compare Match A Interrupt Flag ===== */
    intFlag_clear(TIFR0, OCF0A);         /**< Clear any pending interrupt flag before enabling */

    /* ===== Set Compare Match Value for 1ms Interval ===== */
    OCR0A = 249;                         /**< 250 ticks = 1ms at 250kHz timer frequency */
                                         /**< OCR0A = 249 because counter resets when reaching this value (0-249 = 250 states) */
};