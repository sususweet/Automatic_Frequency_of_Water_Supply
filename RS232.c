//��������RS232�ӿ���PC���ӣ�PC�����д��ڴ�ʦ�ȴ��ڵ������.
//��RS232����,���µ�0�м�������һ������.
//����Ҫ�رպ��ٴ�һ��
//Ҳ����PC�˷��͵����ַ���������,�����彫�������ݷ���PC.
//��Ҫ�����ַ���Ϊ����,��ʱ��ᷢ����,����˵Ҫ��У��λ.

#include "msp430x54x.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "PIN_DEF.H"
#include "settings.h"

//#define  FLL_FACTOR     549                                          // FLL_FACTOR: DCO��Ƶϵ��


/*��ʼ����ʱ��: MCLK = XT1��(FLL_FACTOR+1)
void Init_CLK(void)
{
  WDTCTL     = WDTPW + WDTHOLD                            ; // �ؿ��Ź�
  P7SEL     |= 0x03                                       ; // �˿�ѡ���ⲿ��Ƶ����XT1
  UCSCTL6   &=~XT1OFF                                     ; // ʹ���ⲿ����
  UCSCTL6   |= XCAP_3                                     ; // �����ڲ����ص���
  UCSCTL3   |= SELREF_2                                   ; // DCOref = REFO
  UCSCTL4   |= SELA_0                                     ; // ACLK   = XT1
  __bis_SR_register(SCG0)                                 ; // �ر�FLL���ƻ�·
  UCSCTL0    = 0x0000                                     ; // ����DCOx, MODx
  UCSCTL1    = DCORSEL_7                                  ; // ����DCO�񵴷�Χ
  UCSCTL2    = FLLD__1 + FLL_FACTOR                       ; // Fdco = ( FLL_FACTOR + 1)��FLLRef = (649 + 1) * 32768 = 21.2992MHz
  __bic_SR_register(SCG0)                                 ; // ��FLL���ƻ�·
  __delay_cycles(1024000)                                 ;
  do
  {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG); // ��� XT2,XT1,DCO �����־
    SFRIFG1 &= ~OFIFG                                     ;
  }while(SFRIFG1&OFIFG)                                   ; // ������������־
}*/

/*  Init_Port(void): ����IO�˿�
void Init_Port(void)
{
  P5DIR  |= POWER                                                  ; // ����Դ
  MAIN_POWER_ON                                                    ;
  P7DIR  |= LED_PWR                                                ; // ��������ܵ�Դ
  P7OUT  &=~LED_PWR                                                ;
  INTERNAL_PULL_UP                                                 ; // ʹ�ܼ��̶˿��ڲ��������� 
  ROW_IN_COL_OUT                                                   ; // ���������룬�����0
}
*/


//  Init_RSUART(void): ��ʼ��RS232/485�˿� 
void Init_RSUART(void) {
    RS_PORT_SEL |= TXD + RXD;         // ѡ�����Ź���(ѡ��P10.4/5ΪƬ�����蹦�ܣ�TXD/RXD)
    RS_PORT_DIR |= TXD;              // ѡ�����Ź���(P10.4�������)
    //DIR Ҫ��Ҫ��RXD�� Ĭ��Ϊ0��

    UCA3CTL1 = UCSWRST;         // ״̬����λ(after PUC���ϵ������Ĭ��Ϊ1,�˾䲻д���,���������״̬������д)
    //reset USCI by setting UCSWRST ��UCSWRSTΪ1ʱ���üĴ���
    UCA3CTL1 |= UCSSEL_1;         // �����ʷ���ʱ��ѡΪACLK
    /******���ò����� 16λ******/
    UCA3BR0 = 0x03;             // 32768/9600=3.41 (3.41ʲô��˼?F149���ֲ᣺��13��_16)
    UCA3BR1 = 0x00;
    // BRx��ʮ��λ�Ĵ��� BR0�ǵ�λ �������� 3
    UCA3MCTL = UCBRS_3 + UCBRF_0; // UCBRSx=3, UCBRFx=0
    //UCBRF�����ڸ�Ƶģʽ ��Ϊ0�Ϳ���
    //UCBRS��С����Ƶ��  ���ڽ��ܹ���������ȡ���ı������������Ҫ3���������� ���Է�Ƶϵ�����ٴ���3
    //UCBRS +1 ��Ƶϵ��+1
    //ǰ�������3.41 С��Ϊ0.41 0.41*8=3.28 ��ӽ���������3 ����BRS_3
    UCA3CTL1 &= ~UCSWRST;         // ����״̬��
    // 0 reset released for operation 0ʱUSCI���ܽ��й���
    // 1 USCI logic held in reset state

    UCA3IE |= UCRXIE;            // ��������ж�
    RS485_IN;
}

// RS232�˿ڷ��ͳ���
void RS232TX_SEND(char type, unsigned char *tx_buf) {
    unsigned char i, length;
    length = strlen(tx_buf);
    UCA3TXBUF = type;              //������������
    while (!(UCA3IFG & UCTXIFG));  //����Ƿ���
    //��������
    for (i = 0; i < length; i++) {
        UCA3TXBUF = *(tx_buf + i);
        while (!(UCA3IFG & UCTXIFG));  //ÿ����һ��,�ͼ��һ���Ƿ��ꡣ
    }
}
