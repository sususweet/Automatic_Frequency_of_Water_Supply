/*
 * ads1118.h
 *
 *  Created on: 2017��7��18��
 *      Author: w
 */

#ifndef ADS1118_H_
#define ADS1118_H_

#define CPU_F ((double)1200000)  //Ĭ��1.2-1.3MHZ ֻ��Ҫ���ž���ı�ı����־Ϳ�����
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))


#define   Data_Out  (P3DIR |= BIT0)  // ����������� DIN
#define   DOUT_H    (P3OUT |= BIT0)
#define   DOUT_L    (P3OUT &=~BIT0)

#define   Data_In   (P3DIR &=~BIT1)  //�����������ݷ��� DOUT
#define   DIN       (P3IN&BIT1)

#define   CLK_Out   (P3DIR |= BIT2)   //ʱ�����������  CLK
#define   CLK_H     (P3OUT |= BIT2)
#define   CLK_L     (P3OUT &=~BIT2)


#define   CS_Out    (P3DIR |= BIT3)   //Ƭѡ���������  CS
#define   CS_H      (P3OUT |= BIT3)
#define   CS_L      (P3OUT &=~BIT3)

#define   OS        0X8000



unsigned int Write_SIP(unsigned int temp);
void ADS1118_GPIO();

#endif /* ADS1118_H_ */
