#ifndef FWAPP_TRACE_H
#define FWAPP_TRACE_H

#include <stdbool.h>
#include <stdint.h>

void fwapp_trace_start(uint32_t baud_rate);
void fwapp_trace_stop(void);

#endif // FWAPP_TRACE_H
