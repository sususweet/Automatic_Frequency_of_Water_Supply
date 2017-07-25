/*
 * PID.h
 *
 *  Created on: 2017年7月20日
 *      Author: w
 */

#ifndef PID_H_
#define PID_H_

#define Max_Fc 10425
//#define Max_Fc 11955
#define Min_Fc 7950

typedef struct _pid pid;

struct _pid{
    //unsigned int Setfreq;            //定义设定值
   // unsigned int Actualfreq;         //定义实际值
    float err;                 //定义偏差值
    float err_last;            //定义上一个偏差值
    float err_last_last;        //定义上上一个偏差值
    float Kp,Ki,Kd;            //定义比例、积分、微分系数
    unsigned int output;             //定义电压值（控制执行器的变量）
    unsigned char stable;
    unsigned char more_offset;
    unsigned char less_offset;
    float integral;            //定义积分值
};

void PID_init();
void PID_realize();

#endif /* PID_H_ */
