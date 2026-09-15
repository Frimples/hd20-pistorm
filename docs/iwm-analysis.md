# IWM analysis

## Aliases used by the SE

The PiStorm adapter models the IWM aliases at a base of `$DFE1FF`, spaced by 0x200 bytes:

```text
register = ((address - 0xDFE1FF) >> 9) & 15
```

Important observed aliases:

```text
$DFFBFF  data/response FIFO
$DFFDFF  handshake/control sensing
$DFFFFF  control and initial command synchronization
```

## Design rules

- Leave unselected and inactive accesses to the normal floppy path.
- Do not claim idle `/HSHK` traffic; doing so changed the hardware result from `?` to `X`.
- Do not use internal `selected` bookkeeping as proof that the ROM recognized the HD20.
- Keep response FIFO data separate from handshake sensing.
- Keep tracing bounded and avoid synchronous output on every pass-through read.

## Evidence still needed

A useful MAME comparison must record:

- every IWM data-register write and read;
- selected IWM register, phase, Q6/Q7 mode, and handshake state;
- 68K PC at each response byte;
- the first ROM routine that rejects or accepts the response;
- RAM locations written with status or error information;
- whether the next block command is generated.
