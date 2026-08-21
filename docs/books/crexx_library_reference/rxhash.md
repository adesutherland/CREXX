# Binary hashing with `rxhash`

The standard `rx_hash` native provider supplies collision-resistant hashing to
Level B and Level G code. It is delivered by default but is not part of the
minimal compiler or VM core.

## Procedures

```rexx
rxhash.sha256(data = .binary) = .binary
```

`sha256` hashes the exact bytes in `data` and returns the 32 raw SHA-256 digest
bytes. Embedded zero bytes participate normally. The input is not changed.
Convert the result with `bin2x()` only when a 64-character hexadecimal form is
needed:

```rexx
options levelg

import rxfnsb
import rxhash

digest = rxhash..sha256("616263"x as .binary)
say bin2x(digest)
/* BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD */
```

The function is declared directly by RXPA, so there is no Rexx wrapper to
import. A retained call records provider `rx_hash` in the compiled image.
Normal `rxvm` and `rxbvm` execution resolves the installed dynamic provider
automatically; `crexx -native` selects its static archive automatically. An
explicit provider filename or `-p` argument is not required for a standard
installation.

This procedure computes one complete digest per call. An incremental file or
streaming hash API has not yet been defined.

The same provider also publishes binary-safe named 32-bit functions:

| Procedure | Contract |
|---|---|
| `djb2(data = .binary)` | DJB2 modulo 2^32. |
| `murmur3(data = .binary, seed = .int)` | MurmurHash3 x86 32-bit with the supplied seed. |
| `fnv1a(data = .binary)` | FNV-1a 32-bit. |
| `crc32(data = .binary)` | IEEE CRC-32. |

They return the unsigned 32-bit bit pattern as a non-negative `.int`. Embedded
zero bytes participate normally. These are checksums or table/container hashes,
not substitutes for SHA-256 where collision resistance is required. The
historical `rxmath` aliases are intentionally absent in this pre-release API.
