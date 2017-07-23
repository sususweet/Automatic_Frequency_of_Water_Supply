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

#define err_max 1.0
#define err_min 80
#define pid_time 1.6
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
    PIDFreq.Kp=(float)(1.58);  //1.625
    PIDFreq.Ki=(float)(0.85);    //0.85  ->  0.8
    PIDFreq.Kd=(float)(0.02);
    //printf("PID_init end \n");
}

void PID_realize() {
    float index;        //index: 变积分系数
    float deta = 1.0;
    float Actual_Pressure;
    float vadc_max, vadc_min, vadc_diff;

    /********  vadc转变成频率 ********/
    Actual_Pressure = Voltage_to_Pressure_Show(Capture_voltage);
    //Actual_Fc = (unsigned int) (Capture_voltage * 6849 + 3651);
    PIDFreq.err = Set_Pressure - Actual_Pressure;   //更新err

 //   if(PIDFreq.err>=0.5){
/*
        vadc_max = Set_Pressure;
        vadc_min = vadc_max-vadc_max/10;
        vadc_diff = vadc_max-vadc_min;

        if(fabsf(PIDFreq.err)<0.5){
           index = 1.0;
           PIDFreq.integral += PIDFreq.err;
        } else if (fabsf(PIDFreq.err)>3){
            index = 0.0;
            // PIDFreq.integral += PIDFreq.err;
        } else{
            index = (float) ((3 - fabsf(PIDFreq.err))/ 2.5);
            PIDFreq.integral += PIDFreq.err;
        }
*/

        /*if (Actual_Pressure >= Set_Pressure + 2.5){           //变积分过程  vadc_max和min的值如何设定？
            index = 0.0;
        } else if (Actual_Pressure <= Set_Pressure - 1.0) {
            index = 1.0;
            PIDFreq.integral += PIDFreq.err;
        } else {
            index = 0.2;
            //index = (vadc_max - PIDFreq.err) / vadc_diff;
            PIDFreq.integral += PIDFreq.err;
        }
        */
        if (fabsf(PIDFreq.err) > err_max){           //比例环节的改进
            deta = 1;
        } else {
            deta = kai;
            //_NOP();
        }

        index = 0.2;
        PIDFreq.integral += PIDFreq.err;

        PIDFreq.output = Actual_Pressure + deta * PIDFreq.Kp * PIDFreq.err + pid_time * index * PIDFreq.Ki * PIDFreq.integral + PIDFreq.Kd * (PIDFreq.err - PIDFreq.err_last) / pid_time;

    //    PIDFreq.output = Actual_Pressure + (deta * (PIDFreq.Kp * PIDFreq.err + pid_time * index * PIDFreq.Ki * PIDFreq.integral + PIDFreq.Kd * (PIDFreq.err - PIDFreq.err_last) / pid_time));

        PIDFreq.err_last = PIDFreq.err;
  //  }
}
