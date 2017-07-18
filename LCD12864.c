// LCD: PSB Select Parallel transmit mode by 100Ωto VCC
#include"msp430f5438a.h"
#include"settings.h"
#include"LCD12864.h"

/*******************************************************************/
/*                                                                 */
/*检查LCD忙状态                                                    */
/*lcd_busy为1时，忙，等待。lcd-busy为0时,闲，可写指令与数据。      */
/*                                                                 */
/*******************************************************************/

unsigned char LCD_Busy()
{
    unsigned char Result;
    LCD_RS_L;
    LCD_RW_H;
    LCD_EN_H;
    Delay_us(50);
    LCD_DataIn;                                             // Data Select Input
    Result = P8IN&0x80;
    LCD_EN_L;
    return Result;
}
/*******************************************************************/
/*                                                                 */
/*写指令到LCD                                                  */
/*RS=L，RW=L，E=高脉冲，D0-D7=指令码。                             */
/*                                                                 */
/*******************************************************************/
void LCD_WriteCommand(unsigned char Cmd)
{
    while(LCD_Busy());                                      // Busy check
    LCD_RS_L;
    LCD_RW_L;
    LCD_EN_L;
    Delay_us(20);
    Delay_us(20);
    LCD_DataOut;
    P8OUT = Cmd;
    //P0 = Cmd;
    Delay_us(50);
    LCD_EN_H;
    Delay_us(50);
    LCD_EN_L;
}
/*******************************************************************/
/*                                                                 */
/*写显示数据到LCD                                                  */
/*RS=H，RW=L，E=高脉冲，D0-D7=数据。                               */
/*                                                                 */
/*******************************************************************/
void LCD_WriteData(unsigned char Dat)
{
    while(LCD_Busy());                                      // Busy check
    LCD_RS_H;
    Delay_us(20);
    LCD_RW_L;
    Delay_us(20);
    LCD_EN_L;
    Delay_us(20);
    LCD_DataOut;
    P8OUT = Dat;
    //P0 = Dat;
    Delay_us(50);
    LCD_EN_H;
    Delay_us(50);
    LCD_EN_L;
}
/*******************************************************************/
/*                                                                 */
/*  LCD初始化设定                                                  */
/*                                                                 */
/*******************************************************************/
void LCD_Init()
{
    LCD_RST_L;		                                    //液晶复位
    Delay_ms(3);
    LCD_RST_H;
    Delay_ms(3);

    LCD_WriteCommand(0x34);                                 //扩充指令操作
    Delay_ms(5);
    LCD_WriteCommand(0x30);                                 //基本指令操作
    Delay_ms(5);
    LCD_WriteCommand(0x0C);                                 //显示开，关光标
    Delay_ms(5);
    LCD_WriteCommand(0x01);                                 //清除LCD的显示内容
    Delay_ms(5);
}
//*********************************************************/
//*                                                       */
//* 设定汉字显示位置
//eg:LCD_Position(4,0); ----表示从第四行的第0个字符开始写*/
//--------------------------即从第四行的第0个字开始写
//eg:LCD_Position(4,1); ----表示从第四行的第2个字符开始写*/
//--------------------------即从第四行的第1个字开始写
/*                                                       */
/*********************************************************/
void LCD_Position(unsigned char X,unsigned char Y)
{
    unsigned char  Pos;
    if (X==1)      X=0x80;

    else if (X==2) X=0x90;

    else if (X==3) X=0x88;

    else if (X==4) X=0x98;

    Pos = X+Y ;

    LCD_WriteCommand(Pos);                                   //显示地址
}
/*********************************************************
*                                                        *
* 闪烁函数                                               *
*                                                        *
*********************************************************/
void LCD_Flash()
{
    LCD_WriteCommand(0x08);                                  // close display
    Delay_ms(400);
    LCD_WriteCommand(0x0c);                                  // open display
    Delay_ms(400);
    LCD_WriteCommand(0x08);
    Delay_ms(400);
    LCD_WriteCommand(0x0c);
    Delay_ms(400);
    LCD_WriteCommand(0x08);
    Delay_ms(200);
    LCD_WriteCommand(0x0c);
    Delay_ms(5);
    LCD_WriteCommand(0x01);                                  // clear display
    Delay_ms(5);
}
/**********************************************************
; 显示字符表代码
**********************************************************/
void  LCD_Char_Display()
{
    unsigned char  s;
    LCD_Clear_Screen();                                      //清屏
    LCD_WriteCommand(0x80);                                  //设置显示位置为第一行
    for(s=0;s<16;s++)
    {
        LCD_WriteData(0x30+s);
    }
    LCD_WriteCommand(0x90);                                  //设置显示位置为第二行
    for(s=0;s<16;s++)
    {
        LCD_WriteData(0x40+s);
    }
    LCD_WriteCommand(0x88);                                  //设置显示位置为第三行
    for(s=0;s<16;s++)
    {
        LCD_WriteData(0x50+s);
    }
    LCD_WriteCommand(0x98);                                  //设置显示位置为第四行
    for(s=0;s<16;s++)
    {
        LCD_WriteData(0x60+s);
    }
}
/*********************************************************
*                                                        *
* 图形显示                                               *
*                                                        *
*********************************************************/
void Photo_Display(const unsigned char *Bmp)
{
    unsigned char i,j;

    LCD_WriteCommand(0x34);                                   //写数据时,关闭图形显示

    for(i=0;i<32;i++)
    {
        LCD_WriteCommand(0x80+i);                               //先写入水平坐标值
        LCD_WriteCommand(0x80);                                 //写入垂直坐标值
        for(j=0;j<16;j++)                                       //再写入两个8位元的数据
            LCD_WriteData(*Bmp++);
        Delay_ms(1);
    }

    for(i=0;i<32;i++)
    {
        LCD_WriteCommand(0x80+i);
        LCD_WriteCommand(0x88);
        for(j=0;j<16;j++)
            LCD_WriteData(*Bmp++);
        Delay_ms(1);
    }
    LCD_WriteCommand(0x36);                                   //写完数据,开图形显示
}
/*********************************************************
*                                                        *
* 清屏函数                                               *
*                                                        *
*********************************************************/
void LCD_Clear_Screen()
{
    LCD_WriteCommand(0x34);                                  //扩充指令操作
    Delay_ms(5);
    LCD_WriteCommand(0x30);                                  //基本指令操作
    Delay_ms(5);
    LCD_WriteCommand(0x01);                                  //清屏
    Delay_ms(5);
}

void LCD_GPIO_Init()
{
    // Liquid 12864 with word lib pin define
    P3DIR |= BIT0+BIT4+BIT5;                                // RS,RW,EN Select output
    P3OUT |= BIT0+BIT4+BIT5;
    P11DIR |= BIT0;                                         // RST Select output

    P11OUT |= BIT0;
    P8DIR |= 0xFF;                                          // LCD_Data,Select output

}
