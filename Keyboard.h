//
// Created by tangyq on 2017/7/18.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

enum key_states_e {
    KEY_STATE_RELEASE,
    KEY_STATE_WAITING,
    KEY_STATE_PRESSED
};

void Key_GPIO_init(void);

void opr_key(unsigned char key_code);

/**
 * @desc: ɨ���뷭��
 * @return: ���̶�Ӧ������,������Ϊ0˵���޼�����
 * */
unsigned char translate_key(unsigned char code);
/**
 * @desc: ɨ����̷��ؼ�ֵ
 * @return: unsigned char, ��������
 * */
unsigned char read_key();
/**
 * @desc:�ж��Ƿ��м�����
 * @return: 1, �м�����; 0, �޼�����
 * */
unsigned char press_key();

#endif //KEYBOARD_H
