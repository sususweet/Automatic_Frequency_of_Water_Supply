#include <msp430f5438a.h>
#include <settings.h>
#include <PID.h>

/**
 * main.c
 */
#define dot 50
#define err_range 0.5

extern pid PIDFreq;

int main(void){
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	int count=0;
	PID_init(200,180);
	while(abs(PIDFreq.err)>0.5)
	{
        PID_realize();
        //printf("%f\n",speed);
        count++;
	}
	return 0;
}
