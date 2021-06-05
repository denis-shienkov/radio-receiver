#include "fwapp.h"
#include "fwapp_adc.h"
#include "fwapp_hid.h"
#include "fwapp_led.h"
#include "fwapp_proto.h"
#include "fwapp_systick.h"
#include "fwapp_trace.h"
#include "fwapp_usb.h"

#include <libopencm3/stm32/rcc.h>

#include <stdio.h>

// A copy of the RCC_CLOCK_HSE8_72MHZ cofiguration
// with the USB support.
const struct rcc_clock_scale g_fwapp_rcc_hse_config = {
    .pll_mul = RCC_CFGR_PLLMUL_PLL_CLK_MUL9,
    .pll_source = RCC_CFGR_PLLSRC_HSE_CLK,
    .hpre = RCC_CFGR_HPRE_NODIV,
    .ppre1 = RCC_CFGR_PPRE_DIV2,
    .ppre2 = RCC_CFGR_PPRE_NODIV,
    .adcpre = RCC_CFGR_ADCPRE_DIV8,
    .flash_waitstates = 2,
    .prediv1 = RCC_CFGR2_PREDIV_NODIV,
    .ahb_frequency = 72e6,
    .apb1_frequency = 36e6,
    .apb2_frequency = 72e6,
    .usbpre = RCC_CFGR_USBPRE_PLL_CLK_DIV1_5
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
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_USART2);
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_ADC1);

    fwapp_trace_start(115200);
    fwapp_led_start(1);
    fwapp_systick_start(1000);
    fwapp_usb_start();
    fwapp_proto_start();
    fwapp_adc_start();

    printf("Hello\n");

    while (1) {
        fwapp_led_schedule();
        fwapp_usb_schedule();
        fwapp_hid_schedule();
        fwapp_proto_schedule();
        fwapp_adc_schedule();
    }

    return 0;
}
