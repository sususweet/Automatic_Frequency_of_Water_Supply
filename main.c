#include "msp430f5438a.h"
#include "settings.h"
#include "frequency_capture.h"

static int start=0;
static int pulseend=0;
static int periodend=0;


/**
 * main.c
 * Default: MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
 */


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


#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer_A1_Cap(void)
{
  static unsigned char cap_flag=0;

  switch(TA1IV)   //������ѯ
  { case 4:      //�����ж�
           if(TA1CCTL2&CM0) //����������
           {
               TA1CCTL2=(TA1CCTL2&(~CM0))|CM1; //��������Ϊ�½��ش���
               if(cap_flag==0){
                   start=TA0R; //��¼��ʼʱ��
                   cap_flag=1; //��ʼһ�����ڵĲ���
               }
               else
                   periodend=TA0R;
                   cap_flag=0;
                   TA1CCTL2 &= ~BIT4; //���ж�ʹ��
           }
           else if (TA1CCTL2&CM1) //==�����½���==
           {
               TA1CCTL2=(TA1CCTL2&(~CM1))|CM0; //��������Ϊ�����ش���
               if(cap_flag==1){
                   pulseend=TA0R; //��¼��ʼʱ��
               }
           }
        break;

    default:
       break;
  }

 }

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    volatile float Voltage;
    initClock();

    Capture_init();


    __bis_SR_register(GIE);        //��ȫ���ж�����

    while (1) {

        process_fre(start,periodend);

        /*CS_L;
        Value = Write_SIP(0x858b);           //AD��ֵ     Conversion Register
        ConfigRegister = Write_SIP(0x858b);  //���üĴ��� Config Register
        CS_H;
        _NOP(); //�ϵ�*/
    }
}
