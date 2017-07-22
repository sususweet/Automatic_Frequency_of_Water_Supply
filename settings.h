/*
 * settings.h
 * Author: tangyq
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#define CPU_CLOCK         ((unsigned long)20)                                   // Unit MHZ
#define Delay_us(x)       __delay_cycles((unsigned int)x*CPU_CLOCK)                 // Unit us
#define Delay_ms(x)       __delay_cycles(CPU_CLOCK*(unsigned int)x*1000)            // Unit ms

/**
 * Units:
 * Pressure: kPa
 * Voltage: V
 * Fc: Hz
 * */
/*#define Voltage_to_Fc(voltage) (unsigned int) (voltage * 6849 + 3651)
#define Voltage_to_Pressure(voltage) (float) ((0.018*voltage)  * 1000)
#define Voltage_to_Fc(pressure)  (unsigned int) (pressure/1000*402100 + 5391)
#define Fc_to_Pressure(Fc)  (unsigned int) ((Fc*2.141e-6 - 0.01011) * 1000)*/

#define Voltage_to_Fc(voltage) (unsigned int) (voltage * 6849 + 3651)
#define Voltage_to_Pressure_Show(voltage) (float) ((0.018*voltage - 0.00393)  * 1000 )
#define Voltage_to_Pressure(voltage) (float) ((0.018*voltage)  * 1000 )
#define Pressure_to_Fc(pressure)  (unsigned int) ((((float)pressure) + 3.93)/1000*402100 + 5391)
#define Fc_to_Pressure(Fc)  (unsigned int) ((Fc*2.141e-6 - 0.01011) * 1000)


/*#define CPU_CLOCK       8000000
#define Delay_us(us)    __delay_cycles(CPU_CLOCK/1000000*(us))
#define Delay_ms(ms)    __delay_cycles(CPU_CLOCK/1000*(ms))*/


/*#if CPU_CLOCK == 8000000
#define delay_us(us)    __delay_cycles(8*(us))
            #define delay_ms(ms)    __delay_cycles(8000*(ms))
#else
#pragma error "CPU_CLOCK is defined implicitly!"
#endif*/

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
 * Single Keyboard Interface
 */
#define KEY1_IN P11IN &=BIT1
#define KEY2_IN P11IN &=BIT2
#define KEY3_IN P1IN &=BIT6
#define KEY4_IN P1IN &=BIT7

/*
 * Matrix Keyboard Interface
 */
#define KH1_OUT_H P7OUT |= BIT4
#define KH1_OUT_L P7OUT &= ~BIT4
#define KH2_OUT_H P7OUT |= BIT5
#define KH2_OUT_L P7OUT &= ~BIT5
#define KH3_OUT_H P7OUT |= BIT6
#define KH3_OUT_L P7OUT &= ~BIT6
#define KH4_OUT_H P7OUT |= BIT7
#define KH4_OUT_L P7OUT &= ~BIT7

#define KEYH_OUT_DIR P7DIR |= BIT4 + BIT5 + BIT6 + BIT7
#define KEYL_IN_DIR P2DIR &= ~BIT4; P2DIR &= ~BIT5; P2DIR &= ~BIT6; P2DIR &= ~BIT7

#define KH_OUT_L KH1_OUT_L; KH2_OUT_L; KH3_OUT_L; KH4_OUT_L
#define KH_OUT_H KH1_OUT_H; KH2_OUT_H; KH3_OUT_H; KH4_OUT_H

#define KL1_IN P2DIR &= ~BIT4; P2IN &= BIT4
#define KL2_IN P2DIR &= ~BIT5; P2IN &= BIT5
#define KL3_IN P2DIR &= ~BIT6; P2IN &= BIT6
#define KL4_IN P2DIR &= ~BIT7; P2IN &= BIT7

#define KEY_OUT P7OUT
#define KEY_IN P2IN                            // 定义端口寄存器
/*
 * LCD12864 Interface
 */
#define LCD_RS_RW_EN_DIR_OUT P3DIR |= BIT0 + BIT4 + BIT5   // RS,RW,EN Select output
#define LCD_RS_H P3OUT |= BIT0
#define LCD_RS_L P3OUT &= ~BIT0
#define LCD_RW_H P3OUT |= BIT5
#define LCD_RW_L P3OUT &= ~BIT5
#define LCD_EN_H P3OUT |= BIT4
#define LCD_EN_L P3OUT &= ~BIT4

#define LCD_RST_DIR_OUT P11DIR |= BIT0            // RST Select output
#define LCD_RST_H P11OUT |= BIT0
#define LCD_RST_L P11OUT &= ~BIT0
#define LCD_DataIn P8DIR &= 0x00    //数据口方向设置为输入
#define LCD_DataOut P8DIR |= 0xFF    //数据口方向设置为输出

//#define LED_0_IN P1IN &=BIT1

/*
 * Motor Interface
 */
#define MOTOR_F0 IO_BIT_ALIAS(&P7IN,2)

//TODO: SPWM output interface.

/*
 * Frequency Capture
 */
#define Fre_Capture_IN P2DIR &= ~BIT3; //设置P2.3口为外部脉冲输入口
#define Fre_Capture_Mode P2SEL |= BIT3; //设置P2.3口为功能模块使用,做捕获源

#endif /* SETTINGS_H_ */
