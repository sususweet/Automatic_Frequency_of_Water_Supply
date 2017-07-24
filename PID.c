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
#define kai 0.15    // 0<kai<1

#define err_Mmax 4.0
#define err_Mmid 1.5
//#define err_Mmid 2.75
#define err_Mmin 1.0


pid PIDFreq;

extern float Set_Pressure;
extern volatile float Capture_voltage;

void PID_init(){
    PIDFreq.err = 0;
    PIDFreq.err_last = 0;
    PIDFreq.err_last_last = 0;
    PIDFreq.output = 0;
    PIDFreq.stable = 0;
    PIDFreq.integral=(float)(0.0);
    //比例系数 积分系数 微分系数
    PIDFreq.Kp=(float)(1.28);  //1.625
    PIDFreq.Ki=(float)(0.30);    //0.30  ->  0.8
    PIDFreq.Kd=(float)(0.0226);
}

void PID_realize() {
    float index = 0.0;        //index: 变积分系数
    float deta = 1.0;
    float Actual_Pressure;

    float vadc_max, vadc_min, vadc_diff;


    float err_change_rate = 0.0, err_change_rate_last = 0.0;
    /********  vadc转变成频率 ********/
    Actual_Pressure = Voltage_to_Pressure_Show(Capture_voltage);

    PIDFreq.err =  Set_Pressure - Actual_Pressure;  //e(k)

    if (Actual_Pressure > 20.0 || Actual_Pressure < 2.0){
        PIDFreq.err = 0;
    }
    
    /*e(k)表示离散化的当前采样时刻的误差值，e(k -1)，e(k -2)分别表示前一个和前两个采样时刻的误差值*/
    /* Δe(k)=e(k)-e(k -1)*/
    err_change_rate = (float) ((PIDFreq.err - PIDFreq.err_last) / pid_time);
    /*Δe(k -1)=e(k -1)-e(k -2)*/
    err_change_rate_last = (float) ((PIDFreq.err_last - PIDFreq.err_last_last) / pid_time);

    //Pressure_to_Fc(PIDFreq.output)
    if (fabsf(PIDFreq.err) > err_Mmax - 1.55 && PIDFreq.err > 0){          /*误差大于一定范围，实施开环控制*/
        PIDFreq.output = Max_Fc;
        //PIDFreq.output += 2000;
        PIDFreq.integral = 0;
    }else if (fabsf(PIDFreq.err) > err_Mmax - 1.55 && PIDFreq.err < 0){
        PIDFreq.output = Min_Fc;
        //PIDFreq.output -= 2000;
        PIDFreq.integral = 0;
    }else{
        /*误差绝对值很小，此时加入积分，减小静态误差*/
        if (fabsf(PIDFreq.err) < err_Mmin){
            deta = 0.0;
            index = 1.0;
            PIDFreq.integral += PIDFreq.err;
            PIDFreq.output =  PIDFreq.output;
            PIDFreq.stable = 1;
        }
        /*e(k)Δe(k)<0，Δe(k)Δe(k -1)>0或e(k)=0*/
        else if ((PIDFreq.err * err_change_rate < 0 && err_change_rate * err_change_rate_last > 0)){
            PIDFreq.output =  PIDFreq.output;                           /*系统基本达到稳定，保持PID*/
            PIDFreq.integral += PIDFreq.err;
            PIDFreq.stable = 1;
            _NOP();
        }else{
            if (fabsf(PIDFreq.err) > 0.8 && fabsf(PIDFreq.err - PIDFreq.err_last) > 1.1) PIDFreq.stable = 0;
            if (PIDFreq.stable == 0){
                if (PIDFreq.err * err_change_rate > 0
                    || fabsf(err_change_rate) < 1e-5){                      /*误差向绝对值增大的方向变化*/
                    if(fabsf(PIDFreq.err) > err_Mmid){                      /*误差较大，实施较强控制*/
                        deta = 0.80;                                        /*deta需要较大以实现快速控制*/
                        index = 0;
                    }else {                                                 /*误差绝对值本身并不是很大，实施一般控制*/
                        deta = 0.6;
                        index = 0.01;
                        PIDFreq.integral += PIDFreq.err;
                    }
                }
                    /*e(k)Δe(k)<0，Δe(k)Δe(k -1)<0时，说明误差处于极值状态*/
                else if (PIDFreq.err * err_change_rate < 0 && err_change_rate * err_change_rate_last < 0){
                    if(fabsf(PIDFreq.err) > err_Mmid){                      /*误差较大，实施较强控制*/
                        deta = 1.0;
                        index = 0;
                    }else {                                                 /*误差绝对值较小，实施一般控制*/
                        deta = 0.2;
                        index = 0.2;
                        PIDFreq.integral += PIDFreq.err;
                    }
                }


                PIDFreq.output =Pressure_to_Fc((float) (Actual_Pressure + deta * PIDFreq.Kp * err_change_rate
                                                        + pid_time * index * PIDFreq.Ki * PIDFreq.integral
                                                        + PIDFreq.Kd * (err_change_rate - err_change_rate_last) / pid_time));

            }

        }

    }







#ifdef DEBUG
    if (Actual_Pressure > 23.0){
        PIDFreq.err = 0;
    }else{
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
        if(fabsf(PIDFreq.err)<0.8){
            index = 1.0;
            PIDFreq.integral += PIDFreq.err;
        } else if (fabsf(PIDFreq.err)>2.0){
            index = 0.0;
            // PIDFreq.integral += PIDFreq.err;
        } else{
            index = (float) ((3 - fabsf(PIDFreq.err))/ 2.2);
            PIDFreq.integral += PIDFreq.err;
            //Todo: Change index to 0.2~0.5
        }
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
        } else{
            deta = kai;
            //_NOP();
        }
        /*else if (fabsf(PIDFreq.err) < 0.5){
            deta = 0;
        }*/
        //index = 0.2;
        //PIDFreq.integral += PIDFreq.err;

        PIDFreq.output = Actual_Pressure + deta * PIDFreq.Kp * PIDFreq.err + pid_time * index * PIDFreq.Ki * PIDFreq.integral + PIDFreq.Kd * (PIDFreq.err - PIDFreq.err_last) / pid_time;

        //    PIDFreq.output = Actual_Pressure + (deta * (PIDFreq.Kp * PIDFreq.err + pid_time * index * PIDFreq.Ki * PIDFreq.integral + PIDFreq.Kd * (PIDFreq.err - PIDFreq.err_last) / pid_time));

        PIDFreq.err_last = PIDFreq.err;
        //  }
    }
#endif


}
