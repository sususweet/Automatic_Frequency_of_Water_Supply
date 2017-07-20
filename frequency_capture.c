/*
 * frequency_capture.c
 *
 *  Created on: 2017��7��19��
 *      Author: w
 */
#include "msp430f5438a.h"
#include "frequency_capture.h"
#include "settings.h"

/*****************************************************
��������:����Timer_A����������
����MSP430��Ƭ����ʱ��A�Ͳ���/�ȽϹ���ģ����ʹ�ã�ʵ�������ȵĲ���
�����õ��˶�ʱ��A��CCI2A�˿ڣ�MSP430F5438a��P2.3���ţ��������ⲿ����
�������ƽ���䣬start,end,����������������������
*****************************************************/

/*
 * init TIMER_A1
 */

void Capture_init()
{
    Fre_Capture_IN;
    Fre_Capture_Mode;

    TA1CTL = TASSEL_1 + ID_0 + TACLR + TAIE + MC_2;//��ʱ��Aʱ���ź�ѡ��ACLK,1��Ƶ,ͬʱ���ö�ʱ��A����ģʽΪ��������ģʽ
    //TASSEL 01 ACLK  10 SMCLK
    //ID 00 /1 01 /2 10 /4 11 /8
    //TACLR clear
    //TAIE  interrupt enabled
    //MC    00 stop 01 up mode 10 continuous mode 11 up then down
    TA1CCTL2 |= CM_1 + SCS + CAP + CCIE + CCIFG; //==���������ز���,CCI2AΪ�����ź�Դ==
    //CM    CM_1 �����ز��� CM_2 �½��� CM_3�����½�������
    //SCS   0�첽���� 1ͬ������
    //CAP   0�Ƚ�ģʽ 1����ģʽ
    //CCIE  �ж�������־λ��
}
