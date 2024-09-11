#ifndef __RF_DECODE_H
#define __RF_DECODE_H

#include "includes.h"
#include "user_config.h"

#define MSG_RF_RECV_FLAG 0xA5 // 消息，表示完成了一次rf数据接收（需要注意不能与 "key_event_deal.h"文件中的rf按键事件的相关定义冲突）

// rf信号接收引脚：
#ifndef RF_DECODE_PIN
#define RF_DECODE_PIN IO_PORTB_05
#endif 

extern volatile u32 rf_data; // 存放接收到的rf数据
extern volatile u32 rf_addr; // 存放rf遥控器的器件地址

void rf_config(void);
void rf_decode_task_handler(void *p);


#endif
