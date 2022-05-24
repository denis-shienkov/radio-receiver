#include "fwapp_led.h"
#include "fwapp_systick.h"

#include <libopencm3/stm32/gpio.h>

#define LED_PORT    GPIOC
#define LED_PIN     GPIO13

static volatile uint32_t m_counter = 0;
static uint32_t m_freq_hz = 1;

static void fwapp_led_reload_counter(void)
{
    const uint32_t systic_freq = fwapp_systick_get_freq_hz();
    m_counter = systic_freq / m_freq_hz / 2;
}

static void fwapp_led_decrement_counter_cb(uint32_t freq_hz)
{
    (void)freq_hz;
    if (m_counter > 0)
        --m_counter;
}

void fwapp_led_start(uint32_t freq_hz)
{
    m_freq_hz = freq_hz;
    fwapp_led_reload_counter();

    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);

    fwapp_systick_register_callback(fwapp_led_decrement_counter_cb);
}

void fwapp_led_stop(void)
{
    fwapp_systick_unregister_callback(fwapp_led_decrement_counter_cb);
}

void fwapp_led_schedule(void)
{
    if (m_counter > 0) {
        // Do nothing.
    } else {
        gpio_toggle(LED_PORT, LED_PIN);
        fwapp_led_reload_counter();
    }
}
