#include <STC15F2K60S2.H>
#include "Initsys.h"
#include "HC138.h"

void InitSys()
{
    P0 = 0xff;
    HC138_Select(4);        //¹Ø±ÕLED
    P0 = 0x00;
    HC138_Select(5);        //¹Ø±Õ·äÃùÆ÷¡¢¼ÌµçÆ÷


}
