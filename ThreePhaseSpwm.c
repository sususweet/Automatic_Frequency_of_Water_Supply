/** SPWM 发生器---对称规则采样法
*  采用6路PWM产生带有死区deadtime的SPWM波形。其中Tc:三角波（载波）周期，一般在1KHZ以上；这里调制信号频率范围在5HZ~20HZ
*  Wr:正弦波角频率，M：载波比，N=Fc/Fr； CPU :20MHZ,DCO System generate,
*  UP/VP/WP---三相上桥臂，UN/VN/WN---三相下桥臂，
*       PWM_UP|----|P4.2/TB0.2
*       PWM_VP|----|P4.4/TB0.4
*       PWM_WP|----|P4.6/TB0.6
*       PWM_UN|----|P4.3/TB0.3
*       PWM_VN|----|P4.5/TB0.5
*       PWM_WN|----|P4.1/TB0.1
*       Error |----|P7.2/TB0OUTH
*/

#include "msp430f5438a.h"
#include "ThreePhaseSpwm.h"
#include "settings.h"
#include "math.h"
#include "intrinsics.h"

#define POINT_PER_TIME 10
unsigned int IntervalTimer_UA_Tmp[M] = {0};                 // 0 phase temp
float IntervalTimer_UA_SinValue[M] = {0};
unsigned int IntervalTimer_UA[M] = {0};                     // 0 phase
unsigned int IntervalTimer_UB[M] = {0};                     // -120 phase
unsigned int IntervalTimer_UC[M] = {0};                     // +120 phase
unsigned int Fc = Fc_Default;                               // Default 10khz
unsigned char Fc_Change_Flag = 0;
unsigned char SPWM_Calculation_Finished = 1;

void SPWM_GPIO_INIT() {
    P4SEL |= (BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
    P4DIR |= (BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6);               // P4.1~P4.6 Select output low default
    P4OUT |= BIT3;
    P4OUT &= ~BIT2;
    //P7DIR &= ~BIT2;                                                 // P7.2 select input
}

/**
 * Output low level to close MOSFET OR IGBT
 */
void SPWM_GPIO_OFF() {
    P4SEL &=~(BIT1+BIT2+BIT3+BIT4+BIT5+BIT6);
    P4DIR |= (BIT1+BIT2+BIT3+BIT4+BIT5+BIT6);
    P4OUT &=~(BIT1+BIT2+BIT3+BIT4+BIT5+BIT6);
    TBCTL &= ~TBIE;
}

void SPWM_CLOCK_INIT(){
    TBCCR0 = CPU_CLOCK * 1000000 / (Fc * 2);                            // PWM Period
    // U相，Tdeadtime = Tdco*(TBCCR2-TBCCR3)=1us;
    TBCCTL2 = OUTMOD_6;                                     // CCR2 Toggle/Set
    TBCCR2 = IntervalTimer_UA[0];                           // CCR2 PWM Duty Cycle
    TBCCTL3 = OUTMOD_2;                                     // CCR3 reset/Reset
    TBCCR3 = IntervalTimer_UA[0] - DeadTime;                  // CCR3 PWM Duty Cycle
    // V相，Tdeadtime = Tdco*(TBCCR4-TBCCR5)=1us;
    TBCCTL4 = OUTMOD_6;                                     // CCR4 Toggle/Set
    TBCCR4 = IntervalTimer_UB[0];                           // CCR4 PWM Duty Cycle
    TBCCTL5 = OUTMOD_2;                                     // CCR5 reset/Reset
    TBCCR5 = IntervalTimer_UB[0] - DeadTime;                        // CCR5 PWM Duty Cycle
    // W相，Tdeadtime = Tdco*(TBCCR2-TBCCR3)=1us;
    TBCCTL6 = OUTMOD_6;                                     // CCR6 Toggle/Set
    TBCCR6 = IntervalTimer_UC[0];                           // CCR6 PWM Duty Cycle
    TBCCTL1 = OUTMOD_2;                                     // CCR1 reset/Reset
    TBCCR1 = IntervalTimer_UC[0] - DeadTime;                        // CCR1 PWM Duty Cycle
    TBCTL = TBSSEL_2 + MC_3 + TBCLR + TBIE;                 // SMCLK=20M, contmode, clear TBR
}


/**
 *  Change frequency of Triangle Wave to change the frequency and amplitude of output sine wave.
 *  Fc range from 5000 to 20000 Hz
 */
void SPWM_Change_Freq(unsigned int freq) {
    Fc = freq;
    Fc_Change_Flag = 1;
}

void SPWM_Init() {
    unsigned int i;
    unsigned int iu, iv, iw;
    SPWM_GPIO_OFF();
    //SPWM_GPIO_INIT();

    //U
    for (iu = 0; iu < M; iu++) {
        IntervalTimer_UA_SinValue[iu] = (float) ((0.25 - a_m * sin(2 * PI * iu / M)) * CPU_CLOCK * 1000000);
    }

    for (iu = 0; iu < M; iu++) {
        IntervalTimer_UA[iu] = (unsigned int) (IntervalTimer_UA_SinValue[iu] / Fc);
    }
    __no_operation();
    //V
    for (iu = 0, iv = iu + M / 3 ; iu < M, iv < M; iu++,iv++) {
        IntervalTimer_UB[iv] = IntervalTimer_UA[iu];
    }
    for (iv = M / 3 - 1, iu = M - 1 ; iv > 0 ; iu--,iv--) {
        IntervalTimer_UB[iv] = IntervalTimer_UA[iu];
    }
    IntervalTimer_UB[0] = IntervalTimer_UA[2 * M / 3];
    __no_operation();

    //W
    for (iu = 0, iw = iu + 2 * M / 3 ; iu< M,iw < M; iu++,iw++) {
        IntervalTimer_UC[iw] = IntervalTimer_UA[iu];
    }
    for (iw = 2 * M / 3 - 1, iu = M - 1 ; iw > 0 ; iu--,iw--) {
        IntervalTimer_UC[iw] = IntervalTimer_UA[iu];
    }
    IntervalTimer_UC[0] = IntervalTimer_UA[M / 6];
    __no_operation();
    //SPWM_CLOCK_INIT();

    /*//U
    for (i = 0; i < M; i++) {
        IntervalTimer_UA[i] = (unsigned int) ((0.25 - a_m * sin(2 * PI * i / M)) * CPU_CLOCK * 1000000 / Fc);
    }
    __no_operation();
    //V
    for (i = 0; i < M; i++) {
        IntervalTimer_UB[i] = (unsigned int) ((0.25 - a_m * sin(2 * PI * i / M - Rad)) * CPU_CLOCK * 1000000 / Fc);
    }
    __no_operation();
    //W
    for (i = 0; i < M; i++) {
        IntervalTimer_UC[i] = (unsigned int) ((0.25 - a_m * sin(2 * PI * i / M + Rad)) * CPU_CLOCK * 1000000 / Fc);
    }
    __no_operation();*/
}

void SPWM_FreqChangeCheck() {
    unsigned int i;

    if (Fc_Change_Flag == 1) {
        Fc_Change_Flag = 0;
        SPWM_Calculation_Finished = 0;

        /* for (i = 0; i < M; i++) {
            IntervalTimer_UA[i] = (unsigned int) ((0.25 - a_m * sin(2 * PI * i / M)) * CPU_CLOCK * 1000000 / Fc);
        }
        __no_operation();
        //V
        for (i = 0; i < M; i++) {
            IntervalTimer_UB[i] = (unsigned int) ((0.25 - a_m * sin(2 * PI * i / M - Rad)) * CPU_CLOCK * 1000000 / Fc);
        }
        __no_operation();
        //W
        for (i = 0; i < M; i++) {
            IntervalTimer_UC[i] = (unsigned int) ((0.25 - a_m * sin(2 * PI * i / M + Rad)) * CPU_CLOCK * 1000000 / Fc);
        }*/
    }
    if (SPWM_Calculation_Finished == 0){
        SPWM_Calculate();
    }
}

void SPWM_Calculate(){
    static unsigned int iu_tmp = 0;
    unsigned int iu, iv, iw;

    unsigned int point = 0;
    if (Fc_Change_Flag == 1) iu_tmp = 0;

    //U_TMP
    for (; iu_tmp < M, point < POINT_PER_TIME; iu_tmp++, point++) {
        IntervalTimer_UA_Tmp[iu_tmp] = (unsigned int) (IntervalTimer_UA_SinValue[iu_tmp] / Fc);
    }
    __no_operation();
    if (iu_tmp >= M) {
        SPWM_Calculation_Finished = 1;

        TBCCR0 = CPU_CLOCK * 1000000 / (Fc * 2);

        //U
        for (iu = 0; iu < M; iu++) {
            IntervalTimer_UA[iu] = IntervalTimer_UA_Tmp[iu];
        }
        __no_operation();

        //V
        for (iu = 0, iv = iu + M / 3 ; iu < M,iv < M; iu++,iv++) {
            IntervalTimer_UB[iv] = IntervalTimer_UA[iu];
        }
        for (iv = M / 3 - 1, iu = M - 1 ; iv > 0 ; iu--,iv--) {
            IntervalTimer_UB[iv] = IntervalTimer_UA[iu];
        }
        IntervalTimer_UB[0] = IntervalTimer_UA[2 * M / 3];
        __no_operation();

        //W
        for (iu = 0, iw = iu + 2 * M / 3 ; iu< M,iw < M; iu++,iw++) {
            IntervalTimer_UC[iw] = IntervalTimer_UA[iu];
        }
        for (iw = 2 * M / 3 - 1, iu = M - 1 ; iw > 0 ; iu--,iw--) {
            IntervalTimer_UC[iw] = IntervalTimer_UA[iu];
        }
        IntervalTimer_UC[0] = IntervalTimer_UA[M / 6];
        __no_operation();

    }

    /*//W
    for (iu = 0, iw = iu + 2 * M / 3 ; iu< M,iw < M; iu++,iw++) {
        IntervalTimer_UC[iw] = IntervalTimer_UA[iu];
    }
    for (iw = 2 * M / 3 - 1, iu = M - 1 ; iw > 0 ; iu--,iw--) {
        IntervalTimer_UC[iw] = IntervalTimer_UA[iu];
    }
    IntervalTimer_UC[0] = IntervalTimer_UA[M / 6];*/
    //__no_operation();
}

#pragma vector=TIMERB1_VECTOR
__interrupt void TIMERB1_ISR(void) {
    static unsigned int k = 1;
    switch (TBIV) {         //向量查询
        case 0x04: {         //捕获中断
            //_NOP();
            break;
        }
        case 0x0E: {
            TBCTL = TBSSEL_2 + MC_3 + TBCLR + TBIE;                 // SMCLK=20M, contmode, clear TBR
            //U
            TBCCR2 = IntervalTimer_UA[k];                           // CCR2 PWM Duty Cycle
            TBCCR3 = IntervalTimer_UA[k] - DeadTime;                        // CCR3 PWM Duty Cycle
            //V
            TBCCR4 = IntervalTimer_UB[k];                           // CCR2 PWM Duty Cycle
            TBCCR5 = IntervalTimer_UB[k] - DeadTime;                        // CCR3 PWM Duty Cycle
            //W
            TBCCR6 = IntervalTimer_UC[k];                           // CCR2 PWM Duty Cycle
            TBCCR1 = IntervalTimer_UC[k] - DeadTime;                        // CCR3 PWM Duty Cycle
            k += 1;
            if (k >= M) k = 0;
            break;
        }
        default: {
            //_NOP();
            break;
        }
    }
}

/*
void KEY_GPIO_INIT() {
    P10DIR &=~(BIT6+BIT7);                                  //P10.6,7 Configure input port
}
// DCP-200 AN1,AN2 used to setting tri-wave frequency range 5k to 20k
void Key_Control()
{
    unsigned char KeyValue ;
    if((P10IN&(BIT6+BIT7))!=(BIT6+BIT7))
    {
        Delay_ms(10);
        if((P10IN&(BIT6+BIT7))!=(BIT6+BIT7))
        {
            if((P10IN&BIT6) != BIT6)                          //AN1 pressed down
            {
                KeyValue = 1;
            }
            if((P10IN&BIT7) != BIT7)                          //AN2 pressed down
            {
                KeyValue = 2;
            }
        }
        while((P10IN&(BIT6+BIT7))!=(BIT6+BIT7))
        {
            Delay_ms(1);
        }
        Delay_ms(10);
        Fc_Change_Flag = 1;
        if(KeyValue == 1)                                   // Add 500HZ Fc
        {
            Fc += 500;
            if(Fc >= 20000)
            {
                Fc = 20000;
            }
        }
        if(KeyValue == 2)                                   // Sub 500HZ Fc
        {
            Fc -= 500;
            if(Fc <= 5000)
            {
                Fc = 5000;
            }
        }
        if(Fc <= 5000)
        {
            LED2_ON;
            LED1_OFF;
        }
        else if(Fc >= 20000)
        {
            LED1_ON;
            LED2_OFF;
        }
        else
        {
            LED1_OFF;
            LED2_OFF;
        }
    }
}
*/
