#include "fwapp.h"
#include "fwapp_adc.h"
#include "fwapp_uac.h"

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include <libopencm3/cm3/nvic.h>

#include <stddef.h> // for NULL
#include <string.h> // for memcpy

#define ADC_PORT        GPIOA
#define ADC_PIN_LIN     GPIO0
#define ADC_PIN_RIN     GPIO1

#define TEST_PORT       GPIOA
#define TEST_PIN_SOF    GPIO6
#define TEST_PIN_READY  GPIO7

extern const struct rcc_clock_scale g_fwapp_rcc_hse_config;

static volatile bool m_data_ready = false;
static uint32_t m_period = 0;
static struct fwapp_uac_buffer *m_uac_buf = NULL;
static uint8_t m_uac_sample_pos = 0;

static void fwapp_adc_restart_timer(void)
{
    timer_disable_counter(TIM2);
    timer_set_counter(TIM2, m_period);
    timer_enable_counter(TIM2);
}

static void fwapp_adc_set_uac_buffer_cb(struct fwapp_uac_buffer *uac_buf)
{
    m_uac_buf = uac_buf;
    m_uac_sample_pos = 0;

    fwapp_adc_restart_timer();
    gpio_toggle(TEST_PORT, TEST_PIN_SOF);
}

static void fwapp_adc_add_sample(void)
{
    if (!m_uac_buf)
        return;

    if (m_uac_sample_pos < (sizeof(m_uac_buf->samples) / USB_AUDIO_SUB_FRAME_SIZE)) {
        gpio_toggle(TEST_PORT, TEST_PIN_READY);

        uint16_t left = adc_read_injected(ADC1, 1);
        uint16_t right = adc_read_injected(ADC1, 2);

        uint8_t *out_left = m_uac_buf->samples
                            + (m_uac_sample_pos
                               * USB_AUDIO_SUB_FRAME_SIZE
                               * USB_AUDIO_CHANNELS_NUMBER);

        uint8_t *out_right = out_left + USB_AUDIO_SUB_FRAME_SIZE;

        memcpy(out_left, &left, sizeof(left));
        memcpy(out_right, &right, sizeof(right));

        ++m_uac_sample_pos;
    } else {
        timer_disable_counter(TIM2);
    }
}

void adc1_2_isr(void)
{
    adc_clear_flag(ADC1, ADC_SR_JEOC);
    m_data_ready = true;
}

void fwapp_adc_start(uint32_t freq_hz)
{
    // Configure test pin.
    gpio_set_mode(TEST_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, TEST_PIN_SOF);
    gpio_set_mode(TEST_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, TEST_PIN_READY);

    fwapp_uac_register_set_buffer_callback(fwapp_adc_set_uac_buffer_cb);

    // Configure timer.

    // TIM2 on APB1 is running at double frequency (it is 72 MHz).
    const uint32_t timer_clock = g_fwapp_rcc_hse_config.apb1_frequency * 2;
    const uint32_t divider = timer_clock / freq_hz;
    const uint32_t prescaler = divider / 10;
    m_period = (divider / prescaler) - 1;
    rcc_periph_reset_pulse(RST_TIM2);
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_disable_counter(TIM2);
    timer_set_prescaler(TIM2, prescaler);
    timer_set_period(TIM2, m_period);
    timer_set_master_mode(TIM2, TIM_CR2_MMS_UPDATE);

    // Configure ADC.

    gpio_set_mode(ADC_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, ADC_PIN_LIN | ADC_PIN_RIN);

    nvic_set_priority(NVIC_ADC1_2_IRQ, 0);
    nvic_enable_irq(NVIC_ADC1_2_IRQ);

    adc_power_off(ADC1);
    adc_enable_scan_mode(ADC1);
    adc_set_single_conversion_mode(ADC1);
    adc_enable_external_trigger_injected(ADC1, ADC_CR2_JEXTSEL_TIM2_TRGO);
    adc_enable_eoc_interrupt_injected(ADC1);
    adc_set_right_aligned(ADC1);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_28DOT5CYC);

    enum { LEFT_CHANNEL = 0, RIGHT_CHANNEL = 1, CHANNEL_COUNT = 2 };
    uint8_t channel_array[CHANNEL_COUNT] = {LEFT_CHANNEL, RIGHT_CHANNEL};
    adc_set_injected_sequence(ADC1, CHANNEL_COUNT, channel_array);

    adc_power_on(ADC1);

    // Wait for ADC starting up.
    fwapp_delay_cycles(800000);

    adc_reset_calibration(ADC1);
    adc_calibrate(ADC1);
}

void fwapp_adc_stop(void)
{
    timer_disable_counter(TIM2);
}

void fwapp_adc_schedule(void)
{
    if (m_data_ready) {
        m_data_ready = false;
        fwapp_adc_add_sample();
    }
}
