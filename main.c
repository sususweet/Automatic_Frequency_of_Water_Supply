#include "stdio.h"
#include "msp430f5438a.h"
#include "settings.h"
#include "intrinsics.h"
#include "in430.h"
#include "LCD12864.h"
#include "Keyboard.h"
#include "ADC.h"
#include "ThreePhaseSpwm.h"
#include "frequency_capture.h"

/**
 * main.c
 * Default: MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
 */

#define KEY_WAIT 4    /*键盘扫描延迟周期*/
#define NONE_KEY_CODE 0xFF
#define NONE_KEY_NUM 0
#define LCD_TWINKLE_FREQ 50    /*LCD闪烁周期*/
#define PID_CALCULATE_FREQ 50  /*PID计算周期*/

#define MAX_STANDBY_PRESSURE 3000
#define MAX_WORKING_PRESSURE 5000
#define MIN_WORKING_PRESSURE 20000

#define DEFAULT_STANDBY_PRESSURE 0
#define DEFAULT_WORKING_PRESSURE 0

enum setting_state {
    NORMAL, STANDBY1, STANDBY2, STANDBY3, WORKING1, WORKING2, WORKING3
};

enum motor_state {
    MOTOR_WORKING, MOTOR_STOPPED
};

/*unsigned int */
float voltageArray[50] =  {0};
unsigned int voltageArrayIndex = 0;
float frequencyArray[50] =  {0};
unsigned int frequencyArrayIndex = 0;

unsigned int freq_periodStart = 0;
unsigned int freq_pulseEnd = 0;
unsigned int freq_periodEnd = 0;
unsigned char freq_overflow = 0;             //防止溢出
unsigned char cap_flag = 0;
volatile float frequency;
volatile float voltage;

unsigned int tri_frequency = Fc_Default;

/*为减小占用空间，以正整数形式存储，使用时除10*/
unsigned int standbyPressure = DEFAULT_STANDBY_PRESSURE;
unsigned int workingPressure = DEFAULT_WORKING_PRESSURE;

unsigned char setting_stage = NORMAL;
unsigned char motor_stage = MOTOR_STOPPED;
unsigned char lcd_twinkle_cursor = 0;
unsigned char lcd_twinkle_num = 0;
unsigned char pid_calculate_num = 0;

unsigned char standbyPressureChangeFlag = 1;
unsigned char workingPressureChangeFlag = 1;
unsigned char FCChangeFlag = 1;

const unsigned char line1[16] = {"FC  "};
const unsigned char line2[16] = {"待机压力："};
const unsigned char line3[16] = {"供水压力："};
const unsigned char line41[8] = {"水压"};
const unsigned char line42[8] = {"流量"};
unsigned char displayCache[9];

void LCD_Init_Show();

void LCD_Show_Update();

void LCD_Twinkle_Update();

void opr_key(unsigned char key_num);

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


/*#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {              // 1s溢出中断
    //_NOP();
}*/

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1(void) {             // 10ms溢出中断
    /*0Eh Timer overflow TAxCTL TAIFG Lowest*/
    switch (TA0IV) {
        case 0x0E: {
            //__bis_SR_register(GIE);                    // Enable all interrupt.
            scan_key();
            //pid_calculate_num++;
            lcd_twinkle_num++;

            if (lcd_twinkle_num >= LCD_TWINKLE_FREQ) {   //500MS
                 voltage = ADC();
                 lcd_twinkle_num = 0;
                 voltageArray[voltageArrayIndex] = voltage;
                 voltageArrayIndex ++;
                 if (voltageArrayIndex >= 50) voltageArrayIndex = 0;
                 if (setting_stage == NORMAL) LCD_Show_Update();
                 else LCD_Twinkle_Update();
             }

            /*if (pid_calculate_num >= PID_CALCULATE_FREQ) {
                pid_calculate_num = 0;
                // TODO: PID calculation loop
            }*/

            SPWM_FreqChangeCheck();

            //LCD_Show_Update();
            break;
        }
        default:
            break;
    }
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer_A1_Cap(void) {
    switch (TA1IV) {         //向量查询
        case 0x04: {         //捕获中断
            if (TA1CCTL2 & CM0) {//捕获到上升沿
                TA1CCTL2 = (TA1CCTL2 & (~CM0)) | CM1; //更变设置为下降沿触发
                if (cap_flag == 0) {
                    freq_periodStart = TA1CCR2; //记录初始时间
                    cap_flag = 1; //开始一个周期的捕获
                } else {
                    freq_periodEnd = TA1CCR2;   //一个周期的时间
                    if (!freq_overflow) {
                        frequency = 32768 / (freq_periodEnd - freq_periodStart);
                        frequencyArray[frequencyArrayIndex] = frequency;
                        frequencyArrayIndex ++;
                        if (frequencyArrayIndex >= 50) frequencyArrayIndex = 0;
                    } else { freq_overflow = 0; }
                    cap_flag = 0;
                    //TA1CCTL2 &= ~CCIE;   //关捕获使能
                    //TA1CTL &= ~TAIE;     //关中断使能
                }
            } else if (TA1CCTL2 & CM1) { //==捕获到下降沿==
                TA1CCTL2 = (TA1CCTL2 & (~CM1)) | CM0; //更变设置为上升沿触发
                if (cap_flag == 1) {
                    freq_pulseEnd = TA1CCR2; //记录脉冲宽度结束时间
                }
            }
            break;
        }
        case 0X0E:{                                 // 溢出数加1
            freq_overflow++;
            break;
        }
        default:
            break;
    }
}


void opr_key(unsigned char key_num) {
    switch (key_num) {
        case 1: {
            LCD_Show_Update();
            lcd_twinkle_cursor = 0;
            switch (setting_stage) {
                case WORKING3: {
                    setting_stage = NORMAL;
                    break;
                }
                default: {
                    setting_stage++;
                    //_NOP();
                    break;
                }
            }
            break;
        }
        case 2: {
            switch (setting_stage) {
                case STANDBY1: {
                    standbyPressure += 100;
                    standbyPressureChangeFlag = 1;
                    break;
                }
                case STANDBY2: {
                    standbyPressure += 10;
                    standbyPressureChangeFlag = 1;
                    break;
                }
                case STANDBY3: {
                    standbyPressure += 1;
                    standbyPressureChangeFlag = 1;
                    break;
                }
                case WORKING1: {
                    workingPressure += 100;
                    workingPressureChangeFlag = 1;
                    break;
                }
                case WORKING2: {
                    workingPressure += 10;
                    workingPressureChangeFlag = 1;
                    break;
                }
                case WORKING3: {
                    workingPressure += 1;
                    workingPressureChangeFlag = 1;
                    break;
                }
                case NORMAL: {
                    tri_frequency += 250;
                    FCChangeFlag = 1;
                    SPWM_Change_Freq(tri_frequency);
                    break;
                }
                default:
                    break;

            }
            if (setting_stage != NORMAL) {
                if (standbyPressure > MAX_STANDBY_PRESSURE) standbyPressure = MAX_STANDBY_PRESSURE;
                if (workingPressure > MAX_WORKING_PRESSURE) workingPressure = MAX_WORKING_PRESSURE;
            }
            break;
        }
        case 3: {
            switch (setting_stage) {
                case STANDBY1: {
                    if (standbyPressure / 100 != 0) {
                        standbyPressure -= 100;
                        standbyPressureChangeFlag = 1;
                    }
                    break;
                }
                case STANDBY2: {
                    if (standbyPressure / 100 != 0 || standbyPressure % 100 / 10 != 0) {
                        standbyPressure -= 10;
                        standbyPressureChangeFlag = 1;
                    }
                    break;
                }
                case STANDBY3: {
                    if (standbyPressure / 100 != 0 || standbyPressure % 100 / 10 != 0 ||
                        standbyPressure % 100 % 10 != 0){
                        standbyPressure -= 1;
                        standbyPressureChangeFlag = 1;
                    }

                    break;
                }
                case WORKING1: {
                    if (workingPressure / 100 != 0) {
                        workingPressure -= 100;
                        workingPressureChangeFlag = 1;
                    }
                    break;
                }
                case WORKING2: {
                    if (workingPressure / 100 != 0 || workingPressure % 100 / 10 != 0) {
                        workingPressure -= 10;
                        workingPressureChangeFlag = 1;
                    }
                    break;
                }
                case WORKING3: {
                    if (workingPressure / 100 != 0 || workingPressure % 100 / 10 != 0 ||
                        workingPressure % 100 % 10 != 0){
                        workingPressure -= 1;
                        workingPressureChangeFlag = 1;
                    }
                    break;
                }
                case NORMAL: {
                    tri_frequency -= 250;
                    SPWM_Change_Freq(tri_frequency);
                    FCChangeFlag = 1;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case 4: {                       /*抽水系统启动/停止*/
            switch (motor_stage) {
                case MOTOR_STOPPED: {
                    SPWM_GPIO_INIT();
                    SPWM_CLOCK_INIT();
                    motor_stage = MOTOR_WORKING;
                    break;
                }
                case MOTOR_WORKING: {
                    SPWM_GPIO_OFF();
                    motor_stage = MOTOR_STOPPED;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case 5:
        case 6:
        case 7:
        case 9:
        case 10:
        case 11:
        case 13:
        case 14:
        case 15:
        case 16: {
            unsigned int num = 10;
            switch (key_num) {
                case 5:
                case 6:
                case 7: {
                    num = (unsigned int) (key_num - 4);
                    break;
                }
                case 9:
                case 10:
                case 11: {
                    num = (unsigned int) (key_num - 5);
                    break;
                }
                case 13:
                case 14:
                case 15: {
                    num = (unsigned int) (key_num - 6);
                    break;
                }
                case 16: {
                    num = 0;
                    break;
                }
                default:
                    break;
            }
            switch (setting_stage) {
                case STANDBY1: {
                    standbyPressure = standbyPressure % 100 + num * 100;
                    break;
                }
                case STANDBY2: {
                    standbyPressure = standbyPressure / 100 * 100 + standbyPressure % 100 % 10 + num * 10;
                    break;
                }
                case STANDBY3: {
                    standbyPressure = standbyPressure / 100 * 100 + standbyPressure % 100 / 10 * 10 + num;
                    if (standbyPressure > MAX_STANDBY_PRESSURE) standbyPressure = MAX_STANDBY_PRESSURE;
                    standbyPressureChangeFlag = 1;
                    break;
                }
                case WORKING1: {
                    workingPressure = workingPressure % 100 + num * 100;
                    break;
                }
                case WORKING2: {
                    workingPressure = workingPressure / 100 * 100 + workingPressure % 100 % 10 + num * 10;
                    break;
                }
                case WORKING3: {
                    workingPressure = workingPressure / 100 * 100 + workingPressure % 100 / 10 * 10 + num;
                    if (workingPressure > MAX_WORKING_PRESSURE) workingPressure = MAX_WORKING_PRESSURE;
                    workingPressureChangeFlag = 1;
                    break;
                }
                default:
                    break;
            }

            switch (setting_stage) {
                case WORKING3: {
                    setting_stage = NORMAL;
                    break;
                }
                default: {
                    if (setting_stage != NORMAL) setting_stage++;
                    break;
                }
            }
            LCD_Show_Update();
            lcd_twinkle_cursor = 0;
        }

        default:
            break;
    }
}


void LCD_Init_Show() {
    LCD_Show(1, 0, line1);
    LCD_Show(2, 0, line2);
    LCD_Show(3, 0, line3);
    LCD_Show(4, 0, line41);
    //LCD_Show(4, 4, line42);
    LCD_Show_Update();
}


void LCD_Show_Get_Data(unsigned int variable) {
    displayCache[0] = (unsigned char) (variable / 100 + '0');
    displayCache[1] = (unsigned char) (variable % 100 / 10 + '0');
    displayCache[2] = '.';
    displayCache[3] = (unsigned char) (variable % 100 % 10 + '0');
    displayCache[4] = '\0';
}

void LCD_Twinkle_Update() {
    unsigned int waterFlow = 0;
    unsigned int waterPressure = 0;

    if (lcd_twinkle_cursor == 0) {
        lcd_twinkle_cursor = 1;
        switch (setting_stage) {
            case STANDBY1: {
                displayCache[0] = ' ';
                displayCache[1] = (unsigned char) (standbyPressure % 100 / 10 + '0');
                displayCache[2] = '.';
                displayCache[3] = (unsigned char) (standbyPressure % 100 % 10 + '0');
                displayCache[4] = '\0';
                LCD_Show(2, 5, displayCache);
                break;
            }
            case STANDBY2: {
                displayCache[0] = (unsigned char) (standbyPressure / 100 + '0');
                displayCache[1] = ' ';
                displayCache[2] = '.';
                displayCache[3] = (unsigned char) (standbyPressure % 100 % 10 + '0');
                displayCache[4] = '\0';
                LCD_Show(2, 5, displayCache);
                break;
            }
            case STANDBY3: {
                displayCache[0] = (unsigned char) (standbyPressure / 100 + '0');
                displayCache[1] = (unsigned char) (standbyPressure % 100 / 10 + '0');
                displayCache[2] = '.';
                displayCache[3] = ' ';
                displayCache[4] = '\0';
                LCD_Show(2, 5, displayCache);
                break;
            }
            case WORKING1: {
                displayCache[0] = ' ';
                displayCache[1] = (unsigned char) (workingPressure % 100 / 10 + '0');
                displayCache[2] = '.';
                displayCache[3] = (unsigned char) (workingPressure % 100 % 10 + '0');
                displayCache[4] = '\0';
                LCD_Show(3, 5, displayCache);
                break;
            }
            case WORKING2: {
                displayCache[0] = (unsigned char) (workingPressure / 100 + '0');
                displayCache[1] = ' ';
                displayCache[2] = '.';
                displayCache[3] = (unsigned char) (workingPressure % 100 % 10 + '0');
                displayCache[4] = '\0';
                LCD_Show(3, 5, displayCache);
                break;
            }
            case WORKING3: {
                displayCache[0] = (unsigned char) (workingPressure / 100 + '0');
                displayCache[1] = (unsigned char) (workingPressure % 100 / 10 + '0');
                displayCache[2] = '.';
                displayCache[3] = ' ';
                displayCache[4] = '\0';
                LCD_Show(3, 5, displayCache);
                break;
            }
            default:
                break;
        }

    } else {
        lcd_twinkle_cursor = 0;
        switch (setting_stage) {
            case STANDBY1:
            case STANDBY2:
            case STANDBY3: {
                LCD_Show_Get_Data(standbyPressure);
                LCD_Show(2, 5, displayCache);
                break;
            }
            case WORKING1:
            case WORKING2:
            case WORKING3: {
                LCD_Show_Get_Data(workingPressure);
                LCD_Show(3, 5, displayCache);
                break;
            }
            default:
                break;
        }
    }
    if (FCChangeFlag == 1) {
        displayCache[0] = ' ';
        displayCache[1] = ' ';
        displayCache[2] = ' ';
        displayCache[3] = ' ';
        displayCache[4] = ' ';
        displayCache[5] = '\0';
        LCD_Show(1, 2, displayCache);
        sprintf(displayCache, "%04d", tri_frequency);
        LCD_Show(1, 2, displayCache);
        FCChangeFlag = 0;
    }

    sprintf(displayCache,"%6.5f",voltage);
    /*waterPressure = (unsigned int) (voltage * 10);
    LCD_Show_Get_Data(waterPressure);*/
    LCD_Show(4, 2, displayCache);

    /*waterFlow = (unsigned int) (frequency / 7.5 * 10);
    LCD_Show_Get_Data(waterFlow);
    LCD_Show(4, 6, displayCache);*/

}

void LCD_Show_Update() {
    unsigned int waterFlow = 0;
    unsigned int waterPressure = 0;

   /* //sprintf(displayCache,"%04d",tri_frequency);
    LCD_Show(1, 2, displayCache);*/
    if (FCChangeFlag == 1){
        displayCache[0] = ' ';
        displayCache[1] = ' ';
        displayCache[2] = ' ';
        displayCache[3] = ' ';
        displayCache[4] = ' ';
        displayCache[5] = '\0';
        LCD_Show(1, 2, displayCache);
        sprintf(displayCache,"%04d",tri_frequency);
        LCD_Show(1, 2, displayCache);
        FCChangeFlag = 0;
    }

    if (standbyPressureChangeFlag == 1){
        LCD_Show_Get_Data(standbyPressure);
        LCD_Show(2, 5, displayCache);
        standbyPressureChangeFlag = 0;
    }

    if (workingPressureChangeFlag == 1){
        LCD_Show_Get_Data(workingPressure);
        LCD_Show(3, 5, displayCache);
        workingPressureChangeFlag = 0;
    }

    sprintf(displayCache,"%6.5f",voltage);
    //waterPressure = (unsigned int) (voltage * 10);
    //LCD_Show_Get_Data(waterPressure);
    LCD_Show(4, 2, displayCache);

    /*waterFlow = (unsigned int) (frequency / 7.5 * 10);
    LCD_Show_Get_Data(waterFlow);
    LCD_Show(4, 6, displayCache);*/
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    initClock();
    ADS1118_GPIO_Init();           //initialize the GPIO
    ADS1118_SPI_Init();
    Capture_init();
    Key_GPIO_Init();
    LCD_GPIO_Init();
    LCD_Init();
    LCD_Init_Show();

    _NOP();
    SPWM_Init();
    //SPWM_GPIO_INIT();
    _NOP();

    initTimerA0();

    //SPWM_GPIO_INIT();
    //SPWM_CLOCK_INIT();

    while (1) {
        /*CS_L;
        Value = Write_SIP(0x858b);           //AD数值     Conversion Register
        ConfigRegister = Write_SIP(0x858b);  //配置寄存器 Config Register
        CS_H;
        _NOP(); //断点*/
    }
    return 0;
}
