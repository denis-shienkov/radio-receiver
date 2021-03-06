#include "fwapp.h"
#include "fwapp_hid.h"

#include <libopencm3/usb/hid.h>

#include <stddef.h> // for NULL

static usbd_device *m_dev = NULL;
static fwapp_hid_report_cb m_recv_report_cb = NULL;
static fwapp_hid_report_cb m_send_report_cb = NULL;
static volatile bool m_need_call_recv_cb = false;
static volatile bool m_need_call_send_cb = false;

// This HID report descriptor declares the following usages:
// - input report, in 64 bytes
// - output report, in 64 bytes
static const uint8_t m_hid_report_dsc[] = {
    0x06, 0x00, 0xff,                   // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,                         // USAGE (Vendor Usage 1)
    0xa1, 0x01,                         // COLLECTION (Application)
    0x15, 0x00,                         //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                   //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                         //   REPORT_SIZE (8)
    0x95, UDB_HID_REPORT_PAYLOAD_SIZE,  //   REPORT_COUNT (32)
    0x09, 0x01,                         //   USAGE (Vendor Usage 1)
    0x81, 0x02,                         //   INPUT (Data,Var,Abs)
    0x95, UDB_HID_REPORT_PAYLOAD_SIZE,  //   REPORT_COUNT (32)
    0x09, 0x01,                         //   USAGE (Vendor Usage 1)
    0x91, 0x02,                         //   OUTPUT (Data,Var,Abs)
    0xc0                                // END_COLLECTION
};

struct usb_hid_report {
    uint8_t bReportDescriptorType;
    uint16_t wDescriptorLength;
} __attribute__((packed));

static const struct {
    struct usb_hid_descriptor definition;
    struct usb_hid_report report;
} __attribute__((packed)) m_hid_function = {

    .definition = {
        .bLength = sizeof(m_hid_function),
        .bDescriptorType = USB_DT_HID,
        .bcdHID = USB_BCD_HID,
        .bCountryCode = 0,
        .bNumDescriptors = 1,
        },

    .report = {
        .bReportDescriptorType = USB_DT_REPORT,
        .wDescriptorLength = sizeof(m_hid_report_dsc),
        }
};

static const struct usb_endpoint_descriptor m_hid_endpoints[USB_HID_EP_COUNT] = {
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = USB_HID_EP_IN_ADDRESS,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = USB_HID_EP_LENGTH,
        .bInterval = USB_HID_EP_INTERVAL
    },
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = USB_HID_EP_OUT_ADDRESS,
        .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
        .wMaxPacketSize = USB_HID_EP_LENGTH,
        .bInterval = USB_HID_EP_INTERVAL
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

    .endpoint = m_hid_endpoints,
    .extra = &m_hid_function,
    .extralen = sizeof(m_hid_function)
};

static enum usbd_request_return_codes fwapp_hid_control_request_cb(
    usbd_device *dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    (void)complete;
    (void)dev;

    enum { EXPECTED_BM_REQ_TYPE = USB_REQ_TYPE_IN | USB_REQ_TYPE_INTERFACE };
    if (req->bmRequestType == EXPECTED_BM_REQ_TYPE
        && req->bRequest == USB_REQ_GET_DESCRIPTOR) {
        const uint8_t report_code = get_byte_hi(req->wValue);
        const  uint8_t zero_code = get_byte_lo(req->wValue);
        if (report_code == USB_HID_DT_REPORT && zero_code == 0) {
            *buf = (uint8_t *)m_hid_report_dsc;
            *len = sizeof(m_hid_report_dsc);
            return USBD_REQ_HANDLED;
        }
    }

    return USBD_REQ_NEXT_CALLBACK;
}

static void fwapp_hid_data_send_cb(usbd_device *dev, uint8_t ep)
{
    (void)dev;
    (void)ep;

    m_need_call_send_cb = true;
}

static void fwapp_hid_data_recv_cb(usbd_device *dev, uint8_t ep)
{
    (void)dev;
    (void)ep;

    m_need_call_recv_cb = true;
}

void fwapp_hid_setup(usbd_device *dev)
{
    m_dev = dev;

    usbd_ep_setup(m_dev,
                  USB_HID_EP_IN_ADDRESS,
                  USB_ENDPOINT_ATTR_INTERRUPT,
                  USB_HID_EP_LENGTH,
                  fwapp_hid_data_send_cb);

    usbd_ep_setup(m_dev,
                  USB_HID_EP_OUT_ADDRESS,
                  USB_ENDPOINT_ATTR_INTERRUPT,
                  USB_HID_EP_LENGTH,
                  fwapp_hid_data_recv_cb);

    usbd_register_control_callback(
        m_dev,
        USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        fwapp_hid_control_request_cb);
}

void fwapp_hid_schedule(void)
{
    if (m_need_call_recv_cb) {
        m_need_call_recv_cb = false;
        if (m_recv_report_cb)
            m_recv_report_cb();
    }

    if (m_need_call_send_cb) {
        m_need_call_send_cb = false;
        if (m_send_report_cb)
            m_send_report_cb();
    }
}

uint16_t fwapp_hid_recv_report(uint8_t *report, uint16_t length)
{
    if (length > UDB_HID_REPORT_PAYLOAD_SIZE)
        length = UDB_HID_REPORT_PAYLOAD_SIZE;

    return usbd_ep_read_packet(
        m_dev,
        USB_HID_EP_OUT_ADDRESS,
        report,
        length);
}

uint16_t fwapp_hid_send_report(const uint8_t *report, uint16_t length)
{
    if (length > UDB_HID_REPORT_PAYLOAD_SIZE)
        length = UDB_HID_REPORT_PAYLOAD_SIZE;

    return usbd_ep_write_packet(
        m_dev,
        USB_HID_EP_IN_ADDRESS,
        report,
        length);
}

void fwapp_hid_register_recv_report_callback(fwapp_hid_report_cb recv_report_cb)
{
    if (recv_report_cb)
        m_recv_report_cb = recv_report_cb;
}

void fwapp_hid_register_send_report_callback(fwapp_hid_report_cb send_report_cb)
{
    if (send_report_cb)
        m_send_report_cb = send_report_cb;
}
