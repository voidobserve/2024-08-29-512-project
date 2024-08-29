#include "audio_vbass_demo.h"
#include "app_config.h"

#if AUDIO_VBASS_CONFIG
VirtualBass_TOOL_SET vbass_parm;
vbass_hdl *audio_vbass_open_demo(u32 vbass_name, u32 sample_rate, u8 ch_num)
{
    u8 bypass = vbass_parm.is_bypass;
    VirtualBassParam parm = {0};
    parm.ratio = vbass_parm.parm.ratio;
    parm.boost = vbass_parm.parm.boost;
    parm.fc    = vbass_parm.parm.fc;
    parm.channel = ch_num;
    parm.SampleRate = sample_rate;
    //printf("vbass ratio %d, boost %d, fc %d, channel %d, SampleRate %d\n", parm.ratio, parm.boost, parm.fc,parm.channel, parm.SampleRate);
    vbass_hdl *vbass = audio_vbass_open(vbass_name, &parm);
    audio_vbass_bypass(vbass_name, bypass);
    clock_add(DEC_VBASS_CLK);
    return vbass;
}


void audio_vbass_close_demo(vbass_hdl *vbass)
{
    if (vbass) {
        audio_vbass_close(vbass);
        vbass = NULL;
    }
    clock_remove(DEC_VBASS_CLK);
}



void audio_vbass_update_demo(u32 vbass_name, VirtualBassUdateParam *parm, u32 bypass)
{
    audio_vbass_parm_update(vbass_name, parm);
    audio_vbass_bypass(vbass_name, bypass);
}



#endif
