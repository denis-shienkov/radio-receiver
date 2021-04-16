#ifndef FWAPP_USB_H
#define FWAPP_USB_H

// Device descriptor defines.

// USB BCD version.
#define USB_BCD_2                   0x0200 // 2.0

// USB device classe.
#define USB_CLASS_MISC              0xEF
// USB device sub-classe.
#define USB_SUBCLASS_COMMON         0x02
// USB device protocol.
#define USB_PROTOCOL_IAD            0x01
// USB device vendor.
#define USB_VENDOR_ID               0xFFFF
// USB device product.
#define USB_PRODUCT_ID              0xFFFF
// USB maximum ep0 size.
#define USB_MAX_EP0_SIZE            64
// USB maximum configurations number.
#define USB_MAX_NUM_CONFIGURATION   1
// USB number of strings.
#define USB_STRINGS_NUMBER          6
#define USB_MANUFACTURER_STRING_IDX 1
#define USB_PRODUCT_STRING_IDX      2
#define USB_SERIALNUM_STRING_IDX    3
#define USB_CONFIG_STRING_IDX       4
#define USB_HID_STRING_IDX          5
#define USB_UAC_STRING_IDX          6

// Device configuration descriptor defines.

// USB number of all interfaces (HID + UAC1).
#define USB_INTERFACES_NUMBER       2
#define USB_HID_INTERFACE_IDX       0
#define USB_UAC_INTERFACE_IDX       1

// USB configuration value.
#define USB_CONFIGURATION_VALUE     1
// USB maximum power.
#define USB_MAX_POWER               0x32

// USB control buffer length.
// This needs to be big enough to hold any descriptor,
// the largest of which will be the configuration descriptor.
#define USB_CONTROL_BUFFER_LENGTH   512

void fwapp_usb_start(void);
void fwapp_usb_stop(void);
void fwapp_usb_schedule(void);

#endif // FWAPP_USB_H