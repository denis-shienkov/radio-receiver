#ifndef FWAPP_UAC_H
#define FWAPP_UAC_H

#include <libopencm3/usb/usbd.h>

extern const struct usb_iface_assoc_descriptor g_uac_iface_assoc_dsc;
extern const struct usb_interface_descriptor g_uac_iface_control_dsc;
extern const struct usb_interface_descriptor g_uac_iface_stream_dscs[];
extern uint8_t g_uac_stream_iface_cur_altsetting;

void fwapp_uac_setup(usbd_device *dev);
void fwapp_uac_handle_set_altsetting(usbd_device *dev, uint16_t iface_idx, uint16_t alt_setting);
void fwapp_uac_handle_sof(void);

#endif // FWAPP_UAC_H
