#ifndef __RF_KEYHANDLE_H
#define __RF_KEYHANDLE_H

#include "includes.h"
#include "user_config.h"


#ifndef RF_LEARN_KEY
#define RF_LEARN_KEY 0x02 // 学习按键的键值(4位，如果是8位的键值，要修改相应的驱动)
#endif

extern struct key_driver_para rfkey_scan_para;
extern u16 rfkey_event_to_msg(u8 cur_task, struct key_event *key);

#endif
