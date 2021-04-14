#include "fwapp_systick.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <stddef.h> // for NULL

enum {
    SYSTICK_CLOCK = 6000000,
    SYSTICK_DEFAULT_FREQ_HZ = 1000,
    SYSTICK_HANDLERS_COUNT = 8,
};

static uint32_t m_freq_hz = SYSTICK_DEFAULT_FREQ_HZ;
static fwapp_systick_cb m_callbacks[SYSTICK_HANDLERS_COUNT] = {0};

void sys_tick_handler(void)
{
    for (uint32_t i = 0; i < SYSTICK_HANDLERS_COUNT; ++i) {
        if (m_callbacks[i])
            m_callbacks[i](m_freq_hz);
    }
}

bool fwapp_systick_register_callback(fwapp_systick_cb systick_cb)
{
    for (uint32_t i = 0; i < SYSTICK_HANDLERS_COUNT; ++i) {
        if (!m_callbacks[i]) {
            m_callbacks[i] = systick_cb;
            return true;
        }
    }
    return false;
}

bool fwapp_systick_unregister_callback(fwapp_systick_cb systick_cb)
{
    for (uint32_t i = 0; i < SYSTICK_HANDLERS_COUNT; ++i) {
        if (m_callbacks[i] && (m_callbacks[i] == systick_cb)) {
            m_callbacks[i] = NULL;
            return true;
        }
    }
    return false;
}

uint32_t fwapp_systick_get_freq_hz(void)
{
    return m_freq_hz;
}

void fwapp_systick_start(uint32_t freq_hz)
{
    m_freq_hz = freq_hz;

    // 48MHz / 8 => 6000000 counts per second.

    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    // SYSTICK_CLOCK/reload_value = freq_hz overflows per second - every 1/freq_hz
    // second one interrupt.
    // SysTick interrupt every N clock pulses: set reload to N-1.

    const uint32_t reload_value = (SYSTICK_CLOCK / (float)m_freq_hz) - 1;
    systick_set_reload(reload_value);

    systick_interrupt_enable();
    systick_counter_enable();
}

void fwapp_systick_stop(void)
{
    systick_counter_disable();
    systick_interrupt_disable();
}
