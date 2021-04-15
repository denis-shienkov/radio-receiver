#include "fwapp_usb_composite.h"

#include <libopencm3/stm32/gpio.h>

#include <libopencm3/usb/audio.h>
#include <libopencm3/usb/usbd.h>

#include <stddef.h> // for NULL

static usbd_device *m_usbd_dev = NULL;
static uint8_t m_usbd_control_buffer[128] = {0};

static const struct usb_device_descriptor m_usb_dev_dsc = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0xFF,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0xFFFF,
    .idProduct = 0xFFFF,
    .bcdDevice = 0x0200,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1
};

static const struct usb_interface_descriptor m_usb_iface_dsc = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = 0xFF,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0
};

static const struct usb_interface m_usb_ifaces[] = {
    {
        .num_altsetting = 1,
        .altsetting = &m_usb_iface_dsc
    }
};

static const struct usb_config_descriptor m_usb_config_dsc = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0x80,
    .bMaxPower = 0x32,
    .interface = m_usb_ifaces
};

static const char *m_usb_strings[] = {
    "Denis Shienkov", // Manufacturer string.
    "SI4730 AM/FM Radio Receiver", // Product string.
    "12345678", // Serial number string.
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

static enum usbd_request_return_codes fwapp_usb_composite_control_cb(
    usbd_device *usbd_dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    (void)buf;
    (void)len;
    (void)complete;
    (void)usbd_dev;

    if (req->bmRequestType != 0x40)
        return USBD_REQ_NOTSUPP; // Only accept vendor request.

    return USBD_REQ_HANDLED;
}

static void fwapp_usb_composite_set_config_cb(usbd_device *usbd_dev, uint16_t wValue)
{
    (void)wValue;
    usbd_register_control_callback(usbd_dev, USB_REQ_TYPE_VENDOR, USB_REQ_TYPE_TYPE,
                                   fwapp_usb_composite_control_cb);
}

static void fwapp_usb_composite_setup(void)
{
    m_usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &m_usb_dev_dsc,
                           &m_usb_config_dsc, m_usb_strings, 3,
                           m_usbd_control_buffer, sizeof(m_usbd_control_buffer));

    usbd_register_set_config_callback(m_usbd_dev, fwapp_usb_composite_set_config_cb);
}

void fwapp_usb_composite_start(void)
{
    fwapp_usb_composite_reenumerate();
    fwapp_usb_composite_setup();
}

void fwapp_usb_composite_stop(void)
{
    // TODO: I,plement me.
}

void fwapp_usb_composite_schedule(void)
{
    usbd_poll(m_usbd_dev);
}
