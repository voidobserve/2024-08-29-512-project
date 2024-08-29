#include "audio_gain_process_demo.h"
#include "app_config.h"

#if GAIN_PROCESS_EN
Gain_Process_TOOL_SET rl_gain_parm;//rl通道gain parm
Gain_Process_TOOL_SET gain_parm;//音乐模式尾部的增益调节
Gain_Process_TOOL_SET linein_gain_parm;//音乐模式尾部的增益调节
#endif
Gain_Process_TOOL_SET vbass_prev_gain_parm;
struct aud_gain_process *audio_gain_open_demo(u16 gain_name, u8 channel)
{
    if (gain_name != AEID_MUSIC_VBASS_PREV_GAIN) {
        return NULL;
    }
    struct aud_gain_parm parm = {0};

    float gain = 0;
    u8 bypass = 0;
#if GAIN_PROCESS_EN
    if (gain_name == AEID_MUSIC_GAIN) {
        gain = gain_parm.parm.gain;
        bypass = gain_parm.is_bypass;
    } else if (gain_name == AEID_MUSIC_RL_GAIN) {
        gain = rl_gain_parm.parm.gain;
        bypass = rl_gain_parm.is_bypass;
    } else if (gain_name == AEID_LINEIN_GAIN) {
#if defined(LINEIN_MODE_SOLE_EQ_EN) && LINEIN_MODE_SOLE_EQ_EN
        gain = linein_gain_parm.parm.gain;
        bypass = linein_gain_parm.is_bypass;
#endif
    } else if (gain_name == AEID_MIC_GAIN) {
#if defined(TCFG_MIC_EFFECT_ENABLE) && TCFG_MIC_EFFECT_ENABLE

        u8 mode = get_mic_eff_mode();
        gain = eff_mode[mode].gain_parm.parm.gain;
        bypass = eff_mode[mode].gain_parm.is_bypass;
#endif
    } else
#endif/*GAIN_PROCESS_EN*/
        if (gain_name == AEID_MUSIC_VBASS_PREV_GAIN) {
            gain = vbass_prev_gain_parm.parm.gain;
            bypass = vbass_prev_gain_parm.is_bypass;
        }

    parm.gain = gain;
    parm.channel = channel;
    parm.indata_inc = (channel == 1) ? 1 : 2;
    parm.outdata_inc = (channel == 1) ? 1 : 2;
    parm.bit_wide = 0; //16bit_wide
    parm.gain_name = gain_name;
    struct aud_gain_process *hdl = audio_gain_process_open(&parm);
    audio_gain_process_bypass(parm.gain_name, bypass);
    return hdl;
}


void audio_gain_close_demo(struct aud_gain_process *hdl)
{
    if (!hdl) {
        return;
    }
    audio_gain_process_close(hdl);
    hdl = NULL;
}

void audio_gain_update_parm(u16 gain_name, void *parm, int bypass)
{
    audio_gain_process_update(gain_name, parm);
    audio_gain_process_bypass(gain_name, bypass);
}



