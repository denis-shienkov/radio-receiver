#ifndef FWAPP_USB_H
#define FWAPP_USB_H

#include <libopencm3/usb/usbd.h>

void fwapp_usb_start(void);
void fwapp_usb_stop(void);
void fwapp_usb_schedule(void);
void fwapp_usb_dump_setup_req(struct usb_setup_data *req);

#endif // FWAPP_USB_H
