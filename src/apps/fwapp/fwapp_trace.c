#include "fwapp_cbuf.h"
#include "fwapp_trace.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include <libopencm3/cm3/nvic.h>

#include <errno.h> // for errno

// We use USART2 TX for traces (pin PA2).
#define TRACE_TX            GPIO2
#define TRACE_DATA_SIZE     512

static uint8_t m_data[TRACE_DATA_SIZE] = {0};
static int m_cbuf_index = -1;

void usart2_isr(void)
{
    // Check if we were called because of txe.
    if (((USART_CR1(USART2) & USART_CR1_TXEIE) != 0)
        && ((USART_SR(USART2) & USART_SR_TXE) != 0)) {

        usart_disable_tx_interrupt(USART2);
        uint8_t c = 0;
        if (fwapp_cbuf_get(m_cbuf_index, &c)) {
            usart_send(USART2, c);
            usart_enable_tx_interrupt(USART2);
        }
    }
}

extern int _write(int file, char *ptr, int len);
int _write(int file, char *ptr, int len)
{
    usart_disable_tx_interrupt(USART2);

    int rc = -1;
    if (file == 1) {
        rc = 0;
        for (int i = 0; i < len; ++i) {
            const uint8_t c = ptr[i];
            if (fwapp_cbuf_put(m_cbuf_index, c))
                rc += 1;
            else
                break;
        }
    } else {
        errno = EIO;
    }

    usart_enable_tx_interrupt(USART2);
    return rc;
}

void fwapp_trace_start(uint32_t baud_rate)
{
    m_cbuf_index = fwapp_cbuf_open(m_data, TRACE_DATA_SIZE);

    // Enable usart interrupts.
    nvic_enable_irq(NVIC_USART2_IRQ);
    // Configure usart tx pin.
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, TRACE_TX);
    gpio_set_af(GPIOA, GPIO_AF7, TRACE_TX);
    // Setup usrt parameters.
    usart_set_baudrate(USART2, baud_rate);
    usart_set_databits(USART2, 8);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART2, USART_MODE_TX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    // Enable usart.
    usart_enable(USART2);
}

void fwapp_trace_stop(void)
{
    usart_disable(USART2);
    nvic_disable_irq(NVIC_USART2_IRQ);
    fwapp_cbuf_close(m_cbuf_index);
}
