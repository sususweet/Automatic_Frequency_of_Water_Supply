/*
 * ads1118.c
 *
 *  Created on: 2017��7��18��
 *      Author: w
 */

#include "msp430x26x.h"
#include "intrinsics.h"

#include "ads1118.H"


/************************************************************
*��������: Write_SIP(unsigned int temp)
*��������: ��ADS118д�����üĴ���ֵ�����Ҷ���AD��ֵ
*��ڲ���: temp (д�����üĴ���)
*������ֵ: Read_Data
* ˵  �� ������ʱ��ͼ��ǰ16λ������ADת����ֵ ��16λ���ص����üĴ���
*
* ��ʫ��
* �� �� ��14.8.20
**************************************************************/

unsigned int Write_SIP(unsigned int temp)
{
    char i;
    unsigned int Read_Data;
    Data_Out;                   //����P3.0Ϊ���     DIN -->��Ӧ��SOMI��
    Data_In;                    //����P3.1Ϊ�������� Dout-->��Ӧ��SOMO��;

    CLK_L;
    //CS_L;
    for(i=0;i<16;i++)           //ֻ����ʱ��������ʱ���ݱ����棬Ϊ������ͨ���½��ض�ȡ����
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

  //  CS_H;  //����Config Register������
    _NOP();

    return Read_Data;

}

/************************************************************
*��������: ADS1118_GPIO()
*��������: ����ADS1118��IO�ڳ�ʼ��
*��ڲ���:
*������ֵ:
*
* ��ʫ��
* �� �� ��14.8.20
**************************************************************/

void ADS1118_GPIO()
{
    Data_Out; //MOSI
    Data_In;  //MISO

    CLK_Out;  //CLK
    CS_Out;   //CS
}
