#include "fwapp_usb_composite.h"

#include <libopencm3/stm32/gpio.h>

#include <libopencm3/usb/audio.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/usb/usbd.h>

#include <stddef.h> // for NULL

static usbd_device *m_usbd_dev = NULL;
static uint8_t m_usbd_control_buffer[USB_CONTROL_BUFFER_LENGTH] = {0};

static const struct usb_device_descriptor m_usb_dev_dsc = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = USB_BCD_2,
    .bDeviceClass = 0, //USB_CLASS_MISC,
    .bDeviceSubClass = 0, //USB_SUBCLASS_COMMON,
    .bDeviceProtocol = 0, // USB_PROTOCOL_IAD,
    .bMaxPacketSize0 = USB_MAX_EP0_SIZE,
    .idVendor = USB_VENDOR_ID,
    .idProduct = USB_PRODUCT_ID,
    .bcdDevice = USB_BCD_2,
    .iManufacturer = USB_MANUFACTURER_STRING_IDX,
    .iProduct = USB_PRODUCT_STRING_IDX,
    .iSerialNumber = USB_SERIALNUM_STRING_IDX,
    .bNumConfigurations = USB_MAX_NUM_CONFIGURATION
};

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

static const struct usb_interface_descriptor m_usb_hid_iface_dsc = {
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

static const struct usb_interface_descriptor m_usb_uac_iface_dsc = {
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

static const struct usb_interface m_usb_ifaces[USB_INTERFACES_NUMBER] = {
    {
        .num_altsetting = 1,
        .altsetting = &m_usb_hid_iface_dsc
    },
    {
        .num_altsetting = 1,
        .altsetting = &m_usb_uac_iface_dsc
    }
};

static const struct usb_config_descriptor m_usb_config_dsc = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,
    .bNumInterfaces = USB_INTERFACES_NUMBER,
    .bConfigurationValue = USB_CONFIGURATION_VALUE,
    .iConfiguration = USB_CONFIG_STRING_IDX,
    .bmAttributes = USB_CONFIG_ATTR_DEFAULT,
    .bMaxPower = USB_MAX_POWER,
    .interface = m_usb_ifaces
};

static const char *m_usb_strings[USB_STRINGS_NUMBER] = {
    "Denis Shienkov", // Manufacturer string.
    "SI4730 AM/FM Radio Receiver", // Product string.
    "12345678", // Serial number string.
    "SI4730 AM/FM Radio Receiver Configuration", // Configuration string.
    "SI4730 AM/FM Radio Receiver Control", // Control string.
    "SI4730 AM/FM Radio Receiver Audio Stream", // Audio stream string.
};

static void fwapp_usb_composite_reenumerate(void)
{
    // This is a somewhat common cheap hack to trigger device re-enumeration
    // on startup. Assuming a fixed external pullup on D+, (For USB-FS)
    // setting the pin to output, and driving it explicitly low effectively
    // "removes" the pullup.  The subsequent USB init will "take over" the
    // pin, and it will appear as a proper pullup to the host.
    // The magic delay is somewhat arbitrary, no guarantees on USBIF
    // compliance here, but "it works" in most places.

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
    gpio_clear(GPIOA, GPIO12);
    for (uint32_t i = 0; i < 800000; ++i) {
        __asm__("nop");
    }
}

static enum usbd_request_return_codes fwapp_usb_composite_uac_control_request_cb(
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

static enum usbd_request_return_codes fwapp_usb_composite_hid_control_request_cb(
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

static void fwapp_usb_composite_set_config_cb(usbd_device *usbd_dev, uint16_t wValue)
{
    (void)wValue;

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
        fwapp_usb_composite_hid_control_request_cb);

    usbd_register_control_callback(
        usbd_dev,
        USB_REQ_TYPE_VENDOR,
        USB_REQ_TYPE_TYPE,
        fwapp_usb_composite_uac_control_request_cb);
}

static void fwapp_usb_composite_setup(void)
{
    m_usbd_dev = usbd_init(
        &st_usbfs_v1_usb_driver,
        &m_usb_dev_dsc,
        &m_usb_config_dsc,
        m_usb_strings,
        USB_STRINGS_NUMBER,
        m_usbd_control_buffer,
        sizeof(m_usbd_control_buffer));

    usbd_register_set_config_callback(
        m_usbd_dev,
        fwapp_usb_composite_set_config_cb);
}

void fwapp_usb_composite_start(void)
{
    fwapp_usb_composite_reenumerate();
    fwapp_usb_composite_setup();
}

void fwapp_usb_composite_stop(void)
{
    // TODO: Implement me.
}

void fwapp_usb_composite_schedule(void)
{
    usbd_poll(m_usbd_dev);
}
