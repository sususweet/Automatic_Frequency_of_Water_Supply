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

#define KEY_WAIT 8    /*����ɨ���ӳ�����*/
#define NONE_KEY_CODE 0xFF
#define NONE_KEY_NUM 0
#define LCD_TWINKLE_FREQ 100    /*LCD��˸����*/
enum setting_state {
    NORMAL, STANDBY1, STANDBY2, STANDBY3, WORKING1, WORKING2, WORKING3
};

/*Ϊ��Сռ�ÿռ䣬����������ʽ�洢��ʹ��ʱ��10*/
unsigned int standbyPressure = 20;
unsigned int workingPressure = 0;
unsigned int waterPressure = 0;
unsigned int waterFlow = 0;

unsigned char setting_stage = NORMAL;
unsigned char lcd_twinkle_cursor = 0;
unsigned char lcd_twinkle_num = 0;

const unsigned char line1[16]={"��ѹ��ˮϵͳ"};
const unsigned char line2[16]={"����ѹ����"};
const unsigned char line3[16]={"��ˮѹ����"};
const unsigned char line41[8]={"ˮѹ"};
const unsigned char line42[8]={"����"};
unsigned char displayCache[9];

void LCD_Init_Show();
void LCD_Show_Update();
void LCD_Twinkle_Update();
void opr_key(unsigned char key_num);

void scan_key() {
    static unsigned char key_state = KEY_STATE_RELEASE;   /*״̬��״̬��ʼ��������static����״̬*/
    static unsigned char key_code = NONE_KEY_CODE;
    static unsigned char key_num = NONE_KEY_NUM;
    unsigned char pressed = press_key(); /*press_keyΪ����Ƿ��а������µĺ���*/
    static unsigned char scan_time = 0;
    switch (key_state) {
        case KEY_STATE_RELEASE: {   /*��ԭʼ״̬Ϊ�ް�������RELEASE��ͬʱ�ּ�⵽�������£���״̬ת����WAITING*/
            if (pressed == 1) {
                key_state = KEY_STATE_WAITING;
            }
            break;
        }
        case KEY_STATE_WAITING: {   /*ԭʼ״̬ΪWAITING���԰������ж���ж�*/
            if (pressed) {
                scan_time++;
                if (scan_time >= KEY_WAIT) {   /*���������µ�ʱ�䳬��һ��ʱ�䣬����Ϊ�������£�������*/
                    key_state = KEY_STATE_PRESSED;
                    scan_time = 0;
                    key_code = read_key();  /*read_keyΪ�������ĺ���*/
                }
            } else {    /*�������ɿ�����ָ�����ʼ״̬*/
                scan_time = 0;
                key_state = KEY_STATE_RELEASE;
            }
            break;
        }
        case KEY_STATE_PRESSED: {   /*��������ȷ�ϰ��£���ȴ������ɿ��ٽ��в���*/
            if (pressed == 0) {
                key_num = translate_key(key_code);
                opr_key(key_num);  /*opr_keyΪ�����¼���Ӧ����*/
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
    P7SEL |= 0x03;                            // P7.0��P7.1�˿�ѡ���ⲿ��Ƶ����XT1
    UCSCTL6 &= ~XT1OFF;                       // ʹ���ⲿ����
    UCSCTL6 |= XCAP_3;                        // �����ڲ����ص���: 1100������3
    UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_0;                        // Set ACLK = XT1

    __bis_SR_register(SCG0);                  // Disable the FLL control loop �ر�FLL���ƻ�·
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx ����DCOx, MODx

    /*
     * ʱ��Ƶ�����ã����㹫ʽ��
     * Ĭ��n=1;
     * fDCOCLK = D*(N + 1)*(fFLLREFCLK/n)
     * fDCOCLKDIV = (N + 1)*(fFLLREFCLK/n)
     * */
    UCSCTL1 = DCORSEL_7;                     // Select DCO range 40MHz operation  ����DCO�񵴷�Χ
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
    UCSCTL5 |= DIVM__1 + DIVS__1;              //MCLK=20M��SMCLK=20M��ACLK=32768
}

void initTimerA0(void){
    TA0CCR0 = 164;                            // 32768: �����жϼ�������1s,ʱ��Ƶ��Ϊ32.768kHZ,32768 / 32768 = 1s
    TA0CCTL0 &= ~CCIE;                        // TA0CCR0����/�Ƚ��жϼĴ����ж�ʹ��

    TA0CCR1 = 164;                            // �����ж��������5ms
    TA0CCTL1 |= CCIE;                         // TA0CCR0����/�Ƚ��жϼĴ����ж�ʹ��

    TA0CTL = TASSEL_1 + MC_1 + TACLR + TAIE;         // TASSEL_1: ACLKʱ��Դ, MC_1:������ģʽ, TACLR: �����ʱ��
}


#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void){              // 1s����ж�
    _NOP();
}


#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1(void){             // 2ms����ж�
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
        /*��ˮϵͳ����/ֹͣ*/
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
        Value = Write_SIP(0x858b);           //AD��ֵ     Conversion Register
        ConfigRegister = Write_SIP(0x858b);  //���üĴ��� Config Register
        CS_H;
        _NOP(); //�ϵ�*/
    }
    return 0;
}
