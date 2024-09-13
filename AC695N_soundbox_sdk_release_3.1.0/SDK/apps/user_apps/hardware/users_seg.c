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

static void users_seg_main_menu(void);                                        // 声明
static int users_seg_subinterface(void *hd, void *private, u8 menu, u32 arg); // 声明，子界面显示
static void *users_open_seg(void *hd);
static void users_close_seg(void *hd, void *private);
// 自定义led7界面配置数据
const struct ui_dis_api users_led7_main = {
    .ui = UI_USERS_LED_MENU,
    .open = users_open_seg,            // 打开
    .ui_main = users_seg_main_menu,    // 主界面
    .ui_user = users_seg_subinterface, // 子界面
    .close = users_close_seg,          // 关闭
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

// 主界面
static void users_seg_main_menu(void)
{
    led7_show_lock(1);
    led7_setX(0);     // 设置开始显示的坐标
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

static void __show_num_1(void)
{
    static u16 num = 0;
    static u8 cnt = 0;

    led7_show_lock(1);
    led7_setX(0);     // 设置开始显示的坐标
    led7_show_null(); // 清空显示

    switch (cnt)
    {
    case 0:
        led7_show_number(5536);
        break;

    case 1:
        led7_show_number(4417);
        break;

    case 2:
        led7_show_number(8972);
        break;

    default:
        printf("err =========== %s %d\n", __func__, __LINE__);
        break;
    }
    led7_show_lock(0);

    cnt++;
    if (cnt >= 3)
    {
        cnt = 0;
    }
}

static void __call_show_num_2(void)
{
    // 界面/窗口的超时时间要大于sys_timer_add()设定的时间,才不会在执行完对应的功能前返回主界面
    // 如果后续添加了更多的功能,这里的时间可能还要更长一些
    // sys_timer_add()添加的定时器在定时时间抵达时,会打断当前的界面显示,进行覆盖
    ui_set_tmp_menu(MENU_SHOW_NUM_2, 1100, 0, NULL);
}

static void __show_num_2(void)
{
    static int16_t cnt = 60;
    static u16 timer_id = 0; // 存放sys_timer_add()函数返回的id

    u8 ret = ui_get_app_menu(0); 
    // 如果不是当前的子界面,说明在界面来回切换时,
    // 可能没有来得及关闭定时器,又进来了这里
    if (ret != MENU_SHOW_NUM_2) 
    {
        if (timer_id) // 确认id不为0,再删除
        {
            sys_timer_del(timer_id); // 根据id删除sys_timer_add添加的功能
        }

        timer_id = 0; // 清除变量的值
        cnt = 60;     // 恢复变量的默认值
        return;
    }

    led7_show_lock(1); // 锁定待显示的缓冲区,避免出现闪烁的情况
    if (timer_id == 0)
    {
        led7_show_null(); // 清空显示/
        timer_id = sys_timer_add(NULL, __call_show_num_2, 1000);
    }

    printf("%s %d \n", __func__, __LINE__);

    led7_setX(0); // 设置开始显示的坐标
    led7_show_number((u16)cnt);
    led7_show_lock(0); // 编辑好待显示的缓冲区后,再解锁

    if (cnt >= 0)
    {
        cnt--;
    }

    if (cnt < 0)
    {
        // 进入到这里，说明cnt == -1
        if (timer_id) // 确认id不为0,再删除
        {
            sys_timer_del(timer_id); // 根据id删除sys_timer_add添加的功能
        }

        timer_id = 0; // 清除变量的值
        cnt = 60;     // 恢复变量的默认值
    }
}

// 子界面
// 通过调用ui_set_tmp_menu()来进入
static int users_seg_subinterface(void *hd, void *private, u8 menu, u32 arg) // 子界面显示 //返回true不继续传递 ，返回false由common统一处理
{
    int ret = true;
    LCD_API *dis = (LCD_API *)hd;
    if (!hd)
    {
        return false;
    }

    // 根据ui_set_tmp_menu()传入的参数进行判断
    switch (menu)
    {
    case MENU_SHOW_NUM_1:
        __show_num_1();
        break;
    case MENU_SHOW_NUM_2:
        __show_num_2();
        break;

    default:
        ret = false;
    }

    return ret;
}

// 用户的数码管配置函数
void users_seg_config(void)
{
    led7_init(&led7_test_data);          // 初始化，里面会将参数的内容拷贝至__this->user_data中
    ui_set_main_menu(UI_USERS_LED_MENU); // 设置主界面
}
