#ifndef __DMX512_H
#define __DMX512_H

#include "includes.h"

bool dmx512_rx_config(void); 

// 发送dmx512数据包
void dmx512_send_start(void);

#endif
