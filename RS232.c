//将开发板RS232接口与PC连接，PC上运行串口大师等串口调试软件.
//打开RS232串口,按下第0行键盘则发送一串数据.
//串口要关闭后再打开一次
//也可由PC端发送单个字符至开发板,开发板将接收数据发回PC.
//将要发的字符改为中文,有时候会发乱码,网友说要加校验位.

#include "msp430x54x.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "PIN_DEF.H"
#include "settings.h"

//#define  FLL_FACTOR     549                                          // FLL_FACTOR: DCO倍频系数


/*初始化主时钟: MCLK = XT1×(FLL_FACTOR+1)
void Init_CLK(void)
{
  WDTCTL     = WDTPW + WDTHOLD                            ; // 关看门狗
  P7SEL     |= 0x03                                       ; // 端口选择外部低频晶振XT1
  UCSCTL6   &=~XT1OFF                                     ; // 使能外部晶振
  UCSCTL6   |= XCAP_3                                     ; // 设置内部负载电容
  UCSCTL3   |= SELREF_2                                   ; // DCOref = REFO
  UCSCTL4   |= SELA_0                                     ; // ACLK   = XT1
  __bis_SR_register(SCG0)                                 ; // 关闭FLL控制回路
  UCSCTL0    = 0x0000                                     ; // 设置DCOx, MODx
  UCSCTL1    = DCORSEL_7                                  ; // 设置DCO振荡范围
  UCSCTL2    = FLLD__1 + FLL_FACTOR                       ; // Fdco = ( FLL_FACTOR + 1)×FLLRef = (649 + 1) * 32768 = 21.2992MHz
  __bic_SR_register(SCG0)                                 ; // 打开FLL控制回路
  __delay_cycles(1024000)                                 ;
  do
  {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG); // 清除 XT2,XT1,DCO 错误标志
    SFRIFG1 &= ~OFIFG                                     ;
  }while(SFRIFG1&OFIFG)                                   ; // 检测振荡器错误标志
}*/

/*  Init_Port(void): 设置IO端口
void Init_Port(void)
{
  P5DIR  |= POWER                                                  ; // 主电源
  MAIN_POWER_ON                                                    ;
  P7DIR  |= LED_PWR                                                ; // 发光二极管电源
  P7OUT  &=~LED_PWR                                                ;
  INTERNAL_PULL_UP                                                 ; // 使能键盘端口内部上拉电阻 
  ROW_IN_COL_OUT                                                   ; // 设置行输入，列输出0
}
*/


//  Init_RSUART(void): 初始化RS232/485端口 
void Init_RSUART(void) {
    RS_PORT_SEL |= TXD + RXD;         // 选择引脚功能(选择P10.4/5为片内外设功能：TXD/RXD)
    RS_PORT_DIR |= TXD;              // 选择引脚功能(P10.4向外输出)
    //DIR 要不要加RXD？ 默认为0？

    UCA3CTL1 = UCSWRST;         // 状态机复位(after PUC（上电清除）默认为1,此句不写亦可,但后句启动状态机必须写)
    //reset USCI by setting UCSWRST 在UCSWRST为1时配置寄存器
    UCA3CTL1 |= UCSSEL_1;         // 波特率发生时钟选为ACLK
    /******设置波特率 16位******/
    UCA3BR0 = 0x03;             // 32768/9600=3.41 (3.41什么意思?F149的手册：第13章_16)
    UCA3BR1 = 0x00;
    // BRx是十六位寄存器 BR0是低位 配置整数 3
    UCA3MCTL = UCBRS_3 + UCBRF_0; // UCBRSx=3, UCBRFx=0
    //UCBRF是用在高频模式 设为0就可以
    //UCBRS是小数分频器  串口接受过程中有三取二的表决过程至少需要3个机器周期 所以分频系数至少大于3
    //UCBRS +1 分频系数+1
    //前面算出的3.41 小数为0.41 0.41*8=3.28 最接近的整数是3 所以BRS_3
    UCA3CTL1 &= ~UCSWRST;         // 启动状态机
    // 0 reset released for operation 0时USCI才能进行工作
    // 1 USCI logic held in reset state

    UCA3IE |= UCRXIE;            // 允许接收中断
    RS485_IN;
}

// RS232端口发送程序
void RS232TX_SEND(char type, unsigned char *tx_buf) {
    unsigned char i, length;
    length = strlen(tx_buf);
    UCA3TXBUF = type;              //发送数据类型
    while (!(UCA3IFG & UCTXIFG));  //检查是否发完
    //发送数据
    for (i = 0; i < length; i++) {
        UCA3TXBUF = *(tx_buf + i);
        while (!(UCA3IFG & UCTXIFG));  //每发送一个,就检查一次是否发完。
    }
}
