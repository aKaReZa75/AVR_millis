/**
 ******************************************************************************
 * @file     main.c
 * @brief    Non-blocking multitasking example using millis() timing for AVR
 * 
 * @author   Hossein Bagheri
 * @github   https://github.com/aKaReZa75
 * 
 * @note     This example demonstrates:
 *           - Millisecond time base (System_millis)
 *           - Four independent periodic tasks without blocking delays
 *           - LED toggling on PORTC pins 0 and 1
 *           - One-shot timed event on PORTC pin 2
 *           - Live millis counter shown on a 16x2 alphanumeric LCD
 ******************************************************************************
 */

#include "aKaReZa.h"

/* ============================================================================
 *                            GLOBAL VARIABLES
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Millisecond counter, incremented by the millis timer interrupt
 * @note Defined in the millis module, started by millis_Init()
 * ------------------------------------------------------- */
extern volatile uint32_t System_millis;

/* ============================================================================
 *                               TASKS
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Task 1: toggle PORTC pin 0 every 1000 ms
 * @retval None
 * @note Non-blocking: compares elapsed time against the interval on each call
 *       Must be called continuously from the main loop
 * ------------------------------------------------------- */
void Task1(void)
{
    /* Timing state is kept between calls (Delta, Interval in ms, Previous timestamp) */
    static millis_T Task1_Millis = {.Delta=0, .Interval=1000, .Previous = 0};

    /* Time elapsed since the last execution */
    Task1_Millis.Delta = System_millis - Task1_Millis.Previous;

    /* Interval elapsed: save the new timestamp and run the task */
    if(Task1_Millis.Delta > Task1_Millis.Interval)
    {
        Task1_Millis.Previous = System_millis;
        bitToggle(PORTC, 0);
    };
};

/* -------------------------------------------------------
 * @brief Task 2: toggle PORTC pin 1 every 350 ms
 * @retval None
 * @note Runs independently of Task 1, so the two LEDs blink at different rates
 * ------------------------------------------------------- */
void Task2(void)
{
    static millis_T Task2_Millis = {.Delta=0, .Interval=350, .Previous = 0};

    Task2_Millis.Delta = System_millis - Task2_Millis.Previous;

    if(Task2_Millis.Delta > Task2_Millis.Interval)
    {
        Task2_Millis.Previous = System_millis;
        bitToggle(PORTC, 1);
    };
};

/* -------------------------------------------------------
 * @brief Task 3: clear PORTC pin 2 after 5000 ms
 * @retval None
 * @note Pin 2 is set in main() at startup, so it goes low about 5 s later
 *       The pin is only cleared (never toggled), so it stays low afterwards
 * ------------------------------------------------------- */
void Task3(void)
{
    static millis_T Task3_Millis = {.Delta=0, .Interval=5000, .Previous = 0};

    Task3_Millis.Delta = System_millis - Task3_Millis.Previous;

    if(Task3_Millis.Delta > Task3_Millis.Interval)
    {
        Task3_Millis.Previous = System_millis;
        bitClear(PORTC, 2);
    };
};

/* -------------------------------------------------------
 * @brief Task 4: show the millis counter on the LCD every 100 ms
 * @retval None
 * @note Prints at the top-left corner (column 0, row 0) of the 16x2 LCD
 * ------------------------------------------------------- */
void Task4(void)
{
    static millis_T Task4_Millis = {.Delta = 0, .Interval = 100, .Previous = 0};

    /* Text buffer: 16 characters + null terminator for the 16x2 LCD */
    char alcd_Buffer[17];

    Task4_Millis.Delta = System_millis - Task4_Millis.Previous;

    if(Task4_Millis.Delta >= Task4_Millis.Interval)
    {
        Task4_Millis.Previous = System_millis;

        /* Format the counter as text, then print it at the first position */
        sprintf(alcd_Buffer, "miilis=%ld", System_millis);
        alcd_gotoxy(0, 0);
        alcd_puts(alcd_Buffer);
    };
};

/* ============================================================================
 *                             MAIN PROGRAM
 * ============================================================================ */

/* -------------------------------------------------------
 * @brief Application entry point
 * @retval None
 * @note Initializes the time base, LCD, and GPIO, then runs all tasks forever
 * ------------------------------------------------------- */
int main(void)
{
    /* Start the millisecond timer and enable global interrupts */
    millis_Init();
    globalInt_Enable;

    /* Initialize the LCD and show a start-up message */
    alcd_init();
    alcd_puts("aKaReZa");

    /* Configure PORTC pins 0, 1, 2 as outputs */
    GPIO_Config_OUTPUT(DDRC, 0);
    GPIO_Config_OUTPUT(DDRC, 1);
    GPIO_Config_OUTPUT(DDRC, 2);

    /* Pin 2 starts high; Task 3 will clear it after 5 s */
    bitSet(PORTC, 2);

    /* Main loop: call every task each pass (cooperative scheduling) */
    while(1)
    {
        Task1();
        Task2();
        Task3();
        Task4();
    };
};