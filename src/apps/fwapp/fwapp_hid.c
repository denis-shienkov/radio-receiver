#include "fwapp_hid.h"
#include "fwapp_usb.h"

#include <libopencm3/usb/hid.h>

#include <stddef.h> // for NULL

// USB HID BCD version.
#define USB_BCD_HID                 0x0100 // 1.0
// USB HID endpoint addresses.
#define USB_HID_EP_COUNT            2
#define USB_HID_EP_IN_ADDRESS       0x81
#define USB_HID_EP_OUT_ADDRESS      0x01
// USB HID endpoint buffer size.
#define USB_HID_EP_LENGTH           64
// USB HID endpoint polling interval (16 ms).
#define USB_HID_EP_POLL_INTERVAL    0x20

// This HID report descriptor declares three usages:
// - input report, in 1024 bytes
// - output report, in 1024 bytes
// - feature report, in 64 bytes
static const uint8_t m_usb_hid_report_dsc[] = {
    0x06, 0x00, 0xff,              // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x96, 0x00, 0x04,              //   REPORT_COUNT (1024)
    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x96, 0x00, 0x04,              //   REPORT_COUNT (1024)
    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x40,                    //   REPORT_COUNT (64)
    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0xc0                           // END_COLLECTION
};

static const struct {
    struct usb_hid_descriptor hid_descriptor;
    struct {
        uint8_t bReportDescriptorType;
        uint16_t wDescriptorLength;
    } __attribute__((packed)) hid_report;
} __attribute__((packed)) m_usb_hid_function = {
    .hid_descriptor = {
        .bLength = sizeof(m_usb_hid_function),
        .bDescriptorType = USB_DT_HID,
        .bcdHID = USB_BCD_HID,
        .bCountryCode = 0,
        .bNumDescriptors = 1,
        },
    .hid_report = {
        .bReportDescriptorType = USB_DT_REPORT,
        .wDescriptorLength = sizeof(m_usb_hid_report_dsc),
        }
};

static const struct usb_endpoint_descriptor m_usb_hid_endpoints[USB_HID_EP_COUNT] = {
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = USB_HID_EP_IN_ADDRESS,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = USB_HID_EP_LENGTH,
        .bInterval = USB_HID_EP_POLL_INTERVAL
    },
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = USB_HID_EP_OUT_ADDRESS,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = USB_HID_EP_LENGTH,
        .bInterval = USB_HID_EP_POLL_INTERVAL
    }
};

const struct usb_interface_descriptor g_hid_iface_dsc = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = USB_HID_INTERFACE_IDX,
    .bAlternateSetting = 0,
    .bNumEndpoints = USB_HID_EP_COUNT,
    .bInterfaceClass = USB_CLASS_HID,
    .bInterfaceSubClass = USB_HID_SUBCLASS_NO,
    .bInterfaceProtocol = USB_HID_INTERFACE_PROTOCOL_NONE,
    .iInterface = USB_HID_STRING_IDX,

    .endpoint = m_usb_hid_endpoints,
    .extra = &m_usb_hid_function,
    .extralen = sizeof(m_usb_hid_function)
};

static enum usbd_request_return_codes fwapp_hid_control_request_cb(
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

    enum { EXPECTED_BM_REQ_TYPE = USB_REQ_TYPE_IN | USB_REQ_TYPE_INTERFACE };
    if (req->bmRequestType != EXPECTED_BM_REQ_TYPE)
        return USBD_REQ_NOTSUPP; // Only accept the interface input request.
    if (req->bRequest != USB_REQ_GET_DESCRIPTOR)
        return USBD_REQ_NOTSUPP; // Only accept the get descriptor request.
    enum { EXPECTED_VALUE = (USB_HID_DT_REPORT << 8) };
    if (req->wValue != EXPECTED_VALUE)
        return USBD_REQ_NOTSUPP; // Only accept the HID report descriptor request.

    // Send the HID report descriptor.
    *buf = (uint8_t *)m_usb_hid_report_dsc;
    *len = sizeof(m_usb_hid_report_dsc);
    return USBD_REQ_HANDLED;
}

void fwapp_hid_ep_setup(usbd_device *usbd_dev)
{
    usbd_ep_setup(usbd_dev,
                  USB_HID_EP_IN_ADDRESS,
                  USB_ENDPOINT_ATTR_INTERRUPT,
                  USB_HID_EP_LENGTH,
                  NULL);

    usbd_ep_setup(usbd_dev,
                  USB_HID_EP_OUT_ADDRESS,
                  USB_ENDPOINT_ATTR_INTERRUPT,
                  USB_HID_EP_LENGTH,
                  NULL);

    usbd_register_control_callback(
        usbd_dev,
        USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        fwapp_hid_control_request_cb);
}
