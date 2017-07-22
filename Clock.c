//
// Created by tangyq on 2017/7/22.
//

#include "msp430f5438a.h"
#include "intrinsics.h"
#include "Clock.h"

void initClock(void) {
    /* Initialize System Clock.
     * Modified: ACLK = REFO = 32kHz, MCLK = SMCLK = 8MHz
     */
    P7SEL |= 0x03;                            // P7.0、P7.1端口选择外部低频晶振XT1
    UCSCTL6 &= ~XT1OFF;                       // 使能外部晶振
    UCSCTL6 |= XCAP_3;                        // 设置内部负载电容: 1100，电容3
    UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_0;                        // Set ACLK = XT1

    __bis_SR_register(SCG0);                  // Disable the FLL control loop 关闭FLL控制回路
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx 设置DCOx, MODx

    /*
     * 时钟频率设置，计算公式：
     * 默认n=1;
     * fDCOCLK = D*(N + 1)*(fFLLREFCLK/n)
     * fDCOCLKDIV = (N + 1)*(fFLLREFCLK/n)
     * */
    UCSCTL1 = DCORSEL_7;                     // Select DCO range 40MHz operation  设置DCO振荡范围
    UCSCTL2 = FLLD_1 + 609;                  // Set DCO Multiplier for 20MHz
    /* (N + 1) * FLLRef = Fdco
     * (609 + 1) * 32768 = 20MHz
     * Set FLL Div = fDCOCLK/2
     * */
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
    //__delay_cycles(250000);
    __delay_cycles(1024000);

    // Loop until XT1,XT2 & DCO fault flag is cleared
    do {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
        // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    } while (SFRIFG1 & OFIFG);                   // Test oscillator fault flag
    UCSCTL5 |= DIVM__1 + DIVS__1;              // MCLK=20M；SMCLK=20M；ACLK=32768

    __bis_SR_register(GIE);                    // Enable all interrupt.
}

void initTimerA0(void) {
    TA0CCR0 = 328;                            // 32768: 定义中断计数周期1s,时钟频率为32.768kHZ,32768 / 32768 = 1s
    TA0CCTL0 &= ~CCIE;                        // TA0CCR0捕获/比较中断寄存器中断使能

    TA0CCR1 = 328;                            // 定义中断溢出周期10ms
    TA0CCTL1 &= ~CCIE;                         // TA0CCR0捕获/比较中断寄存器中断使能

    TA0CTL = TASSEL_1 + MC_1 + TACLR + TAIE;         // TASSEL_1: ACLK时钟源, MC_1:增计数模式, TACLR: 清零计时器
}
