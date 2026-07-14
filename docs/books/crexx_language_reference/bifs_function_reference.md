# Built-in functions

These are built-in functions for cRexx strings and binary values. The `.string` type is central to the Rexx language and all its variants. This is true for cRexx and this implementation also contains the expected *built-in-functions*.[^bif] Level B also provides byte-oriented helpers for the `.binary` type.

Use of these functions needs import of the `rxfnsb` package:
```rexx
import rxfnsb
```


| BIF             | Signature                                    |
|-----------------|----------------------------------------------|
| ABS             | ABS(number)                                  |
| FORMAT          | FORMAT(number [,before [,after [,expp [,expt]]]]) |
| MAX             | MAX(number, ...)                             |
| MIN             | MIN(number, ...)                             |
| SIGN            | SIGN(number)                                 |
| TRUNC           | TRUNC(number [,digits])                      |
| B2X             | B2X(b)                                       |
| C2D             | C2D(s)                                       |
| C2X             | C2X(s)                                       |
| D2C             | D2C(number [,length])                        |
| D2X             | D2X(number [,length])                        |
| X2B             | X2B(x)                                       |
| X2BIN           | X2BIN(x)                                     |
| X2C             | X2C(x)                                       |
| X2D             | X2D(hexadecimal [,length])                   |
| XRANGE          | XRANGE([start [,end]])                       |
| CENTER          | CENTER(string, length [,pad])                |
| CENTRE          | CENTRE(string, length [,pad])                |
| CHANGESTR       | CHANGESTR(needle, haystack, replacement)     |
| COMPARE         | COMPARE(left, right [,pad])                  |
| CHARIN          | CHARIN(name, count)                          |
| CHAROUT         | CHAROUT(name, string)                        |
| COPIES          | COPIES(string, count)                        |
| COUNTSTR        | COUNTSTR(needle, haystack)                   |
| DELSTR          | DELSTR(string, start [,length])              |
| DELWORD         | DELWORD(s, start, n)                         |
| INSERT          | INSERT(new, target [,before [,length [,pad]]]) |
| JUSTIFY         | JUSTIFY(s, width, pad)                       |
| LEFT            | LEFT(string, length [,pad])                  |
| LENGTH          | LENGTH(string)                               |
| LINEIN          | LINEIN(name)                                 |
| LINEOUT         | LINEOUT(name, string)                        |
| LINES           | LINES(name)                                  |
| LOWER           | LOWER(string)                                |
| OVERLAY         | OVERLAY(new, target [,start [,length [,pad]]]) |
| POS             | POS(needle, haystack [,start])               |
| LASTPOS         | LASTPOS(needle, haystack [,start])           |
| RIGHT           | RIGHT(string, length [,pad])                 |
| REVERSE         | REVERSE(string)                              |
| SPACE           | SPACE(string [,count [,pad]])                |
| STRIP           | STRIP(string [,option [,char]])              |
| SUBSTR          | SUBSTR(string, start [,length [,pad]])       |
| SUBSTRO         | SUBSTRO(string, start [,length [,pad]])      |
| SUBWORD         | SUBWORD(s, start, n)                         |
| TRANSLATE       | TRANSLATE(s, new, old)                       |
| UPPER           | UPPER(string)                                |
| VERIFY          | VERIFY(string, reference [,option [,start]]) |
| WORD            | WORD(s, n)                                   |
| WORDINDEX       | WORDINDEX(s, n)                              |
| WORDLENGTH      | WORDLENGTH(s, n)                             |
| WORDS           | WORDS(s)                                     |
| DATATYPE        | DATATYPE(s, type)                            |
| RANDOM          | RANDOM(min, max, seed)                       |
| TIME            | TIME(option)                                 |
| DATE            | DATE(oformat, date, iformat, osep, isep)     |
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


| Function | Signature                  |
|----------|----------------------------|
| ARRAYAPPEND | ARRAYAPPEND(array, value [,count]) |
| ARRAYCONTAINS | ARRAYCONTAINS(array, value [,case]) |
| ARRAYCOPY | ARRAYCOPY(array [, from [, count]]) |
| ARRAYDELETE | ARRAYDELETE(array, from, count) |
| ARRAYDROP | ARRAYDROP(array) |
| ARRAYFIND | ARRAYFIND(needle, array [,from [,case_sensitive]]) |
| ARRAYGET | ARRAYGET(array, index [, default]) |
| ARRAYHI | ARRAYHI(array[, 'GET' | 'SET'[, new_hi]]) |
| ARRAYINDEXOF | ARRAYINDEXOF(array, value [, from [, case]]) |
| ARRAYINSERT | ARRAYINSERT(array, from, count[, default]) |
| ARRAYJOIN | ARRAYJOIN(array, [, separator]) |
| ARRAYMOVE | ARRAYMOVE(array, from, count, to) |
| ARRAYPOP | ARRAYPOP(array, [default]) |
| ARRAYPREPEND | ARRAYPREPEND(array, value [,count]) |
| ARRAYREVERSE | ARRAYREVERSE(array) |
| ARRAYSET | ARRAYSET(array, index, value [, fill]) | 
| ARRAYSHIFT | ARRAYSHIFT(array [,default]) |
| ARRAYSORT | ARRAYSORT(array [, offset] [, order] [,debug]) |
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
| BINAPPEND       | BINAPPEND(dst, src)                          |
| BINCLEAR        | BINCLEAR(binary)                             |
| BINCOPY         | BINCOPY(dst, dst_offset, src, src_offset, length) |
| BINDROP         | BINDROP(binary, offset, length)              |
| BINFILL         | BINFILL(binary, byte)                        |
| BINFILLAT       | BINFILLAT(binary, offset, length, byte)      |
| BINMAKEGAP      | BINMAKEGAP(binary, offset, length)           |
| BINMEMMOVE      | BINMEMMOVE(binary, dst_offset, src_offset, length) |
| BINRESIZE       | BINRESIZE(binary, length)                    |
| BINUPDATE       | BINUPDATE(binary, offset, src)               |
| QEXTRACTALL | QEXTRACTALL(open, close, text [, start [, mode]]) |
| QEXTRACTPAIR | QEXTRACTPAIR(open, close, text [, start [, mode]]) |
| QPOS | QPOS(needle, text [, start]) |
| QREMOVEALL | QREMOVEALL(open, close, text [, mode])   |
| QSPLIT | QSPLIT(text, sep) |
| QSPLITSAFE | QSPLITSAFE(text, sep [, start [, pairs]]) |
| QSTRIPCOMMENT | QSTRIPCOMMENT(open [, close], text) |
| QSUBWORD | QSUBWORD(string, wordnum [, count]) |
| QWORD | QWORD(line, wanted) |
| QWORDINDEX | QWORDINDEX(string,wordnum) |
| QWORDLENGTH | QWORDLENGTH(string,wordnum) |
| QWORDPOS | QWORDPOS(search, string [,start])  |
| QWORDS | QWORDS(string) |
| RERADIX | RERADIX(subject, fromradix, toradix) |
| SEQUENCE | SEQUENCE(from, to) |
| SPLICE | SPLICE(replacement, source, at, remove_length) |
| VERSION | VERSION() |


Table: cRexx additional functions. {#tbl:id}




[^bif]: also colloqually referred to with the jargon-like expression BIFs.

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

## Binary byte helpers

These Level B helpers operate on `.binary` byte buffers, not `.string`
codepoints. Positions are 1-based at the Rexx surface; byte values are integers
in the range `0..255`. Invalid text is never routed through these functions.

| Function | Result | Notes |
|----------|--------|-------|
| `BINLENGTH(data)` | `.int` | Byte length. |
| `BINBYTE(data, position)` | `.int` | Byte at position, or `-1` if out of range. |
| `BINSETBYTE(data, position, byte)` | `.binary` | Copy with one byte replaced; invalid byte/position raises `OUT_OF_RANGE`. |
| `BINSUBSTR(data, start, length)` | `.binary` | Byte slice; omitted/negative length means to the end. |
| `BINCONCAT(left, right)` | `.binary` | Byte concatenation. The `||` operator does the same when either operand is `.binary`. |
| `BINOVERLAY(new, target, start)` | `.binary` | Fixed-size byte overlay; writes past target raise `OUT_OF_RANGE`. |
| `BININSERT(new, target, before)` | `.binary` | Insert before the 1-based byte position; past the end appends. |
| `BINDELSTR(target, start, length)` | `.binary` | Delete a byte range; length `0` deletes to the end. |
| `BINPOS(needle, haystack, start)` | `.int` | 1-based byte search, `0` when not found. |
| `BINCOMPARE(left, right)` | `.int` | `0` if equal, otherwise first differing byte position. |
| `BIN2X(data)` | `.string` | Uppercase hexadecimal text for the bytes. |
| `X2BIN(hex)` | `.binary` | Hex text to bytes; blanks are ignored and an odd nibble is left-padded with `0`. |

The `||` operator also performs byte concatenation when either operand is
`.binary`. In that case the result is `.binary`; a `.string` operand is copied
as its exact UTF-8 bytes. Blank concatenation remains a text operation and is
not for binary payload construction.

## Packed binary memory helpers

The helpers in this section are the Release 1 packed-memory helper surface. They
are distinct from the older `BIN*` byte helpers above:

- packed-memory helpers use zero-based byte offsets;
- mutating helpers update the first binary argument through `arg expose`;
- mutating helpers return the new logical byte length unless stated otherwise;
- the compiler or inliner may lower selected helpers directly to RXAS;
- use binary-memory intrinsics, not helpers, for direct reads from binary
  constants and for zero-copy compare.

Implementation note: Release 1 provides these helpers in `rxfnsb`. Some helpers
currently use conservative library code and may be direct-lowered by the
compiler or inliner later.

| Function | Result | Notes |
|----------|--------|-------|
| `BINRESIZE(data, length)` | `.int` | Resize `data` to `length` bytes. Existing bytes are preserved and growth is zero-filled. |
| `BINCLEAR(data)` | `.int` | Clear `data` to length `0`; returns `0`. |
| `BINFILL(data, byte)` | `.int` | Fill the whole current logical byte range with `byte`; returns the byte length. |
| `BINFILLAT(data, offset, length, byte)` | `.int` | Fill a zero-based byte span with `byte`; returns the byte length. |
| `BINCOPY(dst, dst_offset, src, src_offset, length)` | `.int` | Copy `length` bytes from `src` to `dst`. Use `BINMEMMOVE` for overlapping ranges in the same binary value. |
| `BINMEMMOVE(data, dst_offset, src_offset, length)` | `.int` | Move `length` bytes within one binary value. Overlapping ranges are safe. |
| `BINAPPEND(dst, src)` | `.int` | Append all bytes from `src` to `dst`; returns the new byte length. |
| `BINUPDATE(dst, offset, src)` | `.int` | Overlay all bytes from `src` into `dst` at zero-based `offset`; the write must fit. |
| `BINMAKEGAP(data, offset, length)` | `.int` | Open a zero-filled gap of `length` bytes at `offset`; returns the new byte length. |
| `BINDROP(data, offset, length)` | `.int` | Delete `length` bytes at `offset`; returns the new byte length. |

Examples:

```rexx
call binresize page, 4096
call binfill page, 0
call binmemmove page, dst_offset, src_offset, span_len
call bincopy target, 0, source, source_offset, span_len
```

Packed-memory helper offsets are intentionally zero-based so they match
`<at..type>` and the RXAS binary-memory instructions. The older
`BINBYTE`/`BINSUBSTR`/`BINOVERLAY` compatibility helpers remain 1-based and
copy-returning.

Invalid packed-memory spans, negative lengths, and byte values outside
`0..255` raise `OUT_OF_RANGE`. Zero-length spans accept offsets from zero
through the current logical byte length, inclusive. The complete Level B
contract and signal examples are in
[`binary.md`](../../../lib/rxfnsb/rexx/binary.md).

## Array helpers

Level B arrays use `array[0]` as the high-water mark. User elements are stored
in `array[1]` through `array[array[0]]`. The `array*` helpers below live in
`rxfnsb`; mutating helpers take the array by `expose` and update it in place.
<!--  They are the supported array surface for new code. The older native arrays -->
<!-- plugin is deprecated. -->

These helpers operate on `.string[]` arrays. They are not generic array helpers
and are not the supported surface for typed numeric arrays such as `.int[]`;
use direct indexing for those arrays until typed helpers are added.

| Function | Result | Notes |
|----------|--------|-------|
| `ARRAYHI(array, mode, newhi)` | `.int` | Get the high-water mark, or shrink it with mode `SET`. |
| `ARRAYDROP(array)` | `.int` | Clear the array in place and return `0`. |
| `ARRAYINSERT(array, from, count [,default])` | `.int` | Open a gap at `from`, fill it, and return the new high-water mark. |
| `OBJECTARRAYINSERT(array, from, count, value)` | `.int` | Open an object-array gap and fill it with object-value copies. |
| `OBJECTARRAYDELETE(array, from, count)` | `.int` | Delete an object-array range and return the new high-water mark. |
| `OBJECTARRAYAPPEND(array, value [,count])` | `.int` | Append object-value copies and return the new high-water mark. |
| `OBJECTARRAYPREPEND(array, value [,count])` | `.int` | Prepend object-value copies and return the new high-water mark. |
| `OBJECTARRAYDROP(array)` | `.int` | Clear an object array in place and return zero. |
| `OBJECTARRAYMOVE(array, from, count, to)` | `.int` | Move an object-array block and preserve its high-water mark. |
| `ARRAYDELETE(array, from, count)` | `.int` | Delete a range and return the new high-water mark. |
| `ARRAYAPPEND(array, value [,count])` | `.int` | Append `value` `count` times. |
| `ARRAYPREPEND(array, value [,count])` | `.int` | Prepend `value` `count` times. |
| `ARRAYPOP(array, default)` | `.string` | Remove and return the last element, or `default` when empty. |
| `ARRAYSHIFT(array, default)` | `.string` | Remove and return the first element, or `default` when empty. |
| `ARRAYSET(array, index, value, fill)` | `.int` | Set an element; growing gaps are initialised with `fill`. |
| `ARRAYGET(array, index, default)` | `.string` | Return an element, or `default` for an out-of-range index. |
| `ARRAYCOPY(array, from, count)` | `.string[]` | Return a copied slice; negative `from` counts from the end. |
| `ARRAYMOVE(array, from, count, to)` | `.int` | Move a range within the same array. |
| `ARRAYREVERSE(array)` | `.int` | Reverse the array in place. |
| `ARRAYSORT(array, offset, order, debug)` | `.int` | Sort strings by a substring key. |
| `ARRAYFIND(needle, array [,from [,case_sensitive]])` | `.int` | Find the first element containing a substring. |
| `ARRAYINDEXOF(array, value, from, case)` | `.int` | Find the first element equal to `value`. |
| `ARRAYCONTAINS(array, value, case)` | `.int` | Return `1` when an element equals `value`, else `0`. |
| `ARRAYJOIN(array, separator)` | `.string` | Join all elements with `separator`. |

Insert, delete, append, prepend, pop, shift, shrink, and clear operations use
the VM array attribute instructions, so the pointer array can be adjusted
without a Rexx-level per-element copy loop. Element values are still ordinary
Rexx strings and keep the usual copy and lifetime rules.

## Quote-aware helpers

The `q*` helpers share one positional Unicode scanner. They are ordinary Level B
`rxfnsb` functions, not Classic Level C BIFs. Single and double ASCII quotes can
occur anywhere in a word, matching doubled quotes are escapes, quote delimiters
remain in returned values, and malformed quote or pair grammar signals
`INVALID_ARGUMENTS`. Positions and lengths are Unicode codepoints; word
separation uses Unicode 17.0 `White_Space`.

`QPOS(needle, text [, start])` returns the 1-based position of `needle` outside
single- or double-quoted regions, or `0` when it is not found. `start` is a
positive codepoint position.

`QSPLIT(text, sep)` splits `text` on `sep` only when the separator is outside
quoted regions. It returns exact `.string[]` fields, including empty and
trailing fields, without stripping source whitespace.

`QSPLITSAFE(text, sep, start, pairs)` is the nested-safe splitter. In addition
to quote tracking, it tracks nested one-codepoint delimiter pairs from `pairs`,
such as the default `()`, and only splits at depth zero.

`QEXTRACTPAIR` and `QEXTRACTALL` return balanced top-level source spans. Modes
`X`/`E` return the contents and `I`/`C` include delimiters. `QREMOVEALL` uses the
same spans; its default inclusive mode removes the complete regions, while
exclusive mode retains the delimiters. Equal text outside a selected region is
never removed.

`QSTRIPCOMMENT` removes line comments when `close` is omitted or empty and
preserves CRLF, LF, and CR endings exactly. With a closer it removes nested
balanced block comments.

`QWORD`, `QWORDINDEX`, `QWORDLENGTH`, and `QWORDS` use the same word spans.
`QWORDPOS` matches an exact word sequence. `QSUBWORD` preserves separators
inside the selected source span; omitted `count` selects through the last word
and explicit zero returns an empty string. The selector-local pages under
`lib/rxfnsb/rexx` contain the complete signatures and examples.


## ABS(number)

returns the absolute value of *string*, which must be a
number.
 Any sign is removed from the number, and it is then formatted by adding
zero with a digits setting that is either nine or, if greater, the
number of digits in the mantissa of the number (excluding leading
insignificant zeros).
Scientific notation is used, if necessary.

The native Level B function accepts and returns `.decimal`. Its invalid dynamic
conversion signal is currently tracked as a VM dependency in the programme
worklist. The standalone Level C BIF accepts Classic numeric text through
`rNUM`, including the blank-separated leading-sign forms below, and reports
standard `RXC-LC-40.*` context errors. See the separate
[Level B ABS](../../../lib/rxfnsb/rexx/abs.md) and
[Level C ABS](../../../lib/rxfnsc/abs.md) pages for their distinct contracts.

**Examples:**
```
ABS('12.3')              == 12.3
ABS(' -0.307')           == 0.307
ABS('123.45E+16')        == 1.2345E+18
ABS('- 1234567.7654321') == 1234567.7654321
```



## FORMAT(number [,before [,after [,expp [,expt]]]])

`FORMAT` lays out a numeric value under the numeric settings current at the
invocation. With only `number`, it returns that normalized number.

`before` and `after` are optional non-negative whole numbers. `before` is the
width of the integer part, including a minus sign, and left-pads with blanks.
If the integer part cannot fit, error 40.38 results. `after` is the exact number
of digits after the decimal point: missing digits are zero-filled and excess
digits are rounded half up. A supplied zero removes the decimal point.

`expp` is the exponent digit width. If it is too small, error 40.38 results; if
the exponent is zero, a supplied width produces `expp + 2` blanks. A supplied
zero suppresses exponential notation. `expt` controls when exponent form is
used; when `expp` is present and `expt` is omitted, the current `NUMERIC DIGITS`
setting is the trigger. The current `NUMERIC FORM` setting selects scientific
or engineering layout. `FORMAT` has five arguments; form is not a sixth
argument.

```rexx
FORMAT(' - 12.73')          == '-12.73'
FORMAT('1.73', 4, 0)        == '   2'
FORMAT('-.76', 4, 1)        == '  -0.8'
FORMAT('12345.73',,,2,2)    == '1.234573E+04'
FORMAT('1.2345',,3,2,0)     == '1.235    '
```

See the separate [Level C BIF contract](../../../lib/rxfnsc/format.md) and
[native Level B typed API](../../../lib/rxfnsb/rexx/format.md).



## MAX(number, ...)

returns the larger of *string* and *number*, which
must both be numbers.  If they compare equal (that is, when subtracted,
the result is 0), then *string* is selected for the result.

The comparison is effected using a numerical comparison with a digits
setting that is either nine or, if greater, the larger of the number of
digits in the mantissas of the two numbers (excluding leading
insignificant zeros).

The selected result is formatted by adding zero to the selected number
with a digits setting that is either nine or, if greater, the number of
digits in the mantissa of the number (excluding leading insignificant
zeros).
Scientific notation is used, if necessary.

The native Level B function accepts and returns `.decimal` and performs one
linear scan. The standalone Level C BIF accepts Classic variadic `rNUM...`
text, preserving the first selected normalized argument representation. See
the separate [Level B MAX](../../../lib/rxfnsb/rexx/max.md) and
[Level C MAX](../../../lib/rxfnsc/max.md) pages for their distinct contracts.

**Examples:**
```
MAX(0, 1)          ==1
MAX('-1', 1)       ==1
MAX('+1', -1)      ==1
MAX('1.0', 1.00)   =='1.0'
MAX('1.00', 1.0)   =='1.00'
MAX('123456700000', 1234567E+5)   == '123456700000'
MAX('1234567E+5', '123456700000') == '1.234567E+11'
```



## MIN(number, ...)

returns the smaller of *string* and *number*, which
must both be numbers.  If they compare equal (that is, when subtracted,
the result is 0), then *string* is selected for the result.

The comparison is effected using a numerical comparison with a digits
setting that is either nine or, if greater, the larger of the number of
digits in the mantissas of the two numbers (excluding leading
insignificant zeros).

The selected result is formatted by adding zero to the selected number
with a digits setting that is either nine or, if greater, the number of
digits in the mantissa of the number (excluding leading insignificant
zeros).
Scientific notation is used, if necessary.

The native Level B function accepts and returns `.decimal` and performs one
linear scan. The standalone Level C BIF accepts Classic variadic `rNUM...`
text, preserving the first selected normalized argument representation. See
the separate [Level B MIN](../../../lib/rxfnsb/rexx/min.md) and
[Level C MIN](../../../lib/rxfnsc/min.md) pages for their distinct contracts.

**Examples:**
```
MIN(0, 1)          ==0
MIN('-1', 1)       =='-1'
MIN('+1', -1)      =='-1'
MIN('1.0', 1.00)   =='1.0'
MIN('1.00', 1.0)   =='1.00'
MIN('123456700000', 1234567E+5)   == '123456700000'
MIN('1234567E+5', '123456700000') == '1.234567E+11'
```



## SIGN(number)

returns a number that indicates the sign of *string*, which
must be a number.
*string* is first formatted, just as though the operation
"**string+0**" had been carried out with sufficient digits
to avoid rounding.
If the number then starts with **'-'** then **'-1'** is
returned; if it is **'0'** then **'0'** is returned; and
otherwise **'1'** is returned.

**Examples:**
```
SIGN('12.3')    ==  1
SIGN('0.0')     ==  0
SIGN(' -0.307') == -1
```

The native Level B helper accepts a `.decimal` and returns `.int`, preserving
sign outside binary floating-point range. The standalone Level C BIF accepts
Classic `rNUM` text and returns a RexxValue integer with standard
`RXC-LC-40.*` errors. See the separate [Level B SIGN](../../../lib/rxfnsb/rexx/sign.md)
and [Level C SIGN](../../../lib/rxfnsc/sign.md) pages for their distinct
contracts.



## TRUNC(number [,digits])

returns the integer part of *string*, which must be a
number, with *n* decimal places (digits after the decimal
point).
*n* must be a non-negative whole number, and defaults to zero.

The number *string* is formatted by adding zero with a digits
setting that is either nine or, if greater, the number of digits in the
mantissa of the number (excluding leading insignificant zeros).
It is then truncated to *n* decimal places (or trailing zeros
are added if needed to make up the specified length).
If *n* is 0 (the default) then an integer with no decimal
point is returned.
The result will never be in exponential form.

**Examples:**
```
TRUNC('12.3')         == 12
TRUNC('127.09782', 3) == 127.097
TRUNC('127.1', 3)     == 127.100
TRUNC('127', 2)       == 127.00
TRUNC('0', 2)         == 0.00
```

The native Level B helper accepts `.decimal` plus a non-negative `.int` and
signals `INVALID_ARGUMENTS` for a negative digit count. The standalone Level C
BIF accepts Classic `rNUM` and optional `oWHOLE>=0` RexxValue text with standard
`RXC-LC-40.*` errors. Both avoid binary floating point. See the separate
[Level B TRUNC](../../../lib/rxfnsb/rexx/trunc.md) and
[Level C TRUNC](../../../lib/rxfnsc/trunc.md) pages for their distinct
contracts and test scope.



## B2X(b)

Binary to hexadecimal.
Converts *string*, a string of zero or more binary
(**0** and/or **1**) digits, to an equivalent string of
hexadecimal characters.
The returned string will use uppercase Roman letters for the values A-F,
and will not include any blanks.
 If the number of binary digits in the string is not a multiple of four,
then up to three **'0'** digits will be added on the left before
conversion to make a total that is a multiple of four.

The empty string returns the empty string. Interior blanks may separate groups
only when a multiple of four binary digits is to their right. Leading/trailing
blanks, misplaced blanks, and other characters are invalid binary strings.

**Examples:**
```
B2X('11000011')  == 'C3'
B2X('10111')     == '17'
B2X('0101')      == '5'
B2X('101')       == '5'
B2X('111110000') == '1F0'
```

See the separate [Level C BIF contract](../../../lib/rxfnsc/b2x.md) and
[native Level B API](../../../lib/rxfnsb/rexx/b2x.md).


## BINLENGTH(data)

Binary byte length.
Returns the number of bytes stored in `.binary` value *data*.
This is a byte count, not a UTF-8 codepoint count.

**Examples:**
```rexx
BINLENGTH("ff0041"x as .binary) == 3
empty = .binary
BINLENGTH(empty)                == 0
BINLENGTH("α" as .binary)       == 2
```


## BINBYTE(data, position)

Binary byte lookup.
Returns the byte at 1-based byte *position* in `.binary` value *data* as an
integer in the range `0..255`. If *position* is outside the byte buffer, `-1`
is returned.

**Examples:**
```rexx
BINBYTE("ff0041"x as .binary, 1) == 255
BINBYTE("ff0041"x as .binary, 3) == 65
BINBYTE("ff0041"x as .binary, 4) == -1
```


## BINSETBYTE(data, position, byte)

Binary byte replacement.
Returns a copy of `.binary` value *data* with the byte at 1-based byte
*position* replaced by *byte*. *byte* must be in the range `0..255`; invalid
positions or byte values raise `OUT_OF_RANGE`.

**Examples:**
```rexx
BIN2X(BINSETBYTE("001122"x as .binary, 2, 255)) == "00FF22"
```


## BINSUBSTR(data, start, length)

Binary substring.
Returns a `.binary` byte slice from `.binary` value *data*, starting at 1-based
byte position *start*. If *length* is omitted or negative, the slice continues
to the end of the buffer. If the requested range extends past the end, the
result is truncated at the end of *data*. A nonpositive *start* raises
`INVALID_ARGUMENTS`.

**Examples:**
```rexx
BIN2X(BINSUBSTR("001122ff"x as .binary, 2, 2)) == "1122"
BIN2X(BINSUBSTR("001122ff"x as .binary, 3))    == "22FF"
BIN2X(BINSUBSTR("001122ff"x as .binary, 9))    == ""
```


## BINCONCAT(left, right)

Binary concatenation.
Returns the byte concatenation of `.binary` values *left* and *right*. The
source-level `||` operator performs the same byte concatenation when either
operand is `.binary`.

**Examples:**
```rexx
BIN2X(BINCONCAT("0011"x as .binary, "22ff"x as .binary)) == "001122FF"
BIN2X(("ff"x as .binary) || "A")                         == "FF41"
```


## BINOVERLAY(new, target, start)

Binary overlay.
Returns a copy of `.binary` value *target* with the bytes from `.binary` value
*new* overlaid starting at 1-based byte position *start*. The overlay is
fixed-size: it must fit inside *target*, or `OUT_OF_RANGE` is raised.

**Examples:**
```rexx
BIN2X(BINOVERLAY("abcd"x as .binary, "001122ff"x as .binary, 2)) == "00ABCDFF"
```


## BININSERT(new, target, before)

Binary insert.
Returns a copy of `.binary` value *target* with `.binary` value *new* inserted
before 1-based byte position *before*. If *before* is less than or equal to 1,
*new* is prepended. If *before* is beyond the end of *target*, *new* is
appended.

**Examples:**
```rexx
BIN2X(BININSERT("abcd"x as .binary, "001122ff"x as .binary, 3))  == "0011ABCD22FF"
BIN2X(BININSERT("abcd"x as .binary, "001122ff"x as .binary, 99)) == "001122FFABCD"
```


## BINDELSTR(target, start, length)

Binary delete substring.
Returns a copy of `.binary` value *target* with a byte range removed. Deletion
starts at 1-based byte position *start*. If *length* is omitted or `0`, bytes
from *start* to the end are removed. A nonpositive *start* or negative *length*
raises `INVALID_ARGUMENTS`.

**Examples:**
```rexx
BIN2X(BINDELSTR("001122ff"x as .binary, 2, 2)) == "00FF"
BIN2X(BINDELSTR("001122ff"x as .binary, 3))    == "0011"
```


## BINPOS(needle, haystack, start)

Binary position.
Searches `.binary` value *haystack* for `.binary` value *needle*, starting at
1-based byte position *start* (default `1`). Returns the 1-based byte position
of the first match, or `0` if no match is found. A zero-length *needle* returns
`0`. A nonpositive *start* raises `INVALID_ARGUMENTS`.

**Examples:**
```rexx
BINPOS("1122"x as .binary, "001122ff"x as .binary)    == 2
BINPOS("22"x as .binary, "001122ff"x as .binary, 3)   == 3
BINPOS("33"x as .binary, "001122ff"x as .binary)      == 0
```


## BINCOMPARE(left, right)

Binary compare.
Compares two `.binary` values byte by byte. Returns `0` when they are equal.
Otherwise, returns the 1-based position of the first differing byte. If one
value is a prefix of the other, the first differing position is one past the
shorter value.

**Examples:**
```rexx
BINCOMPARE("001122ff"x as .binary, "001122ff"x as .binary) == 0
BINCOMPARE("001122ff"x as .binary, "001123ff"x as .binary) == 3
BINCOMPARE("001122ff"x as .binary, "001122"x as .binary)   == 4
```


## BIN2X(data)

Binary bytes to hexadecimal.
Converts `.binary` value *data* to uppercase hexadecimal text. Each byte becomes
two hexadecimal characters, so the output length is always twice the input byte
length.

**Examples:**
```rexx
BIN2X("ff0041"x as .binary) == "FF0041"
empty = .binary
BIN2X(empty)                == ""
BIN2X("α" as .binary)       == "CEB1"
```


## X2BIN(hex)

Hexadecimal to binary bytes.
Converts hexadecimal text *hex* to a `.binary` byte buffer. Blanks are ignored.
If there is an odd number of hexadecimal digits, a leading `0` nibble is
assumed. A non-blank character that is not hexadecimal raises
`INVALID_ARGUMENTS`.

**Examples:**
```rexx
BIN2X(X2BIN("ff 00 aa")) == "FF00AA"
BIN2X(X2BIN("f"))        == "0F"
```

See the stable [Level B binary module contract](../../../lib/rxfnsb/rexx/binary.md).



## C2D(s)

Coded character to decimal.
The native Level B helper converts the Unicode code point of *string*, which
must contain exactly one character, to a non-negative `.int`. Empty or
multi-character input raises `CONVERSION_ERROR`.

**Examples:**
```
C2D('M')  == 77
C2D('α')  == 945
C2D('🔥') == 128293
C2D('00'x) == 0
```
 The  **c2x** function %% (see page refc2x)
 can be used to
convert the encoding of a character to a hexadecimal representation.

Classic Level C C2D is a different API with an optional signed-width argument.
See the separate [Level C BIF contract](../../../lib/rxfnsc/c2d.md) and
[native Level B API](../../../lib/rxfnsb/rexx/c2d.md).



## C2X(s)

Coded characters to hexadecimal.
Converts every character in *string* to its established two-digit hexadecimal
representation. The returned string uses uppercase Roman letters for A-F and
contains no inserted blanks. The empty string returns the empty string,
multi-character input is valid, and leading zero digits are retained.

The current Level B implementation preserves RXAS `hexchar` behavior for
Unicode: a code point beyond the single-byte range contributes its low eight
bits. Classic Level C C2X instead converts the exact coded bytes selected by
the call context's BYTE or UTF8 profile.

**Examples:**
```
C2X('M')     == '4D'
C2X('72s')   == '373273' -- ASCII/Unicode build
C2X('0123'x) == '0123'
C2X('')      == ''
```
 The  **c2d** function %% (see page refc2d)
 can be used to
convert the encoding of a character to a decimal number.

See the separate [Level C BIF contract](../../../lib/rxfnsc/c2x.md) and
[native Level B API](../../../lib/rxfnsb/rexx/c2x.md).



## D2C(number [,length])

Classic Level C D2C converts a decimal whole number to
configuration-coded characters. Without *length*, the number must be
non-negative and the result uses the minimum encoded width. With a
non-negative *length*, negative numbers use twos-complement and the result is
padded or truncated to exactly that many coded characters. The configuration,
not Unicode code-point numbering, defines those characters.

Level B deliberately provides a different typed helper:
`d2c(codepoint=.int [,output_length=.int])`. It emits one Unicode scalar value;
its explicit output length may only be zero or one.

See the separate [Level C BIF contract](../../../lib/rxfnsc/d2c.md) and
[native Level B API](../../../lib/rxfnsb/rexx/d2c.md). The direct Level C
implementation returns exact bytes and records valid UTF-8 text when applicable.



## D2X(number [,length])

Classic Level C decimal to hexadecimal.
Returns a string of hexadecimal characters of length as needed or of
length *n*, which is the hexadecimal (unpacked) representation
of the decimal number.  The returned string will use uppercase
Roman letters for the values A-F, and will not include any blanks.
 *string* must be a whole number, and must be non-negative
unless *n* is specified, or an error will result.
If *n* is not specified, the length of the result returned is
such that there are no leading 0 characters, unless *string*
was equal to 0 (in which case **'0'** is returned).

If *n* is specified it is the length of the final result in
characters; that is, after conversion the input string will be
sign-extended to the required length (negative numbers are converted
assuming twos-complement form).
If the number is too big to fit into *n* characters, it will be
truncated on the left.
*n* must be a non-negative whole number.

**Examples:**
```
D2X('9')       == '9'
D2X('129')     == '81'
D2X('129', 1)  == '1'
D2X('129', 2)  == '81'
D2X('127', 3)  == '07F'
D2X('129', 4)  == '0081'
D2X('257', 2)  == '01'
D2X('-127', 2) == '81'
D2X('-127', 4) == 'FF81'
D2X('12', 0)   == ''
```

The standalone Level C implementation accepts caller-context Rexx whole
numbers without narrowing them to a native integer. Level B provides a
separate signed-64-bit typed helper with the same result rules over its smaller
numeric domain.

See the separate [Level C BIF contract](../../../lib/rxfnsc/d2x.md) and
[native Level B API](../../../lib/rxfnsb/rexx/d2x.md).



## X2B(hexadecimal)

Hexadecimal to binary. Converts every hexadecimal digit to four binary digits;
letters are case-insensitive and leading zero nibbles are retained. The empty
string returns empty.

Interior blanks are ignored only when an even number of hexadecimal digits lies
to their right. Leading/trailing blanks, mis-grouped blanks, or invalid
characters are errors. The result contains no blanks and its length is four
times the number of input digits.

**Examples:**
```
X2B('C3')  == '11000011'
X2B('7')   == '0111'
X2B('1 C1') == '000111000001'
X2B('0001') == '0000000000000001'
X2B('') == ''
```

See the separate [Level C BIF contract](../../../lib/rxfnsc/x2b.md) and
[native Level B API](../../../lib/rxfnsb/rexx/x2b.md).



## X2C(hexadecimal)

Classic Level C X2C validates hexadecimal text, removes valid interior
grouping blanks, left-pads an odd leading nibble, and converts each full byte
through the implementation's configured coded-character encoding. Empty input
returns empty and encoded leading zero bytes are retained.

Hexadecimal letters are case-insensitive. Leading/trailing blanks,
mis-grouped blanks, and non-hexadecimal characters are errors. Because the
character encoding is configured, the character produced by a byte such as
`4D` is not portable across ASCII/Unicode and EBCDIC configurations.

Level B deliberately provides a different typed helper that maps every parsed
byte to Unicode U+0000 through U+00FF.

See the separate [Level C BIF contract](../../../lib/rxfnsc/x2c.md) and
[native Level B API](../../../lib/rxfnsb/rexx/x2c.md). Direct Level C X2C
preserves exact configured bytes and records a text view only for valid UTF-8.



## X2D(hexadecimal [,length])

Hexadecimal to decimal.
Converts the *string* (a string of hexadecimal characters) to
a decimal number, without rounding.
If *string* is the null string, 0 is returned.

If *n* is not specified, *string* is taken to
be an unsigned number.

**Examples:**
```
X2D('0E')    == 14
X2D('81')    == 129
X2D('F81')   == 3969
X2D('FF81')  == 65409
X2D('c6f0')  == 50928
```

If *n* is specified, *string* is taken as a signed
number expressed in *n* hexadecimal characters.
If the most significant (left-most) bit is zero then the number is
positive; otherwise it is a negative number in twos-complement form.
In both cases it is converted to a CREXX number which may,
therefore, be negative.
If *n* is 0, 0 is always returned.

If necessary, *string* is padded on the left
with **'0'** characters (note, not "sign-extended"), or
truncated on the left, to length *n* characters; (that is, as
though *string***.right(***n*, **'0')**
had been executed.)

**Examples:**
```
X2D('81', 2)   == -127
X2D('81', 4)   == 129
X2D('F081', 4) == -3967
X2D('F081', 3) == 129
X2D('F081', 2) == -127
X2D('F081', 1) == 1
X2D('0031', 0) == 0
```
 The  **c2d** function %% (see page refc2d)
 can be used to convert
a character to a decimal representation of its encoding.

Classic Level C accepts standard grouped hexadecimal text and returns an exact
Rexx whole number subject to the caller's `NUMERIC DIGITS`. Invalid grouping or
digits report `40.25`; invalid lengths use the standard `40.12`/`40.13`
errors, and a result too large for the current digits reports `40.35`.

The typed Level B helper instead returns a native signed `.int`. It signals
`INVALID_ARGUMENTS` for invalid text or length and `OVERFLOW_UNDERFLOW` when
the selected result is outside the signed-64-bit range. See the separate
[Level C contract](../../../lib/rxfnsc/x2d.md) and
[Level B API](../../../lib/rxfnsb/rexx/x2d.md).



## CENTER(string, length [,pad])

Returns a string of length *length* with *string*
centered in it, with *pad* characters added as necessary to
make up the required length.
*length* must be a non-negative whole number.
The default *pad* character is blank, and an explicit *pad* must contain
exactly one character.
If the string is longer than *length*, it will be truncated at
both ends to fit.
If an odd number of characters are truncated or added, the right hand
end loses or gains one more character than the left hand end.

The Level B helper types *length* as `.int`; an invalid length or pad signals
`INVALID_ARGUMENTS`. The standalone Level C BIF accepts Classic whole-number
text and reports the standard `RXC-LC-40.*` context errors. Level B measures
Unicode codepoints. Level C measures configured units: octets in the default
BYTE profile and Unicode codepoints in the opt-in UTF8 profile. See the separate
[Level B CENTER](../../../lib/rxfnsb/rexx/center.md) and
[Level C CENTER](../../../lib/rxfnsc/center.md) pages for their distinct
contracts.

**Examples:**
```
CENTER('ABC', 7)          == '  ABC  '
CENTER('ABC', 8, '-')     == '--ABC---'
CENTER('The blue sky', 8) == 'e blue s'
CENTER('The blue sky', 7) == 'e blue '
```
**Note:** This function may be called either **centre** or **center**,
which avoids difficulties due to the difference between the British and
American spellings.



## CENTRE(string, length [,pad])

Returns a string of length *length* with *string*
centered in it, with *pad* characters added as necessary to
make up the required length.
*length* must be a non-negative whole number.
The default *pad* character is blank, and an explicit *pad* must contain
exactly one character.
If the string is longer than *length*, it will be truncated at
both ends to fit.
If an odd number of characters are truncated or added, the right hand
end loses or gains one more character than the left hand end.

The Level B helper types *length* as `.int`; an invalid length or pad signals
`INVALID_ARGUMENTS`. The standalone Level C BIF accepts Classic whole-number
text and reports the standard `RXC-LC-40.*` context errors. Level B measures
Unicode codepoints. Level C measures configured units: octets in the default
BYTE profile and Unicode codepoints in the opt-in UTF8 profile. See the separate
[Level B CENTRE](../../../lib/rxfnsb/rexx/centre.md) and
[Level C CENTRE](../../../lib/rxfnsc/centre.md) pages for their distinct
contracts.

**Examples:**
```
CENTRE('ABC', 7)          == '  ABC  '
CENTRE('ABC', 8, '-')     == '--ABC---'
CENTRE('The blue sky', 8) == 'e blue s'
CENTRE('The blue sky', 7) == 'e blue '
```
**Note:** This function may be called either **centre** or **center**,
which avoids difficulties due to the difference between the British and
American spellings.



## CHANGESTR(needle, haystack, replacement)

Returns a copy of *haystack* in which every case-sensitive,
non-overlapping occurrence of *needle* is replaced by *replacement*.
Searching proceeds from left to right through the original haystack; inserted
replacement text is not searched again. If *needle* is null or is not found,
the haystack is returned unchanged. A null replacement deletes each match.

The Level B helper requires three `.string` arguments. The standalone Level C
BIF accepts any three RexxValue texts and reports standard `RXC-LC-40.*`
argument-presence errors. Level B finds codepoint-aligned text matches. Level C
uses exact octet matches in BYTE and codepoint-aligned matches in UTF8. See the
separate
[Level B CHANGESTR](../../../lib/rxfnsb/rexx/changestr.md) and
[Level C CHANGESTR](../../../lib/rxfnsc/changestr.md) pages for their distinct
contracts and implementation notes.

**Examples:**
```
CHANGESTR('the', 'the cat and the dog', 'a') == 'a cat and a dog'
CHANGESTR('aa', 'aaaaa', 'X')                == 'XXa'
CHANGESTR('a', 'banana', '')                 == 'bnn'
CHANGESTR('', 'unchanged', '!')              == 'unchanged'
```



## COMPARE(left, right [,pad])

Returns `0` when *left* and *right* compare equal. Otherwise it returns the
first 1-based character position at which they differ. The shorter string is
conceptually padded on the right before comparison. *pad* defaults to blank and
must contain exactly one character.

The Level B helper takes two `.string` arguments, returns `.int`, and signals
`INVALID_ARGUMENTS` for an invalid pad. The standalone Level C BIF accepts
RexxValue text and reports standard `RXC-LC-40.*` context errors. Its result
position and one-unit pad use octets in BYTE and Unicode codepoints in UTF8;
Level B always uses codepoints. See the
separate [Level B COMPARE](../../../lib/rxfnsb/rexx/compare.md) and
[Level C COMPARE](../../../lib/rxfnsc/compare.md) pages.

**Examples:**
```
COMPARE('abc', 'abc')        == 0
COMPARE('abc', 'ak')         == 2
COMPARE('ab ', 'ab')         == 0
COMPARE('ab-- ', 'ab', '-')  == 5
```



## COPIES(string, count)

Returns *count* directly concatenated copies of *string*.
*count* must be a non-negative whole number; zero returns the null string.

The Level B helper types *count* as `.int` and signals `INVALID_ARGUMENTS` for
a negative value. The standalone Level C BIF accepts Classic whole-number text
and reports standard `RXC-LC-40.*` errors. Level C repeats the exact RexxValue
bytes and preserves a BYTE result as binary-authoritative; Level B repeats valid
UTF-8 `.string` text. See the separate
[Level B COPIES](../../../lib/rxfnsb/rexx/copies.md) and
[Level C COPIES](../../../lib/rxfnsc/copies.md) pages for their contracts and
performance notes.

**Examples:**
```
COPIES('abc', 3) == 'abcabcabc'
COPIES('abc', 0) == ''
COPIES('', 2)    == ''
```



## COUNTSTR(needle, haystack)

Returns the number of case-sensitive, non-overlapping occurrences of *needle*
in *haystack*, searching from left to right. A null, absent, or oversized
needle returns `0`.

The Level B helper takes two `.string` arguments and returns `.int`. The
standalone Level C BIF accepts any two RexxValue texts and reports standard
`RXC-LC-40.*` argument-presence errors. Level B matches at codepoint boundaries;
Level C matches exact octets in BYTE and codepoint-aligned text in UTF8. See the
separate
[Level B COUNTSTR](../../../lib/rxfnsb/rexx/countstr.md) and
[Level C COUNTSTR](../../../lib/rxfnsc/countstr.md) pages.

**Examples:**
```
COUNTSTR('bc', 'abcabcabc') == 3
COUNTSTR('aa', 'aaaaa')     == 2
COUNTSTR('', 'anything')    == 0
```



## DELSTR(string, start [,length])

Returns a copy of *string* with the substring that begins at the *start*
character and is
of length *length* characters, deleted.
If *length* is omitted, or is greater than the number of characters from
*start* to the end, the rest of the string is deleted. An explicitly supplied
zero deletes nothing. *length* must be non-negative, and *start* must be a
positive whole number. A start beyond the string returns it unchanged.

The Level B helper types *start* and *length* as `.int` and signals
`INVALID_ARGUMENTS` for invalid values. The standalone Level C BIF accepts
Classic whole-number text and reports standard `RXC-LC-40.*` errors. Level B
positions and lengths are codepoints. Level C uses octets in BYTE and codepoints
in UTF8. See the
separate [Level B DELSTR](../../../lib/rxfnsb/rexx/delstr.md) and
[Level C DELSTR](../../../lib/rxfnsc/delstr.md) pages.

**Examples:**
```
DELSTR('abcd', 3)    == 'ab'
DELSTR('abcde', 3, 2) == 'abe'
DELSTR('abcde', 6)   == 'abcde'
DELSTR('abcde', 3, 0) == 'abcde'
```



## DELWORD(s, start, n)

returns a copy of *string* with the sub-string of
*string* that starts at the `n`th word, and is of
length *length* blank-delimited words, deleted.
If *length* is not specified, or is greater than number of
remaining words in the string, it defaults to be the remaining words
in the string (including the **n*th word).
*length* must be a non-negative whole number, and *n*
must be a positive whole number.  If *n* is greater than the
number of words in *string*, the string is returned unchanged.
The string deleted includes any blanks following the final word
involved, but none of the blanks preceding the first word involved.

**Examples:**
```
DELWORD('Now is the  time', 2, 2) == 'Now time'
DELWORD('Now is the time ', 3)   == 'Now is '
DELWORD('Now  time', 5)          == 'Now  time'
```



## INSERT(new, target [,before [,length [,pad]]])

Returns a copy of *target* with *new* inserted after *before* characters.
*before* defaults to zero, which inserts before the first target character.
When *before* is beyond the target, padding extends the target to that
position.

*length* controls the width of the inserted text. It defaults to the character
length of *new*; a supplied zero inserts no new text. The insertion is
truncated or padded to that width. *before* and *length* must be non-negative
whole numbers. *pad* defaults to blank and a supplied pad must contain exactly
one character.

The Level B helper uses `.string` text and `.int` position/length values and
signals `INVALID_ARGUMENTS` for invalid values. The standalone Level C BIF
accepts RexxValue text and reports standard `RXC-LC-40.*` context errors. Level
B measures codepoints. Level C measures octets in BYTE and codepoints in UTF8,
including the one-unit pad rule. See
the separate [Level B INSERT](../../../lib/rxfnsb/rexx/insert.md) and
[Level C INSERT](../../../lib/rxfnsc/insert.md) pages for their distinct
contracts and implementation notes.

**Examples:**
```
INSERT('123', 'abc')         == '123abc'
INSERT(' ', 'abcdef', 3)      == 'abc def'
INSERT('123', 'abc', 5, 6)     == 'abc  123   '
INSERT('123', 'abc', 5, 6, '+') == 'abc++123+++'
INSERT('123', 'abc', 0, 5, '-') == '123--abc'
INSERT('abc', 'def', 2, 1)      == 'deaf'
```



## JUSTIFY(s, width, pad)




## LEFT(string, length [,pad])

returns a string of length *length* containing the
left-most *length* characters of *string*.
The string is padded with *pad* characters (or truncated) on
the right as needed.
The default *pad* character is a blank.
*length* must be a non-negative whole number.
A supplied pad must contain exactly one character.

The Level B helper requires `.int` *length* and signals `INVALID_ARGUMENTS` for
an invalid length or pad. The standalone Level C BIF accepts Classic
whole-number text and reports standard `RXC-LC-40.*` context errors. Level B
counts Unicode codepoints. Level C counts exact octets in BYTE and codepoints in
UTF8. See the separate
[Level B LEFT](../../../lib/rxfnsb/rexx/left.md) and
[Level C LEFT](../../../lib/rxfnsc/left.md) pages.

**Examples:**
```
LEFT('abc d', 8)     == 'abc d   '
LEFT('abc d', 8, '.') == 'abc d...'
LEFT('abc defg', 6)  == 'abc de'
```



## LENGTH(string)

Returns the character-unit length of *string*. Level B returns its Unicode
codepoint count, not the number of bytes in its UTF-8 representation; a
combining codepoint is counted separately from the base character it follows.
Level C returns the exact octet count in BYTE and the codepoint count in UTF8.

The Level B helper accepts `.string` and returns `.int`. The standalone Level C
BIF accepts RexxValue text and returns the decimal count in a RexxValue, with
standard `RXC-LC-40.*` argument errors. See the separate
[Level B LENGTH](../../../lib/rxfnsb/rexx/length.md) and
[Level C LENGTH](../../../lib/rxfnsc/length.md) pages. The specification's
`23.1` invalid-character-data case is unreachable after normal cREXX text has
entered the valid configured `.string` model.

**Examples:**
```
LENGTH('abcdefgh') == 8
LENGTH('')         == 0
LENGTH('é日🙂')    == 3
```



## LOWER(string)

Returns a copy of *string* after applying Level B's locale-independent, limited
simple lowercase table. Covered letters are replaced by their lowercase
equivalents; other codepoints are unchanged. This is deliberately not full
Unicode case folding. The surface accepts only the string argument; it has no
substring position or length options.

The helper accepts and returns `.string`, does not modify its argument, performs
the runtime's limited simple mapping, and has no error branch for valid text. See
[Level B LOWER](../../../lib/rxfnsb/rexx/lower.md) for its exact contract and
implementation notes. LOWER is not a required Level C BIF in the repository
catalog; the existing common-runtime helper is compatibility surface only.

**Examples:**
```
LOWER('SumA') == 'suma'
LOWER('ÄÖÜÉ') == 'äöüé'
LOWER('')     == ''
```



## OVERLAY(new, target [,start [,length [,pad]]])

Returns a copy of *target* with formatted *new* text written from the 1-based
character position *start*. The default start is one. If the characters before
*start* extend beyond the target, padding fills that gap.

When *length* is omitted it is the character length of *new*. A supplied zero
writes no new text; otherwise *new* is truncated or padded to that width before
replacing the corresponding target characters. *start* must be positive,
*length* must be non-negative, and a supplied *pad* must contain exactly one
character. The default pad is blank.

The Level B helper uses `.string` text and `.int` start/length values and
signals `INVALID_ARGUMENTS` for invalid values. The standalone Level C BIF
accepts RexxValue text and reports standard `RXC-LC-40.*` context errors. Level
B measures codepoints. Level C measures octets in BYTE and codepoints in UTF8,
including the one-unit pad rule. See
the separate [Level B OVERLAY](../../../lib/rxfnsb/rexx/overlay.md) and
[Level C OVERLAY](../../../lib/rxfnsc/overlay.md) pages.

**Examples:**
```
OVERLAY(' ', 'abcdef', 3)      == 'ab def'
OVERLAY('.', 'abcdef', 3, 2)    == 'ab. ef'
OVERLAY('qq', 'abcd')         == 'qqcd'
OVERLAY('qq', 'abcd', 4)       == 'abcqq'
OVERLAY('123', 'abc', 5, 6, '+') == 'abc+123+++'
OVERLAY('foo', 'abcdef', 3, 0) == 'abcdef'
```



## POS(needle, haystack [,start])

returns the position of the string *needle*, in
*string* (the "haystack"), searching from left to right.
If the string *needle* is not found, or is the null string,
0 is returned.
By default the search starts at the first character of
*string* (that is, *start* has the value 1).
This may be overridden by specifying *start* (which must be a
positive whole number), the point at which to start the search; if
*start* is greater than the length of *string* then 0
is returned.

The Level B helper types *start* as `.int` and signals `INVALID_ARGUMENTS` for
a non-positive value. The standalone Level C BIF accepts Classic whole-number
text and reports standard `RXC-LC-40.*` context errors. Level B returns Unicode
codepoint positions. Level C returns octet positions in BYTE and codepoint
positions in UTF8. See the separate
[Level B POS](../../../lib/rxfnsb/rexx/pos.md) and
[Level C POS](../../../lib/rxfnsc/pos.md) pages for their contracts and direct
search implementation.


**Examples:**
```
POS('day', 'Saturday')    == 6
POS('x', 'abc def ghi')   == 0
POS(' ', 'abc def ghi')   == 4
POS(' ', 'abc def ghi', 5) == 8
```



## LASTPOS(needle, haystack [,start])

Returns the position of the last occurrence of *needle* whose final character
unit is at or before *start*. The search is case-sensitive. Level B positions
are 1-based Unicode codepoints; Level C positions are octets in BYTE and
codepoints in UTF8. When *start* is omitted, the complete haystack is considered.
A value beyond the haystack has the same effect as omission. A null or absent
needle returns `0`.

*start* must be a positive whole number when supplied. The Level B helper types
it as `.int` and signals `INVALID_ARGUMENTS` for an invalid value. The
standalone Level C BIF accepts Classic whole-number text and reports standard
`RXC-LC-40.*` context errors. See the separate
[Level B LASTPOS](../../../lib/rxfnsb/rexx/lastpos.md) and
[Level C LASTPOS](../../../lib/rxfnsc/lastpos.md) pages.

**Examples:**
```
LASTPOS(' ', 'abc def ghi')    == 8
LASTPOS(' ', 'abc def ghi', 7) == 4
LASTPOS('abc', 'abc abc', 6)   == 1
LASTPOS('aa', 'aaa')           == 2
LASTPOS('', 'anything')        == 0
```



## RIGHT(string, length [,pad])

returns a string of length *length* containing the
right-most *length* characters of *string* -
that is, padded with *pad* characters (or truncated) on the
left as needed.  The default *pad* character is a blank.
*length* must be a non-negative whole number.
A supplied pad must contain exactly one character.

The Level B helper requires `.int` *length* and signals `INVALID_ARGUMENTS` for
an invalid length or pad. The standalone Level C BIF accepts Classic
whole-number text and reports standard `RXC-LC-40.*` context errors. Level B
counts Unicode codepoints. Level C counts exact octets in BYTE and codepoints in
UTF8. See the separate
[Level B RIGHT](../../../lib/rxfnsb/rexx/right.md) and
[Level C RIGHT](../../../lib/rxfnsc/right.md) pages.

**Examples:**
```
RIGHT('abc  d', 8)  == '  abc  d'
RIGHT('abc def', 5) == 'c def'
RIGHT('12', 5, '0')  == '00012'
```



## REVERSE(string)

Returns a copy of *string* with its character units in reverse order. Level B
reverses Unicode codepoints, so combining marks are independent and grapheme
clusters are not preserved as units. Level C reverses exact octets in BYTE and
Unicode codepoints in UTF8.

The Level B helper accepts and returns `.string` and has no domain error for
valid text. The standalone Level C BIF accepts RexxValue text and reports
standard `RXC-LC-40.*` argument errors. Both use a single reverse pass over
their active unit representation. See the separate
[Level B REVERSE](../../../lib/rxfnsb/rexx/reverse.md) and
[Level C REVERSE](../../../lib/rxfnsc/reverse.md) pages.

**Examples:**
```
REVERSE('abc')    == 'cba'
REVERSE('aé日🙂') == '🙂日éa'
REVERSE('')       == ''
```



## SPACE(string [,count [,pad]])

Returns a copy of *string* with its blank-delimited words joined by exactly
*count* copies of *pad*. Level B uses Unicode 17.0.0 `White_Space`. Level C BYTE
uses ASCII space plus configured blank octets; Level C UTF8 uses Unicode
`White_Space` plus configured blank codepoints. Leading/trailing blanks are
removed and each internal blank run becomes the requested separator. *count*
must be non-negative; zero joins words directly. The default count is one and
the default pad is blank. A supplied pad must contain one active character unit.

The Level B helper takes `.string`, an optional `.int` count, and `.string` pad,
and signals `INVALID_ARGUMENTS` for an invalid count or pad. The standalone
Level C BIF accepts RexxValue text and reports standard `RXC-LC-40.*` context
errors. Both scan the source once. See the separate
[Level B SPACE](../../../lib/rxfnsb/rexx/space.md) and
[Level C SPACE](../../../lib/rxfnsc/space.md) pages.

**Examples:**
```
SPACE('abc  def  ')        == 'abc def'
SPACE('  abc def ', 3)     == 'abc   def'
SPACE('abc  def  ', 1)     == 'abc def'
SPACE('abc  def  ', 0)     == 'abcdef'
SPACE('abc  def  ', 2, '+') == 'abc++def'
```



## STRIP(string [,option [,char]])

Returns a copy of *string* with a leading, trailing, or both leading and
trailing runs removed. The first codepoint of *option* is `L`, `T`, or `B`
respectively, case-insensitively; the default is `B`.

When *char* is omitted, Level B removes Unicode 17.0.0 `White_Space`; Level C
BYTE removes ASCII space plus configured blank octets and Level C UTF8 removes
Unicode `White_Space` plus configured blank codepoints. When supplied, *char*
must contain exactly one active unit and only that unit is removed. Thus an
explicit blank differs from omission when other configured whitespace occurs at
an edge.

The Level B helper accepts strings and signals `INVALID_ARGUMENTS` for an
invalid option or supplied char. The standalone Level C BIF reports standard
`RXC-LC-40.*` context errors. Both compute one direct source slice. See the
separate [Level B STRIP](../../../lib/rxfnsb/rexx/strip.md) and
[Level C STRIP](../../../lib/rxfnsc/strip.md) pages.

**Examples:**
```
STRIP('  ab c  ')        == 'ab c'
STRIP('  ab c  ', 'L')   == 'ab c  '
STRIP('  ab c  ', 't')   == '  ab c'
STRIP('12.70000', 't', '0') == '12.7'
STRIP('0012.700', 'b', '0') == '12.7'
```



## SUBSTR(string, start [,length [,pad]])

Returns the substring of *string* beginning at the positive 1-based position
*start*. Level B measures Unicode codepoints. Level C measures exact octets in
BYTE and codepoints in UTF8. When *length* is omitted, the result continues
through the end, or is empty when *start* is beyond the source. A supplied
*length* must be non-negative and fixes the result width; missing source units
are replaced with *pad*. The default pad is blank and a supplied pad must
contain exactly one active unit.

The Level B helper requires `.int` start/length values and signals
`INVALID_ARGUMENTS` for invalid values. The standalone Level C BIF accepts
Classic whole-number text and reports standard `RXC-LC-40.*` context errors.
Both leave the source unchanged; their active units differ as described above.
See the separate [Level B SUBSTR](../../../lib/rxfnsb/rexx/substr.md) and
[Level C SUBSTR](../../../lib/rxfnsc/substr.md) pages.

**Examples:**
```
SUBSTR('abc', 2)       == 'bc'
SUBSTR('abc', 2, 4)     == 'bc  '
SUBSTR('abc', 5, 4)     == '    '
SUBSTR('abc', 2, 6, '.') == 'bc....'
SUBSTR('abc', 5, 6, '.') == '......'
```
**Note:** In some situations the positional (numeric) patterns of parsing
templates are more convenient for selecting sub-strings, especially if
more than one sub-string is to be extracted from a string.


### SUBSTRO

`SUBSTRO` is a cREXX-specific Level B alternate name with the same typed,
signal-based slicing behavior as `SUBSTR`. It has a standalone direct VM
implementation because it remains a bootstrap-library export, but it is not a
Level C BIF. See [Level B SUBSTRO](../../../lib/rxfnsb/rexx/substro.md).



## SUBWORD(s, start, n)

returns the sub-string of *string* that starts at the
**n*th word, and is up to *length* blank-delimited
words long.
*n* must be a positive whole number; if greater than the number
of words in the string then the null string is returned.
*length* must be a non-negative whole number.
If *length* is omitted it defaults to be the remaining words
in the string.
The returned string will never have leading or trailing blanks, but
will include all blanks between the selected words.

**Examples:**
```
SUBWORD('Now is the  time', 2, 2) == 'is the'
SUBWORD('Now is the  time', 3)   == 'the  time'
SUBWORD('Now is the  time', 5)   == ''
```



## TRANSLATE(string [,outputTable [,inputTable [,pad]]])

returns a copy of *string* with each character in
*string* either unchanged or translated to another character.

The **translate** function acts by searching the input translate
table, *tablei*, for each character in *string*.
If the character is found in *tablei* (the first, leftmost,
occurrence being used if there are duplicates) then the corresponding
character in the same position in the output translate table,
*tableo*, is used in the result string; otherwise the original
character found in *string* is used.
The result string is always the same length as *string*.

The translate tables may be of any length, including the null string.
The output table, *tableo*, is padded with *pad* or
truncated on the right as necessary to be the same length as
*tablei*.
The default *pad* is a blank.

When both tables are omitted, TRANSLATE applies the active profile's uppercase
mapping. When the output table is supplied and the input table is omitted,
Level B uses its fixed U+0000 through U+00FF codepoint domain and Level C BYTE
uses its exact `00` through `FF` XRANGE. Level C UTF8 rejects that form because
Classic XRANGE is not a Unicode range. Level B tables are codepoint based;
Level C table units follow its BYTE or UTF8 profile. See the separate
[Level B API](../../../lib/rxfnsb/rexx/translate.md) and
[Level C BIF contract](../../../lib/rxfnsc/translate.md).

**Examples:**
```
TRANSLATE('abbc', '&', 'b')           == 'a&&c'
TRANSLATE('abcdef', '12', 'ec')       == 'ab2d1f'
TRANSLATE('abcdef', '12', 'abcd', '.') == '12..ef'
TRANSLATE('4123', 'abcd', '1234')     == 'dabc'
TRANSLATE('4123', 'hods', '1234')     == 'shod'
```
**Note:** The last two examples show how the **translate** function
may be used to move around the characters in a string.
In these examples, any 4-character string could be specified as the
first argument and its last character would be moved to the beginning of
the string.
Similarly, the term:
```
TRANSLATE('gh.ef.abcd', 19970827, 'abcdefgh')
```
(which returns "**27.08.1997**") shows how a string (in
this case perhaps a date) might be re-formatted and merged with other
characters using the **translate** function.



## UPPER(string)

Returns a copy of *string* after applying Level B's locale-independent, limited
simple uppercase table. Covered letters are replaced by their uppercase
equivalents; other codepoints are unchanged. This is deliberately not full
Unicode case folding. The surface accepts only the string argument; it has no
substring position or length options.

The helper accepts and returns `.string`, performs the runtime's limited simple
mapping, and has no error branch for valid text. See
[Level B UPPER](../../../lib/rxfnsb/rexx/upper.md) for its exact contract and
implementation notes. UPPER is not a required Level C BIF in the repository
catalog; the existing common-runtime helper is compatibility surface only.

**Examples:**
```
UPPER('Fou-Baa') == 'FOU-BAA'
UPPER('äöüé')    == 'ÄÖÜÉ'
UPPER('')        == ''
```



## VERIFY(string, reference [,option [,start]])

verifies that *string* is composed only of characters
from *reference*, by returning the position of the first
character in *string* that is not also in
*reference*.  If all the characters were found in
*reference*, 0 is returned.
 The *option* may be either **'Nomatch'** (the
default) or **'Match'**.  Only the first character of
*option* is significant and it may be in uppercase or in
lowercase.
If **'Match'** is specified, the position of the first character
in *string* that **is** in *reference* is
returned, or 0 is returned if none of the characters were found.
 The default for *start* is 1 (that is, the search starts at
the first character of *string*).
This can be overridden by giving a different *start* point,
which must be positive.
 If *string* is the null string, the function returns 0,
regardless of the value of the *option*.
Similarly if *start* is greater than
*string***.length**, 0 is returned.
 If *reference* is the null string, then the returned value
is the same as the value used for *start*,
unless **'Match'** is specified as the *option*, in
which case 0 is returned.

The Level B helper types *start* as `.int`; an empty or invalid option and a
non-positive start signal `INVALID_ARGUMENTS`. The standalone Level C BIF
accepts Classic whole-number text and reports standard `RXC-LC-40.*` context
errors. Level B reports codepoint positions. Level C reports octet positions in
BYTE and codepoint positions in UTF8. See the separate
[Level B VERIFY](../../../lib/rxfnsb/rexx/verify.md) and
[Level C VERIFY](../../../lib/rxfnsc/verify.md) pages for their distinct
contracts.

**Examples:**
```
VERIFY('123', '1234567890')          == 0
VERIFY('1Z3', '1234567890')          == 2
VERIFY('AB4T', '1234567890', 'M')     == 3
VERIFY('1P3Q4', '1234567890', 'N', 3)  == 4
VERIFY('ABCDE', '', 'n', 3)            == 3
VERIFY('AB3CD5', '1234567890', 'm', 4) == 6
```



## WORD(s, n)

returns the n-th blank-delimited word in
*string*.
*n* must be positive.
If there are fewer than *n* words in *string*, the
null string is returned.
This function is exactly equivalent to
*string***.subword(***n*,**1)**.

**Examples:**
```
WORD('Now is the time', 3) == 'the'
WORD('Now is the time', 5) == ''
```



## WORDINDEX(s, n)

returns the character position of the **n*th
blank-delimited word in *string*.
*n* must be positive.
If there are fewer than *n* words in the string, 0 is returned.

**Examples:**
```
WORDINDEX('Now is the time', 3) == 8
WORDINDEX('Now is the time', 6) == 0
```



## WORDLENGTH(s, n)

returns the length of the **n*th blank-delimited word in
*string*.
*n* must be positive.
If there are fewer than *n* words in the string, 0 is returned.

**Examples:**
```
WORDLENGTH('Now is the time', 2)    == 2
WORDLENGTH('Now comes the time', 2) == 5
WORDLENGTH('Now is the time', 6)    == 0
```



## WORDS(s)

returns the number of blank-delimited words in *string*.

**Examples:**
```
WORDS('Now is the time') == 4
WORDS(' ')               == 0
WORDS('')                == 0
```



## DATATYPE(s, type)

returns 1 if *string* matches the description requested with
the *option*, or 0 otherwise.
If *string* is the null string, 0 is always returned.

Only the first character of *option* is significant, and it may
be in either uppercase or lowercase.
The following *option* characters are recognized:
\begin{description}
\item[A]
(Alphanumeric); returns 1 if *string* only contains
characters from the ranges "a-z", "A-Z", and "0-9".
\item[B]
(Binary); returns 1 if *string* only contains the
characters "0" and/or "1".
\item[D]
(Digits); returns 1 if *string* only contains
characters from the range "0-9".
\item[L]
(Lowercase); returns 1 if *string* only contains
characters from the range "a-z".
\item[M]
(Mixed case); returns 1 if *string* only contains
characters from the ranges "a-z" and "A-Z".
\item[N]
(Number); returns 1 if *string* is a syntactically valid
CREXX number that could be added to **'0'** without error,
\item[S]
(Symbol); returns 1 if *string* only contains characters
that are valid in non-numeric symbols (the alphanumeric characters and
underscore), and does not start with a digit.  Note that both uppercase
and lowercase letters are permitted.
\item[U]
(Uppercase); returns 1 if *string* only contains
characters from the range "A-Z".
\item[W]
(Whole Number); returns 1 if *string* is a syntactically valid
CREXX number that can be added to **'0'** without error, and
whose decimal part after that addition, with no rounding, is zero.
\item[X]
(heXadecimal); returns 1 if *string* only contains
characters from the ranges "a-f", "A-F", and "0-9".
\end{description}

**Examples:**
```
DATATYPE('101', 'B')    == 1
DATATYPE('12.3', 'D')   == 0
DATATYPE('12.3', 'N')   == 1
DATATYPE('12.3', 'W')   == 0
DATATYPE('LaArca', 'M') == 1
DATATYPE('', 'M')       == 0
DATATYPE('Llanes', 'L') == 0
DATATYPE('3 d', 's')    == 0
DATATYPE('BCd3', 'X')   == 1
DATATYPE('BCgd3', 'X')  == 0
```
**Note:** The **datatype** function tests the meaning of the characters
in a string, independent of the encoding of those characters.  Extra
letters and Extra digits cause **datatype** to return 0 except
for the number tests ("**N**" and "**W**"),
which treat extra digits whose value is in the range 0-9 as though they
were the corresponding Arabic numeral.



## RANDOM(min, max, seed)

Returns a pseudo-random integer in the inclusive range `min` through `max`.
When omitted, `min` defaults to `0`, `max` defaults to `999`, and `seed`
defaults to `-1`. The current implementation raises a syntax condition for a
negative minimum or for `min > max`.

`seed` is passed to the VM `irand` instruction. The default `-1` asks the
instruction to choose its normal seed behavior.


## TIME(option)

Returns time information. The option is case-insensitive and defaults to `N`.

| Option | Result |
|--------|--------|
| `N` | Local time as `hh:mm:ss`. |
| `L` | Local time as `hh:mm:ss.ffffff`. |
| `H` | Hour since midnight. |
| `M` | Minutes since midnight. |
| `S` | Seconds since midnight. |
| `US` | Microseconds since midnight. |
| `E` | Elapsed seconds. |
| `R` | Reset/read the elapsed timer. |
| `C` | Civil-style time with `am` or `pm`. |
| `UTC` | UTC time using the normal `hh:mm:ss` format. |
| `ZD` | UTC offset in seconds. |
| `T` | CPU ticks since program start. |
| `TS` | Ticks per second. |
| `ZN` | Time zone name. |

An unsupported option signals `INVALID_ARGUMENTS`. This typed Level B extension
is documented separately from the Classic three-argument Level C BIF in the
[Level B API](../../../lib/rxfnsb/rexx/time.md) and
[Level C contract](../../../lib/rxfnsc/time.md).


## DATE(oformat, date, iformat, osep, isep)

Converts dates between supported date formats. With no `date` argument, it
uses the current local date. Empty `oformat` or `iformat` values default to
`NORMAL`; `osep` and `isep` can override the output or input separator.

Input formats are matched by abbreviation and currently include `NORMAL`,
`STANDARD`, `ORDERED`, `EUROPEAN`, `GERMAN`, `USA`, `INTERNATIONAL`,
`QUALIFIED`, `JULIAN`, `BASE`, `UNIX`, and `EPOCH`.
`NORMAL` and `QUALIFIED` input dates accept full or abbreviated English month
names; this month-prefix matching belongs to DATE and does not change the exact
word matching performed by `WORDPOS`.

Output formats are also matched by abbreviation and currently include
`NORMAL`, `XNORMAL`, `STANDARD`, `ORDERED`, `XORDERED`, `EUROPEAN`,
`XEUROPEAN`, `GERMAN`, `XGERMAN`, `USA`, `XUSA`, `INTERNATIONAL`,
`QUALIFIED`, `JULIAN`, `DAYS`, `WEEKDAY`, `MONTH`, `CENTURY`, `BASE`,
`UNIX`, `JDN`, `EPOCH`, `DEC`, and `XDEC`.


## SEQUENCE(from, to)

Returns the sequence of characters from `from` through `to`, using Unicode
codepoint values. It is the Unicode-capable replacement for byte-oriented
`XRANGE`; unlike `XRANGE`, it does not wrap around when `from` is greater than
`to`. Both endpoints must be single characters. A descending range or an
invalid endpoint signals `INVALID_ARGUMENTS`; surrogate code points are skipped
because they are not Unicode scalar values. See the stable
[Level B API](../../../lib/rxfnsb/rexx/sequence.md).


## XRANGE(start, end)

The typed Level B helper returns the inclusive U+0000 through U+00FF
byte-domain character range and wraps at U+00FF. Both endpoints are required,
must contain exactly one character, and invalid endpoints signal
`INVALID_ARGUMENTS`. It is retained for legacy byte-range use; `SEQUENCE` is
the non-wrapping Unicode range API.

Classic Level C `XRANGE([start [,end]])` is different. In the default BYTE
profile it returns the inclusive wrapping exact-byte range, defaulting to
`00` through `FF`. It is intentionally unavailable in UTF8 because it is not a
Unicode scalar or grapheme range.
See the separate [Level B API](../../../lib/rxfnsb/rexx/xrange.md) and
[Level C contract](../../../lib/rxfnsc/xrange.md).


<!-- ## SOURCELINE(n) -->




<!-- ## ARG(n) -->




<!-- ## STORAGE(address, length, newvalue) -->




## TRACE(option)

The SAA `TRACE(option)` built-in function name is reserved for compatibility.
In the current beta, use the `TRACE` statement to set tracing:

```rexx
trace off
trace normal
trace results
trace value option
```

`TRACE VALUE option` evaluates `option` at runtime and applies the same option
rules as a static `TRACE` statement. See the `TRACE` statement reference for
the supported modes, output targets, and namespace suppression controls.

The Level C library now also contains a standalone direct `RexxValue`
implementation of the Classic `TRACE([option])` query/update contract. It is
covered through a direct library harness, but normal compiled calls are not yet
lowered to that entry point; that wiring is intentionally deferred to the later
bulk Level C lowering change. Its library contract is documented in
`lib/rxfnsc/trace.md`.



## VALUE(name, newvalue, pool)

There are two intentionally different library contracts.

The Level B helper is read-only:

```rexx
import rxfnsb
count = 12
say value("count")    /* 12 */
say value("missing")  /* MISSING */
```

It accepts one `.string` name, searches only the immediate caller procedure's
scalar/constant metadata, and returns a `.string`. It does not assign variables
or implement Classic stems. See `lib/rxfnsb/rexx/value.md`.

The standalone Level C BIF implements `VALUE(name [,newvalue [,pool]])` over
`RexxValue` and the caller `RexxVariablePool`. The internal form expands
compound-variable tails, returns the old value, and optionally assigns the new
value. Invalid internal symbols use error `40.26`.

The repository does not yet have the configuration service required to resolve
the optional external `pool` name. Until that co-dependency exists, a supplied
third argument reports `40.37`. The exact direct-call contract and test scope
are documented in `lib/rxfnsc/value.md`. Compiler lowering to the direct entry
point remains part of the later bulk Level C lowering change.


## VERSION()

Returns a string with implementation and build information supplied by the VM
`rxvers` instruction. The current string layout is:

```bash
platform bits crexx-version build-date
```

`platform` is one of the VM's compiled platform names: `linux`, `windows`,
`macOS`, `cms`, or `unknown`. `bits` is `32` or `64`; `crexx-version` starts
with `crexx-` and may contain build metadata; and `build-date` is `yyyymmdd`.
The function does not currently expose endianness as a separate field. Its
selector-local Level B contract is in `lib/rxfnsb/rexx/version.md`.



## ABBREV(info, word, length)

Returns 1 if *word* is equal to the leading characters of
*info* and *word* is not less than
the minimum length, *length*; 0 is returned
if either of these conditions is not met.
*length* must be a non-negative whole number; the default is
the length of *word* in the Level C contract. The Level B helper uses an
integer default of zero, which produces the same result for an omitted minimum
because the entire candidate must still match.


**Examples:**
```
ABBREV('Print', 'Pri')   == 1
ABBREV('PRINT', 'Pri')   == 0
ABBREV('PRINT', 'PRI', 4) == 0
ABBREV('PRINT', 'PRY')   == 0
ABBREV('PRINT', '')      == 1
ABBREV('PRINT', '', 1)    == 0
```
**Note:** A null string will always match if a length of 0 (or the default)
is used.
This allows a default keyword to be selected automatically if desired.

The typed Level B and direct `RexxValue` contracts are documented separately in
`lib/rxfnsb/rexx/abbrev.md` and `lib/rxfnsc/abbrev.md`. Level B measures the
prefix and minimum in codepoints. Level C measures octets in BYTE and codepoints
in UTF8.


**Example:**
```
say 'Enter option:';  option=ask
select  /* keyword1 is to be the default */
  when ABBREV('keyword1', option) then ...
  when ABBREV('keyword2', option) then ...
     ...
  otherwise ...
  end
```



<!-- ## ADDRESS() -->




<!-- ## CONDITION([info]) -->




## DIGITS()

Returns the positive number of significant decimal digits in the current
procedure's numeric context. The value is controlled by `NUMERIC DIGITS`; a
called BIF observes the setting inherited from its immediate caller.

```rexx
numeric digits 12
say DIGITS()  /* 12 */
```

`DIGITS` accepts no arguments. The distinct typed Level B and direct RexxValue
contracts are documented in [Level B numeric accessors](../../../lib/rxfnsb/rexx/numeric.md)
and [Level C numeric BIFs](../../../lib/rxfnsc/numeric.md).


## FORM()

Returns `SCIENTIFIC` or `ENGINEERING`, identifying the exponential notation
selected by the current procedure's `NUMERIC FORM` setting.

```rexx
numeric form engineering
say FORM()  /* ENGINEERING */
```

`FORM` accepts no arguments. Level B's typed helper intentionally returns the
lowercase cREXX name, while the Level C BIF returns the uppercase Classic BIF
result. See the separate [Level B](../../../lib/rxfnsb/rexx/numeric.md) and
[Level C](../../../lib/rxfnsc/numeric.md) contracts.


## FUZZ()

Returns the non-negative number of least-significant digits ignored during
numeric comparisons in the current procedure. The value is controlled by
`NUMERIC FUZZ` and is always smaller than `DIGITS()`.

```rexx
numeric digits 12
numeric fuzz 2
say FUZZ()  /* 2 */
```

`FUZZ` accepts no arguments. Its typed Level B and direct RexxValue contracts
are documented separately in [Level B numeric accessors](../../../lib/rxfnsb/rexx/numeric.md)
and [Level C numeric BIFs](../../../lib/rxfnsc/numeric.md).




<!-- ## QUEUED() -->
