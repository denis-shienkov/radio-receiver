#ifndef FWAPP_HID_H
#define FWAPP_HID_H

#include <libopencm3/usb/usbd.h>

// USB HID configuration.

// USB HID BCD version.
#define USB_BCD_HID                     0x0100 // 1.0
// USB HID endpoint addresses.
#define USB_HID_EP_COUNT                2
#define USB_HID_EP_IN_ADDRESS           0x81
#define USB_HID_EP_OUT_ADDRESS          0x01
// USB HID endpoint buffer size.
#define USB_HID_EP_LENGTH               64
// USB HID endpoint polling interval.
#define USB_HID_EP_POLL_INTERVAL        0x20
// USB HID report data size.
#define UDB_HID_REPORT_DATA_SIZE        32

extern const struct usb_interface_descriptor g_hid_iface_dsc;

typedef void (*fwapp_hid_report_cb)(void);

void fwapp_hid_setup(usbd_device *dev);
uint16_t fwapp_hid_recv_report(uint8_t *report, uint16_t length);
uint16_t fwapp_hid_send_report(const uint8_t *report, uint16_t length);
void fwapp_hid_register_recv_report_callback(fwapp_hid_report_cb rec_report_cb);
void fwapp_hid_register_send_report_callback(fwapp_hid_report_cb send_report_cb);

#endif // FWAPP_HID_H
