#ifndef __RF_KEYHANDLE_H
#define __RF_KEYHANDLE_H

#include "includes.h"
#include "user_config.h"

extern struct key_driver_para rfkey_scan_para;
extern u16 rfkey_event_to_msg(u8 cur_task, struct key_event *key);

#endif
