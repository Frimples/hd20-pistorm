#include "hd20_iwm.h"
#include "hd20_protocol.h"
#include "vsony_host.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

static uint32_t alias(unsigned reg) { return 0xDFE1FFu + (reg << 9); }

int main(void) {
    const char *image = "/tmp/hd20-iwm.img";
    unlink(image);
    struct vsony_host *disk = vsony_open(image, 2ULL * 1024 * 1024 * 1024);
    assert(disk);
    struct hd20_iwm *iwm = hd20_iwm_create(disk);
    assert(iwm);

    /* Captured startup probe occurs before the HD20 select transition. */
    assert(hd20_iwm_device_select(iwm) == 0);
    assert(!hd20_iwm_active(iwm));
    assert(hd20_iwm_read(iwm, alias(7), 0) == (uint32_t)HD20_IWM_PASS_THROUGH);
    assert(hd20_iwm_read(iwm, alias(5), 0) == (uint32_t)HD20_IWM_PASS_THROUGH);
    assert(hd20_iwm_read(iwm, alias(1), 0) == (uint32_t)HD20_IWM_PASS_THROUGH);
    assert(hd20_iwm_read(iwm, alias(5), 0) == (uint32_t)HD20_IWM_PASS_THROUGH);
    assert(hd20_iwm_read(iwm, alias(7), 0) == (uint32_t)HD20_IWM_PASS_THROUGH);

    /* The physical IWM leaves phase bits latched across the probe. Establish
     * the known phase-0 baseline before replaying the DCD sequence. */
    hd20_iwm_write(iwm, alias(0), 0, 0);
    hd20_iwm_write(iwm, alias(2), 0, 0);
    hd20_iwm_write(iwm, alias(4), 0, 0);
    hd20_iwm_write(iwm, alias(6), 0, 0);

    /* Select HD20, then reproduce the phase-6, phase-7, phase-5 probe. */
    hd20_iwm_write(iwm, alias(15), 0, 0x17);
    hd20_iwm_write(iwm, alias(11), 0, 0);
    assert(hd20_iwm_selected(iwm));
    assert(hd20_iwm_active(iwm));
    assert(hd20_iwm_device_select(iwm) == 2);
    hd20_iwm_write(iwm, alias(5), 0, 0); /* phase 4 */
    hd20_iwm_write(iwm, alias(3), 0, 0); /* phase 6 */
    assert(hd20_iwm_selected(iwm));
    assert(hd20_iwm_read(iwm, alias(1), 0) == 1);
    hd20_iwm_write(iwm, alias(1), 0, 0); /* phase 7 */
    assert(hd20_iwm_read(iwm, alias(1), 0) == 1);
    hd20_iwm_write(iwm, alias(2), 0, 0); /* phase 5 */
    assert(hd20_iwm_read(iwm, alias(1), 0) == 0);

    /* Enter command receive (phase 1, active, read-data mode). */
    hd20_iwm_write(iwm, alias(1), 0, 0); /* phase 1 */
    hd20_iwm_write(iwm, alias(9), 0, 0); /* enable */
    hd20_iwm_write(iwm, alias(13), 0, 0); /* select read-data slot */
    assert(hd20_iwm_state(iwm) == HD20_IWM_COMMAND_RECEIVE);

    /* Raw AA is followed by one 7-byte encoded command block. */
    uint8_t command[] = { HD20_CMD_READ, 1, 0, 0, 0, 0, 0 };
    uint8_t encoded[8];
    assert(hd20_encode7(command, sizeof(command), encoded, sizeof(encoded)) == 8);
    assert(hd20_iwm_write(iwm, alias(13), 0, HD20_SYNC_READ) == 1);
    assert(hd20_iwm_write(iwm, alias(13), 0, 0x81) == 1);
    assert(hd20_iwm_write(iwm, alias(13), 0, 0x81) == 1);
    assert(hd20_iwm_write(iwm, alias(13), 0, 0x81) == 1);
    assert(hd20_iwm_write(iwm, alias(13), 0, 0x81) == 1);
    for (unsigned i = 0; i < sizeof(encoded); ++i)
        assert(hd20_iwm_write(iwm, alias(13), 0, encoded[i]) == 1);

    /* Fast-ACK completion makes the response ready before the ROM's
     * subsequent phase transition. */
    assert(hd20_iwm_state(iwm) == HD20_IWM_RESPONSE);
    assert(hd20_iwm_ready(iwm));

    /* Exit write/control aliases; response sync is on the odd read-data alias. */
    hd20_iwm_write(iwm, alias(14), 0, 0);
    assert(hd20_iwm_read(iwm, alias(13), 0) == HD20_SYNC_READ);

    /* Unselected HD20 aliases must remain available to the normal floppy path. */
    hd20_iwm_write(iwm, alias(10), 0, 0); /* deselect */
    assert(!hd20_iwm_selected(iwm));
    assert(!hd20_iwm_active(iwm));
    assert(hd20_iwm_device_select(iwm) == 1); /* delayed physical release */
    assert(hd20_iwm_read(iwm, alias(12), 0) == (uint32_t)HD20_IWM_PASS_THROUGH);
    assert(hd20_iwm_write(iwm, alias(13), 0, 0x55) == HD20_IWM_PASS_THROUGH);
    hd20_iwm_advance(iwm, 1);
    assert(hd20_iwm_device_select(iwm) == 0);

    hd20_iwm_destroy(iwm);
    puts("HD20 IWM replay regression passed");
    return 0;
}
