
#include "app_config.h"
#include "system/includes.h"
#include "asm/charge.h"
#include "app_power_manage.h"
#include "update.h"
#include "app_main.h"
#include "app_charge.h"
#include "chgbox_ctrl.h"
#include "update_loader_download.h"


extern void setup_arch();
extern int audio_dec_init();
extern int audio_enc_init();

static void do_initcall()
{
    __do_initcall(initcall);
}

static void do_early_initcall()
{
    __do_initcall(early_initcall);
}

static void do_late_initcall()
{
    __do_initcall(late_initcall);
}

static void do_platform_initcall()
{
    __do_initcall(platform_initcall);
}

static void do_module_initcall()
{
    __do_initcall(module_initcall);
}

void __attribute__((weak)) board_init()
{

}
void __attribute__((weak)) board_early_init()
{

}

int eSystemConfirmStopStatus(void)
{
    /* 系统进入在未来时间里，无任务超时唤醒，可根据用户选择系统停止，或者系统定时唤醒(100ms) */
    //1:Endless Sleep
    //0:100 ms wakeup
    if (get_charge_full_flag()) {
        log_i("Endless Sleep");
        power_set_soft_poweroff();
        return 1;
    } else {
        log_i("100 ms wakeup");
        return 0;
    }

}

static void check_power_on_key(void)
{
    u32 delay_10ms_cnt = 0;
    u32 delay_10msp_cnt = 0;

    while (1) {
        clr_wdt();
        os_time_dly(2);

        extern u8 get_power_on_status(void);
        if (get_power_on_status()) {
            putchar('+');
            delay_10msp_cnt = 0;
            delay_10ms_cnt++;
            if (delay_10ms_cnt > 70) {
                return;
            }
        } else {
            putchar('-');
            delay_10ms_cnt = 0;

            delay_10msp_cnt++;
            if (delay_10msp_cnt > 20) {
                puts("enter softpoweroff\n");
                power_set_soft_poweroff();
            }
        }
    }
}

static void app_init()
{
    int update;

    do_early_initcall();
    do_platform_initcall();

    board_init();

    do_initcall();

    do_module_initcall();
    do_late_initcall();


    audio_enc_init();
    audio_dec_init();

    if (!UPDATE_SUPPORT_DEV_IS_NULL()) {
        update = update_result_deal();
    }

    app_var.play_poweron_tone = 1;

    if (!get_charge_online_flag()) {
        check_power_on_voltage();

#if TCFG_POWER_ON_NEED_KEY
        /*充电拔出,CPU软件复位, 不检测按键，直接开机*/
#if TCFG_CHARGE_OFF_POWERON_NE
        if ((!update && cpu_reset_by_soft()) || is_ldo5v_wakeup()) {
#else
        if (!update && cpu_reset_by_soft()) {
#endif
            app_var.play_poweron_tone = 0;
        } else {
            check_power_on_key();
        }
#endif
    }

#if (TCFG_MC_BIAS_AUTO_ADJUST == MC_BIAS_ADJUST_POWER_ON)
    extern u8 power_reset_src;
    u8 por_flag = 0;
    u8 cur_por_flag = 0;
    /*
     *1.update
     *2.power_on_reset(BIT0:上电复位)
     *3.pin reset(BIT4:长按复位)
     */
    if (update || (power_reset_src & BIT(0)) || (power_reset_src & BIT(4))) {
        //log_info("reset_flag:0x%x",power_reset_src);
        cur_por_flag = 0xA5;
    }
    int ret = syscfg_read(CFG_POR_FLAG, &por_flag, 1);
    if ((cur_por_flag == 0xA5) && (por_flag != cur_por_flag)) {
        //log_info("update POR flag");
        ret = syscfg_write(CFG_POR_FLAG, &cur_por_flag, 1);
    }
#endif

#if (TCFG_CHARGE_ENABLE && TCFG_CHARGE_POWERON_ENABLE)
    if (is_ldo5v_wakeup()) { //LDO5V唤醒
        extern u8 get_charge_online_flag(void);
        if (get_charge_online_flag()) { //关机时，充电插入

        } else { //关机时，充电拔出
            power_set_soft_poweroff();
        }
    }
#endif

#if(TCFG_CHARGE_BOX_ENABLE)
    /* clock_add_set(CHARGE_BOX_CLK); */
    chgbox_init_app();
#endif
}

static void app_task_handler(void *p)
{
    app_init();
    app_main();

    printf("test demo \n");
}

__attribute__((used)) int *__errno()
{
    static int err;
    return &err;
}

int main()
{


#if(CONFIG_CPU_BR25)

#if (TCFG_DEC2TWS_ENABLE ||RECORDER_MIX_EN || TCFG_DRC_ENABLE || TCFG_USER_BLE_ENABLE || TCFG_DEC_APE_ENABLE || TCFG_DEC_FLAC_ENABLE || TCFG_DEC_DTS_ENABLE || TCFG_USER_EMITTER_ENABLE)
    clock_set_sfc_max_freq(100 * 1000000, 100 * 1000000);
#else

#if ((TCFG_AEC_ENABLE) && (TCFG_USER_TWS_ENABLE))
    clock_set_sfc_max_freq(80 * 1000000, 80 * 1000000);
#else
    clock_set_sfc_max_freq(64 * 1000000, 64 * 1000000);
#endif

#endif

#endif

    wdt_close();

    os_init();

    setup_arch();

    board_early_init();

    task_create(app_task_handler, NULL, "app_core");

    os_start();

    local_irq_enable();

    while (1) {
        asm("idle");
    }

    return 0;
}

#include "system/includes.h"
#include "app_config.h"
#include "asm/uart_dev.h"

/*
 *串口导出数据配置
 *注意IO口设置不要和普通log输出uart冲突
 */
#define MOD_UART_TX_PORT			IO_PORTB_01  /*数据导出发送IO*/
#define MOD_UART_RX_PORT			IO_PORTB_00  
#define MOD_UART_BAUDRATE			460800       /*数据导出波特率, 和接收端设置一致*/
#define MOD_UART_TIMEOUT            20           /*接收超时时间(ms)*/


static u8 uart_module_cbuf[512] __attribute__((aligned(4)));  // 接收缓存, 用于SDK提供的接口对应使用的硬件缓存
static u8 uart_module_rxbuf[512] __attribute__((aligned(4))); // 应用层接收缓存, 用于提取与处理数据
static u8 uart_module_txbuf[128] __attribute__((aligned(4))); // 发送缓存, 发送时会将发送内容写入缓存再发送
static const uart_bus_t *uart_bus = NULL;


void uart_module_event_deal(u8 *buff, u16 len)
{
    // 此处数据处理
}

/**
 * @brief uart串口接收事件响应
 * @note 获取完成的整个buff在这里处理
 * @note 使用另一个缓存 uart_module_rxbuf[] 直接复制DMA中的数据, 使原本的DMA缓存的正常接收工作不影响之后速度慢的识别和处理
*/
void uart_module_event_handler(struct sys_event *e)
{
    const uart_bus_t *_uart_bus;
    u32 uart_rxcnt = 0;

    if (!strcmp(e->arg, DEVICE_EVENT_FROM_UART_RX_OVERFLOW)) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            _uart_bus = (const uart_bus_t *)e->u.dev.value;

            // 获取数据包(uart_module_rxbuf)及长度(uart_rxcnt)
            uart_rxcnt = _uart_bus->read(uart_module_rxbuf, sizeof(uart_module_rxbuf), 0);

            // 此处数据处理
            uart_module_event_deal(uart_module_rxbuf, uart_rxcnt);
        }
    }
    else if (!strcmp(e->arg, DEVICE_EVENT_FROM_UART_RX_OVERFLOW)) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            _uart_bus = (const uart_bus_t *)e->u.dev.value;

            // 获取数据包(uart_module_rxbuf)及长度(uart_rxcnt)
            uart_rxcnt = _uart_bus->read(uart_module_rxbuf, sizeof(uart_module_rxbuf), 0);

            // 此处数据处理
            uart_module_event_deal(uart_module_rxbuf, uart_rxcnt);
        }
    }
}

// SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, uart_module_event_handler, 2);
// 将该函数加入SYS_DEVICE_EVENT队列中, 当队列中有新的推送消息时, 会进入这里进行判断


/**
 * @brief uart接收中断, 接收到数据后推送接收事件
*/
static void uart_module_isr_hook(void *arg, u32 status)
{
    const uart_bus_t *ubus = arg;
    struct sys_event e;

    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    
    if (status == UT_RX)  // 数据长度达到 u_arg.frame_length 时所调用中断的参数
    {
        e.type = SYS_DEVICE_EVENT;
        e.arg = DEVICE_EVENT_FROM_UART_RX_OVERFLOW;
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
    }
    if (status == UT_RX_OT)  // 超时达到 u_arg.rx_timeout 时所调用中断的参数
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
void uart_module_tx(u8 *buf, u16 len)
{
    if (uart_bus)
    {
        memcpy(uart_module_txbuf, buf, len);
        uart_bus->write(uart_module_txbuf, len);		// 把数据写到DMA
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
bool uart_module_init(void)
{
    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = MOD_UART_TX_PORT;
    u_arg.rx_pin = MOD_UART_RX_PORT;
    u_arg.rx_cbuf = uart_module_cbuf;
    u_arg.rx_cbuf_size = sizeof(uart_module_cbuf);  // 设置SDK接口使用的缓存大小
    u_arg.frame_length = sizeof(uart_module_cbuf);  // 设置帧大小, 当一次数据包长度达到这个数值时会中断推送UT_RX
    u_arg.rx_timeout = MOD_UART_TIMEOUT;  // 超时时间, 当接收数据后超过这个时间仍未接收到新数据时会中断推送UT_RX_OT
    u_arg.isr_cbfun = uart_module_isr_hook;  // 推送中断函数
    u_arg.baud = MOD_UART_BAUDRATE;
    u_arg.is_9bit = 0;


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
bool uart_module_close(void)
{
    if (uart_bus && !uart_dev_close(uart_bus))
    {
        uart_bus = NULL;
        return true;
    }
    return false;
}
