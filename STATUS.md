# Status

Last updated: 2026-09-14

## Working

- Host backend uses 64-bit offsets and rejects unaligned, zero-length, and out-of-range accesses.
- Maximum accepted image size is exactly `2 GiB` (`2147483648` bytes).
- HD20 protocol encoder/decoder tests pass.
- HD20 transport tests pass.
- IWM replay regression passes.
- Static ARMHF production builds complete for the older 32-bit Pi OS.
- HD20 is opt-in through `hd20_image`.
- Normal floppy pass-through was restored after early broad-interception regressions.

## Proven on hardware

The stock SE ROM performs the HD20 presence sequence and sends a native block command. The adapter decodes:

```text
AA 00 00 00 00 00 FC 00
```

The adapter reads block 0 and transmits a complete response. The SE retries block 0 three times and displays the question-mark floppy icon. Standard-speed response pacing did not change this result.

## Not working

- Successful HD20 mount or boot
- Finder from the HD20 image
- A second distinct block request
- Proof that the current 20-byte tags are correct
- MAME validation of response variants

## Acceptance criteria for calling it working

Do not mark the project working until at least one of these is observed on a stock SE ROM:

1. the SE mounts the HD20 volume in Finder;
2. the SE boots a valid System Folder from the HD20 image; or
3. a MAME trace proves the response is accepted and the ROM proceeds to the next block.

## Current response model

```text
logical offset 0       response/status byte 80
logical offset 1       sequence 00
logical offsets 2-5    status 00000000
logical offsets 6-25   20-byte tag area
logical offsets 26-537 512-byte sector
logical offset 538     additive checksum
```

The tag area is currently zero-filled and remains unproven.
