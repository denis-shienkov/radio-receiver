#ifndef FWAPP_ADC_H
#define FWAPP_ADC_H

#include <stdbool.h>
#include <stdint.h>

void fwapp_adc_start(uint32_t freq_hz);
void fwapp_adc_stop(void);
void fwapp_adc_schedule(void);

#endif // FWAPP_ADC_H
