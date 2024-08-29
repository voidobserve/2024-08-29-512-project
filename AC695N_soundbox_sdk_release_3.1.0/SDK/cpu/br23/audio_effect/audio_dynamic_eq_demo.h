#ifndef __AUDIO_DYNAMEIC_EQ_H__
#define __AUDIO_DYNAMEIC_EQ_H__

#include "media/dynamic_eq.h"
#include "media/effects_adj.h"
#include "audio_effect/audio_eff_default_parm.h"


struct dynamic_eq_hdl {
    struct dynamic_eq_detection *dy_eq_det;
    struct dynamic_eq *dy_eq;
};

/* 先打开audio_dynamic_eq_detection_open_demo */
/* 再打开audio_dynamic_eq_open_demo */
struct dynamic_eq *audio_dynamic_eq_open_demo(u32 sample_rate, u8 channel);
void audio_dynamic_eq_close_demo(struct dynamic_eq *hdl);

struct dynamic_eq_detection *audio_dynamic_eq_detection_open_demo(u32 sample_rate, u8 channel);
void audio_dynamic_eq_detection_close_demo(struct dynamic_eq_detection *hdl);

struct dynamic_eq_hdl *audio_dynamic_eq_ctrl_open(u32 sample_rate, u8 channel);
void audio_dynamic_eq_ctrl_close(struct dynamic_eq_hdl *hdl);

#endif/*__AUDIO_DYNAMEIC_EQ_H__*/
