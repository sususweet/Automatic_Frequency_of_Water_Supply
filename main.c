#include "stdio.h"
#include "msp430f5438a.h"
#include "intrinsics.h"
#include "in430.h"
#include "settings.h"
#include "LCD12864.h"
#include "Keyboard.h"
#include "ADC.h"

/**
 * main.c
 * Default: MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
 */

#define KEY_WAIT 8    /*键盘扫描延迟周期*/
#define NONE_KEY_CODE 0xFF
#define NONE_KEY_NUM 0
#define LCD_TWINKLE_FREQ 100    /*LCD闪烁周期*/
enum setting_state {
    NORMAL, STANDBY1, STANDBY2, STANDBY3, WORKING1, WORKING2, WORKING3
};

/*为减小占用空间，以正整数形式存储，使用时除10*/
unsigned int standbyPressure = 20;
unsigned int workingPressure = 0;
unsigned int waterPressure = 0;
unsigned int waterFlow = 0;

unsigned char setting_stage = NORMAL;
unsigned char lcd_twinkle_cursor = 0;
unsigned char lcd_twinkle_num = 0;

const unsigned char line1[16]={"恒压供水系统"};
const unsigned char line2[16]={"待机压力："};
const unsigned char line3[16]={"供水压力："};
const unsigned char line41[8]={"水压"};
const unsigned char line42[8]={"流量"};
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
    TA0CCR0 = 164;                            // 32768: 定义中断计数周期1s,时钟频率为32.768kHZ,32768 / 32768 = 1s
    TA0CCTL0 &= ~CCIE;                        // TA0CCR0捕获/比较中断寄存器中断使能

    TA0CCR1 = 164;                            // 定义中断溢出周期5ms
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
            lcd_twinkle_num++;

            if (lcd_twinkle_num >= LCD_TWINKLE_FREQ) {
                lcd_twinkle_num = 0;
                if (setting_stage == NORMAL) LCD_Show_Update();
                else LCD_Twinkle_Update();
            }

            //LCD_Show_Update();
            break;
        }

        default:break;
    }
}

void opr_key(unsigned char key_num) {
    switch (key_num) {
        case 1: {
            LCD_Show_Update();
            lcd_twinkle_cursor = 0;
            switch (setting_stage) {
                case WORKING3:{
                    setting_stage = NORMAL;
                    break;
                }
                default:{
                    setting_stage ++;
                    _NOP();
                    break;
                }
            }
            break;
        }
        case 2: {
            switch (setting_stage) {
                case STANDBY1:{
                    standbyPressure += 100;
                    break;
                }
                case STANDBY2:{
                    standbyPressure += 10;
                    break;
                }
                case STANDBY3:{
                    standbyPressure += 1;
                    break;
                }
                case WORKING1:{
                    workingPressure += 100;
                    break;
                }
                case WORKING2:{
                    workingPressure += 10;
                    break;
                }
                case WORKING3:{
                    workingPressure += 1;
                    break;
                }
                default:break;

            }
            if (standbyPressure > 300) standbyPressure = 300;
            if (workingPressure > 500) workingPressure = 500;
            break;
        }
        case 3: {
            switch (setting_stage) {
                case STANDBY1:{
                    if (standbyPressure / 100 != 0) standbyPressure -= 100;
                    break;
                }
                case STANDBY2:{
                    if (standbyPressure / 100 != 0 || standbyPressure % 100 / 10 != 0) standbyPressure -= 10;
                    break;
                }
                case STANDBY3:{
                    if (standbyPressure / 100 != 0 || standbyPressure % 100 / 10 != 0 || standbyPressure % 100 % 10 != 0) standbyPressure -= 1;
                    break;
                }
                case WORKING1:{
                    if (workingPressure / 100 != 0) workingPressure -= 100;
                    break;
                }
                case WORKING2:{
                    if (workingPressure / 100 != 0 || workingPressure % 100 / 10 != 0) workingPressure -= 10;
                    break;
                }
                case WORKING3:{
                    if (workingPressure / 100 != 0 || workingPressure % 100 / 10 != 0 || workingPressure % 100 % 10 != 0) workingPressure -= 1;
                    break;
                }
                default:break;
            }
            break;
        }
        /*抽水系统启动/停止*/
        case 4: {
            switch (setting_stage) {
                case STANDBY1:{

                    break;
                }
                default:break;

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
        case 16:{
            unsigned int num = 10;
            switch (key_num){
                case 5:
                case 6:
                case 7:{
                    num = (unsigned int) (key_num - 4);
                    break;
                }
                case 9:
                case 10:
                case 11:{
                    num = (unsigned int) (key_num - 5);
                    break;
                }
                case 13:
                case 14:
                case 15:{
                    num = (unsigned int) (key_num - 6);
                    break;
                }
                case 16:{
                    num = 0;
                    break;
                }
                default:break;
            }
            switch (setting_stage) {
                case STANDBY1:{
                    standbyPressure = standbyPressure % 100 + num * 100;
                    break;
                }
                case STANDBY2:{
                    standbyPressure = standbyPressure / 100 * 100 + standbyPressure % 100 % 10 + num * 10;
                    break;
                }
                case STANDBY3:{
                    standbyPressure = standbyPressure / 100 * 100 + standbyPressure % 100 / 10 * 10 + num;
                    break;
                }
                case WORKING1:{
                    workingPressure = workingPressure % 100 + num * 100;
                    break;
                }
                case WORKING2:{
                    workingPressure = workingPressure / 100 * 100 + workingPressure % 100 % 10 + num * 10;
                    break;
                }
                case WORKING3:{
                    workingPressure = workingPressure / 100 * 100 + workingPressure % 100 / 10 * 10 + num;
                    break;
                }
                default:break;
            }

            switch (setting_stage){
                case WORKING3:{
                    setting_stage = NORMAL;
                    break;
                }
                default:{
                    setting_stage ++;
                    break;
                }
            }
            LCD_Show_Update();
            lcd_twinkle_cursor = 0;
        }

        default: break;
    }
}


void LCD_Init_Show(){
    LCD_Show(1, 0, line1);
    LCD_Show(2, 0, line2);
    LCD_Show(3, 0, line3);
    LCD_Show(4, 0, line41);
    LCD_Show(4, 4, line42);
}

void LCD_Show_Get_Data(unsigned int variable){
    displayCache[0] = (unsigned char) (variable / 100 + '0');
    displayCache[1] = (unsigned char) (variable % 100 / 10 + '0');
    displayCache[2] = '.';
    displayCache[3] = (unsigned char) (variable % 100 % 10 + '0');
}

void LCD_Twinkle_Update(){
    if (lcd_twinkle_cursor == 0){
        lcd_twinkle_cursor = 1;
        switch (setting_stage) {
            case STANDBY1:{
                displayCache[0] = ' ';
                displayCache[1] = (unsigned char) (standbyPressure % 100 / 10 + '0');
                displayCache[2] = '.';
                displayCache[3] = (unsigned char) (standbyPressure % 100 % 10 + '0');
                LCD_Show(2, 5, displayCache);
                break;
            }
            case STANDBY2:{
                displayCache[0] = (unsigned char) (standbyPressure / 100 + '0');
                displayCache[1] = ' ';
                displayCache[2] = '.';
                displayCache[3] = (unsigned char) (standbyPressure % 100 % 10 + '0');
                LCD_Show(2, 5, displayCache);
                break;
            }
            case STANDBY3:{
                displayCache[0] = (unsigned char) (standbyPressure / 100 + '0');
                displayCache[1] = (unsigned char) (standbyPressure % 100 / 10 + '0');
                displayCache[2] = '.';
                displayCache[3] = ' ';
                LCD_Show(2, 5, displayCache);
                break;
            }
            case WORKING1:{
                displayCache[0] = ' ';
                displayCache[1] = (unsigned char) (workingPressure % 100 / 10 + '0');
                displayCache[2] = '.';
                displayCache[3] = (unsigned char) (workingPressure % 100 % 10 + '0');
                LCD_Show(3, 5, displayCache);
                break;
            }
            case WORKING2:{
                displayCache[0] = (unsigned char) (workingPressure / 100 + '0');
                displayCache[1] = ' ';
                displayCache[2] = '.';
                displayCache[3] = (unsigned char) (workingPressure % 100 % 10 + '0');
                LCD_Show(3, 5, displayCache);
                break;
            }
            case WORKING3:{
                displayCache[0] = (unsigned char) (workingPressure / 100 + '0');
                displayCache[1] = (unsigned char) (workingPressure % 100 / 10 + '0');
                displayCache[2] = '.';
                displayCache[3] = ' ';
                LCD_Show(3, 5, displayCache);
                break;
            }
            default:break;
        }

    }else{
        lcd_twinkle_cursor = 0;
        switch (setting_stage) {
            case STANDBY1:
            case STANDBY2:
            case STANDBY3:{
                LCD_Show_Get_Data(standbyPressure);
                LCD_Show(2, 5, displayCache);
                break;
            }
            case WORKING1:
            case WORKING2:
            case WORKING3:{
                LCD_Show_Get_Data(workingPressure);
                LCD_Show(3, 5, displayCache);
                break;
            }
            default:break;
        }
    }

    LCD_Show_Get_Data(waterPressure);
    LCD_Show(4, 2, displayCache);
    LCD_Show_Get_Data(waterFlow);
    LCD_Show(4, 6, displayCache);
}

void LCD_Show_Update(){
    LCD_Show_Get_Data(standbyPressure);
    LCD_Show(2, 5, displayCache);
    LCD_Show_Get_Data(workingPressure);
    LCD_Show(3, 5, displayCache);
    LCD_Show_Get_Data(waterPressure);
    LCD_Show(4, 2, displayCache);
    LCD_Show_Get_Data(waterFlow);
    LCD_Show(4, 6, displayCache);
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
    LCD_Init_Show();

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
