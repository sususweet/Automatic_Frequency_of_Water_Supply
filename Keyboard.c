//
// Created by tangyq on 2017/7/18.
//

#include "msp430f5438a.h"
#include "settings.h"
#include "Keyboard.h"

const unsigned char key_table[17]={                     // 扫描码译码表
        0xFF, // 无按键按下                          // 高四位为扫描行，低四位为扫描列
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
 * @desc: 扫描码翻译
 * @return: 键盘对应的数字,如果输出为0说明无键按下
 * */
unsigned char translate_key(unsigned char code) {
    unsigned char key_cd = code;                         // 扫描码
    unsigned char key_val;                               // 键盘数字
    for(key_val = 0; key_val <= 16; key_val++){       // 查找扫描码表，得到键盘数字
        if(key_table[key_val] == key_cd){        // 扫描码是否相符
            break;
        }
    }
    return key_val;
}

/**
 * @desc: 扫描键盘返回键值
 * @return: unsigned char, 完整键码
 * */
unsigned char read_key() {
    unsigned int i;
    unsigned char col_scan = 0;                          // 行扫描变量
    unsigned char key_code;                              // 扫描码

    key_code = (unsigned char) (KEY_IN & 0xF0) >> 4;          // 保存列扫描码

    col_scan = BIT7;                       // 先扫描第一行
    for(i=4 ;i>0; i--)                   // 扫描行
    {
        KEY_OUT = (0xFF & (~col_scan));    // 扫描的相应行置低电平
        if((KEY_IN & 0xF0) != 0xF0)      // 是否这一行有键按下
        {
            break;
        }
        col_scan = col_scan >> 1;        // 扫描下一行
    }

    col_scan = (unsigned char) ((~col_scan) & 0xF0);       // 得到行扫描码
    key_code |= col_scan;                // 得到完整键盘扫描码
    return key_code;
}


/**
 * @desc:判断是否有键按下
 * @return: 1, 有键按下; 0, 无键按下
 * */
unsigned char press_key() {
    KEY_OUT = 0x00;                             // 4行全部置零，判断是否有键按下
    //KH_OUT_L;                                 // 4行全部置零，判断是否有键按下
    if((KEY_IN & 0xF0) != 0xF0) {
        _NOP();
        return 1;       // 有键按下
    }
    else return 0;
}
