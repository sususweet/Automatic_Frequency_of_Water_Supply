/*
 * settings.h
 *
 *  Created on: 2017年7月17日
 *      Author: tangyq
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

/*
 * ADC1118 Interface
 */
#define ADC_SCLK_H P5OUT |= BIT5
#define ADC_SCLK_L P5OUT &= ~BIT5

#define ADC_SCLK IO_BIT_ALIAS(&P5OUT,5)
#define ADC_DIN IO_BIT_ALIAS(&P5IN,4)
#define ADC_DOUT IO_BIT_ALIAS(&P3OUT,7)
#define ADC_CS IO_BIT_ALIAS(&P1OUT,1)

/*
 * Keyboard Interface
 */
#define KEY1_IN P11IN &=BIT1
#define KEY2_IN P11IN &=BIT2
#define KEY3_IN P1IN &=BIT6
#define KEY4_IN P1IN &=BIT7

/*
 * LCD12864 Interface
 */
#define LCD_RS_H P3OUT |= BIT0
#define LCD_RS_L P3OUT &= ~BIT0
#define LCD_RW_H P3OUT |= BIT5
#define LCD_RW_L P3OUT &= ~BIT5
#define LCD_EN_H P3OUT |= BIT4
#define LCD_EN_L P3OUT &= ~BIT4
#define LCD_RST_H P11OUT |= BIT0
#define LCD_RST_L P11OUT &= ~BIT0
#define LCD_DataIn P8DIR = 0x00    //数据口方向设置为输入
#define LCD_DataOut P8DIR = 0xff    //数据口方向设置为输出

//#define LED_0_IN P1IN &=BIT1
#define LED_D2 IO_BIT_ALIAS(&P3OUT,5)
#define LED_D3 IO_BIT_ALIAS(&P3OUT,4)
#define LED_D4 IO_BIT_ALIAS(&P11OUT,0)
#define LED_D5 IO_BIT_ALIAS(&P11OUT,1)
#define LED_D6 IO_BIT_ALIAS(&P11OUT,2)
#define LED_D7 IO_BIT_ALIAS(&P1OUT,6)
#define LED_D8 IO_BIT_ALIAS(&P1OUT,7)

/*
 * Motor Interface
 */
#define MOTOR_F0 IO_BIT_ALIAS(&P7IN,2)

//TODO: SPWM output interface.

#endif /* SETTINGS_H_ */
