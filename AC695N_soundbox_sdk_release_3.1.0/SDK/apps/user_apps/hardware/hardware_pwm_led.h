#ifndef __HARDWARE_PWM_LED_H
#define __HARDWARE_PWM_LED_H

#include "includes.h"
#include "user_config.h"

// 配置驱动LED的PWM
void hardware_pwm_led_config(void);

// 使用mcpwm驱动LED
void hardware_mcpwm_led_config(void);

#endif

