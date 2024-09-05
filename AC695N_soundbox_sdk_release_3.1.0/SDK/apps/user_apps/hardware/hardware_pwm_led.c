#include "hardware_pwm_led.h"
#include "asm/mcpwm.h"

// 配置驱动LED的PWM
void hardware_pwm_led_config(void)
{
    // （实际上占空比 == duty / fre，由这两个传参共同决定）
    timer_pwm_init(JL_TIMER0, 10000, 994, IO_PORTA_05, 0);
    timer_pwm_init(JL_TIMER1, 10000, 994, IO_PORTA_12, 0);

    // os_time_dly(1000);
    // set_timer_pwm_duty(JL_TIMER0, 0); // 0%占空比（不知道实际情况会不会让灯微微点亮）

    // os_time_dly(555); // 延时基本单位：10ms
    // set_timer_pwm_duty(JL_TIMER0, 195);
}

// 点亮黄灯
void hardware_pwm_led_y_on(void)
{

}

// 点亮白灯
void hardware_pwm_led_w_on(void)
{

}

// 熄灭黄灯
void hardware_pwm_led_y_off(void)
{

}

// 熄灭白灯
void hardware_pwm_led_w_off(void)
{

}

// 使用mcpwm驱动LED（测试通过）
void hardware_mcpwm_led_config(void)
{
    struct pwm_platform_data pwm_p_data;
    // CH0
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned; // 边沿对齐
    pwm_p_data.frequency = 1000;                    // 1KHz
    pwm_p_data.pwm_ch_num = pwm_ch0;                // 通道0
    pwm_p_data.pwm_timer_num = pwm_timer0;          // 时基选择通道0
    pwm_p_data.duty = 5000;                         // 占空比50%
    // 硬件引脚：
    pwm_p_data.h_pin = IO_PORTA_00; // 没有则填 -1。h_pin_output_ch_num无效，可不配置
    pwm_p_data.l_pin = IO_PORTA_01; // 硬件引脚，l_pin_output_ch_num无效，可不配置
    // 非mcpwm硬件的引脚：
    /* pwm_p_data.h_pin = IO_PORTB_00;                          //没有则填 -1。h_pin_output_ch_num无效，可不配置 */
    /* pwm_p_data.l_pin = IO_PORTB_01;                         //硬件引脚，l_pin_output_ch_num无效，可不配置 */
    /* pwm_p_data.h_pin_output_ch_num = 0;                          //output channel0 */
    /* pwm_p_data.l_pin_output_ch_num = 1;                          //output channel1 */
    pwm_p_data.complementary_en = 1; // 两个引脚的波形, 1: 互补, 0: 同步;

    mcpwm_init(&pwm_p_data);
}
