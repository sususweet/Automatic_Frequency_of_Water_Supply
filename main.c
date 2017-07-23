/*
 * main.c
 *
 *  Created on: 2017��7��23��
 *      Author: w
 */

#include "msp430x54x.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"
#include "Clock.h"
#include "RS232.h"

char REC_BUF[10];       //���ջ�����
char datatype[5]={'P','F','W','S','M'};  //ˮѹ ���� ��ˮѹ�� ����ѹ�� ���״̬

void main( void )
{
  WDTCTL = WDTPW + WDTHOLD ;
  unsigned char datatest[9]={'1','2','3','4','5','6','7','8','9'};
  initClock();
  Init_RSUART();

  __bis_SR_register(GIE);        //��ȫ���ж�����
  //test send
  RS232TX_SEND(datatype[0],datatest);
}

//RS232/485�����жϷ������
//_nop()ֻ�ǲ����������ǲ��ǽ��˶�Ӧ��λ���õ� ����ɾ��
#pragma vector=USCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void)
{
  static unsigned int datacount=0;
  //char fake;
  if(datacount==0) {
    REC_BUF[datacount] = UCA3RXBUF;   //���ʽ��ջ���Ĵ���
    datacount++;
    switch(REC_BUF[0]){         //judge data type
      case('P'):_nop();break;
      case('F'):_nop();break;
      case('W'):_nop();break;
      case('S'):_nop();break;
      case('M'):_nop();break;
      default:
        datacount=0;
        break;
    }
  } else if(datacount<10){
    REC_BUF[datacount]=UCA3RXBUF;
    /*   �ٶ�\0 ����  ����û��������
    if(datacount==1){

        REC_BUF[1]=REC_BUF[2];
    }
    */
    datacount++;
  }
  if(datacount==10){
    datacount=0;
    switch (REC_BUF[0]){
      //todo ��ֵ����Ӧ�����鲢����flag�Ȳ���
      case('P'):_nop();break;
      case('F'):_nop();break;
      case('W'):_nop();break;
      case('S'):_nop();break;
      case('M'):_nop();break;
      default:break;
    }
  }
}
