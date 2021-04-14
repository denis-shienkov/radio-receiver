#ifndef FWAPP_SYSTICK_H
#define FWAPP_SYSTICK_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*fwapp_systick_cb)(uint32_t freq_hz);

bool fwapp_systick_register_callback(fwapp_systick_cb systick_cb);
bool fwapp_systick_unregister_callback(fwapp_systick_cb systick_cb);
uint32_t fwapp_systick_get_freq_hz(void);
void fwapp_systick_start(uint32_t freq_hz);
void fwapp_systick_stop(void);

#endif // FWAPP_SYSTICK_H
