
#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "clock_cfg.h"
#include "media/audio_eq_drc_apply.h"
#include "audio_eq_drc_demo.h"
#include "media/effects_adj.h"
#include "audio_effect/audio_eff_default_parm.h"
#include "audio_effect/audio_sound_track_2_p_x.h"


#if 0
//系数切换
void eq_sw_demo()
{
    eq_mode_sw();//7种默认系数切换
}

//获取当前eq系数表类型
void eq_mode_get_demo()
{
    u8 mode ;
    mode = eq_mode_get_cur();
}
//宏TCFG_USE_EQ_FILE配0
//自定义系数表动态更新
//本demo 示意更新中心截止频率，增益，总增益，如需设置更多参数，请查看eq_config.h头文件的demo
void eq_update_demo()
{
    eq_mode_set_custom_info(0, 200, 2);//第0段,200Hz中心截止频率，2db
    eq_mode_set_custom_info(5, 2000, 2);//第5段,2000Hz中心截止频率，2db

    set_global_gain(get_eq_cfg_hdl(), song_eq_mode, -2);//设置普通音乐eq 总增益 -2db（可避免最大增益大于0db，导致失真）

    eq_mode_set(EQ_MODE_CUSTOM);//设置系数、总增益更新
}

#endif

//用户自定义eq  drc系数
#if 1

static const struct eq_seg_info your_audio_out_eq_tab[] = {//2段系数
    {0, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 12000, 0, 0.7f},
};
static int your_eq_coeff_tab[2][5];//2段eq系数表的长度
/*----------------------------------------------------------------------------*/
/**@brief    用户自定义eq的系数回调
   @param    eq:句柄
   @param    sr:采样率
   @param    info: 系数地址
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int eq_get_filter_info_demo(void *_eq, int sr, struct audio_eq_filter_info *info)
{
    if (!sr) {
        sr = 44100;
    }
#if TCFG_EQ_ENABLE
    local_irq_disable();
    u8 nsection = ARRAY_SIZE(your_audio_out_eq_tab);
    for (int i = 0; i < nsection; i++) {
        eq_seg_design(&your_audio_out_eq_tab[i], sr, your_eq_coeff_tab[i]);//根据采样率对eq系数表，重计算，得出适用的系数表
    }
    local_irq_enable();
    info->L_coeff = info->R_coeff = (void *)your_eq_coeff_tab;//系数指针赋值
    info->L_gain = info->R_gain = 0;//总增益填写，用户可修改（-20~20db）,注意大于0db存在失真风险
    info->nsection = nsection;//eq段数，根据提供给的系数表来填写，例子是2
#endif//TCFG_EQ_ENABLE

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    更新自定义eq系数后，需要使用本函数将系数更新到eq模块
   @param    *_drc: 句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void eq_filter_info_update_demo(void *_eq)
{
#if TCFG_EQ_ENABLE
    struct audio_eq *eq = (struct audio_eq *)_eq;
    local_irq_disable();
    if (eq) {
        eq->updata = 1;
    }
    local_irq_enable();
#endif//TCFG_EQ_ENABLE

}



static struct drc_ch drc_fliter = {0};
#define your_threshold  (0)
/*----------------------------------------------------------------------------*/
/**@brief    自定义限幅器系数回调
   @param    *drc: 句柄
   @param    *info: 系数结构地址
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int drc_get_filter_info_demo(void *drc, struct audio_drc_filter_info *info)
{
#if TCFG_DRC_ENABLE
    float th = your_threshold;//-60 ~0db
    int threshold = round(pow(10.0, th / 20.0) * 32768); // 0db:32768, -60db:33

    /* drc_fliter.nband = 1; */
    /* drc_fliter.type = 1; */
    /* drc_fliter._p.limiter[0].attacktime = 5; */
    /* drc_fliter._p.limiter[0].releasetime = 300; */
    /* drc_fliter._p.limiter[0].threshold[0] = threshold; */
    /* drc_fliter._p.limiter[0].threshold[1] = 32768; */

    /* info->pch = info->R_pch = &drc_fliter; */
#endif//TCFG_DRC_ENABLE

    return 0;
}
#endif


/*----------------------------------------------------------------------------*/
/**@brief    更新自定义限幅器系数后，需要使用本函数将系数更新到drc模块
   @param    *_drc: 句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void drc_filter_info_update_demo(void *_drc)
{
#if TCFG_DRC_ENABLE
    struct audio_drc *drc = (struct audio_drc *)_drc;
    local_irq_disable();
    if (drc) {
        drc->updata = 1;
    }
    local_irq_enable();
#endif//TCFG_DRC_ENABLE

}

struct audio_eq *music_eq_open(u32 sample_rate, u8 ch_num)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    struct audio_eq_param parm = {0};
    parm.channels = ch_num;
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE
    parm.out_32bit = 1;//32bit位宽输出
#endif
    parm.no_wait = 1;
    parm.cb = eq_get_filter_info;
    parm.sr = sample_rate;
    parm.eq_name = AEID_MUSIC_EQ;

    parm.max_nsection = music_mode.eq_parm.seg_num;
    parm.nsection = music_mode.eq_parm.seg_num;
    parm.seg = music_mode.eq_parm.seg;
    parm.global_gain = music_mode.eq_parm.global_gain;
    struct audio_eq *eq = audio_dec_eq_open(&parm);
    clock_add(EQ_CLK);
    return eq;
#endif //TCFG_EQ_ENABLE
    return NULL;
}

void music_eq_close(struct audio_eq *eq)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    if (eq) {
        audio_dec_eq_close(eq);
        clock_remove(EQ_CLK);
    }
#endif/*TCFG_EQ_ENABLE*/
}

struct audio_eq *music_eq2_open(u32 sample_rate, u8 ch_num)
{
#if TCFG_DYNAMIC_EQ_ENABLE
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    struct audio_eq_param parm = {0};
    parm.channels = ch_num;
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE
    parm.out_32bit = 1;//32bit位宽输出
#endif
    parm.no_wait = 1;
    parm.cb = eq_get_filter_info;
    parm.sr = sample_rate;
    parm.eq_name = AEID_MUSIC_EQ2;

    parm.max_nsection = music_eq2_parm.seg_num;
    parm.nsection = music_eq2_parm.seg_num;
    parm.seg = music_eq2_parm.seg;
    parm.global_gain = music_eq2_parm.global_gain;
    struct audio_eq *eq = audio_dec_eq_open(&parm);
    clock_add(EQ_CLK);
    return eq;
#endif //TCFG_EQ_ENABLE
#endif
    return NULL;
}

void music_eq2_close(struct audio_eq *eq)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    if (eq) {
        audio_dec_eq_close(eq);
        clock_remove(EQ_CLK);
    }
#endif/*TCFG_EQ_ENABLE*/
}



struct audio_drc *music_drc_open(u32 sample_rate, u8 ch_num)
{
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE
    struct audio_drc_param parm = {0};
    parm.channels = ch_num;
    parm.sr = sample_rate;
    parm.out_32bit = 1;
    parm.cb = drc_get_filter_info;
    parm.drc_name = AEID_MUSIC_DRC;
#if defined(TCFG_AUDIO_MDRC_ENABLE) && (TCFG_AUDIO_MDRC_ENABLE == 1)
    parm.nband = CROSSOVER_EN;//
#elif defined(TCFG_AUDIO_MDRC_ENABLE) && (TCFG_AUDIO_MDRC_ENABLE == 2)
    parm.nband = CROSSOVER_EN | MORE_BAND_EN;//
#endif/*TCFG_AUDIO_MDRC_ENABLE*/

    parm.crossover = &music_mode.drc_parm.crossover;
    parm.wdrc = music_mode.drc_parm.wdrc_parm;
    struct audio_drc *drc = audio_dec_drc_open(&parm);
    clock_add(EQ_DRC_CLK);
    return drc;
#endif/*TCFG_DRC_ENABLE*/
    return NULL;
}

void music_drc_close(struct audio_drc *drc)
{
    if (drc) {
        audio_dec_drc_close(drc);
        clock_remove(EQ_DRC_CLK);
    }
}



struct audio_eq *esco_eq_open(u32 sample_rate, u8 ch_num, u8 bit_wide)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    struct audio_eq_param parm = {0};
    parm.channels = ch_num;
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE
    parm.out_32bit = bit_wide;//32bit位宽输出
#endif
    parm.no_wait = 1;
    parm.cb = eq_get_filter_info;
    parm.sr = sample_rate;
    parm.eq_name = AEID_ESCO_DL_EQ;
    u8 index = 0;
    if (sample_rate == 8000) { //窄频
        index = 1;
    }
    parm.max_nsection = phone_mode[index].eq_parm.seg_num;
    parm.nsection = phone_mode[index].eq_parm.seg_num;
    parm.seg = phone_mode[index].eq_parm.seg;
    parm.global_gain = phone_mode[index].eq_parm.global_gain;
    struct audio_eq *eq = audio_dec_eq_open(&parm);
    clock_add(EQ_CLK);
    return eq;
#endif //TCFG_EQ_ENABLE
    return NULL;
}

void esco_eq_close(struct audio_eq *eq)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    if (eq) {
        audio_dec_eq_close(eq);
        clock_remove(EQ_CLK);
    }
#endif/*TCFG_EQ_ENABLE*/
}

struct audio_drc *esco_drc_open(u32 sample_rate, u8 ch_num, u8 bit_wide)
{
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE
    struct audio_drc_param parm = {0};
    parm.channels = ch_num;
    parm.sr = sample_rate;
    parm.out_32bit = bit_wide;
    parm.cb = drc_get_filter_info;
    parm.drc_name = AEID_ESCO_DL_DRC;
    u8 index = 0;
    if (sample_rate == 8000) { //窄频
        index = 1;
    }
    parm.wdrc = &phone_mode[index].drc_parm;
    struct audio_drc *drc = audio_dec_drc_open(&parm);
    clock_add(EQ_DRC_CLK);
    return drc;
#endif/*TCFG_DRC_ENABLE*/
    return NULL;
}

void esco_drc_close(struct audio_drc *drc)
{
    if (drc) {
        audio_dec_drc_close(drc);
        clock_remove(EQ_DRC_CLK);
    }
}

struct audio_eq *music_eq_rl_rr_open(u32 sample_rate, u8 ch_num)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    struct audio_eq_param parm = {0};
    parm.channels = ch_num;
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE
    parm.out_32bit = 1;//32bit位宽输出
#endif
    parm.no_wait = 1;
#if defined(SOUND_TRACK_2_P_X_CH_CONFIG) &&SOUND_TRACK_2_P_X_CH_CONFIG
    parm.cb = low_bass_eq_get_filter_info_demo;
#else
    parm.cb = eq_get_filter_info;
#endif
    parm.sr = sample_rate;
    parm.eq_name = AEID_MUSIC_RL_EQ;

    parm.max_nsection = music_mode.eq_parm.seg_num;
    parm.nsection = music_mode.eq_parm.seg_num;
    parm.seg = music_mode.eq_parm.seg;
    parm.global_gain = music_mode.eq_parm.global_gain;
    struct audio_eq *eq = audio_dec_eq_open(&parm);
    clock_add(EQ_CLK);
    return eq;
#endif //TCFG_EQ_ENABLE
    return NULL;
}

void music_eq_rl_rr_close(struct audio_eq *eq)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    if (eq) {
        audio_dec_eq_close(eq);
        clock_remove(EQ_CLK);
    }
#endif/*TCFG_EQ_ENABLE*/
}


struct audio_drc *music_drc_rl_rr_open(u32 sample_rate, u8 ch_num)
{
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE
    struct audio_drc_param parm = {0};
    parm.channels = ch_num;
    parm.sr = sample_rate;
    parm.out_32bit = 1;
    parm.cb = drc_get_filter_info;
    parm.drc_name = AEID_MUSIC_RL_DRC;
#if defined(TCFG_AUDIO_MDRC_ENABLE) && (TCFG_AUDIO_MDRC_ENABLE == 1)
    parm.nband = CROSSOVER_EN;//
#elif defined(TCFG_AUDIO_MDRC_ENABLE) && (TCFG_AUDIO_MDRC_ENABLE == 2)
    parm.nband = CROSSOVER_EN | MORE_BAND_EN;//
#endif/*TCFG_AUDIO_MDRC_ENABLE*/

    parm.crossover = &rl_drc_parm.crossover;//参数需要修改
    parm.wdrc = rl_drc_parm.wdrc_parm;
    struct audio_drc *drc = audio_dec_drc_open(&parm);
    clock_add(EQ_DRC_CLK);
    return drc;
#endif/*TCFG_DRC_ENABLE*/
#endif
    return NULL;
}

void music_drc_rl_rr_close(struct audio_drc *drc)
{
    if (drc) {
        audio_dec_drc_close(drc);
        clock_remove(EQ_DRC_CLK);
    }
}




#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
struct audio_eq *linein_eq_open(u32 sample_rate, u8 ch_num)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    struct audio_eq_param parm = {0};
    parm.channels = ch_num;
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE
    parm.out_32bit = 1;//32bit位宽输出
#endif
    parm.no_wait = 1;
    parm.cb = eq_get_filter_info;
    parm.sr = sample_rate;
    parm.eq_name = AEID_LINEIN_EQ;

    parm.max_nsection = linein_mode.eq_parm.seg_num;
    parm.nsection = linein_mode.eq_parm.seg_num;
    parm.seg = linein_mode.eq_parm.seg;
    parm.global_gain = linein_mode.eq_parm.global_gain;
    struct audio_eq *eq = audio_dec_eq_open(&parm);
    clock_add(EQ_CLK);
    return eq;
#endif //TCFG_EQ_ENABLE
    return NULL;
}

void linein_eq_close(struct audio_eq *eq)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    if (eq) {
        audio_dec_eq_close(eq);
        clock_remove(EQ_CLK);
    }
#endif/*TCFG_EQ_ENABLE*/
}

struct audio_drc *linein_drc_open(u32 sample_rate, u8 ch_num)
{
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE
    struct audio_drc_param parm = {0};
    parm.channels = ch_num;
    parm.sr = sample_rate;
    parm.out_32bit = 1;
    parm.cb = drc_get_filter_info;
    parm.drc_name = AEID_LINEIN_DRC;

#if 0
#if defined(TCFG_AUDIO_MDRC_ENABLE) && (TCFG_AUDIO_MDRC_ENABLE == 1)
    parm.nband = CROSSOVER_EN;//
#elif defined(TCFG_AUDIO_MDRC_ENABLE) && (TCFG_AUDIO_MDRC_ENABLE == 2)
    parm.nband = CROSSOVER_EN | MORE_BAND_EN;//
#endif/*TCFG_AUDIO_MDRC_ENABLE*/

    parm.crossover = &linein_mode.drc_parm.crossover;
#endif
    parm.wdrc = linein_mode.drc_parm.wdrc_parm;
    struct audio_drc *drc = audio_dec_drc_open(&parm);
    clock_add(EQ_DRC_CLK);
    return drc;
#endif/*TCFG_DRC_ENABLE*/
    return NULL;
}

void linein_drc_close(struct audio_drc *drc)
{
    if (drc) {
        audio_dec_drc_close(drc);
        clock_remove(EQ_DRC_CLK);
    }
}
#endif

struct music_eq_tool high_bass_eq_parm = {0};
struct eq_seg_info high_bass_eq_seg[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 12000, 0, 0.3f},
};
struct audio_eq *high_bass_eq_open(u32 sample_rate, u8 ch_num)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE && TCFG_AUDIO_OUT_EQ_ENABLE

    u16 seg_num = high_bass_eq_parm.seg_num;
    for (int i = 0; i < ARRAY_SIZE(high_bass_eq_seg); i++) {
        high_bass_eq_seg[i].index = seg_num + i;
        memcpy(&high_bass_eq_parm.seg[seg_num + i], &high_bass_eq_seg[i], sizeof(struct eq_seg_info));
    }

    struct audio_eq_param parm = {0};
    parm.channels = ch_num;
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE && TCFG_AUDIO_OUT_DRC_ENABLE
    parm.out_32bit = 1;//32bit位宽输出
#endif
    parm.no_wait = 1;
    parm.cb = eq_get_filter_info;
    parm.sr = sample_rate;
    parm.eq_name = AEID_HIGH_BASS_EQ;
    seg_num += ARRAY_SIZE(high_bass_eq_seg);
    parm.max_nsection = seg_num;
    parm.nsection = seg_num;
    parm.seg = high_bass_eq_parm.seg;
    parm.global_gain = high_bass_eq_parm.global_gain;

    parm.fade = 1;//高低音增益更新差异大，会引入哒哒音，此处使能系数淡入
    parm.fade_step = 0.4f;//淡入步进（0.1f~1.0f）

    struct audio_eq *eq = audio_dec_eq_open(&parm);
    clock_add(EQ_CLK);
    return eq;
#endif /*TCFG_EQ_ENABLE*/
    return NULL;
}

void high_bass_eq_close(struct audio_eq *eq)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE && TCFG_AUDIO_OUT_EQ_ENABLE
    if (eq) {
        audio_dec_eq_close(eq);
        clock_remove(EQ_CLK);
    }
#endif/*TCFG_EQ_ENABLE*/
}
/*
 *index: 0, 更新低音中心频率freq,增益gain
 *index: 1, 更新高音中心频率freq,增益gain
 * */
void high_bass_eq_udpate(u8 index, int freq, float gain)
{
    if (freq) {
        high_bass_eq_seg[index].freq = freq;
    }
    high_bass_eq_seg[index].gain = gain;
    printf("index %d ,gain %d\n", index, (int)gain);
    cur_eq_set_update(AEID_HIGH_BASS_EQ, &high_bass_eq_seg[index], high_bass_eq_parm.seg_num + ARRAY_SIZE(high_bass_eq_seg), 0);
}

//兼容旧接口
void mix_out_high_bass(u32 cmd, struct high_bass *hb)
{
    if (cmd == AUDIO_EQ_HIGH) {
        high_bass_eq_udpate(1, hb->freq, hb->gain);
    } else if (cmd == AUDIO_EQ_BASS) {
        high_bass_eq_udpate(0, hb->freq, hb->gain);
    }
}

wdrc_struct_TOOL_SET high_bass_drc_parm = {0};
struct audio_drc *high_bass_drc_open(u32 sample_rate, u8 ch_num)
{
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE && TCFG_AUDIO_OUT_DRC_ENABLE
    struct threshold_group group[] = {{0, 0}, {50, 50}, {90, 90}};
    high_bass_drc_parm.is_bypass = 0;
    high_bass_drc_parm.parm.attacktime = 10;
    high_bass_drc_parm.parm.releasetime = 300;
    high_bass_drc_parm.parm.inputgain = 0;
    high_bass_drc_parm.parm.outputgain = 0;
    high_bass_drc_parm.parm.threshold_num = ARRAY_SIZE(group);
    memcpy(high_bass_drc_parm.parm.threshold, group, sizeof(group));
    high_bass_drc_parm.parm.rms_time = 25;
    high_bass_drc_parm.parm.algorithm = 0;
    high_bass_drc_parm.parm.mode = 1;

    struct audio_drc_param parm = {0};
    parm.channels = ch_num;
    parm.sr = sample_rate;
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE && TCFG_AUDIO_OUT_EQ_ENABLE
    parm.out_32bit = 1;
#else
    parm.out_32bit = 0;
#endif
    parm.cb = drc_get_filter_info;
    parm.drc_name = AEID_HIGH_BASS_DRC;
    parm.wdrc = &high_bass_drc_parm;
    struct audio_drc *drc = audio_dec_drc_open(&parm);
    clock_add(EQ_DRC_CLK);
    return drc;
#endif/*TCFG_DRC_ENABLE*/

    return NULL;
}

void high_bass_drc_close(struct audio_drc *drc)
{
#if defined(TCFG_DRC_ENABLE) && TCFG_DRC_ENABLE && TCFG_AUDIO_OUT_DRC_ENABLE
    if (drc) {
        audio_dec_drc_close(drc);
        clock_remove(EQ_DRC_CLK);
    }
#endif
}
/*----------------------------------------------------------------------------*/
/**@brief    高低音限幅器系数回调
   @param    *drc: 句柄
   @param    *info: 系数结构地址
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int high_bass_drc_set_filter_info(int th)
{
    if (th < -60) {
        th = -60;
    }
    if (th > 0) {
        th = 0;
    }

    struct threshold_group group[] = {{0, 0}, {50, 50}, {90, 90}};
    group[1].in_threshold = 90 + th;
    group[1].out_threshold = 90 + th;
    group[2].in_threshold = 90.3f;
    group[2].out_threshold = 90 + th;
    memcpy(high_bass_drc_parm.parm.threshold, group, sizeof(group));
    cur_drc_set_update(AEID_HIGH_BASS_DRC, 0, &high_bass_drc_parm);
    return 0;
}

