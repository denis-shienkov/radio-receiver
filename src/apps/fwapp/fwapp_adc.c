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

extern const struct rcc_clock_scale g_fwapp_rcc_hse_config;

#define ADC_MEASUREMENTS_FOR_SAMPLE 4

static volatile bool m_data_ready = false;
static uint32_t m_period = 0;
static struct fwapp_uac_buffer *m_samples_buf = NULL;
static uint8_t m_sample_pos = 0;

static uint16_t m_left_meass[ADC_MEASUREMENTS_FOR_SAMPLE] = {0};
static uint16_t m_right_meass[ADC_MEASUREMENTS_FOR_SAMPLE] = {0};
static uint8_t m_meas_pos = 0;

static void fwapp_adc_restart_timer(void)
{
    //timer_disable_counter(TIM2);
    timer_set_counter(TIM2, m_period);
    timer_enable_counter(TIM2);
}

static void fwapp_adc_set_uac_buffer_cb(struct fwapp_uac_buffer *uac_buf)
{
    m_samples_buf = uac_buf;
    m_sample_pos = 0;

    fwapp_adc_restart_timer();
}

static uint16_t fwapp_adc_calc_average_sample(const uint16_t *meass)
{
    uint32_t average = 0;
    for (uint8_t i = 0; i < ADC_MEASUREMENTS_FOR_SAMPLE; ++i)
        average += meass[i];
    return average / ADC_MEASUREMENTS_FOR_SAMPLE;
}

static void fwapp_adc_add_average_sample(void)
{
    if (!m_samples_buf)
        return;

    const uint32_t channel_samples = (sizeof(m_samples_buf->samples) / sizeof(m_samples_buf->samples[0])) / USB_AUDIO_CHANNELS_NUMBER;
    if (m_sample_pos < channel_samples) {
        const uint16_t left = fwapp_adc_calc_average_sample(m_left_meass);
        const uint16_t right = fwapp_adc_calc_average_sample(m_right_meass);
        m_samples_buf->samples[m_sample_pos * USB_AUDIO_CHANNELS_NUMBER] = left;
        m_samples_buf->samples[m_sample_pos * USB_AUDIO_CHANNELS_NUMBER + 1] = right;
        ++m_sample_pos;
    } else {
        timer_disable_counter(TIM2);
    }
}

static void fwapp_adc_add_measurement(void)
{
    if (m_meas_pos < ADC_MEASUREMENTS_FOR_SAMPLE) {
        m_left_meass[m_meas_pos] = adc_read_injected(ADC1, 1);
        m_right_meass[m_meas_pos] = adc_read_injected(ADC1, 2);
        ++m_meas_pos;
    }
    if (m_meas_pos < ADC_MEASUREMENTS_FOR_SAMPLE) {
        // Do nothing.
    } else {
        m_meas_pos = 0;
        fwapp_adc_add_average_sample();
    }
}

void adc1_2_isr(void)
{
    adc_clear_flag(ADC1, ADC_SR_JEOC);
    m_data_ready = true;
}

void fwapp_adc_start(void)
{
    fwapp_uac_register_set_buffer_callback(fwapp_adc_set_uac_buffer_cb);
    m_sample_pos = 0;
    m_meas_pos = 0;

    // Configure timer.

    const uint32_t sampling_rate = ADC_MEASUREMENTS_FOR_SAMPLE * USB_AUDIO_SAMPLE_RATE;

    // TIM2 on APB1 is running at double frequency (it is 72 MHz).
    const uint32_t timer_clock = g_fwapp_rcc_hse_config.apb1_frequency * 2;
    const uint32_t divider = timer_clock / sampling_rate;
    const uint32_t prescaler = divider / 5;
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
        fwapp_adc_add_measurement();
    }
}
