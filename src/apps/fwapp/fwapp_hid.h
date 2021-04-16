#ifndef FWAPP_HID_H
#define FWAPP_HID_H

#include <libopencm3/usb/usbd.h>

extern const struct usb_interface_descriptor g_hid_iface_dsc;

void fwapp_hid_ep_setup(usbd_device *usbd_dev);

#endif // FWAPP_HID_H
