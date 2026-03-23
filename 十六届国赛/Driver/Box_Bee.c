#include <STC15F2K60S2.H>
#include "Box_Bee.h"
#include "HC138.h"

void Box_On()
{
    P0 = 0x10;
    P0 &= ~(0x40);
    HC138_Select(5);
    
}

void Box_Off()
{
    P0 &= ~(0x10);
    P0 &= ~(0x40);
    HC138_Select(5);
}

void Bee_On()
{
    P0 |= 0x40;
    P0 &= ~(0x10);
    HC138_Select(5);
}

void Bee_Off()
{
    P0 &= ~(0x40);
    P0 &= ~(0x10);
    HC138_Select(5);
}


