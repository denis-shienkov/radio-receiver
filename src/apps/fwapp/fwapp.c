#include "fwapp_led.h"
#include "fwapp_systick.h"
#include "fwapp_usb_composite.h"

#include <libopencm3/stm32/rcc.h>

int main(void)
{
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_48MHZ]);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);

    fwapp_led_start(5);
    fwapp_systick_start(1000);
    fwapp_usb_composite_start();

    while (1) {
        fwapp_led_schedule();
        fwapp_usb_composite_schedule();
    }

    return 0;
}
