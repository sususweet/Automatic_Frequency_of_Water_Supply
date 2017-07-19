#include "msp430f5438a.h"
#include "intrinsics.h"
#include "in430.h"
#include "settings.h"
#include "LCD12864.h"
#include "Keyboard.h"
#include "ADC.h"

const unsigned char line1[16]={"��ѹ��ˮϵͳ"};

/**
 * main.c
 * Default: MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
 */

#define KEY_WAIT 4    /*����ɨ���ӳ�����*/
#define NONE_KEY_CODE 0xFF
#define NONE_KEY_NUM 0

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
    TA0CCR0 = 164;                        // 32768: �����жϼ�������1s,ʱ��Ƶ��Ϊ32.768kHZ,32768 / 32768 = 1s
    TA0CCTL0 &= ~CCIE;                        // TA0CCR0����/�Ƚ��жϼĴ����ж�ʹ��

    TA0CCR1 = 164;                             // �����ж��������5ms
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
        Value = Write_SIP(0x858b);           //AD��ֵ     Conversion Register
        ConfigRegister = Write_SIP(0x858b);  //���üĴ��� Config Register
        CS_H;
        _NOP(); //�ϵ�*/
    }
    return 0;
}
