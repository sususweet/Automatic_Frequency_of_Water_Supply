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
 * @desc: 扫描码翻译
 * @return: 键盘对应的数字,如果输出为0说明无键按下
 * */
unsigned char translate_key(unsigned char code);
/**
 * @desc: 扫描键盘返回键值
 * @return: unsigned char, 完整键码
 * */
unsigned char read_key();
/**
 * @desc:判断是否有键按下
 * @return: 1, 有键按下; 0, 无键按下
 * */
unsigned char press_key();

#endif //KEYBOARD_H
