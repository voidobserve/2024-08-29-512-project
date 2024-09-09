#include "rf_keyhandle.h"

#include "key_event_deal.h" // 包含rf遥控器按键事件的定义

#define RF_KEY_NUM_MAX 10

#if RFKEY_ENABLE
// rf遥控器按键的事件表，相应的事件要到 key_event_deal.h 中注册
const u16 rf_key_table[RF_KEY_NUM_MAX][KEY_EVENT_MAX] = {
    // 按键真实的键值   单击         //长按               //hold             //抬起              //双击                     //三击
    [0] = {0x02, KEY_RF_NUM_1_CLICK, KEY_RF_NUM_1_LONG, KEY_RF_NUM_1_HOLD, KEY_RF_NUM_1_LOOSE, KEY_RF_NUM_1_DOUBLE_CLICK, KEY_RF_NUM_1_TRIPLE_CLICK},
    [1] = {0x08, KEY_RF_NUM_2_CLICK, KEY_RF_NUM_2_LONG, KEY_RF_NUM_2_HOLD, KEY_RF_NUM_2_LOOSE, KEY_RF_NUM_2_DOUBLE_CLICK, KEY_RF_NUM_2_TRIPLE_CLICK},
    [2] = {0x01, KEY_RF_NUM_3_CLICK, KEY_RF_NUM_3_LONG, KEY_RF_NUM_3_HOLD, KEY_RF_NUM_3_LOOSE, KEY_RF_NUM_3_DOUBLE_CLICK, KEY_RF_NUM_3_TRIPLE_CLICK},
    [3] = {0x04, KEY_RF_NUM_4_CLICK, KEY_RF_NUM_4_LONG, KEY_RF_NUM_4_HOLD, KEY_RF_NUM_4_LOOSE, KEY_RF_NUM_4_DOUBLE_CLICK, KEY_RF_NUM_4_TRIPLE_CLICK},
    [4] = {0x0C, KEY_RF_NUM_5_CLICK, KEY_RF_NUM_5_LONG, KEY_RF_NUM_5_HOLD, KEY_RF_NUM_5_LOOSE, KEY_RF_NUM_5_DOUBLE_CLICK, KEY_RF_NUM_5_TRIPLE_CLICK},
    [5] = {0x06, KEY_RF_NUM_6_CLICK, KEY_RF_NUM_6_LONG, KEY_RF_NUM_6_HOLD, KEY_RF_NUM_6_LOOSE, KEY_RF_NUM_6_DOUBLE_CLICK, KEY_RF_NUM_6_TRIPLE_CLICK},
};
#endif

static u8 rf_get_key_value(void);     // 声明，获取rf遥控器的键值的函数
static u8 key_driver_timer_isr(void); // 声明，获取rf遥控器的键值的函数
extern u8 rf_learn(void);             // 声明，用于执行学习对应的功能

// 按键驱动扫描参数列表（在key_driver.c文件中注册了按键扫描）
/*
    注意事项：
    1. 按键消抖延时只能是零，否则检测单击的时候会有延迟,
       也可能检测不到单击，或者是按下较长的时间后松开，才认为是单击
    2. 按键被抬起后等待连击延时数量大于20的时候，检测单击时就会不灵敏
    3. 按键扫描频率要大于 信号的周期，包括单击时，两个信号之间的时间间隔
*/
#define __RFKEY_SCAN_TIME 38 // rf按键扫描频率，单位: ms
struct key_driver_para rfkey_scan_para = {
    .scan_time = __RFKEY_SCAN_TIME,    // 按键扫描频率, 单位: ms
    .last_key = NO_KEY, // 上一次get_value按键值, 初始化为NO_KEY;
    .filter_time = 1,   // 按键消抖延时;
    // .filter_time = 0,              // 按键消抖延时;
    .long_time = 750 / 38,         // 按键判定长按数量（注意是数量）
    .hold_time = (750 + 150) / 38, // 按键判定HOLD数量（注意是数量）
    // .click_delay_time = 20,         // 按键被抬起后等待连击延时数量（注意是数量）
    .click_delay_time = 0,             // 按键被抬起后等待连击延时数量（注意是数量）
    .key_type = KEY_DRIVER_TYPE_RF,    // 按键类型为RF遥控器按键
    .get_value = key_driver_timer_isr, // 获取rf遥控器按键的键值的函数
};

// 获取rf遥控器按键的键值(要求.filter_time按键消抖延时至少要大于等于1)
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

// .filter_time需要设置为0
#define FILTER_TIME_MAX 2 // FILTER_TIME_MAX不能为0，否则函数中的数组会越界
static u8 key_driver_timer_isr(void)
{
    static u8 learning_flag = 0; // 是否处于学习状态的标志位，0--不在学习，1--学习状态

#if FILTER_TIME_MAX
    static u32 code[FILTER_TIME_MAX + 1] = // 当防抖用
        {
            0xFFFFFFFF, // code[0],记录抬起/按下用, 在判断为抬起时置0xFFFFFFFF
            0xFFFFFFFF};

    // 如果按键有按下（有按下的话，code[1]及之后的元素不为0xFFFFFFFF）
    if (code[1] != 0xFFFFFFFF)
    {
        rf_data = code[1];

        /*
        将数组的元素往前移动：
        code[1] = code[2];
        code[2] = code[3];
        code[3] = 0xFFFFFFFF;
        */
        for (u8 i = 1; i < ARRAY_SIZE(code) - 1; i++)
        {
            code[i] = code[i + 1];
        }
        code[ARRAY_SIZE(code) - 1] = 0xFFFFFFFF; // 移动后，最末尾的元素清零
    }
#endif

    // 如果没有学习过器件地址并且按下了学习按键
    // if (((u32)0xFFFFFFFF == rf_addr) &&
    //     ((u8)0x02 == (u8)(rf_data & 0x0F)))
    if (learning_flag ||
        (/*((u32)0xFFFFFFFF == rf_addr) &&*/ ((u8)(rf_data & 0x0F) == (u8)RF_LEARN_KEY)))
    {
        // u8 loose_flag = 0;
        // printf("enter rf learn\n");
        learning_flag = 1;
        u8 ret = rf_learn();
        if ((ret == 0) || (2 == ret))
        {
            // 如果学习成功或是学习超时，都结束学习
            learning_flag = 0;
        }

        return NO_KEY; // 学习期间，只能返回无效键值，避免其他功能函数识别了该键值
    }
    // if (解码值 != 记录的地址码)
    // 如果地址不匹配
    // else if ((rf_data >> 4) != 0x9BFA4)
    else if ((rf_data >> 4) != rf_addr)
    {
#if FILTER_TIME_MAX
        code[0] = 0xFFFFFFFF;
#endif
        return NO_KEY;
    }

    // printf("key == %d\n", (int16_t)(rf_data & 0x0F));

#if FILTER_TIME_MAX
    // 如果能运行到这里，说明地址匹配且收到了有效的数据
    if (code[0] == (u32)0xFFFFFFFF)
    {
        // printf("rf  press\n");

        /*
        // 将数组中的元素全部填充为接收到的有效数据：
        code[0] = 解码值;
        code[1] = 解码值;
        code[2] = 解码值;
        code[3] = 解码值;
        */
        for (u8 i = 0; i < ARRAY_SIZE(code); i++)
        {
            code[i] = rf_data;
        }
    }
#endif

    rf_data = (u32)0xFFFFFFFF; // 清零

    return code[0] & 0x0F; // 返回键值
}

// 根据当前的任务和按键键值，返回对应的事件（事件号）
// 目前还没有加入根据不同任务执行不同的处理的功能
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

    // printf("-----------------function  %s %d\n", __FUNCTION__, __LINE__);
    // printf("key->value %u \nkey->event %d \n", key->value, key->event);

    for (i = 0; i < RF_KEY_NUM_MAX; i++)
    {
        if (key->value == rf_key_table[i][0])
        {
            return rf_key_table[i][1 + key->event];
        }
    }

    return KEY_NULL; // 返回无效的事件
}

/**
 * @brief 学习时对应的功能(检测到学习按键按下才进入)
 *          每隔38ms左右调用一次
 * 
 * @return u8 0--学习成功
 *            1--未学习成功，还在学习期间
 *            2--超时，未学习成功
 */
// 学习的总时间，在学习总时间内若长按学习按键的时间大于长按学习的有效时间，则认为有学习事件 
#define RF_LEARN_TOTAL_TIME (5000) // 学习的总时间, 单位：ms
#define RF_LEARN_TOTAL_TIME_CNT (RF_LEARN_TOTAL_TIME / __RFKEY_SCAN_TIME) // 在检测学习时，学习总时间的计数(等于 总时间/扫描时间)
#define RF_LEARN_VALID_TIME (1000) // 长按学习的有效时间（长按多少秒视为有效学习）
#define RF_LEARN_VALID_TIME_CNT (RF_LEARN_VALID_TIME / __RFKEY_SCAN_TIME) // 在检测学习时，学习按键按下的计数（等于 长按学习的有效时间 / 扫描时间）

#define RF_LEARN_LOOSE_TIME (1000) // 在学习期间，判定为松手所需的时间，单位：ms
#define RF_LEARN_LOOSE_TIME_CNT (RF_LEARN_LOOSE_TIME / __RFKEY_SCAN_TIME) // 在检测学习时，判定为松手的计数（等于 判定为松手所需的时间 / 扫描时间）
u8 rf_learn(void)
{
    // 如果已经检测到学习按键长按，推送消息，让线程保存遥控器的地址
    // 如果5s内没有检测到学习按键的长按，自动退出
    static u16 rf_learn_key_cnt = 0; // 计数值，存放检测到学习按键的次数
    static u16 loose_cnt = 0;        // 松手计数
    static u16 delay_cnt = 0;        // 记录时间，如果超过了时间且松手，才退出学习
    static u32 tmp = (u32)0xFFFFFFFF; // 临时存放数据

    delay_cnt++;
    // printf("[] %s, %d\n", __FUNCTION__, __LINE__);

    // 如果是学习按键（按键键值一致）
    // 5s内按下的学习按键均视为有效（131 * 38 约为 5000ms）
    // if (((rf_data & 0x0F) == RF_LEARN_KEY ) && delay_cnt <= 131) 
    if (((rf_data & 0x0F) == RF_LEARN_KEY ) && delay_cnt <= RF_LEARN_TOTAL_TIME_CNT) 
    {
        tmp = rf_data;
        rf_data = (u32)0xFFFFFFFF; // 清零
        loose_cnt = 0;             // 清除松手计数
        rf_learn_key_cnt++;
        // printf("rf_learn_key_cnt++\n");
    }
    // 如果一直按着按键，直到松手，才退出
    else if (rf_data == (u32)0xFFFFFFFF)
    {
        loose_cnt++;
        // printf("loose_cnt++\n");
    }

    // if (delay_cnt >= 131 && loose_cnt >= 25) // 必须要确定是松手，才能退出学习操作（这里松手约1s左右）
    if (delay_cnt >= RF_LEARN_TOTAL_TIME_CNT && loose_cnt >= RF_LEARN_LOOSE_TIME_CNT) // 必须要确定是松手，才能退出学习操作（这里松手约1s左右）
    {
        loose_cnt = 0;
        delay_cnt = 0;

        if (rf_learn_key_cnt >= RF_LEARN_VALID_TIME_CNT) // 长按学习按键约1s
        {
            // 退出学习操作
            rf_learn_key_cnt = 0;
            // 传递事件和包含遥控器地址的变量
            tmp >>= 4;
            os_taskq_post_msg("rf_decode", 2, KEY_RF_LEARN, (int)tmp); 
            printf("-----------------------rf learn\n");
            return 0; // 表示学习成功
        }
        else
        {
            printf("---------------------------learn time out\n");
            return 2; // 超时，未学习成功
        }
    }

    return 1; // 表示未学习成功，还在学习期间
}