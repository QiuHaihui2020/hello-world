#ifndef _AUDIO_DIGITAL_VOL_H_
#define _AUDIO_DIGITAL_VOL_H_

#include "generic/typedef.h"
#include "os/os_type.h"
#include "os/os_api.h"


void audio_digital_vol_open(u8 vol, u8 vol_max, u16 fade_step);
void audio_digital_vol_close(void);
void audio_digital_vol_set(u8 vol);
u8 audio_digital_vol_get(void);
int audio_digital_vol_run(void *data, u32 len);

/*************************自定义支持重入的数字音量调节****************************/
void *user_audio_digital_volume_open(u8 vol, u8 vol_max, u16 fade_step);
int user_audio_digital_volume_close(void *_d_volume);
u8 user_audio_digital_volume_get(void *_d_volume);
int user_audio_digital_volume_set(void *_d_volume, u8 vol);
int user_audio_digital_volume_reset_fade(void *_d_volume);
int user_audio_digital_volume_run(void *_d_volume, void *data, u32 len, u8 ch_num);
void user_audio_digital_handler_run(void *_d_volume, void *data, u32 len);
void user_audio_digital_set_volume_tab(void *_d_volume, u16 *user_vol_tab, u8 user_vol_max);

void *user_audio_process_open(void *parm, void *priv, void (*handler)(void *priv, void *data, int len, u8 ch_num));
int user_audio_process_close(void *_uparm_hdl);
void user_audio_process_handler_run(void *_uparm_hdl, void *data, u32 len, u8 ch_num);

struct user_audio_digital_parm {
    u8 en;
    u8 vol;
    u8 vol_max;
    u16 fade_step;
};

struct digital_volume {
    u8 toggle;					/*数字音量开关*/
    u8 fade;					/*淡入淡出标志*/
    u8 vol;						/*淡入淡出当前音量*/
    u8 vol_max;					/*淡入淡出最大音量*/
    s16 vol_fade;				/*淡入淡出对应的起始音量*/
    volatile s16 vol_target;	/*淡入淡出对应的目标音量*/
    volatile u16 fade_step;				/*淡入淡出的步进*/

    OS_MUTEX mutex;
    u8 ch_num;
    void *priv;
    u8 user_vol_max;                                 /*自定义音量表级数*/
    volatile s16 *user_vol_tab;	                     /*自定义音量表*/

};

struct user_audio_parm {
    void *priv;
    void (*handler)(void *priv, void *data, int len, u8 ch_num);/*用户自定义回调处理*/
    struct digital_volume *dvol_hdl;
};

#endif
