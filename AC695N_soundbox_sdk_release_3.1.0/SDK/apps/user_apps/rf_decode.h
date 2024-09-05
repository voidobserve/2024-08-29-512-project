#ifndef __RF_DECODE_H
#define __RF_DECODE_H

#include "includes.h"
#include "user_config.h"

extern volatile u32 rf_data; // 倒序存放接收到的rf数据

void rf_config(void);

#endif
