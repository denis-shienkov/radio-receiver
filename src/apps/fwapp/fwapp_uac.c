#include "fwapp.h"
#include "fwapp_uac.h"
#include "fwapp_usb.h"

#include <libopencm3/usb/audio.h>
#include <libopencm3/usb/dwc/otg_fs.h>

#include <stddef.h> // for NULL
#include <stdio.h> // for printf
#include <string.h> // for memset

#define USB_AUDIO_ALL_CHANNELS_NUMBER   (USB_AUDIO_CHANNELS_NUMBER + 1)

enum uac_stream_status {
    UAC_STREAM_DISABLED = 0,
    UAC_STREAM_IDLE,
    UAC_STREAM_ENABLED
};

enum uac_sample_buf_index {
    UAC_SAMPLE_BUF0 = 0,
    UAC_SAMPLE_BUF1,
    UAC_SAMPLE_BUFS_COUNT
};

static struct fwapp_uac_buffer m_sample_buffers[UAC_SAMPLE_BUFS_COUNT] = {0};

static enum uac_sample_buf_index m_sample_buf_index = UAC_SAMPLE_BUF0;
static fwapp_uac_set_buffer_cb m_set_buf_cb = NULL;

static usbd_device *m_dev = NULL;
uint8_t g_uac_stream_iface_cur_altsetting = 0;
static enum uac_stream_status m_uac_stream_status = UAC_STREAM_DISABLED;

// Array of channels configuration, include the master channel.
// Note: Should contains swapped ushort values!
static struct usb_audio_ch_cfg {
    uint8_t muted; // =1 - muted.
} m_channels_cfg[USB_AUDIO_ALL_CHANNELS_NUMBER] = {
    {SET_MUTED}, // Master channel.
    {SET_MUTED}, // Left channel.
    {SET_MUTED}  // Right channel.
};

static void fwapp_uac_sof_cb(void)
{
    if (m_uac_stream_status != UAC_STREAM_ENABLED)
        return;
    if (!m_set_buf_cb)
        return;
    struct fwapp_uac_buffer *buf = &m_sample_buffers[m_sample_buf_index];
    // Toggle sample buffer index.
    m_sample_buf_index = (m_sample_buf_index == UAC_SAMPLE_BUF0) ? UAC_SAMPLE_BUF1
                                                                 : UAC_SAMPLE_BUF0;
    m_set_buf_cb(buf);
}

// Association interface configuration.

const struct usb_iface_assoc_descriptor g_uac_iface_assoc_dsc = {
    .bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
    .bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
    .bFirstInterface = USB_UAC_CONTROL_INTERFACE_IDX,
    .bInterfaceCount = USB_UAC_INTERFACES_NUMBER,
    .bFunctionClass = USB_CLASS_AUDIO,
    .bFunctionSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
    .bFunctionProtocol = USB_AUDIO_PROTOCOL_NONE,
    .iFunction = USB_UAC_ASSOC_STRING_IDX
};

// Control interface configuration.

struct usb_audio_header_cs_descriptor {
    struct usb_audio_header_descriptor_head head;
    struct usb_audio_header_descriptor_body body;
} __attribute__((packed));

static const struct {
    struct usb_audio_header_cs_descriptor header_cs_dsc;
    struct usb_audio_input_terminal_descriptor input_terminal_dsc;
    struct usb_audio_feature_unit_descriptor_2ch feature_unit_dsc;
    struct usb_audio_output_terminal_descriptor output_terminal_dsc;
} __attribute__((packed)) m_uac_control_function = {

    .header_cs_dsc = {
        .head = {
            .bLength = sizeof(struct usb_audio_header_cs_descriptor),
            .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
            .bDescriptorSubtype = USB_AUDIO_TYPE_HEADER,
            .bcdADC = USB_BCD_ADC,
            .wTotalLength = sizeof(struct usb_audio_header_cs_descriptor)
                            + sizeof(struct usb_audio_input_terminal_descriptor)
                            + sizeof(struct usb_audio_feature_unit_descriptor_2ch)
                            + sizeof(struct usb_audio_output_terminal_descriptor),
            .binCollection = USB_UAC_STREAM_INTERFACES_NUMBER
        },
        .body = {
            .baInterfaceNr = USB_UAC_STREAM_INTERFACE_IDX
        }
    },

    .input_terminal_dsc = {
        .bLength = sizeof(struct usb_audio_input_terminal_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = USB_AUDIO_TYPE_INPUT_TERMINAL,
        .bTerminalID = USB_AUDIO_INPUT_TERMINAL_ID,
        .wTerminalType = RADIO_RECEIVER,
        .bAssocTerminal = USB_AUDIO_NONE_TERMINAL_ID,
        .bNrChannels = 2,
        .wChannelConfig = (bmLEFT_FRONT | bmRIGHT_FRONT),
        .iChannelNames = 0,
        .iTerminal = 0
    },

    .feature_unit_dsc = {
        .head = {
            .bLength = sizeof(struct usb_audio_feature_unit_descriptor_2ch),
            .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
            .bDescriptorSubtype = USB_AUDIO_TYPE_FEATURE_UNIT,
            .bUnitID = USB_AUDIO_FEATURE_UNITL_ID,
            .bSourceID = USB_AUDIO_INPUT_TERMINAL_ID,
            .bControlSize = 2,
            .bmaControlMaster = bmMUTE
        },
        .channel_control = {
            {
                .bmaControl = 0x0000
            },
            {
                .bmaControl = 0x0000
            }
        },
        .tail = {
            .iFeature = 0x00
        }
    },

    .output_terminal_dsc = {
        .bLength = sizeof(struct usb_audio_output_terminal_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = USB_AUDIO_TYPE_OUTPUT_TERMINAL,
        .bTerminalID = USB_AUDIO_OUTPUT_TERMINAL_ID,
        .wTerminalType = TERMINAL_STREAMING,
        .bAssocTerminal = USB_AUDIO_NONE_TERMINAL_ID,
        .bSourceID = USB_AUDIO_FEATURE_UNITL_ID,
        .iTerminal = 0
    }
};

const struct usb_interface_descriptor g_uac_iface_control_dsc = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = USB_UAC_CONTROL_INTERFACE_IDX,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = USB_CLASS_AUDIO,
    .bInterfaceSubClass = USB_AUDIO_SUBCLASS_CONTROL,
    .bInterfaceProtocol = USB_AUDIO_PROTOCOL_NONE,
    .iInterface = USB_UAC_CONTROL_STRING_IDX,

    .extra = &m_uac_control_function,
    .extralen = sizeof(m_uac_control_function)
};

// Streaming interface configuration.

static const struct usb_audio_stream_audio_endpoint_descriptor m_uac_cs_ep_dscs[] = {
    {
        .bLength = sizeof(struct usb_audio_stream_audio_endpoint_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_ENDPOINT,
        .bDescriptorSubtype = EP_GENERAL,
        .bmAttributes = 0,
        .bLockDelayUnits = 0,
        .wLockDelay = 0x0000
    }
};

static const struct usb_endpoint_descriptor m_uac_stream_endpoints[USB_AUDIO_EP_COUNT] = {
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = USB_AUDIO_EP_IN_ADDRESS,
        .bmAttributes = USB_ENDPOINT_ATTR_ISOCHRONOUS,
        .wMaxPacketSize = USB_AUDIO_EP_LENGTH,
        .bInterval = USB_AUDIO_EP_POLL_INTERVAL,

        .extra = m_uac_cs_ep_dscs,
        .extralen = sizeof(m_uac_cs_ep_dscs)
    }
};

static const struct {
    struct usb_audio_stream_interface_descriptor cs_stream_iface_dsc;
    struct usb_audio_format_type1_descriptor_1freq type1_format_dsc;
} __attribute__((packed)) m_uac_stream_function = {

    .cs_stream_iface_dsc = {
        .bLength = sizeof(struct usb_audio_stream_interface_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = AS_GENERAL,
        .bTerminalLink = USB_AUDIO_OUTPUT_TERMINAL_ID,
        .bDelay = 0,
        .wFormatTag = PCM
    },

    .type1_format_dsc = {
        .head = {
            .bLength = sizeof(struct usb_audio_format_type1_descriptor_1freq),
            .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
            .bDescriptorSubtype = FORMAT_TYPE,
            .bFormatType = FORMAT_TYPE_I,
            .bNrChannels = USB_AUDIO_CHANNELS_NUMBER,
            .bSubFrameSize = USB_AUDIO_SUB_FRAME_SIZE,
            .bBitResolution = USB_AUDIO_BIT_RESOLUTION,
            .bSamFreqType = USB_AUDIO_DISCRETE_FREQS_NUMBER // 1 discrete sampling frequency
        },
        .freqs = {
            {
                .tSamFreq = USB_AUDIO_SAMPLE_RATE
            }
        }
    }
};

const struct usb_interface_descriptor g_uac_iface_stream_dscs[] = {
    {
        .bLength = USB_DT_INTERFACE_SIZE,
        .bDescriptorType = USB_DT_INTERFACE,
        .bInterfaceNumber = USB_UAC_STREAM_INTERFACE_IDX,
        .bAlternateSetting = USB_AUDIO_STREAM_ALT_SETTING_PASSIVE,
        .bNumEndpoints = 0,
        .bInterfaceClass = USB_CLASS_AUDIO,
        .bInterfaceSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
        .bInterfaceProtocol = USB_AUDIO_PROTOCOL_NONE,
        .iInterface = USB_UAC_STREAM_STRING_IDX,

        .extra = NULL,
        .extralen = 0
    },
    {
        .bLength = USB_DT_INTERFACE_SIZE,
        .bDescriptorType = USB_DT_INTERFACE,
        .bInterfaceNumber = USB_UAC_STREAM_INTERFACE_IDX,
        .bAlternateSetting = USB_AUDIO_STREAM_ALT_SETTING_ACTIVE,
        .bNumEndpoints = USB_AUDIO_EP_COUNT,
        .bInterfaceClass = USB_CLASS_AUDIO,
        .bInterfaceSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
        .bInterfaceProtocol = USB_AUDIO_PROTOCOL_NONE,
        .iInterface = USB_UAC_STREAM_STRING_IDX,

        .endpoint = m_uac_stream_endpoints,
        .extra = &m_uac_stream_function,
        .extralen = sizeof(m_uac_stream_function)
    }
};

static uint8_t fwapp_uac_get_channel_muted(uint8_t ch_index)
{
    uint8_t muted = SET_MUTED;
    if (ch_index < USB_AUDIO_ALL_CHANNELS_NUMBER)
        muted = m_channels_cfg[ch_index].muted;
    printf("uac: get ch %u muted: %u\n", ch_index, muted);
    return muted;
}

static void fwapp_uac_set_channel_muted(uint8_t ch_index, uint8_t muted)
{
    printf("uac: set ch %u muted: %u\n", ch_index, muted);
    if (ch_index < USB_AUDIO_ALL_CHANNELS_NUMBER)
        m_channels_cfg[ch_index].muted = muted;
}

static void fwapp_uac_set_stream_status(enum uac_stream_status status)
{
    m_uac_stream_status = status;
    printf("uac: set stream status %u\n", m_uac_stream_status);

    if (m_uac_stream_status == UAC_STREAM_ENABLED) {
        m_sample_buf_index = UAC_SAMPLE_BUF0;
        for (uint8_t i = UAC_SAMPLE_BUF0; i < UAC_SAMPLE_BUFS_COUNT; ++i)
            memset(m_sample_buffers[i].samples, 0, sizeof(m_sample_buffers[i].samples));
    }
}

static enum usbd_request_return_codes fwapp_uac_handle_mute_selector(
    usbd_device *dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    (void)complete;
    (void)dev;

    const uint8_t ch_index = get_byte_lo(req->wValue);
    if ((ch_index == USB_AUDIO_MASTER_CHANNEL_IDX)
        && (req->wLength == MUTED_LENGTH)) {
        // Check for request type, get/set the mute for requested channel.
        switch (req->bRequest) {
        case GET_CUR: {
            uint8_t muted = fwapp_uac_get_channel_muted(ch_index);
            *buf = &muted;
            *len = sizeof(muted);
            return USBD_REQ_HANDLED;
        }
        case SET_CUR: {
            const uint8_t muted = *buf[0];
            fwapp_uac_set_channel_muted(ch_index, muted);
            return USBD_REQ_HANDLED;
        }
        default:
            break;
        }
    }

    printf("uac: unsupported setup:\n");
    fwapp_usb_dump_setup_req(req);
    return USBD_REQ_NOTSUPP;
}

static enum usbd_request_return_codes fwapp_uac_handle_feature_unit_request(
    usbd_device *dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    // Check for feature unit control selector.
    const uint8_t control = get_byte_hi(req->wValue);
    switch (control) {
    case MUTE_CONTROL:
        return fwapp_uac_handle_mute_selector(dev, req, buf, len, complete);
    default:
        break;
    }

    printf("uac: unsupported setup:\n");
    fwapp_usb_dump_setup_req(req);
    return USBD_REQ_NOTSUPP;
}

static enum usbd_request_return_codes fwapp_uac_control_interface_request_cb(
    usbd_device *dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    enum { EXPECTED_BM_REQ_TYPE = USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE };
    if ((req->bmRequestType & EXPECTED_BM_REQ_TYPE) == 0)
        return USBD_REQ_NOTSUPP;
    const uint8_t iface_num = get_byte_hi(req->wIndex);
    // Check for units ID's.
    switch (iface_num) {
    case USB_AUDIO_FEATURE_UNITL_ID:
        return fwapp_uac_handle_feature_unit_request(dev, req, buf, len, complete);
    default:
        break;
    }

    printf("uac: unsupported setup:\n");
    fwapp_usb_dump_setup_req(req);
    return USBD_REQ_NOTSUPP;
}

static enum usbd_request_return_codes fwapp_uac_control_endpoint_request_cb(
    usbd_device *dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    (void)buf;
    (void)complete;
    (void)dev;
    (void)len;
    (void)req;

    printf("uac: unsupported setup:\n");
    fwapp_usb_dump_setup_req(req);
    return USBD_REQ_NOTSUPP;
}

static void fwapp_uac_stream_cb(usbd_device *dev, uint8_t ep)
{
    (void)ep;

    const struct fwapp_uac_buffer *buf = &m_sample_buffers[m_sample_buf_index];

    usbd_ep_write_packet(dev, USB_AUDIO_EP_IN_ADDRESS, buf->samples,
                         sizeof(buf->samples));
}

void fwapp_uac_setup(usbd_device *dev)
{
    m_dev = dev;

    fwapp_usb_register_sof_callback(fwapp_uac_sof_cb);

    usbd_ep_setup(
        dev,
        USB_AUDIO_EP_IN_ADDRESS,
        USB_ENDPOINT_ATTR_ISOCHRONOUS,
        USB_AUDIO_EP_LENGTH,
        fwapp_uac_stream_cb);

    usbd_register_control_callback(
        dev,
        USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        fwapp_uac_control_interface_request_cb);

    usbd_register_control_callback(
        dev,
        USB_REQ_TYPE_CLASS | USB_REQ_TYPE_ENDPOINT,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        fwapp_uac_control_endpoint_request_cb);
}

void fwapp_uac_handle_set_altsetting(usbd_device *dev, uint16_t iface_idx, uint16_t alt_setting)
{
    (void)dev;

    if (iface_idx != USB_UAC_STREAM_INTERFACE_IDX)
        return;
    printf("uac: curr alt setting: %u\n", alt_setting);
    if (alt_setting == USB_AUDIO_STREAM_ALT_SETTING_PASSIVE)
        fwapp_uac_set_stream_status(UAC_STREAM_DISABLED);
    else if (alt_setting == USB_AUDIO_STREAM_ALT_SETTING_ACTIVE)
        fwapp_uac_set_stream_status(UAC_STREAM_ENABLED);
}

void fwapp_uac_register_set_buffer_callback(fwapp_uac_set_buffer_cb prep_buf_cb)
{
    m_set_buf_cb = prep_buf_cb;
}
