/*
 * PID.c
 *
 *  Created on: 2017年7月20日
 *      Author: w
 */

#include <msp430f5438a.h>
#include <settings.h>
#include "PID.h"

#define Max_Fre 250
#define Min_Fre 10
#define err_max 100
#define err_min 80
#define err_diff err_max-err_min
#define pid_time 2
#define kai 0.5    // 0<kai<1
pid PIDFreq;

void PID_init(float set,float actual){
    //printf("PID_init begin \n");
    // pid* pidfre;
    PIDFreq.Setfreq=set;
    PIDFreq.Actualfreq=actual;
    PIDFreq.err=abs(set-actual);
    PIDFreq.err_last=0;
    PIDFreq.freq=(float)(0.0);
    PIDFreq.integral=(float)(0.0);
    PIDFreq.Kp=(float)(0.4);
    PIDFreq.Ki=(float)(0.2);
    PIDFreq.Kd=(float)(0.2);
    //printf("PID_init end \n");
}

float PID_realize() {
    float index;
    float deta;
    /********  vadc转变成频率 ********/
   // PIDFreq.Setfreq = vadc;
    int vadc_max=PIDFreq.Setfreq;
    int vadc_min=vadc_max-vadc_max/10;
    int vadc_diff=vadc_max-vadc_min;

    if (abs(PIDFreq.err) >= vadc_max)           //变积分过程  vadc_max和min的值如何设定？
    {
        index = 0.0;
    } else if (abs(PIDFreq.err) <= vadc_min) {
        index = 1.0;
        PIDFreq.integral += PIDFreq.err;
    } else {
        index = (float)((vadc_max - PIDFreq.err) / vadc_diff);
        PIDFreq.integral += PIDFreq.err;
    }

    if (abs(PIDFreq.err) >= err_max)           //比例环节的改进
    {
        deta = 1;
    } else {
        deta = kai;
    }

    PIDFreq.freq = deta * (PIDFreq.Kp * PIDFreq.err + pid_time*index * PIDFreq.Ki * PIDFreq.integral + PIDFreq.Kd * (PIDFreq.err - PIDFreq.err_last)/pid_time);

    PIDFreq.err_last = PIDFreq.err;

    //频率范围的限制
    if( PIDFreq.freq > Max_Fre ){
        PIDFreq.Actualfreq =Max_Fre;
    }
    else if (PIDFreq.freq < Min_Fre){
        PIDFreq.Actualfreq =Min_Fre;
    }
    else {
        PIDFreq.Actualfreq = PIDFreq.freq;
    }

    PIDFreq.err = PIDFreq.Setfreq - PIDFreq.Actualfreq;   //更新err
/*
    PIDFreq.Actualfreq = PIDFreq.freq;
*/
    //return PIDFreq.Actualfreq;
}
