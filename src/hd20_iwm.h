#ifndef HD20_IWM_H
#define HD20_IWM_H
#include <stdint.h>

struct vsony_host;
struct hd20_iwm;

enum hd20_iwm_state {
    HD20_IWM_IDLE = 0,
    HD20_IWM_PRESENCE = 1,
    HD20_IWM_COMMAND_RECEIVE = 2,
    HD20_IWM_RESPONSE = 3
};

#define HD20_IWM_PASS_THROUGH (-1)

struct hd20_iwm *hd20_iwm_create(struct vsony_host *disk);
void hd20_iwm_destroy(struct hd20_iwm *);
int hd20_iwm_owns(uint32_t addr);
uint32_t hd20_iwm_read(struct hd20_iwm *, uint32_t addr, uint8_t type);
int hd20_iwm_selected(const struct hd20_iwm *);
int hd20_iwm_active(const struct hd20_iwm *);
int hd20_iwm_device_select(const struct hd20_iwm *);
int hd20_iwm_presence(const struct hd20_iwm *);
int hd20_iwm_ready(const struct hd20_iwm *);
enum hd20_iwm_state hd20_iwm_state(const struct hd20_iwm *);
void hd20_iwm_observe_select(struct hd20_iwm *, uint32_t addr, uint32_t value);
int hd20_iwm_write(struct hd20_iwm *, uint32_t addr, uint8_t type, uint32_t value);

/* MAME's deselection timer is explicit here so tests do not depend on wall time. */
void hd20_iwm_advance(struct hd20_iwm *, unsigned ticks);

#endif
