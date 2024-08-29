/*********************************************************************************************
    *   Filename        : lib_driver_config.c

    *   Description     : Optimized Code & RAM (编译优化配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-18 14:58

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "system/includes.h"
#include "media/includes.h"
#include "audio_config.h"


#if (TCFG_AUDIO_DECODER_OCCUPY_TRACE)
const u8 audio_decoder_occupy_trace_enable = 1;
const u8 audio_decoder_occupy_trace_dump = 0;
#else
const u8 audio_decoder_occupy_trace_enable = 0;
const u8 audio_decoder_occupy_trace_dump = 0;
#endif/*TCFG_AUDIO_DECODER_OCCUPY_TRACE*/

/*************************************************************************************************/
const int config_audio_eq_en = 0
#if TCFG_EQ_ENABLE
                               | EQ_EN         //eq使能
#if TCFG_EQ_ONLINE_ENABLE
                               | EQ_ONLINE_EN  //在线调试使能
                               | EQ_ONLINE_FILE_SAVE //eq在线调试保存到vm使能
#endif/*TCFG_AUDIO_OUT_EQ_ENABLE*/
#if TCFG_USE_EQ_FILE
                               | EQ_FILE_EN   //使用eq_cfg_hw.bin文件效果
                               //|EQ_FILE_SWITCH_EN   //使能eq_cfg_hw.bin多文件切换，对应旧版config_audio_eq_file_sw_en
#endif/*TCFG_USE_EQ_FILE*/
#if TCFG_AUDIO_OUT_EQ_ENABLE
                               | EQ_HIGH_BASS_EN //高低音接口使能
                               | EQ_HIGH_BASS_FADE_EN  //高低音接口数据更新使用淡入淡出,配合config_audio_eq_fade_step步进使用
#endif/*TCFG_AUDIO_OUT_EQ_ENABLE*/
#if (RCSP_ADV_EN)&&(JL_EARPHONE_APP_EN)&&(TCFG_DRC_ENABLE == 0)
                               /* |EQ_FILTER_COEFF_LIMITER_ZERO_EN */
#endif
#ifndef CONFIG_SOUNDBOX_FLASH_256K
                               | EQ_HW_UPDATE_COEFF_ONLY_EN
                               | EQ_HW_LR_ALONE
#endif/* CONFIG_SOUNDBOX_FLASH_256K */
#if defined(EQ_CORE_V1) && TCFG_DRC_ENABLE
                               | EQ_SUPPORT_MULIT_CHANNEL_EN //eq是否支持多声道（3~8） 打开:支持  否则：仅支持1~2声道*/
                               | EQ_HW_CROSSOVER_TYPE0_EN //硬件分频器用序列进序列出
                               //|EQ_HW_CROSSOVER_TYPE1_EN  //硬件分频器使用块出方式，会增加mem(该方式仅支持单声道处理)
#endif/*(EQ_CORE_V1) && TCFG_DRC_ENABLE*/
#if defined(TCFG_EQ_DIVIDE_ENABLE) && TCFG_EQ_DIVIDE_ENABLE
                               | EQ_LR_DIVIDE_EN
#endif/*TCFG_EQ_DIVIDE_ENABLE*/
                               | EQ_SUPPORT_32BIT_SYNC_EN //32bit同步方式eq使能,eq_lite_en后，该宏需使能
                               | EQ_DRC_PROCESS_SPLIT

#if defined(EQ_LITE_CODE) && EQ_LITE_CODE
                               | EQ_LITE_VER_EN //不支持异步，不支持默认效果切换接口，仅支持文件解析
#if TCFG_DRC_ENABLE
                               | EQ_SUPPORT_32BIT_SYNC_EN //32bit同步方式eq使能,eq_lite_en后，该宏需使能
#endif/*TCFG_DRC_ENABLE*/
#endif/* EQ_LITE_CODE */
#if defined(SOUND_TRACK_2_P_X_CH_CONFIG) &&SOUND_TRACK_2_P_X_CH_CONFIG
                               | EQ_SUPPORT_32BIT_SYNC_EN //32bit同步方式eq使能
#endif

                               //|EQ_FILTER_COEFF_FADE_EN//默认系数切换更新时使用淡入淡出
#endif/* TCFG_EQ_ENABLE */
                               | 0; //end

const float config_audio_eq_fade_step = 0.1f;//播歌高低音增益调节步进

const int const_eq_debug = 1;
const int AUDIO_EQ_CLEAR_MEM_BY_MUTE_TIME_MS = 0;//300 //连续多长时间静音就清除EQ MEM
const int AUDIO_EQ_CLEAR_MEM_BY_MUTE_LIMIT = 0; //静音判断阀值

const int config_audio_drc_en = 0
#if TCFG_DRC_ENABLE
                                | DRC_EN  //drc使能
                                | WDRC_TYPE_EN
                                /* |DRC_COMPRESSOF_DIS //关闭压缩器 */
                                /* |DRC_NBAND_DIS //关闭多带压缩器,关闭多带限幅器 */
#endif
                                | 0; //end
/*************************************************************************************************/
#if TCFG_MIC_EFFECT_ENABLE
const int config_audio_dac_mix_enable = 1;
#else
const int config_audio_dac_mix_enable = 0;
#endif

const int config_audio_dac_noisefloor_optimize_enable = 0;

#ifdef CONFIG_SOUNDBOX_FLASH_256K
// mixer模块使能。不使能将关闭大部分功能，mix为直通
const int config_mixer_en = 0;
// mixer变采样使能
const int config_mixer_src_en = 0;

// audio解码资源叠加功能使能。不使能，如果配置了叠加方式，将改成抢占方式
const int config_audio_dec_wait_protect_en = 0;

// audio数据流分支功能使能。
const int config_audio_stream_frame_copy_en = 0;

// audio dec app调用mixer相关函数控制。关闭后需上层设置数据流的输出节点
const int audio_dec_app_mix_en = 0;

#else

// mixer模块使能。不使能将关闭大部分功能，mix为直通
const int config_mixer_en = 1;
// mixer变采样使能
const int config_mixer_src_en = 1;

// audio解码资源叠加功能使能。不使能，如果配置了叠加方式，将改成抢占方式
const int config_audio_dec_wait_protect_en = 1;

// audio数据流分支功能使能。
const int config_audio_stream_frame_copy_en = 1;

// audio dec app调用mixer相关函数控制。关闭后需上层设置数据流的输出节点
const int audio_dec_app_mix_en = 1;

#endif

//wts解码支持采样率可选择，可以同时打开也可以单独打开
const  int  silk_fsN_enable = 1;   //支持8-12k采样率
const  int  silk_fsW_enable = 1;  //支持16-24k采样率

// audio数据流分支cbuf大小控制
const int config_audio_stream_frame_copy_cbuf_min = 128;
const int config_audio_stream_frame_copy_cbuf_max = 1024;

#if RECORDER_MIX_EN
// 超时等待其他解码unactive步骤完成
const int config_audio_dec_unactive_to = 50;
// audio数据流ioctrl使能
const int config_audio_stream_frame_ioctrl_en = 1;
#else
// 超时等待其他解码unactive步骤完成
const int config_audio_dec_unactive_to = 0;
// audio数据流ioctrl使能
const int config_audio_stream_frame_ioctrl_en = 0;
#endif

#if TCFG_TONE2TWS_ENABLE

// audio dec app tws同步使能
const int audio_dec_app_sync_en = 1;

// wma tws 解码控制
const int WMA_TWSDEC_EN = 1;

// wav最大支持比特率，单位kbps
const int WAV_MAX_BITRATEV = (48 * 2 * 12);

#else

// audio dec app tws同步使能
const int audio_dec_app_sync_en = 0;

// wma tws 解码控制
const int WMA_TWSDEC_EN = 0;

// wav最大支持比特率，单位kbps
const int WAV_MAX_BITRATEV = (48 * 2 * 32);

#endif


#if TCFG_DEC2TWS_ENABLE || SOUNDCARD_ENABLE || TCFG_MIC_EFFECT_ENABLE || RECORDER_MIX_EN || TCFG_USER_EMITTER_ENABLE

// 解码一次输出点数，1代表32对点，n就是n*32对点
// 超过1时，解码需要使用malloc，如config_mp3_dec_use_malloc=1
#if(CONFIG_CPU_BR25)
const int MP3_OUTPUT_LEN = 4;
const int WMA_OUTPUT_LEN = 4;
#else
const int MP3_OUTPUT_LEN = 4;
const int WMA_OUTPUT_LEN = 4;
#endif

// output超过1时，如果不使用malloc，需要增大对应buf
// 可以看打印中解码器需要的大小，一般输出每增加1长度增加32*2*2
int mp3_mem_ext[(16100 + 3) / 4] SEC(.mp3_mem);
int wma_mem_ext[(23900 + 3) / 4] SEC(.wma_mem);
int wma_mem_tws_ext[(29314 + 3) / 4] SEC(.wma_mem);

#else

// 解码一次输出点数，1代表32对点，n就是n*32对点
// 超过1时，解码需要使用malloc，如config_mp3_dec_use_malloc=1
const int MP3_OUTPUT_LEN = 1;
const int WMA_OUTPUT_LEN = 1;
#endif

// mixer在单独任务中输出
#if TCFG_MIXER_CYCLIC_TASK_EN
const int config_mixer_task = 1;
#else
const int config_mixer_task = 0;
#endif

// tws音频解码自动设置输出声道。
// 单声道：AUDIO_CH_L/AUDIO_CH_R。双声道：AUDIO_CH_DUAL_L/AUDIO_CH_DUAL_R
// 关闭后，按照output_ch_num和output_ch_type/ch_type设置输出声道
const int audio_tws_auto_channel = 1;


// 解码使用单独任务做输出
#if TCFG_AUDIO_DEC_OUT_TASK
const int config_audio_dec_out_task_en = 1;
#else
const int config_audio_dec_out_task_en = 0;
#endif


/*省电容mic配置*/
#if TCFG_SUPPORT_MIC_CAPLESS
const u8 const_mic_capless_en = 1;
#else
const u8 const_mic_capless_en = 0;
#endif/*TCFG_SUPPORT_MIC_CAPLESS*/

#if AUDIO_EQUALLOUDNESS_CONFIG
const int const_equall_loundness_en = 1;
#else
const int const_equall_loundness_en = 0;
#endif

#if AUDIO_VBASS_CONFIG
const int const_vbass_en = 1;
#else
const int const_vbass_en = 0;
#endif

#if AUDIO_SURROUND_CONFIG
const int const_surround_en = 1;//|BIT(1) ;//或上bit1 将使能effect_3d_type2 mips消耗55M ram占33k
#else
const int const_surround_en = 0;
#endif




#ifdef CONFIG_MIDI_DEC_ADDR
const int MIDI_TONE_MODE = 0;//0是地址访问(仅支持在内置flash,读数快，消耗mips低)，1 是文件访问(内置、外挂flash,sd,u盘均可,读数慢，消耗mips较大)
#else
const int MIDI_TONE_MODE = 1;
#endif
const int MAINTRACK_USE_CHN = 0;	//MAINTRACK_USE_CHN控制主通道区分方式；   0用track号区分主通道 1用chn号区分主通道
const int MAX_PLAYER_CNT = 18;     //控制可配置的最大同时发声的key数的BUF[1,32]

#if TCFG_MEDIA_LIB_USE_MALLOC
const int config_mp3_dec_use_malloc     = 1;
const int config_mp3pick_dec_use_malloc = 1;
const int config_wma_dec_use_malloc     = 1;
const int config_wmapick_dec_use_malloc = 1;
const int config_m4a_dec_use_malloc     = 1;
const int config_m4apick_dec_use_malloc = 1;
const int config_wav_dec_use_malloc     = 1;
const int config_alac_dec_use_malloc    = 1;
const int config_dts_dec_use_malloc     = 1;
const int config_amr_dec_use_malloc     = 1;
const int config_flac_dec_use_malloc    = 1;
const int config_ape_dec_use_malloc     = 1;
const int config_aac_dec_use_malloc     = 1;
const int config_aptx_dec_use_malloc    = 1;
const int config_midi_dec_use_malloc    = 1;
#else
const int config_mp3_dec_use_malloc     = 0;
const int config_mp3pick_dec_use_malloc = 0;
const int config_wma_dec_use_malloc     = 0;
const int config_wmapick_dec_use_malloc = 0;
const int config_m4a_dec_use_malloc     = 0;
const int config_m4apick_dec_use_malloc = 0;
const int config_wav_dec_use_malloc     = 0;
const int config_alac_dec_use_malloc    = 0;
const int config_dts_dec_use_malloc     = 0;
const int config_amr_dec_use_malloc     = 0;
const int config_flac_dec_use_malloc    = 0;
const int config_ape_dec_use_malloc     = 0;
const int config_aac_dec_use_malloc     = 0;
const int config_aptx_dec_use_malloc    = 0;
const int config_midi_dec_use_malloc    = 0;
#endif

enum {
    PLATFORM_FREQSHIFT_CORDIC = 0,
    PLATFORM_FREQSHIFT_CORDICV2 = 1
};

/* 备注:  */
/* br23/br25/br30/br34/br40是PLATFORM_FREQSHIFT_CORDIC  */
/* ， br36/br28/br27是PLATFORM_FREQSHIFT_CORDICV2 */
const  int  howling_freshift_PLATFORM = PLATFORM_FREQSHIFT_CORDIC;

const int vc_pitchshift_fastmode_flag        = 1; //变声快速模式使能
const  int  vc_pitchshift_only = 0;
//变声库数学函数版本配置，br23/br25/br30/br34/br40是PLATFORM_VOICECHANGE_CORDIC，br27/br28/br36是PLATFORM_VOICECHANGE_CORDICV2
const  int  voicechange_mathfun_PLATFORM = PLATFORM_FREQSHIFT_CORDIC;
//变声库的fft配置，  br28是PLATFORM_VOICECHANGE_FFTV2, br23/br25/br30/br36/br40是PLATFORM_VOICECHANGE_FFT
const  int  voicechange_fft_PLATFORM = PLATFORM_FREQSHIFT_CORDIC;

const  int  vc_pitchshift_downmode_flag = 0;  //变声下采样处理使能
const int  VC_KINDO_TVM = 1;       //含义为EFFECT_VOICECHANGE_KIN0是否另一种算法 : 0表示跟原来一样，1表示用另一种
const  int  howling_freshift_highmode_flag = 0;              //移频快速模式
const int howling_pitchshift_fastmode_flag   = 1;//移频啸叫抑制快速模式使能
const  int RS_FAST_MODE_QUALITY = 2;	//软件变采样 滤波阶数配置，范围2到8， 8代表16阶的变采样模式 ,速度跟它的大小呈正相关
/* const int RMAX_ROOMSIZE         = 1; */
const int config_howling_enable_pemafrow_mode = 0;
const int config_howling_enable_trap_mode     = 1;//陷波啸叫抑制模式使能
const int config_howling_enable_pitchps_mode  = 1; //移频啸叫抑制模式使能
const  int  ECHO_INT_VAL_OUT = 0;       //  置1: echo的输出是int 后级需接DRC限幅 功能未实现

#if(CONFIG_CPU_BR25)
const  int DOWN_S_FLAG 				= 1; //混响降采样处理使能
#else
const  int DOWN_S_FLAG 				= 0; //混响降采样处理使能
#endif
#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DONGLE)
const int config_mp3_enc_use_layer_3	= 1;
#else
const int config_mp3_enc_use_layer_3	= 0;
#endif

#define FAST_FREQ_restrict				0x01 //限制超过16k的频率不解【一般超出人耳听力范围，但是仪器会测出来】
#define FAST_FILTER_restrict			0x02 //限制滤波器长度【子带滤波器旁瓣加大，边缘不够陡】
#define FAST_CHANNEL_restrict			0x04 //混合左右声道，再解码【如果是左右声道独立性较强的歌曲，会牺牲空间感，特别耳机听会听出来的话】
const int config_mp3_dec_speed_mode 	=  0;//FAST_FREQ_restrict | FAST_FILTER_restrict | FAST_CHANNEL_restrict; //3个开关都置上，是最快的解码模式

// 快进快退到文件end返回结束消息
const int config_decoder_ff_fr_end_return_event_end = 0;

/**
 * @brief Log (Verbose/Info/Debug/Warn/Error)
 */
/*-----------------------------------------------------------*/
/* const char log_tag_const_v_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE); */
/* const char log_tag_const_i_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE); */
/* const char log_tag_const_d_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE); */
/* const char log_tag_const_w_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE); */
/* const char log_tag_const_e_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE); */

/* const char log_tag_const_v_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE); */
/* const char log_tag_const_i_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE); */
/* const char log_tag_const_d_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE); */
/* const char log_tag_const_w_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE); */
/* const char log_tag_const_e_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE); */

const char log_tag_const_v_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);


const char log_tag_const_v_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);


const char log_tag_const_v_VBASS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_VBASS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_VBASS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_VBASS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_VBASS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUD_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_AUD_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_AUD_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_AUD_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_e_AUD_ADC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);

const char log_tag_const_v_AUD_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_AUD_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_AUD_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_AUD_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_e_AUD_DAC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);

const char log_tag_const_v_AUD_AUX AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_AUD_AUX AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_AUD_AUX AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_AUD_AUX AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_e_AUD_AUX AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);



const char log_tag_const_v_MIXER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_c_MIXER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_MIXER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_MIXER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_MIXER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_STREAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_c_AUDIO_STREAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_AUDIO_STREAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_AUDIO_STREAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_AUDIO_STREAM AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_c_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_ENCODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_c_AUDIO_ENCODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_AUDIO_ENCODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_AUDIO_ENCODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_AUDIO_ENCODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);



