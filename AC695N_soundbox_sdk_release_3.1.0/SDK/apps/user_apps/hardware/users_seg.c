// encoding -- UTF-8
// SEG驱动源程序
#include "users_seg.h"
#include "led7_driver.h"
#include "ui/ui_api.h"

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

extern void users_seg_main_menu(void);
static void *users_open_seg(void *hd);
static void users_close_seg(void *hd, void *private);
// 自定义led7主界面
const struct ui_dis_api users_led7_main = {
    .ui = UI_USERS_LED_MENU,
    .open = users_open_seg,         // 打开
    .ui_main = users_seg_main_menu, // 主界面
    .ui_user = NULL,                // 子界面
    .close = users_close_seg,       // 关闭
};

static void *users_open_seg(void *hd)
{
    void *private = NULL;
    ui_set_auto_reflash(500); // 设置主页500ms自动刷新
    return private;
}

static void users_close_seg(void *hd, void *private)
{
    LCD_API *dis = (LCD_API *)hd;
    if (!dis)
    {
        return;
    }
    if (private)
    {
        free(private);
    }
}

// 用户的数码管配置函数
void users_seg_config(void)
{
    led7_init(&led7_test_data);          // 初始化，里面会将参数的内容拷贝至__this->user_data中
    ui_set_main_menu(UI_USERS_LED_MENU); // 设置主界面
}

// 主界面
void users_seg_main_menu(void)
{
    led7_show_lock(1);
    led7_setX(0); // 设置开始显示的坐标
    led7_show_null(); // 清空显示
    led7_show_icon(LED7_USB);
    led7_show_icon(LED7_FM);
    led7_show_icon(LED7_COLON);
    led7_show_icon(LED7_ST);
    led7_show_icon(LED7_MHZ);
    led7_show_icon(LED7_SD);
    led7_show_icon(LED7_POINT);
    led7_show_icon(LED7_MP3);
    led7_show_string("tley");
    led7_show_lock(0);
}

// 子界面
void users_seg_subinterface(void)
{
    led7_show_lock(1);
    led7_setX(0);
    led7_show_null(); // 清空显示

    // led7_show_number();

    led7_show_lock(0);
}
