#include "fwapp_cbuf.h"
#include "fwapp_trace.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

#include <libopencm3/cm3/nvic.h>

#include <errno.h> // for errno

#define TRACE_DATA_SIZE       512

static uint8_t m_data[TRACE_DATA_SIZE] = {0};
static int m_cbuf_index = -1;

void usart1_isr(void)
{
    // Check if we were called because of txe.
    if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0)
        && ((USART_SR(USART1) & USART_SR_TXE) != 0)) {

        usart_disable_tx_interrupt(USART1);
        uint8_t c = 0;
        if (fwapp_cbuf_get(m_cbuf_index, &c)) {
            usart_send(USART1, c);
            usart_enable_tx_interrupt(USART1);
        }
    }
}

extern int _write(int file, char *ptr, int len);
int _write(int file, char *ptr, int len)
{
    usart_disable_tx_interrupt(USART1);

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

    usart_enable_tx_interrupt(USART1);
    return rc;
}

void fwapp_trace_start(uint32_t baud_rate)
{
    m_cbuf_index = fwapp_cbuf_open(m_data, TRACE_DATA_SIZE);

    // Enable usart interrupts.
    nvic_enable_irq(NVIC_USART1_IRQ);
    // Configure usart tx pin.
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
    // Setup usrt parameters.
    usart_set_baudrate(USART1, baud_rate);
    usart_set_databits(USART1, 8);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART1, USART_MODE_TX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    // Enable usart.
    usart_enable(USART1);
}

void fwapp_trace_stop(void)
{
    usart_disable(USART1);
    nvic_disable_irq(NVIC_USART1_IRQ);
    fwapp_cbuf_close(m_cbuf_index);
}
