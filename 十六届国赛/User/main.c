#include <STC15F2K60S2.H>
#include "Initsys.h"
#include "Key.h"
#include "Nixie.h"
#include "Led.h"
#include "Box_Bee.h"
#include "HC138.h"
#include "Wave.h"
#include "iic.h"
#include "onewire.h"
#include "ds1302.h"
#include "Uart.h"
#include <stdio.h>
#include "string.h"


/* =====================================================================================================*/
//变量声明区
unsigned char pdata i = 0;        //循环变量
unsigned char pdata Key_Value,Key_Down,Key_Up,Key_Old;        //按键变量
unsigned char pdata Key_Slow_Down = 0;        //按键延时
unsigned char xdata Nixie_Show[8] = {10,10,10,10,10,10,10,10};       //数码管显示变量
unsigned char xdata Nixie_Point[8] = {0,0,0,0,0,0,0,0};        //数码管小数点变量
unsigned char Nixie_Led_Pos = 0;        //显示轮询
unsigned int Nixie_Slow_Down = 0;        //数码管显示延时
unsigned char xdata Led_Enable[8] = {0,0,0,0,0,0,0,0};        //led使能变量
unsigned int pdata NE555_1000MS = 0;          //NE555 1S频率计时

unsigned char pdata Nixie_Show_Mode = 0;        //数码管显示模式—— 0：数据 1：参数 2：标定
unsigned char pdata Nixie_Show_viceMode = 0;        //数码管次屏显示—— 0~7
float pdata Tem_Read = 0;        //温度读取变量
//unsigned int xdata Water_Lenth = 0;        //水位长度       cm
unsigned int xdata Volumn = 0;        //体积       m?
unsigned char Remain_Volumn = 0;        //剩余体积比       
unsigned int Weight = 0;        //重量       t
unsigned int xdata Volt_Weight_Show[5] = {0,10,20,40,50};        //电压模拟重量       V*10
unsigned int xdata Volt_Weight_Set[5] = {0,10,20,40,50};        //电压模拟重量设置       V*10


unsigned int  freq = 0;          //NE555 频率
unsigned int  freq_old = 0;          //NE555 频率旧值
unsigned char pdata Uart_Recv[20] = {0};        //串口接收变量
unsigned char pdata Uart_Rx_Index = 0;        //串口接收索引
// unsigned int Uart_Slow_Down = 0;        //串口接收延时
unsigned char Uart_Rx_Time = 0;					//串口读取时间
bit Uart_Rx = 0;												//单次读标记

unsigned int pdata H_Set[2] ={100,10};         //液位限度—— 0：高水位，1：低水位
unsigned int pdata F_Set = 2000;         //频率参数
unsigned int xdata S_Type = 2;        //容器类型—— 0：球体，1：圆柱体，2：长方体
unsigned int xdata R_Set = 100;         //半径参数  
unsigned int pdata L_W_H_Set[3] ={100,100,100};         //长、宽、高参数

unsigned int pdata Wave_Lenth = 0;        //超声波测出长度       cm
xdata unsigned int Water_d = 0;        //水位实际高度       cm

unsigned char pdata Uart_Id[10] = {0} ;        //串口ID
unsigned char pdata id_value[10] = {0};               //数值
unsigned char xdata error = 0;								//错误标记
unsigned char xdata ok = 0;										//正常标记
unsigned char pdata temp_Set[2] = {0};
unsigned char pdata Error_Index = 0;                //错误变量          0_正常 1_水位超出范围 2_水位低于范围  3_频率超出规定    4_频率变化连续   5_频率变化过大
bit Error_Flag = 0;                //错误标志  
bit Freq_Flag = 0;                //频率标志    0_连续下降  1_连续上升
bit Freq_Flag_Old = 0;                //频率标志旧值
bit Freq_Big_Change = 0;                //频率变化剧烈标志

 bit Error_1 = 0;		//500
 bit Error_2 = 0;		//501
 bit Error_3 = 0;		//502
 bit Error_4 = 0;		//503
 bit Error_5 = 0;		//504

unsigned int weight_result = 0;

unsigned char pdata Timer_Show[3] = {23,59,50};        //时间设置
unsigned char Freq_5T = 0;        //5次连续上升变化
unsigned char Freq_5D = 0;        //5次连续下降变化
unsigned char Time_100Ms = 0;			//100ms延时
unsigned char Time_200Ms = 0;			//200ms延时 
unsigned int Time_3S = 0;					//3S延时
bit Fight_Led_7 = 0;							//led7的闪烁标志
bit Led_8_Start_Index = 0;				//led8开始标志
bit Fight_Led_8 = 0;							//led8的闪烁标志

unsigned char Trend_Error_Flag = 0;     // 趋势异常标志
unsigned char Trend_Error_Count = 0;     // 趋势异常计数器
unsigned char Trend_Error_Reported = 0;  // 趋势异常已上报标志

unsigned int freq_diff =0;					//频率不同			
/* =====================================================================================================*/
//函数声明区

void Error_Report()
{
    // 趋势异常检测
    if (((freq - freq_old) > 60000) || (freq - freq_old < -60000)) {
        // 计算实际频率差（考虑溢出）
        unsigned int freq_diff = ((freq - freq_old) > 60000) ? freq_old - freq : freq - freq_old;
        
        // 趋势异常持续检测
        if (freq_diff > 1000) {  // 假设阈值为1000
            Trend_Error_Count++;
            if (Trend_Error_Count >= 3 && !Trend_Error_Reported) {  // 连续3次检测到异常
                Trend_Error_Flag = 1;
                Error_Index = 5;  // 趋势异常
                Trend_Error_Reported = 1;  // 标记为已上报
            }
        } else {
            // 趋势恢复正常
            Trend_Error_Count = 0;
            Trend_Error_Reported = 0;
        }
    } else {
        // 趋势恢复正常
        Trend_Error_Count = 0;
        Trend_Error_Reported = 0;
    }

    // 现有异常上报逻辑
    if(Error_Flag == 0) 
    { 
        switch (Error_Index) 
        { 
        case 1:     // 500 
            printf("[500,%bu%bu%bu,%u.%u%u]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],Water_d / 100 % 10,Water_d / 10 % 10,Water_d % 10 ); 
            break; 
        
        case 2:     // 501 
            printf("[501,%bu%bu%bu,%u.%u%u]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],Water_d / 100 % 10,Water_d / 10 % 10,Water_d % 10 ); 
            break; 
        case 3:     // 502 
            printf("[503,%bu%bu%bu,%u]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],Water_d / 100 % 10,freq ); 
            break; 
        case 4:     // 504   
            printf("[504,%bu%bu%bu,%bu]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],Water_d / 100 % 10,Freq_Flag ); 
            break; 
        case 5:     // 505 - 趋势异常
            printf("[505,%bu%bu%bu,%bu]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],Water_d / 100 % 10,((freq - freq_old) > 60000) ? freq_old- freq : freq - freq_old ); 
            break; 
        } 
        Error_Flag = 1; 
    } 
    else if( Error_Index == 0) 
    { 
        Error_Flag = 0;         //不再发送错误信息 
        // 重置趋势异常标志
        Trend_Error_Flag = 0;
        Trend_Error_Reported = 0;
    }
}

//获取串口ID和数值
unsigned char Uart_Get_IdAndValue(unsigned char pos,unsigned char *Uart_Id,unsigned char *id_value);
void Volumn_Calc(unsigned int Water_Lenth,unsigned int R_Set,unsigned int *L_W_H_Set,unsigned int S_Type,unsigned int *Volumn,unsigned char *Remain_Volumn,unsigned int *Water_d);

void Nixie_Clear()
{
    unsigned char Nixie_i = 0;
    for ( Nixie_i = 0; Nixie_i < 8; Nixie_i++)
    {
        Nixie_Show[Nixie_i] = 10;
        Nixie_Point[Nixie_i] = 0;
    }
}


//将字符串转换为整数（单位：厘米）
unsigned int FloatToInt_cm(unsigned char *id_value,unsigned char *error) 
{
    unsigned int temp1 = 0;
    for(i = 0;i<strlen(id_value);i++)
    {
        if(id_value[i] == '.') continue;
        if(id_value[i] < '0' || id_value[i] > '9')
        {
            *error = 1; 
            return 0xff;
        }
        temp1 = temp1 * 10 + (id_value[i] - '0');
    }
    return temp1 * 10;
}   



//解析串口数据
void Uart_Solve_temp(unsigned char *Uart_Id,unsigned char *id_value, unsigned char *error, unsigned char *ok)
{
	unsigned int temp2 = 0;
    if(id_value[0] == '?')
    {
        if(strstr(Uart_Id,"H1"))
        {
            printf("(H1:%u.%u)\r\n",H_Set[0] / 100,H_Set[0] / 10 % 10);
            *ok = 1;
        }
        else if(strstr(Uart_Id,"H2"))
        {
            printf("(H2:%u.%u)\r\n",H_Set[1] / 100,H_Set[1] / 10 % 10);
            *ok = 1;
        }
        else if (strstr(Uart_Id,"F"))
        {
            printf("(F:%u)\r\n",F_Set);
            *ok = 1;
        }
        else if(strstr(Uart_Id,"S"))
        {
            printf("(S:%u)\r\n",S_Type);
            *ok = 1;
        }
        else if(strstr(Uart_Id,"r"))
        {
            printf("(r:%u.%u)\r\n",R_Set / 100,R_Set / 10 % 10);
            *ok = 1;
        }
        else if(strstr(Uart_Id,"L"))
        {
            printf("(L:%u.%u)\r\n",L_W_H_Set[0] / 100,L_W_H_Set[0] / 10 % 10);
            *ok = 1;
        }
        else if(strstr(Uart_Id,"W"))
        {
            printf("(W:%u.%u)\r\n",L_W_H_Set[1] / 100,L_W_H_Set[1] / 10 % 10);
            *ok = 1;
        }
        else if(strstr(Uart_Id,"H"))
        {
            printf("(H:%u.%u)\r\n",L_W_H_Set[2] / 100,L_W_H_Set[2] / 10 % 10);
            *ok = 1;
        }
        else
        {   //未知符号
            *error = 1;
        }

        return;     //解析完成
    }

    //时间设置  （T:HHMMSS）
    if(strstr(Uart_Id,"T"))
    {
        unsigned char h,m,s;        //判断变量
        
        //必须是6位数
        if(
            id_value[0] < '0' || id_value[0] > '9' || 
            id_value[1] < '0' || id_value[1] > '9' || 
            id_value[2] < '0' || id_value[2] > '9' || 
            id_value[3] < '0' || id_value[3] > '9' || 
            id_value[4] < '0' || id_value[4] > '9' || 
            id_value[5] < '0' || id_value[5] > '9' || 
            id_value[6] != '\0' 
        )
        {
            *error = 1;
            return; 
        }
        
        h = (id_value[0] - '0') * 10 + (id_value[1] - '0');
        m = (id_value[2] - '0') * 10 + (id_value[3] - '0');
        s = (id_value[4] - '0') * 10 + (id_value[5] - '0');

        //判断是否超出范围
        if(h >= 24 || m >= 60 || s >= 60)
        {
            *error = 1;
            return;
        }
        //设置时间  
        Timer_Show[0] = h ;
        Timer_Show[1] = m ;
        Timer_Show[2] = s ;
        Set_Timer(Timer_Show);
        *ok = 1;
        return;

    }
    
    //参数设置 
    if(strstr(Uart_Id,"H1"))    
    {
			
        temp2 = FloatToInt_cm(id_value,error);    
        if(temp2 < H_Set[1] )
        {
           *error = 1;      //上限不可以低于下限
           return   ;
        }
        else if (temp2 == 0xff)
        {
            *error = 1;      //参数错误
           return   ;
        }
        
        H_Set[0] = temp2;
        *ok = 1;
        return  ;
    }
    else if(strstr(Uart_Id,"H2"))    
    {
        temp2 = FloatToInt_cm(id_value,error);    
        if(temp2 > H_Set[0])
        {
           *error = 1;      //下限不可以高于上限
           return   ;
        }
        else if (temp2 == 0xff)
        {
            *error = 1;      //参数错误
           return   ;
        }

        H_Set[1] = temp2;
        *ok = 1;
        return  ;
    }
    else if(strstr(Uart_Id,"F"))
    {
        temp2 = FloatToInt_cm(id_value,error);  
        if(temp2 == 0xff)   
        {
            *error = 1;      //参数错误
            return   ;
        }
        F_Set = temp2 / 10;
        *ok = 1;
    }
    else if(strstr(Uart_Id,"S"))
    {
        temp2 = FloatToInt_cm(id_value,error);  
        temp2 /= 10;            //除以10，得到正确数值
        if(temp2 == 0xff || temp2 > 2)   
        {
            *error = 1;      //参数错误
            return   ;
        }
        S_Type = temp2;
        *ok = 1;
    }
    else if(strstr(Uart_Id,"r"))
    {
        temp2 = FloatToInt_cm(id_value,error);  
        if(temp2 == 0xff )   
        {
            *error = 1;      //参数错误
            return   ;
        }
        R_Set = temp2;     
        *ok = 1;
    }
    else if(strstr(Uart_Id,"L"))
    {
        temp2 = FloatToInt_cm(id_value,error);  
        if(temp2 == 0xff )   
        {
            *error = 1;      //参数错误
            return   ;
        }
        L_W_H_Set[0] = temp2;
        *ok = 1;
    }
    else if(strstr(Uart_Id,"W"))
    {
        temp2 = FloatToInt_cm(id_value,error);  
        if(temp2 == 0xff )   
        {
            *error = 1;      //参数错误
            return   ;
        }
        L_W_H_Set[1] = temp2;
        *ok = 1;
    }
    else if(strstr(Uart_Id,"H"))
    {
        temp2 = FloatToInt_cm(id_value,error);  
        if(temp2 == 0xff )   
        {
            *error = 1;      //参数错误
            return   ;
        }
        L_W_H_Set[2] = temp2;
        *ok = 1;
    }

}

//超声波解析容器



void Key_PutO()         //按键处理
{
    if(Key_Slow_Down <= 10) return;
    Key_Slow_Down = 0;

    Key_Value = Key_Read();
    Key_Down = Key_Value & (Key_Value ^ Key_Old);
    Key_Up = ~Key_Value & (Key_Value ^ Key_Old);
    Key_Old = Key_Value;

    switch (Key_Down)
    {
    case 4:     //显示模式
        if(++Nixie_Show_Mode >= 3)    
        {
            Nixie_Show_Mode = 0;
        }   
        Nixie_Show_viceMode = 0;
        if(Nixie_Show_Mode == 0)
        {
            for(i = 0;i<5;i++)
            {
                if(i < 4 && i > 0)
                {
                    if(Volt_Weight_Show[i] < Volt_Weight_Show[i+1]
                        && Volt_Weight_Show[i] > Volt_Weight_Show[i-1])
                        {
                            Volt_Weight_Set[i] = Volt_Weight_Show[i];
                        }
                }
                else if(i == 4)
                {
                    if(Volt_Weight_Show[i] <= 50 && Volt_Weight_Show[i] > Volt_Weight_Show[i-1])
                    {
                        Volt_Weight_Set[i] = Volt_Weight_Show[i];
                    }
                }
                else if(i == 0)
                {
                    if(Volt_Weight_Show[i] < Volt_Weight_Show[i+1] && Volt_Weight_Show[i] > 0)
                    {
                        Volt_Weight_Set[i] = Volt_Weight_Show[i];
                    }
                }
            }
        }
        if(Nixie_Show_Mode == 2)
        {
            memcpy(Volt_Weight_Show,Volt_Weight_Set,sizeof(Volt_Weight_Set));
        }
        break;
    
    case 5:     //次屏切换显示
        switch (Nixie_Show_Mode)
        {
            case 0://数据显示
                if(++Nixie_Show_viceMode > 3)
                {
                    Nixie_Show_viceMode = 0;            
                }
                break;
            case 1://参数界面
            
                if(++Nixie_Show_viceMode > 7)
                {
                    Nixie_Show_viceMode = 0;            
                }
                break;
            case 2://标定界面
                if(++Nixie_Show_viceMode > 4)
                {
                    Nixie_Show_viceMode = 0;            
                }
                break;
        }
        

        break;

    case 8:         //标定界面 加按钮
        if(Nixie_Show_Mode == 2)
        {
            Volt_Weight_Show[Nixie_Show_viceMode] += 1;
            if(Volt_Weight_Show[Nixie_Show_viceMode] >= 50)
            {
                Volt_Weight_Show[Nixie_Show_viceMode] = 50;
            }
            // if(Nixie_Show_viceMode < 4 && Nixie_Show_viceMode > 0)
            // {
            //     if(Volt_Weight_Show[Nixie_Show_viceMode] < Volt_Weight_Show[Nixie_Show_viceMode+1]
            //         && Volt_Weight_Show[Nixie_Show_viceMode] > Volt_Weight_Show[Nixie_Show_viceMode-1])
            //         {
            //             memcpy(Volt_Weight_Set,Volt_Weight_Show,5);
            //         }
            // }
            // else if(Nixie_Show_viceMode == 4)
            // {
            //     if(Volt_Weight_Show[Nixie_Show_viceMode] <= 50)
            //     {
            //         memcpy(Volt_Weight_Set,Volt_Weight_Show,5);
            //     }
            // }
            // else if(Nixie_Show_viceMode == 0)
            // {
            //     if(Volt_Weight_Show[Nixie_Show_viceMode] < Volt_Weight_Show[Nixie_Show_viceMode+1])
            //     {
            //         memcpy(Volt_Weight_Set,Volt_Weight_Show,5);
            //     }
            // }
        }
        

        break;

    case 9:         //标定界面 减按钮
        if(Nixie_Show_Mode == 2)
        {
            Volt_Weight_Show[Nixie_Show_viceMode] -= 1;
            if(Volt_Weight_Show[Nixie_Show_viceMode] > 60000)
            {
                Volt_Weight_Show[Nixie_Show_viceMode] = 0;
            }
            // if(Nixie_Show_viceMode < 4 && Nixie_Show_viceMode > 0)
            // {
            //     if(Volt_Weight_Show[Nixie_Show_viceMode] < Volt_Weight_Show[Nixie_Show_viceMode+1]
            //         && Volt_Weight_Show[Nixie_Show_viceMode] > Volt_Weight_Show[Nixie_Show_viceMode-1])
            //         {
            //             memcpy(Volt_Weight_Set,Volt_Weight_Show,5);
            //         }
            // }
            // else if(Nixie_Show_viceMode == 4)
            // {
            //     if(Volt_Weight_Show[Nixie_Show_viceMode] <= 50)
            //     {
            //         memcpy(Volt_Weight_Set,Volt_Weight_Show,5);
            //     }
            // }
            // else if(Nixie_Show_viceMode == 0)
            // {
            //     if(Volt_Weight_Show[Nixie_Show_viceMode] < Volt_Weight_Show[Nixie_Show_viceMode+1] && Volt_Weight_Show[Nixie_Show_viceMode] >= 0)
            //     {
            //         memcpy(Volt_Weight_Set,Volt_Weight_Show,5);
            //     }
            // }
        }
        break;
    }


}

void Nixie_PutO()         //数码管显示处理
{
    if (Nixie_Slow_Down <= 500) return;
    Nixie_Slow_Down = 0;
	

    //数据处理
    Read_Timer(Timer_Show);             //读取时间

    Tem_Read = rd_Temperature();        //读取温度

    //超声波处理
    Wave_Lenth = (unsigned int)((float)rd_Length_Time() / 1000.0f * (330 + 0.6f * Tem_Read) / 20 );        //读取超声波解析出的长度
    Volumn_Calc(Wave_Lenth,R_Set,L_W_H_Set,S_Type,&Volumn,&Remain_Volumn,&Water_d);

    if(Water_d > H_Set[0])      //500——水位超出范围
    {
        Error_Index = 1;
			if(Error_1 == 0)
			{
				printf("[500,%bu%bu%bu,%u.%u%u]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],Water_d / 100 % 10,Water_d / 10 % 10,Water_d % 10 );
				Error_1 = 1;
			}
    }
    else if (Water_d < H_Set[1])     //501——水位超出范围
    {
        Error_Index = 2;
			if(Error_2 == 0)
			{
				printf("[501,%bu%bu%bu,%u.%u%u]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],Water_d / 100 % 10,Water_d / 10 % 10,Water_d % 10 );
				Error_2 = 1;
			}
    }
    else                //正常
    {
        Error_Index = 0;
			Error_1 = 0;
			Error_2 = 0;
    }

    
// 读取重量
Weight = ad_Read(0x03);        //读取重量 
Weight += (unsigned int)(((float)(0.04f * Tem_Read  * Tem_Read )) - (10 * Tem_Read) + 225); 
Weight = ((Weight * 100 / 51.0 ) + 5) / 10;  //电压模拟重量    数值 高10倍         

// 重量转换 - 使用线性插值

if (Weight < Volt_Weight_Set[0]) {
    // 低于空载值，设为 0
    weight_result = 0;
} else if (Weight >= Volt_Weight_Set[0] && Weight < Volt_Weight_Set[1]) {
    // 0~5吨
    if (Volt_Weight_Set[1] > Volt_Weight_Set[0]) {
        weight_result = (unsigned int)(50.0f * (Weight - Volt_Weight_Set[0]) / (Volt_Weight_Set[1] - Volt_Weight_Set[0]));
    } else {
        weight_result = 0;
    }
} else if (Weight >= Volt_Weight_Set[1] && Weight < Volt_Weight_Set[2]) {
    // 5~10吨
    if (Volt_Weight_Set[2] > Volt_Weight_Set[1]) {
        weight_result = 50 + (unsigned int)(50.0f * (Weight - Volt_Weight_Set[1]) / (Volt_Weight_Set[2] - Volt_Weight_Set[1]));
    } else {
        weight_result = 50;
    }
} else if (Weight >= Volt_Weight_Set[2] && Weight < Volt_Weight_Set[3]) {
    // 10~15吨
    if (Volt_Weight_Set[3] > Volt_Weight_Set[2]) {
        weight_result = 100 + (unsigned int)(50.0f * (Weight - Volt_Weight_Set[2]) / (Volt_Weight_Set[3] - Volt_Weight_Set[2]));
    } else {
        weight_result = 100;
    }
} else if (Weight >= Volt_Weight_Set[3] && Weight < Volt_Weight_Set[4]) {
    // 15~20吨
    if (Volt_Weight_Set[4] > Volt_Weight_Set[3]) {
        weight_result = 150 + (unsigned int)(50.0f * (Weight - Volt_Weight_Set[3]) / (Volt_Weight_Set[4] - Volt_Weight_Set[3]));
    } else {
        weight_result = 150;
    }
} else {
    // 超过20吨
    weight_result = 200;
}

// 确保结果在 0~20 范围内
if (weight_result > 200) {
    weight_result = 200;
} else if (weight_result < 0) {
    weight_result = 0;
}

Weight = weight_result;
    
    
    
    //显示处理
    switch (Nixie_Show_Mode)
    {
    case 0:     //数据
        switch (Nixie_Show_viceMode)
        {
            case 0: //时间
            {
            Nixie_Clear();
                Nixie_Show[0] = Timer_Show[0] / 10 % 10;
                Nixie_Show[1] = Timer_Show[0] % 10;

                Nixie_Show[3] = Timer_Show[1] / 10 % 10;
                Nixie_Show[4] = Timer_Show[1] % 10;

                Nixie_Show[6] = Timer_Show[2] / 10 % 10;
                Nixie_Show[7] = Timer_Show[2] % 10;

                Nixie_Show[2] = Nixie_Show[5] = 11;

                break;
            }
            case 1: //液位
            {
            Nixie_Clear();
                if (Tem_Read < 0)
                {
                    Tem_Read = -Tem_Read;
                    Nixie_Show[0] = 11;     //温度负数符号
                }
                
                Nixie_Show[2] = (unsigned char)Tem_Read / 10 % 10;      //温度  单位：℃
                Nixie_Show[3] = (unsigned char)Tem_Read % 10;

                Nixie_Show[4] = 11;

                if (S_Type == 0)        //球容器时
                {

                    Nixie_Show[5] = (unsigned char)(2*R_Set - Wave_Lenth) / 100 % 10;     //水位长度  单位：cm
                    Nixie_Show[6] = (unsigned char)(2*R_Set - Wave_Lenth) / 10 % 10;
                    Nixie_Show[7] = (unsigned char)(2*R_Set - Wave_Lenth) % 10;   

                    Nixie_Point[5] = 1;
                }
                else 
                {
                    Nixie_Show[5] = (unsigned char)(L_W_H_Set[3] - Wave_Lenth) / 100 % 10;     //水位长度  单位：cm
                    Nixie_Show[6] = (unsigned char)(L_W_H_Set[3] - Wave_Lenth) / 10 % 10;
                    Nixie_Show[7] = (unsigned char)(L_W_H_Set[3] - Wave_Lenth)  % 10;   

                    Nixie_Point[5] = 1;
                }
                
                break;
            }
            case 2: //体积
            {
            Nixie_Clear();
                i = 0;
                Nixie_Show[0] = Remain_Volumn / 100 % 10;      //剩余体积比
                Nixie_Show[1] = Remain_Volumn / 10 % 10;
                Nixie_Show[2] = Remain_Volumn  % 10;

                while(Nixie_Show[i] == 0)
                {
                    Nixie_Show[i] = 10;     //熄灭
                    i++;
                    if(i >= 2)
                    {
                        break;
                    }
                }

                Nixie_Show[3] = 11;

                Nixie_Show[4] = Volumn / 1000 % 10;        //液体体积
                Nixie_Show[5] = Volumn / 100 % 10;     
                Nixie_Show[6] = Volumn / 10 % 10;
                Nixie_Show[7] = Volumn % 10;   

                Nixie_Point[6] = 1;

                i = 4;
                while(Nixie_Show[i] == 0)
                {
                    Nixie_Show[i] = 10;     //熄灭
                    i++;
                    if(i >= 6)
                    {
                        break;
                    }
                }

                break;
            }
            case 3: //重量
            {
            Nixie_Clear();
                if (Tem_Read < 0)
                {
                    Tem_Read = -Tem_Read;
                    Nixie_Show[0] = 11;         //重量负数符号
                }
                
                Nixie_Show[1] = (unsigned char)Tem_Read / 10 % 10;      //温度  单位：℃
                Nixie_Show[2] = (unsigned char)Tem_Read % 10;

                Nixie_Show[3] = 11;

                i = 4;
                Nixie_Show[4] = Weight / 1000 % 10;        //重量
                Nixie_Show[5] = Weight / 100 % 10;     
                Nixie_Show[6] = Weight / 10 % 10;
                Nixie_Show[7] = Weight % 10;   
                Nixie_Point[6] = 1;                         //重量小数点

                
                while (Nixie_Show[i] == 0)
                {
                    Nixie_Show[i] = 10;     //熄灭
                    i++;
                    if(i >= 6)
                    {
                        break;
                    }
                }
                
                break;
            }

        }

        break;

    case 1:     //参数
    Nixie_Clear();
        Nixie_Show[0] = 12;             //P
        Nixie_Show[1] = Nixie_Show_viceMode + 1;        //参数编号
        Nixie_Show[2] = 11;             //-
        i = 4;
        switch (Nixie_Show_viceMode)
        {
        case 0:         //液面上限
            //L_W_H_Set[0] /= 10;        //只保留1位小数
            Nixie_Show[4] = H_Set[0] / 10000 % 10;
            Nixie_Show[5] = H_Set[0] / 1000 % 10; 
            Nixie_Show[6] = H_Set[0] / 100 % 10;
            Nixie_Show[7] = H_Set[0] / 10 % 10;
            
            
            Nixie_Point[6] = 1;
            break;
        case 1:         //液面下限
            //L_W_H_Set[1] /= 10;        //只保留1位小数
            Nixie_Show[4] = H_Set[1] / 10000 % 10;
            Nixie_Show[5] = H_Set[1] / 1000 % 10; 
            Nixie_Show[6] = H_Set[1] / 100 % 10;
            Nixie_Show[7] = H_Set[1] / 10 % 10;
            
            Nixie_Point[6] = 1;
            break;
        case 2:         //频率
            i = 3;
            //F_Set /= 10;        //只保留1位小数
            Nixie_Show[3] = F_Set / 10000 % 10;
            Nixie_Show[4] = F_Set / 1000 % 10; 
            Nixie_Show[5] = F_Set / 100 % 10;
            Nixie_Show[6] = F_Set / 10 % 10;
            Nixie_Show[7] = F_Set % 10; 
            
            break;
        case 3:         //容器类型

            Nixie_Show[7] = S_Type % 10; 

            break;
        case 4:         //半径
            //R_Set /= 10;        //只保留1位小数
            Nixie_Show[4] = R_Set / 10000 % 10;
            Nixie_Show[5] = R_Set / 1000 % 10; 
            Nixie_Show[6] = R_Set / 100 % 10;
            Nixie_Show[7] = R_Set / 10 % 10;
 
            
            Nixie_Point[6] = 1;
            break;
        case 5:         //长
            //L_W_H_Set[0] /= 10;        //只保留1位小数
            Nixie_Show[4] = L_W_H_Set[0] / 10000 % 10;
            Nixie_Show[5] = L_W_H_Set[0] / 1000 % 10; 
            Nixie_Show[6] = L_W_H_Set[0] / 100 % 10;
            Nixie_Show[7] = L_W_H_Set[0] / 10 % 10;
            
            Nixie_Point[6] = 1;
            break;
        case 6:         //宽
            //L_W_H_Set[1] /= 10;        //只保留1位小数
            Nixie_Show[4] = L_W_H_Set[1] / 10000 % 10;
            Nixie_Show[5] = L_W_H_Set[1] / 1000 % 10; 
            Nixie_Show[6] = L_W_H_Set[1] / 100 % 10;
            Nixie_Show[7] = L_W_H_Set[1] / 10 % 10;
            
            Nixie_Point[6] = 1;
            break;
        case 7:         //高
            //L_W_H_Set[2] /= 10;        //只保留1位小数
            Nixie_Show[4] = L_W_H_Set[2] / 10000 % 10;
            Nixie_Show[5] = L_W_H_Set[2] / 1000 % 10; 
            Nixie_Show[6] = L_W_H_Set[2] / 100 % 10;
            Nixie_Show[7] = L_W_H_Set[2] / 10 % 10;
            
            Nixie_Point[6] = 1;
            break;
        }
        while (Nixie_Show[i] == 0)
        {
            Nixie_Show[i] = 10;     //熄灭
            i++;
            if(Nixie_Show_viceMode == 2 || Nixie_Show_viceMode == 3)
            {   
                if(i >= 7)
                {
                    break;
                }
            }
            if(i >= 6)
            {
                break;
            }
        }
        

        break;
    case 2:     //标定
    Nixie_Clear();
        Nixie_Show[0] = 13;             //E
        if(Nixie_Show_viceMode == 0)        //空载
        {
            Nixie_Show[1] = 0;             
            Nixie_Show[2] = 0;   
            
            Nixie_Show[6] = Volt_Weight_Show[0] / 10 % 10;
            Nixie_Show[7] = Volt_Weight_Show[0] % 10;
            Nixie_Point[6] = 1;
        }
        else if(Nixie_Show_viceMode == 1)     //5吨
        {
            Nixie_Show[1] = 0;             
            Nixie_Show[2] = 5;   
            
            Nixie_Show[6] = Volt_Weight_Show[1] / 10 % 10;
            Nixie_Show[7] = Volt_Weight_Show[1] % 10;
            Nixie_Point[6] = 1;
        }
        else if(Nixie_Show_viceMode == 2)     //10吨
        {
            Nixie_Show[1] = 1;             
            Nixie_Show[2] = 0;   
            
            Nixie_Show[6] = Volt_Weight_Show[2] / 10 % 10;
            Nixie_Show[7] = Volt_Weight_Show[2] % 10;
            Nixie_Point[6] = 1;
        }
        else if(Nixie_Show_viceMode == 3)     //15吨
        {
            Nixie_Show[1] = 1;             
            Nixie_Show[2] = 5;   
            
            Nixie_Show[6] = Volt_Weight_Show[3] / 10 % 10;
            Nixie_Show[7] = Volt_Weight_Show[3] % 10;
            Nixie_Point[6] = 1;
        }
        else if(Nixie_Show_viceMode == 4)     //20吨
        {
            Nixie_Show[1] = 2;             
            Nixie_Show[2] = 0;   
            
            Nixie_Show[6] = Volt_Weight_Show[4] / 10 % 10;
            Nixie_Show[7] = Volt_Weight_Show[4] % 10;
            Nixie_Point[6] = 1;
        }

        break;

    
    }
    

}

void other_PutO()         //其他处理
{
	if(Nixie_Show_Mode == 0 && Nixie_Show_viceMode == 0)	//时间显示 led 1 0 0 0
	{
		Led_Enable[0] = 1;
		Led_Enable[1] = 0;
		Led_Enable[2] = 0;
		Led_Enable[3] = 0;
	}
	else if(Nixie_Show_Mode == 0 && Nixie_Show_viceMode == 1)		//液面显示 0 1 0 0
	{
		Led_Enable[0] = 0;
		Led_Enable[1] = 1;
		Led_Enable[2] = 0;
		Led_Enable[3] = 0;
	}
	else if(Nixie_Show_Mode == 0 && Nixie_Show_viceMode == 2)		//体积显示 0 0 1 0
	{
		Led_Enable[0] = 0;
		Led_Enable[1] = 0;
		Led_Enable[2] = 1;
		Led_Enable[3] = 0;
	}
	else if(Nixie_Show_Mode == 0 && Nixie_Show_viceMode == 3)		//体积显示 0 0 0 1
	{
		Led_Enable[0] = 0;
		Led_Enable[1] = 0;
		Led_Enable[2] = 0;
		Led_Enable[3] = 1;
	}
    else 
    {
       Led_Enable[0] = 0;
		Led_Enable[1] = 0;
		Led_Enable[2] = 0;
		Led_Enable[3] = 0; 
    }
	
	if(Error_1 == 1 || Error_2 == 1)		//液面异常指示灯
	{
		Led_Enable[4] = 1;
	}
	else
	{
		Led_Enable[4] = 0;
	}
	
	if(Error_3 == 1)		//频率数值异常指示灯
	{
		Led_Enable[5] = 1;
	}
	else
	{
		Led_Enable[5] = 0;
	}
	
	if(Error_4 == 1)		//频率趋势异常
	{
		Led_Enable[6] = Fight_Led_7;
	}
	else 
	{
		Led_Enable[6] = 0;
	}

	if(Error_5 == 1)		//频率剧烈变换异常
	{
		Led_8_Start_Index = 1;
		Led_Enable[7] = ((Led_8_Start_Index) ? (1 * Fight_Led_8) : 0);
		
	}
    else 
    {
        Led_Enable[7] = 0;
    }
	
	if(Error_1 || Error_2 || Error_3 || Error_4 || Error_5)			//无异常
	{
		Box_On();
	}
	else 
	{
		Box_Off();
	}

}

//串口处理
void Uart_PutO()         //串口处理
{
    unsigned char Uart_pos = 0;
        error = 0;
        ok = 0;
		// Uart_Id = NULL;
		// id_value = NULL;
    if(Uart_Rx_Time >= 10) 
    {
        Uart_Rx = 0;
        Uart_Rx_Time = 0;

        //串口解析(超时解析)    
        /*好处：一次解析完单次发送数据，不用考虑数据是否完整 */
        while(Uart_pos < Uart_Rx_Index)
        {
            Uart_pos = Uart_Get_IdAndValue(Uart_pos,Uart_Id,id_value);

            if(Uart_pos == 0xff)
            {
                error = 1;
                break;
            }

            Uart_Solve_temp(Uart_Id,id_value,&error,&ok);

            Uart_pos++;         //跳过')'

        }

        if(error && ok)
        {
            printf("WARN\r\n");
        }
        else if(error)
        {
            printf("ERROR\r\n");
        }
        else if (ok)
        {
            printf("OK\r\n");
        }


        memset(Uart_Recv,0,Uart_Rx_Index);
        Uart_Rx_Index = 0;

    }

    // 频率超值
        if(freq > F_Set)
        {
            Error_Index = 3;
            if(Error_3 == 0)
            {
                printf("[503,%bu%bu%bu,%u]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],freq );
                Error_3 = 1;
            }
        }
        else
        {
            Error_Index = 0;
            Error_3 = 0;
        }
 
        // 频率变化过大（趋势异常）
        if (Freq_Flag_Old == 0)
        {
            Freq_Flag_Old = 1;
            // 计算实际频率差（考虑溢出）
            freq_diff = ((freq - freq_old) > 60000) ? freq_old - freq : freq - freq_old;
            
            if(freq_diff > 1000)
            {
                Error_Index = 5;
                Freq_Big_Change = 1;
                if(Error_5 == 0)
                {
                    printf("[504,%bu%bu%bu,%u]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],freq_diff );
                    Error_5 = 1;
                }
            }
            else
            {
                Freq_Big_Change = 0;
                Error_Index = 0;
                Error_5 = 0;
            }
        }
        else if (Freq_Flag_Old == 1)
        {
            Freq_Flag_Old = 0;
        }
 
        // 频率连续变化
        if(freq > freq_old)     //上升变化
        {
            Freq_5T++;
            Freq_5D = 0;
            Freq_Flag = 1;        
            if (Freq_5T >= 5)
            {
                Freq_5T = 0;
                Error_Index = 4;
                if(Error_4 == 0)
                {
                    printf("[503,%bu%bu%bu,%bu]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],Freq_Flag );
                    Error_4 = 1;
                }
            }
        }
        else if (freq < freq_old)       //下降变化          
        {
            Freq_5D++;
            Freq_5T = 0;
            Freq_Flag = 0;        
            if (Freq_5D >= 5)
            {
                Freq_5D = 0;
                Error_Index = 4;
                if(Error_4 == 0)
                {
                    printf("[503,%bu%bu%bu,%bu]\r\n",Timer_Show[0],Timer_Show[1],Timer_Show[2],Freq_Flag );
                    Error_4 = 1;
                }
            }
        }
        else        //不变化
        {
            Freq_5T = 0;
            Freq_5D = 0;
            Error_Index = 0;
            Error_4 = 0;
        }
        // 保存旧频率值
        freq_old = freq;
//        
		

	
}


/*======================================================================================================*/
//定时器1处理区
void Timer1Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x18;		//设置定时初值
	TH1 = 0xFC;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时

    ET1 = 1;
    EA = 1;
}
void Timer1Service() interrupt 3
{
    Key_Slow_Down++;
    Nixie_Slow_Down++;
    if(++Nixie_Led_Pos >= 8) Nixie_Led_Pos = 0;
    Nixie_Disp(Nixie_Led_Pos,Nixie_Show[Nixie_Led_Pos],Nixie_Point[Nixie_Led_Pos]);
    Led_Disp(Nixie_Led_Pos,Led_Enable[Nixie_Led_Pos]);
    if(Uart_Rx == 1) Uart_Rx_Time++;

    if(++NE555_1000MS >= 1000)      //计算频率
    {
        TR0 = 0;
        NE555_1000MS = 0;
        freq = TH0 << 8 | TL0;
        TH0 = 0x00;
        TL0 = 0x00; 
        TR0 = 1;

 
        

    }
		
		if(++Time_100Ms >= 100)
		{
            Time_100Ms = 0;
			Fight_Led_7 ^= 1;
		}
    
		
		if(Led_8_Start_Index == 1)
		{
			if(++Time_200Ms >= 200)
			{
				Time_200Ms = 0;
				Fight_Led_8 ^= 1;
			}
			if(++Time_3S >= 3000)
			{
				Led_8_Start_Index = 0;
				Time_200Ms = 0;
                Time_3S = 0;
			}
			
		}

}

void Uart_Service() interrupt 4
{
    if(RI == 1)     //按字符接收
    {
        Uart_Rx = 1;                //接收标记
        Uart_Rx_Time = 0;           //超时初始
            Uart_Recv[Uart_Rx_Index] = SBUF;
            Uart_Rx_Index++;
			Uart_Recv[Uart_Rx_Index] = '\0'; // 添加结束符
            
        RI = 0;
        if(Uart_Rx_Index >= 19)
        {
            memset(Uart_Recv,0,Uart_Rx_Index);
            Uart_Rx_Index = 0;

        }
    }
}

//NE555处理区
void Timer0Init(void)		//0微秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式

    TMOD |= 0x05;		//设置定时器模式 为计数模式并且为16位不重装载

	TL0 = 0x00;		//设置定时初值
	TH0 = 0x00;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}



/*======================================================================================================*/
//主函数
void main()
{
    InitSys();
    Timer1Init();
	Timer0Init();
    rd_Temperature();
    Set_Timer(Timer_Show);
    UartInit();

    while (1)
    {
        Key_PutO();
        Nixie_PutO();
        other_PutO();
        Uart_PutO();
        //Error_Report();
    }
    
}

//提取标识      pos缓存值，方便递归使用
unsigned char Uart_Get_IdAndValue(unsigned char pos,unsigned char *Uart_Id,unsigned char *id_value)
{
    i = 0;
    while(pos < Uart_Rx_Index && Uart_Recv[pos] != '(')
        pos++;

    if(pos >= Uart_Rx_Index) return 0xff;       //未找到

    pos++;      //跳过'('

    //提取标识ID
    while(pos < Uart_Rx_Index && Uart_Recv[pos] != ',' && Uart_Recv[pos] != ':')
    {
        Uart_Id[i++] = Uart_Recv[pos];
        pos++;
        
    }
        Uart_Id[i] = '\0';  
        pos++;      //跳过','


        //提取数值
        i = 0;      //重新计数，记录数值
        while(pos < Uart_Rx_Index && Uart_Recv[pos] != ')')
        {
            id_value[i++] = Uart_Recv[pos];
            pos++;
        }
        id_value[i] = '\0';
        //pos++;      //跳过')'

        return pos;         //返回下一个位置——')'的位置
}


void Volumn_Calc(unsigned int Water_Lenth,unsigned int R_Set,unsigned int *L_W_H_Set,unsigned int S_Type,unsigned int *Volumn,unsigned char *Remain_Volumn,unsigned int *Water_d)
{
	unsigned int Sum_Volumn = 0;   
	*Water_d = 0;
     
	switch(S_Type)
	{
		case 0:     //球
            *Water_d = (2* R_Set - Water_Lenth);      //液面高度
            if (*Water_d > 60000)
            {
                *Water_d = 0;
            }
            
            Sum_Volumn = (unsigned int)(4 / 3 * 3.14f * R_Set/10 * R_Set/10 * R_Set/10 / 100) ;
			*Volumn = (unsigned int)(3.14f * *Water_d/10 * *Water_d/10 * (R_Set/10- (*Water_d / 30)) / 100) ;                 //Π * d^2 * (R - d/3)
            *Remain_Volumn = (unsigned char)( Sum_Volumn / 100.0f * *Volumn);         //剩余体积比
			break;
        case 1:     //圆柱体
            *Water_d = (L_W_H_Set[2] - Water_Lenth);      //液面高度
            Sum_Volumn = (unsigned int)(3.14f * R_Set/10 * R_Set/10 * (L_W_H_Set[2] /10) / 100);
            if(*Water_d < 60000 || *Water_d == 0)
            {
                *Volumn = (unsigned int)(3.14f * R_Set/10 * R_Set/10 * *Water_d/10 / 100);
                *Remain_Volumn = (unsigned char)( Sum_Volumn / 100.0f * *Volumn );         //剩余体积比
            }
            else{
                *Water_d = 0;
                *Volumn = 0;
                *Remain_Volumn = 100;
            }
            
            break;
        case 2:     //立方体
            *Water_d = (L_W_H_Set[2] - Water_Lenth);      //液面高度
            Sum_Volumn = L_W_H_Set[0]/10 * L_W_H_Set[1]/10 * L_W_H_Set[2] /10 / 100;
            if(*Water_d < 60000 || *Water_d == 0)
            {
                *Volumn = L_W_H_Set[0]/10 * L_W_H_Set[1]/10 * *Water_d/10 / 100;
                *Remain_Volumn = (unsigned char)( ((Sum_Volumn -*Volumn) * 100.0f) / (Sum_Volumn * 100.0f) * 100);         //剩余体积比
            }
            else 
            {
                *Water_d = 0;
                *Volumn = 0;
                *Remain_Volumn = 100;
            }
            
            break;
		
	}
   }


