/*
 * settings.h
 *
 *  Created on: 2017骞�7鏈�17鏃�
 *      Author: tangyq
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

/*
 * ADC1118 Interface
 */
#define ADC_CS P1DIR |= BIT1           // Set P1.1 to output direction
#define ADC_CS_H P1OUT |= BIT1         // Set P1.1 for CS
#define ADC_CS_L P1OUT &=~ BIT1        // Set CS low

#define ADC_DOUT P3SEL |= BIT7                          // P3.7 option select
#define ADC_DIN P5SEL |= BIT4                          // P5.4,5 option select
#define ADC_SCLK P5SEL |= BIT5
//#define AD_P5 P5DIR |= BIT0                          // Set P5.0 to output direction

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
#define LCD_DataIn P8DIR = 0x00    //鏁版嵁鍙ｆ柟鍚戣缃负杈撳叆
#define LCD_DataOut P8DIR = 0xff    //鏁版嵁鍙ｆ柟鍚戣缃负杈撳嚭

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
