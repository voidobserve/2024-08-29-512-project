// 工程中有同名的头文件，不能直接使用
#include "uart.h"

#include "system/includes.h"
#include "app_config.h"
#include "asm/uart_dev.h"

/*
 *串口导出数据配置
 *注意IO口设置不要和普通log输出uart冲突
 */
#define MOD_UART_TX_PORT -1 /*数据导出发送IO*/
#define MOD_UART_RX_PORT IO_PORTA_05 // 数据接收IO
#define MOD_UART_BAUDRATE 250000     /*数据导出波特率, 和接收端设置一致*/
#define MOD_UART_TIMEOUT 20          /*接收超时时间(ms)*/

static u8 uart_module_cbuf[1024] __attribute__((aligned(4)));  // 接收缓存, 用于SDK提供的接口对应使用的硬件缓存
static u8 uart_module_rxbuf[1024] __attribute__((aligned(4))); // 应用层接收缓存, 用于提取与处理数据
static u8 uart_module_txbuf[128] __attribute__((aligned(4)));  // 发送缓存, 发送时会将发送内容写入缓存再发送
static const uart_bus_t *uart_bus = NULL;

static void uart_module_event_deal(u8 *buff, u16 len)
{
    // 此处数据处理
    for (u16 i = 0; i < len; i++)
    {
        printf(" slot_%d 0x%x ", i, buff[i]);
    }
    printf("\n");
}

/**
 * @brief uart串口接收事件响应
 * @note 获取完成的整个buff在这里处理
 * @note 使用另一个缓存 uart_module_rxbuf[] 直接复制DMA中的数据, 使原本的DMA缓存的正常接收工作不影响之后速度慢的识别和处理
 */
static void uart_module_event_handler(struct sys_event *e)
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

/**
 * @brief 发送数据的接口
 */
static void uart_module_tx(u8 *buf, u16 len)
{
    if (uart_bus)
    {
        memcpy(uart_module_txbuf, buf, len);
        uart_bus->write(uart_module_txbuf, len); // 把数据写到DMA
    }
}

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
static bool uart_module_init(void)
{
    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = MOD_UART_TX_PORT;
    u_arg.rx_pin = MOD_UART_RX_PORT;
    u_arg.rx_cbuf = uart_module_cbuf;
    u_arg.rx_cbuf_size = sizeof(uart_module_cbuf); // 设置SDK接口使用的缓存大小
    u_arg.frame_length = sizeof(uart_module_cbuf); // 设置帧大小, 当一次数据包长度达到这个数值时会中断推送UT_RX
    u_arg.rx_timeout = MOD_UART_TIMEOUT;           // 超时时间, 当接收数据后超过这个时间仍未接收到新数据时会中断推送UT_RX_OT
    u_arg.isr_cbfun = uart_module_isr_hook;        // 推送中断函数
    u_arg.baud = MOD_UART_BAUDRATE;
    u_arg.is_9bit = 1;

    uart_bus = uart_dev_open(&u_arg);
    if (uart_bus != NULL)
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

/**
 * @brief 串口关闭
 */
static bool uart_module_close(void)
{
    if (uart_bus && !uart_dev_close(uart_bus))
    {
        uart_bus = NULL;
        return true;
    }
    return false;
}

void user_test_uart_config(void)
{
    uart_module_init();
}

void user_test_uart_close(void)
{
    uart_module_close();
}
