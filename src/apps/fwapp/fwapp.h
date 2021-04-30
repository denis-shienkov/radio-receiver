#ifndef FWAPP_CFG_H
#define FWAPP_CFG_H

//
// Common USB configuration.
//

// USB BCD version.
#define USB_BCD_2                       0x0200 // 2.0
// USB device classe.
#define USB_CLASS_MISC                  0xEF
// USB device sub-classe.
#define USB_SUBCLASS_COMMON             0x02
// USB device protocol.
#define USB_PROTOCOL_IAD                0x01
// USB device vendor.
#define USB_VENDOR_ID                   0xFFFF
// USB device product.
#define USB_PRODUCT_ID                  0xFFFF
// USB maximum ep0 size.
#define USB_MAX_EP0_SIZE                64
// USB maximum configurations number.
#define USB_CONFIGURATIONS_NUMBER       1
// USB number of strings.
#define USB_STRINGS_NUMBER              8
#define USB_MANUFACTURER_STRING_IDX     1
#define USB_PRODUCT_STRING_IDX          2
#define USB_SERIALNUM_STRING_IDX        3
#define USB_CONFIG_STRING_IDX           4
#define USB_HID_STRING_IDX              5
#define USB_UAC_ASSOC_STRING_IDX        6
#define USB_UAC_CONTROL_STRING_IDX      7
#define USB_UAC_STREAM_STRING_IDX       8
// USB number of all interfaces (HID + UAC1).
#define USB_INTERFACES_NUMBER           3
#define USB_UAC_INTERFACES_NUMBER       2
#define USB_UAC_STREAM_INTERFACES_NUMBER    1
#define USB_HID_INTERFACE_IDX           0
#define USB_UAC_CONTROL_INTERFACE_IDX   1
#define USB_UAC_STREAM_INTERFACE_IDX    2
// USB configuration value.
#define USB_CONFIGURATION_VALUE         1
// USB maximum power.
#define USB_MAX_POWER                   0x32
// USB control buffer length.
// This needs to be big enough to hold any descriptor,
// the largest of which will be the configuration descriptor.
#define USB_CONTROL_BUFFER_LENGTH       512

#define get_byte_hi(word)        (word >> 8)
#define get_byte_lo(word)        (word & 0xFF)

//
// Common USB HID confgiuration.
//

// USB HID BCD version.
#define USB_BCD_HID                     0x0100 // 1.0
// USB HID endpoint addresses.
#define USB_HID_EP_COUNT                2
#define USB_HID_EP_IN_ADDRESS           0x81
#define USB_HID_EP_OUT_ADDRESS          0x01
// USB HID endpoint buffer siz (should be more thatn HID report size).
#define USB_HID_EP_LENGTH               40 //
// USB HID endpoint polling interval.
#define USB_HID_EP_INTERVAL             0x01
// USB HID report payload size.
#define UDB_HID_REPORT_PAYLOAD_SIZE     32

//
// Common USB UAC confgiuration.
//

// USB ADC BCD version.
#define USB_BCD_ADC                     0x0100 // 1.0
// USB audio endpoint addresses.
#define USB_AUDIO_EP_COUNT              1
#define USB_AUDIO_EP_IN_ADDRESS         0x82
// USB audio endpoint buffer size.
#define USB_AUDIO_EP_LENGTH             64 // For 16 kHz sample rate.
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
#define USB_AUDIO_SAMPLE_RATE           16000
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
// USB audio output terminal types.
#define TERMINAL_STREAMING              0x0101
// USB audio input terminal types.
#define MICROPHONE                      0x0201 // I, generic microphone.
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
// USB audio class-specific endpoint descriptor subtypes.
#define EP_GENERAL                      0x01
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

#endif // FWAPP_CFG_H
