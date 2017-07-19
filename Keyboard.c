//
// Created by tangyq on 2017/7/18.
//

#include "msp430f5438a.h"
#include "settings.h"
#include "Keyboard.h"

const unsigned char key_table[17]={                     // ɨ���������
        0xFF, // �ް�������                          // ����λΪɨ���У�����λΪɨ����
        0x7E, // K1
        0x7D, // K2
        0x7B, // K3
        0x77, // K4
        0xBE, // K5
        0xBD, // K6
        0xBB, // K7
        0xB7, // K8
        0xDE, // K9
        0xDD, // K10
        0xDB, // K11
        0xD7, // K12
        0xEE, // K13
        0xED, // K14
        0xEB, // K15
        0xE7, // K16
};


void opr_key(unsigned char key_num) {
    switch (key_num) {
        case 0: {

            break;
        }
        case 1: {

            break;
        }
        default: break;
    }
}


void Key_GPIO_init(void){
    KEYH_OUT_DIR;
    KEYL_IN_DIR;
}

/**
 * @desc: ɨ���뷭��
 * @return: ���̶�Ӧ������,������Ϊ0˵���޼�����
 * */
unsigned char translate_key(unsigned char code) {
    unsigned char key_cd = code;                         // ɨ����
    unsigned char key_val;                               // ��������
    for(key_val = 0; key_val <= 16; key_val++){       // ����ɨ������õ���������
        if(key_table[key_val] == key_cd){        // ɨ�����Ƿ����
            break;
        }
    }
    return key_val;
}

/**
 * @desc: ɨ����̷��ؼ�ֵ
 * @return: unsigned char, ��������
 * */
unsigned char read_key() {
    unsigned int i;
    unsigned char col_scan = 0;                          // ��ɨ�����
    unsigned char key_code;                              // ɨ����

    key_code = (unsigned char) (KEY_IN & 0xF0) >> 4;          // ������ɨ����

    col_scan = BIT7;                       // ��ɨ���һ��
    for(i=4 ;i>0; i--)                   // ɨ����
    {
        KEY_OUT = (0xFF & (~col_scan));    // ɨ�����Ӧ���õ͵�ƽ
        if((KEY_IN & 0xF0) != 0xF0)      // �Ƿ���һ���м�����
        {
            break;
        }
        col_scan = col_scan >> 1;        // ɨ����һ��
    }

    col_scan = (unsigned char) ((~col_scan) & 0xF0);       // �õ���ɨ����
    key_code |= col_scan;                // �õ���������ɨ����
    return key_code;
}


/**
 * @desc:�ж��Ƿ��м�����
 * @return: 1, �м�����; 0, �޼�����
 * */
unsigned char press_key() {
    KEY_OUT = 0x00;                             // 4��ȫ�����㣬�ж��Ƿ��м�����
    //KH_OUT_L;                                 // 4��ȫ�����㣬�ж��Ƿ��м�����
    if((KEY_IN & 0xF0) != 0xF0) {
        _NOP();
        return 1;       // �м�����
    }
    else return 0;
}
