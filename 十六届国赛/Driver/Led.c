#include <STC15F2K60S2.H>
#include "Led.h"
#include "HC138.h"


void Led_Disp(unsigned char pos,unsigned char enable)
{
    unsigned char led = 0x00;
    unsigned char led_oled = 0xff;

    if (enable)
        led |= 0x01 << pos;
    else
        led_oled &= ~(0x01 << pos);

    if(led != led_oled)
    {
        P0 = ~led;
        HC138_Select(4);
        led_oled = led;
    }
}






