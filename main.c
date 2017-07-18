#include "msp430f5438a.h"
#include "intrinsics.h"
#include "in430.h"
#include "settings.h"
#include "ads1118.h"

/**
 * main.c
 * Default: MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
 *
 */
void initSystem(void){
    /* Initialize System Clock.
     * Modified: ACLK = REFO = 32kHz, MCLK = SMCLK = 8MHz
     */
    UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_5;                     // Select DCO range 16MHz operation
    UCSCTL2 = FLLD_1 + 249;                   // Set DCO Multiplier for 8MHz
    // (N + 1) * FLLRef = Fdco
    // (249 + 1) * 32768 = 8MHz
    // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
    __delay_cycles(250000);

    // Loop until XT1,XT2 & DCO fault flag is cleared
    do{
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
        // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }
    while (SFRIFG1 & OFIFG);                   // Test oscillator fault flag
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    initSystem();

    ADS1118_GPIO();

    unsigned int Value;
    unsigned int ConfigRegister;

    while(1) {
        CS_L;
        Value = Write_SIP(0x858b);           //AD数值     Conversion Register
        ConfigRegister = Write_SIP(0x858b);  //配置寄存器 Config Register
        CS_H;
        _NOP(); //断点
    }

    return 0;
}
