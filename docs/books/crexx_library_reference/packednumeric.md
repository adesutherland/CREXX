# Packed Numeric Owners

The `rxfnsg` library provides `.packedfloat` and `.packedint` as explicit
Level G owners of contiguous host-native numeric values. They wrap the Level B
`<packed..float>` and `<packed..int>` binary access surface; they are not new
runtime representations and do not change ordinary numeric arrays.

```rexx
options levelg
import rxfnsg

values = .packedfloat(3)
call values.set(0, 1.25)
call values.set(2, 100.0)
say values.get(2)

indexes = .packedint(8)
call indexes.fill(-1)
```

Both classes have the same zero-based API:

| Member | Contract |
|---|---|
| `*(size)` | Allocate `size` items and initialize every byte to zero. Negative or unrepresentable sizes raise `OUT_OF_RANGE`. |
| `fromBinary(data)` | Own a copy of the host-native bytes. A length that is not a whole number of items raises `INVALID_ARGUMENTS`. |
| `size()` | Return the number of native items, not the byte length. |
| `get(index)` | Read the item at a zero-based index. An incomplete or out-of-range item raises `OUT_OF_RANGE`. |
| `set(index, value)` | Replace an existing item without resizing. An invalid index raises before mutation. |
| `resize(size)` | Preserve retained items, discard truncated items, and zero-fill growth. |
| `fill(value)` | Set every existing item to the supplied native value. |
| `binary()` | Return a weak mutable `reference .binary` to the owned bytes. The owner must outlive every use of the reference. |

`.packedfloat` stores exact VM `.float` (`rxfloat`) values and `.packedint`
stores exact VM `.int` (`rxinteger`) values. The object owns those bytes
directly in `register.0.binary`. A native provider with a declared
`.packedfloat` or `.packedint` argument can therefore bind the object's payload
without asking Rexx to call `binary()` or create an intermediate binary value.
`fromBinary` is an inbound copy boundary. For ordinary Rexx callers,
`payload = dereference values.binary()` creates a scoped live alias without
copying, while `payload = snapshot values.binary()` explicitly makes an owned
copy. A raw native payload pointer must not be retained across a call or resize.

The representation uses the current host's width, byte order, alignment, and
numeric representation. It is suitable for in-process numerical kernels and
native providers, but not for files, persistence, network protocols, or
cross-host exchange. Use the encoded `<at..type>` binary-memory surface for
those cases.

Release 1 does not provide packed strings or objects, bracket indexing such as
`values[2]`, or automatic packed storage for ordinary `.int[]` and `.float[]`
arrays. Those require separate post-release language and compatibility work.
