#include "audio_dynamic_eq_demo.h"
#include "app_config.h"

#if TCFG_DYNAMIC_EQ_ENABLE
struct dynamic_eq_detection *detect_hdl = NULL;
struct dynamic_eq *dynamic_eq_hdl = NULL;

/* 先打开audio_dynamic_eq_detection_open_demo */
/* 再打开audio_dynamic_eq_open_demo */
struct dynamic_eq *audio_dynamic_eq_open_demo(u32 sample_rate, u8 channel)
{
    DynamicEQEffectParam effectParam[2];
    DynamicEQParam param = {0};

    param.detect_mode = dynamic_eq.detect_mode;//TWOPOINT;
    param.channel = channel;
    param.nSection = dynamic_eq.nSection;//EQ_NSECTION;
    param.SampleRate = sample_rate;
    param.DetectdataInc = (channel == 1) ? 1 : 2;
    param.IndataInc = (channel == 1) ? 1 : 2;
    param.OutdataInc = (channel == 1) ? 1 : 2;
    memcpy(effectParam, &dynamic_eq.effect_param, sizeof(DynamicEQEffectParam)*param.nSection);
    struct dynamic_eq *hdl = dynamic_eq_open((DynamicEQEffectParam *)effectParam, (DynamicEQParam *)&param);
    return hdl;
}

void audio_dynamic_eq_close_demo(struct dynamic_eq *hdl)
{
    dynamic_eq_close(hdl);
    dynamic_eq_hdl = NULL;
}

void audio_dynamic_eq_update_parm(void *parm, void *parm2, int bypass)
{
    if (dynamic_eq_hdl) {
        dynamic_eq_update(dynamic_eq_hdl, parm, parm2);

        dynamic_eq_bypass(dynamic_eq_hdl, bypass);
    }
}


struct dynamic_eq_detection *audio_dynamic_eq_detection_open_demo(u32 sample_rate, u8 channel)
{
    DynamicEQDetectionParam detectParm[2];
    detectParm[0].fc = dynamic_eq.effect_param[0].fc;//CENTER_FREQ_0;
    detectParm[1].fc = dynamic_eq.effect_param[1].fc;//CENTER_FREQ_1;

    struct dynamic_eq_detection *hdl = dynamic_eq_detection_open((DynamicEQDetectionParam *)&detectParm, dynamic_eq.nSection, channel, sample_rate);
    detect_hdl = hdl;
    return hdl;
}


void audio_dynamic_eq_detection_close_demo(struct dynamic_eq_detection *hdl)
{
    dynamic_eq_detection_close(hdl);
}


void audio_dynamic_eq_detection_update_parm(void *parm, int bypass)
{
    if (detect_hdl) {
        dynamic_eq_detection_update(detect_hdl, parm);

        dynamic_eq_detection_bypass(detect_hdl, bypass);
    }
}


int dy_eq_prob_handler(struct audio_stream_entry *entry,  struct audio_data_frame *in)	// 预处理
{
    if (in->data_len - in->offset > 0) {
        if (detect_hdl) {
            detect_hdl->in_32bit = 1;
            dynamic_eq_detection_run(detect_hdl, (short *)((int)in->data + in->offset), in->data_len - in->offset);
        }
    }
    return 0;
}


struct dynamic_eq_hdl *audio_dynamic_eq_ctrl_open(u32 sample_rate, u8 channel)
{
    struct dynamic_eq_hdl *hdl = zalloc(sizeof(struct dynamic_eq_hdl));
    if (!hdl) {
        return NULL;
    }
    struct dynamic_eq_detection *dy_eq_det = audio_dynamic_eq_detection_open_demo(sample_rate, channel);
    if (!dy_eq_det) {
        return NULL;
    }
    struct dynamic_eq *dy_eq = audio_dynamic_eq_open_demo(sample_rate, channel);
    if (!dy_eq) {
        log_e("dy_eq NULL\n");
        return NULL;
    }
    if (dy_eq_det) {
        dynamic_eq_set_detection_callback(dy_eq, dy_eq_det, get_dynamic_eq_detection_parm);
    }
    hdl->dy_eq_det = dy_eq_det;
    hdl->dy_eq = dy_eq;
    hdl->dy_eq->entry.prob_handler = dy_eq_prob_handler;

    detect_hdl = dy_eq_det;
    dynamic_eq_hdl = dy_eq;
    return hdl;
}

void audio_dynamic_eq_ctrl_close(struct dynamic_eq_hdl *hdl)
{
    if (!hdl) {
        return;
    }

    if (hdl->dy_eq) {
        audio_dynamic_eq_close_demo(hdl->dy_eq);
        hdl->dy_eq = NULL;
        dynamic_eq_hdl = NULL;
    }
    if (hdl->dy_eq_det) {
        audio_dynamic_eq_detection_close_demo(hdl->dy_eq_det);
        hdl->dy_eq_det = NULL;
        detect_hdl = NULL;
    }
    if (hdl) {
        free(hdl);
        hdl = NULL;
    }
}
#endif
