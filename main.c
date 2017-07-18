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
            Value = Write_SIP(0x858b);           //ADÊýÖµ     Conversion Register
            ConfigRegister = Write_SIP(0x858b);  //ÅäÖÃ¼Ä´æÆ÷ Config Register
            CS_H;
            _NOP(); //¶Ïµã
    }
	return 0;
}
