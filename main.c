#include "msp430f5438a.h"
#include "intrinsics.h"
#include "in430.h"
#include "settings.h"
#include "LCD12864.h"
#include "Keyboard.h"
#include "ADC.h"

const unsigned char line1[16]={"恒压供水系统"};

/**
 * main.c
 * Default: MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
 */

#define KEY_WAIT 4    /*键盘扫描延迟周期*/
#define NONE_KEY_CODE 0xFF
#define NONE_KEY_NUM 0

void scan_key() {
    static unsigned char key_state = KEY_STATE_RELEASE;   /*状态机状态初始化，采用static保存状态*/
    static unsigned char key_code = NONE_KEY_CODE;
    static unsigned char key_num = NONE_KEY_NUM;
    unsigned char pressed = press_key(); /*press_key为检测是否有按键按下的函数*/
    static unsigned char scan_time = 0;
    switch (key_state) {
        case KEY_STATE_RELEASE: {   /*若原始状态为无按键按下RELEASE，同时又检测到按键按下，则状态转换到WAITING*/
            if (pressed == 1) {
                key_state = KEY_STATE_WAITING;
            }
            break;
        }
        case KEY_STATE_WAITING: {   /*原始状态为WAITING，对按键进行多次判断*/
            if (pressed) {
                scan_time++;
                if (scan_time >= KEY_WAIT) {   /*若按键按下的时间超过一定时间，则认为按键按下，读按键*/
                    key_state = KEY_STATE_PRESSED;
                    scan_time = 0;
                    key_code = read_key();  /*read_key为读按键的函数*/
                }
            } else {    /*若按键松开，则恢复到初始状态*/
                scan_time = 0;
                key_state = KEY_STATE_RELEASE;
            }
            break;
        }
        case KEY_STATE_PRESSED: {   /*若按键被确认按下，则等待按键松开再进行操作*/
            if (pressed == 0) {
                key_num = translate_key(key_code);
                opr_key(key_num);  /*opr_key为按键事件响应函数*/
                key_state = KEY_STATE_RELEASE;
                key_code = NONE_KEY_CODE;
                key_num = NONE_KEY_NUM;
            }
            break;
        }
        default:
            break;
    }
}


void initClock(void){
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
    do{
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
        // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }
    while (SFRIFG1 & OFIFG);                   // Test oscillator fault flag
    UCSCTL5 |= DIVM__1 + DIVS__1;              //MCLK=20M；SMCLK=20M；ACLK=32768
}

void initTimerA0(void){
    TA0CCR0 = 164;                        // 32768: 定义中断计数周期1s,时钟频率为32.768kHZ,32768 / 32768 = 1s
    TA0CCTL0 &= ~CCIE;                        // TA0CCR0捕获/比较中断寄存器中断使能

    TA0CCR1 = 164;                             // 定义中断溢出周期5ms
    TA0CCTL1 |= CCIE;                         // TA0CCR0捕获/比较中断寄存器中断使能

    TA0CTL = TASSEL_1 + MC_1 + TACLR + TAIE;         // TASSEL_1: ACLK时钟源, MC_1:增计数模式, TACLR: 清零计时器
}


#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void){              // 1s溢出中断
    _NOP();
}


#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1(void){             // 2ms溢出中断
    /*0Eh Timer overflow TAxCTL TAIFG Lowest*/
    switch(TA0IV) {
        case 0x0E:{
            scan_key();
            break;
        }

        default:break;
    }
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    volatile float Voltage;
    
    __bis_SR_register(GIE);
    initClock();
    ADS1118_GPIO_Init();           //initialize the GPIO
    ADS1118_SPI_Init();
    Key_GPIO_init();
    LCD_GPIO_Init();
    LCD_Init();

    LCD_Show(1, 0, line1);
    initTimerA0();
    /*unsigned int Value;
    unsigned int ConfigRegister;*/

    Voltage = ADC();

    while (1) {



        /*CS_L;
        Value = Write_SIP(0x858b);           //AD数值     Conversion Register
        ConfigRegister = Write_SIP(0x858b);  //配置寄存器 Config Register
        CS_H;
        _NOP(); //断点*/
    }
    return 0;
}
