#include "fwapp_systick.h"

#include <libopencm3/stm32/rcc.h>

int main(void)
{
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_48MHZ]);

    fwapp_systick_start(1000);

    while (1) {

    }

    return 0;
}
