# The .rexx class

This section describes the set of methods defined for the Rexx class, a
container which enables object-oriented string manipulation. These
methods implement the traditional *built-in functions*, and include
character manipulation, word manipulation, conversion, and arithmetic
functions.

Use of these methods needs import of the rexx package:

```rexx <!--rexxex1.crexx-->
import rexx
```

Rexx strings must be constructed with a factory constructor.

**General notes on the built-in methods:**

- Every method operates on the receiver's native `.string` value; this is
    referred to by the name *string* in the descriptions. For example, if
    the **word** method were invoked using the term:
```rexx <!--rexxex2.crexx-->
        v=.rexx("Three word phrase"); v.word(2)
```
    then in the description of **word** the name *string* refers to the
    string  
	\"**Three word phrase**\", and the name *n* refers to the
    integer **2**.

- Method arguments use the native types shown by their signatures: positions,
    lengths, counts, and widths are `.int`; text and pad arguments are
    `.string`. Transformations normally return `.rexx` for fluent chaining,
    while observations, positions, counts, and predicates return native scalar
    types. `stringValue()` or `toString()` extracts the wrapped string.

- The first parenthesis in a method call must immediately follow the
    name of the method, with no space in between.

<!-- - The parentheses in a method call can be omitted if no arguments are -->
<!--     required and the method call is part of a [^1] -->

- A position in a string is the number of a character in the string,
    where the first character is at position 1, *etc.*

<!-- -  Where arguments are optional, commas may only be included between -->
<!--     arguments that are present (that is, trailing commas in argument -->
<!--     lists are not permitted). -->

-  A *pad* argument, if specified, must be exactly one character long.

- If a method has a sub-option selected by the first character of a
    string, that character may be in upper or lowercase.

- Conversion between character encodings and decimal or hexadecimal is
    dependent on the machine representation (encoding) of characters and
    hence will return appropriately different results for Unicode,
    ASCII, and EBCDIC.

## abbrev(info \[,length\])

returns 1 if *info* is equal to the leading characters of *string* and
*info* is not less than the minimum length, *length*; 0 is returned if
either of these conditions is not met. *length* must be a non-negative
whole number; the default is the length of *info*. **Examples:**

```rexx <!--rexxex3.crexx-->
    v=.rexx('Print'); v.abbrev('Pri')   == 1
    w=.rexx('PRINT'); w.abbrev('Pri')   == 0
    w=.rexx('PRINT'); w.abbrev('PRI',4) == 0
    w=.rexx('PRINT'); w.abbrev('PRY')   == 0
    w=.rexx('PRINT'); w.abbrev('')      == 1
    w=.rexx('PRINT'); w.abbrev('',1)    == 0
```

**Note:** A null string will always match if a length of 0 (or the
default) is used. This allows a default keyword to be selected
automatically if desired. **Example:**

```rexx <!--rexxex4.crexx-->
    say 'Enter option:';  option=ask
    select  /* keyword1 is to be the default */
      v=.rexx('keyword1'); when v.abbrev(option) then ...
      w=.rexx('keyword2'); when w.abbrev(option) then ...
         ...
      otherwise ...
      end
```

## abs()

returns the absolute value of *string*, which must be a number. Any sign
is removed from the number, and it is then formatted by adding zero with
a digits setting that is either nine or, if greater, the number of
digits in the mantissa of the number (excluding leading insignificant
zeros). Scientific notation is used, if necessary.

**Examples:**
```rexx <!--rexxex5.crexx-->
    v=.rexx('12.3'); v.abs              == 12.3
    w=.rexx(' -0.307'); w.abs           == 0.307
    x=.rexx('123.45E+16'); x.abs        == 1.2345E+18
    y=.rexx('- 1234567.7654321'); y.abs == 1234567.7654321
```

The method accepts leading/trailing Unicode whitespace and whitespace between
an initial sign and the numeric text. Other invalid numeric text raises
`CONVERSION_ERROR`. The typed Level B function and Classic Level C BIF remain
separate contracts: [`abs.md`](../../../lib/rxfnsb/rexx/abs.md) and
[`rxfnsc/abs.md`](../../../lib/rxfnsc/abs.md).

## b2d()

Binary to decimal. Converts the receiver's binary digits to a non-negative
Level B integer. The empty string returns 0. Standard nibble-group blanks are
accepted; invalid text signals `INVALID_ARGUMENTS`. Results must fit the signed
64-bit `.int` range or `OVERFLOW_UNDERFLOW` is signalled.

**Examples:**
```rexx <!--rexxex6.crexx-->
    v=.rexx('01110'); v.b2d == 14 
    w=.rexx('10000001'); w.b2d == 129 
    x=.rexx('111110000001'); x.b2d == 3969 
    y=.rexx('1111111110000001'); y.b2d == 65409 
    z=.rexx('1100011011110000'); z.b2d == 50928 
```

The callable has no signed-width argument. See the stable
[Level B `b2d` contract](../../../lib/rxfnsb/rexx/b2d.md).

## b2x()

Binary to hexadecimal. Converts *string*, a string of at least one
binary (**0** and/or **1**) digits, to an equivalent string of
hexadecimal characters. The returned string will use uppercase Roman
letters for the values A-F, and will not include any blanks. If the
number of binary digits in the string is not a multiple of four, then up
to three **'0'** digits will be added on the left before conversion to
make a total that is a multiple of four.

**Examples:**
```rexx <!--rexxex8.crexx-->
    v=.rexx('11000011'); v.b2x  == 'C3'
    w=.rexx('10111'); w.b2x     == '17'
    x=.rexx('0101'); x.b2x      == '5'
    y=.rexx('101'); y.b2x       == '5'
    z=.rexx('111110000'); z.b2x == '1F0'
```

## center(length \[,pad\])

*or*

## centre(length \[,pad\])

returns a string of length *length* with *string* centered in it, with
*pad* characters added as necessary to make up the required length.
*length* must be a non-negative whole number. The default *pad*
character is blank. If the string is longer than *length*, it will be
truncated at both ends to fit. If an odd number of characters are
truncated or added, the right hand end loses or gains one more character
than the left hand end.

**Examples:**
```rexx <!--rexxex9.crexx-->
    v=.rexx('ABC'); v.centre(7)          == '  ABC  '
    v=.rexx('ABC'); v.center(8,'-')      == '--ABC---'
    w=.rexx('The blue sky'); w.centre(8) == 'e blue s'
    w=.rexx('The blue sky'); w.center(7) == 'e blue '
```

**Note:** This method may be called either **centre** or **center**,
which avoids difficulties due to the difference between the British and
American spellings.

## changestr(needle, new)

returns a copy of *string* in which each occurrence of the *needle*
string is replaced by the *new* string. Each unique (non-overlapping)
occurrence of the *needle* string is changed, searching from left to
right and starting from the first (leftmost) position in *string*. Only
the original *string* is searched for the *needle*, and each character
in *string* can only be included in one match of the *needle*.

If the *needle* is the null string, the result is a copy of *string*,
unchanged.

**Examples:**
```rexx <!--rexxex10.crexx-->
    v=.rexx('elephant'); v.changestr('e','X')    == 'XlXphant'
    v=.rexx('elephant'); v.changestr('ph','X')   == 'eleXant'
    v=.rexx('elephant'); v.changestr('ph','hph') == 'elehphant'
    v=.rexx('elephant'); v.changestr('e','')     == 'lphant'
    v=.rexx('elephant'); v.changestr('','!!')    == 'elephant'
```

## compare(target \[,pad\])

returns 0 if *string* and *target* are the same. If they are not, the
returned number is positive and is the position of the first character
that is not the same in both strings. If one string is shorter than the
other, one or more *pad* characters are added on the right to make it
the same length for the comparison. The default *pad* character is a
blank.

**Examples:**
```rexx <!--rexxex11.crexx-->
    v=.rexx('abc'); v.compare('abc')      == 0
    v=.rexx('abc'); v.compare('ak')       == 2
    w=.rexx('ab '); w.compare('ab')       == 0
    w=.rexx('ab '); w.compare('ab',' ')   == 0
    w=.rexx('ab '); w.compare('ab','x')   == 3
    x=.rexx('ab-- '); x.compare('ab','-') == 5
```

## copies(n)

returns *n* directly concatenated copies of *string*. *n* must be
positive or 0; if 0, the null string is returned.

**Examples:**
```rexx <!--rexxex12.crexx-->
    v=.rexx('abc'); v.copies(3) == 'abcabcabc'
    v=.rexx('abc'); v.copies(0) == ''
    w=.rexx(''); w.copies(2)    == ''
```

## countstr(needle)

returns the count of non-overlapping occurrences of the *needle* string
in *string*, searching from left to right and starting from the first
(leftmost) position in *string*.

If the *needle* is the null string, **0** is returned.

**Examples:**
```rexx <!--rexxex13.crexx-->
    v=.rexx('elephant'); v.countstr('e')  == '2'
    v=.rexx('elephant'); v.countstr('ph') == '1'
    v=.rexx('elephant'); v.countstr('')   == '0'
```

The **changestr** method can be used to change occurrences of *needle*
to some other string.

## c2d()

Coded character to decimal. Converts the Unicode code point of the
character in *string* (which must be exactly one character) to its
native `.int` representation. Empty or multi-character receivers raise
`CONVERSION_ERROR`.

**Examples:**
```rexx <!--rexxex14.crexx-->
    v=.rexx('M'); v.c2d  == 77
    w=.rexx('α'); w.c2d  == 945
    x=.rexx('🔥'); x.c2d == 128293
    y=.rexx('00'x); y.c2d == 0
```

The **c2x** method can be used to convert the encoding of a character to
a hexadecimal representation.

## c2x()

Coded characters to hexadecimal. Converts every character in the receiver to
two uppercase hexadecimal digits. The empty receiver returns an empty Rexx
string; multi-character receivers are valid and leading zero digits are
retained. The method uses the native Level B C2X contract, including its
established low-byte behavior for Unicode code points beyond one byte.

**Examples:**
```rexx <!--rexxex15.crexx-->
    v=.rexx('M'); v.c2x     == '4D'
    w=.rexx('72s'); w.c2x   == '373273' -- ASCII/Unicode build
    x=.rexx('0123'x); x.c2x == '0123'
    y=.rexx(''); y.c2x      == ''
```
The **c2d** method can be used to convert the encoding of a character to
a decimal number.

## datatype(option)

returns 1 if *string* matches the description requested with the
*option*, or 0 otherwise. If *string* is the null string, 0 is always
returned.

Only the first character of *option* is significant, and it may be in
either uppercase or lowercase. The following *option* characters are
recognized:

A

:   (Alphanumeric); returns 1 if *string*
    only contains characters from the ranges \"a-z\", \"A-Z\", and
    \"0-9\".

B

:   (Binary); returns 1 if *string* only
    contains the characters \"0\" and/or \"1\".

D

:   (Digits); returns 1 if *string* only
    contains characters from the range \"0-9\".

L

:   (Lowercase); returns 1 if *string* only
    contains characters from the range \"a-z\".

M

:   (Mixed case); returns 1 if *string* only
    contains characters from the ranges \"a-z\" and \"A-Z\".

N

:   (Number); returns 1 if *string* is a
    syntactically valid number that could be added to **'0'** without
    error,

S

:   (Symbol); returns 1 if *string* only
    contains characters that are valid in non-numeric symbols (the
    alphanumeric characters and underscore), and does not start with a
    digit. Note that both uppercase and lowercase letters are permitted.

U

:   (Uppercase); returns 1 if *string* only
    contains characters from the range \"A-Z\".

W

:   (Whole Number); returns 1 if *string* is
    a syntactically valid number that can be added to **'0'** without
    error, and whose decimal part after that addition, with no rounding,
    is zero.

X

:   (heXadecimal); returns 1 if *string* only
    contains characters from the ranges \"a-f\", \"A-F\", and \"0-9\".

**Examples:**
```rexx <!--rexxex16.crexx-->
    v=.rexx('101'); v.datatype('B')    == 1
    w=.rexx('12.3'); w.datatype('D')   == 0
    w=.rexx('12.3'); w.datatype('N')   == 1
    w=.rexx('12.3'); w.datatype('W')   == 0
    x=.rexx('LaArca'); x.datatype('M') == 1
    y=.rexx(''); y.datatype('M')       == 0
    z=.rexx('Llanes'); z.datatype('L') == 0
    v1=.rexx('3 d'); v1.datatype('s')    == 0
    v2=.rexx('BCd3'); v2.datatype('X')   == 1
    v3=.rexx('BCgd3'); v3.datatype('X')  == 0
```

**Note:** The **datatype** method tests the meaning of the characters in
a string, independent of the encoding of those characters. Extra letters
and Extra digits cause **datatype** to return 0 except for the number
tests (\"**N**\" and \"**W**\"), which treat extra digits whose value is
in the range 0-9 as though they were the corresponding Arabic numeral.

## delstr(n \[,length\])

returns a copy of *string* with the sub-string of *string* that begins
at the *nth* character, and is of length *length* characters, deleted.
If *length* is not specified, or is greater than the number of
characters from *n* to the end of the string, the rest of the string is
deleted (including the *nth* character). *length* must be a non-negative
whole number, and *n* must be a positive whole number. If *n* is greater
than the length of *string*, the string is returned unchanged. An explicitly
supplied zero length also returns the string unchanged. Invalid values raise
`INVALID_ARGUMENTS`.

**Examples:**
```rexx
    v=.rexx('abcd'); v.delstr(3)    == 'ab'
    w=.rexx('abcde'); w.delstr(3,2) == 'abe'
    w=.rexx('abcde'); w.delstr(6)   == 'abcde'
```

See the typed [`delstr` contract](../../../lib/rxfnsb/rexx/delstr.md) and the
separate Classic [`rxfnsc/delstr` contract](../../../lib/rxfnsc/delstr.md).

## delword(n \[,length\])

returns a copy of *string* with the sub-string of *string* that starts
at the *nth* word, and is of length *length* blank-delimited words,
deleted. If *length* is not specified, or is greater than number of
remaining words in the string, it defaults to be the remaining words in
the string (including the *nth* word). *length* must be a non-negative
whole number, and *n* must be a positive whole number. If *n* is greater
than the number of words in *string*, the string is returned unchanged.
The string deleted includes any blanks following the final word
involved, but none of the blanks preceding the first word involved.

**Examples:**
```rexx <!--rexxex17.crexx-->
    v=.rexx('Now is the  time'); v.delword(2,2) == 'Now time'
    w=.rexx('Now is the time '); w.delword(3)   == 'Now is '
    x=.rexx('Now  time'); x.delword(5)          == 'Now  time'
```

## d2b()

Returns the minimal string of `0` and `1` characters representing the
receiver's non-negative native integer value. Zero returns `"0"`; other
results have no leading zeroes. A negative receiver raises
`INVALID_ARGUMENTS`. The method has no width argument and does not perform
signed extension or truncation.

**Examples:**
```rexx <!--rexxex18.crexx-->
    v=.rexx('0'); v.d2b     == '0'
    w=.rexx('9'); w.d2b     == '1001'
    x=.rexx('19'); x.d2b    == '10011'
    y=.rexx('129'); y.d2b   == '10000001'
```

The native function contract is documented in
[`d2b.md`](../../../lib/rxfnsb/rexx/d2b.md).

## d2c()

Converts the receiver's whole-number text to one Unicode character. The value
must name a Unicode scalar from `0` through `1114111` (`U+10FFFF`), excluding
the surrogate range. Invalid values raise `CONVERSION_ERROR`.

The `.Rexx.d2c()` method has no length argument. The underlying typed Level B
function also exposes a separate optional zero-or-one output length; callers
that need that native surface should call `rxfnsb.d2c` directly.

**Examples:**
```rexx <!--rexxex19.crexx-->
    v=.rexx('77'); v.d2c().toString() == 'M'
    w=.rexx('+77'); w.d2c().toString() == 'M'
    x=.rexx('945'); x.d2c().toString() == 'α'
    y=.rexx('128293'); y.d2c().toString() == '🔥'
```

The native function contract is documented in
[`d2c.md`](../../../lib/rxfnsb/rexx/d2c.md). Classic Level C D2C is a different
configuration-coded BIF documented in
[`rxfnsc/d2c.md`](../../../lib/rxfnsc/d2c.md).

## d2x()

Converts the receiver's non-negative native-integer text to minimal uppercase
hexadecimal. Zero returns `"0"`; a negative receiver raises
`INVALID_ARGUMENTS` because the method has no width from which to derive a
signed representation.

The `.Rexx.d2x()` method accepts no arguments. The underlying typed Level B
function also exposes an optional exact output width; callers needing padding,
truncation, or signed twos-complement should call `rxfnsb.d2x` directly.

**Examples:**
```rexx <!--rexxex20.crexx-->
    v=.rexx('0'); v.d2x().toString() == '0'
    w=.rexx('9'); w.d2x().toString() == '9'
    x=.rexx('129'); x.d2x().toString() == '81'
```

The native function contract is documented in
[`d2x.md`](../../../lib/rxfnsb/rexx/d2x.md). Classic Level C D2X accepts
arbitrary caller-context whole numbers and is documented in
[`rxfnsc/d2x.md`](../../../lib/rxfnsc/d2x.md).

## exists(index)

returns 1 if *index* names a sub-value of *string* that has explicitly
been assigned a value, or 0 otherwise.

**Example:** Following the instructions:
```rexx <!--rexxex21.crexx-->
    vowel=0
    vowel['a']=1
    vowel['b']=1
    vowel['b']=null -- drops previous assignment
```

then:

```rexx <!--rexxex22.crexx-->
    vowel.exists('a') == '1'
    vowel.exists('b') == '0'
    vowel.exists('c') == '0'
```

## format(\[before \[,after \[,expp \[,expt\]\]\]\])

Formats the receiver as a decimal using the caller's `NUMERIC DIGITS` and
`NUMERIC FORM`. Leading and trailing whitespace, including whitespace between
an initial sign and the numeric text, is accepted. Other invalid numeric text
raises `CONVERSION_ERROR`.

All four controls are optional non-negative integers and omission is
significant. Use an omitted argument slot, not `null`, when supplying an option
to its right.

- *before* is the width of the integer part, including a possible minus sign.
- *after* is the exact number of fractional digits; the value is rounded and
  zero-filled to fit.
- *expp* is the exponent digit width. An explicit zero suppresses exponent
  notation; a nonzero width leaves a blank exponent field when the exponent is
  zero.
- *expt* is the trigger for exponential notation. When omitted while *expp* is
  present, the current `NUMERIC DIGITS` value is used.

The current `NUMERIC FORM` setting selects scientific or engineering layout;
there is no separate `exform` method argument. A negative control or a field
that cannot fit raises `INVALID_ARGUMENTS`.

```rexx <!--rexxex23.crexx-->
    v=.rexx(' - 12.73'); v.format().toString()       == '-12.73'
    w=.rexx('1.75'); w.format(4,1).toString()        == '   1.8'
    x=.rexx('12345.73'); x.format(,,2,2).toString()  == '1.234573E+04'
    y=.rexx('12345.73'); y.format(,3,,0).toString()  == '1.235E+4'
```

The typed formatter is documented in
[`format.md`](../../../lib/rxfnsb/rexx/format.md). Classic Level C FORMAT has
the separate RexxValue contract in
[`rxfnsc/format.md`](../../../lib/rxfnsc/format.md).

## insert(new \[,n \[,length \[,pad\]\]\])

inserts the string *new*, padded or truncated to length *length*, into a
copy of the target *string* after the *nth* character; the string with
any inserts is returned. *length* and *n* must be a non-negative whole
numbers. If *n* is greater than the length of the target string, padding
is added before the *new* string also. The default value for *n* is 0,
which means insert before the beginning of the string. The default value
for *length* is the length of *new*. The default *pad* character is a
blank. Invalid positions, lengths, or pads raise `INVALID_ARGUMENTS`.

**Examples:**
```rexx <!--rexxex25.crexx-->
    v=.rexx('abc'); v.insert('123')         == '123abc'
    w=.rexx('abcdef'); w.insert(' ',3)      == 'abc def'
    v=.rexx('abc'); v.insert('123',5,6)     == 'abc  123   '
    v=.rexx('abc'); v.insert('123',5,6,'+') == 'abc++123+++'
    v=.rexx('abc'); v.insert('123',0,5,'-') == '123--abc'
```

The receiver is always the target and *new* is the first method argument. See
the typed [`insert` contract](../../../lib/rxfnsb/rexx/insert.md) and separate
Classic [`rxfnsc/insert` contract](../../../lib/rxfnsc/insert.md).

## lastpos(needle \[,start\])

returns the position of the last occurrence of the string *needle* in
*string* (the \"haystack\"), searching from right to left. If the string
*needle* is not found, or is the null string, 0 is returned. By default
the search starts at the last character of *string* and scans backwards.
This may be overridden by specifying *start*, the point at which to
start the backwards scan. *start* must be a positive whole number, and
defaults to the value *string***.length** if larger than that value or
if not specified (with a minimum default value of one). An explicitly supplied
zero or negative *start* raises `INVALID_ARGUMENTS`.

**Examples:**
```rexx <!--rexxex26.crexx-->
    v=.rexx('abc def ghi'); v.lastpos(' ')   == 8
    v=.rexx('abc def ghi'); v.lastpos(' ',7) == 4
    w=.rexx('abcdefghi'); w.lastpos(' ')     == 0
    w=.rexx('abcdefghi'); w.lastpos('cd')    == 3
    x=.rexx(''); x.lastpos('?')              == 0
```

See the typed [`lastpos` contract](../../../lib/rxfnsb/rexx/lastpos.md) and the
separate Classic [`rxfnsc/lastpos` contract](../../../lib/rxfnsc/lastpos.md).

## left(length \[,pad\])

returns a string of length *length* containing the left-most *length*
characters of *string*. The string is padded with *pad* characters (or
truncated) on the right as needed. The default *pad* character is a
blank. *length* must be a non-negative whole number. This method is
exactly equivalent to *string***.substr(1**, *length* \[, *pad*\]**)**.
Invalid lengths or pads raise `INVALID_ARGUMENTS`.

**Examples:**
```rexx <!--rexxex27.crexx-->
    v=.rexx('abc d'); v.left(8)     == 'abc d   '
    v=.rexx('abc d'); v.left(8,'.') == 'abc d...'
    w=.rexx('abc defg'); w.left(6)  == 'abc de'
```

See the typed [`left` contract](../../../lib/rxfnsb/rexx/left.md).

## length()

returns the number of characters in *string*.

**Examples:**
```rexx <!--rexxex28.crexx-->
    v=.rexx('abcdefgh'); v.length == 8
    w=.rexx(''); w.length         == 0
```

## linein(name)

reads a UTF text line from the stream named by the first argument and
returns it without the line terminator. A final line terminator at
physical end-of-file does not create an extra empty record, although
physical blank lines are preserved. This is a Level B `.string` text
function, not a binary byte input function. The companion `lines(name)`
predicate returns `-1` when the stream cannot be opened, `0` at end of
file, and `1` when a record can be read.

## lineout(name,string)

returns 0 after writing the second argument as UTF text followed by a
newline to the stream named by the first argument. When the second
argument is omitted, the named stream is closed. This is a Level B
`.string` text function, not a binary byte output function. Separate
binary file BIFs are expected to use `.binary` values.

## lower()

Returns a copy of the complete receiver with its Unicode uppercase characters
converted to lowercase. The `.Rexx` method has no start or length extension.

**Examples:**
```rexx <!--rexxex29.crexx-->
    v=.rexx('SumA'); v.lower().toString() == 'suma'
    w=.rexx('ÉCOLE'); w.lower().toString() == 'école'
    x=.rexx(''); x.lower().toString() == ''
```

See the native [`lower` contract](../../../lib/rxfnsb/rexx/lower.md).

## max(number)

returns the larger of *string* and *number*, which must both be numbers.
If they compare equal (that is, when subtracted, the result is 0), then
*string* is selected for the result.

The comparison is effected using a numerical comparison with a digits
setting that is either nine or, if greater, the larger of the number of
digits in the mantissas of the two numbers (excluding leading
insignificant zeros).

The selected result is formatted by adding zero to the selected number
with a digits setting that is either nine or, if greater, the number of
digits in the mantissa of the number (excluding leading insignificant
zeros). Scientific notation is used, if necessary.

**Examples:**
```rexx <!--rexxex30.crexx-->
    0.max(1)          ==1
    v=.rexx('-1'); v.max(1)       ==1
    w=.rexx('+1'); w.max(-1)      ==1
    x=.rexx('1.0'); x.max(1.00)   =='1.0'
    y=.rexx('1.00'); y.max(1.0)   =='1.00'
    z=.rexx('123456700000'); z.max(1234567E+5)   == '123456700000'
    v1=.rexx('1234567E+5'); v1.max('123456700000') == '1.234567E+11'
```

## min(number)

returns the smaller of *string* and *number*, which must both be
numbers. If they compare equal (that is, when subtracted, the result is
0), then *string* is selected for the result.

The comparison is effected using a numerical comparison with a digits
setting that is either nine or, if greater, the larger of the number of
digits in the mantissas of the two numbers (excluding leading
insignificant zeros).

The selected result is formatted by adding zero to the selected number
with a digits setting that is either nine or, if greater, the number of
digits in the mantissa of the number (excluding leading insignificant
zeros). Scientific notation is used, if necessary.

**Examples:**
```rexx <!--rexxex31.crexx-->
    0.min(1)          ==0
    v=.rexx('-1'); v.min(1)       =='-1'
    w=.rexx('+1'); w.min(-1)      =='-1'
    x=.rexx('1.0'); x.min(1.00)   =='1.0'
    y=.rexx('1.00'); y.min(1.0)   =='1.00'
    z=.rexx('123456700000'); z.min(1234567E+5)   == '123456700000'
    v1=.rexx('1234567E+5'); v1.min('123456700000') == '1.234567E+11'
```

## overlay(new \[,n \[,length \[,pad\]\]\])

overlays the string *new*, padded or truncated to length *length*, onto
a copy of the target *string* starting at the *nth* character; the
string with any overlays is returned. Overlays may extend beyond the end
of the original *string*. If *length* is specified it must be a
non-negative whole number. If *n* is greater than the length of the
target string, padding is added before the *new* string also. The
default *pad* character is a blank, and the default value for *n* is 1.
*n* must be greater than 0. The default value for *length* is the length
of *new*. Invalid starts, lengths, or pads raise `INVALID_ARGUMENTS`.

**Examples:**
```rexx <!--rexxex32.crexx-->
    v=.rexx('abcdef'); v.overlay(' ',3)      == 'ab def'
    v=.rexx('abcdef'); v.overlay('.',3,2)    == 'ab. ef'
    w=.rexx('abcd'); w.overlay('qq')         == 'qqcd'
    w=.rexx('abcd'); w.overlay('qq',4)       == 'abcqq'
    x=.rexx('abc'); x.overlay('123',5,6,'+') == 'abc+123+++'
```

The receiver is always the target and *new* is the first method argument. See
the typed [`overlay` contract](../../../lib/rxfnsb/rexx/overlay.md) and separate
Classic [`rxfnsc/overlay` contract](../../../lib/rxfnsc/overlay.md).

## pos(needle \[,start\])

returns the position of the string *needle*, in *string* (the
\"haystack\"), searching from left to right. If the string *needle* is
not found, or is the null string, 0 is returned. By default the search
starts at the first character of *string* (that is, *start* has the
value 1). This may be overridden by specifying *start* (which must be a
positive whole number), the point at which to start the search; if
*start* is greater than the length of *string* then 0 is returned. Zero or
negative *start* values raise `INVALID_ARGUMENTS`.
**Examples:**
```rexx <!--rexxex33.crexx-->
    v=.rexx('Saturday'); v.pos('day')    == 6
    w=.rexx('abc def ghi'); w.pos('x')   == 0
    w=.rexx('abc def ghi'); w.pos(' ')   == 4
    w=.rexx('abc def ghi'); w.pos(' ',5) == 8
```

See the typed [`pos` contract](../../../lib/rxfnsb/rexx/pos.md).

## reverse()

returns a copy of *string*, swapped end for end.

**Examples:**
```rexx <!--rexxex34.crexx-->
    v=.rexx('ABc.'); v.reverse        == '.cBA'
    w=.rexx('XYZ '); w.reverse        == ' ZYX'
    x=.rexx('Tranquility'); x.reverse == 'ytiliuqnarT'
```

## right(length \[,pad\])

returns a string of length *length* containing the right-most *length*
characters of *string* - that is, padded with *pad* characters (or
truncated) on the left as needed. The default *pad* character is a
blank. *length* must be a non-negative whole number. Invalid lengths or pads
raise `INVALID_ARGUMENTS`.

**Examples:**
```rexx <!--rexxex35.crexx-->
    v=.rexx('abc  d'); v.right(8)  == '  abc  d'
    w=.rexx('abc def'); w.right(5) == 'c def'
    x=.rexx('12'); x.right(5,'0')  == '00012'
```

See the typed [`right` contract](../../../lib/rxfnsb/rexx/right.md).

## sequence(final)

returns a string of all characters, in ascending order of encoding,
between and including the character in *string* and the character in
*final*. *string* and *final* must be single characters; if *string* is
greater than *final*, an error is reported.

**Examples:**
```rexx <!--rexxex36.crexx-->
    v=.rexx('a'); v.sequence('f')           == 'abcdef'
    w=.rexx('\\0'); w.sequence('\\x03')       == '\\x00\\x01\\x02\\x03'
    x=.rexx('\\ufffe'); x.sequence('\\uffff') == '\\ufffe\\uffff'
```

## xrange(final)

returns the inclusive legacy byte-domain range from the single character in
*string* through the single character in *final*. Endpoints are limited to
U+0000 through U+00FF, and a descending range wraps at U+00FF. Invalid
endpoints signal `INVALID_ARGUMENTS`. Use `sequence` for non-wrapping Unicode
ranges.

```rexx
    v=.rexx('a'); v.xrange('f').toString() == 'abcdef'
```

The native contract is documented in
[`xrange.md`](../../../lib/rxfnsb/rexx/xrange.md). Classic Level C XRANGE uses
configuration-coded characters and is documented separately in
[`rxfnsc/xrange.md`](../../../lib/rxfnsc/xrange.md).

## sign()

returns a number that indicates the sign of *string*, which must be a
number. *string* is first formatted, just as though the operation
\"**string+0**\" had been carried out with sufficient digits to avoid
rounding. If the number then starts with **'-'** then **'-1'** is
returned; if it is **'0'** then **'0'** is returned; and otherwise
**'1'** is returned.

**Examples:**
```rexx <!--rexxex37.crexx-->
    v=.rexx('12.3'); v.sign    ==  1
    w=.rexx('0.0'); w.sign     ==  0
    x=.rexx(' -0.307'); x.sign == -1
```

The method returns a `.rexx` value containing `-1`, `0`, or `1`, preserving
fluent class behavior. It accepts the same Classic sign-whitespace form as
`abs()` and raises `CONVERSION_ERROR` for invalid numeric text. See the typed
[`sign` contract](../../../lib/rxfnsb/rexx/sign.md) and separate Classic
[`rxfnsc/sign` contract](../../../lib/rxfnsc/sign.md).

## space(\[n \[,pad\]\])

returns a copy of *string* with the blank-delimited words in *string*
formatted with *n* (and only *n*) *pad* characters between each word.
*n* must be a non-negative whole number. If *n* is 0, all blanks are
removed. Leading and trailing blanks are always removed. The default for
*n* is 1, and the default *pad* character is a blank.

**Examples:**
```rexx <!--rexxex38.crexx-->
    v=.rexx('abc  def  '); v.space        == 'abc def'
    w=.rexx('  abc def '); w.space(3)     == 'abc   def'
    v=.rexx('abc  def  '); v.space(1)     == 'abc def'
    v=.rexx('abc  def  '); v.space(0)     == 'abcdef'
    v=.rexx('abc  def  '); v.space(2,'+') == 'abc++def'
```

## strip(\[option \[,char\]\]\])

returns a copy of *string* with Leading, Trailing, or Both leading and
trailing characters removed, when the first character of *option* is L,
T, or B respectively (these may be given in either uppercase or
lowercase). The default is B. When *char* is omitted, the complete Unicode
whitespace set is removed. When supplied, *char* must be exactly one character
and only that character is removed; an explicit blank therefore differs from
omission.

**Examples:**
```rexx <!--rexxex39.crexx-->
    v=.rexx('  ab c  '); v.strip        == 'ab c'
    v=.rexx('  ab c  '); v.strip('L')   == 'ab c  '
    v=.rexx('  ab c  '); v.strip('t')   == '  ab c'
    w=.rexx('12.70000'); w.strip('t','0') == '12.7'
    x=.rexx('0012.700'); x.strip('b','0') == '12.7'
    y=.rexx('　ab　'); y.strip() == 'ab'
```

See the typed [`strip` contract](../../../lib/rxfnsb/rexx/strip.md) and the
separate Classic [`rxfnsc/strip` contract](../../../lib/rxfnsc/strip.md).

## substr(n \[,length \[,pad\]\])

returns the sub-string of *string* that begins at the *nth* character,
and is of length *length*, padded with *pad* characters if necessary.
*n* must be a positive whole number, and *length* must be a non-negative
whole number. If *n* is greater than *string***.length**, then only pad
characters can be returned. If *length* is omitted it defaults to be the
rest of the string (or 0 if *n* is greater than the length of the
string). The default *pad* character is a blank. Invalid starts, supplied
lengths, or pads raise `INVALID_ARGUMENTS`.

**Examples:**
```rexx <!--rexxex40.crexx-->
    v=.rexx('abc'); v.substr(2)       == 'bc'
    v=.rexx('abc'); v.substr(2,4)     == 'bc  '
    v=.rexx('abc'); v.substr(5,4)     == '    '
    v=.rexx('abc'); v.substr(2,6,'.') == 'bc....'
    v=.rexx('abc'); v.substr(5,6,'.') == '......'
```

**Note:** In some situations the positional (numeric) patterns of
parsing templates are more convenient for selecting sub-strings,
especially if more than one sub-string is to be extracted from a string.

See the typed [`substr` contract](../../../lib/rxfnsb/rexx/substr.md) and the
separate Classic [`rxfnsc/substr` contract](../../../lib/rxfnsc/substr.md).

## subword(n \[,length\])

returns the sub-string of *string* that starts at the *nth* word, and is
up to *length* blank-delimited words long. *n* must be a positive whole
number; if greater than the number of words in the string then the null
string is returned. *length* must be a non-negative whole number. If
*length* is omitted it defaults to be the remaining words in the string.
The returned string will never have leading or trailing blanks, but will
include all blanks between the selected words.

**Examples:**
```rexx <!--rexxex41.crexx-->
    v=.rexx('Now is the  time'); v.subword(2,2) == 'is the'
    v=.rexx('Now is the  time'); v.subword(3)   == 'the  time'
    v=.rexx('Now is the  time'); v.subword(5)   == ''
```

## translate(tableo, tablei \[,pad\])

returns a copy of *string* with each character in *string* either
unchanged or translated to another character.

The **translate** method acts by searching the input translate table,
*tablei*, for each character in *string*. If the character is found in
*tablei* (the first, leftmost, occurrence being used if there are
duplicates) then the corresponding character in the same position in the
output translate table, *tableo*, is used in the result string;
otherwise the original character found in *string* is used. The result
string is always the same length as *string*.

The translate tables may be of any length, including the null string.
The output table, *tableo*, is padded with *pad* or truncated on the
right as necessary to be the same length as *tablei*. The default *pad*
is a blank.

**Examples:**
```rexx <!--rexxex42.crexx-->
    v=.rexx('abbc'); v.translate('&','b')           == 'a&&c'
    w=.rexx('abcdef'); w.translate('12','ec')       == 'ab2d1f'
    w=.rexx('abcdef'); w.translate('12','abcd','.') == '12..ef'
    x=.rexx('4123'); x.translate('abcd','1234')     == 'dabc'
    x=.rexx('4123'); x.translate('hods','1234')     == 'shod'
```

**Note:** The last two examples show how the **translate** method may be
used to move around the characters in a string. In these examples, any
4-character string could be specified as the first argument and its last
character would be moved to the beginning of the string. Similarly, the
term:
```rexx <!--rexxex43.crexx-->
    v=.rexx('gh.ef.abcd'); v.translate(19970827,'abcdefgh')
```

(which returns \"**27.08.1997**\") shows how a string (in this case
perhaps a date) might be re-formatted and merged with other characters
using the **translate** method.

## trunc(\[n\])

returns the integer part of *string*, which must be a number, with *n*
decimal places (digits after the decimal point). *n* must be a
non-negative whole number, and defaults to zero.

The number *string* is formatted by adding zero with a digits setting
that is either nine or, if greater, the number of digits in the mantissa
of the number (excluding leading insignificant zeros). It is then
truncated to *n* decimal places (or trailing zeros are added if needed
to make up the specified length). If *n* is 0 (the default) then an
integer with no decimal point is returned. The result will never be in
exponential form.

**Examples:**
```rexx <!--rexxex44.crexx-->
    v=.rexx('12.3'); v.trunc         == 12
    w=.rexx('127.09782'); w.trunc(3) == 127.097
    x=.rexx('127.1'); x.trunc(3)     == 127.100
    y=.rexx('127'); y.trunc(2)       == 127.00
    z=.rexx('0'); z.trunc(2)         == 0.00
```

The receiver uses the same Classic numeric-text normalization as `abs()` and
`sign()`; invalid text raises `CONVERSION_ERROR`, while a negative *n* raises
`INVALID_ARGUMENTS`. See the typed
[`trunc` contract](../../../lib/rxfnsb/rexx/trunc.md) and separate Classic
[`rxfnsc/trunc` contract](../../../lib/rxfnsc/trunc.md).

## upper()

Returns a copy of the complete receiver with its Unicode lowercase characters
converted to uppercase. The `.Rexx` method has no start or length extension.

**Examples:**
```rexx <!--rexxex45.crexx-->
    v=.rexx('Fou-Baa'); v.upper().toString() == 'FOU-BAA'
    w=.rexx('école'); w.upper().toString() == 'ÉCOLE'
    x=.rexx(''); x.upper().toString() == ''
```

See the native [`upper` contract](../../../lib/rxfnsb/rexx/upper.md).

## verify(reference \[,option \[,start\]\])

verifies that *string* is composed only of characters from *reference*,
by returning the position of the first character in *string* that is not
also in *reference*. If all the characters were found in *reference*, 0
is returned. The *option* may be either **'Nomatch'** (the default) or
**'Match'**. Only the first character of *option* is significant and it
may be in uppercase or in lowercase. If **'Match'** is specified, the
position of the first character in *string* that **is** in *reference*
is returned, or 0 is returned if none of the characters were found. The
default for *start* is 1 (that is, the search starts at the first
character of *string*). This can be overridden by giving a different
*start* point, which must be positive. If *string* is the null string,
the method returns 0, regardless of the value of the *option*. Similarly
if *start* is greater than *string***.length**, 0 is returned. If
*reference* is the null string, then the returned value is the same as
the value used for *start*, unless **'Match'** is specified as the
*option*, in which case 0 is returned.

**Examples:**
```rexx <!--rexxex46.crexx-->
    v=.rexx('123'); v.verify('1234567890')          == 0
    w=.rexx('1Z3'); w.verify('1234567890')          == 2
    x=.rexx('AB4T'); x.verify('1234567890','M')     == 3
    y=.rexx('1P3Q4'); y.verify('1234567890','N',3)  == 4
    z=.rexx('ABCDE'); z.verify('','n',3)            == 3
    v1=.rexx('AB3CD5'); v1.verify('1234567890','m',4) == 6
```

## word(n)

returns the *nth* blank-delimited word in *string*. *n* must be
positive. If there are fewer than *n* words in *string*, the null string
is returned. This method is exactly equivalent to
*string***.subword(***n*,**1)**.

**Examples:**
```rexx <!--rexxex47.crexx-->
    v=.rexx('Now is the time'); v.word(3) == 'the'
    v=.rexx('Now is the time'); v.word(5) == ''
```

## wordindex(n)

returns the character position of the *nth* blank-delimited word in
*string*. *n* must be positive. If there are fewer than *n* words in the
string, 0 is returned.

**Examples:**
```rexx <!--rexxex48.crexx-->
    v=.rexx('Now is the time'); v.wordindex(3) == 8
    v=.rexx('Now is the time'); v.wordindex(6) == 0
```

## wordlength(n)

returns the length of the *nth* blank-delimited word in *string*. *n*
must be positive. If there are fewer than *n* words in the string, 0 is
returned.

**Examples:**
```rexx <!--rexxex49.crexx-->
    v=.rexx('Now is the time'); v.wordlength(2)    == 2
    w=.rexx('Now comes the time'); w.wordlength(2) == 5
    v=.rexx('Now is the time'); v.wordlength(6)    == 0
```

## wordpos(phrase \[,start\])

searches *string* for the first occurrence of the sequence of
blank-delimited words *phrase*, and returns the word number of the first
word of *phrase* in *string*. Multiple blanks between words in either
*phrase* or *string* are treated as a single blank for the comparison,
but otherwise the words must match exactly. Similarly, leading or
trailing blanks on either string are ignored. If *phrase* is not found,
or contains no words, 0 is returned. By default the search starts at the
first word in *string*. This may be overridden by specifying *start*
(which must be positive), the word at which to start the search.

**Examples:**
```rexx <!--rexxex50.crexx-->
    v=.rexx('now is the time'); v.wordpos('the')       == 3
    v=.rexx('now is the time'); v.wordpos('The')       == 0
    v=.rexx('now is the time'); v.wordpos('is the')    == 2
    v=.rexx('now is the time'); v.wordpos('is    the') == 2
    v=.rexx('now is the time'); v.wordpos('is  time')  == 0
    w=.rexx('To be or not to be'); w.wordpos('be')     == 2
    w=.rexx('To be or not to be'); w.wordpos('be',3)   == 6
```

## words()

returns the number of blank-delimited words in *string*.

**Examples:**
```rexx <!--rexxex51.crexx-->
    v=.rexx('Now is the time'); v.words == 4
    w=.rexx(' '); w.words               == 0
    x=.rexx(''); x.words                == 0
```

## x2b()

Converts each hexadecimal digit in the receiver to four binary digits. Letters
are case-insensitive, leading zero nibbles are retained, and an empty receiver
returns empty. Interior blanks are accepted only when an even number of
hexadecimal digits lies to their right. Invalid text raises
`INVALID_ARGUMENTS`.

The method accepts no arguments and returns a Rexx string object.

**Examples:**
```rexx <!--rexxex52.crexx-->
    v=.rexx('C3'); v.x2b().toString() == '11000011'
    w=.rexx('7'); w.x2b().toString() == '0111'
    x=.rexx('1 C1'); x.x2b().toString() == '000111000001'
    y=.rexx('0001'); y.x2b().toString() == '0000000000000001'
```

The native function contract is documented in
[`x2b.md`](../../../lib/rxfnsb/rexx/x2b.md). Classic Level C X2B is documented
separately in [`rxfnsc/x2b.md`](../../../lib/rxfnsc/x2b.md).

## x2c()

Parses hexadecimal bytes from the receiver and maps each byte to the Unicode
code point U+0000 through U+00FF. An odd leading nibble is padded with zero;
empty input returns empty and leading zero bytes are retained. Values above
`7F` are encoded as valid UTF-8 text, not copied as raw bytes.

Interior blanks are valid only when an even number of hexadecimal digits lies
to their right. Invalid text raises `INVALID_ARGUMENTS`.
**Examples:**
```rexx <!--rexxex53.crexx-->
    v=.rexx('416263'); v.x2c().toString() == 'Abc'
    w=.rexx('4d'); w.x2c().toString() == 'M'
    x=.rexx('FF'); x.x2c().toString() == 'ÿ'
```

The native function contract is documented in
[`x2c.md`](../../../lib/rxfnsb/rexx/x2c.md). Classic Level C X2C uses a
configuration-coded character service and is documented separately in
[`rxfnsc/x2c.md`](../../../lib/rxfnsc/x2c.md).

## x2d(\[n\])

Hexadecimal to decimal. Converts the *string* (a string of hexadecimal
characters) to a decimal number, without rounding. If *string* is the
null string, 0 is returned.

If *n* is not specified, *string* is taken to be an unsigned number.

**Examples:**
```rexx <!--rexxex54.crexx-->
    v=.rexx('0E'); v.x2d    == 14
    w=.rexx('81'); w.x2d    == 129
    x=.rexx('F81'); x.x2d   == 3969
    y=.rexx('FF81'); y.x2d  == 65409
    z=.rexx('c6f0'); z.x2d  == 50928
```

If *n* is specified, *string* is taken as a signed number expressed in
*n* hexadecimal characters. If the most significant (left-most) bit is
zero then the number is positive; otherwise it is a negative number in
twos-complement form. In both cases it is converted to a number which
may, therefore, be negative. If *n* is 0, 0 is always returned.

If necessary, *string* is padded on the left with **'0'** characters
(note, not \"sign-extended\"), or truncated on the left, to length *n*
characters; (that is, as though *string***.right(***n*, **'0')** had
been executed.)

**Examples:**
```rexx <!--rexxex55.crexx-->
    v=.rexx('81'); v.x2d(2)   == -127
    v=.rexx('81'); v.x2d(4)   == 129
    w=.rexx('F081'); w.x2d(4) == -3967
    w=.rexx('F081'); w.x2d(3) == 129
    w=.rexx('F081'); w.x2d(2) == -127
    w=.rexx('F081'); w.x2d(1) == 1
    x=.rexx('0031'); x.x2d(0) == 0
```

The **c2d** method can be used to convert a character to a decimal
representation of its encoding.

This method is the native Level B signed-64-bit surface. Invalid hexadecimal
text or a negative *n* signals `INVALID_ARGUMENTS`; a result outside the native
range signals `OVERFLOW_UNDERFLOW`. Its optional argument is presence-aware,
so an omitted width is unsigned while an explicit zero returns zero. See the
[native function contract](../../../lib/rxfnsb/rexx/x2d.md). Classic Level C
X2D is an arbitrary-width RexxValue BIF with caller `NUMERIC DIGITS` handling,
documented separately in
[`rxfnsc/x2d.md`](../../../lib/rxfnsc/x2d.md).

[^1]: Unless an implementation-provided option to disallow parenthesis
    omission is in force.
