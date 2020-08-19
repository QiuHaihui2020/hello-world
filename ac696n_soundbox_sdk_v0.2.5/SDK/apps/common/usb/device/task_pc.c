/**
 * @file task_pc.c
 * @brief 从机模式
 * @author chenrixin@zh-jieli.com
 * @version 1.0.0
 * @date 2020-02-29
 */

#include "system/app_core.h"
#include "system/includes.h"
#include "server/server_core.h"
#include "app_config.h"
#include "app_action.h"
#include "os/os_api.h"
#include "device/sdmmc.h"
#if TCFG_PC_ENABLE
#include "usb/usb_config.h"
#include "usb/device/usb_stack.h"
#include "usb/device/hid.h"
#include "usb/device/msd.h"
#include "audio_config.h"
#include "asm/charge.h"
#include "app_charge.h"
#include "media/audio_base.h"
#include "clock_cfg.h"


#define LOG_TAG_CONST       APP_PC
#define LOG_TAG             "[USB_TASK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define     USB_TASK_NAME   "usb_msd"

#define USBSTACK_EVENT		    0x80
#define USBSTACK_MSD_RUN		0x81
#define USBSTACK_MSD_RELASE		0x82
#define USBSTACK_HID		    0x83
#define USBSTACK_MSD_RESET      0x84

extern int usb_audio_demo_init(void);
extern void usb_audio_demo_exit(void);
extern void charge_event_to_user(u8 event);

static usb_dev usbfd ;//SEC(.usb_g_bss);
static OS_MUTEX msd_mutex ;//SEC(.usb_g_bss);


#if (TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0)
const int usb_dm_multiplex = 1;
#else
const int usb_dm_multiplex = 0;
#endif

static void usb_task(void *p)
{
    int ret = 0;
    int msg[16];
    while (1) {
        ret = os_task_pend("taskq", msg, ARRAY_SIZE(msg));
        if (ret != OS_TASKQ) {
            continue;
        }
        if (msg[0] != Q_MSG) {
            continue;
        }
        switch (msg[1]) {
        case USBSTACK_MSD_RUN:
            os_mutex_pend(&msd_mutex, 0);
            USB_MassStorage((void *)msg[2]);
            os_mutex_post(&msd_mutex);
            break;
        case USBSTACK_MSD_RELASE:
            os_sem_post((OS_SEM *)msg[2]);
            while (1) {
                os_time_dly(10000);
            }
            break;
        case USBSTACK_MSD_RESET:
            os_mutex_pend(&msd_mutex, 0);
            msd_reset((struct usb_device_t *)msg[2], (u32)msg[3]);
            os_mutex_post(&msd_mutex);
            break;
        default:
            break;
        }
    }
}

static void usb_msd_wakeup(struct usb_device_t *usb_device)
{
    int err = os_taskq_post_msg(USB_TASK_NAME, 2, USBSTACK_MSD_RUN, usb_device);
}
static void usb_msd_reset_wakeup(struct usb_device_t *usb_device, u32 itf_num)
{
    os_taskq_post_msg(USB_TASK_NAME, 3, USBSTACK_MSD_RESET, usb_device, itf_num);
}
static void usb_msd_init()
{
    r_printf("%s()", __func__);
    int err;
    os_mutex_create(&msd_mutex);
    err = task_create(usb_task, NULL, USB_TASK_NAME);
    if (err != OS_NO_ERR) {
        r_printf("usb_msd task creat fail %x\n", err);
    }
}
static void usb_msd_free()
{
    r_printf("%s()", __func__);

    os_mutex_del(&msd_mutex, 0);

    int err;
    OS_SEM *sem = zalloc(sizeof(OS_SEM));;
    os_sem_create(sem, 0);
    os_taskq_post_msg(USB_TASK_NAME, 2, USBSTACK_MSD_RELASE, (int)sem);
    os_sem_pend(sem, 0);
    free(sem);


    err = task_kill(USB_TASK_NAME);
    if (!err) {
        r_printf("usb_msd_uninit succ!!\n");
    } else {
        r_printf("usb_msd_uninit fail!!\n");
    }
}

void usb_start()
{
    /* #if CONFIG_CLOCK_BY_TYPE
        matching_code_type(AUDIO_ANOTHER_CLK_TYPE_PC, 1);
    #else
        clk_set("sys", PC_CLOCK);
    #endif */

    clock_idle(PC_IDLE_CLOCK);

#if USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS
    usb_audio_demo_init();
#endif

    usb_device_mode(usbfd, USB_DEVICE_CLASS_CONFIG);

    if (!usb_dm_multiplex) {
#if TCFG_SD0_ENABLE
        msd_register_disk("sd0", NULL);
#endif
#if TCFG_SD1_ENABLE
        msd_register_disk("sd1", NULL);
#endif
    }
#if TCFG_NORFLASH_DEV_ENABLE
    msd_register_disk("norflash", NULL);
#endif

#if USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS
    msd_set_wakeup_handle(usb_msd_wakeup);
    msd_set_reset_wakeup_handle(usb_msd_reset_wakeup);
    usb_msd_init();
#endif
}
static void usb_remove_disk()
{
#if USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS
    os_mutex_pend(&msd_mutex, 0);
    msd_unregister_all();
    os_mutex_post(&msd_mutex);
#endif
}
void usb_pause()
{
    log_info("usb pause");

    usb_sie_disable(usbfd);

#if USB_DEVICE_CLASS_CONFIG & MASSSTORAGE_CLASS
    if (msd_set_wakeup_handle(NULL)) {
        usb_remove_disk();
        usb_msd_free();
    }
#endif


#if USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS
    usb_audio_demo_exit();
#endif

    usb_device_mode(usbfd, 0);
}

void usb_stop()
{
    log_info("App Stop - usb");

    usb_pause();

    usb_sie_close(usbfd);
}


extern void usb_otg_suspend(usb_dev usb_id, u8 uninstall);
int pc_sys_event_handler(struct sys_event *event)
{
    int switch_app_case = false;
    switch (event->type) {
    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_OTG) {
            const char *usb_msg = (const char *)event->u.dev.value;
            log_debug("usb event : %d DEVICE_EVENT_FROM_OTG %s", event->u.dev.event, usb_msg);

            if (usb_msg[0] == 's') {
                if (event->u.dev.event == DEVICE_EVENT_ONLINE) {
                    log_info("usb %c online", usb_msg[2]);
                    usbfd = usb_msg[2] - '0';
#ifdef  CONFIG_DONGLE_CASE
                    usb_start();
#else

#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
                    usb_otg_suspend(0, 0);
#endif
                    usb_pause();
#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
                    extern void m_sdio_resume(u8 num);
                    m_sdio_resume(2);
#endif

#endif
                    switch_app_case = 1;
                } else {
                    log_info("usb %c offline", usb_msg[2]);
                    switch_app_case = 2;
#ifdef  CONFIG_DONGLE_CASE
                    usb_stop();
#else

#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
                    extern void m_sdio_resume(u8 num);
                    extern int m_sdio_supend(u8 num);
                    m_sdio_resume(2);
                    m_sdio_supend(2);
#endif

                    if (!app_cur_task_check(APP_NAME_PC)) {
                        usb_stop();
                    }
#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
                    extern void m_sdio_resume(u8 num);
                    m_sdio_resume(2);
#endif

#endif
                }
            }
        } else if ((u32)event->arg == DRIVER_EVENT_FROM_SD0) {
            log_info("usb event : %d DRIVER_EVENT_FROM_SD0 ", event->u.dev.event);
            if (event->u.dev.event == DEVICE_EVENT_IN) {
                if (!usb_dm_multiplex) {
                    msd_register_disk("sd0", NULL);
                }
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                if (!usb_dm_multiplex) {
                    msd_unregister_disk("sd0");
                }
            }
        } else if ((u32)event->arg == DRIVER_EVENT_FROM_SD1) {
            log_info("usb event : %d DRIVER_EVENT_FROM_SD0 ", event->u.dev.event);
            if (event->u.dev.event == DEVICE_EVENT_IN) {
                if (!usb_dm_multiplex) {
                    msd_register_disk("sd1", NULL);
                }
            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                if (!usb_dm_multiplex) {
                    msd_unregister_disk("sd1");
                }
            }
        }
        break;
    default:
        log_d("%x\n", event->type);
        break;
    }
    return switch_app_case;
}

#ifdef  CONFIG_DONGLE_CASE
static void usb_event_handler(struct sys_event *event, void *priv)
{
    pc_sys_event_handler(event);
}

static u16 sys_event_id;
void usbstack_init()
{
    sys_event_id = register_sys_event_handler(SYS_ALL_EVENT, 2, NULL, usb_event_handler);
}
void usbstack_exit()
{
    if (sys_event_id) {
        unregister_sys_event_handler(sys_event_id);
        sys_event_id = 0;
    }
}
#endif
#endif
