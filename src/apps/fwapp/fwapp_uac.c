#include "fwapp_uac.h"
#include "fwapp_usb.h"

#include <libopencm3/usb/audio.h>

const struct usb_interface_descriptor g_uac_iface_dsc = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = USB_UAC_INTERFACE_IDX,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = 0xFF,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = USB_UAC_STRING_IDX
};

static enum usbd_request_return_codes fwapp_uac_control_request_cb(
    usbd_device *usbd_dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    (void)buf;
    (void)complete;
    (void)len;
    (void)usbd_dev;

    enum { EXPECTED_BM_REQ_TYPE = USB_REQ_TYPE_VENDOR };
    if (req->bmRequestType != EXPECTED_BM_REQ_TYPE)
        return USBD_REQ_NOTSUPP; // Only accept vendor request.
    return USBD_REQ_HANDLED;
}

void fwapp_uac_ep_setup(usbd_device *usbd_dev)
{
    // TODO: Setup endpoints here.

    usbd_register_control_callback(
        usbd_dev,
        USB_REQ_TYPE_VENDOR,
        USB_REQ_TYPE_TYPE,
        fwapp_uac_control_request_cb);
}
