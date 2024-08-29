#ifndef _AUDIO_NOISEGATE_API_H
#define _AUDIO_NOISEGATE_API_H

#include "media/audio_stream.h"
#include "media/noisegate_api.h"

typedef struct _NOISEGATE_API_STRUCT_ {
    void				*workbuf;    //运算buf指针
    NoiseGateParam 	parm;  //参数

    struct audio_stream_entry entry;	// 音频流入口
    u8 status;
    u8 update;
} NOISEGATE_API_STRUCT;


NOISEGATE_API_STRUCT *open_noisegate(NoiseGateParam *noisegate_parm);
int run_noisegate(NOISEGATE_API_STRUCT *noisegate_hdl, short *in, short *out, int len);
void close_noisegate(NOISEGATE_API_STRUCT *noisegate_hdl);
// void pause_noisegate(NOISEGATE_API_STRUCT *noisegate_hdl, u8 run_mark);
void update_noisegate(NOISEGATE_API_STRUCT *noisegate_hdl, NoiseGateParam *parm);
void noisegate_bypass(NOISEGATE_API_STRUCT *hdl, u8 bypass);
#ifndef RUN_NORMAL
#define RUN_NORMAL  0
#endif

#ifndef RUN_BYPASS
#define RUN_BYPASS  1
#endif
#endif

