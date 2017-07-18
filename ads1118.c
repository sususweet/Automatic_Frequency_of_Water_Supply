/*
 * ads1118.c
 *
 *  Created on: 2017年7月18日
 *      Author: w
 */

#include "msp430x26x.h"
#include "intrinsics.h"

#include "ads1118.H"


/************************************************************
*函数名称: Write_SIP(unsigned int temp)
*功能描述: 向ADS118写入配置寄存器值，并且读回AD数值
*入口参数: temp (写入配置寄存器)
*返回数值: Read_Data
* 说  明 ：根据时序图，前16位读的是AD转换数值 后16位读回的配置寄存器
*
* 林诗发
* 日 期 ：14.8.20
**************************************************************/

unsigned int Write_SIP(unsigned int temp)
{
    char i;
    unsigned int Read_Data;
    Data_Out;                   //设置P3.0为输出     DIN -->对应（SOMI）
    Data_In;                    //设置P3.1为数据输入 Dout-->对应（SOMO）;

    CLK_L;
    //CS_L;
    for(i=0;i<16;i++)           //只有在时钟上升沿时数据被锁存，为控制器通过下降沿读取数据
    {
      if((temp&0x8000)==0x8000)  { DOUT_H;}
      else                       { DOUT_L;}
      temp<<=1;

      CLK_H;
     // delay_us(50);

      Read_Data<<=1;
      if(DIN){Read_Data++;}

      CLK_L;
    //  delay_us(50);

    }
    DOUT_L;

  //  CS_H;  //拉高Config Register读错误
    _NOP();

    return Read_Data;

}

/************************************************************
*函数名称: ADS1118_GPIO()
*功能描述: 连接ADS1118的IO口初始化
*入口参数:
*返回数值:
*
* 林诗发
* 日 期 ：14.8.20
**************************************************************/

void ADS1118_GPIO()
{
    Data_Out; //MOSI
    Data_In;  //MISO

    CLK_Out;  //CLK
    CS_Out;   //CS
}
