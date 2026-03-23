#include <STC15F2K60S2.H>
#include "Wave.h"
#include "intrins.h"

sbit Tx = P1^0;
sbit Rx = P1^1;


void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 38;
	while (--i);
}

void Wave_Init()
{
    unsigned char i = 0;
    for (i = 0; i < 8; i++)
    {
        Tx = 1;
        Delay12us();
        Tx = 0;
        Delay12us();
    }
    
}

unsigned int rd_Length_Time()
{
    unsigned int temp = 0;
    CMOD = 0x00;
    CH = CL = 0;
    Wave_Init();
    CR = 1;
    while ((Rx == 1) && (CF == 0));
    CR = 0;
    
    if (CF == 0)
    {
        temp = CH << 8 | CL;
        return temp ;
    }
    else 
    {
        CF = 0;
        return 0;
    }
   
}

