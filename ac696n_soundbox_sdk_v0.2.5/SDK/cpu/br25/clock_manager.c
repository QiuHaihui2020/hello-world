#include "system/includes.h"
#include "app_config.h"
#include "clock_cfg.h"
#include "asm/dac.h"
struct clock_type {
    u8 type;
    u32 clock;
    const char *name;
};

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR)
#define  EQ4_CLK  47  //20段eq
#define  MIX4_CLK 17  //256点、4声道、
#else
#define  EQ4_CLK  0
#define  MIX4_CLK 0
#endif
const struct clock_type  clock_enum[] = {

    { BT_IDLE_CLOCK     	, (24), "	BT_IDLE_CLOCK      " },
    { MUSIC_IDLE_CLOCK  	, (24), "	MUSIC_IDLE_CLOCK   " },
    { FM_IDLE_CLOCK     	, (12), "	FM_IDLE_CLOCK      " },
    { LINEIN_IDLE_CLOCK 	, (96), "	LINEIN_IDLE_CLOCK  " },
    { PC_IDLE_CLOCK     	, (120), "	PC_IDLE_CLOCK      " },
    { REC_IDLE_CLOCK    	, (24), "	REC_IDLE_CLOCK     " },
    { RTC_IDLE_CLOCK    	, (12), "	RTC_IDLE_CLOCK     " },
    { SPDIF_IDLE_CLOCK  	, (12), "	SPDIF_IDLE_CLOCK   " },
    { BOX_IDLE_CLOCK  		, (24), "	BOX_IDLE_CLOCK   " },


    { DEC_SBC_CLK	, (32), "DEC_SBC_CLK	 " },
    { DEC_AAC_CLK	, (32), "DEC_AAC_CLK	 " },
    { DEC_MSBC_CLK	, (12), "DEC_MSBC_CLK	 " },
    { DEC_CVSD_CLK	, (8), "DEC_CVSD_CLK	 " },

    { AEC8K_CLK	, (48), "AEC8K_CLK	     " },
    { AEC8K_ADV_CLK	, (60), "AEC8K_ADV_CLK	 " },
    { AEC16K_CLK	, (80), "AEC16K_CLK	 " },
    { AEC16K_ADV_CLK, (120), "AEC16K_ADV_CLK " },

    { DEC_TONE_CLK	, (24 + 32), "DEC_TONE_CLK	 " },
    { DEC_MP3_CLK	, (80), "DEC_MP3_CLK	 " },
    { DEC_WAV_CLK	, (32), "DEC_WAV_CLK	 " },
    { DEC_G729_CLK	, (24), "DEC_G729_CLK	 " },
    { DEC_G726_CLK	, (24), "DEC_G726_CLK	 " },
    { DEC_PCM_CLK	, (24), "DEC_PCM_CLK	 " },
    { DEC_MTY_CLK	, (24), "DEC_MTY_CLK	 " },
    { DEC_WMA_CLK	, (48), "DEC_WMA_CLK	 " },

    { DEC_APE_CLK	, (160), "DEC_APE_CLK	 "},
    { DEC_FLAC_CLK	, (96), "DEC_FLAC_CLK	 "},
    { DEC_AMR_CLK	, (96), "DEC_AMR_CLK	 "},
    { DEC_DTS_CLK	, (120), "DEC_DTS_CLK	 " },

    { DEC_M4A_CLK	, (48), "DEC_M4A_CLK	 "},
    { DEC_FM_CLK	, (12), "DEC_FM_CLK	 " },
    { DEC_LINE_CLK	, (24), "DEC_LINE_CLK	 " },
    { DEC_TWS_SBC_CLK, (32), "DEC_TWS_SBC_CLK" },
    { SPDIF_CLK		, (120), "SPDIF_CLK	     " },

    { ENC_RECODE_CLK, (64), "ENC_RECODE_CLK "  },
    { ENC_SBC_CLK	, (64), "ENC_SBC_CLK	 "  },
    { ENC_WAV_CLK	, (96), "ENC_WAV_CLK	 "  },
    { ENC_G726_CLK, (64), "ENC_G726_CLK   "  },
    { ENC_MP3_CLK	, (120), "ENC_MP3_CLK	 "  },
    { ENC_TWS_SBC_CLK, (48), "ENC_TWS_SBC_CLK" },
    { ENC_MSBC_CLK	, (12), "ENC_MSBC_CLK	 "  },
    { ENC_CVSD_CLK	, (8), "ENC_CVSD_CLK	 "  },

    { SYNC_CLK	, (4), "SYNC_CLK	     "  },
    { AUTOMUTE_CLK	, (16), "AUTOMUTE_CLK" },

    { FINDF_CLK	, (120), "FINDF_CLK	     "  },
    { FM_INSIDE_CLK	, (96), "FM_INSIDE_CLK	 " },
    { BT_CONN_CLK	, (16), "BT_CONN_CLK	 " },


    { EQ_CLK	, (24 + EQ4_CLK), " EQ_CLK	     "   },
    { EQ_DRC_CLK	, (120), " EQ_DRC_CLK	 "   },
    { EQ_ONLINE_CLK, (120), " EQ_ONLINE_CLK "   },
    { REVERB_CLK	, (32), " REVERB_CLK	 "   },
    { REVERB_HOWLING_CLK, (120), " REVERB_HOWLING_CLK	 "   },
    { REVERB_PITCH_CLK	, (24), " REVERB_PITCH_CLK	 "   },

    { DEC_MP3PICK_CLK,	(8), 	"DEC_MP3PICK_CLK"   },
    { DEC_WMAPICK_CLK,	(8),	"DEC_WMAICK_CLK"   },
    { DEC_MIX_CLK, (6 + MIX4_CLK),	"DEC_MIX_CLK"   },

    { DEC_IIS_CLK, (64),	"DEC_IIS_CLK"   },
    { DEC_UI_CLK, (8),	"DEC_UI_CLK"   },


    { DEC_MIDI_CLK, (32),	"DEC_MIDI_CLK"   },
};



const u8 clock_tb[] = {
    24,
    32,
    48,
    60,
    80,
    96,
    120,
    160,
    192,
};

static u8 ext_clk_tb[10];
static u32 idle_type = 0;

static void clock_ext_dump()
{
    u8 i, j;
    for (i = 0; i < ARRAY_SIZE(clock_enum); i++) {
        if (idle_type ==  clock_enum[i].type) {
            y_printf("--- %d  %s \n", clock_enum[i].clock, clock_enum[i].name);
            break;
        }
    }

    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (ext_clk_tb[i]) {
            for (j = 0; j < ARRAY_SIZE(clock_enum); j++) {
                if (ext_clk_tb[i] == clock_enum[j].type) {
                    y_printf("--- %d  %s \n", clock_enum[j].clock, clock_enum[j].name);
                    continue;
                }
            }
        }
    }
}


u8 clock_idle_selet(u32 type)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(clock_enum); i++) {
        if (type ==  clock_enum[i].type) {
            /* y_printf("--- %d  %s \n", clock_enum[i].clock, clock_enum[i].name); */
            return clock_enum[i].clock;
        }
    }

    return 24;
}

u8 clock_ext_push(u8 ext_type)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (ext_type == ext_clk_tb[i]) {
            return 0;
        }
    }

    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (!ext_clk_tb[i]) {
            ext_clk_tb[i] = ext_type;
            return 1;
        }
    }

    y_printf("clock ext over!!! \n");
    return 0;
}

u8 clock_ext_pop(u8 ext_type)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (ext_type == ext_clk_tb[i]) {
            ext_clk_tb[i] = 0;
            return 1;
        }
    }
    return 0;
}

u16 clock_match(u16 clk)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(clock_tb); i++) {
        if (clk <= clock_tb[i]) {
            return clock_tb[i];
        }
    }
    y_printf("clock overlimit!!! %d\n", clock_tb[ARRAY_SIZE(clock_tb) - 1]);
    return clock_tb[ARRAY_SIZE(clock_tb) - 1];
}


u16 clock_ext_cal()
{
    u32 ext_clk = 0 ;
    u8 i, j;

    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (ext_clk_tb[i]) {
            for (j = 0; j < ARRAY_SIZE(clock_enum); j++) {
                if (ext_clk_tb[i] == clock_enum[j].type) {
                    /* y_printf("--- %d  %s \n", clock_enum[j].clock, clock_enum[j].name); */
                    ext_clk += clock_enum[j].clock;
                    continue;
                }
            }
        }
    }

    return ext_clk;
}

u32 clock_cur_cal()
{
    u32 idle_clk, cur_clk, ext_clk;
    local_irq_disable();
    idle_clk = clock_idle_selet(idle_type);
    ext_clk = clock_ext_cal();
    cur_clk = idle_clk + ext_clk;
    cur_clk = clock_match(cur_clk);
    local_irq_enable();
    return cur_clk ;
}

void clock_pause_play(u8 mode)
{
    u32 idle_clk, cur_clk ;
    if (mode) {
        idle_clk = clock_idle_selet(idle_type);
        clk_set("sys", idle_clk * 1000000L);
    } else {
        clock_ext_dump();
        cur_clk = clock_cur_cal();
        clk_set("sys", cur_clk * 1000000L);
    }
}

void clock_idle(u32 type)
{
    u32 cur_clk;
    local_irq_disable();
    idle_type = type;
    cur_clk = clock_cur_cal();
    local_irq_enable();
    clock_ext_dump();
    clk_set("sys", cur_clk * 1000000L);
}

//////把时钟设置加入到ext中，但是不是立刻设置时钟
void  clock_add(u32 type)
{
    u32 cur_clk ;
    u8 resoult = clock_ext_push(type);
    if (!resoult) {
        return;
    }
}

void clock_remove(u32 type)
{
    u32 cur_clk ;

    u8 resoult = clock_ext_pop(type);
    if (!resoult) {
        return;
    }
}

void clock_set_cur(void)
{
    u32 cur_clk ;
    clock_ext_dump();
    cur_clk = clock_cur_cal();
    clk_set("sys", cur_clk * 1000000L);
}

//////把时钟设置加入到ext中，立刻设置时钟
void clock_add_set(u32 type)
{
    u32 cur_clk ;
    u8 resoult = clock_ext_push(type);
    if (!resoult) {
        return;
    }
    clock_ext_dump();
    cur_clk = clock_cur_cal();
    clk_set("sys", cur_clk * 1000000L);
}

void clock_remove_set(u32 type)
{
    u32 cur_clk ;

    u8 resoult = clock_ext_pop(type);
    if (!resoult) {
        return;
    }

    clock_ext_dump();
    cur_clk = clock_cur_cal();
    clk_set("sys", cur_clk * 1000000L);
}
