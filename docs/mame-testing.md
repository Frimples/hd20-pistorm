# MAME testing plan

## Baseline completed

Installed MAME 0.276 with the stock `macse` driver and the user-supplied supporting ADB/modem ROM staged outside this repository. The SE boots the supplied 400K HD20 Startup floppy and eventually reaches Finder. The long delay is normal for this emulated setup.

## Current implementation stage

The source tree now contains an uncommitted trace/injection harness:

- optional IWM data-write observer;
- optional IWM data-read observer with 68K PC logging;
- optional data-read override with an explicit validity callback;
- separate `macsehd20` machine registration;
- captured-command trigger for `AA 00 00 00 00 00 FC 00`;
- first synthetic 539-byte response variant using 7-for-8 encoding.

The harness does not alter stock `macse`. The patch is preserved at [`../mame/macsehd20-instrumentation.patch`](../mame/macsehd20-instrumentation.patch). It has not yet been compiled because this host lacks MAME's SDL2, ALSA, fontconfig, and Qt development dependencies.

## Required temporary harness

Add a separate `macsehd20` configuration derived from `macse`:

1. Add optional callbacks around `iwm_device::data_w()`.
2. Add a read callback only for the selected IWM data-register return.
3. Preserve all stock control, phase, motor, drive-select, timer, and flux behavior.
4. Recognize the captured command and feed a selectable response variant.
5. Log FIFO index, substituted byte, 68K PC, retry/error transitions, and the next command.
6. Leave ordinary `macse` unchanged.

## Response variants to compare

- current 539-byte packet with zero tags;
- 537-byte packet variant;
- candidate tag values from the ROM listing/reverse-engineering notes;
- checksum coverage variants;
- final padding variants.

A response is accepted only if the ROM proceeds to another block or reaches a visible startup/Finder state. FIFO exhaustion alone is insufficient.
