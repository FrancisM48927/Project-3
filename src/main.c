/**********************************************************************
* Author: F. MAILOM
* CPEG222 Project 3, 10/1/26
* NucleoF466ZE CMSIS Stopwatch w/SysTick, TIM5 SSD multiplexing, and
* interrupt driven buttons
**********************************************************************/
#include "stm32f4xx.h"
#include "ssd.h"

#define PAUSE 0
#define COUNT_UP 1
#define COUNT_DOWN 2

#define MAX_TIME 9999       // 99.99 seconds in hundredths of a second
#define MIN_TIME 0          // 0.00 seconds
#define TICKS_PER_SECOND 100 // SysTick interrupts every 10 ms (0.01 s)
#define DEBOUNCE_TICKS 3     // Ignore button presses for 30 ms after one is accepted

volatile uint8_t state = PAUSE;
volatile uint16_t time_hundredths = 0;
volatile uint8_t debounce_ticks = 0;

// SysTick interrupt, occurs every 0.01 seconds
void SysTick_Handler(void)
{
    // Count down the button debounce time
    if (debounce_ticks > 0)
    {
        debounce_ticks--;
    }

    if (state == COUNT_UP)
    {
        time_hundredths++;
        // Stop at 99.99 seconds
        if (time_hundredths >= MAX_TIME)
        {
            time_hundredths = MAX_TIME;
            state = PAUSE;
        }
        SSD_DisplayValue(time_hundredths);
    }
    else if (state == COUNT_DOWN)
    {
        time_hundredths--;
        // Stop at 0.00 seconds
        if (time_hundredths <= MIN_TIME)
        {
            time_hundredths = MIN_TIME;
            state = PAUSE;
        }
        SSD_DisplayValue(time_hundredths);
    }
}

// TIM5 interrupt, refreshes one SSD digit per interrupt
void TIM5_IRQHandler(void)
{
    if (TIM5->SR & (1 << 0))
    {
        // Clear the update flag
        TIM5->SR &= ~(1 << 0);
        SSD_Refresh();
    }
}

void EXTI9_5_IRQHandler(void)
{
    // Only accept a press if the previous one was more than 30 ms ago
    uint8_t accept = (debounce_ticks == 0);
    uint32_t pending = EXTI->PR & ((1 << 9) | (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5));

    // LEFT button: PF9 (OPTIONAL: set to 0.00 and pause)
    if (EXTI->PR & (1 << 9))
    {
        EXTI->PR = (1 << 9);
        if (accept)
        {
            state = PAUSE;
            time_hundredths = MIN_TIME;
            SSD_DisplayValue(time_hundredths);
        }
    }

    // CENTER button: PF8
    if (EXTI->PR & (1 << 8))
    {
        EXTI->PR = (1 << 8);
        if (accept)
        {
            state = PAUSE;
        }
    }

    // UP button: PF7
    if (EXTI->PR & (1 << 7))
    {
        EXTI->PR = (1 << 7);
        if (accept)
        {
            state = COUNT_UP;
        }
    }

    // RIGHT button: PE6 (OPTIONAL: set to 99.99 and pause)
    if (EXTI->PR & (1 << 6))
    {
        EXTI->PR = (1 << 6);
        if (accept)
        {
            state = PAUSE;
            time_hundredths = MAX_TIME;
            SSD_DisplayValue(time_hundredths);
        }
    }

    // DOWN button: PE5
    if (EXTI->PR & (1 << 5))
    {
        EXTI->PR = (1 << 5);
        if (accept)
        {
            state = COUNT_DOWN;
        }
    }

    // Start the debounce time if a press was accepted
    if (accept && pending)
    {
        debounce_ticks = DEBOUNCE_TICKS;
    }
}

int main(void)
{
    // Enable clock for GPIOE and GPIOF (SSD helper enables its own clocks)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN |
                    RCC_AHB1ENR_GPIOFEN;

    // Initialize the SSD array and TIM5 multiplexing, show 0.00 (first digit blank)
    SSD_Init();
    SSD_DisplayValue(time_hundredths);

    // Configure GPIO pins PE5 (DOWN), PE6 (RIGHT), PF7 (UP), PF8 (CENTER), PF9 (LEFT) as inputs
    GPIOE->MODER &= ~((3 << (5 * 2)) | (3 << (6 * 2)));
    GPIOF->MODER &= ~((3 << (7 * 2)) | (3 << (8 * 2)) | (3 << (9 * 2)));

    // Configure GPIO for all buttons to pull-up
    // Pressing the button creates a falling edge
    GPIOE->PUPDR &= ~((3 << (5 * 2)) | (3 << (6 * 2)));
    GPIOE->PUPDR |=  ((1 << (5 * 2)) | (1 << (6 * 2)));
    GPIOF->PUPDR &= ~((3 << (7 * 2)) | (3 << (8 * 2)) | (3 << (9 * 2)));
    GPIOF->PUPDR |=  ((1 << (7 * 2)) | (1 << (8 * 2)) | (1 << (9 * 2)));

    // Enable EXTI clock and connect PE5, PE6, PF7, PF8, and PF9 to EXTI5-9
    // Port codes: E = 0x4, F = 0x5
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[1] &= ~(0xFFF << 4);                           // EXTI5, EXTI6, EXTI7
    SYSCFG->EXTICR[1] |= (0x4 << 4) | (0x4 << 8) | (0x5 << 12);   // PE5, PE6, PF7
    SYSCFG->EXTICR[2] &= ~(0x00FF);                               // EXTI8, EXTI9
    SYSCFG->EXTICR[2] |= (0x0055);                                // PF8, PF9

    // Configure EXTI to detect falling edges and enable EXTI lines
    EXTI->FTSR |= (1 << 5) |
                  (1 << 6) |
                  (1 << 7) |
                  (1 << 8) |
                  (1 << 9);
    EXTI->IMR |= (1 << 5) |
                 (1 << 6) |
                 (1 << 7) |
                 (1 << 8) |
                 (1 << 9);

    // Enable EXTI9_5 interrupt in NVIC (lower priority than TIM5 and SysTick)
    NVIC_SetPriority(EXTI9_5_IRQn, 3);
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    // Configure SysTick for an interrupt every 0.01 seconds
    SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);
    NVIC_SetPriority(SysTick_IRQn, 2);

    while(1)
    {
        // Everything is interrupt driven, sleep until the next interrupt
        __WFI();
    }
    return(0);
}