
#include <msp430f5438a.h>
#include <settings.h>
#include "ADC.h"
/**
 * main.c
 */
int main(void)
{
    volatile float Voltage;

    ADS1118_GPIO_Init();           //initialize the GPIO
    ADS1118_SPI_Init();

    Voltage=ADC();

    return 0;
}
