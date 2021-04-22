#ifndef FWAPP_UAC_H
#define FWAPP_UAC_H

#include <libopencm3/usb/usbd.h>

// USB audio configuration.

// USB ADC BCD version.
#define USB_BCD_ADC                     0x0100 // 1.0
// USB audio endpoint addresses.
#define USB_AUDIO_EP_COUNT              1
#define USB_AUDIO_EP_IN_ADDRESS         0x82
// USB audio endpoint buffer size.
#define USB_AUDIO_EP_LENGTH             256
// USB audio endpoint polling interval.
#define USB_AUDIO_EP_POLL_INTERVAL      0x01
// USB audio units identifiers.
#define USB_AUDIO_NONE_TERMINAL_ID      0
#define USB_AUDIO_INPUT_TERMINAL_ID     1
#define USB_AUDIO_FEATURE_UNITL_ID      2
#define USB_AUDIO_OUTPUT_TERMINAL_ID    3
// USB audio channels number.
#define USB_AUDIO_CHANNELS_NUMBER       2
#define USB_AUDIO_MASTER_CHANNEL_IDX    0
#define USB_AUDIO_LEFT_CHANNEL_IDX      1
#define USB_AUDIO_RIGHT_CHANNEL_IDX     2
// USB audio sample rate, in Hz.
#define USB_AUDIO_SAMPLE_RATE           8000
// USB audio sub-frame size (two bytes per audio subframe).
#define USB_AUDIO_SUB_FRAME_SIZE        2
// USB audio bit resolution (16-bits per sample).
#define USB_AUDIO_BIT_RESOLUTION        16
// USB audio number of discrete sample rates (one 8kHz).
#define USB_AUDIO_DISCRETE_FREQS_NUMBER 1
// USB audio stream alternate settings.
#define USB_AUDIO_STREAM_ALT_SETTING_PASSIVE    0
#define USB_AUDIO_STREAM_ALT_SETTING_ACTIVE     1

// USB audio class specification defines.

// USB audio interface protocol.
#define USB_AUDIO_PROTOCOL_NONE         0
// USB audio terminal types.
#define TERMINAL_STREAMING              0x0101
// USB audio embedded function terminal types.
#define RADIO_RECEIVER                  0x0710 // I, AM/FM radio.
// USB audio channels spatial locations.
#define bmLEFT_FRONT                    0x0001
#define bmRIGHT_FRONT                   0x0002
// USB audio feature channel controls.
#define bmMUTE                          0x0001
#define bmVOLUME                        0x0002
// USB audio class-specific subtypes.
#define AS_GENERAL                      0x01
#define FORMAT_TYPE                     0x02
// USB audio data format type-1 codes
#define PCM                             0x0001
// USB audio format type codes.
#define FORMAT_TYPE_I                   0x01
// USB audio feature unit control selectors.
// (high byte of wValue).
#define MUTE_CONTROL                    0x01
// USB audio request codes (in bRequest).
#define SET_CUR                         0x01
#define GET_CUR                         0x81
// USB audio muted value.
#define SET_MUTED                       1
#define MUTED_LENGTH                    1

extern const struct usb_iface_assoc_descriptor g_uac_iface_assoc_dsc;
extern const struct usb_interface_descriptor g_uac_iface_control_dsc;
extern const struct usb_interface_descriptor g_uac_iface_stream_dscs[];
extern uint8_t g_uac_stream_iface_cur_altsetting;

void fwapp_uac_setup(usbd_device *dev);
void fwapp_uac_handle_set_altsetting(usbd_device *dev, uint16_t iface_idx, uint16_t alt_setting);


#endif // FWAPP_UAC_H
