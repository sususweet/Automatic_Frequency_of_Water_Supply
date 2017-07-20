/*
 * frequency_capture.c
 *
 *  Created on: 2017年7月19日
 *      Author: w
 */
#include "msp430f5438a.h"
#include "frequency_capture.h"
#include "settings.h"

/*****************************************************
程序描述:利用Timer_A捕获脉冲宽度
利用MSP430单片机定时器A和捕获/比较功能模块结合使用，实现脉冲宽度的测量
程序用到了定时器A的CCI2A端口（MSP430F5438a的P2.3引脚）作捕获外部输入
的脉冲电平跳变，start,end,两个个变量来计算脉冲宽度
*****************************************************/

/*
 * init TIMER_A1
 */

void Capture_init()
{
    Fre_Capture_IN;
    Fre_Capture_Mode;

    TA1CTL = TASSEL_1 + ID_0 + TACLR + TAIE + MC_2;//定时器A时钟信号选择ACLK,1分频,同时设置定时器A计数模式为连续增计模式
    //TASSEL 01 ACLK  10 SMCLK
    //ID 00 /1 01 /2 10 /4 11 /8
    //TACLR clear
    //TAIE  interrupt enabled
    //MC    00 stop 01 up mode 10 continuous mode 11 up then down
    TA1CCTL2 |= CM_1 + SCS + CAP + CCIE + CCIFG; //==输入上升沿捕获,CCI2A为捕获信号源==
    //CM    CM_1 上升沿捕获 CM_2 下降沿 CM_3上升下降都捕获
    //SCS   0异步捕获 1同步捕获
    //CAP   0比较模式 1捕获模式
    //CCIE  中断允许（标志位）
}
