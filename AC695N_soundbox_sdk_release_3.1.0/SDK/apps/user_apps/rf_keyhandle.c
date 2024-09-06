#include "rf_keyhandle.h"

#include "key_event_deal.h" // 包含rf遥控器按键事件的定义

#define RF_KEY_NUM_MAX 10

#if RFKEY_ENABLE
// rf遥控器按键的事件表，相应的事件要到 key_event_deal.h 中注册
const u16 rf_key_table[RF_KEY_NUM_MAX][KEY_EVENT_MAX] = {
    // 按键真实的键值   单击         //长按          //hold         //抬起            //双击                //三击
    [0] = {0x02, KEY_RF_NUM_1, KEY_EVENT_LONG, KEY_EVENT_HOLD, KEY_NULL, KEY_NULL, KEY_NULL},
    [1] = {0x08, KEY_RF_NUM_2, KEY_EVENT_LONG, KEY_EVENT_HOLD, KEY_NULL, KEY_NULL, KEY_NULL},
    [2] = {0x01, KEY_RF_NUM_3, KEY_EVENT_LONG, KEY_EVENT_HOLD, KEY_NULL, KEY_NULL, KEY_NULL},
    [3] = {0x04, KEY_RF_NUM_4, KEY_EVENT_LONG, KEY_EVENT_HOLD, KEY_NULL, KEY_NULL, KEY_NULL},
    [4] = {0x0C, KEY_RF_NUM_5, KEY_EVENT_LONG, KEY_EVENT_HOLD, KEY_NULL, KEY_NULL, KEY_NULL},
    [5] = {0x06, KEY_RF_NUM_6, KEY_EVENT_LONG, KEY_EVENT_HOLD, KEY_NULL, KEY_NULL, KEY_NULL},
};
#endif

u8 rf_get_key_value(void); // 声明，获取rf遥控器的键值的函数
// 按键驱动扫描参数列表（在key_driver.c文件中注册了按键扫描）
/*
    注意事项：
    1. 按键消抖延时只能是零，否则检测单击的时候会有延迟,
       也可能检测不到单击，或者是按下较长的时间后松开，才认为是单击
    2. 按键被抬起后等待连击延时数量大于20的时候，检测单击时就会不灵敏
    3. 按键扫描频率要大于 信号的周期，包括单击时，两个信号之间的时间间隔
*/
struct key_driver_para rfkey_scan_para = {
    .scan_time = 38,        // 按键扫描频率, 单位: ms
    .last_key = NO_KEY,     // 上一次get_value按键值, 初始化为NO_KEY;
    .filter_time = 1,       // 按键消抖延时;
    .long_time = 750 / 38,        // 按键判定长按数量
    .hold_time = (750 + 150) / 38, // 按键判定HOLD数量
    // .click_delay_time = 20,         // 按键被抬起后等待连击延时数量
    .click_delay_time = 8,          // 按键被抬起后等待连击延时数量
    .key_type = KEY_DRIVER_TYPE_RF, // 按键类型为RF遥控器按键
    .get_value = rf_get_key_value,
};

// 获取rf遥控器按键的键值
static u8 rf_get_key_value(void)
{
    /*
        加入 filter_time 的相关处理
        防止单击rf按键的信号个数较少时，检测不到单击
    */
    static u8 filter_cnt = 0;
    u32 tmp_rf_data = 0;

    tmp_rf_data = rf_data; // 读取接收到的rf数据

    if (filter_cnt > 1)
    {
        filter_cnt--;
    }

    // 如果键值检测不通过，但是filter_cnt 大于1，依然认为是有效按键
    // if ((tmp_rf_data & 0x0F) == 0 && filter_cnt <= 1)
    // if ((tmp_rf_data & 0x0F) == 0 || (tmp_rf_data & 0x0F) == 0x0F)
    if (((tmp_rf_data >> 4) & 0xFFFFFFF) != 0x9BFA4)
    {
        // 如果地址不匹配
        if (filter_cnt == 1)
        {
            filter_cnt = 0;
        }
        return NO_KEY;
    }

    if (0 == filter_cnt)
    {
        // tmp_rf_data = 0xFFFFFFFF;
        filter_cnt = rfkey_scan_para.filter_time + 2;
    }

    if (filter_cnt == 1)
    {
        rf_data = 0xFFFFFFFF; // 清零
    }
        
    // printf("%s %d\n", __FUNCTION__, __LINE__);
    // printf("rf_key %d\n", (tmp_rf_data & 0x0F));
    // printf("rf_addr %x", (tmp_rf_data >> 4)); // 打印遥控器的地址
    return tmp_rf_data & 0x0F; // 直接获取接收到的rf数据
}

// 根据当前的任务和按键键值，返回对应的事件（事件号）
u16 rfkey_event_to_msg(u8 cur_task, struct key_event *key)
{
    u16 i = 0;

#if 0
    // if (rf_key_table[cur_task] == NULL)
    // {
    //     return KEY_NULL;
    // }

    // type_key_ad_table cur_task_ad_table = ad_table[cur_task];
    // return cur_task_ad_table[key->value][key->event];

    // return rf_key_table[key->value][key->event]; // 返回rf遥控器按键的事件表中对应的事件
    // return KEY_RF_NUM_1;
    // return rf_key_table[key->value][key->event];

#endif

    printf("-----------------function  %s %d\n", __FUNCTION__, __LINE__);
    printf("key->value %u \nkey->event %d \n", key->value, key->event);

    for (i = 0; i < RF_KEY_NUM_MAX; i++)
    {
        if (key->value == rf_key_table[i][0])
        {
            return rf_key_table[i][1 + key->event];
        }
    }

    return KEY_NULL; // 返回无效的事件
}
