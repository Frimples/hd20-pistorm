# Reproducible tests

## Focused adapter replay

From `src/`:

```bash
cc -std=c11 -Wall -Wextra -Werror -I. \
  hd20_protocol.c hd20_transport.c hd20_iwm.c vsony_host.c \
  test_hd20_iwm.c -o /tmp/test_hd20_iwm
/tmp/test_hd20_iwm
```

Expected:

```text
HD20 IWM replay regression passed
```

## Host-boundary checks

Test a sparse file at exactly 2 GiB and verify:

- final-sector read/write succeeds;
- an unaligned access fails;
- a request at or beyond the limit fails;
- a larger file is rejected.

## MAME acceptance test

The future `macsehd20` harness must record the response variant, the ROM PC at rejection/acceptance, and whether a second block command occurs. Do not infer success from a screenshot of Finder alone unless the HD20 volume is visibly mounted.
