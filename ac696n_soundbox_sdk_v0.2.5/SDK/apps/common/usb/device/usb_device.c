#include "usb/device/usb_stack.h"
#include "usb_config.h"
#include "usb/device/msd.h"
#include "usb/scsi.h"
#include "usb/device/hid.h"
#include "usb/device/uac_audio.h"
#include "irq.h"
#include "init.h"
#include "gpio.h"
#include "app_config.h"

#if TCFG_PC_ENABLE


static void usb_device_init(const usb_dev usb_id)
{
    usb_config(usb_id);
    usb_g_sie_init(usb_id);
    usb_slave_init(usb_id);
    usb_set_dma_raddr(usb_id, 0, usb_get_ep_buffer(usb_id, 0));
    usb_set_dma_raddr(usb_id, 1, usb_get_ep_buffer(usb_id, 0));
    usb_set_dma_raddr(usb_id, 2, usb_get_ep_buffer(usb_id, 0));
    usb_set_dma_raddr(usb_id, 3, usb_get_ep_buffer(usb_id, 0));
    usb_set_dma_raddr(usb_id, 4, usb_get_ep_buffer(usb_id, 0));

    usb_write_intr_usbe(usb_id, INTRUSB_RESET_BABBLE | INTRUSB_SUSPEND);
    usb_set_intr_txe(usb_id, 0);
    usb_set_intr_rxe(usb_id, 0);
    usb_g_isr_reg(usb_id, 3, 0);
    /* usb_sof_isr_reg(usb_id,3,0); */
    /* usb_sofie_enable(usb_id); */
    /* USB_DEBUG_PRINTF("ep0 addr %x %x\n", usb_get_dma_taddr(0), ep0_dma_buffer); */
}
static void usb_device_hold(const usb_dev usb_id)
{
    usb_clr_intr_txe(usb_id, -1);
    usb_clr_intr_rxe(usb_id, -1);

    usb_g_hold(usb_id);
    usb_release(usb_id);
}


int usb_device_mode(const usb_dev usb_id, const u32 class)
{
    /* usb_device_set_class(CLASS_CONFIG); */
    u8 class_index = 0;
    if (class == 0) {
        gpio_direction_input(IO_PORT_DM + 2 * usb_id);
        gpio_set_pull_up(IO_PORT_DM + 2 * usb_id, 0);
        gpio_set_pull_down(IO_PORT_DM + 2 * usb_id, 0);
        gpio_set_die(IO_PORT_DM + 2 * usb_id, 0);

        gpio_direction_input(IO_PORT_DP + 2 * usb_id);
        gpio_set_pull_up(IO_PORT_DP + 2 * usb_id, 0);
        gpio_set_pull_down(IO_PORT_DP + 2 * usb_id, 0);
        gpio_set_die(IO_PORT_DP + 2 * usb_id, 0);

        os_time_dly(15);

        gpio_set_die(IO_PORT_DM + 2 * usb_id, 1);
        gpio_set_die(IO_PORT_DP + 2 * usb_id, 1);

        msd_release(usb_id);
        uac_release(usb_id);
        usb_device_hold(usb_id);
        return 0;
    }

    usb_add_desc_config(usb_id, MAX_INTERFACE_NUM, NULL);
    if ((class & MASSSTORAGE_CLASS) == MASSSTORAGE_CLASS) {
        msd_register(usb_id);
        usb_add_desc_config(usb_id, class_index++, msd_desc_config);
    }

    if ((class & AUDIO_CLASS) == AUDIO_CLASS) {
        usb_add_desc_config(usb_id, class_index++, uac_audio_desc_config);
    } else if ((class & SPEAKER_CLASS) == SPEAKER_CLASS) {
        usb_add_desc_config(usb_id, class_index++, uac_spk_desc_config);
    } else if ((class & MIC_CLASS) == MIC_CLASS) {
        usb_add_desc_config(usb_id, class_index++, uac_mic_desc_config);
    }

    if ((class & HID_CLASS) == HID_CLASS) {
        usb_add_desc_config(usb_id, class_index++, hid_desc_config);
    }

    usb_device_init(usb_id);
    user_setup_filter_install(usb_id2device(usb_id));
    return 0;
}
/* module_initcall(usb_device_mode); */

#endif
