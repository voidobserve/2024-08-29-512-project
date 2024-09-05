#ifndef __RF_DECODE_H
#define __RF_DECODE_H

#include "includes.h"
#include "user_config.h"

#define MSG_RF_RECV_FLAG 0xA5 // 消息，表示完成了一次接收

extern volatile u32 rf_data; // 倒序存放接收到的rf数据
// extern volatile u8 rf_recv_flag; // 标志位，表示是否完成一次rf数据的接收，0--未接收，1--接收

void rf_config(void);

void rf_decode_task_handler(void *p);
// void fun(void *p);

#endif
