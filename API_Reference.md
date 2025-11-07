# millis Library - API Reference

This library provides Arduino-style millisecond timing functionality for AVR microcontrollers. It uses Timer0 in CTC (Clear Timer on Compare Match) mode with interrupts to maintain an accurate millisecond counter, enabling non-blocking delays and precise timing operations without the overhead of busy-wait loops.

The library is ideal for:
- Non-blocking timing operations
- Multi-tasking applications
- Periodic event scheduling
- Performance measurement
- Timeout implementations
- Real-time systems

---

## Hardware Requirements

### Microcontroller Support

**Compatible AVR Microcontrollers:**
- ATmega328P (Arduino Uno/Nano)
- ATmega2560 (Arduino Mega)
- ATmega32U4 (Arduino Leonardo/Pro Micro)
- ATmega168
- ATmega8
- Any AVR with Timer0 hardware

**Clock Frequency:**
- Default configuration: 16MHz
- Adjustable for other frequencies (requires OCR0A recalculation)

**Timer Usage:**
- Uses Timer0 exclusively
- Cannot be shared with PWM or other Timer0 functions
- Timer0 must be available and not used elsewhere

---

## Timer Configuration

### Default Configuration (16MHz)

The library configures Timer0 with the following parameters:

```
Clock Frequency: 16MHz
Prescaler: 64
Timer Frequency: 16MHz / 64 = 250kHz
Tick Period: 1 / 250kHz = 4µs
Ticks per 1ms: 1ms / 4µs = 250 ticks
OCR0A Value: 249 (counter counts 0-249 = 250 states)
```

### Timer Mode: CTC (Clear Timer on Compare Match)

**Mode Details:**
- WGM02:WGM00 = 010 (Mode 2)
- Timer resets automatically when reaching OCR0A
- Generates interrupt on compare match
- No PWM output generation

**Prescaler Settings:**
- CS02:CS00 = 011 (Prescaler 64)
- Optimal balance between resolution and range
- Allows 1ms precision with minimal CPU overhead

### Configuring for Different Clock Frequencies

If your microcontroller runs at a different frequency, calculate the new OCR0A value:

**Formula:**
```
Timer_Freq = F_CPU / Prescaler
Tick_Period = 1 / Timer_Freq
Ticks_for_1ms = 0.001 / Tick_Period
OCR0A = Ticks_for_1ms - 1
```

**Example for 8MHz:**
```
Timer_Freq = 8MHz / 64 = 125kHz
Tick_Period = 1 / 125kHz = 8µs
Ticks_for_1ms = 1ms / 8µs = 125 ticks
OCR0A = 125 - 1 = 124
```

**Example for 20MHz:**
```
Timer_Freq = 20MHz / 64 = 312.5kHz
Tick_Period = 1 / 312.5kHz = 3.2µs
Ticks_for_1ms = 1ms / 3.2µs = 312.5 ticks
OCR0A = 313 - 1 = 312
```

> [!NOTE]
> After calculating OCR0A, modify the value in `millis_Init()` function in millis.c:
> ```c
> OCR0A = 249;  // Change this value based on your calculation
> ```

---

## API Functions

### Initialization

#### `void millis_Init(void)`

**Description:**  
Initializes Timer0 for millisecond timing. This function **must be called once** during system initialization before using the millisecond counter.

**Operation:**
- Configures Timer0 in CTC mode (Mode 2)
- Sets prescaler to 64
- Enables Compare Match A interrupt
- Sets OCR0A to 249 for 1ms interval at 16MHz
- Clears any pending interrupt flags

**Parameters:**  
None

**Returns:**  
None

**Important:**  
Global interrupts must be enabled separately using `sei()` or `globalInt_Enable()` for the millisecond counter to work.

**Example:**
```c
#include "aKaReZa.h"
#include "millis.h"

extern volatile uint32_t System_millis;

int main(void) 
{
    millis_Init();        // Initialize millisecond timer
    globalInt_Enable();   // Enable global interrupts (required!)
    
    // Your code here
    
    while(1)
    {
        // Main loop
    }
}
```

> [!IMPORTANT]
> - The `millis_Init()` function **does not enable global interrupts**
> - You must call `sei()` or `globalInt_Enable()` after initialization
> - Without enabled interrupts, `System_millis` will not increment

---

### Global Variables

#### `volatile uint32_t System_millis`

**Description:**  
Global millisecond counter variable that increments every 1 millisecond via Timer0 interrupt.

**Type:**  
`volatile uint32_t` (32-bit unsigned integer)

**Range:**  
0 to 4,294,967,295 milliseconds (approximately 49.7 days)

**Rollover:**  
Counter rolls over to 0 after reaching maximum value (~49.7 days)

**Volatile Keyword:**  
Ensures the compiler doesn't optimize access to this variable, critical for variables modified in ISRs.

**Usage:**  
```c
// Read current millisecond count
uint32_t current_time = System_millis;

// Calculate elapsed time
uint32_t start_time = System_millis;
// ... do something ...
uint32_t elapsed = System_millis - start_time;

// Wait for specific duration (blocking)
uint32_t start = System_millis;
while ((System_millis - start) < 1000);  // Wait 1 second
```

> [!WARNING]
> - Always read `System_millis` atomically if reading more than 8 bits
> - For AVR, 32-bit reads are not atomic - disable interrupts if critical:
> ```c
> uint32_t current;
> cli();  // Disable interrupts
> current = System_millis;
> sei();  // Re-enable interrupts
> ```

---

### Non-Blocking Timing Structure

#### `typedef struct millis_T`

**Description:**  
Structure for implementing non-blocking delays and periodic tasks.

**Structure Members:**
```c
typedef struct
{
    uint32_t Previous;    // Previous timestamp (ms)
    uint32_t Delta;       // Elapsed time since previous (ms)
    uint32_t Interval;    // Desired interval duration (ms)
} millis_T;
```

**Member Details:**

- **Previous**: Stores the last event timestamp in milliseconds
- **Delta**: Calculated elapsed time since the previous timestamp
- **Interval**: Desired interval for periodic events

**Usage Pattern:**
```c
extern volatile uint32_t System_millis;

millis_T timer;
timer.Previous = 0;
timer.Delta = 0;
timer.Interval = 1000;  // 1 second

while(1)
{
    uint32_t current = System_millis;
    timer.Delta = current - timer.Previous;
    
    if (timer.Delta >= timer.Interval)
    {
        timer.Previous = current;
        // Execute periodic task
    }
}
```

**Example - LED Blinking:**
```c
#include "aKaReZa.h"
#include "millis.h"

extern volatile uint32_t System_millis;

int main(void) 
{
    // Configure LED pin
    bitSet(DDRB, PB5);  // Set PB5 as output
    
    // Initialize timing
    millis_Init();
    globalInt_Enable;
    
    // Create timer structure
    millis_T ledTimer;
    ledTimer.Previous = 0;
    ledTimer.Delta = 0;
    ledTimer.Interval = 500;  // 500ms interval
    
    while(1)
    {
        uint32_t current = System_millis;
        ledTimer.Delta = current - ledTimer.Previous;
        
        if (ledTimer.Delta >= ledTimer.Interval)
        {
            ledTimer.Previous = current;
            bitToggle(PORTB, PB5);  // Toggle LED
        }
        
        // Other non-blocking tasks can run here
    }
}
```

**Example - Multiple Timers:**
```c
#include "aKaReZa.h"
#include "millis.h"

extern volatile uint32_t System_millis;

int main(void) 
{
    // Initialize
    millis_Init();
    globalInt_Enable();
    
    // Create multiple timers
    millis_T fastTimer = {.Delta = 0, .Previous = 0, .Interval = 100};   // 100ms
    millis_T slowTimer = {.Delta = 0, .Previous = 0, .Interval = 1000};  // 1000ms
    
    while(1)
    {
        uint32_t current = System_millis;
        
        // Fast task (100ms)
        fastTimer.Delta = current - fastTimer.Previous;
        if (fastTimer.Delta >= fastTimer.Interval)
        {
            fastTimer.Previous = current;
            // Execute fast task
        }
        
        // Slow task (1000ms)
        slowTimer.Delta = current - slowTimer.Previous;
        if (slowTimer.Delta >= slowTimer.Interval)
        {
            slowTimer.Previous = current;
            // Execute slow task
        }
    }
}
```

---

## Complete Examples

### Example 1: Basic Millisecond Counter

```c
#include "aKaReZa.h"
#include "millis.h"
#include <stdio.h>

extern volatile uint32_t System_millis;

int main(void) 
{
    // Initialize millisecond timer
    millis_Init();
    globalInt_Enable();
    
    // Initialize UART for debugging (optional)
    // uart_init();
    
    while(1)
    {
        // Print current millisecond count every second
        static uint32_t lastPrint = 0;
        
        if ((System_millis - lastPrint) >= 1000)
        {
            lastPrint = System_millis;
            // printf("Millis: %lu\n", System_millis);
        }
    }
}
```

---

### Example 2: Non-Blocking LED Blink

```c
#include "aKaReZa.h"
#include "millis.h"

#define LED_PIN PB5

extern volatile uint32_t System_millis;

int main(void) 
{
    // Configure LED pin as output
    bitSet(DDRB, LED_PIN);
    
    // Initialize millisecond timer
    millis_Init();
    globalInt_Enable();
    
    millis_T ledTimer = {.Delta = 0, .Previous = 0, .Interval = 500};  // Blink every 500ms
    
    while(1)
    {
        uint32_t current = System_millis;
        ledTimer.Delta = current - ledTimer.Previous;
        
        if (ledTimer.Delta >= ledTimer.Interval)
        {
            ledTimer.Previous = current;
            bitToggle(PORTB, LED_PIN);
        }
        
        // CPU is free to do other tasks here
    }
}
```

---

### Example 3: Multi-Task System

```c
#include "aKaReZa.h"
#include "millis.h"

#define LED1_PIN PB0
#define LED2_PIN PB1
#define LED3_PIN PB2

extern volatile uint32_t System_millis;

int main(void) 
{
    // Configure LED pins
    bitSet(DDRB, LED1_PIN);
    bitSet(DDRB, LED2_PIN);
    bitSet(DDRB, LED3_PIN);
    
    // Initialize millisecond timer
    millis_Init();
    globalInt_Enable();
    
    // Create separate timers for each task
    millis_T timer1 = {.Delta = 0, .Previous = 0, .Interval =  200};   // LED1: 200ms
    millis_T timer2 = {.Delta = 0, .Previous = 0, .Interval =  500};   // LED2: 500ms
    millis_T timer3 = {.Delta = 0, .Previous = 0, .Interval =  1000};  // LED3: 1000ms
    
    while(1)
    {
        uint32_t current = System_millis;
        
        // Task 1: Fast blink (200ms)
        timer1.Delta = current - timer1.Previous;
        if (timer1.Delta >= timer1.Interval)
        {
            timer1.Previous = current;
            bitToggle(PORTB, LED1_PIN);
        }
        
        // Task 2: Medium blink (500ms)
        timer2.Delta = current - timer2.Previous;
        if (timer2.Delta >= timer2.Interval)
        {
            timer2.Previous = current;
            bitToggle(PORTB, LED2_PIN);
        }
        
        // Task 3: Slow blink (1000ms)
        timer3.Delta = current - timer3.Previous;
        if (timer3.Delta >= timer3.Interval)
        {
            timer3.Previous = current;
            bitToggle(PORTB, LED3_PIN);
        }
    }
}
```

---

### Example 4: Timeout Implementation

```c
#include "aKaReZa.h"
#include "millis.h"

#define BUTTON_PIN PD2
#define TIMEOUT_MS 5000  // 5 second timeout

extern volatile uint32_t System_millis;

int main(void) 
{
    // Configure button pin with pull-up
    bitClear(DDRD, BUTTON_PIN);   // Input
    bitSet(PORTD, BUTTON_PIN);    // Enable pull-up
    
    // Initialize millisecond timer
    millis_Init();
    globalInt_Enable();
    
    uint32_t buttonPressTime = 0;
    bool buttonPressed = false;
    
    while(1)
    {
        // Check button state
        if (bitRead(PIND, BUTTON_PIN) == 0)  // Button pressed (active low)
        {
            if (!buttonPressed)
            {
                buttonPressed = true;
                buttonPressTime = System_millis;
            }
            
            // Check for timeout
            if ((System_millis - buttonPressTime) >= TIMEOUT_MS)
            {
                // Button held for 5 seconds
                // Execute long-press action
                buttonPressed = false;
            }
        }
        else
        {
            if (buttonPressed)
            {
                // Button released
                uint32_t pressDuration = System_millis - buttonPressTime;
                
                if (pressDuration < TIMEOUT_MS)
                {
                    // Short press detected
                    // Execute short-press action
                }
                
                buttonPressed = false;
            }
        }
    }
}
```

---

### Example 5: Performance Measurement

```c
#include "aKaReZa.h"
#include "millis.h"

extern volatile uint32_t System_millis;

void someFunction(void)
{
    // Function to measure
    for (uint32_t i = 0; i < 100000; i++)
    {
        asm("nop");
    }
}

int main(void) 
{
    // Initialize millisecond timer
    millis_Init();
    globalInt_Enable();
    
    while(1)
    {
        // Measure execution time
        uint32_t startTime = System_millis;
        
        someFunction();
        
        uint32_t endTime = System_millis;
        uint32_t executionTime = endTime - startTime;
        
        // executionTime now contains the duration in milliseconds
        // Send to UART or use for profiling
        
        _delay_ms(1000);  // Wait before next measurement
    }
}
```

---

## Function Summary Table

| Function/Variable | Type | Purpose |
|-------------------|------|---------|
| `millis_Init()` | Function | Initialize Timer0 for millisecond timing |
| `System_millis` | Variable | Global millisecond counter (volatile uint32_t) |
| `millis_T` | Structure | Non-blocking timing structure |
| `TIMER0_COMPA_vect` | ISR | Interrupt service routine (automatic) |

---

## Troubleshooting Guide

### Counter Not Incrementing

**Symptom:** `System_millis` stays at 0

**Solutions:**
1. Check if `millis_Init()` was called
2. Verify global interrupts are enabled (`sei()` or `globalInt_Enable()`)
3. Confirm no other code disables interrupts with `cli()`
4. Check if Timer0 is being reinitialized elsewhere

**Debug Code:**
```c
// Check if interrupts are enabled
if (SREG & (1 << SREG_I))
{
    // Interrupts enabled
}

// Check if Timer0 interrupt is enabled
if (TIMSK0 & (1 << OCIE0A))
{
    // Timer0 interrupt enabled
}
```

---

### Inaccurate Timing

**Symptom:** Timing drifts or is consistently wrong

**Solutions:**
1. Verify F_CPU matches actual clock frequency
2. Recalculate OCR0A for your clock frequency
3. Check crystal oscillator accuracy
4. Verify power supply stability

**Calibration Test:**
```c
// Run this test to verify 1 second = 1000ms
millis_Init();
globalInt_Enable();

uint32_t start = System_millis;
_delay_ms(10000);  // 10 seconds using calibrated delay
uint32_t actual = System_millis - start;

// actual should be ~10000
// If significantly different, adjust OCR0A
```

---

## FAQ (Frequently Asked Questions)

**Q: How do I implement a delay using millis?**  
A: Use the subtraction method:
```c
uint32_t start = System_millis;
while ((System_millis - start) < 1000);  // Wait 1 second
```

**Q: What happens after 49.7 days?**  
A: The counter rolls over to 0. Use the subtraction method for timing, which handles rollover automatically.

**Q: Can I change the interrupt frequency?**  
A: Yes, by modifying the prescaler and OCR0A value. However, 1ms is optimal for most applications.

**Q: How much does this affect my program?**  
A: Minimal impact. About 0.1% CPU time at 16MHz, and ~100 bytes of flash memory.

**Q: Why not use Timer1 instead?**  
A: Timer1 is 16-bit and better suited for PWM. Timer0 is simpler and traditionally used for system timing.

**Q: Is this library safe for interrupt-driven applications?**  
A: Yes. The ISR is very short and only increments a counter. However, ensure atomic access if reading `System_millis` from other ISRs.

**Q: Can I port this to other AVR models?**  
A: Yes. All AVR microcontrollers with Timer0 can use this library. Just verify register names in the datasheet.

---

## Migration Guide

### From Blocking Delays to Non-Blocking

**Before (Blocking):**
```c
while(1)
{
    LED_ON;
    _delay_ms(500);
    LED_OFF;
    _delay_ms(500);
}
```

**After (Non-Blocking):**
```c
millis_Init();
globalInt_Enable();

millis_T timer = {0, 0, 500};

while(1)
{
    uint32_t current = System_millis;
    timer.Delta = current - timer.Previous;
    
    if (timer.Delta >= timer.Interval)
    {
        timer.Previous = current;
        bitToggle(PORTB, LED_PIN);
    }
    
    // Other tasks can run here
}
```

---

# 🌟 Support Me
If you found this repository useful:
- Subscribe to my [YouTube Channel](https://www.youtube.com/@aKaReZa75).
- Share this repository with others.
- Give this repository and my other repositories a star.
- Follow my [GitHub account](https://github.com/aKaReZa75).

# ✉️ Contact Me
Feel free to reach out to me through any of the following platforms:
- 📧 [Email: aKaReZa75@gmail.com](mailto:aKaReZa75@gmail.com)
- 🎥 [YouTube: @aKaReZa75](https://www.youtube.com/@aKaReZa75)
- 💼 [LinkedIn: @akareza75](https://www.linkedin.com/in/akareza75)
