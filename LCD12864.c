// LCD: PSB Select Parallel transmit mode by 100��to VCC
#include "msp430f5438a.h"
#include "settings.h"
#include "LCD12864.h"

/*******************************************************************/
/*                                                                 */
/*���LCDæ״̬                                                    */
/*lcd_busyΪ1ʱ��æ���ȴ���lcd-busyΪ0ʱ,�У���дָ�������ݡ�      */
/*                                                                 */
/*******************************************************************/

unsigned char LCD_Busy() {
    unsigned char Result;
    LCD_RS_L;
    LCD_RW_H;
    LCD_EN_H;
    Delay_us(50);
    LCD_DataIn;                                             // Data Select Input
    Result = P8IN & 0x80;
    LCD_EN_L;
    return Result;
}
/*******************************************************************/
/*                                                                 */
/*дָ�LCD                                                  */
/*RS=L��RW=L��E=�����壬D0-D7=ָ���롣                             */
/*                                                                 */
/*******************************************************************/
void LCD_WriteCommand(unsigned char Cmd) {
    while (LCD_Busy());                                      // Busy check
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
/*д��ʾ���ݵ�LCD                                                  */
/*RS=H��RW=L��E=�����壬D0-D7=���ݡ�                               */
/*                                                                 */
/*******************************************************************/
void LCD_WriteData(unsigned char Dat) {
    while (LCD_Busy());                                      // Busy check
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
/*  LCD��ʼ���趨                                                  */
/*                                                                 */
/*******************************************************************/
void LCD_Init() {
    LCD_RST_L;                                            //Һ����λ
    Delay_ms(3);
    LCD_RST_H;
    Delay_ms(3);

    LCD_WriteCommand(0x34);                                 //����ָ�����
    Delay_ms(5);
    LCD_WriteCommand(0x30);                                 //����ָ�����
    Delay_ms(5);
    LCD_WriteCommand(0x0C);                                 //��ʾ�����ع��
    Delay_ms(5);
    LCD_WriteCommand(0x01);                                 //���LCD����ʾ����
    Delay_ms(5);
}
//*********************************************************/
//*                                                       */
//* �趨������ʾλ��
//eg:LCD_Position(4,0); ----��ʾ�ӵ����еĵ�0���ַ���ʼд*/
//--------------------------���ӵ����еĵ�0���ֿ�ʼд
//eg:LCD_Position(4,1); ----��ʾ�ӵ����еĵ�2���ַ���ʼд*/
//--------------------------���ӵ����еĵ�1���ֿ�ʼд
/*                                                       */
/*********************************************************/
void LCD_Position(unsigned char X, unsigned char Y) {
    unsigned char Pos;
    if (X == 1) X = 0x80;

    else if (X == 2) X = 0x90;

    else if (X == 3) X = 0x88;

    else if (X == 4) X = 0x98;

    Pos = X + Y;

    LCD_WriteCommand(Pos);                                   //��ʾ��ַ
}

/*********************************************************
*                                                        *
* ��˸����                                               *
*                                                        *
*********************************************************/
void LCD_Flash() {
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
; ��ʾ�ַ������
**********************************************************/
void LCD_Char_Display() {
    unsigned char s;
    LCD_Clear_Screen();                                      //����
    LCD_WriteCommand(0x80);                                  //������ʾλ��Ϊ��һ��
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x30 + s);
    }
    LCD_WriteCommand(0x90);                                  //������ʾλ��Ϊ�ڶ���
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x40 + s);
    }
    LCD_WriteCommand(0x88);                                  //������ʾλ��Ϊ������
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x50 + s);
    }
    LCD_WriteCommand(0x98);                                  //������ʾλ��Ϊ������
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x60 + s);
    }
}

/*********************************************************
*                                                        *
* ͼ����ʾ                                               *
*                                                        *
*********************************************************/
void Photo_Display(const unsigned char *Bmp) {
    unsigned char i, j;

    LCD_WriteCommand(0x34);                                   //д����ʱ,�ر�ͼ����ʾ

    for (i = 0; i < 32; i++) {
        LCD_WriteCommand(0x80 + i);                               //��д��ˮƽ����ֵ
        LCD_WriteCommand(0x80);                                 //д�봹ֱ����ֵ
        for (j = 0; j < 16; j++)                                       //��д������8λԪ������
            LCD_WriteData(*Bmp++);
        Delay_ms(1);
    }

    for (i = 0; i < 32; i++) {
        LCD_WriteCommand(0x80 + i);
        LCD_WriteCommand(0x88);
        for (j = 0; j < 16; j++)
            LCD_WriteData(*Bmp++);
        Delay_ms(1);
    }
    LCD_WriteCommand(0x36);                                   //д������,��ͼ����ʾ
}

/*********************************************************
*                                                        *
* ��������                                               *
*                                                        *
*********************************************************/
void LCD_Clear_Screen() {
    LCD_WriteCommand(0x34);                                  //����ָ�����
    Delay_ms(5);
    LCD_WriteCommand(0x30);                                  //����ָ�����
    Delay_ms(5);
    LCD_WriteCommand(0x01);                                  //����
    Delay_ms(5);
}

void LCD_GPIO_Init() {
    // Liquid 12864 with word lib pin define
    LCD_RS_RW_EN_DIR_OUT;                                   // RS,RW,EN Select output
    LCD_RS_H;
    LCD_RW_H;
    LCD_EN_H;

    LCD_RST_DIR_OUT;                                        // RST Select output
    LCD_RST_H;

    LCD_DataOut;                                          // LCD_Data,Select output
}


void LCD_Show(unsigned char x, unsigned char y,unsigned char *str) {
    LCD_Position (x,y);
    while (*str != '\0')  //����д���ַ������ݣ�ֱ����⵽������
    {
        LCD_WriteData(*str++);
    }
}
