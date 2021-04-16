#include "fwapp_uac.h"
#include "fwapp_usb.h"

#include <libopencm3/usb/audio.h>

#include <stddef.h> // for NULL

// USB ADC BCD version.
#define USB_BCD_ADC                         0x0100 // 1.0
// USB audio endpoint addresses.
#define USB_AUDIO_EP_COUNT                  1
#define USB_AUDIO_EP_IN_ADDRESS             0x82
// USB audio endpoint buffer size.
#define USB_AUDIO_EP_LENGTH                 256
// USB audio endpoint polling interval.
#define USB_AUDIO_EP_POLL_INTERVAL          0x01
// USB audio interface protocol.
#define USB_AUDIO_INTERFACE_PROTOCOL_NONE   0
// USB audio terminal identifiers.
#define USB_AUDIO_INPUT_TERMINAL_ID         1
#define USB_AUDIO_OUTPUT_TERMINAL_ID        2
#define USB_AUDIO_FEATURE_UNITL_ID          3

#define USB_AUDIO_SAMPLE_RATE               8000

// USB terminal types.
#define TERMINAL_STREAMING                  0x0101 // Streaming.

// USB embedded function terminal types.
#define RADIO_RECEIVER                      0x0710 // I, AM/FM radio.

// Association interface configuration.

const struct usb_iface_assoc_descriptor g_uac_iface_assoc_dsc = {
    .bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
    .bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
    .bFirstInterface = USB_UAC_CONTROL_INTERFACE_IDX,
    .bInterfaceCount = USB_UAC_INTERFACES_NUMBER,
    .bFunctionClass = USB_CLASS_AUDIO,
    .bFunctionSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
    .bFunctionProtocol = USB_AUDIO_INTERFACE_PROTOCOL_NONE,
    .iFunction = USB_UAC_ASSOC_STRING_IDX
};

// Control interface configuration.

static const struct {
    struct usb_audio_header_descriptor_head header_head_dsc;
    struct usb_audio_header_descriptor_body header_body_dsc;
    struct usb_audio_input_terminal_descriptor input_terminal_dsc;
    struct usb_audio_output_terminal_descriptor output_terminal_dsc;
    struct usb_audio_feature_unit_descriptor_2ch feature_unit_dsc;
} __attribute__((packed)) m_uac_control_function = {

    .header_head_dsc = {
        .bLength = sizeof(struct usb_audio_header_descriptor_head)
                   + 1 * sizeof(struct usb_audio_header_descriptor_body),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = USB_AUDIO_TYPE_HEADER,
        .bcdADC = USB_BCD_ADC,
        .wTotalLength =
            sizeof(struct usb_audio_header_descriptor_head)
            + 1 * sizeof(struct usb_audio_header_descriptor_body)
            + sizeof(struct usb_audio_input_terminal_descriptor)
            + sizeof(struct usb_audio_feature_unit_descriptor_2ch)
            + sizeof(struct usb_audio_output_terminal_descriptor),
        .binCollection = 1
    },

    .header_body_dsc = {
        .baInterfaceNr = 0x01
    },

    .input_terminal_dsc = {
        .bLength = sizeof(struct usb_audio_input_terminal_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = USB_AUDIO_TYPE_INPUT_TERMINAL,
        .bTerminalID = USB_AUDIO_INPUT_TERMINAL_ID,
        .wTerminalType = RADIO_RECEIVER,
        .bAssocTerminal = 0,
        .bNrChannels = 2,
        .wChannelConfig = 0x0003, // Left & Right channels.
        .iChannelNames = 0,
        .iTerminal = 0
    },

    .output_terminal_dsc = {
        .bLength = sizeof(struct usb_audio_output_terminal_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
        .bDescriptorSubtype = USB_AUDIO_TYPE_OUTPUT_TERMINAL,
        .bTerminalID = USB_AUDIO_OUTPUT_TERMINAL_ID,
        .wTerminalType = TERMINAL_STREAMING,
        .bAssocTerminal = 0,
        .bSourceID = USB_AUDIO_FEATURE_UNITL_ID,
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
            .bmaControlMaster = 0x0001// 'Mute' is supported.
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
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL_NONE,
    .iInterface = USB_UAC_CONTROL_STRING_IDX,

    .extra = &m_uac_control_function,
    .extralen = sizeof(m_uac_control_function)
};

// Streaming interface configuration.

static const struct usb_audio_stream_audio_endpoint_descriptor m_uac_cs_ep_dscs[] = {
    {
        .bLength = sizeof(struct usb_audio_stream_audio_endpoint_descriptor),
        .bDescriptorType = USB_AUDIO_DT_CS_ENDPOINT,
        .bDescriptorSubtype = 1, // EP_GENERAL
        .bmAttributes = 0,
        .bLockDelayUnits = 0x02, // PCM samples.
        .wLockDelay = 0x0000
    }
};

static const struct usb_endpoint_descriptor m_uac_endpoints[USB_AUDIO_EP_COUNT] = {
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = USB_AUDIO_EP_IN_ADDRESS,
        .bmAttributes = USB_ENDPOINT_ATTR_ASYNC | USB_ENDPOINT_ATTR_ISOCHRONOUS,
        .wMaxPacketSize = USB_AUDIO_EP_LENGTH,
        .bInterval = USB_AUDIO_EP_POLL_INTERVAL,

        // not using usb_audio_stream_endpoint_descriptor??
        // (Why? These are USBv1.0 endpoint descriptors)

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
        .bDescriptorSubtype = 1, // AS_GENERAL
        .bTerminalLink = USB_AUDIO_OUTPUT_TERMINAL_ID,
        .bDelay = 0,
        .wFormatTag = 0x0001 // PCM Format
    },

    .type1_format_dsc = {
        .head = {
            .bLength = sizeof(struct usb_audio_format_type1_descriptor_1freq),
            .bDescriptorType = USB_AUDIO_DT_CS_INTERFACE,
            .bDescriptorSubtype = 2, // FORMAT_TYPE
            .bFormatType = 1, // FORMAT_TYPE 1
            .bNrChannels = 2,
            .bSubFrameSize = 2,
            .bBitResolution = 16, // Should be 10, but 16 is more reliable
            .bSamFreqType = 1 // 1 discrete sampling frequency
        },
        .freqs = {
            {
                .tSamFreq = USB_AUDIO_SAMPLE_RATE
            }
        }
    }
};

const struct usb_interface_descriptor g_uac_iface_stream_dsc = {
        .bLength = USB_DT_INTERFACE_SIZE,
        .bDescriptorType = USB_DT_INTERFACE,
        .bInterfaceNumber = USB_UAC_STREAMIMG_INTERFACE_IDX,
        .bAlternateSetting = 0,
        .bNumEndpoints = 0,
        .bInterfaceClass = USB_CLASS_AUDIO,
        .bInterfaceSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
        .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL_NONE,
        .iInterface = USB_UAC_STREAM_STRING_IDX,

        .extra = NULL,
        .extralen = 0

//    {
//        .bLength = USB_DT_INTERFACE_SIZE,
//        .bDescriptorType = USB_DT_INTERFACE,
//        .bInterfaceNumber = USB_UAC_STREAMIMG_INTERFACE_IDX,
//        .bAlternateSetting = 1,
//        .bNumEndpoints = 1,
//        .bInterfaceClass = USB_CLASS_AUDIO,
//        .bInterfaceSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
//        .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL_NONE,
//        .iInterface = USB_UAC_STREAM_STRING_IDX,

//        .endpoint = m_uac_endpoints,

//        .extra = &m_uac_stream_function,
//        .extralen = sizeof(m_uac_stream_function)
//    }
};

static enum usbd_request_return_codes fwapp_uac_control_request_cb(
    usbd_device *dev,
    struct usb_setup_data *req,
    uint8_t **buf,
    uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    (void)buf;
    (void)complete;
    (void)len;
    (void)dev;

    enum { EXPECTED_BM_REQ_TYPE = USB_REQ_TYPE_VENDOR };
    if (req->bmRequestType != EXPECTED_BM_REQ_TYPE)
        return USBD_REQ_NOTSUPP; // Only accept vendor request.
    return USBD_REQ_HANDLED;
}

void fwapp_uac_setup(usbd_device *dev)
{
    usbd_ep_setup(
        dev,
        USB_AUDIO_EP_IN_ADDRESS,
        USB_ENDPOINT_ATTR_ISOCHRONOUS,
        32, // WAVEFORM_SAMPLES*2
        NULL); // iso stream handler

    usbd_register_control_callback(
        dev,
        USB_REQ_TYPE_VENDOR,
        USB_REQ_TYPE_TYPE,
        fwapp_uac_control_request_cb);
}
