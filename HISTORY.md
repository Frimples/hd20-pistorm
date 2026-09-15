# Work history

## Foundation

- Confirmed the target is a stock 256 KiB Macintosh SE ROM with native HD20 support.
- Established that images larger than floppy capacity require HD20/DCD semantics, not ordinary GCR floppy emulation.
- Added a host-backed sector interface with 64-bit offsets and an exact 2 GiB limit.
- Kept SCSI integration and the separate 24-bit ROM patch out of the HD20 path.

## Protocol and adapter

- Implemented 7-for-8 encoding/decoding, synchronization, checksums, status/identity responses, block commands, and multi-block storage access.
- Added an IWM adapter with register aliases, motor/select handling, Q6/Q7 modes, presence probing, phase state, response FIFO, and physical-floppy pass-through.
- Corrected the IWM address model to base `$DFE1FF` with 0x200-byte spacing.
- Corrected the dispatcher so pass-through (`-1`) is not mistaken for a handled access.
- Added bounded tracing after unrestricted tracing starved emulation.

## Hardware discoveries

- Presence handling must occur before device selection, but idle traffic must not broadly claim `/HSHK`.
- The ROM sends initial command traffic on odd aliases 13 and 15.
- Four raw prefix bytes follow the command synchronization byte.
- Response bytes are read on `$DFFBFF`; `$DFFDFF` is separate handshake/control sensing.
- The response begins with two synchronization bytes: fast acknowledgement followed by response synchronization.
- The SE reads the complete response but retries block 0.
- Pacing at approximately 230.4 kbit/s did not fix the retry.

## Investigation state

- Response layouts of 537 and 539 logical bytes were compared; 539 bytes was retained.
- The 20-byte tag region was identified as 12 standard floppy tag bytes plus 8 HD20-specific bytes, but exact values remain unresolved.
- Stock MAME reaches Finder from the supplied 400K HD20 Startup floppy after a long delay.
- Stock MAME has no HD20 endpoint. The next controlled experiment is a separate opt-in `macsehd20` machine with hooks only at the IWM data register.
