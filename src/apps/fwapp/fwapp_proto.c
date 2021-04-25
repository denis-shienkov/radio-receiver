#include "fwapp_hid.h"
#include "fwapp_proto.h"

#include <stdio.h> // for printf

static void fwapp_dump_report(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

static void fwapp_proto_recv_cb(void)
{
    uint8_t report[128] = {0};
    const uint16_t bytes_read = fwapp_hid_recv_report(report, sizeof(report));
    printf("pro: recv %u\n", bytes_read);
    fwapp_dump_report(report, bytes_read);
}

static void fwapp_proto_send_cb(void)
{

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
