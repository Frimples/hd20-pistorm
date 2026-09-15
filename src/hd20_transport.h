#ifndef HD20_TRANSPORT_H
#define HD20_TRANSPORT_H
#include <stddef.h>
#include <stdint.h>
#include "hd20_protocol.h"
struct vsony_host;
enum hd20_phase { HD20_P0=0,HD20_P1=1,HD20_P2=2,HD20_P3=3,HD20_P4=4,HD20_P5=5,HD20_P6=6,HD20_P7=7 };
struct hd20_transport;
struct hd20_transport *hd20_transport_create(struct vsony_host *disk);
void hd20_transport_destroy(struct hd20_transport *);
void hd20_transport_reset(struct hd20_transport *);
void hd20_transport_phase(struct hd20_transport *,enum hd20_phase);
int hd20_transport_host_byte(struct hd20_transport *,uint8_t);
int hd20_transport_device_ready(const struct hd20_transport *);
int hd20_transport_device_byte(struct hd20_transport *,uint8_t *);
#endif
