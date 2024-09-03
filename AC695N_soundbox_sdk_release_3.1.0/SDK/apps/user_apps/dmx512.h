#ifndef __DMX512_H
#define __DMX512_H

#include "includes.h"

extern u8 dmx512_txbuff[513]; // 存放待发送的dmx512信息包中的数据

void dmx512_config(void);

// 发送dmx512数据包
void dmx512_send_start(void);

#endif
