# HD20 protocol notes

This is an evolving implementation record, not a claim of complete protocol conformance.

## Host storage

- Sector size: 512 bytes
- Address field: three bytes, supporting a 24-bit block number
- Maximum host image: `2147483648` bytes exactly
- Accesses must be sector aligned
- Sparse files are useful for boundary testing

## Command reception

The observed native read command is:

```text
AA 00 00 00 00 00 FC 00
```

The first `$AA` establishes synchronization. Hardware traces showed four raw prefix bytes before the 7-for-8-decoded command payload. Treating those bytes as encoded data produced an invalid block address; consuming four produces block 0.

## Response transfer

Current implementation sends:

```text
AA                 fast acknowledgement
AA                 response synchronization
7-for-8 payload    response packet
```

The logical response packet is 539 bytes:

```text
80 | sequence | 4-byte status | 20-byte tags | 512-byte sector | checksum
```

The additive checksum is internally self-consistent in the implementation, but that is not proof that the SE's ROM applies the same coverage or interpretation.

## Open protocol questions

- Exact values and meanings of all 20 tag bytes
- Whether status is signed, bit-packed, or interpreted differently by command
- Checksum coverage and byte order
- Whether the final padding/dummy byte is required
- Exact `/HSHK` level and timing during each byte
- Whether response bytes need to be latched on a different IWM phase edge
