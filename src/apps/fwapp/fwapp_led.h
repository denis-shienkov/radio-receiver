#ifndef FWAPP_LED_H
#define FWAPP_LED_H

#include <stdbool.h>
#include <stdint.h>

void fwapp_led_start(uint32_t freq_hz);
void fwapp_led_stop(void);
void fwapp_led_schedule(void);

#endif // FWAPP_LED_H
