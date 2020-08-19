#include "system/includes.h"
#include "app_power_manage.h"
#include "app_main.h"
#include "app_config.h"
#include "app_action.h"
#include "asm/charge.h"
#include "ui_manage.h"
#include "tone_player.h"
#include "asm/adc_api.h"
#include "btstack/avctp_user.h"
#include "user_cfg.h"
#include "asm/charge.h"

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif

#define LOG_TAG_CONST       APP_POWER
#define LOG_TAG             "[APP_POWER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

enum {
    VBAT_NORMAL = 0,
    VBAT_WARNING,
    VBAT_LOWPOWER,
} VBAT_STATUS;

enum {
    VBAT_TIMER_2_MS = 0,
    VBAT_TIMER_10_S,
} VBAT_CHECK_TIME;

static int vbat_timer = 0;
static u8 vbat_check_idle = 1;
static u8 old_battery_level = 9;
static u16 bat_val = 0;
static volatile u8 cur_battery_level = 0;
static u16 battery_full_value = 0;
static u8 tws_sibling_bat_level = 0xff;
static u8 tws_sibling_bat_percent_level = 0xff;

void vbat_check(void *priv);
void sys_enter_soft_poweroff(void *priv);
void clr_wdt(void);
void power_event_to_user(u8 event);


u8 get_tws_sibling_bat_level(void)
{
#if TCFG_USER_TWS_ENABLE
    /* log_info("***********get_tws_sibling_bat_level: %2x", tws_sibling_bat_percent_level); */
    return tws_sibling_bat_level & 0x7f;
#endif
    return 0xff;
}

u8 get_tws_sibling_bat_persent(void)
{
#if TCFG_USER_TWS_ENABLE
    /* log_info("***********get_tws_sibling_bat_level: %2x", tws_sibling_bat_percent_level); */
    return tws_sibling_bat_percent_level;
#endif
    return 0xff;
}

//soundbox
/* void set_tws_sibling_bat_level(u16 level) */
/* { */
/* #if TCFG_USER_TWS_ENABLE */
/* log_info("***********set_tws_sibling_bat_level: %2x", level); */
/* tws_sibling_bat_level = level; */
/* tws_sibling_bat_percent_level = level >> 8; */
/*
 ** 发出电量同步事件进行进一步处理
 **/
/* power_event_to_user(POWER_EVENT_SYNC_TWS_VBAT_LEVEL); */
/* #endif */
/* } */

/* void tws_sync_bat_level(void) */
/* { */
/* #if (TCFG_USER_TWS_ENABLE && BT_SUPPORT_DISPLAY_BAT) */
/* struct tws_sync_info_t vbat_m; */
/* int result = 0; */
/* u8 battery_level = cur_battery_level; */
/* #if CONFIG_DISPLAY_DETAIL_BAT */
/* u8 percent_level = get_vbat_percent(); */
/* #else */
/* u8 percent_level = get_self_battery_level() * 10 + 10; */
/* #endif */
/* if (get_charge_online_flag()) { */
/* percent_level |= BIT(7); */
/* } */
/* vbat_m.type = TWS_SYNC_VBAT; */
/* vbat_m.u.vbat_lev = battery_level; */
/* vbat_m.u.vbat_lev |= (percent_level << 8); */
/* log_info("***********tws_sync_bat_level: %2x", vbat_m.u.vbat_lev); */
/* result = tws_api_send_data_to_sibling((u8 *)&vbat_m, sizeof(struct tws_sync_info_t)); */
/* #endif */
/* } */


void app_power_set_tws_sibling_bat_level(u8 vbat, u8 percent)
{
#if TCFG_USER_TWS_ENABLE
    tws_sibling_bat_level = vbat;
    tws_sibling_bat_percent_level = percent;
    /*
     ** 发出电量同步事件进行进一步处理
     **/
    power_event_to_user(POWER_EVENT_SYNC_TWS_VBAT_LEVEL);

    log_info("set_sibling_bat_level: %d, %d\n", vbat, percent);
#endif
}


static void set_tws_sibling_bat_level(void *_data, u16 len, bool rx)
{
    u8 *data = (u8 *)_data;

    if (rx) {
        app_power_set_tws_sibling_bat_level(data[0], data[1]);
    }
}

#if TCFG_USER_TWS_ENABLE
REGISTER_TWS_FUNC_STUB(vbat_sync_stub) = {
    .func_id = TWS_FUNC_ID_VBAT_SYNC,
    .func    = set_tws_sibling_bat_level,
};
#endif

void tws_sync_bat_level(void)
{
#if (TCFG_USER_TWS_ENABLE && BT_SUPPORT_DISPLAY_BAT)
    u8 battery_level = cur_battery_level;
#if CONFIG_DISPLAY_DETAIL_BAT
    u8 percent_level = get_vbat_percent();
#else
    u8 percent_level = get_self_battery_level() * 10 + 10;
#endif
    if (get_charge_online_flag()) {
        percent_level |= BIT(7);
    }

    u8 data[2];
    data[0] = battery_level;
    data[1] = percent_level;
    tws_api_send_data_to_sibling(data, 2, TWS_FUNC_ID_VBAT_SYNC);

    log_info("tws_sync_bat_level: %d,%d\n", battery_level, percent_level);
#endif
}

void power_event_to_user(u8 event)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg  = (void *)DEVICE_EVENT_FROM_POWER;
    e.u.dev.event = event;
    e.u.dev.value = 0;
    sys_event_notify(&e);
}

int app_power_event_handler(struct device_event *dev)
{
    int ret = false;

#if(TCFG_SYS_LVD_EN == 1)
    switch (dev->event) {
    case POWER_EVENT_POWER_NORMAL:
        ui_update_status(STATUS_EXIT_LOWPOWER);
        break;
    case POWER_EVENT_POWER_WARNING:
        ui_update_status(STATUS_LOWPOWER);
        /* tone_play(TONE_LOW_POWER); */
        STATUS *p_tone = get_tone_config();
        tone_play_index(p_tone->lowpower, 0);
        break;
    case POWER_EVENT_POWER_LOW:
        r_printf(" POWER_EVENT_POWER_LOW");
        sys_enter_soft_poweroff(NULL);
        break;
#if TCFG_USER_TWS_ENABLE
    case POWER_EVENT_SYNC_TWS_VBAT_LEVEL:
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            user_send_cmd_prepare(USER_CTRL_HFP_CMD_UPDATE_BATTARY, 0, NULL);
        }
        break;
#endif

    case POWER_EVENT_POWER_CHANGE:
        /* log_info("POWER_EVENT_POWER_CHANGE\n"); */
#if TCFG_USER_TWS_ENABLE
        if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
            if (tws_api_get_tws_state()&TWS_STA_ESCO_OPEN) {
                break;
            }
            tws_sync_bat_level();
        }
#endif
        user_send_cmd_prepare(USER_CTRL_HFP_CMD_UPDATE_BATTARY, 0, NULL);
        break;
    default:
        break;
    }
#endif

    return ret;
}


u16 get_vbat_level(void)
{
    //return 370;     //debug
    return (adc_get_voltage(AD_CH_VBAT) * 4 / 10);
}

#ifdef CONFIG_QCY_CASE_ENABLE
#define USER_DEFINE_BAT_LEVEL       1
#else
#define USER_DEFINE_BAT_LEVEL       0
#endif

#if USER_DEFINE_BAT_LEVEL
#define USER_BAT_LEVEL 9
static const u16 bat_val_tab[USER_BAT_LEVEL] = {
    LOW_POWER_WARN_VAL, 368, 372, 375, 378, 382, 388, 396, 405
};
#endif

u8 get_vbat_percent(void)
{
    u16 tmp_bat_val;
    u16 bat_val = get_vbat_level();
    if (battery_full_value == 0) {
#if TCFG_CHARGE_ENABLE
        battery_full_value = (get_charge_full_value() - 100) / 10; //防止部分电池充不了这么高电量，充满显示未满的情况
#else
        battery_full_value = 420;
#endif
    }

    if (bat_val <= app_var.poweroff_tone_v) {
        return 0;
    }
#if USER_DEFINE_BAT_LEVEL
    u8 i;
    static u16 last_bat_val = 0;
    if ((last_bat_val == 0)) { //电量变化再0.01V以内不变，
        last_bat_val = bat_val;
    } else if (get_charge_online_flag()) { //充电时电量实时变化
        last_bat_val = bat_val;
    } else if ((last_bat_val - bat_val) > 1) { //防止电压反弹时电量跳动，没充电时电量只会越来越少
        last_bat_val = bat_val;
    }
    for (i = 0; i < USER_BAT_LEVEL; i++) {
        if (last_bat_val < bat_val_tab[i]) {
            break;
        }
    }
    tmp_bat_val = (i + 1) * 10;
#else
    tmp_bat_val = ((u32)bat_val - app_var.poweroff_tone_v) * 100 / (battery_full_value - app_var.poweroff_tone_v);
    if (tmp_bat_val > 100) {
        tmp_bat_val = 100;
    }
#endif
    return (u8)tmp_bat_val;
}

bool get_vbat_need_shutdown(void)
{
    if (bat_val <= LOW_POWER_SHUTDOWN) {
        return TRUE;
    }
    return FALSE;
}

//将当前电量转换为1~9级发送给手机同步电量
u8  battery_value_to_phone_level(u16 bat_val)
{
    u8  battery_level = 0;
    u8 vbat_percent = get_vbat_percent();

    if (vbat_percent < 5) { //小于5%电量等级为0，显示10%
        return 0;
    }

    battery_level = (vbat_percent - 5) / 10;

    return battery_level;
}

//获取自身的电量
u8  get_self_battery_level(void)
{
    return cur_battery_level;
}

u8  get_cur_battery_level(void)
{
    u8 bat_lev = tws_sibling_bat_level & (~BIT(7));
#if TCFG_USER_TWS_ENABLE
    if (bat_lev == 0x7f) {
        return cur_battery_level;
    }

#if (CONFIG_DISPLAY_TWS_BAT_TYPE == CONFIG_DISPLAY_TWS_BAT_LOWER)
    return cur_battery_level < bat_lev ? cur_battery_level : bat_lev;
#elif (CONFIG_DISPLAY_TWS_BAT_TYPE == CONFIG_DISPLAY_TWS_BAT_HIGHER)
    return cur_battery_level < bat_lev ? bat_lev : cur_battery_level;
#elif (CONFIG_DISPLAY_TWS_BAT_TYPE == CONFIG_DISPLAY_TWS_BAT_LEFT)
    return tws_api_get_local_channel() == 'L' ? cur_battery_level : bat_lev;
#elif (CONFIG_DISPLAY_TWS_BAT_TYPE == CONFIG_DISPLAY_TWS_BAT_RIGHT)
    return tws_api_get_local_channel() == 'R' ? cur_battery_level : bat_lev;
#else
    return cur_battery_level;
#endif //END CONFIG_DISPLAY_TWS_BAT_TYPE

#else  //TCFG_USER_TWS_ENABLE == 0
    return cur_battery_level;
#endif
}

void vbat_check_init(void)
{
    if (vbat_timer == 0) {
        vbat_timer = sys_timer_add(NULL, vbat_check, 2);
        vbat_check_idle = 0;
    }
}

void vbat_timer_update(u32 msec)
{
    if (vbat_timer) {
        sys_timer_modify(vbat_timer,msec);
        /* sys_timer_del(vbat_timer); */
        /* vbat_timer = sys_timer_add(NULL, vbat_check, msec); */
    }
}

void vbat_timer_delete(void)
{
    if (vbat_timer) {
        sys_timer_del(vbat_timer);
        vbat_timer = 0;
        vbat_check_idle = 1;
    }
}

static u8 cur_bat_st = VBAT_NORMAL;
void vbat_check(void *priv)
{
    static u8 cur_timer_period = VBAT_TIMER_2_MS;
    static u8 unit_cnt = 0;
    static u8 low_warn_cnt = 0;
    static u8 low_off_cnt = 0;
    static u8 low_voice_cnt = 0;
    static u8 low_power_cnt = 0;
    static u8 power_normal_cnt = 0;
    static u8 charge_ccvol_v_cnt = 0;
    static u8 charge_online_flag = 0;
#if USER_DEFINE_BAT_LEVEL
    static u8 low_voice_first_flag = 1;//进入低电后先提醒一次，后面三分钟提醒一次
#endif

    u8 detect_cnt = 6;

    if (cur_timer_period == VBAT_TIMER_10_S) {
        vbat_timer_update(2);
        cur_timer_period = VBAT_TIMER_2_MS;
        vbat_check_idle = 0;
    }

    if (!bat_val) {
        bat_val = get_vbat_level();
    } else {
        bat_val = (get_vbat_level() + bat_val) / 2;
    }

    cur_battery_level = battery_value_to_phone_level(bat_val);

    /* printf("bv:%d, bl:%d\n", bat_val, cur_battery_level); */

    unit_cnt++;

    /* if (bat_val < LOW_POWER_OFF_VAL) { */
    if (bat_val <= app_var.poweroff_tone_v) {
        low_off_cnt++;
    }
    /* if (bat_val < LOW_POWER_WARN_VAL) { */
    if (bat_val <= app_var.warning_tone_v) {
        low_warn_cnt++;
    }
#if TCFG_CHARGE_ENABLE
    if (bat_val >= CHARGE_CCVOL_V) {
        charge_ccvol_v_cnt++;
    }
#endif

    /* log_info("unit_cnt:%d\n", unit_cnt); */

    if (unit_cnt >= detect_cnt) {

        if (get_charge_online_flag() == 0) {
            if (low_off_cnt > (detect_cnt / 2)) { //低电关机
                low_power_cnt++;
                low_voice_cnt = 0;
                power_normal_cnt = 0;
                cur_bat_st = VBAT_LOWPOWER;
                if (low_power_cnt > 6) {
                    log_info("\n*******Low Power,enter softpoweroff********\n");
                    low_power_cnt = 0;
                    sys_timer_del(vbat_timer);
                    power_event_to_user(POWER_EVENT_POWER_LOW);
                }
            } else if (low_warn_cnt > (detect_cnt / 2)) { //低电提醒
                low_voice_cnt ++;
                low_power_cnt = 0;
                power_normal_cnt = 0;
                cur_bat_st = VBAT_WARNING;
#if USER_DEFINE_BAT_LEVEL
                if ((low_voice_first_flag && low_voice_cnt > 1) || //第一次进低电10s后报一次
                    (!low_voice_first_flag && low_voice_cnt >= 17)) { //3分钟报一次
                    low_voice_first_flag = 0;
#else
                if (low_voice_cnt > 5) {
#endif
                    log_info("\n**Low Power,Please Charge Soon!!!**\n");
                    low_voice_cnt = 0;
                    power_event_to_user(POWER_EVENT_POWER_WARNING);
                }
            } else {
                power_normal_cnt++;
                low_voice_cnt = 0;
                low_power_cnt = 0;
                if (power_normal_cnt > 2) {
                    if (cur_bat_st != VBAT_NORMAL) {
                        log_info("[Noraml power]\n");
                        cur_bat_st = VBAT_NORMAL;
                        power_event_to_user(POWER_EVENT_POWER_NORMAL);
                    }
                }
            }
        } else {
#if TCFG_CHARGE_ENABLE
            if (charge_ccvol_v_cnt > (detect_cnt / 2)) {
                set_charge_mA(get_charge_mA_config());
            }
#endif
        }

        unit_cnt = 0;
        low_off_cnt = 0;
        low_warn_cnt = 0;
        charge_ccvol_v_cnt = 0;

        if ((cur_bat_st != VBAT_LOWPOWER) && (cur_timer_period == VBAT_TIMER_2_MS)) {
            if (get_charge_online_flag()) {
                vbat_timer_update(60 * 1000);
            } else {
                vbat_timer_update(10 * 1000);
            }
            cur_timer_period = VBAT_TIMER_10_S;
            vbat_check_idle = 1;
            cur_battery_level = battery_value_to_phone_level(bat_val);
            if (cur_battery_level != old_battery_level) {
                power_event_to_user(POWER_EVENT_POWER_CHANGE);
            } else {
                if (charge_online_flag != get_charge_online_flag()) {
                    //充电变化也要交换，确定是否在充电仓
                    power_event_to_user(POWER_EVENT_POWER_CHANGE);
                }
            }
            charge_online_flag =  get_charge_online_flag();
            old_battery_level = cur_battery_level;
        }
    }
}

bool vbat_is_low_power(void)
{
    return (cur_bat_st != VBAT_NORMAL);
}

static u8 vbat_check_idle_query(void)
{
    return vbat_check_idle;
}

REGISTER_LP_TARGET(vbat_check_lp_target) = {
    .name = "vbat_check",
    .is_idle = vbat_check_idle_query,
};


void check_power_on_voltage(void)
{
#if(TCFG_SYS_LVD_EN == 1)

    u16 val = 0;
    u8 normal_power_cnt = 0;
    u8 low_power_cnt = 0;

    while (1) {
        clr_wdt();
        val = get_vbat_level();
        printf("vbat: %d\n", val);
        if (val < app_var.poweroff_tone_v) {
            low_power_cnt++;
            normal_power_cnt = 0;
            if (low_power_cnt > 10) {
                ui_update_status(STATUS_POWERON_LOWPOWER);
                os_time_dly(100);
                log_info("power on low power , enter softpoweroff!\n");
                power_set_soft_poweroff();
            }
        } else {
            normal_power_cnt++;
            low_power_cnt = 0;
            if (normal_power_cnt > 10) {
                vbat_check_init();
                return;
            }
        }
    }
#endif
}