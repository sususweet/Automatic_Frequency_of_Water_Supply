/*
 * PID.h
 *
 *  Created on: 2017��7��20��
 *      Author: w
 */

#ifndef PID_H_
#define PID_H_

#define Max_Fc 10450
//#define Max_Fc 11955
#define Min_Fc 7950

typedef struct _pid pid;

struct _pid{
    //unsigned int Setfreq;            //�����趨ֵ
   // unsigned int Actualfreq;         //����ʵ��ֵ
    float err;                 //����ƫ��ֵ
    float err_last;            //������һ��ƫ��ֵ
    float err_last_last;        //��������һ��ƫ��ֵ
    float Kp,Ki,Kd;            //������������֡�΢��ϵ��
    unsigned int output;             //�����ѹֵ������ִ�����ı�����
    unsigned char stable;
    float integral;            //�������ֵ
};

void PID_init();
void PID_realize();

#endif /* PID_H_ */
