#include "audio_effect/audio_gain_process_demo.h"
#include "audio_effect/audio_voice_changer_demo.h"
#include "audio_effect/audio_vbass_demo.h"
#include "media/effects_adj.h"
#include "audio_effect/audio_eff_default_parm.h"
#include "app_sound_box_tool.h"
#include "app_config.h"
#include "audio_mic/effect_parm.h"

#define PARM_DEBUG  0

#define LOG_TAG     "[EFFECT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#include "debug.h"
#ifdef SUPPORT_MS_EXTENSIONS
//#pragma bss_seg(".audio_mic_stream_bss")
//#pragma data_seg(".audio_mic_stream_data")
#pragma const_seg(".audio_effect_adj_const")
#pragma code_seg(".audio_effect_adj_code")
#endif

#define EFF_CFG_FILE_NAME 			SDFILE_RES_ROOT_PATH"eq_cfg_hw.bin"
const u8 eff_sdk_name[16] 		= "AC695N";
const u8 eff_eq_ver[4] 			= {1, 0, 0, 0};


extern const u16 eq_name[5] ;
extern const u16 drc_name[5] ;
static const u16 eff_mode_seq[8] = {mic_mode_seq0, mic_mode_seq1, mic_mode_seq2, mic_mode_seq3, mic_mode_seq4, mic_mode_seq5, mic_mode_seq6, mic_mode_seq7}; //混响模式标号

//每个模块id对应一个group_id(功能id)
const struct mode_list mlist[] = {
    {AEID_ESCO_DL_EQ, phone_eq_nsection, 2,    {EFF_PHONE_WIDEBAND_EQ, EFF_PHONE_NARROWBAND_EQ}},
    {AEID_ESCO_DL_DRC, phone_eq_nsection, 2,    {EFF_PHONE_WIDEBAND_DRC, EFF_PHONE_NARROWBAND_DRC}},
    {AEID_ESCO_UL_EQ,  phone_eq_nsection, 2,    {EFF_AEC_WIDEBAND_EQ, EFF_AEC_NARROWBAND_EQ}},
    {AEID_ESCO_UL_DRC,  phone_eq_nsection, 2,    {EFF_AEC_WIDEBAND_DRC, EFF_AEC_NARROWBAND_DRC}},

    {AEID_MUSIC_EQ,  music_eq_nsection, 1,    {EFF_MUSIC_EQ}},
    {AEID_MUSIC_EQ2,  music_eq2_nsection, 1,    {EFF_MUSIC_EQ2}},
    {AEID_MUSIC_DRC,  music_eq_nsection, 5,    {EFF_MUSIC_LOW_DRC, EFF_MUSIC_MID_DRC, EFF_MUSIC_HIGH_DRC, EFF_MUSIC_WHOLE_DRC, EFF_MUSIC_CROSSOVER }},   //最后一个groud_id存方分频器
    {AEID_MUSIC_FR_EQ,  music_eq_nsection, 1,    {0x2003}},
    {AEID_MUSIC_FR_DRC,  music_eq_nsection, 1,    {0x2004}},
    {AEID_MUSIC_RL_EQ,  music_eq_nsection, 1,    {0x2005}},
    {AEID_MUSIC_RL_DRC,  music_eq_nsection, 5,    {EFF_MUSIC_RL_LOW_DRC, EFF_MUSIC_RL_MID_DRC, EFF_MUSIC_RL_MID_DRC, EFF_MUSIC_RL_HIGH_DRC, EFF_MUSIC_RL_WHOLE_DRC }},
    {AEID_MUSIC_RR_EQ,  music_eq_nsection, 1,    {0x2007}},
    {AEID_MUSIC_RR_DRC,  music_eq_nsection, 1,    {0x2008}},

    {AEID_MIC_EQ0,  mic_eq_nsection, 1,    {EFF_MIC_EQ0}},
    {AEID_MIC_DRC0,  mic_eq_nsection, 1,    {EFF_MIC_DRC0}},
    {AEID_MIC_EQ1,  mic_eq_nsection, 1,    {EFF_MIC_EQ1}},
    {AEID_MIC_DRC1,  mic_eq_nsection, 1,    {EFF_MIC_DRC1}},
    {AEID_MIC_EQ2,  mic_eq_nsection, 1,    {EFF_MIC_EQ2}},
    {AEID_MIC_DRC2,  mic_eq_nsection, 1,    {EFF_MIC_DRC2}},
    {AEID_MIC_EQ3,  mic_eq_nsection, 1,    {EFF_MIC_EQ3}},
    {AEID_MIC_DRC3,  mic_eq_nsection, 1,    {EFF_MIC_DRC3}},
    {AEID_MIC_EQ4,  mic_eq_nsection, 1,    {EFF_MIC_EQ4}},
    {AEID_MIC_DRC4,  mic_eq_nsection, 1,    {EFF_MIC_DRC4}},

    {AEID_LINEIN_EQ,  music_eq_nsection, 1,    {EFF_LINEIN_EQ}},
    {AEID_LINEIN_DRC,  music_eq_nsection, 1,    {EFF_LINEIN_DRC}},
    {AEID_HIGH_BASS_EQ,  music_eq_nsection, 1,    {EFF_MUSIC_HIGH_BASS_EQ}},
    {AEID_HIGH_BASS_DRC,  music_eq_nsection, 1,    {EFF_MUSIC_HIGH_BASS_DRC}},
    {AEID_MUSIC_LPF_EQ,  music_eq_nsection, 1,    {EFF_MUSIC_LPF_EQ}},

};

int get_module_name(u16 group_id)
{
    for (int i = 0; i < ARRAY_SIZE(mlist); i++) {
        struct mode_list *list = (struct mode_list *)&mlist[i];
        for (int j = 0; j < list->group_num; j++) {
            if (list->group_id[j] == group_id) {
                return list->module_name;
            }
        }
    }
    log_e("get_module_name error\n");
    return 0;
}
int get_module_name_and_index(u16 group_id, u16 *index)
{
    for (int i = 0; i < ARRAY_SIZE(mlist); i++) {
        struct mode_list *list = (struct mode_list *)&mlist[i];
        for (int j = 0; j < list->group_num; j++) {
            if (list->group_id[j] == group_id) {
                *index = j;
                return list->module_name;
            }
        }
    }
    log_e("get_module_name error\n");
    return 0;
}

int get_group_id(u16 module_name, u8 tar)
{
    for (int i = 0; i < ARRAY_SIZE(mlist); i++) {
        struct mode_list *list = (struct mode_list *)&mlist[i];
        if (list->module_name == module_name) { //
            return list->group_id[tar];
        }
    }
    log_e("get_module_id error\n");
    return 0;
}
struct mode_list *get_group_list(u16 module_name)
{
    for (int i = 0; i < ARRAY_SIZE(mlist); i++) {
        struct mode_list *list = (struct mode_list *)&mlist[i];
        if (list->module_name == module_name) { //
            return list;
        }
    }
    return NULL;
}
int get_eq_nsection(u16 module_name)
{
    for (int i = 0; i < ARRAY_SIZE(mlist); i++) {
        struct mode_list *list = (struct mode_list *)&mlist[i];
        if (list->module_name == module_name) {
            return list->nsection;
        }
    }
    log_e("get_eq_nsection error\n");
    return 0;
}

int get_eq_seg(u16 module_name)
{
    return 0;
}
void get_eff_mode(u16 mode_id, u16 group_id, u8 *mode_index, u8 *drc_index)//获取混响模式的index
{
    *mode_index = mode_id - mic_mode_seq0;
    *drc_index = group_id - EFF_MIC_DRC0;
}
int get_phone_mode(u16 group_id)
{
    return group_id - EFF_PHONE_WIDEBAND_EQ;
}

#define REPLY_TO_TOOL  (0)
#define EFF_CFG_FILE_ID  (0x3)//音效配置项源文件id

static u8 eff_mode_save = 0;
#if TCFG_DYNAMIC_EQ_ENABLE
DynamicEQParam_TOOL_SET  dynamic_eq;
#endif
struct eff_parm  eff_mode[EFFECT_REVERB_PARM_MAX];//混响的8个模式

#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
struct music_parm_tool_set linein_mode;//linin模式

#endif
struct music_parm_tool_set music_mode;//音乐模式

#if TCFG_DYNAMIC_EQ_ENABLE
struct music_eq2_tool music_eq2_parm;//音乐模式下第二eq
#endif

struct phone_parm_tool_set phone_mode[4];//通话上下行模式 0:下行宽 1：下行窄  2：上行宽  3:上行窄

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
struct music_eq_tool rl_eq_parm;//rl通道eq
struct nband_drc rl_drc_parm;//rl通道drc
#endif
#if defined(SOUND_TRACK_2_P_X_CH_CONFIG) &&SOUND_TRACK_2_P_X_CH_CONFIG
LowPassParam_TOOL_SET  low_pass_parm;//rl rr通道低通滤波器
#endif

void wdrc_printf(void *_wdrc);
static u16 alive_timer = 0;

int vbass_prev_gain_process_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if AUDIO_VBASS_CONFIG
    u8 mode_id = packet->data[0];
    memcpy(&vbass_prev_gain_parm, &packet->data[1], sizeof(vbass_prev_gain_parm));
    audio_gain_update_parm(AEID_MUSIC_VBASS_PREV_GAIN, &vbass_prev_gain_parm.parm, vbass_prev_gain_parm.is_bypass);
#if PARM_DEBUG
    log_debug("vbass_prev_gain_parm.is_bypass %d, gain 0x%x", vbass_prev_gain_parm.is_bypass, *(int *)&vbass_prev_gain_parm.parm.gain);
#endif
#endif
    return 0;
}


int music_vbass_parm_ananlyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if AUDIO_VBASS_CONFIG
    u8 mode_id = packet->data[0];
    memcpy(&vbass_parm, &packet->data[1], sizeof(vbass_parm));
    audio_vbass_update_demo(AEID_MUSIC_VBASS, &vbass_parm.parm, vbass_parm.is_bypass);
#if PARM_DEBUG
    log_debug("is_bypass %d, ratio %d, boost %d, fc %d\n", vbass_parm.is_bypass, vbass_parm.parm.ratio, vbass_parm.parm.boost, vbass_parm.parm.fc);
#endif
#endif
    return 0;
}


int mic_voice_changer_parm_ananlyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
#if defined(TCFG_MIC_VOICE_CHANGER_ENABLE) && TCFG_MIC_VOICE_CHANGER_ENABLE
    u8 mode_id = packet->data[0];
    VoiceChangerParam_TOOL_SET *changer_parm = &eff_mode[mode_id - mic_mode_seq0].voicechanger_parm;
    memcpy(changer_parm, &packet->data[1], sizeof(eff_mode[mode_id - mic_mode_seq0].voicechanger_parm));
    audio_voice_changer_update_demo(AEID_MIC_VOICE_CHANGER, &changer_parm->parm, changer_parm->is_bypass);
#if PARM_DEBUG
    log_debug("effect_v %d, shiftv %d, formant_shift %d, is_bypass %d\n", changer_parm->parm.effect_v, changer_parm->parm.shiftv, changer_parm->parm.formant_shift, changer_parm->is_bypass);
#endif
#endif
#endif
    return 0;
}

int mic_gain_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if GAIN_PROCESS_EN
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    u8 mode_id = packet->data[0];
    Gain_Process_TOOL_SET *gain_parm = &eff_mode[mode_id - mic_mode_seq0].gain_parm; //增益
    memcpy(gain_parm, &packet->data[1], sizeof(eff_mode[mode_id - mic_mode_seq0].gain_parm));
    audio_gain_update_parm(AEID_MIC_GAIN, &gain_parm->parm, eff_mode[mode_id - mic_mode_seq0].gain_parm.is_bypass);
#if PARM_DEBUG
    log_debug("gain_parm.is_bypass %d, gain 0x%x", gain_parm->is_bypass, *(int *)&gain_parm->parm.gain);
#endif
#endif
#endif
    return 0;
}

int gain_process_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if GAIN_PROCESS_EN
    u8 mode_id = packet->data[0];
    memcpy(&gain_parm, &packet->data[1], sizeof(gain_parm));
    audio_gain_update_parm(AEID_MUSIC_GAIN, &gain_parm.parm, gain_parm.is_bypass);
#if PARM_DEBUG
    log_debug("gain_parm.is_bypass %d, gain 0x%x", gain_parm.is_bypass, *(int *)&gain_parm.parm.gain);
#endif
#endif
    return 0;
}
int linein_gain_process_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if GAIN_PROCESS_EN
#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
    u8 mode_id = packet->data[0];
    memcpy(&linein_gain_parm, &packet->data[1], sizeof(linein_gain_parm));
    audio_gain_update_parm(AEID_LINEIN_GAIN, &linein_gain_parm.parm, linein_gain_parm.is_bypass);
#if PARM_DEBUG
    log_debug("linein_gain_parm.is_bypass %d, gain 0x%x", linein_gain_parm.is_bypass, *(int *)&linein_gain_parm.parm.gain);
#endif
#endif
#endif
    return 0;
}

int rl_gain_process_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if GAIN_PROCESS_EN
    u8 mode_id = packet->data[0];
    memcpy(&rl_gain_parm, &packet->data[1], sizeof(rl_gain_parm));
    audio_gain_update_parm(AEID_MUSIC_RL_GAIN, &rl_gain_parm.parm, rl_gain_parm.is_bypass);
#if PARM_DEBUG
    log_debug("rl_gain_parm.is_bypass %d, gain 0x%x", rl_gain_parm.is_bypass, *(int *)&rl_gain_parm.parm.gain);
#endif
#endif
    return 0;
}

void dynamic_eq_printf(DynamicEQParam_TOOL_SET *dy_parm)
{
#if PARM_DEBUG
    log_debug("dy_parm->is_bypass %d\n", dy_parm->is_bypass);
    for (int i = 0; i < dy_parm->nSection; i++) {
        log_debug("fc %d , Q 0x%x, gain:0x%x, type %d, attacktime %d, releaseTime %d, rmsTime %d, threshold 0x%x, ratio 0x%x, noisegate_threshold 0x%x, fixGain %x, algorithm %d\n",
                  dy_parm->effect_param[i].fc,
                  *(int *)&dy_parm->effect_param[i].Q,
                  *(int *)&dy_parm->effect_param[i].gain,
                  dy_parm->effect_param[i].type,
                  dy_parm->effect_param[i].attackTime,
                  dy_parm->effect_param[i].releaseTime,
                  dy_parm->effect_param[i].rmsTime,
                  *(int *)&dy_parm->effect_param[i].threshold,
                  *(int *)&dy_parm->effect_param[i].ratio,
                  *(int *)&dy_parm->effect_param[i].noisegate_threshold,
                  *(int *)&dy_parm->effect_param[i].fixGain,
                  dy_parm->effect_param[i].algorithm);
    }

    log_debug("nSection %d, detect_mode %d\n", dy_parm->nSection, dy_parm->detect_mode);
#endif
}


int dynamic_eq_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if TCFG_DYNAMIC_EQ_ENABLE
    u8 mode_id = packet->data[0];
    memcpy(&dynamic_eq, &packet->data[1], sizeof(dynamic_eq));

    DynamicEQParam_TOOL_SET *dy_parm = &dynamic_eq;//&packet->data[1];
    DynamicEQEffectParam *parm1 = dy_parm->effect_param;

    DynamicEQParam parm2 = {0};
    parm2.nSection = dy_parm->nSection;
    parm2.detect_mode = dy_parm->detect_mode;

    DynamicEQDetectionParam detectParm[2] = {0};
    detectParm[0].fc = dy_parm->effect_param[0].fc;
    detectParm[1].fc = dy_parm->effect_param[1].fc;

    dynamic_eq_printf(&dynamic_eq);
    audio_dynamic_eq_detection_update_parm(detectParm, dy_parm->is_bypass);//动态eq检测更新

    audio_dynamic_eq_update_parm((void *)parm1, (void *)&parm2, dy_parm->is_bypass);//动态eq更新
#endif
    return 0;
}

int phone_eq_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
    u32 eq_name = get_module_name(packet->cmd);//查询到相应模块
    float global_gain = 0;
    u8 mode_id = packet->data[0];
    log_debug("mode_id %d\n", mode_id);
    struct eq_seg_info *seg = (struct eq_seg_info *)&packet->data[1];

    if (seg->index == (u16) - 1) {
        memcpy(&global_gain, &seg->iir_type, sizeof(float));
        cur_eq_set_global_gain(eq_name, global_gain);
        log_info("global_gain 0x%x\n", *(int *)&global_gain);
    }

    void *tar_seg = NULL;
    u8 tar = packet->cmd - EFF_PHONE_WIDEBAND_EQ;
    log_debug("phone_mode tar %d\n", tar);
    if (seg->index == (u16) - 1) {
        phone_mode[tar].eq_parm.global_gain = global_gain;
    } else {
        tar_seg = &phone_mode[tar].eq_parm.seg[seg->index];
    }
    if (seg->index != (u16) - 1) {
#if PARM_DEBUG
        log_info("idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q);
#endif
        memcpy(tar_seg, seg, sizeof(struct eq_seg_info));
        cur_eq_set_update(eq_name, tar_seg, get_eq_nsection(eq_name), 0);
    }

    return 0;
}

int low_pass_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(SOUND_TRACK_2_P_X_CH_CONFIG) &&SOUND_TRACK_2_P_X_CH_CONFIG
    u8 mode_id = packet->data[0];
    memcpy(&low_pass_parm, &packet->data[1], sizeof(LowPassParam_TOOL_SET));
    cur_eq_set_update(AEID_MUSIC_RL_EQ, NULL, 1, 0);
#if PARM_DEBUG
    log_info("low_pass_parm.is_bypass %d\n", low_pass_parm.is_bypass);
    struct advance_iir *low_p = &low_pass_parm.low_pass;
    log_info("low_p->fc %d, low_p->order %d, low_p->type %d\n", low_p->fc, low_p->order, low_p->type);
#endif
#endif
    return 0;
}
int mic_eq_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    u32 eq_name = get_module_name(packet->cmd);//查询到相应模块
    float global_gain = 0;
    u8 mode_id = packet->data[0];
    log_debug("mode_id %d\n", mode_id);
    struct eq_seg_info *seg = (struct eq_seg_info *)&packet->data[1];

    if (seg->index == (u16) - 1) {
        memcpy(&global_gain, &seg->iir_type, sizeof(float));
        cur_eq_set_global_gain(eq_name, global_gain);
        log_info("global_gain 0x%x\n", *(int *)&global_gain);
    }

    void *tar_seg = NULL;
    u8 tar = packet->cmd - EFF_MIC_EQ0;
    log_debug("eff_mode tar %d\n", tar);
    if (seg->index == (u16) - 1) {
        eff_mode[mode_id - mic_mode_seq0].eq_parm[tar].global_gain = global_gain;
    } else {
        tar_seg = &eff_mode[mode_id - mic_mode_seq0].eq_parm[tar].seg[seg->index];
    }
    if (seg->index != (u16) - 1) {
#if PARM_DEBUG
        log_info("idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q);
#endif
        memcpy(tar_seg, seg, sizeof(struct eq_seg_info));
        cur_eq_set_update(eq_name, tar_seg, get_eq_nsection(eq_name), 1);
    }
#endif

    return 0;
}

int music_eq_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
    u32 eq_name = get_module_name(packet->cmd);//查询到相应模块
    float global_gain = 0;
    u8 mode_id = packet->data[0];
    log_debug("mode_id %d\n", mode_id);
    struct eq_seg_info *seg = (struct eq_seg_info *)&packet->data[1];

    if (seg->index == (u16) - 1) {
        memcpy(&global_gain, &seg->iir_type, sizeof(float));
        cur_eq_set_global_gain(eq_name, global_gain);
        log_info("global_gain 0x%x\n", *(int *)&global_gain);
    }

    void *tar_seg = NULL;
    if (packet->cmd == EFF_MUSIC_EQ) {
        if (seg->index == (u16) - 1) {
            music_mode.eq_parm.global_gain = global_gain;
        } else {
            tar_seg = &music_mode.eq_parm.seg[seg->index];
        }
    } else if (packet->cmd == EFF_MUSIC_EQ2) {
#if TCFG_DYNAMIC_EQ_ENABLE
        if (seg->index == (u16) - 1) {
            music_eq2_parm.global_gain = global_gain;
        } else {
            tar_seg = &music_eq2_parm.seg[seg->index];
        }
#endif
    } else if (packet->cmd == EFF_MUSIC_HIGH_BASS_EQ) {
        if (seg->index == (u16) - 1) {
            high_bass_eq_parm.global_gain = global_gain;
        } else {
            tar_seg = 	&high_bass_eq_parm.seg[seg->index];
        }
    }
    if (seg->index != (u16) - 1) {
#if PARM_DEBUG
        log_info("idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q);
#endif
        memcpy(tar_seg, seg, sizeof(struct eq_seg_info));
        cur_eq_set_update(eq_name, tar_seg, get_eq_nsection(eq_name), 0);
    }

    return 0;
}

int linein_eq_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
    u32 eq_name = get_module_name(packet->cmd);//查询到相应模块
    float global_gain = 0;
    u8 mode_id = packet->data[0];
    log_debug("mode_id %d\n", mode_id);
    struct eq_seg_info *seg = (struct eq_seg_info *)&packet->data[1];

    if (seg->index == (u16) - 1) {
        memcpy(&global_gain, &seg->iir_type, sizeof(float));
        cur_eq_set_global_gain(eq_name, global_gain);
        log_info("global_gain 0x%x\n", *(int *)&global_gain);
    }

    void *tar_seg = NULL;
    if (packet->cmd == EFF_LINEIN_EQ) {
        if (seg->index == (u16) - 1) {
            linein_mode.eq_parm.global_gain = global_gain;
        } else {
            tar_seg = &linein_mode.eq_parm.seg[seg->index];
        }
    }
    if (seg->index != (u16) - 1) {
#if PARM_DEBUG
        log_info("idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q);
#endif
        memcpy(tar_seg, seg, sizeof(struct eq_seg_info));
        cur_eq_set_update(eq_name, tar_seg, get_eq_nsection(eq_name), 0);
    }
#endif

    return 0;
}
int linein_wdrc_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
    u16 type = 0; //0:low  1:mid  2:high ,3:other band, 4:crossover
    u32 drc_name = get_module_name_and_index(packet->cmd, &type);
    wdrc_struct_TOOL_SET *wdrc_parm = (wdrc_struct_TOOL_SET *)&packet->data[1];
    void *tar_wdrc = NULL;

    log_debug("wdrc_parm type %d\n", type);
    struct mode_list *list = get_group_list(drc_name);
    if ((list->group_num > 1) && (type == (list->group_num - 1))) { ////最后一个存分频器系数
        CrossOverParam_TOOL_SET *parm = (CrossOverParam_TOOL_SET *)&packet->data[1];
        tar_wdrc = &linein_mode.drc_parm.crossover;
        memcpy(tar_wdrc, parm, sizeof(CrossOverParam_TOOL_SET));
        cur_crossover_set_update(drc_name, tar_wdrc);
        log_debug("way_num %d, N %d, low_freq %d, high_freq %d\n", parm->way_num, parm->N, parm->low_freq, parm->high_freq);
        return 0;
    } else {
        tar_wdrc = &linein_mode.drc_parm.wdrc_parm[type];
    }

    memcpy(tar_wdrc, wdrc_parm, sizeof(wdrc_struct_TOOL_SET));

    wdrc_printf(tar_wdrc);
    cur_drc_set_update(drc_name, type, tar_wdrc);
    cur_drc_set_bypass(drc_name, type, wdrc_parm->is_bypass);
#endif

    return 0;
}


int mic_wdrc_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    u16 type = 0; //0:low  1:mid  2:high ,3:other band, 4:crossover
    u32 drc_name = get_module_name_and_index(packet->cmd, &type);
    type = 0;//混响没有多带drc
    wdrc_struct_TOOL_SET *wdrc_parm = (wdrc_struct_TOOL_SET *)&packet->data[1];
    void *tar_wdrc = NULL;

    log_debug("wdrc_parm type %d\n", type);
    u8 mode_id = packet->data[0];
    u8 tar = packet->cmd - EFF_MIC_DRC0;
    tar_wdrc = &eff_mode[mode_id - mic_mode_seq0].drc_parm[tar];
    memcpy(tar_wdrc, wdrc_parm, sizeof(wdrc_struct_TOOL_SET));

    wdrc_printf(tar_wdrc);
    cur_drc_set_update(drc_name, type, tar_wdrc);
    cur_drc_set_bypass(drc_name, type, wdrc_parm->is_bypass);
#endif

    return 0;
}
int high_bass_wdrc_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
    u16 type = 0; //0:low  1:mid  2:high ,3:other band, 4:crossover
    u32 drc_name = get_module_name_and_index(packet->cmd, &type);
    type = 0;//混响没有多带drc
    wdrc_struct_TOOL_SET *wdrc_parm = (wdrc_struct_TOOL_SET *)&packet->data[1];
    void *tar_wdrc = NULL;

    log_debug("wdrc_parm type %d\n", type);
    u8 mode_id = packet->data[0];
    u8 tar = packet->cmd - EFF_MIC_DRC0;
    tar_wdrc = &eff_mode[mode_id - mic_mode_seq0].drc_parm[tar];
    memcpy(tar_wdrc, wdrc_parm, sizeof(wdrc_struct_TOOL_SET));

    wdrc_printf(tar_wdrc);
    cur_drc_set_update(drc_name, type, tar_wdrc);
    cur_drc_set_bypass(drc_name, type, wdrc_parm->is_bypass);

    return 0;
}

int music_wdrc_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
    u16 type = 0; //0:low  1:mid  2:high ,3:other band, 4:crossover
    u32 drc_name = get_module_name_and_index(packet->cmd, &type);
    wdrc_struct_TOOL_SET *wdrc_parm = (wdrc_struct_TOOL_SET *)&packet->data[1];
    void *tar_wdrc = NULL;

    log_debug("wdrc_parm type %d\n", type);
    struct mode_list *list = get_group_list(drc_name);
    if (type == (list->group_num - 1)) { ////最后一个存分频器系数
        CrossOverParam_TOOL_SET *parm = (CrossOverParam_TOOL_SET *)&packet->data[1];
        tar_wdrc = &music_mode.drc_parm.crossover;
        memcpy(tar_wdrc, parm, sizeof(CrossOverParam_TOOL_SET));
        cur_crossover_set_update(drc_name, tar_wdrc);
        log_debug("way_num %d, N %d, low_freq %d, high_freq %d\n", parm->way_num, parm->N, parm->low_freq, parm->high_freq);
        return 0;
    } else {
        tar_wdrc = &music_mode.drc_parm.wdrc_parm[type];
    }

    memcpy(tar_wdrc, wdrc_parm, sizeof(wdrc_struct_TOOL_SET));

    wdrc_printf(tar_wdrc);
    cur_drc_set_update(drc_name, type, tar_wdrc);
    cur_drc_set_bypass(drc_name, type, wdrc_parm->is_bypass);

    return 0;
}


int music_rl_wdrc_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
    u16 type = 0; //0:low  1:mid  2:high ,3:other band, 4:crossover
    u32 drc_name = get_module_name_and_index(packet->cmd, &type);
    wdrc_struct_TOOL_SET *wdrc_parm = (wdrc_struct_TOOL_SET *)&packet->data[1];
    void *tar_wdrc = NULL;

    log_debug("wdrc_parm type %d\n", type);
    struct mode_list *list = get_group_list(drc_name);
    if (type == (list->group_num - 1)) { ////最后一个存分频器系数
        CrossOverParam_TOOL_SET *parm = (CrossOverParam_TOOL_SET *)&packet->data[1];
        tar_wdrc = &rl_drc_parm.crossover;
        memcpy(tar_wdrc, parm, sizeof(CrossOverParam_TOOL_SET));
        cur_crossover_set_update(drc_name, tar_wdrc);
        log_debug("way_num %d, N %d, low_freq %d, high_freq %d\n", parm->way_num, parm->N, parm->low_freq, parm->high_freq);
        return 0;
    } else {
        tar_wdrc = &rl_drc_parm.wdrc_parm[type];
    }

    memcpy(tar_wdrc, wdrc_parm, sizeof(wdrc_struct_TOOL_SET));

    wdrc_printf(tar_wdrc);
    cur_drc_set_update(drc_name, type, tar_wdrc);
    cur_drc_set_bypass(drc_name, type, wdrc_parm->is_bypass);
#endif
    return 0;
}


int phone_wdrc_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
    u16 type = 0; //0:low  1:mid  2:high ,3:other band, 4:crossover
    u32 drc_name = get_module_name_and_index(packet->cmd, &type);
    wdrc_struct_TOOL_SET *wdrc_parm = (wdrc_struct_TOOL_SET *)&packet->data[1];
    type = 0;//通话下行使用type位置做更新
    void *tar_wdrc = NULL;
    log_debug("wdrc_parm type %d\n", type);
    u8 tar = packet->cmd - EFF_PHONE_WIDEBAND_DRC;
    tar_wdrc = &phone_mode[tar].drc_parm;
    memcpy(tar_wdrc, wdrc_parm, sizeof(wdrc_struct_TOOL_SET));
    wdrc_printf(tar_wdrc);
    cur_drc_set_update(drc_name, type, tar_wdrc);
    cur_drc_set_bypass(drc_name, type, wdrc_parm->is_bypass);

    return 0;
}


int noise_gate_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    u8 mode_id = packet->data[0];
    memcpy(&eff_mode[mode_id - mic_mode_seq0].noise_gate_parm, &packet->data[1], sizeof(NoiseGateParam_TOOL_SET));
    NoiseGateParam_TOOL_SET *parmt = &eff_mode[mode_id - mic_mode_seq0].noise_gate_parm;
    noisegate_update_parm(&parmt->parm, parmt->is_bypass);

#if PARM_DEBUG
    noisegate_update_param *parm = &parmt->parm;
    log_debug("noise gate attact_time %d,releasetime %d, threshold %d, low_th_gain %d\n",
              parm->attackTime, parm->releaseTime, parm->threshold, parm->low_th_gain);
#endif
#endif
    return 0;
}

int howling_pitch_shift_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    u8 mode_id = packet->data[0];
    memcpy(&eff_mode[mode_id - mic_mode_seq0].howlingps_parm, &packet->data[1], sizeof(HowlingPs_PARM_TOOL_SET));
    HowlingPs_PARM_TOOL_SET *parmt = &eff_mode[mode_id - mic_mode_seq0].howlingps_parm;
    howling_pitch_shift_update_parm(&parmt->parm, parmt->is_bypass);
#if PARM_DEBUG
    HOWLING_PITCHSHIFT_PARM *parm = &parmt->parm;
    log_debug("ps_parm %d, fe_parm %d, effect_v %d\n", parm->ps_parm, parm->fe_parm, parm->effect_v);
#endif
#endif
    return 0;
}

int notchhowline_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    u8 mode_id = packet->data[0];
    memcpy(&eff_mode[mode_id - mic_mode_seq0].notchhowling_parm, &packet->data[1], sizeof(NotchHowlingParam_TOOL_SET));
    NotchHowlingParam_TOOL_SET *parmt = &eff_mode[mode_id - mic_mode_seq0].notchhowling_parm;
    notchhowline_update_parm(&parmt->parm, parmt->is_bypass);

#if PARM_DEBUG
    NotchHowlingParam *parm = &parmt->parm;
    log_debug("parmt->is_bypass %d\n", parmt->is_bypass);
    log_debug("Q 0x%x, gain 0x%x, fade_n %d\n", *(int *)&parm->Q, *(int *)&parm->gain, parm->fade_n);
#endif
#endif
    return 0;
}


int reverb_parm_analyze(EFF_ONLINE_PACKET *packet)
{
#if 0
    u8 mode_id = packet->data[0];
    memcpy(&eff_mode[mode_id - mic_mode_seq0].reverb_parm, &packet->data[1], sizeof(REVERBN_PARM_TOOL_SET));
    REVERBN_PARM_TOOL_SET *parmt = &eff_mode[mode_id - mic_mode_seq0].reverb_parm;
    reverb_update_parm(&parmt->parm);
    REVERBN_PARM_SET *parm = &parmt->parm;
    log_debug("dry %d,\n wet %d,\n delay %d,\n rot60 %d,\n Erwet %d,\nErfactor %d,\n Ewidth %d,\n Ertolate %d,\nErbasslpf %d,\nErbassB %d,\n predelay %d,\nallpassfeedback %d,\ndiffusion %d,\n dampinglpf %d,\n basslpf %d,\n bassB %d,\n lowcut %d\n",
              parm->dry,
              parm->wet,
              parm->delay,
              parm->rot60,
              parm->Erwet,
              parm->Erfactor,
              parm->Ewidth,
              parm->Ertolate,
              parm->Erbasslpf,
              parm->ErbassB,
              parm->predelay,
              parm->allpassfeedback,
              parm->diffusion,
              parm->dampinglpf,
              parm->basslpf,
              parm->bassB,
              parm->lowcut
             );
#endif
    return 0;
}

int plate_reverb_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    u8 mode_id = packet->data[0];
    memcpy(&eff_mode[mode_id - mic_mode_seq0].plate_reverb_parm, &packet->data[1], sizeof(Plate_reverb_TOOL_SET));
    Plate_reverb_TOOL_SET *parmt = &eff_mode[mode_id - mic_mode_seq0].plate_reverb_parm;
    plate_reverb_update_parm(&parmt->parm, parmt->is_bypass);

#if PARM_DEBUG
    Plate_reverb_parm *parm = &parmt->parm;
    log_debug("wet %d\n dry %d\n pre_delay %d\n highcutoff %d\n diffusion %d\n decayfactor %d\n highfrequencydamping %d\n modulate %d\n roomsize %d\n", parm->wet, parm->dry, parm->pre_delay, parm->highcutoff, parm->diffusion, parm->decayfactor, parm->highfrequencydamping, parm->modulate, parm->roomsize);
#endif
#endif
    return 0;
}


int echo_parm_analyze(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    u8 mode_id = packet->data[0];
    memcpy(&eff_mode[mode_id - mic_mode_seq0].echo_parm, &packet->data[1], sizeof(EF_ECHO_TOOL_SET));
    EF_ECHO_TOOL_SET *parmt = &eff_mode[mode_id - mic_mode_seq0].echo_parm;
    echo_updata_parm(&parmt->parm, parmt->is_bypass);
#if PARM_DEBUG
    ECHO_PARM_SET *parm = &parmt->parm;
    log_debug("delay %d, decayval %d, filt_enable %d, lpf_cutoff %d, wetgain %d, drygain %d\n",
              parm->delay, parm->decayval, parm->filt_enable, parm->lpf_cutoff, parm->wetgain, parm->drygain);
#endif
#endif
    return 0;
}



void eff_send_packet(void *priv, u32 id, u8 *packet, int size, u8 sq)
{
    all_assemble_package_send_to_pc(id, sq, packet, size);
}

void alive_timer_send_packet(void *p)
{
    u32 T = 2;
    u8 sq  = 0;
    eff_send_packet(NULL, T, (u8 *)"empty", 5, sq);
}


int eff_tool_get_cfg_file_size(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
    struct file_s {
        int id;
        int fileid;
    };
    struct file_s fs;
    memcpy(&fs, packet, sizeof(struct file_s));
    u32 file_size  = 0;
    if (fs.fileid == EFF_CFG_FILE_ID) {
        FILE *file = NULL;
        file = fopen(EFF_CFG_FILE_NAME, "r");
        if (!file) {
            log_error("EFF_CFG_FILE_NAME err %s\n", EFF_CFG_FILE_NAME);
        } else {
            file_size = flen(file);
            fclose(file);
        }
        eff_send_packet(NULL, REPLY_TO_TOOL, (u8 *)&file_size, sizeof(u32), sq);
    }
    return 1;
}
int eff_tool_get_cfg_file_data(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
    struct file_s {
        int id;
        int fileid;
        int offset;
        int size;
    };
    struct file_s fs;
    memcpy(&fs, packet, sizeof(struct file_s));
    if (fs.fileid == (int)EFF_CFG_FILE_ID) {
        FILE *file = NULL;
        file = fopen(EFF_CFG_FILE_NAME, "r");
        if (!file) {
            return -EINVAL;
        }

        fseek(file, fs.offset, SEEK_SET);
        u8 *data = malloc(fs.size);
        if (!data) {
            fclose(file);
            return -EINVAL;
        }
        int ret = fread(file, data, fs.size);
        if (ret != fs.size) {
        }
        fclose(file);
        eff_send_packet(NULL, REPLY_TO_TOOL, (u8 *)data, fs.size, sq);
        free(data);
    }
    return 1;
}


static s32 eff_online_update(void *_packet)
{
    u8 *ptr = _packet;
    u8 id = ptr[0];
    u8 sq = ptr[1];
    int res = -1;
    EFF_ONLINE_PACKET *packet = (EFF_ONLINE_PACKET *)&ptr[2];
    struct cmd_interface *p = NULL;
#if PARM_DEBUG
    if (packet->cmd != 0x4) {//离线查询
        log_debug(" id 0x%x, sq %d\n", id, sq);
        log_debug("_cmd:0x%x mode_id %d\n", packet->cmd, packet->data[0]);
    }
#endif
    if (!alive_timer) {
        alive_timer = sys_timer_add(NULL, alive_timer_send_packet, 900);//定时往上位机推送，可能对工具离线的消息有帮助
    }
    list_for_each_cmd_interface(p) {
        if (p->cmd == packet->cmd) {
            if (p->cmd_deal) {
                res = p->cmd_deal(packet, REPLY_TO_TOOL, sq);
            }
        }
    }
    return res;
}

int eff_tool_get_version(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
    struct _ver_info {
        char sdkname[16];
        u8 eqver[4];
    };
    struct _ver_info _ver_info = {0};
    memcpy(_ver_info.sdkname, eff_sdk_name, sizeof(eff_sdk_name));
    memcpy(_ver_info.eqver, eff_eq_ver, sizeof(eff_eq_ver));
    eff_send_packet(NULL, REPLY_TO_TOOL, (u8 *)&_ver_info, sizeof(struct _ver_info), sq);
    return 1;
}
int eff_tool_set_channge_mode(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    struct cmd {
        int id;
        int modeId;
    };
    struct cmd cmd;
    memcpy(&cmd, packet, sizeof(struct cmd));
    log_debug("cmd.modeId %d\n", cmd.modeId);
    if ((cmd.modeId >= mic_mode_seq0) && (cmd.modeId <= mic_mode_seq7)) { //混响0~7
        set_mic_reverb_mode_by_id(cmd.modeId - mic_mode_seq0);
    }
#endif
    eff_send_packet(NULL, REPLY_TO_TOOL, (u8 *)"OK", 2, sq);
    return 1;
}
//在线检测应答
int eff_tool_set_inquire(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
    //eff_send_packet(NULL, id, (u8 *)"OK", 2, sq);//OK表示需要重传，NO表示不需要重传,ER还是表示未知命令
    eff_send_packet(NULL, id, (u8 *)"NO", 2, sq);//OK表示需要重传，NO表示不需要重传,ER还是表示未知命令
    return 1;
}

int eff_tool_resync_parm_begin(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    eff_mode_save = get_mic_eff_mode();
#endif
    eff_send_packet(NULL, REPLY_TO_TOOL, (u8 *)"OK", 2, sq);
    return 1;
}

int eff_tool_resync_parm_end(EFF_ONLINE_PACKET *packet, u8 id, u8 sq)
{
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    u8 eff_mode = eff_mode_save;
    set_mic_reverb_mode_by_id(eff_mode);//还原混响的模式
#endif
    eff_send_packet(NULL, REPLY_TO_TOOL, (u8 *)"OK", 2, sq);
    return 1;
}


static void effect_tool_callback(u8 *_packet, u32 size)
{
    int res = 0;
    u8 *ptr = _packet;
    u8 id = ptr[0];
    u8 sq = ptr[1];
    u8 *packet = (u8 *)&ptr[2];
    ASSERT(((int)packet & 0x3) == 0, "buf %x size %d\n", (unsigned int)packet, size - 2);

    res = eff_online_update(ptr);
    if (!res) {
        log_debug("Ack");
        eff_send_packet(NULL, REPLY_TO_TOOL, (u8 *)"OK", 2, sq);
    } else if (res != 1) {
        log_debug("Nack");
        eff_send_packet(NULL, REPLY_TO_TOOL, (u8 *)"ER", 2, sq);
    }
}

REGISTER_DETECT_TARGET(eff_adj_target) = {
    .id         = 0x11,//EFF_CONFIG_ID,
    .tool_message_deal   = effect_tool_callback,
};

//在线调试不进power down
static u8 effect_tool_idle_query(void)
{
#if TCFG_EFFECT_TOOL_ENABLE
    return 1;
#endif
    return 0;
}

REGISTER_LP_TARGET(effect_adj_lp_target) = {
    .name = "effect_adj",
    .is_idle = effect_tool_idle_query,
};

#define INDEX_HEAD  0xFFFD
int eff_file_exist()
{
    FILE *file = fopen((const char *)EFF_CFG_FILE_NAME, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

int eff_file_analyze(u32 mode_id, u16 group_id, void *data_buf, u32 buf_len)
{
#if PARM_DEBUG
    log_debug("mode_id 0x%x, group_id 0x%x\n", mode_id, group_id);
#endif
    int ret = 0;
    u8 *crc_buf = NULL;
    FILE *file = NULL;
    file = fopen((const char *)EFF_CFG_FILE_NAME, "r");
    if (!file) {
        log_error("eff file open err\n");
        return  -ENOENT;
    }
    u8 FV = 0;
    if (1 != fread(file, &FV, 1)) {
        ret = -1;

        goto err_exit;
    }

    u8 ver[4] = {0};
    if (4 != fread(file, ver, 4)) {
        ret = -2;
        goto err_exit;
    }

    if (memcmp(ver, eff_eq_ver, sizeof(eff_eq_ver))) {
        log_error("eff ver err \n");
        log_error_hexdump((unsigned char *)ver, 4);
        fseek(file, 0, SEEK_SET);
        ret = -3;
        goto err_exit;
    }

    u16 index_head = 0;//索引头
    if (2 != fread(file, &index_head, 2)) {
        ret = -4;
        goto err_exit;
    }

    if (index_head != INDEX_HEAD) {
        ret = -5;
        goto err_exit;
    }
    u16 index_len = 0;//索引区域长度
    if (2 != fread(file, &index_len, 2)) {
        ret = -6;
        goto err_exit;
    }
    u8 mode_num = 0;//模式个数
    if (1 != fread(file, &mode_num, 1)) {
        ret = -7;
        goto err_exit;
    }

    u16 off0 = 0;	//组索引区域的偏移
    u16 off1 = 0;	//模式索引区域的偏移
    for (int mode_cnt = 0; mode_cnt < mode_num; mode_cnt++) {
        u16 mode_seq  = 0;//模式标识
        if (2 != fread(file, &mode_seq, 2)) {
            ret = -8;
            goto err_exit;
        }
        if (2 != fread(file, &off0, 2)) {
            ret = -9;
            goto err_exit;
        }
        if (2 != fread(file, &off1, 2)) {
            ret = -10;
            goto err_exit;
        }
        if (mode_seq != mode_id) { //查询到相应的模式
            continue;
        } else {
            break;
        }
    }
    fseek(file, off0, SEEK_SET);//偏移到组索引区域

    u8 group_num = 0;
    if (1 != fread(file, &group_num, 1)) {
        ret = -11;
        goto err_exit;
    }
    u16 g_id = 0;//模块id
    u16 group_id_addr = 0;//模块id的内容地址
    int group_cnt = 0;
    for (group_cnt = 0; group_cnt < group_num; group_cnt++) {
        if (2 != fread(file, &g_id, 2)) {
            ret = -12;
            goto err_exit;
        }
        if (2 != fread(file, &group_id_addr, 2)) {
            ret = -13;
            goto err_exit;
        }
        if (g_id != group_id) {
            continue;
        } else {
            break;
        }
    }
    if (group_cnt == group_num) {
        ret = -14;
#if PARM_DEBUG
        log_error("seek group id addr err\n");
#endif
        goto err_exit;
    }
    fseek(file, group_id_addr, SEEK_SET);//偏移到模式id的具体内容

    u16 crc16 = 0;
    u16 verify_group_id = 0;
    u16 group_len;
    if (2 != fread(file, &crc16, 2)) {
        ret = -15;
        goto err_exit;
    }
    int cur_addr = fpos(file);
    if (2 != fread(file, &verify_group_id, 2)) {
        ret = -16;
        goto err_exit;
    }
    if (2 != fread(file, &group_len, 2)) {
        ret = -17;
        goto err_exit;
    }
    if (!group_len) {
        ret = -18;
        goto err_exit;
    }
    if (verify_group_id != group_id) {
        ret = -19;
        log_error("verify_group_id %x != group_id %x\n", verify_group_id, group_id);
        log_error("verify groud id err\n");
        goto err_exit;
    }

    fseek(file, cur_addr, SEEK_SET);
    u16 size = sizeof(crc16) + sizeof(group_len);
    crc_buf = zalloc(group_len  + size);
    u8 *group_buf = &crc_buf[size];
    if ((group_len + size) != fread(file, crc_buf, group_len + size)) {
        ret = -20;
        goto err_exit;
    }
    u16 calc_crc = CRC16(crc_buf, group_len + size); //id + len +playload
    if (crc16 != calc_crc) {
        ret = -21;
        log_error("crc16 %x, calc_crc %x\n", crc16, calc_crc);
        goto err_exit;
    }
    if (!data_buf) {
        ret = -22;
#if PARM_DEBUG
        log_error("input data buf NULL\n");
#endif
        goto err_exit;
    }
    if (group_len <= buf_len) {
        memcpy(data_buf, group_buf, group_len);
    } else {
        ret = -23;
        log_error("input data buf len : %d < group_len:%d\n", buf_len, group_len);
        goto err_exit;
    }

err_exit:
    if (crc_buf) {
        free(crc_buf);
        crc_buf = NULL;
    }
    fclose(file);

#if PARM_DEBUG
    if (ret) {
        log_error("ret :%d, analyze eff file err, please check it\n", ret);
    }
#endif
    return 0;
}

void phone_eff_analyze_data(void)
{
    u16 phone_eq_name[] = {AEID_ESCO_DL_EQ};
    u16 aec_eq_name[] = {AEID_ESCO_UL_EQ};
    u8 index = 0;
    void *tar_buf = NULL;
    u16 tar_len = 0;
    u16 group_id = 0;
    u16 mode_seq = 0;

    mode_seq = phone_mode_seq;
    for (int i = 0; i < ARRAY_SIZE(phone_eq_name); i++) {
        struct mode_list *list = get_group_list(phone_eq_name[i]);
        for (int j = 0; j < list->group_num; j++) {
            group_id = get_group_id(phone_eq_name[i], j);
            index = group_id - EFF_PHONE_WIDEBAND_EQ;
            tar_buf = &phone_mode[index].eq_parm;
            tar_len = sizeof(phone_mode[index].eq_parm);
            eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
        }
    }
    mode_seq = aec_mode_seq;
    for (int i = 0; i < ARRAY_SIZE(aec_eq_name); i++) {
        struct mode_list *list = get_group_list(aec_eq_name[i]);
        for (int j = 0; j < list->group_num; j++) {
            group_id = get_group_id(aec_eq_name[i], j);
            index = group_id - EFF_PHONE_WIDEBAND_EQ;
            tar_buf = &phone_mode[index].eq_parm;
            tar_len = sizeof(phone_mode[index].eq_parm);
            eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
        }
    }

    u16 phone_drc_name[] = {AEID_ESCO_DL_DRC};
    u16 aec_drc_name[] = {AEID_ESCO_UL_DRC};
    mode_seq = phone_mode_seq;
    for (int i = 0; i < ARRAY_SIZE(phone_drc_name); i++) {
        struct mode_list *list = get_group_list(phone_drc_name[i]);
        for (int j = 0; j < list->group_num; j++) {
            group_id = get_group_id(phone_drc_name[i], j);
            index = group_id - EFF_PHONE_WIDEBAND_DRC;
            tar_buf = &phone_mode[index].drc_parm;
            tar_len = sizeof(phone_mode[index].drc_parm);
            eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
            wdrc_printf(tar_buf);
        }
    }

    mode_seq = aec_mode_seq;
    for (int i = 0; i < ARRAY_SIZE(aec_drc_name); i++) {
        struct mode_list *list = get_group_list(aec_drc_name[i]);
        for (int j = 0; j < list->group_num; j++) {
            group_id = get_group_id(aec_drc_name[i], j);
            index = group_id - EFF_PHONE_WIDEBAND_DRC;
            tar_buf = &phone_mode[index].drc_parm;
            tar_len = sizeof(phone_mode[index].drc_parm);
            eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
            wdrc_printf(tar_buf);
        }
    }
}

void music_eq_printf(void *_parm)
{
#if PARM_DEBUG
    puts("----------------------- music eq ----------------------\n");
    struct music_eq_tool *parm =  _parm;
    log_debug("global_gain:0x%x\n", *(int *)&parm->global_gain);
    log_debug("seg_num:%d\n", parm->seg_num);
    struct eq_seg_info *seg;
    for (int i = 0; i < parm->seg_num; i++) {
        seg = &parm->seg[i];
        log_debug("idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q);
    }
#endif
}
void music_eq2_printf(void *_parm)
{
#if PARM_DEBUG
    puts("----------------------- music eq2 ----------------------\n");
    struct music_eq2_tool *parm =  _parm;
    log_debug("global_gain:0x%x\n", *(int *)&parm->global_gain);
    log_debug("seg_num:%d\n", parm->seg_num);
    struct eq_seg_info *seg;
    for (int i = 0; i < parm->seg_num; i++) {
        seg = &parm->seg[i];
        log_debug("idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q);
    }
#endif
}

void music_eff_analyze_data(void)
{
    u8 index = 0;
    void *tar_buf = NULL;
    u16 tar_len = 0;
    u16 group_id = 0;
    u16 mode_seq = music_mode_seq0;


#if AUDIO_VBASS_CONFIG
    tar_buf = &vbass_prev_gain_parm;
    tar_len = sizeof(vbass_prev_gain_parm);
    eff_file_analyze(mode_seq, EFF_MUSIC_VBASS_PREV_GAIN, tar_buf, tar_len);

    tar_buf = &vbass_parm;
    tar_len = sizeof(vbass_parm);
    eff_file_analyze(mode_seq, EFF_MUSIC_VBASS, tar_buf, tar_len);
#endif

#if defined(TCFG_EQ_ENABLE) && TCFG_EQ_ENABLE
//eq
    group_id = get_group_id(AEID_MUSIC_EQ, 0);
    tar_buf = &music_mode.eq_parm;
    tar_len = sizeof(music_mode.eq_parm);
    eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
    music_eq_printf(tar_buf);
#endif
//drc
    struct mode_list *list = get_group_list(AEID_MUSIC_DRC);
    if (list) {
        for (int i = 0; i < list->group_num; i++) {
            group_id = list->group_id[i];
            if (i == (list->group_num - 1)) { //最后一个存分频器系数
                tar_buf = &music_mode.drc_parm.crossover;
                tar_len = sizeof(music_mode.drc_parm.crossover);
                eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
#if PARM_DEBUG
                CrossOverParam_TOOL_SET *parm = tar_buf;
                log_debug("way_num %d, N %d, low_freq %d, high_freq %d\n", parm->way_num, parm->N, parm->low_freq, parm->high_freq);
#endif
            } else {
                tar_buf = &music_mode.drc_parm.wdrc_parm[i];
                tar_len = sizeof(music_mode.drc_parm.wdrc_parm[i]);
                eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
                wdrc_printf(tar_buf);
            }
        }
    }

#if TCFG_DYNAMIC_EQ_ENABLE
    group_id = get_group_id(AEID_MUSIC_EQ2, 0);
    tar_buf = &music_eq2_parm;
    tar_len = sizeof(music_eq2_parm);
    eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
    music_eq2_printf(tar_buf);

//dynamic eq
    group_id = EFF_MUSIC_DYNAMIC_EQ;//get_group_id(AEID_MUSIC_EQ2, 0);
    tar_buf = &dynamic_eq;
    tar_len = sizeof(DynamicEQParam_TOOL_SET);
    eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
    dynamic_eq_printf(tar_buf);
#endif

#if GAIN_PROCESS_EN
//gain
    group_id = EFF_MUSIC_GAIN;
    tar_buf = &gain_parm;
    tar_len = sizeof(gain_parm);
    eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
#if PARM_DEBUG
    log_debug("gain_parm.is_bypass %d, gain 0x%x", gain_parm.is_bypass, *(int *)&gain_parm.parm.gain);
#endif
#endif

#if defined(SOUND_TRACK_2_P_X_CH_CONFIG) &&SOUND_TRACK_2_P_X_CH_CONFIG
//low pass
    group_id =  EFF_MUSIC_RL_RR_LOW_PASS;
    tar_buf = &low_pass_parm;
    tar_len = sizeof(low_pass_parm);
    eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
#if PARM_DEBUG
    log_info("low_pass_parm.is_bypass %d\n", low_pass_parm.is_bypass);
    struct advance_iir *low_p = &low_pass_parm.low_pass;
    log_info("low_p->fc %d, low_p->order %d, low_p->type %d\n", low_p->fc, low_p->order, low_p->type);
#endif
#endif

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
//rl_wdrc
    u8 type = 0;
    group_id = EFF_MUSIC_RL_LOW_DRC;
    tar_buf = &rl_drc_parm.wdrc_parm[type];
    tar_len = sizeof(rl_drc_parm.wdrc_parm[type]);
    eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
    wdrc_printf(tar_buf);
#endif
#if GAIN_PROCESS_EN
//rl_rr_gain
    group_id = EFF_MUSIC_RL_GAIN;
    tar_buf = &rl_gain_parm;
    tar_len = sizeof(rl_gain_parm);
    eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
#if PARM_DEBUG
    log_debug("rl_gain_parm.is_bypass %d, gain 0x%x", rl_gain_parm.is_bypass, *(int *)&rl_gain_parm.parm.gain);
#endif
#endif
}

#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
void linein_eff_analyze_data(void)
{
    u8 index = 0;
    void *tar_buf = NULL;
    u16 tar_len = 0;
    u16 group_id = 0;
    u16 mode_seq = linein_mode_seq;
//eq
    group_id = get_group_id(AEID_LINEIN_EQ, 0);
    tar_buf = &linein_mode.eq_parm;
    tar_len = sizeof(linein_mode.eq_parm);
    eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
    music_eq_printf(tar_buf);
//drc
    struct mode_list *list = get_group_list(AEID_LINEIN_DRC);
    if (list) {
        for (int i = 0; i < list->group_num; i++) {
            group_id = list->group_id[i];
            if ((list->group_num > 1) && (i == (list->group_num - 1))) { //最后一个存分频器系数
                tar_buf = &linein_mode.drc_parm.crossover;
                tar_len = sizeof(linein_mode.drc_parm.crossover);
                eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
#if PARM_DEBUG
                CrossOverParam_TOOL_SET *parm = tar_buf;
                log_debug("way_num %d, N %d, low_freq %d, high_freq %d\n", parm->way_num, parm->N, parm->low_freq, parm->high_freq);
#endif
            } else {
                tar_buf = &linein_mode.drc_parm.wdrc_parm[i];
                tar_len = sizeof(linein_mode.drc_parm.wdrc_parm[i]);
                eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
                wdrc_printf(tar_buf);
            }
        }
    }

#if GAIN_PROCESS_EN
//gain
    group_id = EFF_LINEIN_GAIN;
    tar_buf = &gain_parm;
    tar_len = sizeof(gain_parm);
    eff_file_analyze(mode_seq, group_id, tar_buf, tar_len);
#if PARM_DEBUG
    log_debug("gain_parm.is_bypass %d, gain 0x%x", gain_parm.is_bypass, *(int *)&gain_parm.parm.gain);
#endif
#endif
}
#endif


extern struct phone_parm_tool_set phone_mode[4];
extern int eff_file_analyze(u32 mode_id, u16 group_id, void *data_buf, u32 buf_len);
void wdrc_printf(void *_wdrc)
{
#if PARM_DEBUG
    puts("---------------------- drc -----------------------\n");
    wdrc_struct_TOOL_SET *twdrc = _wdrc;
    struct wdrc_struct *wdrc = &twdrc->parm;
    printf("wdrc->is_bypass %d\n", twdrc->is_bypass);
    printf("wdrc attacktime %d, releasetime %d threshold_num %d rms_time %d algorithm%d, mode%d ,0x%x,0x%x\n",  wdrc->attacktime, wdrc->releasetime, wdrc->threshold_num, wdrc->rms_time, wdrc->algorithm, wdrc->mode, *(int *)&wdrc->inputgain, *(int *)&wdrc->outputgain);
    struct threshold_group  *tt = (struct threshold_group *)wdrc->threshold;
    for (int i = 0; i < wdrc->threshold_num; i++) {
        printf("in_threshold  0x%x, out_threshold 0x%x\n", *(int *)&tt[i].in_threshold, *(int *)&tt[i].out_threshold);
    }
#endif
}

void eq_printf(void *_parm)
{
#if PARM_DEBUG
    puts("----------------------- eq ----------------------\n");
    struct eq_tool *parm =  _parm;
    printf("global_gain:0x%x\n", *(int *)&parm->global_gain);
    printf("seg_num:%d\n", parm->seg_num);
    struct eq_seg_info *seg;// = parm->seg;
    for (int i = 0; i < parm->seg_num; i++) {
        seg = &parm->seg[i];
        printf("idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q);
    }
#endif
}


#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
//文件效果还原到 相应结构体
void  mic_eff_analyze_data(u8 tar_mode)
{
#if PARM_DEBUG
    log_debug("------------------------------ eff mode %d\n", tar_mode);
#endif
    u8 index = tar_mode;
    void *tar_buf = NULL;//
    u16 tar_len = 0;
    u16 group_id = 0;
//noisegate
    tar_buf = (void *)&eff_mode[index].noise_gate_parm;
    tar_len = sizeof(eff_mode[index].noise_gate_parm);
    eff_file_analyze(eff_mode_seq[index], EFF_MIC_NOISEGATE, tar_buf, tar_len);
//howlingps_parm
    tar_buf = (void *)&eff_mode[index].howlingps_parm;
    tar_len = sizeof(eff_mode[index].howlingps_parm);
    eff_file_analyze(eff_mode_seq[index], EFF_MIC_HOWLINE_PS, tar_buf, tar_len);
//notchowling_parm
    tar_buf = (void *)&eff_mode[index].notchhowling_parm;
    tar_len = sizeof(eff_mode[index].notchhowling_parm);
    eff_file_analyze(eff_mode_seq[index], EFF_MIC_NOTCH_HOWLING, tar_buf, tar_len);
//voice_changer
#if defined(TCFG_MIC_VOICE_CHANGER_ENABLE) && TCFG_MIC_VOICE_CHANGER_ENABLE
    tar_buf = (void *)&eff_mode[index].voicechanger_parm;
    tar_len = sizeof(eff_mode[index].voicechanger_parm);
    eff_file_analyze(eff_mode_seq[index], EFF_MIC_VOICE_CHANGER, tar_buf, tar_len);
#endif

//echo
    tar_buf = (void *)&eff_mode[index].echo_parm;
    tar_len = sizeof(eff_mode[index].echo_parm);
    eff_file_analyze(eff_mode_seq[index], EFF_MIC_ECHO, tar_buf, tar_len);
//palte_reverb
    tar_buf = (void *)&eff_mode[index].plate_reverb_parm;
    tar_len = sizeof(eff_mode[index].plate_reverb_parm);
    eff_file_analyze(eff_mode_seq[index], EFF_MIC_PLATE_REVERB, tar_buf, tar_len);
    Plate_reverb_TOOL_SET *parm = tar_buf;


//eq
    for (int i = 0; i < ARRAY_SIZE(eq_name); i++) {
        group_id = get_group_id(eq_name[i], 0);
        tar_buf = (void *)&eff_mode[index].eq_parm[i];
        tar_len = sizeof(eff_mode[index].eq_parm[i]);
        eff_file_analyze(eff_mode_seq[index], group_id, tar_buf, tar_len);
        eq_printf(tar_buf);
    }
//wdrc
    for (int i = 0; i < ARRAY_SIZE(drc_name); i++) {
        group_id = get_group_id(drc_name[i], 0);
        tar_buf = (void *)&eff_mode[index].drc_parm[i];
        tar_len = sizeof(eff_mode[index].drc_parm[i]);
        eff_file_analyze(eff_mode_seq[index], group_id, tar_buf, tar_len);
        wdrc_printf(tar_buf);
    }

#if GAIN_PROCESS_EN
//gain
    group_id = EFF_MIC_GAIN;
    tar_buf = &eff_mode[index].gain_parm;
    tar_len = sizeof(eff_mode[index].gain_parm);
    eff_file_analyze(eff_mode_seq[index], group_id, tar_buf, tar_len);
#if PARM_DEBUG
    log_debug("gain_parm.is_bypass %d, gain 0x%x", gain_parm.is_bypass, *(int *)&gain_parm.parm.gain);
#endif
#endif
}
#endif
/**
 *注册相应模块的解析函数,返回值0成功，返回小于0失败
 * */
REGISTER_CMD_TARGET(mic_g) = {
    .cmd = EFF_MIC_GAIN,
    .cmd_deal = mic_gain_parm_analyze,
};
REGISTER_CMD_TARGET(music_g) = {
    .cmd = EFF_MUSIC_GAIN,
    .cmd_deal = gain_process_parm_analyze,
};
REGISTER_CMD_TARGET(music_rl_g) = {
    .cmd = EFF_MUSIC_RL_GAIN,
    .cmd_deal = rl_gain_process_parm_analyze,
};


REGISTER_CMD_TARGET(dyeq) = {
    .cmd = EFF_MUSIC_DYNAMIC_EQ,
    .cmd_deal = dynamic_eq_parm_analyze,
};
REGISTER_CMD_TARGET(echo) = {
    .cmd = EFF_MIC_ECHO,
    .cmd_deal = echo_parm_analyze,
};
REGISTER_CMD_TARGET(p_reverb) = {
    .cmd = EFF_MIC_PLATE_REVERB,
    .cmd_deal = plate_reverb_parm_analyze,
};
REGISTER_CMD_TARGET(notch_howling) = {
    .cmd = EFF_MIC_NOTCH_HOWLING,
    .cmd_deal = notchhowline_parm_analyze,
};
REGISTER_CMD_TARGET(howling_ps) = {
    .cmd = EFF_MIC_HOWLINE_PS,
    .cmd_deal = howling_pitch_shift_parm_analyze,
};
REGISTER_CMD_TARGET(noisegate) = {
    .cmd = EFF_MIC_NOISEGATE,
    .cmd_deal = noise_gate_parm_analyze,
};
REGISTER_CMD_TARGET(pw_drc) = {//下行宽频drc
    .cmd = EFF_PHONE_WIDEBAND_DRC,
    .cmd_deal = phone_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(ph_drc) = {//下行窄频drc
    .cmd = EFF_PHONE_NARROWBAND_DRC,
    .cmd_deal = phone_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(aw_drc) = {//上行宽频drc
    .cmd = EFF_AEC_WIDEBAND_DRC,
    .cmd_deal = phone_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(an_drc) = {//上行窄频drc
    .cmd = EFF_AEC_NARROWBAND_DRC,
    .cmd_deal = phone_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(ml_drc) = {//music_drc
    .cmd = EFF_MUSIC_LOW_DRC,
    .cmd_deal = music_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(mm_drc) = {//music_drc
    .cmd = EFF_MUSIC_MID_DRC,
    .cmd_deal = music_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(mh_drc) = {//music_drc
    .cmd = EFF_MUSIC_HIGH_DRC,
    .cmd_deal = music_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(m_whole_drc) = {//music_drc
    .cmd = EFF_MUSIC_WHOLE_DRC,
    .cmd_deal = music_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(m_cross) = {//music_crossover
    .cmd = EFF_MUSIC_CROSSOVER,
    .cmd_deal = music_wdrc_parm_analyze,
};

REGISTER_CMD_TARGET(rl_drc_p) = {//music_rl_drc
    .cmd = EFF_MUSIC_RL_LOW_DRC,
    .cmd_deal = music_rl_wdrc_parm_analyze,
};


REGISTER_CMD_TARGET(micDrc0) = {//mic_drc0
    .cmd = EFF_MIC_DRC0,
    .cmd_deal = mic_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(micDrc1) = {//mic_drc1
    .cmd = EFF_MIC_DRC1,
    .cmd_deal = mic_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(micDrc2) = {//mic_drc2
    .cmd = EFF_MIC_DRC2,
    .cmd_deal = mic_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(micDrc3) = {//mic_drc3
    .cmd = EFF_MIC_DRC3,
    .cmd_deal = mic_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(micDrc4) = {//mic_drc4
    .cmd = EFF_MIC_DRC4,
    .cmd_deal = mic_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(music_eq) = {//music eq
    .cmd = EFF_MUSIC_EQ2,
    .cmd_deal = music_eq_parm_analyze,
};
REGISTER_CMD_TARGET(music_eq2) = {//music eq2
    .cmd = EFF_MUSIC_EQ,
    .cmd_deal = music_eq_parm_analyze,
};
REGISTER_CMD_TARGET(music_hbass_eq) = {//high bass eq
    .cmd = EFF_MUSIC_HIGH_BASS_EQ,
    .cmd_deal = music_eq_parm_analyze,
};

REGISTER_CMD_TARGET(micEq0) = {//mic eq0
    .cmd = EFF_MIC_EQ0,
    .cmd_deal = mic_eq_parm_analyze,
};
REGISTER_CMD_TARGET(micEq1) = {//mic eq1
    .cmd = EFF_MIC_EQ1,
    .cmd_deal = mic_eq_parm_analyze,
};
REGISTER_CMD_TARGET(micEq2) = {//mic eq2
    .cmd = EFF_MIC_EQ2,
    .cmd_deal = mic_eq_parm_analyze,
};
REGISTER_CMD_TARGET(micEq3) = {//mic eq3
    .cmd = EFF_MIC_EQ3,
    .cmd_deal = mic_eq_parm_analyze,
};
REGISTER_CMD_TARGET(micEq4) = {//mic eq4
    .cmd = EFF_MIC_EQ4,
    .cmd_deal = mic_eq_parm_analyze,
};

REGISTER_CMD_TARGET(ph_Eq) = {//通话下行宽频(16k sr)
    .cmd = EFF_PHONE_WIDEBAND_EQ,
    .cmd_deal = phone_eq_parm_analyze,
};
REGISTER_CMD_TARGET(pn_Eq) = {//通话下行窄频(8k sr)
    .cmd = EFF_PHONE_NARROWBAND_EQ,
    .cmd_deal = phone_eq_parm_analyze,
};
REGISTER_CMD_TARGET(aw_Eq) = {//通话上行宽频(16k sr)
    .cmd = EFF_AEC_WIDEBAND_EQ,
    .cmd_deal = phone_eq_parm_analyze,
};
REGISTER_CMD_TARGET(an_Eq) = {//通话上行窄频(8k sr)
    .cmd = EFF_AEC_NARROWBAND_EQ,
    .cmd_deal = phone_eq_parm_analyze,
};
REGISTER_CMD_TARGET(lowpass_p) = {//2.1/2.2声道低通滤波器
    .cmd = EFF_MUSIC_RL_RR_LOW_PASS,
    .cmd_deal = low_pass_parm_analyze,
};
REGISTER_CMD_TARGET(linein_eq) = {//linein eq
    .cmd = EFF_LINEIN_EQ,
    .cmd_deal = linein_eq_parm_analyze,
};
REGISTER_CMD_TARGET(linein_drc) = {//linein drc
    .cmd = EFF_LINEIN_DRC,
    .cmd_deal = linein_wdrc_parm_analyze,
};
REGISTER_CMD_TARGET(linein_g) = {
    .cmd = EFF_LINEIN_GAIN,
    .cmd_deal = linein_gain_process_parm_analyze,
};
REGISTER_CMD_TARGET(mic_voice_changer) = {
    .cmd = EFF_MIC_VOICE_CHANGER,
    .cmd_deal = mic_voice_changer_parm_ananlyze,
};
REGISTER_CMD_TARGET(vbass_h) = {
    .cmd = EFF_MUSIC_VBASS,
    .cmd_deal = music_vbass_parm_ananlyze,
};

REGISTER_CMD_TARGET(vbass_prev_g) = {
    .cmd = EFF_MUSIC_VBASS_PREV_GAIN,
    .cmd_deal = vbass_prev_gain_process_parm_analyze,
};


/**
 *注册个别查询命令解析函数,返回值1成功，返回小于0失败
 * */
REGISTER_CMD_TARGET(version) = {
    .cmd = EFF_CMD_GETVER,
    .cmd_deal = eff_tool_get_version,
};
REGISTER_CMD_TARGET(file_s) = {
    .cmd = EFF_CMD_FILE_SIZE,
    .cmd_deal = eff_tool_get_cfg_file_size,
};
REGISTER_CMD_TARGET(file_p) = {
    .cmd = EFF_CMD_FILE,
    .cmd_deal = eff_tool_get_cfg_file_data,
};
REGISTER_CMD_TARGET(change_mode) = {
    .cmd = EFF_CMD_CHANGE_MODE,
    .cmd_deal = eff_tool_set_channge_mode,
};
REGISTER_CMD_TARGET(inquire) = {
    .cmd = EFF_CMD_INQUIRE,
    .cmd_deal = eff_tool_set_inquire,
};
REGISTER_CMD_TARGET(resync_begin) = {
    .cmd = EFF_CMD_RESYNC_PARM_BEGIN,
    .cmd_deal = eff_tool_resync_parm_begin,
};

REGISTER_CMD_TARGET(resync_end) = {
    .cmd = EFF_CMD_RESYNC_PARM_END,
    .cmd_deal = eff_tool_resync_parm_end,
};



int eff_init(void)
{
    int i = 0;
    phone_eff_default_parm();
    music_eff_default_parm();
#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
    linein_eff_default_parm();
#endif
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    for (i = 0 ; i < EFFECT_REVERB_PARM_MAX; i++) {
        mic_eff_default_parm(i);
    }
#endif

    phone_eff_analyze_data();
    music_eff_analyze_data();
#if (TCFG_EQ_ENABLE != 0)
    cp_eq_file_seg_to_custom_tab();
#endif
#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
    linein_eff_analyze_data();
#endif

#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE
    for (i = 0 ; i < EFFECT_REVERB_PARM_MAX; i++) {
        mic_eff_analyze_data(i);
    }
#endif
    return 0;
}
__initcall(eff_init);
