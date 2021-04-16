#ifndef FWAPP_UAC_H
#define FWAPP_UAC_H

#include <libopencm3/usb/usbd.h>

extern const struct usb_interface_descriptor g_uac_iface_dsc;

void fwapp_uac_setup(usbd_device *dev);

#endif // FWAPP_UAC_H
