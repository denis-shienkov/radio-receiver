#ifndef FWAPP_HID_H
#define FWAPP_HID_H

#include <libopencm3/usb/usbd.h>

extern const struct usb_interface_descriptor g_hid_iface_dsc;

typedef void (*fwapp_hid_report_cb)(void);

void fwapp_hid_setup(usbd_device *dev);
void fwapp_hid_schedule(void);
uint16_t fwapp_hid_recv_report(uint8_t *report, uint16_t length);
uint16_t fwapp_hid_send_report(const uint8_t *report, uint16_t length);
void fwapp_hid_register_recv_report_callback(fwapp_hid_report_cb recv_report_cb);
void fwapp_hid_register_send_report_callback(fwapp_hid_report_cb send_report_cb);

#endif // FWAPP_HID_H
