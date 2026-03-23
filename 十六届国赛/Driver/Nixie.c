#include <STC15F2K60S2.H>
#include "Nixie.h"
#include "HC138.h"

unsigned char code Nixie_Num[] = {0xc0,0xf9,0xa4,0xB0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff,0xbf,0x8c,0x86};
                                                                                        // -   P    E
void Nixie_Disp(unsigned char pos,unsigned char dat,unsigned char point)
{
    P0 = 0x00;
    HC138_Select(7);    //ฯ๛าþ

    P0 = (0x01<<pos);
    HC138_Select(6);    //ฮปัก

    P0 = Nixie_Num[dat];
    if (point)
        P0 &= 0x7f;
    HC138_Select(7);
    
}

