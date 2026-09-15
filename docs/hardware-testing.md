# Hardware testing record

## Platform

- Macintosh SE
- Stock 256 KiB SE ROM
- PiStorm on older 32-bit ARM Pi OS
- External 800K floppy path retained for baseline tests

## Baseline

With HD20 disabled, the external 800K floppy boots normally. With HD20 enabled and the external floppy disconnected, early revisions produced `X` or `?`; broad interception was removed to preserve ordinary floppy behavior.

## Current result

The adapter now reaches the native HD20 transaction:

```text
command received: AA 00 00 00 00 00 FC 00
block requested: 0
logical response length: 539
complete response transfers: 3
block-0 retries: 3
final screen: question-mark floppy
```

A sample sector payload matched Macintosh boot-sector-looking data, and the implementation's checksum verified against itself. Neither proves ROM acceptance.

## Negative tests

- Standard-speed response pacing: no improvement.
- 537-byte response layout: no improvement.
- Handshake-low experiment: ROM polled indefinitely and did not read the FIFO.
- Broad idle `/HSHK` claiming: caused `X` and was reverted.

No additional PiStorm binary should be sent until a local MAME experiment or equivalent byte-level evidence justifies a change.
