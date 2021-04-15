#ifndef FWAPP_USB_COMPOSITE_H
#define FWAPP_USB_COMPOSITE_H

#include <stdbool.h>
#include <stdint.h>

void fwapp_usb_composite_start(void);
void fwapp_usb_composite_stop(void);
void fwapp_usb_composite_schedule(void);

#endif // FWAPP_USB_COMPOSITE_H
