#include <msp430.h> 
#include <ads1118.h>

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    ADS1118_GPIO();

    unsigned int Value;
    unsigned int ConfigRegister;

    while(1)
    {

            CS_L;
            Value = Write_SIP(0x858b);           //AD��ֵ     Conversion Register
            ConfigRegister = Write_SIP(0x858b);  //���üĴ��� Config Register
            CS_H;
            _NOP(); //�ϵ�
    }
	return 0;
}
