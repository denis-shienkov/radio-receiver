#include "fwapp_cbuf.h"

#include <stddef.h> // for NULL

#define CBUF_COUNT      1

static struct fwapp_cbuf {
    uint8_t *data;
    uint32_t len;
    uint32_t head;
    uint32_t tail;
    bool is_full;
} m_cbufs[CBUF_COUNT] = {
    {NULL, 0, 0, 0, false}
};

static void fwapp_cbuf_clear(int cbuf_index)
{
    struct fwapp_cbuf *cbuf = &m_cbufs[cbuf_index];
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->is_full =0;
}

static void fwapp_cbuf_advance(struct fwapp_cbuf *cbuf)
{
    if (cbuf->is_full)
        cbuf->tail = (cbuf->tail + 1) % cbuf->len;
    cbuf->head = (cbuf->head + 1) % cbuf->len;
    cbuf->is_full = (cbuf->head == cbuf->tail);
}

static void fwapp_cbuf_retreat(struct fwapp_cbuf *cbuf)
{
    cbuf->is_full = false;
    cbuf->tail = (cbuf->tail + 1) % cbuf->len;
}

int fwapp_cbuf_open(uint8_t *data, uint32_t len)
{
    if (data && (len > 0)) {
        for (int cbuf_index = 0; cbuf_index < CBUF_COUNT; ++cbuf_index) {
            struct fwapp_cbuf *cbuf = &m_cbufs[cbuf_index];
            if (cbuf->data)
                continue;
            cbuf->data = data;
            cbuf->len = len;
            return cbuf_index;
        }
    }
    return -1;
}

void fwapp_cbuf_close(int cbuf_index)
{
    if ((cbuf_index >= 0) && (cbuf_index < CBUF_COUNT)) {
        struct fwapp_cbuf *cbuf = &m_cbufs[cbuf_index];
        cbuf->data = NULL;
        cbuf->len = 0;
        fwapp_cbuf_clear(cbuf_index);
    }
}

bool fwapp_cbuf_put(int cbuf_index, uint8_t c)
{
    if ((cbuf_index >= 0) && (cbuf_index < CBUF_COUNT)) {
        struct fwapp_cbuf *cbuf = &m_cbufs[cbuf_index];
        if (cbuf->is_full)
            return false; // Buffer is full.
        cbuf->data[cbuf->head] = c;
        fwapp_cbuf_advance(cbuf);
        return true;
    }
    return false;
}

bool fwapp_cbuf_get(int cbuf_index, uint8_t *c)
{
    if ((cbuf_index >= 0) && (cbuf_index < CBUF_COUNT)) {
        struct fwapp_cbuf *cbuf = &m_cbufs[cbuf_index];
        if (!cbuf->is_full && (cbuf->head == cbuf->tail))
            return false; // Buffer is empty.
        *c = cbuf->data[cbuf->tail];
        fwapp_cbuf_retreat(cbuf);
        return true;
    }
    return false;
}
