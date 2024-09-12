// encoding -- UTF-8
// SEG驱动源程序
#include "users_seg.h"
#include "led7_driver.h"



// 配置引脚 定义了 led7_platform_data 类型的变量：
LED7_PLATFORM_DATA_BEGIN(led7_test_data)
    .pin_type = LED7_PIN7, // 7脚SEG
    .pin_cfg.pin7.pin[0] = IO_PORTB_00,
   .pin_cfg.pin7.pin[1] = IO_PORTB_01,
   .pin_cfg.pin7.pin[2] = IO_PORTB_02,
   .pin_cfg.pin7.pin[3] = IO_PORTB_03,
   .pin_cfg.pin7.pin[4] = IO_PORTB_04,
   .pin_cfg.pin7.pin[5] = IO_PORTB_05,
   .pin_cfg.pin7.pin[6] = IO_PORTB_06,

   LED7_PLATFORM_DATA_END();

void led7_test(void)
{
    led7_init(&led7_test_data); // 初始化，里面会将参数的内容拷贝至__this->user_data中

    led7_show_lock(1);;
    led7_setX(0);
    led7_show_null();

    led7_show_icon(LED7_USB);
    led7_show_icon(LED7_FM);
    led7_show_icon(LED7_COLON);
    led7_show_icon(LED7_ST);
    led7_show_icon(LED7_MHZ);
    led7_show_icon(LED7_SD);
    led7_show_icon(LED7_POINT);
    led7_show_icon(LED7_MP3);
    led7_show_lock(0);

    led7_show_string("tle");
    
    while (1)
    {
        printf("%s %d\n", __func__, __LINE__);

#if 0
            // led7_show_icon(LED7_USB);
        // led7_show_icon(LED7_FM);
        // led7_show_icon(LED7_COLON);
        // led7_show_icon(LED7_ST);
        // led7_show_icon(LED7_MHZ);
        // led7_show_icon(LED7_SD);
        // led7_show_icon(LED7_POINT);
        // led7_show_icon(LED7_MP3);
        // os_time_dly(100);

        // led7_show_icon(LED7_USB);
        // os_time_dly(100);

        // led7_show_icon(LED7_FM);
        // os_time_dly(100);

        // led7_show_icon(LED7_COLON);
        // os_time_dly(100);

        // led7_show_icon(LED7_ST);
        // os_time_dly(100);

        // led7_show_icon(LED7_MHZ); //
        // os_time_dly(100);

        // led7_show_icon(LED7_SD);
        // os_time_dly(100);

        // led7_show_icon(LED7_POINT); //
        // os_time_dly(100);

        // led7_show_icon(LED7_MP3); //
        // os_time_dly(100);
#endif
    }
}

void users_seg_task_init(void)
{
    task_create(led7_test, NULL, "users_seg");
}
module_initcall(users_seg_task_init);
