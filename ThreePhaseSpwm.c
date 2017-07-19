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

#define M 1000                                             // 载波比1000
#define PI 3.14159
#define DeadTime 20                                         // 1us
#define a_m 0.05                                                // a_m:调制度
#define Rad 2.0944                                          // 2PI/3

unsigned int IntervalTimer_UA[M] = {0};                     // 0 phase
unsigned int IntervalTimer_UB[M] = {0};                     // -120 phase
unsigned int IntervalTimer_UC[M] = {0};                     // +120 phase
unsigned int Fc = 10000;									// Default 10khz
unsigned char Fc_Change_Flag = 0;

void SPWM_GPIO_INIT() {
    P4SEL |= (BIT1+BIT2+BIT3+BIT4+BIT5+BIT6);
    P4DIR |= (BIT1+BIT2+BIT3+BIT4+BIT5+BIT6);               //P4.1~P4.6 Select output low default
    P4OUT |= BIT3;
    P4OUT &= ~BIT2;
    P7DIR &= ~BIT2;                                          // P7.2 select input
}

void SPWM_Init() {
    unsigned int i;
    SPWM_GPIO_INIT();
    //KEY_GPIO_INIT();
    //U
    for(i=0;i<M;i++)
    {
        IntervalTimer_UA[i] = (unsigned int)((0.25-a_m*sin(2*PI*i/M))*CPU_CLOCK*1000000/Fc);
    }
    __no_operation();
    //V
    for(i=0;i<M;i++)
    {
        IntervalTimer_UB[i] = (unsigned int)((0.25-a_m*sin(2*PI*i/M-Rad))*CPU_CLOCK*1000000/Fc);
    }
    __no_operation();
    //W
    for(i=0;i<M;i++)
    {
        IntervalTimer_UC[i] = (unsigned int)((0.25-a_m*sin(2*PI*i/M+Rad))*CPU_CLOCK*1000000/Fc);
    }
    __no_operation();
    TBCCR0 = CPU_CLOCK* 1000000/(Fc*2);     						// PWM Period
    // U相，Tdeadtime = Tdco*(TBCCR2-TBCCR3)=1us;
    TBCCTL2 = OUTMOD_6;                                     // CCR2 Toggle/Set
    TBCCR2 = IntervalTimer_UA[0];                           // CCR2 PWM Duty Cycle
    TBCCTL3 = OUTMOD_2;                                     // CCR3 reset/Reset
    TBCCR3 = IntervalTimer_UA[0]-DeadTime;                  // CCR3 PWM Duty Cycle
    // V相，Tdeadtime = Tdco*(TBCCR4-TBCCR5)=1us;
    TBCCTL4 = OUTMOD_6;                                     // CCR4 Toggle/Set
    TBCCR4 = IntervalTimer_UB[0];                           // CCR4 PWM Duty Cycle
    TBCCTL5 = OUTMOD_2;                                     // CCR5 reset/Reset
    TBCCR5 = IntervalTimer_UB[0]-DeadTime;                        // CCR5 PWM Duty Cycle
    // W相，Tdeadtime = Tdco*(TBCCR2-TBCCR3)=1us;
    TBCCTL6 = OUTMOD_6;                                     // CCR6 Toggle/Set
    TBCCR6 = IntervalTimer_UC[0];                           // CCR6 PWM Duty Cycle
    TBCCTL1 = OUTMOD_2;                                     // CCR1 reset/Reset
    TBCCR1 = IntervalTimer_UC[0]-DeadTime;                        // CCR1 PWM Duty Cycle
    TBCTL = TBSSEL_2 + MC_3 + TBCLR + TBIE;                 // SMCLK=20M, contmode, clear TBR
}

void SPWM_FreqChangeCheck(){
    unsigned int i;
    if(Fc_Change_Flag == 1)
    {
        Fc_Change_Flag = 0;
        TBCCR0 = CPU_CLOCK*1000000/(Fc*2);
        for(i=0;i<M;i++)
        {
            IntervalTimer_UA[i] = (unsigned int)((0.25-a_m*sin(2*PI*i/M))*CPU_CLOCK*1000000/Fc);
        }
        __no_operation();
        //V
        for(i=0;i<M;i++)
        {
            IntervalTimer_UB[i] = (unsigned int)((0.25-a_m*sin(2*PI*i/M-Rad))*CPU_CLOCK*1000000/Fc);
        }
        __no_operation();
        //W
        for(i=0;i<M;i++)
        {
            IntervalTimer_UC[i] = (unsigned int)((0.25-a_m*sin(2*PI*i/M+Rad))*CPU_CLOCK*1000000/Fc);
        }
    }
}

#pragma vector=TIMERB1_VECTOR
__interrupt void TIMERB1_ISR(void)
{
    static unsigned int k = 1;
    TBCTL = TBSSEL_2 + MC_3 + TBCLR + TBIE;
    //U
    TBCCR2 = IntervalTimer_UA[k];                           // CCR2 PWM Duty Cycle
    TBCCR3 = IntervalTimer_UA[k]-DeadTime;                        // CCR3 PWM Duty Cycle
    //V
    TBCCR4 = IntervalTimer_UB[k];                           // CCR2 PWM Duty Cycle
    TBCCR5 = IntervalTimer_UB[k]-DeadTime;                        // CCR3 PWM Duty Cycle
    //W
    TBCCR6 = IntervalTimer_UC[k];                           // CCR2 PWM Duty Cycle
    TBCCR1 = IntervalTimer_UC[k]-DeadTime;                        // CCR3 PWM Duty Cycle
    k +=1;
    if(k>M) k = 0;
}

/**
 *  Change frequency of Triangle Wave to change the frequency and amplitude of output sine wave.
 *  Fc range from 5000 to 20000 Hz
 */
void SPWM_Change_Freq(unsigned int freq){
    Fc = freq;
    Fc_Change_Flag = 1;
    return;
}


void SPWM_Stop(){
    TBCTL &= ~TBIE;
    return;
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
