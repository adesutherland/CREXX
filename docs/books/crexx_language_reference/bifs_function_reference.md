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
| ARRAYCOPY | ARRAYCOPY(array, from [, count]) |
| ARRAYDELETE | ARRAYDELETE(array, from, count) |
| ARRAYDROP | ARRAYDROP(array) |
| ARRAYDUMP | ARRAYDUMP(array[, from[, to[, flags[, hdr[, prefix]]]]]) |
| ARRAYFIND | ARRAYFIND(find, array[, from[, case]]) |
| ARRAYFORMAT | ARRAYFORMAT(array[, from[, to[, flags[, hdr[, prefix]]]]]) |
| ARRAYGET | ARRAYGET(array=, index [, default]) |
| ARRAYHI | ARRAYHI(array[, 'GET' | 'SET'[, new_hi]]) |
| ARRAYINDEXOF | ARRAYINDEXOF(array, value,[from] ,[case]) |
| ARRAYINSERT | ARRAYINSERT(array, from, count[, default]) |
| ARRAYJOIN | ARRAYJOIN(array, [, separator]) |
| ARRAYMOVE | ARRAYMOVE(array, from, count, to) |
| ARRAYPOP | ARRAYPOP(array, [default]) |
| ARRAYPREPEND | ARRAYPREPEND(array, value [,default]) |
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
| QEXTRACTALL | QEXTRACTALL(open, close, text [, start])      |
| QEXTRACTPAIR | QEXTRACTPAIR(open, close, text, start, mode=')   |
| QPOS | QPOS(needle, text [, start]) |
| QREMOVEALL | QREMOVEALL(open, close, text [, mode])   |
| QSPLIT | QSPLIT(text, sep) |
| QSPLITSAFE | QSPLITSAFE(text, sep, start, pairs) |
| QSTRIPCOMMENT | QSTRIPCOMMENT(open, close, text) |
| QSUBWORD | QSUBWORD(string,wordnum)|
| QWORD | QWORD(line, wanted) |
| QWORDINDEX | QWORDINDEX(string,wordnum) |
| QWORDLENGTH | QWORDLENGTH(string,wordnum) |
| QWORDPOS | QWORDPOS(search, string [,start])  |
| QWORDS | QWORDS(string) |
| RERADIX | RERADIX(subject, fromradix, toradix) |
| SEQUENCE | SEQUENCE(from, to) |
| SPLICE | SPLICE(needle, haystack, at, len) |
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

## Array helpers

Level B arrays use `array[0]` as the high-water mark. User elements are stored
in `array[1]` through `array[array[0]]`. The `array*` helpers below live in
`rxfnsb`; mutating helpers take the array by `expose` and update it in place.
<!--  They are the supported array surface for new code. The older native arrays -->
<!-- plugin is deprecated. -->

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

## Quote-aware helpers

The `q*` helpers perform string operations while ignoring separators or words
that appear inside quoted text. They are ordinary `rxfnsb` functions and follow
the current implementation in `lib/rxfnsb/rexx`.

`QPOS(needle, text [, start])` returns the 1-based position of `needle` outside
single- or double-quoted regions, or `0` when it is not found. The current
quote scan treats matching single and double quote characters as delimiters.

`QSPLIT(text, sep)` splits `text` on `sep` only when the separator is outside
quoted regions. It returns a `.string[]` array of parts.

`QSPLITSAFE(text, sep, start, pairs)` is the nested-safe splitter. In addition
to quote tracking, it tracks paired delimiters from the `pairs` string, such as
the default `()`, and only splits at depth zero.


## ABS(x)

returns the absolute value of *string*, which must be a
number.
 Any sign is removed from the number, and it is then formatted by adding
zero with a digits setting that is either nine or, if greater, the
number of digits in the mantissa of the number (excluding leading
insignificant zeros).
Scientific notation is used, if necessary.

**Examples:**
```
ABS('12.3')              == 12.3
ABS(' -0.307')           == 0.307
ABS('123.45E+16')        == 1.2345E+18
ABS('- 1234567.7654321') == 1234567.7654321
```



## FORMAT(x, n, d)

formats (lays out) *string*, which must be a number.

The number, *string*, is first formatted by adding zero with a
digits setting that is either nine or, if greater, the number of digits
in the mantissa of the number (excluding leading insignificant zeros).
If no arguments are given, the result is precisely that of this
operation.

The arguments *before* and *after* may be specified to
control the number of characters to be used for the integer part and
decimal part of the result respectively.  If either of these is omitted
(with no arguments specified to its right), or is **null**, the
number of characters used will be as many as are needed for that part.

*before* must be a positive number; if it is larger than is
needed to contain the integer part, that part is padded on the left with
blanks to the requested length.
If *before* is not large enough to contain the integer part
of the number (including the sign, for negative numbers), an error
results.

*after* must be a non-negative number; if it is not the same
size as the decimal part of the number, the number will be rounded (or
extended with zeros) to fit.  Specifying 0 for *after* will
cause the number to be rounded to an integer (that is, it will have no
decimal part or decimal point).

**Examples:**
```
FORMAT(' - 12.73')         == '-12.73'
FORMAT('0.000')            == '0'
FORMAT('3', 4)             == '   3'
FORMAT('1.73', 4, 0)        == '   2'
FORMAT('1.73', 4, 3)        == '   1.730'
FORMAT('-.76', 4, 1)        == '  -0.8'
FORMAT('3.03', 4)          == '   3.03'
FORMAT(' - 12.73', null, 4) == '-12.7300'
```

Further arguments may be passed to the format function to control
the use of exponential notation.
The full syntax of the function is then:

format([before[,after[,explaces[,exdigits[,exform]]]]])
 The first two arguments are as already described.  The other three
(*explaces*, *exdigits*, and *exform*)
control the exponent part of the result.  The default for any of the
arguments may be selected by omitting them (if there are no arguments to
be specified to their right) or by using the value **null**.

*explaces* must be a positive number; it sets the number of
places (digits after the sign of the exponent) to be used for any
exponent part, the default being to use as many as are needed.
If *explaces* is specified and is not large enough to contain
the exponent, an error results.
If *explaces* is specified and the exponent will be 0,
then *explaces*+2 blanks are supplied for the exponent
part of the result.

*exdigits* sets the trigger point for use of exponential
notation.
If, after the first formatting, the number of places needed before the
decimal point exceeds *exdigits*, or if the absolute value of
the result is less than **0.000001**, then exponential form will
be used, provided that *exdigits* was specified.
When *exdigits* is not specified, exponential notation
will never be used.
The current setting of numeric digits may be used for
*exdigits* by specifying the special word
 **digits** %% (see page refswdigit)
 .
If 0 is specified for *exdigits*, exponential
notation is always used unless the exponent would be 0.

*exform* sets the form for exponential notation (if needed).
*exform* may be either **'Scientific'** (the default)
or **'Engineering'**.  Only the first character of
*exform* is significant and it may be in uppercase or in
lowercase.
The current setting of numeric form may be used by specifying
the special word  **form** %% (see page refswform)
.
If engineering form is in effect, up to three digits (plus sign) may be
needed for the integer part of the result (*before*).

**Examples:**
```
FORMAT('12345.73', null, null, 2, 2) == '1.234573E+04'
FORMAT('12345.73', null, 3, null, 0) == '1.235E+4'
FORMAT('1.234573', null, 3, null, 0) == '1.235'
FORMAT('123.45', null, 3, 2, 0)      == '1.235E+02'
FORMAT('1234.5', null, 3, 2, 0, 'e')  == '1.235E+03'
FORMAT('1.2345', null, 3, 2, 0)      == '1.235    '
FORMAT('12345.73', null, null, 3, 6) == '12345.73     '
FORMAT('12345e+5', null, 3)        == '1234500000.000'
```
 **Implementation minimum:** If exponents are supported in an
implementation, then they must be supported for exponents whose
absolute value is at least as large as the largest number that can be
expressed as an exact integer in default precision, *i.e.*, 999999999.
Therefore, values for *explaces* of up to 9 should also be
supported.



## MAX(x, y, ...)

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

**Examples:**
```
0.max(1)          ==1
MAX('-1', 1)       ==1
MAX('+1', -1)      ==1
MAX('1.0', 1.00)   =='1.0'
MAX('1.00', 1.0)   =='1.00'
MAX('123456700000', 1234567E+5)   == '123456700000'
MAX('1234567E+5', '123456700000') == '1.234567E+11'
```



## MIN(x, y, ...)

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

**Examples:**
```
0.min(1)          ==0
MIN('-1', 1)       =='-1'
MIN('+1', -1)      =='-1'
MIN('1.0', 1.00)   =='1.0'
MIN('1.00', 1.0)   =='1.00'
MIN('123456700000', 1234567E+5)   == '123456700000'
MIN('1234567E+5', '123456700000') == '1.234567E+11'
```



## SIGN(x)

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



## TRUNC(x, n)

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



## B2X(b)

Binary to hexadecimal.
Converts *string*, a string of at least one binary
(**0** and/or **1**) digits, to an equivalent string of
hexadecimal characters.
The returned string will use uppercase Roman letters for the values A-F,
and will not include any blanks.
 If the number of binary digits in the string is not a multiple of four,
then up to three **'0'** digits will be added on the left before
conversion to make a total that is a multiple of four.

**Examples:**
```
B2X('11000011')  == 'C3'
B2X('10111')     == '17'
B2X('0101')      == '5'
B2X('101')       == '5'
B2X('111110000') == '1F0'
```


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
result is truncated at the end of *data*.

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
from *start* to the end are removed.

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
`0`.

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
assumed. If any non-blank character is not a hexadecimal digit, the current
implementation returns an empty `.binary` value.

**Examples:**
```rexx
BIN2X(X2BIN("ff 00 aa")) == "FF00AA"
BIN2X(X2BIN("f"))        == "0F"
BINLENGTH(X2BIN("zz"))   == 0
```



## C2D(s)

Coded character to decimal.
Converts the Unicode code point of the character in *string* (which must
be exactly one character) to its decimal representation.
The returned string will be a non-negative number that represents
the code point of the character and will not include any sign, blanks,
insignificant leading zeros, or decimal part.

**Examples:**
```
C2D('M')  == '77'  -- ASCII or Unicode
C2D('🔥') == '128293'
C2D('7')  == '247' -- EBCDIC
C2D('\textbackslash{}r') == '13'  -- ASCII or Unicode
C2D('\textbackslash{}0') == '0'
```
 The  **c2x** function %% (see page refc2x)
 can be used to
convert the encoding of a character to a hexadecimal representation.



## C2X(s)

Coded character to hexadecimal.
Converts the encoding of the character in *string* (which must be
exactly one character) to its hexadecimal representation (unpacks).
The returned string will use uppercase Roman letters for the values A-F,
and will not include any blanks.
Insignificant leading zeros are removed.

**Examples:**
```
C2X('M')  == '4D' -- ASCII or Unicode
C2X('7')  == 'F7' -- EBCDIC
C2X('\textbackslash{}r') == 'D'  -- ASCII or Unicode
C2X('\textbackslash{}0') == '0'
```
 The  **c2d** function %% (see page refc2d)
 can be used to
convert the encoding of a character to a decimal number.



## D2C(n)

Decimal to coded character.
Converts the *string* (a CREXX *number*) to a
single character, where the number is used as the Unicode code point of the
character.

*string* must be a non-negative whole number naming a valid Unicode code
point. An error results if the code point is invalid for Unicode (for example,
if it is outside the Unicode range or in the surrogate range).
If *length* is specified under Unicode semantics, it may be **0** or
**1**. A length of **0** returns the null string; a length of
**1** is equivalent to omitting it.

**Examples:**
```
D2C('77')  == 'M' -- ASCII or Unicode
D2C('77', 1) == 'M'
D2C('12', 0) == ''
D2C('128293') == '🔥'
D2C('+77') == 'M' -- ASCII or Unicode
D2C('247') == '7' -- EBCDIC
D2C('0')   == '\textbackslash 0'
```



## D2X(n)

Decimal to hexadecimal.
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



## X2B(x)

Hexadecimal to binary.
Converts *string* (a string of at least one hexadecimal
characters) to an equivalent string of binary digits.
Hexadecimal characters may be any decimal digit character (0-9) or any
of the first six alphabetic characters (a-f), in either lowercase or
uppercase.
 *string* may be of any length; each hexadecimal character
with be converted to a string of four binary digits.
The returned string will have a length that is a multiple of four, and
will not include any blanks.

**Examples:**
```
X2B('C3')  == '11000011'
X2B('7')   == '0111'
X2B('1C1') == '000111000001'
```



## X2C(x)

Hexadecimal to coded character.
Converts the *string* (a string of hexadecimal characters) to
a single character (packs).
Hexadecimal characters may be any decimal digit character (0-9) or any
of the first six alphabetic characters (a-f), in either lowercase or
uppercase.

*string* must contain at least one hexadecimal character;
insignificant leading zeros are removed, and the string is then padded
with leading zeros if necessary to make a sufficient number of
hexadecimal digits to describe a character encoding for the
implementation.

An error results if the encoding described does not produce a valid
character for the implementation (for example, if it has more
significant bits than the implementation's encoding for characters).


**Examples:**
```
X2C('004D') == 'M' -- ASCII or Unicode
X2C('4d')   == 'M' -- ASCII or Unicode
X2C('A2')   == 's' -- EBCDIC
X2C('0')    == '\textbackslash 0'
```
 The  **d2c** function %% (see page refd2c)
 can be used to
convert a CREXX number to the encoding of a character.



## X2D(x)

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



## CENTER(s, n, pad)

returns a string of length *length* with *string*
centered in it, with *pad* characters added as necessary to
make up the required length.
*length* must be a non-negative whole number.
The default *pad* character is blank.
If the string is longer than *length*, it will be truncated at
both ends to fit.
If an odd number of characters are truncated or added, the right hand
end loses or gains one more character than the left hand end.

**Examples:**
```
CENTRE('ABC', 7)          == '  ABC  '
CENTER('ABC', 8, '-')      == '--ABC---'
CENTRE('The blue sky', 8) == 'e blue s'
CENTER('The blue sky', 7) == 'e blue '
```
**Note:** This function may be called either **centre** or **center**,
which avoids difficulties due to the difference between the British and
American spellings.



## CENTRE(s, n, pad)

returns a string of length *length* with *string*
centered in it, with *pad* characters added as necessary to
make up the required length.
*length* must be a non-negative whole number.
The default *pad* character is blank.
If the string is longer than *length*, it will be truncated at
both ends to fit.
If an odd number of characters are truncated or added, the right hand
end loses or gains one more character than the left hand end.

**Examples:**
```
CENTRE('ABC', 7)          == '  ABC  '
CENTER('ABC', 8, '-')      == '--ABC---'
CENTRE('The blue sky', 8) == 'e blue s'
CENTER('The blue sky', 7) == 'e blue '
```
**Note:** This function may be called either **centre** or **center**,
which avoids difficulties due to the difference between the British and
American spellings.



## COPIES(s, n)

returns *n* directly concatenated copies of
*string*.
*n* must be positive or 0; if 0, the null string is returned.

**Examples:**
```
COPIES('abc', 3) == 'abcabcabc'
COPIES('abc', 0) == ''
COPIES('', 2)    == ''
```



## DELSTR(s, start, length)

returns a copy of *string* with the sub-string of
*string* that begins at the **n*th character, and is
of length *length* characters, deleted.
If *length* is not specified, or is greater than the number of
characters from *n* to the end of the string, the rest of the
string is deleted (including the **n*th character).
*length* must be a non-negative whole number, and *n*
must be a positive whole number.  If *n* is greater than the
length of *string*, the string is returned unchanged.

**Examples:**
```
DELSTR('abcd', 3)    == 'ab'
DELSTR('abcde', 3, 2) == 'abe'
DELSTR('abcde', 6)   == 'abcde'
```



## DELWORD(s, start, n)

returns a copy of *string* with the sub-string of
*string* that starts at the **n*th word, and is of
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



## INSERT(s, target, position, length, pad)

inserts the string *new*, padded or truncated to length
*length*, into a copy of the target *string* after the
**n*th character; the string with any inserts is returned.
*length* and *n* must be a non-negative whole numbers.
If *n* is greater than the length of the target string,
padding is added before the *new* string also.
The default value for *n* is 0, which means insert before the
beginning of the string.  The default value for *length* is
the length of *new*.  The default *pad* character is
a blank.

**Examples:**
```
INSERT('123', 'abc')         == '123abc'
INSERT(' ', 'abcdef', 3)      == 'abc def'
INSERT('123', 'abc', 5, 6)     == 'abc  123   '
INSERT('123', 'abc', 5, 6, '+') == 'abc++123+++'
INSERT('123', 'abc', 0, 5, '-') == '123--abc'
```



## JUSTIFY(s, width, pad)




## LEFT(s, n, pad)

returns a string of length *length* containing the
left-most *length* characters of *string*.
The string is padded with *pad* characters (or truncated) on
the right as needed.
The default *pad* character is a blank.
*length* must be a non-negative whole number.
This function is exactly equivalent to
*string***.substr(1**, *length*
[, *pad*]**)**.

**Examples:**
```
LEFT('abc d', 8)     == 'abc d   '
LEFT('abc d', 8, '.') == 'abc d...'
LEFT('abc defg', 6)  == 'abc de'
```



## LENGTH(s)

returns the number of characters in *string*.

**Examples:**
```
LENGTH('abcdefgh') == 8
LENGTH('')         == 0
```



## LOWER(s)

returns a copy of *string* with any uppercase characters in
the sub-string of *string* that begins at the **n*th
character, and is of length *length* characters, replaced by
their lowercase equivalent.

*n* must be a positive whole number, and defaults to 1 (the
first character in *string*).  If *n* is greater than
the length of *string*, the string is returned unchanged.

*length* must be a non-negative whole number.
If *length* is not specified, or is greater than the number of
characters from *n* to the end of the string, the rest of the
string (including the **n*th character) is assumed.

**Examples:**
```
LOWER('SumA')      == 'suma'
LOWER('SumA', 2)   == 'Suma'
LOWER('SuMB', 1, 1) == 'suMB'
LOWER('SUMB', 2, 2) == 'SumB'
LOWER('')          == ''
```



## OVERLAY(s, target, position, length, pad)

overlays the string *new*, padded or truncated to length
*length*, onto a copy of the target *string* starting
at the **n*th character; the string with any overlays is
returned.  Overlays may extend beyond the end of the original
*string*.
If *length* is specified it must be a non-negative whole
number.
If *n* is greater than the length of
the target string, padding is added before the *new* string
also.
The default *pad* character is a blank, and the default value
for *n* is 1.
*n* must be greater than 0.
The default value for *length* is the length of *new*.

**Examples:**
```
OVERLAY(' ', 'abcdef', 3)      == 'ab def'
OVERLAY('.', 'abcdef', 3, 2)    == 'ab. ef'
OVERLAY('qq', 'abcd')         == 'qqcd'
OVERLAY('qq', 'abcd', 4)       == 'abcqq'
OVERLAY('123', 'abc', 5, 6, '+') == 'abc+123+++'
```



## POS(needle, haystack, start)

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


**Examples:**
```
POS('day', 'Saturday')    == 6
POS('x', 'abc def ghi')   == 0
POS(' ', 'abc def ghi')   == 4
POS(' ', 'abc def ghi', 5) == 8
```



## RIGHT(s, n, pad)

returns a string of length *length* containing the
right-most *length* characters of *string* -
that is, padded with *pad* characters (or truncated) on the
left as needed.  The default *pad* character is a blank.
*length* must be a non-negative whole number.

**Examples:**
```
RIGHT('abc  d', 8)  == '  abc  d'
RIGHT('abc def', 5) == 'c def'
RIGHT('12', 5, '0')  == '00012'
```



## SPACE(s, n, pad)

returns a copy of *string* with the blank-delimited words in
*string* formatted with *n* (and only *n*)
*pad* characters between each word.
*n* must be a non-negative whole number.
If *n* is 0, all blanks are removed.
Leading and trailing blanks are always removed.
The default for *n* is 1, and the default *pad*
character is a blank.

**Examples:**
```
SPACE('abc  def  ')        == 'abc def'
SPACE('  abc def ', 3)     == 'abc   def'
SPACE('abc  def  ', 1)     == 'abc def'
SPACE('abc  def  ', 0)     == 'abcdef'
SPACE('abc  def  ', 2, '+') == 'abc++def'
```



## STRIP(s, option, char)

returns a copy of *string* with Leading, Trailing, or Both
leading and trailing characters removed, when the first character of
*option* is L, T, or B respectively (these may be given in
either uppercase or lowercase).  The default is B.
The second argument, *char*, specifies the character to be
removed, with the default being a blank.
If given, *char* must be exactly one character long.

**Examples:**
```
STRIP('  ab c  ')        == 'ab c'
STRIP('  ab c  ', 'L')   == 'ab c  '
STRIP('  ab c  ', 't')   == '  ab c'
STRIP('12.70000', 't', 0) == '12.7'
STRIP('0012.700', 'b', 0) == '12.7'
```



## SUBSTR(s, start, length)

returns the sub-string of *string* that begins at the
**n*th character, and is of length *length*, padded
with *pad* characters if necessary.
*n* must be a positive whole number, and *length* must
be a non-negative whole number.
If *n* is greater than *string***.length**,
then only pad characters can be returned.
 If *length* is omitted it defaults to be the rest of the
string (or 0 if *n* is greater than the length of the string).
The default *pad* character is a blank.

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



## TRANSLATE(s, new, old)

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



## UPPER(s)

returns a copy of *string* with any lowercase characters in
the sub-string of *string* that begins at the **n*th
character, and is of length *length* characters, replaced by
their uppercase equivalent.

*n* must be a positive whole number, and defaults to 1 (the
first character in *string*).  If *n* is greater than
the length of *string*, the string is returned unchanged.

*length* must be a non-negative whole number.
If *length* is not specified, or is greater than the number of
characters from *n* to the end of the string, the rest of the
string (including the **n*th character) is assumed.

**Examples:**
```
UPPER('Fou-Baa')        == 'FOU-BAA'
UPPER('Mad Sheep')      == 'MAD SHEEP'
UPPER('Mad sheep', 5)   == 'Mad SHEEP'
UPPER('Mad sheep', 5, 1) == 'Mad Sheep'
UPPER('Mad sheep', 5, 4) == 'Mad SHEEp'
UPPER('tinganon', 1, 1)  == 'Tinganon'
UPPER('')               == ''
```



## VERIFY(s, reference, option, start)

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

An unsupported option currently returns the string `time invalid option`.


## DATE(oformat, date, iformat, osep, isep)

Converts dates between supported date formats. With no `date` argument, it
uses the current local date. Empty `oformat` or `iformat` values default to
`NORMAL`; `osep` and `isep` can override the output or input separator.

Input formats are matched by abbreviation and currently include `NORMAL`,
`STANDARD`, `ORDERED`, `EUROPEAN`, `GERMAN`, `USA`, `INTERNATIONAL`,
`QUALIFIED`, `JULIAN`, `BASE`, `UNIX`, and `EPOCH`.

Output formats are also matched by abbreviation and currently include
`NORMAL`, `XNORMAL`, `STANDARD`, `ORDERED`, `XORDERED`, `EUROPEAN`,
`XEUROPEAN`, `GERMAN`, `XGERMAN`, `USA`, `XUSA`, `INTERNATIONAL`,
`QUALIFIED`, `JULIAN`, `DAYS`, `WEEKDAY`, `MONTH`, `CENTURY`, `BASE`,
`UNIX`, `JDN`, `EPOCH`, `DEC`, and `XDEC`.


## SEQUENCE(from, to)

Returns the sequence of characters from `from` through `to`, using Unicode
codepoint values. It is the Unicode-capable replacement for byte-oriented
`XRANGE`; unlike `XRANGE`, it does not wrap around when `from` is greater than
`to`. The current implementation prints a diagnostic and returns `BAD` for a
descending range.


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



<!-- ## VALUE(symbol, newvalue, selector) -->


## VERSION()

Returns a string with implementation and build information supplied by the VM
`rxvers` instruction. The current string layout is:

```bash
platform bits crexx-version build-date
```

`platform` is one of the VM's compiled platform names, such as `linux`,
`windows`, `macOS`, `cms`, or `unknown`. `bits` is `32` or `64`. The function
does not currently expose endianness as a separate field.



## ABBREV(info, word, length)

returns 1 if *info* is equal to the leading characters of
*string* and *info* is not less than
the minimum length, *length*; 0 is returned
if either of these conditions is not met.
*length* must be a non-negative whole number; the default is
the length of *info*.


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




<!-- ## DIGITS() -->




<!-- ## FORM() -->




<!-- ## FUZZ() -->




<!-- ## QUEUED() -->
