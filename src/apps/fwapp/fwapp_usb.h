#ifndef FWAPP_USB_H
#define FWAPP_USB_H

#include <libopencm3/usb/usbd.h>

typedef void (*fwapp_usb_sof_cb)(void);

void fwapp_usb_start(void);
void fwapp_usb_stop(void);
void fwapp_usb_schedule(void);
void fwapp_usb_dump_setup_req(struct usb_setup_data *req);
bool fwapp_usb_register_sof_callback(fwapp_usb_sof_cb sof_cb);

#endif // FWAPP_USB_H
