## Level B binary helpers

`binary.crexx` is the typed Level B binary library. It has two deliberately
different position conventions:

- the copy-returning `BIN*` value helpers use 1-based byte positions;
- the exposed packed-memory mutators use zero-based byte offsets matching
  `<at..type>` and RXAS binary-memory instructions.

This module is not a Level C Classic BIF. There is consequently no same-named
`lib/rxfnsc` implementation or Level C contract.

## Binary value helpers

| Function | Result | Contract |
| --- | --- | --- |
| `binlength(data = .binary)` | `.int` | Logical byte length. |
| `binbyte(data = .binary, position = .int)` | `.int` | Byte at a 1-based position, or `-1` outside the value. |
| `binsetbyte(data = .binary, position = .int, byte = .int)` | `.binary` | Copy with one byte replaced. |
| `binsubstr(data = .binary, start = .int, count = -1)` | `.binary` | Copied slice; a negative count means through the end. |
| `binconcat(left = .binary, right = .binary)` | `.binary` | Byte concatenation. |
| `binoverlay(new = .binary, target = .binary, start = .int)` | `.binary` | Fixed-size overlay into a copy. |
| `bininsert(new = .binary, target = .binary, before = .int)` | `.binary` | Insert before a 1-based position; prepend at `before <= 1`, append past the end. |
| `bindelstr(target = .binary, start = .int, count = 0)` | `.binary` | Delete bytes; zero means through the end. |
| `binpos(needle = .binary, haystack = .binary, start = 1)` | `.int` | First 1-based match or zero; an empty needle returns zero. |
| `bincompare(left = .binary, right = .binary)` | `.int` | Zero when equal, otherwise the first differing 1-based position. |
| `bin2x(data = .binary)` | `.string` | Two uppercase hexadecimal digits per byte. |
| `x2bin(hex = .string)` | `.binary` | Decode hex, ignoring ASCII blanks and left-padding an odd nibble. |

`binsetbyte` and `binoverlay` raise `OUT_OF_RANGE` for invalid byte ranges;
`binsetbyte` also uses it for a byte outside `0..255`. `binsubstr`, `bindelstr`,
and `binpos` raise `INVALID_ARGUMENTS` for a nonpositive 1-based start.
`bindelstr` raises it for a negative count, and `x2bin` raises it for any
non-blank character that is not hexadecimal.

```rexx
data = "001122ff"x as .binary
binlength(data)                                      /* 4 */
binbyte(data, 2)                                     /* 17 */
bin2x(binsubstr(data, 2, 2))                         /* "1122" */
bin2x(binoverlay("abcd"x as .binary, data, 2))      /* "00ABCDFF" */
binpos("1122"x as .binary, data)                    /* 2 */
bincompare(data, "001123ff"x as .binary)            /* 3 */
bin2x(x2bin("ff 00 aa"))                            /* "FF00AA" */
```

The search implementation compares each candidate directly in the binary
buffer; it does not materialize a slice or call a byte helper for every byte.
Equal-length `bincompare` values also take the whole-value zero-copy fast path.

## Packed-memory mutators

The first argument of each mutator is declared with `arg expose`, so the
caller's binary variable changes in place. Successful calls return the new
logical byte length, except that fixed-size operations naturally return the
unchanged length.

| Function | Effect |
| --- | --- |
| `binresize(data, length)` | Preserve or zero-grow to `length`. |
| `binclear(data)` | Set the logical length to zero. |
| `binfill(data, byte)` | Fill every current byte. |
| `binfillat(data, offset, length, byte)` | Fill a zero-based span. |
| `bincopy(dst, dst_offset, src, src_offset, length)` | Copy between binary values. |
| `binmemmove(data, dst_offset, src_offset, length)` | Overlap-safe move within one value. |
| `binappend(dst, src)` | Append every source byte. |
| `binupdate(dst, offset, src)` | Fixed-size overlay. |
| `binmakegap(data, offset, length)` | Open a zero-filled gap. |
| `bindrop(data, offset, length)` | Delete a span. |

Every offset and length is `.int`. Invalid spans, negative lengths, and bytes
outside `0..255` raise `OUT_OF_RANGE`. A zero-length span is valid when its
offset is from zero through the current length, inclusive. Fixed-size writes
never grow the destination; use `binresize`, `binappend`, or `binmakegap` when
growth is required.

```rexx
page = "00112233"x as .binary
call binmakegap page, 2, 2       /* 00 11 00 00 22 33 */
call binfillat page, 2, 2, 255   /* 00 11 FF FF 22 33 */
call bindrop page, 2, 2          /* 00 11 22 33 */
```

`bincopy` and `binmemmove` lower inside the library to the VM's checked move
instructions, avoiding a temporary chunk. `binmakegap` and `bindrop` likewise
use overlap-safe in-buffer moves.

The focused harness is
`lib/rxfnsb/tests_functional/ts_binary.crexx`. It covers the examples,
copy-versus-mutation behavior, long search, overlap in both directions,
zero-length boundary spans, and each documented signal family in optimized and
unoptimized selector overlays.
