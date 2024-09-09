#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H


#include "delay.h" // 非阻塞的延时，不适用于较长延时的场合
#include "dmx512.h" // dmx512协议相关的功能
#include "hardware_pwm_led.h"
#include "rf_decode.h" // rf信号接收、解码
#include "rf_keyhandle.h" // 将rf键值和按键状态相关的处理注册到demo自带的接口中

#define DMX512_SEND_DATA_PIN IO_PORTC_04 // 接收板1转发DMX-512数据包的发送引脚
#define DMX512_RECV_DATA_PIN IO_PORTB_01 // 接收DMX-512数据包的IO

#define HARDWARE_PWM_LED_Y_PIN  // 驱动黄色LED的引脚
#define HARDWARE_PWM_LED_W_PIN  // 驱动白色LED的引脚

#define RF_DECODE_PIN IO_PORTB_05 // rf信号接收引脚

#define RFKEY_ENABLE 1 // 使能rf按键扫描功能
#define RF_LEARN_KEY 0x02 // 学习按键的键值(4位，如果是8位的键值，要修改相应的驱动)

// #include "uart.h" // 接收dmx512数据使用到的串口

#endif

