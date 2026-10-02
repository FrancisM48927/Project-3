/********************************************************************** 
* Author: F. MAILOM
* CPEG222 SSD Helper, 9/21/26
* NucleoF466ZE CMSIS Initialize/Display SSD
**********************************************************************/
#include "stm32f4xx.h"
#include "ssd.h"

// GPIO definitions

#define SSD_SEGMENT_PORT_G GPIOG
#define SSD_SEGMENT_PORT_F GPIOF
#define SSD_SEGMENT_PORT_E GPIOE
#define SSD_SEGMENT_PORT_B GPIOB

#define SSD_DIGIT_PORT_E GPIOE
#define SSD_DIGIT_PORT_B GPIOB

// Pin definitions

#define SEG_A_PIN 9
#define SEG_B_PIN 12
#define SEG_C_PIN 13
#define SEG_D_PIN 14
#define SEG_E_PIN 8
#define SEG_F_PIN 15
#define SEG_G_PIN 4
#define SEG_DP_PIN 14

#define DIGIT1_PIN 10
#define DIGIT2_PIN 7
#define DIGIT3_PIN 5
#define DIGIT4_PIN 3

// SSD definitions

#define SSD_BLANK 10
const uint8_t digitSegments[] =
{
    0b0111111,    // 0
    0b0000110,    // 1
    0b1011011,    // 2
    0b1001111,    // 3
    0b1100110,    // 4
    0b1101101,    // 5
    0b1111101,    // 6
    0b0000111,    // 7
    0b1111111,    // 8
    0b1101111,    // 9
    0b0000000     // blank
};
volatile uint8_t displayDigits[4] =
{
    SSD_BLANK,
    0,
    0,
    0
};

// Function definitions

void SSD_Init(void)
{
    // Enable GPIOs
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN
                 | RCC_AHB1ENR_GPIOFEN
                 | RCC_AHB1ENR_GPIOEEN
                 | RCC_AHB1ENR_GPIOBEN;

    // GPIOG pins to output mode
    GPIOG->MODER &= ~((3U << (SEG_A_PIN * 2)) |
                      (3U << (SEG_D_PIN * 2)));
    GPIOG->MODER |=  ((1U << (SEG_A_PIN * 2)) |
                      (1U << (SEG_D_PIN * 2)));
    // GPIOF pins to output mode
    GPIOF->MODER &= ~((3U << (SEG_B_PIN * 2)) |
                      (3U << (SEG_C_PIN * 2)) |
                      (3U << (SEG_F_PIN * 2)) |
                      (3U << (SEG_DP_PIN * 2)));
    GPIOF->MODER |=  ((1U << (SEG_B_PIN * 2)) |
                      (1U << (SEG_C_PIN * 2)) |
                      (1U << (SEG_F_PIN * 2)) |
                      (1U << (SEG_DP_PIN * 2)));
    // GPIOE pins to output mode
    GPIOE->MODER &= ~((3U << (SEG_E_PIN * 2)) |
                      (3U << (DIGIT1_PIN * 2)) |
                      (3U << (DIGIT2_PIN * 2)));
    GPIOE->MODER |=  ((1U << (SEG_E_PIN * 2)) |
                      (1U << (DIGIT1_PIN * 2)) |
                      (1U << (DIGIT2_PIN * 2)));
    // GPIOB pins to output mode
    GPIOB->MODER &= ~((3U << (SEG_G_PIN * 2)) |
                      (3U << (DIGIT3_PIN * 2)) |
                      (3U << (DIGIT4_PIN * 2)));
    GPIOB->MODER |=  ((1U << (SEG_G_PIN * 2)) |
                      (1U << (DIGIT3_PIN * 2)) |
                      (1U << (DIGIT4_PIN * 2)));
}

void SSD_DisplayValue(uint16_t value)
{
    if (value < 1000)
    {
        // Less than 10.00 - suppress leading zero
        displayDigits[0] = SSD_BLANK;
        displayDigits[1] = value / 100;
        displayDigits[2] = (value / 10) % 10;
        displayDigits[3] = value % 10;
    }
    else
    {
        // 10.00 through 99.99
        displayDigits[0] = value / 1000;
        displayDigits[1] = (value / 100) % 10;
        displayDigits[2] = (value / 10) % 10;
        displayDigits[3] = value % 10;
    }
}