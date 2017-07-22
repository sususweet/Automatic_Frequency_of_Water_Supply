/*
 * PID.c
 *
 *  Created on: 2017��7��20��
 *      Author: w
 */

#include "msp430f5438a.h"
#include "settings.h"
#include "PID.h"
#include "math.h"

#define err_max 100
#define err_min 80
#define pid_time 2
#define kai 0.5    // 0<kai<1

pid PIDFreq;

extern unsigned int Set_Fc;
extern volatile float Capture_voltage;

void PID_init(){
    //printf("PID_init begin \n");
    // pid* pidfre;
    PIDFreq.err= 0;
    PIDFreq.err_last= 0;
    PIDFreq.freq = 0;

    PIDFreq.integral=(float)(0.0);
    //����ϵ�� ����ϵ�� ΢��ϵ��
    PIDFreq.Kp=(float)(0.2);
    PIDFreq.Ki=(float)(0.0);
    PIDFreq.Kd=(float)(0.0);
    //printf("PID_init end \n");
}

void PID_realize() {
    float index;        //index: �����ϵ��
    float deta;
    unsigned int Actual_Fc;
    /********  vadcת���Ƶ�� ********/
    Actual_Fc = Voltage_to_Fc(Capture_voltage);
    //Actual_Fc = (unsigned int) (Capture_voltage * 6849 + 3651);
    PIDFreq.err = Set_Fc - Actual_Fc;   //����err

    unsigned int vadc_max = Set_Fc;
    unsigned int vadc_min = vadc_max-vadc_max/10;
    unsigned int vadc_diff = vadc_max-vadc_min;

    /*if (fabsf(PIDFreq.err) >= vadc_max){           //����ֹ���  vadc_max��min��ֵ����趨��
        index = 0.0;
    } else if (fabsf(PIDFreq.err) <= vadc_min) {
        index = 1.0;
        PIDFreq.integral += PIDFreq.err;
    } else {
        index = (vadc_max - PIDFreq.err) / vadc_diff;
        PIDFreq.integral += PIDFreq.err;
    }*/

    /*if (fabsf(PIDFreq.err) >= err_max){           //�������ڵĸĽ�
        deta = 1;
    } else {
        deta = kai;
    }*/
    deta = 1;
    index = 0;

    PIDFreq.freq = Actual_Fc + (int) (deta * (PIDFreq.Kp * PIDFreq.err + pid_time * index * PIDFreq.Ki * PIDFreq.integral + PIDFreq.Kd * (PIDFreq.err - PIDFreq.err_last) / pid_time));

    PIDFreq.err_last = PIDFreq.err;
}
