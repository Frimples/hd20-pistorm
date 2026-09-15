# HD20 emulation on PiStorm Macintosh SE

Research and implementation record for exposing a host-backed HD20/DCD disk to a stock Macintosh SE through PiStorm.

> **Status: experimental and not yet bootable.** The SE reaches the HD20 command phase, reads block 0 from the host image, receives the complete response, and retries the request. Finder/HD20 disk access has not been demonstrated on real hardware.

## Objectives

- Use the stock 256 KiB Macintosh SE ROM and its native HD20 support.
- Present a sector-based raw/`.dsk` image through the external floppy-port HD20 protocol.
- Accept images from floppy-sized media through exactly 2 GiB.
- Preserve ordinary 800K floppy behavior when HD20 is inactive.
- Keep HD20, SCSI, and the separate 24-bit ROM patch work independent.
- Validate protocol assumptions in MAME before sending further PiStorm binaries.

## Current evidence

Confirmed on real PiStorm hardware:

- presence probing reaches the adapter;
- the native command is received and decoded;
- command framing requires four raw prefix bytes after the initial `$AA`;
- block 0 is read from the backing image;
- a 539-byte logical response is generated;
- response data is read through `$DFFBFF`;
- the SE consumes complete responses and retries block 0.

Representative trace:

```text
[HD20] cmd_len=8 result=0 aa 00 00 00 00 00 fc 00 block=0 response_len=539
responses=3
response_fifo_complete=3
chk=05 verify=05
```

The remaining failure is after response consumption. Likely areas include HD20 tag semantics, response header/checksum interpretation, and exact IWM handshake/state behavior.

## Repository map

- [`STATUS.md`](STATUS.md) — current truth and acceptance criteria
- [`HISTORY.md`](HISTORY.md) — chronological work log
- [`docs/protocol.md`](docs/protocol.md) — protocol and packet model
- [`docs/iwm-analysis.md`](docs/iwm-analysis.md) — IWM observations and aliases
- [`docs/mame-testing.md`](docs/mame-testing.md) — MAME baseline and planned hook
- [`docs/hardware-testing.md`](docs/hardware-testing.md) — hardware evidence and failures
- [`docs/image-format.md`](docs/image-format.md) — host-image limits and boot-image distinction
- [`src/`](src/) — sanitized snapshot of the HD20 implementation and focused tests
- [`tests/`](tests/) — commands and expected local results

## Reproducing the focused tests

```bash
cd src
cc -std=c11 -Wall -Wextra -Werror -I. \
  hd20_protocol.c hd20_transport.c hd20_iwm.c vsony_host.c \
  test_hd20_iwm.c -o /tmp/test_hd20_iwm
/tmp/test_hd20_iwm
```

Expected result:

```text
HD20 IWM replay regression passed
```

## Important exclusions

This repository intentionally does **not** contain Apple ROMs, copyrighted Macintosh disk images, hardware boot logs containing private paths, credentials, or PiStorm runtime binaries. Supply those separately when reproducing the work.

## References

- [Big Mess O' Wires: Emulating the Apple HD20](https://www.bigmessowires.com/2014/02/06/emulating-the-apple-hd20/)
- [Big Mess O' Wires: Reverse-engineering the HD20](https://www.bigmessowires.com/2014/11/22/reverse-engineering-the-hd20/)
- [Commented HD20 ROM listing](https://www.bigmessowires.com/HD20-rom-commented-listing.asm)
- [MAME debugger documentation](https://docs.mamedev.org/debugger/)
