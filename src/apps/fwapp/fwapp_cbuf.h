#ifndef FWAPP_CBUF_H
#define FWAPP_CBUF_H

#include <stdbool.h>
#include <stdint.h>

int fwapp_cbuf_open(uint8_t *data, uint32_t len);
void fwapp_cbuf_close(int cbuf_index);
bool fwapp_cbuf_put(int cbuf_index, uint8_t c);
bool fwapp_cbuf_get(int cbuf_index, uint8_t *c);

#endif // FWAPP_CBUF_H
