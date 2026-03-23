#include <STC15F2K60S2.H>
#include "HC138.h"


void HC138_Select(unsigned char channel)
{
    unsigned char temp = 0;
    switch (channel)
    {
    case 4:     //LED控制
        temp = P2 & 0x1f;
        temp |= 0x80;
        P2 = temp;
        temp &= 0x1f;
        P2 = temp;
        break;
    case 5:      //蜂鸣器、继电器控制
        temp = P2 & 0x1f;
        temp |= 0xa0;
        P2 = temp;
        temp &= 0x1f;
        P2 = temp;
        break;
    case 6:     //数码管段选控制
        temp = P2 & 0x1f;
        temp |= 0xc0;
        P2 = temp;
        temp &= 0x1f;
        P2 = temp;
        break;
    case 7:     //数码管位选控制
        temp = P2 & 0x1f;
        temp |= 0xe0;
        P2 = temp;
        temp &= 0x1f;
        P2 = temp;
        break;
    }
}

