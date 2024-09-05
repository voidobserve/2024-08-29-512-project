#include "dmx512.h"

#include "system/includes.h"
#include "app_config.h"
#include "asm/uart_dev.h"

#include "user_config.h"

/*
 *串口导出数据配置
 *注意IO口设置不要和普通log输出uart冲突
 */
// #define MOD_UART_TX_PORT DMX512_SEND_DATA_PIN /*数据导出发送IO*/
#define MOD_UART_TX_PORT DMX512_SEND_DATA_PIN /*数据导出发送IO*/
// #define MOD_UART_RX_PORT IO_PORTB_00 // 数据接收IO
#define MOD_UART_RX_PORT DMX512_RECV_DATA_PIN // 数据接收IO

#define MOD_UART_BAUDRATE 250000 /*数据导出波特率, 和接收端设置一致*/
#define MOD_UART_TIMEOUT 20      /*接收超时时间(ms)*/

static u8 uart_module_cbuf[1024] __attribute__((aligned(4)));  // 接收缓存, 用于SDK提供的接口对应使用的硬件缓存
static u8 uart_module_rxbuf[1024] __attribute__((aligned(4))); // 应用层接收缓存, 用于提取与处理数据
static u8 uart_module_txbuf[1024] __attribute__((aligned(4))); // 发送缓存, 发送时会将发送内容写入缓存再发送

// dmx512要使用两个串口，一个专门发送，一个专门接收，不能使用同一个串口来进行收发，因为不能同时进行收/发
static const uart_bus_t *uart_bus_rx = NULL; // 专门用于接收的串口
static const uart_bus_t *uart_bus_tx = NULL; // 专门用于发送的串口

// u16 dmx512_tx_cnt = 0; // 存放待发送的dmx512信息包的数据帧个数
u16 dmx512_rx_cnt = 0;              // 存放接收的dmx512信息包的数据帧个数
static u8 dmx512_txbuff[513] = {0}; // 存放待发送的dmx512信息包中的数据
static u8 dmx512_rxbuff[513] = {0}; // 存放接收到的dmx512信息包中的数据，每个数据刚好8位

static void uart_module_isr_hook(void *arg, u32 status); // 声明uart接收中断, 接收到数据后推送接收事件

void dmx512_transpond(void); // 声明，转发dmx512的数据包

/**
 * @brief 串口初始化
 * @note u_arg.tx_pin       配置串口发送IO
 * @note u_arg.rx_pin       配置串口接收IO
 * @note u_arg.rx_cbuf      配置串口接收缓存空间, DMA会将接收到的数据放进这里
 * @note u_arg.rx_cbuf_size 配置串口接收缓存空间的大小, 不能超过u_arg.rx_cbuf的实际大小
 * @note u_arg.frame_length 配置串口在接收数据时, 接收到的数据长度达到这个值时立刻触发中断
 * @note u_arg.rx_timeout   配置串口在接收数据时的超时时长, 在接收到数据后(长度未达到u_arg.frame_length)超过这个时间后立刻触发中断
 * @note u_arg.rx_pin       配置串口接收的中断回调
 * @note u_arg.baud         配置串口通信的波特率
 * @note u_arg.is_9bit      9bit模式
 */
// static bool uart_module_init(void)
// {
// }

/**
 * @brief 串口关闭（关闭专门用于发送数据的串口）
 */
// static bool uart_module_close(void)
// {
// if (uart_bus_tx && !uart_dev_close(uart_bus_tx))
// {
//     uart_bus_tx = NULL;
//     return true;
// }
// return false;
// }

// 关闭专门用于发送数据的串口
static bool dmx512_tx_close(void)
{
    if (uart_bus_tx && !uart_dev_close(uart_bus_tx))
    {
        uart_bus_tx = NULL;
        return true;
    }
    return false;
}

static void uart_module_event_deal(u8 *buff, u16 len)
{
    // 此处数据处理

    // 测试收到的数据是否正确
    // for (u16 i = 0; i < len; i++)
    // {
    //     printf(" slot_%d 0x%x ", i, buff[i]);
    // }
    // printf("\n");

    if (len < 513)
    {
        return;
    }

    // 先转发收到的数据包，再对数据包的信息进行处理
    dmx512_transpond();

#if 0
    if (buff[2] == DMX512_DEVICE_ON)
    {
        // 开灯，引脚输出低电平
        printf("\n device_on \n");
        gpio_set_output_value(USER_LED_PIN, 0);
        gpio_set_direction(USER_LED_PIN, 0);
    }
    else if (buff[2] == DMX512_DEVICE_OFF)
    {
        // 关灯，引脚输出高电平
        printf("\n device_off \n");
        gpio_set_output_value(USER_LED_PIN, 1);
        gpio_set_direction(USER_LED_PIN, 0);
    }
    else
    {
        printf("\n unkown message \n");
    }
#endif
}

#if 1 // uart串口接收事件响应
/**
 * @brief uart串口接收事件响应
 * @note 获取完成的整个buff在这里处理
 * @note 使用另一个缓存 uart_module_rxbuf[] 直接复制DMA中的数据, 使原本的DMA缓存的正常接收工作不影响之后速度慢的识别和处理
 */
void uart_module_event_handler(struct sys_event *e)
{
    const uart_bus_t *_uart_bus;
    u32 uart_rxcnt = 0;

    if (!strcmp(e->arg, DEVICE_EVENT_FROM_UART_RX_OVERFLOW)) // 接收缓冲区溢出，读取数据
    {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE)
        {
            _uart_bus = (const uart_bus_t *)e->u.dev.value;

            // 获取数据包(uart_module_rxbuf)及长度(uart_rxcnt)
            uart_rxcnt = _uart_bus->read(uart_module_rxbuf, sizeof(uart_module_rxbuf), 0);

            // 此处数据处理
            uart_module_event_deal(uart_module_rxbuf, uart_rxcnt);
        }
    }
    else if (!strcmp(e->arg, DEVICE_EVENT_FROM_UART_RX_OUTTIME)) // 超时，从DMA缓存中读取数据
    {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE)
        {
            _uart_bus = (const uart_bus_t *)e->u.dev.value;

            // 获取数据包(uart_module_rxbuf)及长度(uart_rxcnt)
            uart_rxcnt = _uart_bus->read(uart_module_rxbuf, sizeof(uart_module_rxbuf), 0);

            // 此处数据处理
            uart_module_event_deal(uart_module_rxbuf, uart_rxcnt);
        }
    }
}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, uart_module_event_handler, 2);
// 将该函数加入SYS_DEVICE_EVENT队列中, 当队列中有新的推送消息时, 会进入这里进行判断
#endif // uart串口接收事件响应

#if 1 // uart接收中断, 接收到数据后推送接收事件
/**
 * @brief uart接收中断, 接收到数据后推送接收事件
 */
static void uart_module_isr_hook(void *arg, u32 status)
{
    const uart_bus_t *ubus = arg;
    struct sys_event e;

    // 当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量

    if (status == UT_RX) // 数据长度达到 u_arg.frame_length 时所调用中断的参数
    {
        e.type = SYS_DEVICE_EVENT;
        e.arg = DEVICE_EVENT_FROM_UART_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
    }
    if (status == UT_RX_OT) // 超时达到 u_arg.rx_timeout 时所调用中断的参数
    {
        e.type = SYS_DEVICE_EVENT;
        e.arg = DEVICE_EVENT_FROM_UART_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
    }
    if (status == UT_TX)
    {
        /* putchar('T'); */
        /* gpio_change(0); */
    }
}
#endif // uart接收中断, 接收到数据后推送接收事件

/**
 * @brief 发送数据的接口
 */
static void uart_module_tx(u8 *buf, u16 len)
{
    if (uart_bus_tx)
    {
        memcpy(uart_module_txbuf, buf, len);
        uart_bus_tx->write(uart_module_txbuf, len); // 把数据写到DMA
    }
}

// 配置专门用于接收dxm512数据包的串口
bool dmx512_rx_config(void)
{
    struct uart_platform_data_t u_arg = {0};

    u_arg.tx_pin = -1;
    u_arg.rx_pin = MOD_UART_RX_PORT;
    u_arg.rx_cbuf = uart_module_cbuf;
    u_arg.rx_cbuf_size = sizeof(uart_module_cbuf); // 设置SDK接口使用的缓存大小
    u_arg.frame_length = sizeof(uart_module_cbuf); // 设置帧大小, 当一次数据包长度达到这个数值时会中断推送UT_RX
    u_arg.rx_timeout = MOD_UART_TIMEOUT;           // 超时时间, 当接收数据后超过这个时间仍未接收到新数据时会中断推送UT_RX_OT
    u_arg.isr_cbfun = uart_module_isr_hook;        // 推送中断函数
    u_arg.baud = MOD_UART_BAUDRATE;
    u_arg.is_9bit = 1; // 9个数据位

    uart_bus_rx = uart_dev_open(&u_arg);
    if (uart_bus_rx != NULL)
    {
        // gpio_set_pull_up(MOD_UART_RX_PORT, 1); // 上拉
        return true;
    }
    else
    {
        return false;
    }
}

// 配置专门用于发送dmx512数据包的串口
static bool dmx512_tx_config(void)
{
    struct uart_platform_data_t u_arg = {0};

    u_arg.tx_pin = MOD_UART_TX_PORT;
    u_arg.rx_pin = -1;
    u_arg.rx_cbuf = NULL;
    u_arg.rx_cbuf_size = 0;                        // 设置SDK接口使用的缓存大小
    u_arg.frame_length = sizeof(uart_module_cbuf); // 设置帧大小, 当一次数据包长度达到这个数值时会中断推送UT_RX
    u_arg.rx_timeout = MOD_UART_TIMEOUT;           // 超时时间, 当接收数据后超过这个时间仍未接收到新数据时会中断推送UT_RX_OT
    u_arg.isr_cbfun = NULL;                        // 推送中断函数
    u_arg.baud = MOD_UART_BAUDRATE;
    u_arg.is_9bit = 1; // 9个数据位

    uart_bus_tx = uart_dev_open(&u_arg);
    if (uart_bus_tx != NULL)
    {
        gpio_set_hd(MOD_UART_TX_PORT, 1);
        gpio_set_hd0(MOD_UART_TX_PORT, 1);
        return true;
    }
    else
    {
        return false;
    }
}

// 发送dmx512数据包
void dmx512_send_start(void)
{
    // dmx512_txbuff[0] = 0; // 第0个数据帧，为SC(Start Code)，开始代码帧
    // 1. 发送MTBP
    // gpio_set_direction(MOD_UART_TX_PORT, 0); // IO配置为输出模式
    // gpio_set_output_value(MOD_UART_TX_PORT, 1); // IO输出高电平
    // delay_200us();

    // 2. 发送BREAK
    gpio_set_output_value(MOD_UART_TX_PORT, 0); // IO输出低电平
    delay_88us();
    // gpio_set_direction(MOD_UART_TX_PORT, 1);
    // delay_200us();

    // 3. 发送MAB
    // gpio_set_output_value(MOD_UART_TX_PORT, 1); // IO输出高电平
    // delay_8us();

    // 4. 发送SC
    dmx512_tx_config(); // 打开串口（这一步会把IO电平拉高，约31.5us，相当于发送了MAB）
    // // 5. 发送数据
    // for (u16 i = 0; i < dmx512_tx_cnt; i++)
    // {
    uart_module_tx(dmx512_txbuff, 513);
    // }

    // 6. 发送MTBP
    gpio_set_output_value(MOD_UART_TX_PORT, 1); // IO输出高电平
    gpio_set_pull_down(MOD_UART_TX_PORT, 0);    // 关闭下拉
    gpio_set_pull_up(MOD_UART_TX_PORT, 0);      // 关闭上拉
    dmx512_tx_close();                          // 关闭串口
    gpio_set_direction(MOD_UART_TX_PORT, 0);    // IO配置为输出模式(不重新配置为输出模式，在关闭串口后就直接输出高电平，IO为低电平，不会输出高电平)
    delay_200us();
}

// 解析dmx512数据包
// void dmx512_decode_handle(void)
// {
// }

// 转发dmx512数据包
void dmx512_transpond(void)
{
    memcpy(dmx512_txbuff, (u8 *)uart_module_rxbuf + 1, 513); // 将串口接收缓冲区的数据拷贝至dmx512的发送缓冲区（不拷贝第0个数据帧，那是将一段低电平识别成了串口的信号）
    dmx512_send_start();
}



// 定义dmx512数据包中的信息：
#define DMX512_DEVICE_OFF 0xA5 // 0xA5 表示关闭设备
#define DMX512_DEVICE_ON  0x5A // 0x5A 表示开启设备
// 测试用，定义控制位的状态
enum
{
    DMX512_CONTROL_SUB1_OFF = 0x01, // 关闭从机1的灯
    DMX512_CONTROL_SUB2_OFF = 0x02, // 关闭从机2的灯

    DMX512_CONTROL_SUB1_ON = 0x04, // 点亮从机1的灯
    DMX512_CONTROL_SUB2_ON = 0x08, // 点亮从机2的灯
};
// dmx512数据包发送测试
void dmx512_send_test(void)
{
    /*
        控制从机设备的标志位
    */
    static u8 control_flag = DMX512_CONTROL_SUB1_OFF | DMX512_CONTROL_SUB2_OFF;

    if ((DMX512_CONTROL_SUB1_OFF | DMX512_CONTROL_SUB2_OFF) == control_flag)
    {
        dmx512_txbuff[1] = DMX512_DEVICE_OFF;
        dmx512_txbuff[2] = DMX512_DEVICE_ON;
        control_flag = DMX512_CONTROL_SUB1_OFF | DMX512_CONTROL_SUB2_ON;
    }
    else if ((DMX512_CONTROL_SUB1_OFF | DMX512_CONTROL_SUB2_ON) == control_flag)
    {
        dmx512_txbuff[1] = DMX512_DEVICE_ON; 
        dmx512_txbuff[2] = DMX512_DEVICE_OFF; 
        control_flag = DMX512_CONTROL_SUB1_ON | DMX512_CONTROL_SUB2_OFF;
    }
    else if ((DMX512_CONTROL_SUB1_ON | DMX512_CONTROL_SUB2_OFF) == control_flag)
    {
        dmx512_txbuff[1] = DMX512_DEVICE_ON; //
        dmx512_txbuff[2] = DMX512_DEVICE_ON; // 
        control_flag = DMX512_CONTROL_SUB1_ON | DMX512_CONTROL_SUB2_ON;
    }
    else if ((DMX512_CONTROL_SUB1_ON | DMX512_CONTROL_SUB2_ON) == control_flag)
    {
        dmx512_txbuff[1] = DMX512_DEVICE_OFF; //
        dmx512_txbuff[2] = DMX512_DEVICE_OFF; //
        control_flag = DMX512_CONTROL_SUB1_OFF | DMX512_CONTROL_SUB2_OFF;
    }

    dmx512_send_start(); // 发送数据包
}
