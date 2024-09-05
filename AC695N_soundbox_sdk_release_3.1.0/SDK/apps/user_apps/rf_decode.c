#include "rf_decode.h"

#define RF_TIME_REG JL_TIMER3         // 使用到的定时器
#define RF_IRQ_TIME_IDX IRQ_TIME3_IDX // 使用到的定时器对应的中断号
#define RF_RECV_PIN RF_DECODE_PIN     // 接收RF信号的引脚
// #define RF_FINISH_WAIT 1

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

// static u32 rf_data = 0; // 倒序存放接收到的rf数据
volatile u32 rf_data = 0;     // 倒序存放接收到的rf数据

volatile u32 rf_addr = 0; // 存放遥控器的地址 (后续可能要改成支持多个遥控器，可能要换成数组来存放)
volatile u8 rf_key = 0; // 

// volatile u8 rf_recv_flag = 0; // 标志位，表示是否完成一次rf数据的接收，0--未接收，1--接收


u8 rf_get_key_value(void);
//按键驱动扫描参数列表
struct key_driver_para rfkey_scan_para = {
    .scan_time 	  	  = 10,				//按键扫描频率, 单位: ms
    .last_key 		  = NO_KEY,  		//上一次get_value按键值, 初始化为NO_KEY;
    .filter_time  	  = 2,				//按键消抖延时;
    .long_time 		  = 75,  			//按键判定长按数量
    .hold_time 		  = (75 + 15),  	//按键判定HOLD数量
    .click_delay_time = 20,				//按键被抬起后等待连击延时数量
    .key_type		  = KEY_DRIVER_TYPE_AD,
    .get_value 		  = rf_get_key_value,
};
u8 rf_get_key_value(void)
{
    u8 i;
    u16 ad_data;

    ad_data = adc_get_value(__this->ad_channel);
    /* printf("ad_value = %d \n", ad_data); */
    for (i = 0; i < ADKEY_MAX_NUM; i++) {
        if ((ad_data <= __this->ad_value[i]) && (__this->ad_value[i] < 0x3ffL)) {
            return __this->key_value[i];
        }
    }
    return NO_KEY;
}

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
                rf_data >>= 1;
                // rf_data <<= 1;
                if (Pluse_H_cnt > Pluse_L_cnt)
                {
                    rf_data |= 0x00800000;
                    // rf_data |= 0x01;
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
                // printf("rf decode err \n");
            }
            else // 解码成功
            {

                printf("rf_data:%06x\n", rf_data);
                printf("%s , %d\n", __FUNCTION__, __LINE__);
                os_taskq_post_msg("rf_decode", 1, MSG_RF_RECV_FLAG);

                // 存放地址和键值


                printf("%s , %d\n", __FUNCTION__, __LINE__);
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

void rf_decode_task_handler(void *p)
{
    u8 ret = 0;
    int msg[16] = {0};

    rf_config();



    while (1)
    {
        printf("%s , %d\n", __FUNCTION__, __LINE__);
        ret = os_taskq_pend("rf_decode", msg, ARRAY_SIZE(msg));
        printf("%s , %d\n", __FUNCTION__, __LINE__);
        // printf("ret:  %d\n", (int)ret);
        if (ret != OS_TASKQ)
        {
            continue;
        }
        if (msg[0] != Q_MSG)
        {
            continue;
        }

        switch (ret)
        {
        case MSG_RF_RECV_FLAG:
            printf("get msg rf_recv\n");
            break;

        default:
            break;
        }

        // os_time_dly(10);
    }
}

// void fun(void *p)
// {

//     while (1)
//     {
//         if (rf_recv_flag)
//         {
//             rf_recv_flag = 0;
//             printf("%s , %d\n", __FUNCTION__, __LINE__);
//             os_taskq_post_msg("rf_decode", 1, MSG_RF_RECV_FLAG);
//             printf("%s , %d\n", __FUNCTION__, __LINE__);
//         }

//         os_time_dly(1);
//     }
// }

