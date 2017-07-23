/*
 * main.c
 *
 *  Created on: 2017年7月23日
 *      Author: w
 */

#include "msp430x54x.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"
#include "Clock.h"
#include "RS232.h"

char REC_BUF[10];       //接收缓存区
char datatype[5]={'P','F','W','S','M'};  //水压 流量 供水压力 待机压力 电机状态

void main( void )
{
  WDTCTL = WDTPW + WDTHOLD ;
  unsigned char datatest[9]={'1','2','3','4','5','6','7','8','9'};
  initClock();
  Init_RSUART();

  __bis_SR_register(GIE);        //开全局中断允许
  //test send
  RS232TX_SEND(datatype[0],datatest);
}

//RS232/485接收中断服务程序
//_nop()只是测试用来看是不是进了对应的位置用的 可以删掉
#pragma vector=USCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void)
{
  static unsigned int datacount=0;
  //char fake;
  if(datacount==0) {
    REC_BUF[datacount] = UCA3RXBUF;   //访问接收缓冲寄存器
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
    /*   假读\0 不存  现在没这个情况了
    if(datacount==1){

        REC_BUF[1]=REC_BUF[2];
    }
    */
    datacount++;
  }
  if(datacount==10){
    datacount=0;
    switch (REC_BUF[0]){
      //todo 赋值给对应的数组并设立flag等参数
      case('P'):_nop();break;
      case('F'):_nop();break;
      case('W'):_nop();break;
      case('S'):_nop();break;
      case('M'):_nop();break;
      default:break;
    }
  }
}
