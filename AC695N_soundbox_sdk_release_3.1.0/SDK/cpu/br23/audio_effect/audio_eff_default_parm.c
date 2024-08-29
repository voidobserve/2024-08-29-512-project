#include "media/effects_adj.h"
#include "audio_eff_default_parm.h"
#include "app_config.h"
#include "math.h"
float powf(float x, float y);
extern struct mode_list *get_group_list(u16 module_name);
extern const struct eq_seg_info phone_eq_tab_normal[3];
extern const struct eq_seg_info ul_eq_tab_normal[3];
extern const struct eq_seg_info eq_tab_normal[10];
const struct eq_seg_info mic_eff_eq_tab[5] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 200,   0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 300,   0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, 0.7f},
};

void phone_eff_default_parm(void)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
    //通话下行eq
    for (u8 tar = 0; tar < 2; tar++) {
        phone_mode[tar].eq_parm.global_gain = 0;
        phone_mode[tar].eq_parm.seg_num = ARRAY_SIZE(phone_eq_tab_normal);
        memcpy(phone_mode[tar].eq_parm.seg, phone_eq_tab_normal, sizeof(phone_eq_tab_normal));
    }
    //通话上行eq
    for (u8 tar = 2; tar < 4; tar++) {
        phone_mode[tar].eq_parm.global_gain = 0;
        phone_mode[tar].eq_parm.seg_num = ARRAY_SIZE(ul_eq_tab_normal);
        memcpy(phone_mode[tar].eq_parm.seg, ul_eq_tab_normal, sizeof(ul_eq_tab_normal));
    }
#endif

    //通话下行drc
    struct threshold_group group[] = {{0, 0}, {50, 50}, {90, 90}};
    for (u8 tar = 0; tar < 2; tar++) {
        phone_mode[tar].drc_parm.is_bypass = 0;
        phone_mode[tar].drc_parm.parm.attacktime = 10;
        phone_mode[tar].drc_parm.parm.releasetime = 300;
        phone_mode[tar].drc_parm.parm.inputgain = 0;
        phone_mode[tar].drc_parm.parm.outputgain = 0;
        phone_mode[tar].drc_parm.parm.threshold_num = ARRAY_SIZE(group);
        memcpy(phone_mode[tar].drc_parm.parm.threshold, group, sizeof(group));
        phone_mode[tar].drc_parm.parm.rms_time = 25;
        phone_mode[tar].drc_parm.parm.algorithm = 0;
        phone_mode[tar].drc_parm.parm.mode = 1;
    }

    //通话上行行drc
    struct threshold_group group2[] = {{0, 0}, {50, 50}, {90, 90}};
    for (u8 tar = 2; tar < 4; tar++) {
        phone_mode[tar].drc_parm.is_bypass = 0;
        phone_mode[tar].drc_parm.parm.attacktime = 10;
        phone_mode[tar].drc_parm.parm.releasetime = 300;
        phone_mode[tar].drc_parm.parm.inputgain = 0;
        phone_mode[tar].drc_parm.parm.outputgain = 0;
        phone_mode[tar].drc_parm.parm.threshold_num = ARRAY_SIZE(group2);
        memcpy(phone_mode[tar].drc_parm.parm.threshold, group2, sizeof(group2));
        phone_mode[tar].drc_parm.parm.rms_time = 25;
        phone_mode[tar].drc_parm.parm.algorithm = 0;
        phone_mode[tar].drc_parm.parm.mode = 1;
    }
}



void music_eff_default_parm(void)
{


    float gain = 0;
#if AUDIO_VBASS_CONFIG
//gain
    vbass_prev_gain_parm.is_bypass = 0;
    vbass_prev_gain_parm.parm.gain = powf(10, gain / 20.0f); //db转mag,工具传下来的值

    vbass_parm.is_bypass = 0;
    vbass_parm.parm.ratio = 10;
    vbass_parm.parm.boost = 1;
    vbass_parm.parm.fc = 100;
#endif

#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
//eq
    music_mode.eq_parm.global_gain = 0;
    music_mode.eq_parm.seg_num = ARRAY_SIZE(eq_tab_normal);
    memcpy(music_mode.eq_parm.seg, eq_tab_normal, sizeof(eq_tab_normal));
#endif

//wdcrc
    struct mode_list *list = get_group_list(AEID_MUSIC_DRC);
    if (list) {
        struct threshold_group group[] = {{0, 0}, {50, 50}, {90, 90}};
        for (int i = 0; i < list->group_num; i++) {
            if (i == (list->group_num - 1)) { //最后一个存分频器系数
                music_mode.drc_parm.crossover.way_num = 2;
                music_mode.drc_parm.crossover.N = 2;
                music_mode.drc_parm.crossover.low_freq = 200;
            } else {
                music_mode.drc_parm.wdrc_parm[i].is_bypass = 0;
                music_mode.drc_parm.wdrc_parm[i].parm.attacktime = 10;
                music_mode.drc_parm.wdrc_parm[i].parm.releasetime = 300;
                music_mode.drc_parm.wdrc_parm[i].parm.inputgain = 0;
                music_mode.drc_parm.wdrc_parm[i].parm.outputgain = 0;
                music_mode.drc_parm.wdrc_parm[i].parm.threshold_num = ARRAY_SIZE(group);
                memcpy(music_mode.drc_parm.wdrc_parm[i].parm.threshold, group, sizeof(group));
                music_mode.drc_parm.wdrc_parm[i].parm.rms_time = 25;
                music_mode.drc_parm.wdrc_parm[i].parm.algorithm = 0;
                music_mode.drc_parm.wdrc_parm[i].parm.mode = 1;
            }
        }
    }


#if TCFG_DYNAMIC_EQ_ENABLE
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
//eq2
    music_eq2_parm.global_gain = 0;
    music_eq2_parm.seg_num = ARRAY_SIZE(eq_tab_normal);
    memcpy(music_eq2_parm.seg, eq_tab_normal, sizeof(eq_tab_normal));
#endif

//dynamic eq
    dynamic_eq.is_bypass = 0;
    dynamic_eq.nSection = 1;
    dynamic_eq.detect_mode = 1;
    for (u8 i = 0; i < dynamic_eq.nSection; i++) {
        dynamic_eq.effect_param[i].fc = 1000;
        dynamic_eq.effect_param[i].Q = 0.7f;
        dynamic_eq.effect_param[i].gain = 0.0f;
        dynamic_eq.effect_param[i].type = 0x2;
        dynamic_eq.effect_param[i].attackTime = 5;
        dynamic_eq.effect_param[i].releaseTime = 300;
        dynamic_eq.effect_param[i].rmsTime = 25;
        dynamic_eq.effect_param[i].threshold = 0.0f;
        dynamic_eq.effect_param[i].ratio = 1.0f;
        dynamic_eq.effect_param[i].noisegate_threshold = -90.3f;
        dynamic_eq.effect_param[i].fixGain = 0.0f;
        dynamic_eq.effect_param[i].algorithm = 1;
    }
#endif

#if GAIN_PROCESS_EN
//gain
    gain_parm.is_bypass = 0;
    gain = 0;
    gain_parm.parm.gain = powf(10, gain / 20.0f); //db转mag,工具传下来的值
#endif

#if defined(SOUND_TRACK_2_P_X_CH_CONFIG) &&SOUND_TRACK_2_P_X_CH_CONFIG
//low pass
    low_pass_parm.is_bypass = 0;
    low_pass_parm.low_pass.fc = 100;
    low_pass_parm.low_pass.order = 4;
    low_pass_parm.low_pass.type = 1;
#endif
//rl_wdrc

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
    struct threshold_group group2[] = {{0, 0}, {50, 50}, {90, 90}};
    for (int i = 0; i < 4; i++) {
        rl_drc_parm.wdrc_parm[i].is_bypass = 0;
        rl_drc_parm.wdrc_parm[i].parm.attacktime = 10;
        rl_drc_parm.wdrc_parm[i].parm.releasetime = 300;
        rl_drc_parm.wdrc_parm[i].parm.inputgain = 0;
        rl_drc_parm.wdrc_parm[i].parm.outputgain = 0;
        rl_drc_parm.wdrc_parm[i].parm.threshold_num = ARRAY_SIZE(group2);
        memcpy(rl_drc_parm.wdrc_parm[i].parm.threshold, group2, sizeof(group2));
        rl_drc_parm.wdrc_parm[i].parm.rms_time = 25;
        rl_drc_parm.wdrc_parm[i].parm.algorithm = 0;
        rl_drc_parm.wdrc_parm[i].parm.mode = 1;
    }
#endif

#if GAIN_PROCESS_EN
//rl_gain
    rl_gain_parm.is_bypass = 0;
    rl_gain_parm.parm.gain = powf(10, gain / 20.0f); //db转mag,工具传下来的值
#endif
}


#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
void linein_eff_default_parm(void)
{
#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
//eq
    linein_mode.eq_parm.global_gain = 0;
    linein_mode.eq_parm.seg_num = ARRAY_SIZE(eq_tab_normal);
    memcpy(linein_mode.eq_parm.seg, eq_tab_normal, sizeof(eq_tab_normal));
#endif

//wdcrc
    struct mode_list *list = get_group_list(AEID_MUSIC_DRC);
    if (list) {
        struct threshold_group group[] = {{0, 0}, {50, 50}, {90, 90}};
        for (int i = 0; i < list->group_num; i++) {
            if ((list->group_num > 1) && (i == (list->group_num - 1))) { //最后一个存分频器系数
                linein_mode.drc_parm.crossover.way_num = 2;
                linein_mode.drc_parm.crossover.N = 2;
                linein_mode.drc_parm.crossover.low_freq = 200;
            } else {
                linein_mode.drc_parm.wdrc_parm[i].is_bypass = 0;
                linein_mode.drc_parm.wdrc_parm[i].parm.attacktime = 10;
                linein_mode.drc_parm.wdrc_parm[i].parm.releasetime = 300;
                linein_mode.drc_parm.wdrc_parm[i].parm.inputgain = 0;
                linein_mode.drc_parm.wdrc_parm[i].parm.outputgain = 0;
                linein_mode.drc_parm.wdrc_parm[i].parm.threshold_num = ARRAY_SIZE(group);
                memcpy(linein_mode.drc_parm.wdrc_parm[i].parm.threshold, group, sizeof(group));
                linein_mode.drc_parm.wdrc_parm[i].parm.rms_time = 25;
                linein_mode.drc_parm.wdrc_parm[i].parm.algorithm = 0;
                linein_mode.drc_parm.wdrc_parm[i].parm.mode = 1;
            }
        }
    }
#if GAIN_PROCESS_EN
//rl_gain
    float gain = 0;
    linein_gain_parm.is_bypass = 0;
    linein_gain_parm.parm.gain = powf(10, gain / 20.0f); //db转mag,工具传下来的值
#endif

}
#endif


#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
void mic_eff_default_parm(u8 index)
{
//noisegate
    eff_mode[index].noise_gate_parm.is_bypass = 0;
    eff_mode[index].noise_gate_parm.parm.attackTime = 300;
    eff_mode[index].noise_gate_parm.parm.releaseTime = 5;
    eff_mode[index].noise_gate_parm.parm.threshold = -90300;//mdb -90.3dB
    eff_mode[index].noise_gate_parm.parm.low_th_gain = 0;

//howlingps_parm
    eff_mode[index].howlingps_parm.is_bypass = 0;
    eff_mode[index].howlingps_parm.parm.effect_v = EFFECT_HOWLING_FS;
    eff_mode[index].howlingps_parm.parm.ps_parm = -50;
    eff_mode[index].howlingps_parm.parm.fe_parm = 4;

//notchowling_parm
    eff_mode[index].notchhowling_parm.is_bypass = 0;
    eff_mode[index].notchhowling_parm.parm.Q = 2.0f;
    eff_mode[index].notchhowling_parm.parm.gain = -20;
    eff_mode[index].notchhowling_parm.parm.fade_n = 10;
    eff_mode[index].notchhowling_parm.parm.threshold = 25;

//voice_changer
#if defined(TCFG_MIC_VOICE_CHANGER_ENABLE) && TCFG_MIC_VOICE_CHANGER_ENABLE
    eff_mode[index].voicechanger_parm.is_bypass = 0;
    eff_mode[index].voicechanger_parm.parm.effect_v = 0;
    eff_mode[index].voicechanger_parm.parm.shiftv = 56;
    eff_mode[index].voicechanger_parm.parm.formant_shift = 90;
#endif

//echo
    eff_mode[index].echo_parm.is_bypass = 0;
    eff_mode[index].echo_parm.parm.decayval = 60;
    eff_mode[index].echo_parm.parm.delay = 400;
    eff_mode[index].echo_parm.parm.filt_enable = 1;
    eff_mode[index].echo_parm.parm.lpf_cutoff = 5000;
    eff_mode[index].echo_parm.parm.drygain = 60;
    eff_mode[index].echo_parm.parm.wetgain = 50;

//palte_reverb
    eff_mode[index].plate_reverb_parm.is_bypass = 0;
    eff_mode[index].plate_reverb_parm.parm.pre_delay = 0;
    eff_mode[index].plate_reverb_parm.parm.highcutoff = 12200;
    eff_mode[index].plate_reverb_parm.parm.diffusion = 43;
    eff_mode[index].plate_reverb_parm.parm.decayfactor = 70;
    eff_mode[index].plate_reverb_parm.parm.highfrequencydamping = 26;
    eff_mode[index].plate_reverb_parm.parm.dry = 80;
    eff_mode[index].plate_reverb_parm.parm.wet = 40;
    eff_mode[index].plate_reverb_parm.parm.modulate = 1;
    eff_mode[index].plate_reverb_parm.parm.roomsize = 100;

    for (int i = 0; i < 5; i++) {
//eq
        eff_mode[index].eq_parm[i].global_gain = 0;
        eff_mode[index].eq_parm[i].seg_num = ARRAY_SIZE(mic_eff_eq_tab);
        memcpy(eff_mode[index].eq_parm[i].seg, mic_eff_eq_tab, sizeof(mic_eff_eq_tab));
//drc
        struct threshold_group group[] = {{0, 0}, {50, 50}, {90, 90}};
        eff_mode[index].drc_parm[i].is_bypass = 0;
        eff_mode[index].drc_parm[i].parm.attacktime = 10;
        eff_mode[index].drc_parm[i].parm.releasetime = 300;
        eff_mode[index].drc_parm[i].parm.inputgain = 0;
        eff_mode[index].drc_parm[i].parm.outputgain = 0;
        eff_mode[index].drc_parm[i].parm.threshold_num = ARRAY_SIZE(group);
        memcpy(eff_mode[index].drc_parm[i].parm.threshold, group, sizeof(group));
        eff_mode[index].drc_parm[i].parm.rms_time = 25;
        eff_mode[index].drc_parm[i].parm.algorithm = 0;
        eff_mode[index].drc_parm[i].parm.mode = 1;
    }
#if GAIN_PROCESS_EN
    float gain = 0;
    eff_mode[index].gain_parm.parm.gain = powf(10, gain / 20.0f); //db转mag,工具传下来的值
    eff_mode[index].gain_parm.is_bypass = 0;
#endif
}
#endif
