#include "fwapp_hid.h"
#include "fwapp_led.h"
#include "fwapp_proto.h"
#include "fwapp_systick.h"
#include "fwapp_trace.h"
#include "fwapp_usb.h"

#include <libopencm3/stm32/rcc.h>

#include <stdio.h>

int main(void)
{
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_48MHZ]);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_USART1);

    fwapp_trace_start(115200);
    fwapp_led_start(5);
    fwapp_systick_start(1000);
    fwapp_usb_start();
    fwapp_proto_start();

    printf("Hello\n");

    while (1) {
        fwapp_led_schedule();
        fwapp_usb_schedule();
        fwapp_hid_schedule();
        fwapp_proto_schedule();
    }

    return 0;
}
