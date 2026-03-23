/*	# 	DS1302代码片段说明
	1. 	本文件夹中提供的驱动代码供参赛选手完成程序设计参考。
	2. 	参赛选手可以自行编写相关代码或以该代码为基础，根据所选单片机类型、运行速度和试题
		中对单片机时钟频率的要求，进行代码调试和修改。
*/								

#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "intrins.h"

sbit RST = P1^3;
sbit SCK = P1^7;
sbit SDA = P2^3;

//
void Write_Ds1302(unsigned  char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK = 0;
		SDA = temp&0x01;
		temp>>=1; 
		SCK=1;
	}
}   

//
void Write_Ds1302_Byte( unsigned char address,unsigned char dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();  
 	Write_Ds1302(address);	
 	Write_Ds1302(dat);		
 	RST=0; 
}

//
unsigned char Read_Ds1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_Ds1302(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
	SCK=1;	_nop_();
	SDA=0;	_nop_();
	SDA=1;	_nop_();
	return (temp);			
}

void Set_Timer(unsigned char *Timer)
{
	unsigned char i;
	Write_Ds1302_Byte(0x8e,0x00);		//关写保护
	Write_Ds1302_Byte(0x80,0x80);		//停止计时
	for(i=0;i<3;i++)
	{
		Timer[i] = ((Timer[i] / 10 % 10) << 4) | (Timer[i] % 10);			//十进制转bcd码
		Write_Ds1302_Byte(0x84 - 2*i,Timer[i]);
	}
	Write_Ds1302_Byte(0x8e,0x80);		//打开写保护

}

void Read_Timer(unsigned char *Timer)
{
	unsigned char i; 
	unsigned char temp;
	EA = 0;
	for(i=0;i<3;i++)
	{
		temp = Read_Ds1302_Byte(0x85 - 2*i);
		Timer[i] = (temp / 16 * 10) + (temp % 16);
	}
	EA = 1;

}

