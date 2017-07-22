/*
 * PID.c
 *
 *  Created on: 2017年7月20日
 *      Author: w
 */

#include "msp430f5438a.h"
#include "settings.h"
#include "PID.h"
#include "math.h"

#define err_max 804
#define err_min 80
#define pid_time 2
#define kai 0.2    // 0<kai<1

pid PIDFreq;

extern float Set_Pressure;
extern volatile float Capture_voltage;

void PID_init(){
    //printf("PID_init begin \n");
    // pid* pidfre;
    PIDFreq.err= 0;
    PIDFreq.err_last= 0;
    PIDFreq.output = 0;

    PIDFreq.integral=(float)(0.0);
    //比例系数 积分系数 微分系数
    PIDFreq.Kp=(float)(0.56);
    PIDFreq.Ki=(float)(0.3);
    PIDFreq.Kd=(float)(0.0);
    //printf("PID_init end \n");
}

void PID_realize() {
    float index;        //index: 变积分系数
    float deta = 1.0;
    int Actual_Voltage;
    float vadc_max, vadc_min, vadc_diff;

    /********  vadc转变成频率 ********/
    Actual_Voltage = Voltage_to_Fc(Capture_voltage);
    //Actual_Fc = (unsigned int) (Capture_voltage * 6849 + 3651);
    PIDFreq.err = Pressure_to_Fc(Set_Pressure) - Actual_Voltage;   //更新err

   /* vadc_max = Set_Pressure;
    vadc_min = vadc_max-vadc_max/10;
    vadc_diff = vadc_max-vadc_min;

    if (fabsf(PIDFreq.err) >= vadc_max){           //变积分过程  vadc_max和min的值如何设定？
        index = 0.0;
    } else if (fabsf(PIDFreq.err) <= vadc_min) {
        index = 1.0;
        PIDFreq.integral += PIDFreq.err;
    } else {
        index = (vadc_max - PIDFreq.err) / vadc_diff;
        PIDFreq.integral += PIDFreq.err;
    }*/

    if (fabsf(PIDFreq.err) > err_max){           //比例环节的改进
        deta = 1.0;
    } else {
        deta = kai;
        //_NOP();
    }
    //index = 0.2;
    deta = 1.0;
    index = 0;
    PIDFreq.integral += PIDFreq.err;

   // PIDFreq.output = Actual_Voltage + deta * PIDFreq.Kp * PIDFreq.err + pid_time * index * PIDFreq.Ki * PIDFreq.integral + PIDFreq.Kd * (PIDFreq.err - PIDFreq.err_last) / pid_time;


    PIDFreq.output = (int) (Actual_Voltage + (deta * PIDFreq.Kp) * PIDFreq.err + pid_time * index * PIDFreq.Ki * PIDFreq.integral + PIDFreq.Kd * (PIDFreq.err - PIDFreq.err_last) / pid_time);

    PIDFreq.err_last = PIDFreq.err;
}
