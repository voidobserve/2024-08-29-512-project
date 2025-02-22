#ifndef __EFFECTS_ADJ__H
#define __EFFECTS_ADJ__H


#include "system/includes.h"
#include "config/config_interface.h"
#include "asm/crc16.h"
#include "generic/log.h"
#include "media/audio_eq.h"
#include "media/audio_drc.h"
#include "media/howling_api.h"
#include "media/sw_drc.h"
#include "media/DynamicEQ_api.h"
#include "media/DynamicEQ_Detection_api.h"
#include "media/reverb_api.h"
#include "media/noisegate_api.h"
#include "media/audio_gain_process.h"
#include "media/voiceChanger_api.h"
#include "media/audio_vbass.h"

#define GAIN_PROCESS_EN  0

struct effect_adj {
    u8 eq_type;
    uint8_t password_ok;
    struct __effect_mode_cfg 	*parm;
    struct __tool_callback      *cb;
};

typedef struct {
    int cmd;     			///<EQ_ONLINE_CMD
    int data[64];       	///<data
} EFF_ONLINE_PACKET;


enum {
    EFF_CMD_INQUIRE = 0x4,
    EFF_CMD_GETVER = 0x5,
    EFF_CMD_FILE_SIZE = 0xB,
    EFF_CMD_FILE = 0xC,
    EFF_CMD_CHANGE_MODE = 0xE,//切模式
    EFF_CMD_RESYNC_PARM_END = 0x28,//参数重复结束
    EFF_CMD_RESYNC_PARM_BEGIN = 0x30,//参数重复开始

    EFF_MIC_EQ0 = 0x1001,
    EFF_MIC_EQ1 = 0x1002,
    EFF_MIC_EQ2 = 0x1003,
    EFF_MIC_EQ3 = 0x1004,
    EFF_MIC_EQ4 = 0x1005,
    EFF_MIC_DRC0 = 0x1006,
    EFF_MIC_DRC1 = 0x1007,
    EFF_MIC_DRC2 = 0x1008,
    EFF_MIC_DRC3 = 0x1009,
    EFF_MIC_DRC4 = 0x100a,

    EFF_MIC_PLATE_REVERB  = 0x100c,
    EFF_MIC_ECHO          = 0x100e,
    EFF_MIC_NOISEGATE     = 0x1014,
    EFF_MIC_HOWLINE_PS    = 0x1015,
    EFF_MIC_NOTCH_HOWLING = 0x1016,
    EFF_MIC_VOICE_CHANGER = 0x1017,


    EFF_PHONE_WIDEBAND_EQ   = 0x1101,
    EFF_PHONE_NARROWBAND_EQ = 0x1102,
    EFF_AEC_WIDEBAND_EQ     = 0x1103,
    EFF_AEC_NARROWBAND_EQ   = 0x1104,
    EFF_PHONE_WIDEBAND_DRC  = 0x1105,
    EFF_PHONE_NARROWBAND_DRC = 0x1106,
    EFF_AEC_WIDEBAND_DRC    = 0x1107,
    EFF_AEC_NARROWBAND_DRC  = 0x1108,


    EFF_MUSIC_EQ          = 0x2001,
    EFF_MUSIC_LOW_DRC     = 0x2002,
    EFF_MUSIC_MID_DRC     = 0x2011,
    EFF_MUSIC_HIGH_DRC    = 0x2012,
    EFF_MUSIC_WHOLE_DRC   = 0x2013,
    EFF_MUSIC_CROSSOVER   = 0x2014,
    EFF_MUSIC_DYNAMIC_EQ  = 0x2015,
    EFF_MUSIC_EQ2         = 0x2016,
    EFF_MUSIC_GAIN        = 0x2017,

    EFF_MIC_GAIN          = 0x2018,//混响输出位置的gain
    EFF_MUSIC_RL_GAIN     = 0x2019,//rl_rr通道gain


    EFF_MUSIC_RL_RR_LOW_PASS = 0x2020,//2.1/2.2声道低通滤波器
    EFF_MUSIC_RL_EQ          = 0x2005,
    EFF_MUSIC_RL_LOW_DRC     = 0x2006,
    EFF_MUSIC_RL_MID_DRC     = 0x2027,
    EFF_MUSIC_RL_HIGH_DRC    = 0x2028,
    EFF_MUSIC_RL_WHOLE_DRC   = 0x2029,

    EFF_LINEIN_EQ         = 0x2030,//linein 需要独立的音效时，使用的效果id
    EFF_LINEIN_DRC        = 0x2031,
    EFF_LINEIN_GAIN       = 0x2032,

    EFF_MUSIC_VOICE_CHANGER = 0x2033,
    EFF_MUSIC_VBASS         = 0x2034,//音乐虚拟低音
    EFF_MUSIC_VBASS_PREV_GAIN = 0x2035,

    EFF_MUSIC_LPF_EQ         = 0x2036,
    EFF_MUSIC_HIGH_BASS_EQ       = 0x2037,
    EFF_MUSIC_HIGH_BASS_DRC       = 0x2038,
    EFF_CMD_MAX,//最后一个

};




typedef enum {
//模式id,效果文件解析时会用于定位相应模式下效果参数
    phone_mode_seq = 1,
    aec_mode_seq = 2,

    music_mode_seq0 = 0x4,

    mic_mode_seq0 = 0x5,
    mic_mode_seq1 = 0x6,
    mic_mode_seq2 = 0x7,
    mic_mode_seq3 = 0x8,
    mic_mode_seq4 = 0x9,
    mic_mode_seq5 = 0xa,
    mic_mode_seq6 = 0xb,
    mic_mode_seq7 = 0xc,

    linein_mode_seq = 0xe,

//add xx

    max_seq,
} MODE_NUM;

//AudioEffects ID(AEID) List: EQ/DRC等模块ID，识别不同模式下EQ\DRC效果用
typedef enum {
//通话下行音效处理
    AEID_ESCO_DL_EQ = 1,
    AEID_ESCO_DL_DRC,

//通话上行音效处理
    AEID_ESCO_UL_EQ,
    AEID_ESCO_UL_DRC,

//音乐播放音效处理
    AEID_MUSIC_EQ,
    AEID_MUSIC_DRC,
    AEID_MUSIC_FR_EQ,
    AEID_MUSIC_FR_DRC,
    AEID_MUSIC_RL_EQ,
    AEID_MUSIC_RL_DRC,
    AEID_MUSIC_RR_EQ,
    AEID_MUSIC_RR_DRC,
    AEID_MUSIC_EQ2,

//混响音效处理
    AEID_MIC_EQ0,
    AEID_MIC_DRC0,
    AEID_MIC_EQ1,
    AEID_MIC_DRC1,
    AEID_MIC_EQ2,
    AEID_MIC_DRC2,
    AEID_MIC_EQ3,
    AEID_MIC_DRC3,
    AEID_MIC_EQ4,
    AEID_MIC_DRC4,

//增益计算
    AEID_MIC_GAIN,
    AEID_MUSIC_GAIN,
    AEID_MUSIC_RL_GAIN,//rl通道
//linein
    AEID_LINEIN_EQ,
    AEID_LINEIN_DRC,
    AEID_LINEIN_GAIN,
//高低音eq
    AEID_HIGH_BASS_EQ,
    AEID_HIGH_BASS_DRC,
//变声

    AEID_MIC_VOICE_CHANGER,
    AEID_MUSIC_VOICE_CHANGER,
//虚拟低音
    AEID_MUSIC_VBASS,
    AEID_MUSIC_VBASS_PREV_GAIN,
//环绕音效
    AEID_MUSIC_SURROUND,

    AEID_MUSIC_LPF_EQ,

} AudioEffectsID; //模块id

struct mode_list {
    u16 module_name;
    u8 nsection;
    u8 group_num;
    u16 group_id[5];
};

//reverb 模块：
typedef struct REVERBN_PARM_TOOL_SET_ {
    int is_bypass;          // 1-> byass 0 -> no bypass
    REVERBN_PARM_SET parm;
} REVERBN_PARM_TOOL_SET;

//reverb_palte 模块：
typedef  struct  _Plate_reverb_TOOL_SET_ {
    int is_bypass;          // 1-> byass 0 -> no bypass
    Plate_reverb_parm parm;
} Plate_reverb_TOOL_SET;

//reverb_filter模块：
typedef struct _EF_REVERB0__TOOL_SET_ {
    int is_bypass;          // 1-> byass 0 -> no bypass
    EF_REVERB0_PARM parm;
} EF_REVERB0_TOOL_SET;

//echo模块:
typedef struct _EF_ECHO_TOOL_SET_ {
    int is_bypass;          // 1-> byass 0 -> no bypass
    ECHO_PARM_SET parm;
} EF_ECHO_TOOL_SET;

//移频模块 HowlingPitchShift:
typedef struct _HowlingPs_PARM_TOOL_SET_ {
    int is_bypass;          // 1-> byass 0 -> no bypass
    HOWLING_PITCHSHIFT_PARM parm;
} HowlingPs_PARM_TOOL_SET;

typedef struct _NotchHowlingParam_TOOL_SET {
    int is_bypass;          // 1-> byass 0 -> no bypass
    NotchHowlingParam parm;
} NotchHowlingParam_TOOL_SET;

typedef struct _VoiceChangerParam_TOOL_SET {
    int is_bypass;          // 1-> byass 0 -> no bypass
    VOICECHANGER_PARM parm;
} VoiceChangerParam_TOOL_SET;


typedef struct _NoiseGateParam_TOOL_SET {
    int is_bypass;          // 1-> byass 0 -> no bypass
    noisegate_update_param parm;
} NoiseGateParam_TOOL_SET;

typedef struct _gain_process_TOOL_SET {
    int is_bypass;          // 1-> byass 0 -> no bypass
    struct aud_gain_parm_update parm;
} Gain_Process_TOOL_SET;


struct advance_iir {
    int fc;
    int order;
    int type;
};
// #低通 LowPass:
typedef struct _LowPassParam_TOOL_SET {
    int is_bypass; // 1-> byass 0 -> no bypass
    struct advance_iir low_pass;
} LowPassParam_TOOL_SET;

// #高通 HighPass:
typedef struct _HighPassParam_TOOL_SET {
    int is_bypass; // 1-> byass 0 -> no bypass
    struct advance_iir high_pass;
} HighPassParam_TOOL_SET;

//虚拟低音
typedef struct _VirtualBass_TOOL_SET {
    int is_bypass; // 1-> byass 0 -> no bypass
    VirtualBassUdateParam parm;
} VirtualBass_TOOL_SET;

//动态EQ DynamicEQ:
typedef struct _DynamicEQParam_TOOL_SET {
    int is_bypass;          // 1-> byass 0 -> no bypass
    DynamicEQEffectParam  effect_param[4];
    int nSection;					//段数
    int detect_mode;			//检测模式
} DynamicEQParam_TOOL_SET; //实际发送这个结构体

#define OTHER_SECTION_MAX (5)
struct eq_tool {
    float global_gain;
    int seg_num;          //eq效果文件存储的段数
    int enable_section;   //
    struct eq_seg_info seg[OTHER_SECTION_MAX];   //eq系数存储地址
};

#define mSECTION_MAX (20)
struct music_eq_tool {
    float global_gain;
    int seg_num;          //eq效果文件存储的段数
    int enable_section;   //
    struct eq_seg_info seg[mSECTION_MAX];   //eq系数存储地址
};

#define mSECTION_MAX2 (10)
struct music_eq2_tool {
    float global_gain;
    int seg_num;          //eq效果文件存储的段数
    int enable_section;   //
    struct eq_seg_info seg[mSECTION_MAX2];   //eq系数存储地址
};


struct phone_parm_tool_set {
    struct eq_tool eq_parm;
    wdrc_struct_TOOL_SET drc_parm;
};

struct nband_drc {
    CrossOverParam_TOOL_SET crossover;
    wdrc_struct_TOOL_SET wdrc_parm[4];//[0]low  [1]mid [2]high [3]多带之后附加的全带
};


struct music_parm_tool_set {
    struct music_eq_tool eq_parm;
    struct nband_drc drc_parm;
};

struct eff_parm {
    NoiseGateParam_TOOL_SET noise_gate_parm;
    HowlingPs_PARM_TOOL_SET howlingps_parm;
    NotchHowlingParam_TOOL_SET notchhowling_parm;
    VoiceChangerParam_TOOL_SET voicechanger_parm;
    Plate_reverb_TOOL_SET plate_reverb_parm;
    EF_ECHO_TOOL_SET echo_parm;
    struct eq_tool eq_parm[5];
    wdrc_struct_TOOL_SET drc_parm[5];
#if GAIN_PROCESS_EN
    Gain_Process_TOOL_SET gain_parm;//增益
#endif
};


void get_eff_mode(u16 mode_id, u16 group_id, u8 *mode_index, u8 *drc_index);//获取混响模式的index
int get_phone_mode(u16 group_id);
int get_group_id(u16 mode_name, u8 tar);
int get_eq_nsection(u16 module_name);

void noisegate_update_parm(void *parm, int bypass);
void plate_reverb_update_parm(void *parm, int bypass);
void echo_updata_parm(void *parm, int bypass);
void howling_pitch_shift_update_parm(void *parm, int bypass);
void notchhowline_update_parm(void *parm, int bypass);
void audio_dynamic_eq_update_parm(void *parm, void *parm2, int bypass);
void audio_dynamic_eq_detection_update_parm(void *parm, int bypas);
void mic_gain_update_parm(u16 gain_name, void *parm, int bypass);
void set_mic_reverb_mode_by_id(u8 mode);

void phone_eff_default_parm(void);
void music_eff_default_parm(void);
void mic_eff_default_parm(u8 index);
void linein_eff_default_parm(void);
void cp_eq_file_seg_to_custom_tab();


struct cmd_interface {
    u32 cmd;
    int (*cmd_deal)(EFF_ONLINE_PACKET *packet, u8 id, u8 sq);
};

#define REGISTER_CMD_TARGET(cmd_eff) \
	const struct cmd_interface cmd_eff sec(.eff_cmd)

extern const struct cmd_interface cmd_interface_begin[];
extern const struct cmd_interface cmd_interface_end[];

#define list_for_each_cmd_interface(p) \
	    for (p = (struct cmd_interface *)cmd_interface_begin; p < (struct cmd_interface *)cmd_interface_end; p++)

#define phone_eq_nsection  OTHER_SECTION_MAX
#define music_eq_nsection  10
#define music_eq2_nsection  10
#define mic_eq_nsection   OTHER_SECTION_MAX



#endif/*__EFFECTS_ADJ__H*/
