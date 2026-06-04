# Built-in functions for cRexx strings and binary values

The `.string` type is central to the Rexx language and all its variants. This is true for cRexx and this implementation also contains the expected *built-in-functions*.[^bif] Level B also provides byte-oriented helpers for the `.binary` type.

Use of these functions needs import of the `rxfnsb` package:

```rexx
import rxfnsb
```

| BIF             | Signature                                    |
|-----------------|----------------------------------------------|
| ABS             | ABS(x)                                       |
| FORMAT          | FORMAT(x, n, d)                              |
| MAX             | MAX(x, y, ...)                               |
| MIN             | MIN(x, y, ...)                               |
| SIGN            | SIGN(x)                                      |
| TRUNC           | TRUNC(x, n)                                  |
| B2X             | B2X(b)                                       |
| BIN2X           | BIN2X(binary)                                |
| BINBYTE         | BINBYTE(binary, position)                    |
| BINCOMPARE      | BINCOMPARE(left, right)                      |
| BINCONCAT       | BINCONCAT(left, right)                       |
| BINDELSTR       | BINDELSTR(binary, start, length)             |
| BININSERT       | BININSERT(new, target, before)               |
| BINLENGTH       | BINLENGTH(binary)                            |
| BINOVERLAY      | BINOVERLAY(new, target, start)               |
| BINPOS          | BINPOS(needle, haystack, start)              |
| BINSETBYTE      | BINSETBYTE(binary, position, byte)           |
| BINSUBSTR       | BINSUBSTR(binary, start, length)             |
| C2D             | C2D(s)                                       |
| C2X             | C2X(s)                                       |
| D2C             | D2C(n)                                       |
| D2X             | D2X(n)                                       |
| X2B             | X2B(x)                                       |
| X2BIN           | X2BIN(x)                                     |
| X2C             | X2C(x)                                       |
| X2D             | X2D(x)                                       |
| CENTER          | CENTER(s, n, pad)                            |
| CENTRE          | CENTRE(s, n, pad)                            |
| CHARIN          | CHARIN(name, count)                          |
| CHAROUT         | CHAROUT(name, string)                        |
| COPIES          | COPIES(s, n)                                 |
| DELSTR          | DELSTR(s, start, length)                     |
| DELWORD         | DELWORD(s, start, n)                         |
| INSERT          | INSERT(s, target, position, length, pad)     |
| JUSTIFY         | JUSTIFY(s, width, pad)                       |
| LEFT            | LEFT(s, n, pad)                              |
| LENGTH          | LENGTH(s)                                    |
| LINEIN          | LINEIN(name)                                 |
| LINEOUT         | LINEOUT(name, string)                        |
| LINES           | LINES(name)                                  |
| LOWER           | LOWER(s)                                     |
| OVERLAY         | OVERLAY(s, target, position, length, pad)    |
| POS             | POS(needle, haystack, start)                 |
| RIGHT           | RIGHT(s, n, pad)                             |
| SPACE           | SPACE(s, n, pad)                             |
| STRIP           | STRIP(s, option, char)                       |
| SUBSTR          | SUBSTR(s, start, length)                     |
| SUBWORD         | SUBWORD(s, start, n)                         |
| TRANSLATE       | TRANSLATE(s, new, old)                       |
| UPPER           | UPPER(s)                                     |
| VERIFY          | VERIFY(s, reference, option, start)          |
| WORD            | WORD(s, n)                                   |
| WORDINDEX       | WORDINDEX(s, n)                              |
| WORDLENGTH      | WORDLENGTH(s, n)                             |
| WORDS           | WORDS(s)                                     |
| DATATYPE        | DATATYPE(s, type)                            |
| RANDOM          | RANDOM(min, max, seed)                       |
| TIME            | TIME(option)                                 |
| DATE            | DATE(option)                                 |
| SOURCELINE      | SOURCELINE(n)                                |
| ARG             | ARG(n)                                       |
| STORAGE         | STORAGE(address, length, newvalue)           |
| TRACE           | TRACE(option)                                |
| VALUE           | VALUE(symbol, newvalue, selector)            |


Table: SAA Rexx Built-In-Functions. {#tbl:id}


| Function   | Signature                           |
|------------|-------------------------------------|
| ABBREV     | ABBREV(info, word, length)          |
| ADDRESS    | ADDRESS()                           |
| CONDITION  | CONDITION([info])                   |
| DIGITS     | DIGITS()                            |
| FORM       | FORM()                              |
| FUZZ       | FUZZ()                              |
| QUEUED     | QUEUED()                            |


Table: Non-SAA Functions. {#tbl:id}

## Level B text file I/O

The sequential file BIFs in `rxfnsb` are Level B UTF text functions. They
operate on `.string` values and validate text read from files according to the
normal Level B UTF-8 contract. They are not byte-oriented binary I/O BIFs.

| Function | Result | Notes |
|----------|--------|-------|
| `LINEIN(name)` | `.string` | Read one line from the named text stream, without the line terminator. |
| `LINEOUT(name [, string])` | `.int` | With `string`, write the text followed by a newline. Without `string`, close the named stream. |
| `CHARIN(name [, count])` | `.string` | Read up to `count` UTF codepoints from the named text stream; the default count is `1`. |
| `CHAROUT(name [, string])` | `.int` | With `string`, write text without appending a newline. Without `string`, close the named stream. |
| `LINES(name)` | `.int` | Return `1` when more text can be read from the stream, otherwise `0`. |

Future binary file BIFs should use `.binary` values and the VM byte I/O path.
Do not use these text BIFs for arbitrary byte payloads.

## Array helpers

Level B arrays use `array[0]` as the high-water mark. User elements are stored
in `array[1]` through `array[array[0]]`. The `array*` helpers below live in
`rxfnsb`; mutating helpers take the array by `expose` and update it in place.
They are the supported array surface for new code. The older native arrays
plugin is deprecated.

| Function | Result | Notes |
|----------|--------|-------|
| `ARRAYHI(array, mode, newhi)` | `.int` | Get the high-water mark, or shrink it with mode `SET`. |
| `ARRAYDROP(array)` | `.int` | Clear the array in place and return `0`. |
| `ARRAYINSERT(array, from, count, default)` | `.int` | Open a gap at `from`, fill it, and return the new high-water mark. |
| `ARRAYDELETE(array, from, count)` | `.int` | Delete a range and return the new high-water mark. |
| `ARRAYAPPEND(array, value, count)` | `.int` | Append `value` `count` times. |
| `ARRAYPREPEND(array, value, count)` | `.int` | Prepend `value` `count` times. |
| `ARRAYPOP(array, default)` | `.string` | Remove and return the last element, or `default` when empty. |
| `ARRAYSHIFT(array, default)` | `.string` | Remove and return the first element, or `default` when empty. |
| `ARRAYSET(array, index, value, fill)` | `.int` | Set an element; growing gaps are initialised with `fill`. |
| `ARRAYGET(array, index, default)` | `.string` | Return an element, or `default` for an out-of-range index. |
| `ARRAYCOPY(array, from, count)` | `.string[]` | Return a copied slice; negative `from` counts from the end. |
| `ARRAYMOVE(array, from, count, to)` | `.int` | Move a range within the same array. |
| `ARRAYREVERSE(array)` | `.int` | Reverse the array in place. |
| `ARRAYSORT(array, offset, order, debug)` | `.int` | Sort strings by a substring key. |
| `ARRAYFIND(find, array, from, case)` | `.int` | Find the first element containing a substring. |
| `ARRAYINDEXOF(array, value, from, case)` | `.int` | Find the first element equal to `value`. |
| `ARRAYCONTAINS(array, value, case)` | `.int` | Return `1` when an element equals `value`, else `0`. |
| `ARRAYJOIN(array, separator)` | `.string` | Join all elements with `separator`. |
| `ARRAYFORMAT(array, from, to, flags, hdr, prefix)` | `.string[]` | Return a formatted dump as an array. |
| `ARRAYDUMP(array, from, to, flags, hdr, prefix)` | `.int` | Print a formatted dump and return the printed count. |

Insert, delete, append, prepend, pop, shift, shrink, and clear operations use
the VM array attribute instructions, so the pointer array can be adjusted
without a Rexx-level per-element copy loop. Element values are still ordinary
Rexx strings and keep the usual copy and lifetime rules.

[^bif]: also colloqually referred to with the jargon-like expression BIFs.
