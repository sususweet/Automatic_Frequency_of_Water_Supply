/*
 * ads1118.h
 *
 *  Created on: 2017年7月18日
 *      Author: w
 */

#ifndef ADS1118_H_
#define ADS1118_H_

#define CPU_F ((double)1200000)  //默认1.2-1.3MHZ 只需要随着晶振改变改变数字就可以了
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))


#define   Data_Out  (P3DIR |= BIT0)  // 数据输出方向 DIN
#define   DOUT_H    (P3OUT |= BIT0)
#define   DOUT_L    (P3OUT &=~BIT0)

#define   Data_In   (P3DIR &=~BIT1)  //读回来的数据方向 DOUT
#define   DIN       (P3IN&BIT1)

#define   CLK_Out   (P3DIR |= BIT2)   //时钟线输出方向  CLK
#define   CLK_H     (P3OUT |= BIT2)
#define   CLK_L     (P3OUT &=~BIT2)


#define   CS_Out    (P3DIR |= BIT3)   //片选线输出方向  CS
#define   CS_H      (P3OUT |= BIT3)
#define   CS_L      (P3OUT &=~BIT3)

#define   OS        0X8000



unsigned int Write_SIP(unsigned int temp);
void ADS1118_GPIO();

#endif /* ADS1118_H_ */
