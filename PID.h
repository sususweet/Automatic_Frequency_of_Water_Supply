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
    int err;                 //����ƫ��ֵ
    int err_last;            //������һ��ƫ��ֵ
    float Kp,Ki,Kd;            //������������֡�΢��ϵ��
    int output;             //�����ѹֵ������ִ�����ı�����
    float integral;            //�������ֵ
};

void PID_init();
void PID_realize();

#endif /* PID_H_ */
