#include "rf_decode.h"

#define RF_TIME_REG JL_TIMER3
#define RF_IRQ_TIME_IDX IRQ_TIME3_IDX
#define RF_RECV_PIN IO_PORTB_05
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
volatile u32 rf_data = 0; // 倒序存放接收到的rf数据

___interrupt
    AT_VOLATILE_RAM_CODE // 放在RAM中运行，提高效率
    void
    timer_rf_isr(void)
{
    local_irq_disable(); // 需要极准确的定时效果时开启
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

    request_irq(RF_IRQ_TIME_IDX, 7, timer_rf_isr, 0);
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

    set_rf_clk();
}