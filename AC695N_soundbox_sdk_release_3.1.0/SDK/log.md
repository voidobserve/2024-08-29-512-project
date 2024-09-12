
[17:01:09.541]收←◆ 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         setup_arch Sep 12 2024 10:40:45 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
--P3 Reset Source : 0x8
VCM
[Info]: [SDFILE]VM size: 0x20000 @ 0x5e000
[Info]: [SDFILE]disk capacity 2048 KB
last file_addr:5d5c5 49d
end_addr:5e000

49 53 44 55 04 02 15 FF 3B 95 C5 D6 8C DE FF FF 
[Debug]: [SDFILE]sdfile mount succ

[17:01:09.599]收←◆[Info]: [BOARD]Power init : apps/soundbox/board/br23/board_ac695x_demo/board_ac695x_demo.c

[17:01:09.664]收←◆wvdd_lev: 4
vbat_adc_value = 340
vbg_adc_value = 360
[Info]: [USER_CFG]read new cfg bt name config:jl_soundbox_lihui

[Info]: [USER_CFG]bt name config:jl_soundbox_lihui

[Info]: [USER_CFG]read new cfg ble name config:jl_soundbox_ble_lihui

[Info]: [USER_CFG]ble name config:jl_soundbox_ble_lihui

[Info]: [USER_CFG]new cfg tws pair code config:


FF FF 
[Info]: [USER_CFG]tws pair code config:


FF FF 
[Info]: [USER_CFG]read new cfg rf config:7

[Info]: [USER_CFG]rf config:7

[Info]: [USER_CFG]read new cfg ble rf config:7

[Info]: [USER_CFG]ble rf config:10

[Info]: [USER_CFG]aec config:


03 16 17 01 66 66 A6 3F 66 66 A6 3F 66 66 A6 3F 
66 66 A6 3F 00 00 C0 40 00 00 40 40 00 00 48 C2 
00 00 40 41 00 00 00 00 00 00 20 C2 00 00 8C C2 
00 00 80 3F 00 00 8C C2 00 00 40 C0 00 00 80 40 
00 00 A0 3F EC 51 B8 3D 
[Info]: [USER_CFG]aec_cfg mic_gain:3 dac_gain:22
[Info]: [USER_CFG]status_config:

[Info]: [USER_CFG]app audio_config:


01 1F 19 19 
[Info]: [USER_CFG]max vol:31 default vol:25 tone vol:25

[Info]: [USER_CFG]usb_mic_gain: 7

[Info]: [USER_CFG]max vol:31 default vol:15 tone vol:25 vol_sync:61

[Info]: [USER_CFG]warning_tone_v:340 poweroff_tone_v:330

[Info]: [USER_CFG]auto off time config:

[Info]: [USER_CFG]read new cfg auto time 3

[Info]: [USER_CFG]auto_off_time:180

[Info]: [USER_CFG]mac:

9B E2 A0 32 6F 09 
[Info]: [USER_CFG]ble mac:

FA 8E 8C 09 D0 77 
[Info]: [USER_CFG]lrc cfg:

E0 01 90 01 E0 01 8C 00 01 
add sample ch 3
[LED7] led7_init
[Info]: [TEST-UPDATE]testbox msg handle reg:1e3763c

------------timer2_init :119
PRD : 0x2ee0 / 24000000
[17:01:09.766]收←◆
=================addr : ffffffff
[LED7] led7_init
led7_test 58
audio_enc_init
audio_dec_init
[Info]: [AUDIO-DAC]audio_dac_init

cfg DAC_DTB:32767,ret = -252
[Info]: [APP-UPDATE]<--------update_result_deal=0x5a00 0--------->

=================update_param_len:908 404
[Info]: [APP]app_main

[Info]: [APP]APP_POWERON_TASK 

cur --- 1 
new +++ 3 
[Info]: [APP]APP_BT_TASK 


-----edr + ble 's address-----
9B E2 A0 32 6F 09 

9B E2 A0 32 6F 09 
le_support:3 1
le_config:1 1 0 0

[17:01:10.141]收←◆[Debug]: [LBUF]lbuf misalgin : 0x211270 / 0x5da
[Debug]: [BT]-----------------------bt_connction_status_event_handler 3
[Info]: [BT]BT_STATUS_INIT_OK


09 6F 32 A0 E2 9B 
[Debug]: [BT]connect_switch: 0, 0

[Debug]: [BT]is_1t2_connection:0 	 total_conn_dev:0


[17:01:14.763]收←◆[Info]: [APP_AUDIO]VOL_SAVE

