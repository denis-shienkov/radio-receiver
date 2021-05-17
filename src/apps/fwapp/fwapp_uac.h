#ifndef FWAPP_UAC_H
#define FWAPP_UAC_H

#include "fwapp.h"

#include <libopencm3/usb/usbd.h>

extern const struct usb_iface_assoc_descriptor g_uac_iface_assoc_dsc;
extern const struct usb_interface_descriptor g_uac_iface_control_dsc;
extern const struct usb_interface_descriptor g_uac_iface_stream_dscs[];
extern uint8_t g_uac_stream_iface_cur_altsetting;

#define SINE_SAMPLES_FOR_SOF (USB_AUDIO_SAMPLE_RATE * USB_AUDIO_CHANNELS_NUMBER / 1000)

struct fwapp_uac_buffer {
    // Samples for two interleaved channels.
    uint8_t samples[SINE_SAMPLES_FOR_SOF * USB_AUDIO_SUB_FRAME_SIZE];
};

typedef void (*fwapp_uac_set_buffer_cb)(struct fwapp_uac_buffer *buf);

void fwapp_uac_setup(usbd_device *dev);
void fwapp_uac_handle_set_altsetting(usbd_device *dev, uint16_t iface_idx, uint16_t alt_setting);
void fwapp_uac_register_set_buffer_callback(fwapp_uac_set_buffer_cb prep_buf_cb);

#endif // FWAPP_UAC_H
