#include "fwapp_hid.h"
#include "fwapp_proto.h"

#include <stdio.h> // for printf

static uint8_t payload[128] = {0};
static uint16_t length = 0;

static void fwapp_dump_report(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

static void fwapp_proto_recv_cb(void)
{
    length = fwapp_hid_recv_report(payload, sizeof(payload));
    printf("pro: recv %u\n", length);
    fwapp_dump_report(payload, length);

    length = fwapp_hid_send_report(payload, length);
}

static void fwapp_proto_send_cb(void)
{
    printf("pro: send %u\n", length);
    fwapp_dump_report(payload, length);
}

void fwapp_proto_start(void)
{
    fwapp_hid_register_recv_report_callback(fwapp_proto_recv_cb);
    fwapp_hid_register_send_report_callback(fwapp_proto_send_cb);
}

void fwapp_proto_stop(void)
{

}

void fwapp_proto_schedule(void)
{
    // Do something.
}
