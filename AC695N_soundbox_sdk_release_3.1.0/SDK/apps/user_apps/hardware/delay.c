#include "delay.h"

void delay_8us(void)
{
    // for (u8 i = 200; i > 0; i--)
    for (u8 i = 300; i > 0; i--)
    {
        __asm__ volatile("nop");
    }
}

void delay_88us(void)
{
    // for (u32 i = 2130; i > 0; i--)
    for (u32 i = 2330; i > 0; i--)
    {
        __asm__ volatile("nop");
    }
}

void delay_200us(void)
{
    // for (u32 i = 4700; i > 0; i--)
    for (u32 i = 4900; i > 0; i--)
    {
        __asm__ volatile("nop");
    }
}
