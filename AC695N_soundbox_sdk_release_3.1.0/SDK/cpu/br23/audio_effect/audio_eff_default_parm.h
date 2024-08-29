#ifndef _AUD_EFF_DEFAULT_PARM__H
#define _AUD_EFF_DEFAULT_PARM__H

#include "app_config.h"
#include "media/effects_adj.h"

#if(TCFG_MIC_EFFECT_SEL != MIC_EFFECT_MEGAPHONE)
enum {
    EFFECT_REVERB_PARM_KTV = 0,
    EFFECT_REVERB_PARM_POP,
    EFFECT_REVERB_PARM_qingrou,
    EFFECT_REVERB_PARM_SUPER_REVERB,
    EFFECT_REVERB_PARM_SONG,
    EFFECT_REVERB_PARM_MAX,
};
#else
enum {
    EFFECT_REVERB_PARM_KTV = 0,
    EFFECT_REVERB_PARM_MAX,

    EFFECT_REVERB_PARM_POP,
    EFFECT_REVERB_PARM_qingrou,
    EFFECT_REVERB_PARM_SUPER_REVERB,
    EFFECT_REVERB_PARM_SONG,
};

#endif

extern struct phone_parm_tool_set phone_mode[4];//通话上下行模式 0:下行宽 1：下行窄  2：上行宽  3:上行窄
extern struct eff_parm  eff_mode[EFFECT_REVERB_PARM_MAX];//混响的8个模式
extern struct music_parm_tool_set music_mode;//音乐模式
extern struct music_eq2_tool music_eq2_parm;
extern DynamicEQParam_TOOL_SET  dynamic_eq;//动态eq
extern Gain_Process_TOOL_SET gain_parm;//音乐模式尾部的增益调节
extern LowPassParam_TOOL_SET  low_pass_parm;//rl rr通道低通滤波器
extern struct music_eq_tool rl_eq_parm;//rl通道eq
extern struct nband_drc rl_drc_parm;//rl通道drc
extern Gain_Process_TOOL_SET rl_gain_parm;//rl通道gain parm

extern struct music_parm_tool_set linein_mode;//音乐模式
extern Gain_Process_TOOL_SET linein_gain_parm;//音乐模式尾部的增益调节
extern Gain_Process_TOOL_SET vbass_prev_gain_parm;

extern VirtualBass_TOOL_SET vbass_parm;//虚拟低音

extern struct music_eq_tool high_bass_eq_parm;
extern wdrc_struct_TOOL_SET high_bass_drc_parm;

#endif
