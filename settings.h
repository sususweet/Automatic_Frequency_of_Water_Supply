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
#define ADC_SCLK IO_BIT_ALIAS(&P5OUT,5)
#define ADC_DIN IO_BIT_ALIAS(&P5IN,4)
#define ADC_DOUT IO_BIT_ALIAS(&P3OUT,7)
#define ADC_CS IO_BIT_ALIAS(&P1OUT,1)

/*
 * Keyboard Interface
 */
#define KEY_1 IO_BIT_ALIAS(&P2IN,4)
#define KEY_2 IO_BIT_ALIAS(&P2IN,5)
#define KEY_3 IO_BIT_ALIAS(&P2IN,6)
#define KEY_4 IO_BIT_ALIAS(&P2IN,7)

/*
 * LED bit selection Interface
 */
#define LED_W_DIR P8DIR      // Set to output direction: 0x0FF
#define LED_W_OUT P8OUT      // Set all pins HIGH: 0x0FF

/*
 * LED segment selection Interface
 */
#define LED_D1 IO_BIT_ALIAS(&P3OUT,0)
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
