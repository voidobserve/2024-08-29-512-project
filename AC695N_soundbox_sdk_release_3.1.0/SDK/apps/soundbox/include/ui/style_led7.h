#ifndef UI_STYLE_LED7_H
#define UI_STYLE_LED7_H

#include "ui/ui_common.h"
#include "ui/led7/led7_driver.h"
#include "ui/lcd_seg/lcd_seg3x9_driver.h"

// 定义主界面的序号， 每个主界面对应一个序号
// 调用ui_set_main_menu()设置和显示主界面时，
// 程序会找到主界面序号对应的、存放了界面配置数据的变量 const struct ui_dis_api
// 并且，变量要在 static const struct ui_dis_api *ui_dis_main[] 中注册好
enum ui_menu_main
{
    UI_MENU_MAIN_NULL = 0,
    UI_RTC_MENU_MAIN,
    UI_MUSIC_MENU_MAIN,
    UI_AUX_MENU_MAIN,
    UI_BT_MENU_MAIN,
    UI_RECORD_MENU_MAIN,
    UI_FM_MENU_MAIN,
    UI_PC_MENU_MAIN,
    UI_IDLE_MENU_MAIN,

    UI_USERS_LED_MENU,
};

// 定义子界面对应的序号，方便在子界面中进行处理
// 例如：ui_bt_user()函数中，->switch(序号)，再进行调用对应的函数进行处理
// 触发方式： ui_set_tmp_menu() ，参数填对应的序号
enum
{
    MENU_POWER_UP = 1,
    MENU_WAIT,
    MENU_BT,
    MENU_PC,
    MENU_PC_VOL_UP,
    MENU_PC_VOL_DOWN,
    MENU_AUX,
    MENU_ALM_UP,

    MENU_SHOW_STRING,
    MENU_MAIN_VOL,
    MENU_SET_EQ,
    MENU_SET_PLAY_MODE,

    MENU_PLAY_TIME,
    MENU_FILENUM,
    MENU_INPUT_NUMBER,
    MENU_MUSIC_PAUSE,
    MENU_MUSIC_REPEATMODE,

    MENU_FM_MAIN,
    MENU_FM_DISP_FRE,
    MENU_FM_SET_FRE,
    MENU_FM_STATION,
    MENU_IR_FM_SET_FRE,

    MENU_RTC_SET,
    MENU_RTC_PWD,
    MENU_ALM_SET,

    MENU_BT_SEARCH_DEVICE,
    MENU_BT_CONNECT_DEVICE,
    MENU_BT_DEVICE_ADD,
    MENU_BT_DEVICE_NAME,
    MENU_RECODE_MAIN,
    MENU_RECODE_ERR,
    MENU_POWER,
    MENU_LIST_DISPLAY,

    MENU_LED0,
    MENU_LED1,

    MENU_RECORD, // 
    // =============================================
    // 自定义的子菜单序号:
    MENU_SHOW_NUM_1,
    MENU_SHOW_NUM_2,


    // =============================================
    MENU_SEC_REFRESH = 0x80, // 
    MENU_REFRESH,

    // =============================================

    // =============================================
    MENU_MAIN = 0xff,
};

//=================================================================================//
//                        			UI 配置数据结构                    			   //
//=================================================================================//
struct ui_dis_api
{
    int ui;
    void *(*open)(void *hd);
    void (*ui_main)(void *hd, void *private);
    int (*ui_user)(void *hd, void *private, int menu, int arg);
    void (*close)(void *hd, void *private);
};

typedef struct _LCD_DISP_API
{
    void (*clear)(void);
    void (*setXY)(u32 x, u32 y);
    void (*FlashChar)(u32);
    void (*Clear_FlashChar)(u32);
    void (*show_string)(u8 *);
    void (*show_char)(u8);
    void (*show_number)(u8);
    void (*show_icon)(u32);
    void (*flash_icon)(u32);
    void (*clear_icon)(u32);
    void (*show_pic)(u32);
    void (*hide_pic)(u32);
    void (*lock)(u32);
} LCD_API;

extern const struct ui_dis_api bt_main;
extern const struct ui_dis_api fm_main;
extern const struct ui_dis_api music_main;
extern const struct ui_dis_api record_main;
extern const struct ui_dis_api rtc_main;
extern const struct ui_dis_api pc_main;
extern const struct ui_dis_api linein_main;
extern const struct ui_dis_api idle_main;

#endif
