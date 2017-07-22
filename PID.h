/*
 * PID.h
 *
 *  Created on: 2017��7��20��
 *      Author: w
 */

#ifndef PID_H_
#define PID_H_

typedef struct _pid pid;

struct _pid{
    //unsigned int Setfreq;            //�����趨ֵ
   // unsigned int Actualfreq;         //����ʵ��ֵ
    float err;                 //����ƫ��ֵ
    float err_last;            //������һ��ƫ��ֵ
    float Kp,Ki,Kd;            //������������֡�΢��ϵ��
    float output;             //�����ѹֵ������ִ�����ı�����
    float integral;            //�������ֵ
};

void PID_init();
void PID_realize();

#endif /* PID_H_ */
