# Binary hashing with `rxhash`

The standard `rx_hash` native provider supplies collision-resistant hashing to
Level B and Level G code. It is delivered by default but is not part of the
minimal compiler or VM core. The provider is process-reentrant and retains no
mutable global hashing context.

## SHA-256 procedures

```rexx
rxhash.sha256(data = .binary) = .binary
rxhash.sha256hex(data = .binary) = .string
rxhash.sha256init() = .binary
rxhash.sha256update(state = .binary, data = .binary) = .binary
rxhash.sha256final(state = .binary) = .binary
rxhash.sha256finalhex(state = .binary) = .string
rxhash.sha256file(path = .string) = .binary
rxhash.sha256filehex(path = .string) = .string
```

`sha256`, `sha256final`, and `sha256file` return exactly 32 raw digest bytes.
`sha256hex`, `sha256finalhex`, and `sha256filehex` return exactly 64 canonical
lowercase hexadecimal characters. Binary inputs are hashed byte for byte;
embedded NUL bytes and byte sequences that are not valid UTF-8 participate
normally. Inputs are not changed.

The existing `sha256(data)` procedure remains source- and result-compatible.
Use the direct hexadecimal forms when text is required rather than applying
`bin2x()` and `lower()` at every call site.

## Incremental state

`sha256init()` creates an opaque immutable `.binary` state. Each
`sha256update()` validates and decodes its input, hashes the supplied bytes, and
returns a newly encoded state; it never mutates the supplied state. An empty
update is valid and returns the same canonical value. Finalization validates
and decodes a copy, so `sha256final()` and `sha256finalhex()` may be called
repeatedly with identical results and do not consume the state.

The state contains no native pointer, C structure image, padding, or
host-endian field. It is an ordinary binary value and is safe to copy, retain,
persist, or transfer through the supported task/process binary-value
mechanisms. State version 1 has this fixed 152-byte, platform-independent
encoding:

| Offset | Size | Encoding |
| --- | ---: | --- |
| 0 | 8 | ASCII magic `RXSHA256`. |
| 8 | 1 | Format version `1`. |
| 9 | 1 | Flags; zero in version 1. |
| 10 | 2 | Big-endian header size `56`. |
| 12 | 8 | Total input byte count, unsigned big-endian. |
| 20 | 32 | Eight SHA-256 chaining words, each unsigned big-endian. |
| 52 | 1 | Pending-byte count, from 0 through 63. |
| 53 | 3 | Reserved zero bytes. |
| 56 | 64 | Pending block prefix followed by canonical zero padding. |
| 120 | 32 | SHA-256 integrity digest of bytes 0 through 119. |

The decoder requires the exact size, magic, version, header size, zero flags
and reserved bytes, canonical zero padding, a total count within SHA-256's
64-bit bit-length limit, and a pending count equal to the total byte count
modulo 64. Before any complete block, the chaining words must also equal the
SHA-256 initial value. The integrity digest detects accidental state
corruption; it is not an authenticity or authorization mechanism.

Malformed, truncated, unsupported-version, structurally inconsistent, and
integrity-failing states raise `INVALID_ARGUMENTS` with a stable category in
the message. An update that would exceed the SHA-256 length limit raises
`OVERFLOW_UNDERFLOW`. No invalid state is passed to the hashing engine.

## File hashing

`sha256file()` and `sha256filehex()` synchronously open `path` in binary mode
and hash the exact bytes returned by the filesystem. They use the same
incremental engine as the memory and state procedures, read fixed 32 KiB
chunks, and keep bounded memory regardless of file size. They never read or
concatenate the complete file.

An empty path raises `INVALID_ARGUMENTS`. Open, read, and close failures raise
`NOTREADY` and identify the failing stage as `file open failed`, `file read
failed`, or `file close failed`. The file is closed after EOF and after a read
or length failure; when both reading and closing fail, the read failure is the
reported primary failure. A file exceeding SHA-256's representable length
raises `OVERFLOW_UNDERFLOW`.

These calls deliberately do not select files, connectors, cancellation policy,
application size ceilings, or filesystem authorization. The caller owns those
policies and should perform any needed preflight checks before starting the
synchronous hash.

## Examples

One-shot raw and hexadecimal hashing works identically in Level B and Level G:

```rexx
options levelb

import rxfnsb
import rxhash

data = "610062ff"x as .binary
raw = rxhash..sha256(data)
hex = rxhash..sha256hex(data)
say binlength(raw)                  /* 32 */
say hex                             /* 64 lowercase hexadecimal characters */
```

Incremental callers retain the returned state after every update:

```rexx
state = rxhash..sha256init()
state = rxhash..sha256update(state, "6162"x as .binary)
state = rxhash..sha256update(state, ''x as .binary)
state = rxhash..sha256update(state, "63"x as .binary)

say rxhash..sha256finalhex(state)
/* ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad */
```

File hashing is a single synchronous call:

```rexx
digest = rxhash..sha256file("artifacts/index.bin")
say rxhash..sha256filehex("artifacts/index.bin")
```

## Delivery

The functions are declared directly by RXPA, so there is no Rexx wrapper to
import. A retained call records provider `rx_hash` in the compiled image.
Normal `rxvm`, `rxbvm`, and `rxtvm` execution resolves the installed dynamic
provider automatically; `crexx -native` selects its static archive
automatically. An explicit provider filename or `-p` argument is not required
for a standard installation.

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
