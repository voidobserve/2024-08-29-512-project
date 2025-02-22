#include "app_config.h"

#if TCFG_USER_TWS_ENABLE
tws_local_media_sync
tws_afh_sync
tws_ci_data
tws_acl_data_sync
tws_event_sync
tws_conn_sync
tws_lmp_slot_sync
tws_low_latency
tws_media_sync
tws_power_balance
tws_sync_call
tws_tx_sync
tws_link_sync
#endif


#if CONFIG_FATFS_ENABLE
fat_vfs_ops
#endif

#if VFS_ENABLE
sdfile_vfs_ops
#endif

#if TCFG_NORFLASH_DEV_ENABLE && VFS_ENABLE
nor_fs_vfs_ops
nor_sdfile_vfs_ops
nor_rec_fs_vfs_ops
fat_sdfile_fat_ops
#endif

#if FLASH_INSIDE_REC_ENABLE
inside_nor_fs_vfs_ops
#endif

#if TCFG_DEC_SBC_ENABLE
sbc_decoder
msbc_decoder
sbc_hwaccel
#endif

#if TCFG_DEC_CVSD_ENABLE
cvsd_decoder
#endif

#if TCFG_DEC_PCM_ENABLE
pcm_decoder
#endif

#if TCFG_DEC_MTY_ENABLE
mty_decoder
#endif

#if TCFG_DEC_MP3_ENABLE
mp3_decoder
#if TCFG_DEC2TWS_ENABLE
mp3pick_decoder
#endif
#endif

#if TCFG_DEC_WMA_ENABLE
wma_decoder
#if TCFG_DEC2TWS_ENABLE
wmapick_decoder
#endif
#endif

#if TCFG_DEC_FLAC_ENABLE
flac_decoder
#endif

#if TCFG_DEC_APE_ENABLE
ape_decoder
#endif

#if TCFG_DEC_M4A_ENABLE
m4a_decoder
#if TCFG_DEC2TWS_ENABLE
m4apick_decoder
#endif
#endif

#if TCFG_DEC_ALAC_ENABLE
alac_decoder
#endif

#if TCFG_DEC_AMR_ENABLE
amr_decoder
#endif

#if TCFG_DEC_DTS_ENABLE
dts_decoder
#endif

#if TCFG_DEC_G729_ENABLE
g729_decoder
#endif

#if TCFG_DEC_WTGV2_ENABLE
wtgv2_decoder
#endif

#if (TCFG_DEC_AAC_ENABLE || TCFG_BT_SUPPORT_AAC)
aac_decoder
#endif

#if TCFG_DEC_G726_ENABLE
g726_decoder
#endif

#if TCFG_DEC_MIDI_ENABLE
midi_decoder
#endif

#if TCFG_DEC_WAV_ENABLE
wav_decoder
#endif

#if AUDIO_MIDI_CTRL_CONFIG
midi_ctrl_decoder
#endif



#if TCFG_ENC_CVSD_ENABLE
cvsd_encoder
#endif

#if TCFG_ENC_MSBC_ENABLE
msbc_encoder
#endif

#if TCFG_ENC_MP3_ENABLE
mp3_encoder
#endif

#if TCFG_ENC_G726_ENABLE
g726_encoder
#endif

#if TCFG_ENC_ADPCM_ENABLE
adpcm_encoder
#endif

#if TCFG_ENC_PCM_ENABLE
pcm_encoder
#endif

#if TCFG_ENC_OPUS_ENABLE
opus_encoder
#endif

#if TCFG_ENC_SPEEX_ENABLE
speex_encoder
#endif

#if TCFG_ENC_SBC_ENABLE
sbc_encoder
#endif

eff_adj_target


