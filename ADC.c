/*
 * ADC.c
 *
 *  Created on: 2017Äê7ÔÂ18ÈÕ
 *      Author: w
 */

    //                   MSP430F5438A
    //                 -----------------
    //             /|\|                 |
    //              | |                 |
    //              --|RST              |
    //                |                 |
    //                |             P3.7|-> Data Out (UCB1SIMO)
    //                |                 |
    //                |             P5.4|<- Data In (UCB1SOMI)
    //                |                 |
    //          \CS <-|P1.1         P5.5|-> Serial Clock Out (UCB1CLK)
    //
    //******************************************************************************

#include "msp430f5438a.h"
#include "ADC.h"
#include "settings.h"

void ADS1118_GPIO_Init(void)
{
   /*
    P1OUT |= 0x02;                          // Set P1.1 for CS
    P1DIR |= 0x02;                          // Set P1.1 to output direction
    P3SEL |= 0x80;                          // P3.7 option select
    P5SEL |= 0x30;                          // P5.4,5 option select

    P5DIR |= 0x01;                          // Set P5.0 to output direction
    */
    ADC_CS_H;                      // Set P1.1 for CS
    ADC_CS;                        // Set P1.1 to output direction
    ADC_DOUT;                       // P3.7 option select
    ADC_DIN;                        // P5.4,5 option select
    ADC_SCLK;
}

void ADS1118_SPI_Init(void)
{
    UCB1CTL1 |= UCSWRST;                    // **Put state machine in reset**
    UCB1CTL0 |= UCMST+UCSYNC+UCMSB;         // 3-pin, 8-bit SPI master
                                            // Clock polarity high, MSB
    UCB1CTL1 |= UCSSEL_2;                   // SMCLK
    UCB1BR0 = 0x05;                         // /2
    UCB1BR1 = 0;                            //
    UCB1CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**

    __delay_cycles(100);                    // Wait for slave to initialize
}

void ADS1118_ADS_Config(signed int temp_config_value)
{
    signed int Config_Value;

    Config_Value = temp_config_value;


    ADC_CS_L;                                 // Set CS low
    __delay_cycles(100);                                    // Wait for slave to initialize

    ADS1118_WriteSPI(Config_Value,0);                   // Write configuration to ADS1118

    __delay_cycles(100);                                    // Wait for slave to initialize

    ADC_CS_H;                      // Set CS high
}

int ADS1118_ADS_Read(void)
{
    unsigned int Data, Config_Value;

    Config_Value = 0;


    ADC_CS_L;                     // Set CS low
    Data = ADS1118_WriteSPI(Config_Value,1);                    // Read data from ADS1118
    ADC_CS_H;                      // Set CS high

    return Data;
}

/*
 * Mode 0: Only write config register to ADS1118
 * Mode 1: Write config register to ADS1118 as well as read data from ADS1118
 */
signed int ADS1118_WriteSPI(unsigned int config, unsigned char mode)
{
    signed int msb;
    unsigned int temp;
    signed int dummy;

    temp = config;
    if (mode==1) temp = 0;
    while(!(UCB1IFG&UCTXIFG));
    UCB1TXBUF = (temp >> 8 );                   // Write MSB of Config
    while(!(UCB1IFG&UCRXIFG));
    msb = UCB1RXBUF;                        // Read MSB of Data

    while(!(UCB1IFG&UCTXIFG));
    UCB1TXBUF = (temp & 0xff);                  // Write LSB of Config
    while(!(UCB1IFG&UCRXIFG));
    msb = (msb << 8) | UCB1RXBUF;               // Read LSB of Data

    while(!(UCB1IFG&UCTXIFG));
    UCB1TXBUF = (temp >> 8 );                   // Write MSB of Config
    while(!(UCB1IFG&UCRXIFG));
    dummy = UCB1RXBUF;                      // Read MSB of Config


    while(!(UCB1IFG&UCTXIFG));
    UCB1TXBUF= (temp & 0xff);                   // Write LSB of Config
    while(!(UCB1IFG&UCRXIFG));
    dummy = (dummy <<8) | UCB1RXBUF;                    // Read LSB of Config

   // __delay_cycles(100);

    return msb;
}

/*
 * do ADC once and return a float as voltage
 */
float ADC(void)
{
    signed int ADC_Result;
    float Voltage_ch1;
    ADS1118_ADS_Config(0xC3E3);                          //Only Select AIN1,860SPS,+-4.096V scan voltage range;

    ADC_Result = ADS1118_ADS_Read();                      // Read data from ch1,the last time result
    if (ADC_Result >= 0x8000)
    {
        ADC_Result = 0xFFFF - ADC_Result;
        Voltage_ch1 = (float) (ADC_Result * (-1.0) / 32768 * 4.096);
    }
    else
        Voltage_ch1 = (float) (ADC_Result * 1.0 / 32768 * 4.096);

    return Voltage_ch1;
}
