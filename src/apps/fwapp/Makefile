
PROJECT = fwapp
BUILD_DIR = bin

CFILES = fwapp_adc.c fwapp.c fwapp_cbuf.c fwapp_hid.c fwapp_led.c fwapp_proto.c fwapp_systick.c fwapp_trace.c fwapp_uac.c fwapp_usb.c

DEVICE = stm32f103c8
OOCD_FILE = board/stm32f103c8_blue_pill.cfg

VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR = ../../shared/libopencm3/content

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ./rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
