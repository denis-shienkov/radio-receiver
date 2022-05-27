#include "fwapp.h"
//#include "fwapp_adc.h"
#include "fwapp_led.h"
//#include "fwapp_proto.h"
#include "fwapp_systick.h"
#include "fwapp_trace.h"
#include "fwapp_usb.h"

#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/rcc.h>

#include <stdio.h>

// A copy of the 84MHz cofiguration with the USB support.
const struct rcc_clock_scale g_fwapp_rcc_hse_config = {
    .pllm = 25,
    .plln = 336,
    .pllp = 4,
    .pllq = 7,
    .pllr = 0,
    .pll_source = RCC_CFGR_PLLSRC_HSE_CLK,
    .hpre = RCC_CFGR_HPRE_NODIV,
    .ppre1 = RCC_CFGR_PPRE_DIV2,
    .ppre2 = RCC_CFGR_PPRE_NODIV,
    .voltage_scale = PWR_SCALE1,
    .flash_config = FLASH_ACR_DCEN | FLASH_ACR_ICEN |
            FLASH_ACR_LATENCY_2WS,
    .ahb_frequency  = 84000000,
    .apb1_frequency = 42000000,
    .apb2_frequency = 84000000,
};

void fwapp_delay_cycles(uint32_t cycles_count)
{
    for (uint32_t i = 0; i < cycles_count; ++i) {
        __asm__("nop");
    }
}

int main(void)
{
    rcc_clock_setup_pll(&g_fwapp_rcc_hse_config);
    rcc_periph_clock_enable(RCC_GPIOA); // usb + usart
    rcc_periph_clock_enable(RCC_GPIOC); // led
    rcc_periph_clock_enable(RCC_USART2);
//    rcc_periph_clock_enable(RCC_TIM2);
//    rcc_periph_clock_enable(RCC_ADC1);

    fwapp_trace_start(115200);
    fwapp_led_start(1);
    fwapp_systick_start(1000);
    fwapp_usb_start();
//    fwapp_proto_start();
//    fwapp_adc_start();

    printf("Hello\n");

    while (1) {
        fwapp_led_schedule();
        fwapp_usb_schedule();
//        fwapp_hid_schedule();
//        fwapp_proto_schedule();
//        fwapp_adc_schedule();
    }

    return 0;
}
