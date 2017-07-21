/*
 * ThreePhaseSpwm.h
 *
 *  Created on: 2017年7月19日
 *      Author: tangyq
 */

#ifndef THREEPHASESPWM_H_
#define THREEPHASESPWM_H_

#define M  249                                           // 载波比1000
#define PI 3.14159
#define DeadTime 20                                         // 1us
#define a_m 0.2                                                // a_m:调制度
#define Rad 2.0944                                          // 2PI/3
#define Fc_Default 8715								// Default 10khz
//#define M_Default 450									// Default 10khz

void SPWM_GPIO_INIT();
void SPWM_CLOCK_INIT();
void SPWM_GPIO_OFF();

void SPWM_Init();
void SPWM_FreqChangeCheck();
void SPWM_Calculate();
void SPWM_Change_Freq(unsigned int freq);

#endif /* THREEPHASESPWM_H_ */
