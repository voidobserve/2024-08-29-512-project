#include "rf_decode.h"
#include "key_event_deal.h" // 包含系统的按键事件的相关定义

#define RF_TIME_REG JL_TIMER3         // 使用到的定时器
#define RF_IRQ_TIME_IDX IRQ_TIME3_IDX // 使用到的定时器对应的中断号
#define RF_RECV_PIN RF_DECODE_PIN     // 接收RF信号的引脚

static const u16 timer_div[] = {
    /*0000*/ 1,
    /*0001*/ 4,
    /*0010*/ 16,
    /*0011*/ 64,
    /*0100*/ 2,
    /*0101*/ 8,
    /*0110*/ 32,
    /*0111*/ 128,
    /*1000*/ 256,
    /*1001*/ 4 * 256,
    /*1010*/ 16 * 256,
    /*1011*/ 64 * 256,
    /*1100*/ 2 * 256,
    /*1101*/ 8 * 256,
    /*1110*/ 32 * 256,
    /*1111*/ 128 * 256,
};

static u32 __rf_data = 0xFFFFFFFF; // 存放接收到的rf数据（在定时器中断中使用）
volatile u32 rf_data = 0xFFFFFFFF; // 存放接收到的rf数据（在线程中会读取__rf_data的值来更新）

volatile u32 rf_addr = 0xFFFFFFFF; // 存放遥控器的地址 (后续可能要改成支持多个遥控器，可能要换成数组来存放)
// volatile u8 rf_key = 0;            // 存放遥控器按键的键值

___interrupt
    AT_VOLATILE_RAM_CODE // 放在RAM中运行，提高效率
    void
    timer_rf_isr(void)
{
    local_irq_disable();         // 需要极准确的定时效果时开启
    RF_TIME_REG->CON |= BIT(14); // 清除中断标志

    static bool bit_end;
    static u8 Pluse_H_cnt, Pluse_L_cnt; // 高低电平计数
    static u8 rf_bit_cnt;               // 数据位计数（总共应为24bits）

    static bool f_levelbuf;
    bool flagerror = 0;
    bool flagsuccess = 0;

#define MAX_PULSE 30
#define MIN_PULSE 3     // 最小脉宽不应小于滤波宽度
#define FILTER_CNT 0X07 // 滤波因数 0b_0111
    static u8 filter;
    static bool f_level;

    filter <<= 1;
    // if ((JL_PORTB->IN & BIT(5))) // 检测IO是否为高电平
    if (gpio_read(RF_RECV_PIN))
        filter |= 0x01;

    filter &= FILTER_CNT;
    if (filter == FILTER_CNT)
    {
        f_level = 1; // 高电平
        // JL_PORTB->DIR &= ~BIT(1);
        // JL_PORTB->OUT |= BIT(1);
    }
    else if (filter == 0x00)
    {
        f_level = 0; // 低电平
        // JL_PORTB->DIR &= ~BIT(1);
        // JL_PORTB->OUT &= ~BIT(1);
    }

    if (f_level) // 高电平判断
    {
        if (bit_end) // 一位结束
        {
            bit_end = 0;
            if (Pluse_L_cnt >= MIN_PULSE)
            {
                // __rf_data >>= 1;
                __rf_data <<= 1;
                if (Pluse_H_cnt > Pluse_L_cnt)
                {
                    // __rf_data |= 0x00800000;
                    __rf_data |= 0x01;
                }
                else
                {
                    __rf_data &= ~(0x01);
                }
                Pluse_H_cnt = 0;
            }
            else // 低电平小于150us认为出错
            {
                flagerror = 1;
            }
        }
        else
        {
            f_levelbuf = 1;
            Pluse_H_cnt++;
            if (Pluse_H_cnt >= MAX_PULSE)
            {
                flagerror = 1;
            }
            Pluse_L_cnt = 0;
        }
    }
    else // 低电平判断
    {
        Pluse_L_cnt++;
        if (Pluse_L_cnt >= MAX_PULSE) // 低电平超出最大值
        {
            if (rf_bit_cnt != 25)
            {
                flagerror = 1;
                __rf_data = 0xFFFFFFFF;
                // printf("rf decode err \n");
            }
            else // 解码成功
            {
                __rf_data &= 0xFFFFFF; // 只保留低24位的数据，清除之前的数据残留
                // printf("rf_data:%06x\n", __rf_data);
                // printf("%s , %d\n", __FUNCTION__, __LINE__);
                os_taskq_post_msg("rf_decode", 1, MSG_RF_RECV_FLAG);

                // 存放地址和键值（不能在这里保存地址和键值，会影响学习的操作）
                // rf_addr = __rf_data >> 4;
                // rf_key = __rf_data & 0x0F;

                // printf("%s , %d\n", __FUNCTION__, __LINE__);
                // rf_recv_flag = 1;

                flagsuccess = 1; //
            }
        }
        else
        {
            if (f_levelbuf)
            {
                f_levelbuf = 0;
                Pluse_L_cnt = 0;
                bit_end = 1;
                if (Pluse_H_cnt >= MIN_PULSE)
                {
                    rf_bit_cnt++;
                    if (rf_bit_cnt >= 26)
                    {
                        flagerror = 1;
                    }
                }
                else
                {
                    flagerror = 1;
                }
            }
        }
    }

    if (flagerror || flagsuccess)
    {
        // DEBUG2 = 1;
        rf_bit_cnt = 0;
        Pluse_H_cnt = 0;
        Pluse_L_cnt = 0;
        f_levelbuf = 0;
        bit_end = 0;
    }

    local_irq_enable(); // 需要极准确的定时效果时开启
}

#define APP_RF_TIMER_CLK clk_get("timer") // 12000000L //pll12m
#define RF_TIMER_UNIT_MS 1
#define RF_MAX_TIME_CNT 0x7fff // 0x0080 // 0x07ff //分频准确范围，更具实际情况调整
#define RF_MIN_TIME_CNT 0x100  // 0x0030

void set_rf_clk(void)
{
    u32 prd_cnt;
    u8 index;
    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++)
    {
        prd_cnt = RF_TIMER_UNIT_MS * (APP_RF_TIMER_CLK / 20000) / timer_div[index]; // 配置中断的周期（每隔多久产生一次中断）
        if (prd_cnt > RF_MIN_TIME_CNT && prd_cnt < RF_MAX_TIME_CNT)
        {
            break;
        }
    }

    RF_TIME_REG->CNT = 0;
    RF_TIME_REG->PRD = prd_cnt; // 1ms

    request_irq(RF_IRQ_TIME_IDX, 7, timer_rf_isr, 0); // 7，最高优先级，在时序性要求较高时使用
    // RF_TIME_REG->CON = ((index << 4) | BIT(3) | BIT(1) | BIT(0));//选择osc时钟
    RF_TIME_REG->CON = ((index << 4) | BIT(3) | BIT(0)); // 选择osc时钟
    // JL_IOMAP->CON0 |= BIT(21);//这里已选了timer5,时钟源选io信号里的pll_12m,不是所有的timer都可选pll,修改请看文档
    // RF_TIME_REG->CON = ((index << 4) | BIT(2) | BIT(1) | BIT(0));

    // printf("PRD : 0x%x / %d", RF_TIME_REG->PRD, clk_get("timer"));
}

void rf_config(void)
{
    // JL_IR->RFLT_CON = 0;
    // /* JL_IR->RFLT_CON |= BIT(7);		//256 div */
    // /* JL_IR->RFLT_CON |= BIT(3);		//osc 24m */
    // JL_IR->RFLT_CON |= BIT(7) | BIT(4);		//512 div
    // JL_IR->RFLT_CON |= BIT(3) | BIT(2);		//PLL_48m（兼容省晶振）
    // JL_IR->RFLT_CON |= BIT(0);		//irflt enable

    gpio_set_pull_up(RF_RECV_PIN, 0);   // 关闭上拉
    gpio_set_pull_down(RF_RECV_PIN, 0); // 关闭下拉
    gpio_set_die(RF_RECV_PIN, 1);       // 普通输入、数字输入
    gpio_set_direction(RF_RECV_PIN, 1); // 输入模式

    set_rf_clk(); // 配置扫描rf信号的定时器
}

// 处理rf信号的线程/任务
void rf_decode_task_handler(void *p)
{
    u8 ret = 0;
    int msg[16] = {0};

    // syscfg_read() 和 syscfg_write() 不能早于这些模块的初始化前 就调用
    if (syscfg_read(CFG_USER_RF_ADDR, &rf_addr, sizeof(rf_addr)) != sizeof(rf_addr))
    {
        // 如果是第一次上电或是读取到空的数据，给一初始值并写回
        rf_addr = 0xFFFFFFFF;
        syscfg_write(CFG_USER_RF_ADDR, &rf_addr, sizeof(rf_addr));
    }
    printf("=================addr : %x\n", rf_addr);

    rf_config();

    while (1)
    {
        // printf("%s , %d\n", __FUNCTION__, __LINE__);
        ret = os_taskq_pend("rf_decode", msg, ARRAY_SIZE(msg));
        // printf("%s , %d\n", __FUNCTION__, __LINE__);
        // printf("ret:  %d\n", (int)ret);
        if (ret != OS_TASKQ)
        {
            continue;
        }
        if (msg[0] != Q_MSG)
        {
            continue;
        }

        switch (msg[1])
        {
        case MSG_RF_RECV_FLAG:
            // printf("get msg rf_recv\n");
            rf_data = __rf_data; // 收到消息后，更新一次收到的数据
            break;

        case KEY_RF_LEARN:    // 如果rf学习事件触发
            rf_addr = msg[2]; // 存放学习到的遥控器的地址
            // for (u16 i = 0 ; i < ARRAY_SIZE(msg); i++)
            // {
            //     printf("msg[%d]: 0x-%x \n", (int)i, msg[i]);
            // }
            syscfg_write(CFG_USER_RF_ADDR, &rf_addr, sizeof(rf_addr));
            printf("=================addr : %x\n", rf_addr);
            // syscfg_read(CFG_USER_RF_ADDR, &rf_addr, sizeof(rf_addr)); // 测试读写是否成功
            // printf("=================addr : %x\n", rf_addr);
            break;

        case KEY_RF_NUM_1_CLICK:
            printf("KEY_RF_NUM_1_CLICK\n");
            break;
        case KEY_RF_NUM_1_LONG:
            printf("KEY_RF_NUM_1_LONG\n");
            break;
        case KEY_RF_NUM_1_HOLD:
            printf("KEY_RF_NUM_1_HOLD\n");
            break;
        case KEY_RF_NUM_1_LOOSE:
            printf("KEY_RF_NUM_1_LOOSE\n");
            break;
        case KEY_RF_NUM_1_DOUBLE_CLICK:
            printf("KEY_RF_NUM_1_DOUBLE_CLICK\n");
            break;
        case KEY_RF_NUM_1_TRIPLE_CLICK:
            printf("KEY_RF_NUM_1_TRIPLE_CLICK\n");
            break;

        case KEY_RF_NUM_2_CLICK:
            printf("KEY_RF_NUM_2_CLICK\n");
            break;
        case KEY_RF_NUM_2_LONG:
            printf("KEY_RF_NUM_2_LONG\n");
            break;
        case KEY_RF_NUM_2_HOLD:
            printf("KEY_RF_NUM_2_HOLD\n");
            break;
        case KEY_RF_NUM_2_LOOSE:
            printf("KEY_RF_NUM_2_LOOSE\n");
            break;
        case KEY_RF_NUM_2_DOUBLE_CLICK:
            printf("KEY_RF_NUM_2_DOUBLE_CLICK\n");
            break;
        case KEY_RF_NUM_2_TRIPLE_CLICK:
            printf("KEY_RF_NUM_2_TRIPLE_CLICK\n");
            break;

        case KEY_RF_NUM_3_CLICK:
            printf("KEY_RF_NUM_3_CLICK\n");
            break;
        case KEY_RF_NUM_3_LONG:
            printf("KEY_RF_NUM_3_LONG\n");
            break;
        case KEY_RF_NUM_3_HOLD:
            printf("KEY_RF_NUM_3_HOLD\n");
            break;
        case KEY_RF_NUM_3_LOOSE:
            printf("KEY_RF_NUM_3_LOOSE\n");
            break;
        case KEY_RF_NUM_3_DOUBLE_CLICK:
            printf("KEY_RF_NUM_3_DOUBLE_CLICK\n");
            break;
        case KEY_RF_NUM_3_TRIPLE_CLICK:
            printf("KEY_RF_NUM_3_TRIPLE_CLICK\n");
            break;

        case KEY_RF_NUM_4_CLICK:
            printf("KEY_RF_NUM_4_CLICK\n");
            break;
        case KEY_RF_NUM_4_LONG:
            printf("KEY_RF_NUM_4_LONG\n");
            break;
        case KEY_RF_NUM_4_HOLD:
            printf("KEY_RF_NUM_4_HOLD\n");
            break;
        case KEY_RF_NUM_4_LOOSE:
            printf("KEY_RF_NUM_4_LOOSE\n");
            break;
        case KEY_RF_NUM_4_DOUBLE_CLICK:
            printf("KEY_RF_NUM_4_DOUBLE_CLICK\n");
            break;
        case KEY_RF_NUM_4_TRIPLE_CLICK:
            printf("KEY_RF_NUM_4_TRIPLE_CLICK\n");
            break;

        case KEY_RF_NUM_5_CLICK:
            printf("KEY_RF_NUM_5_CLICK\n");
            break;
        case KEY_RF_NUM_5_LONG:
            printf("KEY_RF_NUM_5_LONG\n");
            break;
        case KEY_RF_NUM_5_HOLD:
            printf("KEY_RF_NUM_5_HOLD\n");
            break;
        case KEY_RF_NUM_5_LOOSE:
            printf("KEY_RF_NUM_5_LOOSE\n");
            break;
        case KEY_RF_NUM_5_DOUBLE_CLICK:
            printf("KEY_RF_NUM_5_DOUBLE_CLICK\n");
            break;
        case KEY_RF_NUM_5_TRIPLE_CLICK:
            printf("KEY_RF_NUM_5_TRIPLE_CLICK\n");
            break;

        case KEY_RF_NUM_6_CLICK:
            printf("KEY_RF_NUM_6_CLICK\n");
            break;
        case KEY_RF_NUM_6_LONG:
            printf("KEY_RF_NUM_6_LONG\n");
            break;
        case KEY_RF_NUM_6_HOLD:
            printf("KEY_RF_NUM_6_HOLD\n");
            break;
        case KEY_RF_NUM_6_LOOSE:
            printf("KEY_RF_NUM_6_LOOSE\n");
            break;
        case KEY_RF_NUM_6_DOUBLE_CLICK:
            printf("KEY_RF_NUM_6_DOUBLE_CLICK\n");
            break;
        case KEY_RF_NUM_6_TRIPLE_CLICK:
            printf("KEY_RF_NUM_6_TRIPLE_CLICK\n");
            break;

        default:
            break;
        }

        // os_time_dly(10);
    }
}

void rf_decode_task_init(void)
{
    task_create(rf_decode_task_handler, NULL, "rf_decode");
}
module_initcall(rf_decode_task_init); // 将函数注册到初始化后，自动调用
