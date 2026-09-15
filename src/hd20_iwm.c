#include "hd20_iwm.h"
#include "hd20_transport.h"
#include "vsony_host.h"
#include <stdlib.h>

#define HD20_IWM_BASE 0xDFE1FFu
#define HD20_IWM_LIMIT 0xE00000u
#define HD20_IWM_DELAY_TICKS 1u

enum active_mode { ACTIVE_IDLE, ACTIVE_DELAY, ACTIVE_ACTIVE };

struct hd20_iwm {
    struct hd20_transport *transport;
    struct vsony_host *disk;
    uint8_t phases;
    uint8_t control;
    uint8_t presence_reads;
    unsigned delay_ticks;
    enum active_mode active;
    enum hd20_iwm_state state;
};

static unsigned alias_register(uint32_t address)
{
    return ((address - HD20_IWM_BASE) >> 9) & 15u;
}

static void update_state(struct hd20_iwm *iwm)
{
    if (iwm->active != ACTIVE_ACTIVE || !(iwm->control & 0x20)) {
        iwm->state = HD20_IWM_IDLE;
    } else if (iwm->phases == HD20_P5 || iwm->phases == HD20_P6 ||
               iwm->phases == HD20_P7) {
        iwm->state = HD20_IWM_PRESENCE;
    } else if (hd20_transport_device_ready(iwm->transport)) {
        iwm->state = HD20_IWM_RESPONSE;
    } else if (iwm->phases == HD20_P1 || iwm->phases == HD20_P3) {
        iwm->state = HD20_IWM_COMMAND_RECEIVE;
    } else {
        iwm->state = HD20_IWM_IDLE;
    }
}

static void set_active(struct hd20_iwm *iwm, enum active_mode mode)
{
    iwm->active = mode;
    iwm->delay_ticks = mode == ACTIVE_DELAY ? HD20_IWM_DELAY_TICKS : 0;
    update_state(iwm);
}

static void apply_alias(struct hd20_iwm *iwm, uint32_t address)
{
    unsigned reg = alias_register(address);
    if (reg < 8) {
        if (reg & 1)
            iwm->phases |= (uint8_t)(1u << (reg >> 1));
        else
            iwm->phases &= (uint8_t)~(1u << (reg >> 1));
        hd20_transport_phase(iwm->transport, (enum hd20_phase)iwm->phases);
    } else if (reg & 1) {
        iwm->control |= (uint8_t)(1u << (reg >> 1));
    } else {
        iwm->control &= (uint8_t)~(1u << (reg >> 1));
    }

    /* The HD20 occupies the device-B selection.  Keep that selection
     * separate from the IWM active lifetime: clearing it starts MAME's
     * delayed shutdown, rather than fabricating a device response. */
    if (iwm->control & 0x20) {
        set_active(iwm, ACTIVE_ACTIVE);
    } else if (iwm->active == ACTIVE_ACTIVE) {
        /* MODE_TIMER is bit 2. Without it MAME enters delayed idle. */
        set_active(iwm, (iwm->control & 0x04) ? ACTIVE_IDLE : ACTIVE_DELAY);
    } else {
        update_state(iwm);
    }
}

struct hd20_iwm *hd20_iwm_create(struct vsony_host *disk)
{
    struct hd20_iwm *iwm = calloc(1, sizeof(*iwm));
    if (!iwm)
        return NULL;
    iwm->disk = disk;
    iwm->transport = hd20_transport_create(disk);
    if (!iwm->transport) {
        free(iwm);
        return NULL;
    }
    iwm->active = ACTIVE_IDLE;
    update_state(iwm);
    return iwm;
}

void hd20_iwm_destroy(struct hd20_iwm *iwm)
{
    if (iwm) {
        hd20_transport_destroy(iwm->transport);
        vsony_close(iwm->disk);
        free(iwm);
    }
}

int hd20_iwm_owns(uint32_t address)
{
    return address >= HD20_IWM_BASE && address < HD20_IWM_LIMIT &&
           (address & 0xffu) == 0xffu;
}

int hd20_iwm_selected(const struct hd20_iwm *iwm)
{
    return iwm && (iwm->control & 0x20) != 0;
}

int hd20_iwm_active(const struct hd20_iwm *iwm)
{
    return iwm && iwm->active == ACTIVE_ACTIVE;
}

int hd20_iwm_device_select(const struct hd20_iwm *iwm)
{
    if (!iwm || iwm->active == ACTIVE_IDLE)
        return 0;
    return hd20_iwm_selected(iwm) ? 2 : 1;
}

int hd20_iwm_presence(const struct hd20_iwm *iwm)
{
    return iwm && hd20_iwm_active(iwm) && iwm->state == HD20_IWM_PRESENCE;
}

int hd20_iwm_ready(const struct hd20_iwm *iwm)
{
    return iwm && hd20_iwm_active(iwm) &&
           hd20_transport_device_ready(iwm->transport);
}

enum hd20_iwm_state hd20_iwm_state(const struct hd20_iwm *iwm)
{
    return iwm ? iwm->state : HD20_IWM_IDLE;
}

void hd20_iwm_advance(struct hd20_iwm *iwm, unsigned ticks)
{
    if (!iwm || iwm->active != ACTIVE_DELAY)
        return;
    if (ticks >= iwm->delay_ticks) {
        iwm->delay_ticks = 0;
        set_active(iwm, ACTIVE_IDLE);
    } else {
        iwm->delay_ticks -= ticks;
    }
}

void hd20_iwm_observe_select(struct hd20_iwm *iwm, uint32_t address,
                             uint32_t value)
{
    (void)value;
    if (iwm && hd20_iwm_owns(address))
        apply_alias(iwm, address);
}

uint32_t hd20_iwm_read(struct hd20_iwm *iwm, uint32_t address, uint8_t type)
{
    uint8_t byte;
    (void)type;
    if (!iwm || !hd20_iwm_owns(address))
        return (uint32_t)HD20_IWM_PASS_THROUGH;
    apply_alias(iwm, address);
    if (!hd20_iwm_active(iwm) || !hd20_iwm_selected(iwm))
        return (uint32_t)HD20_IWM_PASS_THROUGH;
    if (iwm->state == HD20_IWM_RESPONSE && alias_register(address) == 14u)
        return 0x80u;
    if (iwm->state == HD20_IWM_COMMAND_RECEIVE) {
        unsigned reg = alias_register(address);
        if (reg == 12u)
            return 0x80u;
        if (reg == 14u)
            return (iwm->phases & 0x01u) ? 0u : 0x80u;
    }
    if (iwm->state == HD20_IWM_PRESENCE) {
        uint32_t result = iwm->presence_reads < 2 ? 1u : 0u;
        if (iwm->presence_reads < 3)
            iwm->presence_reads++;
        if (iwm->presence_reads == 3) {
            iwm->phases = 0;
            hd20_transport_phase(iwm->transport, HD20_P0);
            update_state(iwm);
        }
        return result;
    }
    if (iwm->state != HD20_IWM_RESPONSE)
        return (uint32_t)HD20_IWM_PASS_THROUGH;
    if (alias_register(address) == 13u) {
        if (hd20_transport_device_byte(iwm->transport, &byte) == 0)
            return byte;
        return 0;
    }
    return (uint32_t)HD20_IWM_PASS_THROUGH;
}

int hd20_iwm_write(struct hd20_iwm *iwm, uint32_t address, uint8_t type,
                   uint32_t value)
{
    unsigned reg;
    (void)type;
    if (!iwm || !hd20_iwm_owns(address))
        return HD20_IWM_PASS_THROUGH;
    reg = alias_register(address);
    apply_alias(iwm, address);
    if (hd20_iwm_active(iwm) && hd20_iwm_selected(iwm) &&
        (reg == 13 || reg == 15) &&
        iwm->state == HD20_IWM_COMMAND_RECEIVE &&
        (iwm->control & 0xc0) == 0xc0 &&
        hd20_transport_host_byte(iwm->transport, (uint8_t)value) == 0) {
        update_state(iwm);
        return 1;
    }
    return HD20_IWM_PASS_THROUGH;
}
