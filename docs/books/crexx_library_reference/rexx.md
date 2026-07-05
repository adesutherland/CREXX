# The .rexx class

This section describes the set of methods defined for the Rexx class, a
container which enables object-oriented string manipulation. These
methods implement the traditional *built-in functions*, and include
character manipulation, word manipulation, conversion, and arithmetic
functions.

Use of these methods needs import of the package:

strings must be constructed with a factory constructor.

**General notes on the built-in methods:**

- All methods work on string input parameters; this is referred to by the
    name *string* in the descriptions of the methods. For example, if
    the **word** method were invoked using the term:

        v=.rexx("Three word phrase"); v.word(2)

    then in the description of **word** the name *string* refers to the
    string  
	\"**Three word phrase**\", and the name *n* refers to the
    string \"**2**\".

- All method arguments are of type '.string' and all methods return a string of
    type '.rexx'.; if a number is returned, it will be formatted as though 0 had
    been added with no rounding.

- The first parenthesis in a method call must immediately follow the
    name of the method, with no space in between.

- The parentheses in a method call can be omitted if no arguments are
    required and the method call is part of a [^1]

- A position in a string is the number of a character in the string,
    where the first character is at position 1, *etc.*

-  Where arguments are optional, commas may only be included between
    arguments that are present (that is, trailing commas in argument
    lists are not permitted).

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

    v=.rexx('Print'); v.abbrev('Pri')   == 1
    w=.rexx('PRINT'); w.abbrev('Pri')   == 0
    w=.rexx('PRINT'); w.abbrev('PRI',4) == 0
    w=.rexx('PRINT'); w.abbrev('PRY')   == 0
    w=.rexx('PRINT'); w.abbrev('')      == 1
    w=.rexx('PRINT'); w.abbrev('',1)    == 0

**Note:** A null string will always match if a length of 0 (or the
default) is used. This allows a default keyword to be selected
automatically if desired. **Example:**

    say 'Enter option:';  option=ask
    select  /* keyword1 is to be the default */
      v=.rexx('keyword1'); when v.abbrev(option) then ...
      w=.rexx('keyword2'); when w.abbrev(option) then ...
         ...
      otherwise ...
      end

## abs()

returns the absolute value of *string*, which must be a number. Any sign
is removed from the number, and it is then formatted by adding zero with
a digits setting that is either nine or, if greater, the number of
digits in the mantissa of the number (excluding leading insignificant
zeros). Scientific notation is used, if necessary.

**Examples:**

    v=.rexx('12.3'); v.abs              == 12.3
    w=.rexx(' -0.307'); w.abs           == 0.307
    x=.rexx('123.45E+16'); x.abs        == 1.2345E+18
    y=.rexx('- 1234567.7654321'); y.abs == 1234567.7654321

## b2d(\[n\])

Binary to decimal. Converts *string*, a string of at least one binary
(**0** and/or **1**) digits, to an equivalent string of decimal
characters (a number), without rounding. The returned string will use
digits, and will not include any blanks. If the number of binary digits
in the string is not a multiple of four, then up to three **'0'** digits
will be added on the left before conversion to make a total that is a
multiple of four. If *string* is the null string, 0 is returned. If n is
not specified, *string* is taken to be an unsigned number.

**Examples:**

    v=.rexx('01110'); v.b2d == 14 
    w=.rexx('10000001'); w.b2d == 129 
    x=.rexx('111110000001'); x.b2d == 3969 
    y=.rexx('1111111110000001'); y.b2d == 65409 
    z=.rexx('1100011011110000'); z.b2d == 50928 

If n is specified, string is taken as a signed number expressed in n
binary characters. If the most significant (left-most) bit is zero then
the number is positive; otherwise it is a negative number in
twos-complement form. In both cases it is converted to a cRexx number
which may, therefore, be negative. If n is 0, 0 is always returned.

If necessary, string is padded on the left with '0' characters (note,
not "signextended"), or truncated on the left, to length n characters;
(that is, as though string.right(n, '0') had been executed.)

**Examples:**

    v=.rexx('10000001'); v.b2d(8) == -127 
    v=.rexx('10000001'); v.b2d(16) == 129 
    w=.rexx('1111000010000001'); w.b2d(16) == -3967 
    w=.rexx('1111000010000001'); w.b2d(12) == 129 
    w=.rexx('1111000010000001'); w.b2d(8) == -127 
    w=.rexx('1111000010000001'); w.b2d(4) == 1 
    x=.rexx('0000000000110001'); x.b2d(0) == 0

## b2x()

Binary to hexadecimal. Converts *string*, a string of at least one
binary (**0** and/or **1**) digits, to an equivalent string of
hexadecimal characters. The returned string will use uppercase Roman
letters for the values A-F, and will not include any blanks. If the
number of binary digits in the string is not a multiple of four, then up
to three **'0'** digits will be added on the left before conversion to
make a total that is a multiple of four.

**Examples:**

    v=.rexx('11000011'); v.b2x  == 'C3'
    w=.rexx('10111'); w.b2x     == '17'
    x=.rexx('0101'); x.b2x      == '5'
    y=.rexx('101'); y.b2x       == '5'
    z=.rexx('111110000'); z.b2x == '1F0'

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

    v=.rexx('ABC'); v.centre(7)          == '  ABC  '
    v=.rexx('ABC'); v.center(8,'-')      == '--ABC---'
    w=.rexx('The blue sky'); w.centre(8) == 'e blue s'
    w=.rexx('The blue sky'); w.center(7) == 'e blue '

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

    v=.rexx('elephant'); v.changestr('e','X')    == 'XlXphant'
    v=.rexx('elephant'); v.changestr('ph','X')   == 'eleXant'
    v=.rexx('elephant'); v.changestr('ph','hph') == 'elehphant'
    v=.rexx('elephant'); v.changestr('e','')     == 'lphant'
    v=.rexx('elephant'); v.changestr('','!!')    == 'elephant'

## compare(target \[,pad\])

returns 0 if *string* and *target* are the same. If they are not, the
returned number is positive and is the position of the first character
that is not the same in both strings. If one string is shorter than the
other, one or more *pad* characters are added on the right to make it
the same length for the comparison. The default *pad* character is a
blank.

**Examples:**

    v=.rexx('abc'); v.compare('abc')      == 0
    v=.rexx('abc'); v.compare('ak')       == 2
    w=.rexx('ab '); w.compare('ab')       == 0
    w=.rexx('ab '); w.compare('ab',' ')   == 0
    w=.rexx('ab '); w.compare('ab','x')   == 3
    x=.rexx('ab-- '); x.compare('ab','-') == 5

## copies(n)

returns *n* directly concatenated copies of *string*. *n* must be
positive or 0; if 0, the null string is returned.

**Examples:**

    v=.rexx('abc'); v.copies(3) == 'abcabcabc'
    v=.rexx('abc'); v.copies(0) == ''
    w=.rexx(''); w.copies(2)    == ''

## countstr(needle)

returns the count of non-overlapping occurrences of the *needle* string
in *string*, searching from left to right and starting from the first
(leftmost) position in *string*.

If the *needle* is the null string, **0** is returned.

**Examples:**

    v=.rexx('elephant'); v.countstr('e')  == '2'
    v=.rexx('elephant'); v.countstr('ph') == '1'
    v=.rexx('elephant'); v.countstr('')   == '0'

The **changestr** method can be used to change occurrences of *needle*
to some other string.

## c2d()

Coded character to decimal. Converts the Unicode code point of the
character in *string* (which must be exactly one character) to its
decimal representation. The returned string will be a non-negative
number that represents the code point of the character and will not
include any sign, blanks, insignificant leading zeros, or decimal part.

**Examples:**

    v=.rexx('M'); v.c2d  == '77'  -- ASCII or Unicode
    w=.rexx('🔥'); w.c2d == '128293'
    x=.rexx('7'); x.c2d  == '247' -- EBCDIC
    y=.rexx('\textbackslash{}r'); y.c2d == '13'  -- ASCII or Unicode
    z=.rexx('\textbackslash{}0'); z.c2d == '0'

The **c2x** method can be used to convert the encoding of a character to
a hexadecimal representation.

## c2x()

Coded character to hexadecimal. Converts the encoding of the character
in *string* (which must be exactly one character) to its hexadecimal
representation (unpacks). The returned string will use uppercase Roman
letters for the values A-F, and will not include any blanks.
Insignificant leading zeros are removed.

**Examples:**

    v=.rexx('M'); v.c2x  == '4D' -- ASCII or Unicode
    w=.rexx('7'); w.c2x  == 'F7' -- EBCDIC
    x=.rexx('\textbackslash{}r'); x.c2x == 'D'  -- ASCII or Unicode
    y=.rexx('\textbackslash{}0'); y.c2x == '0'

The **c2d** method can be used to convert the encoding of a character to
a decimal number.

## datatype(option) {#refdataty}

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
than the length of *string*, the string is returned unchanged.

**Examples:**

    v=.rexx('abcd'); v.delstr(3)    == 'ab'
    w=.rexx('abcde'); w.delstr(3,2) == 'abe'
    w=.rexx('abcde'); w.delstr(6)   == 'abcde'

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

    v=.rexx('Now is the  time'); v.delword(2,2) == 'Now time'
    w=.rexx('Now is the time '); w.delword(3)   == 'Now is '
    x=.rexx('Now  time'); x.delword(5)          == 'Now  time'

## d2b(\[n\])

Returns a string of binary characters of length as needed or of length
n, which is the binary representation of the decimal number. The
returned string will use 0 and 1 characters for binary values. string
must be a whole number, and must be non-negative unless n is specified,
or an error will result. If n is not specified, the length of the result
returned is such that there are no leading 0 characters, unless string
was equal to 0 (in which case '0' is returned).

If n is specified it is the length of the final result in characters;
that is, after conversion the input string will be sign-extended to the
required length (negative numbers are converted assuming twos-complement
form). If the number is too big to fit into n characters, it will be
truncated on the left. n must be a nonnegative whole number.

**Examples:**

    v=.rexx('0'); v.d2b == 0 
    w=.rexx('9'); w.d2b == 1001 
    x=.rexx('19'); x.d2b == 10011 
    y=.rexx('129'); y.d2b == 10000001 
    y=.rexx('129'); y.d2b(1) == 1 
    y=.rexx('129'); y.d2b(8) == 10000001 
    z=.rexx('127'); z.d2b(12) == 000001111111 
    y=.rexx('129'); y.d2b(16) == 0000000010000001 
    v1=.rexx('257'); v1.d2b(8) == 00000001 
    v2=.rexx('-127'); v2.d2b(8) == 10000001 
    v2=.rexx('-127'); v2.d2b(16) == 1111111110000001 
    v3=.rexx('12'); v3.d2b(0) == 

## d2c(\[length\])

Decimal to coded character. Converts the *string* (a *number*) to a
single character, where the number is used as the Unicode code point of
the character.

*string* must be a non-negative whole number naming a valid Unicode code
point. An error results if the code point is invalid for Unicode (for
example, if it is outside the Unicode range or in the surrogate range).
If *length* is specified under Unicode semantics, it may be **0** or
**1**. A length of **0** returns the null string; a length of **1** is
equivalent to omitting it.

**Examples:**

    v=.rexx('77'); v.d2c  == 'M' -- ASCII or Unicode
    v=.rexx('77'); v.d2c(1) == 'M'
    w=.rexx('12'); w.d2c(0) == ''
    x=.rexx('128293'); x.d2c == '🔥'
    y=.rexx('+77'); y.d2c == 'M' -- ASCII or Unicode
    z=.rexx('247'); z.d2c == '7' -- EBCDIC
    v1=.rexx('0'); v1.d2c   == '\textbackslash 0'

## d2x(\[n\])

Decimal to hexadecimal. Returns a string of hexadecimal characters of
length as needed or of length *n*, which is the hexadecimal (unpacked)
representation of the decimal number. The returned string will use
uppercase Roman letters for the values A-F, and will not include any
blanks. *string* must be a whole number, and must be non-negative unless
*n* is specified, or an error will result. If *n* is not specified, the
length of the result returned is such that there are no leading 0
characters, unless *string* was equal to 0 (in which case **'0'** is
returned).

If *n* is specified it is the length of the final result in characters;
that is, after conversion the input string will be sign-extended to the
required length (negative numbers are converted assuming twos-complement
form). If the number is too big to fit into *n* characters, it will be
truncated on the left. *n* must be a non-negative whole number.

**Examples:**

    v=.rexx('9'); v.d2x       == '9'
    w=.rexx('129'); w.d2x     == '81'
    w=.rexx('129'); w.d2x(1)  == '1'
    w=.rexx('129'); w.d2x(2)  == '81'
    x=.rexx('127'); x.d2x(3)  == '07F'
    w=.rexx('129'); w.d2x(4)  == '0081'
    y=.rexx('257'); y.d2x(2)  == '01'
    z=.rexx('-127'); z.d2x(2) == '81'
    z=.rexx('-127'); z.d2x(4) == 'FF81'
    v1=.rexx('12'); v1.d2x(0)   == ''

## exists(index)

returns 1 if *index* names a sub-value of *string* that has explicitly
been assigned a value, or 0 otherwise.

**Example:** Following the instructions:

    vowel=0
    vowel['a']=1
    vowel['b']=1
    vowel['b']=null -- drops previous assignment

then:

    vowel.exists('a') == '1'
    vowel.exists('b') == '0'
    vowel.exists('c') == '0'

## format(\[before \[,after\]\])

formats (lays out) *string*, which must be a number.

The number, *string*, is first formatted by adding zero with a digits
setting that is either nine or, if greater, the number of digits in the
mantissa of the number (excluding leading insignificant zeros). If no
arguments are given, the result is precisely that of this operation.

The arguments *before* and *after* may be specified to control the
number of characters to be used for the integer part and decimal part of
the result respectively. If either of these is omitted (with no
arguments specified to its right), or is **null**, the number of
characters used will be as many as are needed for that part.

*before* must be a positive number; if it is larger than is needed to
contain the integer part, that part is padded on the left with blanks to
the requested length. If *before* is not large enough to contain the
integer part of the number (including the sign, for negative numbers),
an error results.

*after* must be a non-negative number; if it is not the same size as the
decimal part of the number, the number will be rounded (or extended with
zeros) to fit. Specifying 0 for *after* will cause the number to be
rounded to an integer (that is, it will have no decimal part or decimal
point).

**Examples:**

    v=.rexx(' - 12.73'); v.format         == '-12.73'
    w=.rexx('0.000'); w.format            == '0'
    x=.rexx('3'); x.format(4)             == '   3'
    y=.rexx('1.73'); y.format(4,0)        == '   2'
    y=.rexx('1.73'); y.format(4,3)        == '   1.730'
    z=.rexx('-.76'); z.format(4,1)        == '  -0.8'
    v1=.rexx('3.03'); v1.format(4)          == '   3.03'
    v=.rexx(' - 12.73'); v.format(null,4) == '-12.7300'

Further arguments may be passed to the method to control the use of
exponential notation. The full syntax of the method is then:

The first two arguments are as already described. The other three
(*explaces*, *exdigits*, and *exform*) control the exponent part of the
result. The default for any of the arguments may be selected by omitting
them (if there are no arguments to be specified to their right) or by
using the value **null**.

*explaces* must be a positive number; it sets the number of places
(digits after the sign of the exponent) to be used for any exponent
part, the default being to use as many as are needed. If *explaces* is
specified and is not large enough to contain the exponent, an error
results. If *explaces* is specified and the exponent will be 0, then
*explaces*+2 blanks are supplied for the exponent part of the result.

*exdigits* sets the trigger point for use of exponential notation. If,
after the first formatting, the number of places needed before the
decimal point exceeds *exdigits*, or if the absolute value of the result
is less than **0.000001**, then exponential form will be used, provided
that *exdigits* was specified. When *exdigits* is not specified,
exponential notation will never be used. The current setting of may be
used for *exdigits* by specifying the special word **digits** . If 0 is
specified for *exdigits*, exponential notation is always used unless the
exponent would be 0.

*exform* sets the form for exponential notation (if needed). *exform*
may be either **'Scientific'** (the default) or **'Engineering'**. Only
the first character of *exform* is significant and it may be in
uppercase or in lowercase. The current setting of may be used by
specifying the special word **form** . If engineering form is in effect,
up to three digits (plus sign) may be needed for the integer part of the
result (*before*).

**Examples:**

    v=.rexx('12345.73'); v.format(null,null,2,2) == '1.234573E+04'
    v=.rexx('12345.73'); v.format(null,3,null,0) == '1.235E+4'
    w=.rexx('1.234573'); w.format(null,3,null,0) == '1.235'
    x=.rexx('123.45'); x.format(null,3,2,0)      == '1.235E+02'
    y=.rexx('1234.5'); y.format(null,3,2,0,'e')  == '1.235E+03'
    z=.rexx('1.2345'); z.format(null,3,2,0)      == '1.235    '
    v=.rexx('12345.73'); v.format(null,null,3,6) == '12345.73     '
    v1=.rexx('12345e+5'); v1.format(null,3)        == '1234500000.000'

**Implementation minimum:** If exponents are supported in an
implementation, then they must be supported for exponents whose absolute
value is at least as large as the largest number that can be expressed
as an exact integer in default precision, *i.e.*, 999999999. Therefore,
values for *explaces* of up to 9 should also be supported.

## insert(new \[,n \[,length \[,pad\]\]\])

inserts the string *new*, padded or truncated to length *length*, into a
copy of the target *string* after the *nth* character; the string with
any inserts is returned. *length* and *n* must be a non-negative whole
numbers. If *n* is greater than the length of the target string, padding
is added before the *new* string also. The default value for *n* is 0,
which means insert before the beginning of the string. The default value
for *length* is the length of *new*. The default *pad* character is a
blank.

**Examples:**

    v=.rexx('abc'); v.insert('123')         == '123abc'
    w=.rexx('abcdef'); w.insert(' ',3)      == 'abc def'
    v=.rexx('abc'); v.insert('123',5,6)     == 'abc  123   '
    v=.rexx('abc'); v.insert('123',5,6,'+') == 'abc++123+++'
    v=.rexx('abc'); v.insert('123',0,5,'-') == '123--abc'

## lastpos(needle \[,start\])

returns the position of the last occurrence of the string *needle* in
*string* (the \"haystack\"), searching from right to left. If the string
*needle* is not found, or is the null string, 0 is returned. By default
the search starts at the last character of *string* and scans backwards.
This may be overridden by specifying *start*, the point at which to
start the backwards scan. *start* must be a positive whole number, and
defaults to the value *string***.length** if larger than that value or
if not specified (with a minimum default value of one).

**Examples:**

    v=.rexx('abc def ghi'); v.lastpos(' ')   == 8
    v=.rexx('abc def ghi'); v.lastpos(' ',7) == 4
    w=.rexx('abcdefghi'); w.lastpos(' ')     == 0
    w=.rexx('abcdefghi'); w.lastpos('cd')    == 3
    x=.rexx(''); x.lastpos('?')              == 0

## left(length \[,pad\])

returns a string of length *length* containing the left-most *length*
characters of *string*. The string is padded with *pad* characters (or
truncated) on the right as needed. The default *pad* character is a
blank. *length* must be a non-negative whole number. This method is
exactly equivalent to *string***.substr(1**, *length* \[, *pad*\]**)**.

**Examples:**

    v=.rexx('abc d'); v.left(8)     == 'abc d   '
    v=.rexx('abc d'); v.left(8,'.') == 'abc d...'
    w=.rexx('abc defg'); w.left(6)  == 'abc de'

## length()

returns the number of characters in *string*.

**Examples:**

    v=.rexx('abcdefgh'); v.length == 8
    w=.rexx(''); w.length         == 0

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

## lower(\[n \[,length\]\])

returns a copy of *string* with any uppercase characters in the
sub-string of *string* that begins at the *nth* character, and is of
length *length* characters, replaced by their lowercase equivalent.

*n* must be a positive whole number, and defaults to 1 (the first
character in *string*). If *n* is greater than the length of *string*,
the string is returned unchanged.

*length* must be a non-negative whole number. If *length* is not
specified, or is greater than the number of characters from *n* to the
end of the string, the rest of the string (including the *nth*
character) is assumed.

**Examples:**

    v=.rexx('SumA'); v.lower      == 'suma'
    v=.rexx('SumA'); v.lower(2)   == 'Suma'
    w=.rexx('SuMB'); w.lower(1,1) == 'suMB'
    x=.rexx('SUMB'); x.lower(2,2) == 'SumB'
    y=.rexx(''); y.lower          == ''

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

    0.max(1)          ==1
    v=.rexx('-1'); v.max(1)       ==1
    w=.rexx('+1'); w.max(-1)      ==1
    x=.rexx('1.0'); x.max(1.00)   =='1.0'
    y=.rexx('1.00'); y.max(1.0)   =='1.00'
    z=.rexx('123456700000'); z.max(1234567E+5)   == '123456700000'
    v1=.rexx('1234567E+5'); v1.max('123456700000') == '1.234567E+11'

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

    0.min(1)          ==0
    v=.rexx('-1'); v.min(1)       =='-1'
    w=.rexx('+1'); w.min(-1)      =='-1'
    x=.rexx('1.0'); x.min(1.00)   =='1.0'
    y=.rexx('1.00'); y.min(1.0)   =='1.00'
    z=.rexx('123456700000'); z.min(1234567E+5)   == '123456700000'
    v1=.rexx('1234567E+5'); v1.min('123456700000') == '1.234567E+11'

## overlay(new \[,n \[,length \[,pad\]\]\])

overlays the string *new*, padded or truncated to length *length*, onto
a copy of the target *string* starting at the *nth* character; the
string with any overlays is returned. Overlays may extend beyond the end
of the original *string*. If *length* is specified it must be a
non-negative whole number. If *n* is greater than the length of the
target string, padding is added before the *new* string also. The
default *pad* character is a blank, and the default value for *n* is 1.
*n* must be greater than 0. The default value for *length* is the length
of *new*.

**Examples:**

    v=.rexx('abcdef'); v.overlay(' ',3)      == 'ab def'
    v=.rexx('abcdef'); v.overlay('.',3,2)    == 'ab. ef'
    w=.rexx('abcd'); w.overlay('qq')         == 'qqcd'
    w=.rexx('abcd'); w.overlay('qq',4)       == 'abcqq'
    x=.rexx('abc'); x.overlay('123',5,6,'+') == 'abc+123+++'

## pos(needle \[,start\])

returns the position of the string *needle*, in *string* (the
\"haystack\"), searching from left to right. If the string *needle* is
not found, or is the null string, 0 is returned. By default the search
starts at the first character of *string* (that is, *start* has the
value 1). This may be overridden by specifying *start* (which must be a
positive whole number), the point at which to start the search; if
*start* is greater than the length of *string* then 0 is returned.
**Examples:**

    v=.rexx('Saturday'); v.pos('day')    == 6
    w=.rexx('abc def ghi'); w.pos('x')   == 0
    w=.rexx('abc def ghi'); w.pos(' ')   == 4
    w=.rexx('abc def ghi'); w.pos(' ',5) == 8

## reverse()

returns a copy of *string*, swapped end for end.

**Examples:**

    v=.rexx('ABc.'); v.reverse        == '.cBA'
    w=.rexx('XYZ '); w.reverse        == ' ZYX'
    x=.rexx('Tranquility'); x.reverse == 'ytiliuqnarT'

## right(length \[,pad\])

returns a string of length *length* containing the right-most *length*
characters of *string* - that is, padded with *pad* characters (or
truncated) on the left as needed. The default *pad* character is a
blank. *length* must be a non-negative whole number.

**Examples:**

    v=.rexx('abc  d'); v.right(8)  == '  abc  d'
    w=.rexx('abc def'); w.right(5) == 'c def'
    x=.rexx('12'); x.right(5,'0')  == '00012'

## sequence(final)

returns a string of all characters, in ascending order of encoding,
between and including the character in *string* and the character in
*final*. *string* and *final* must be single characters; if *string* is
greater than *final*, an error is reported.

**Examples:**

    v=.rexx('a'); v.sequence('f')           == 'abcdef'
    w=.rexx('\\0'); w.sequence('\\x03')       == '\\x00\\x01\\x02\\x03'
    x=.rexx('\\ufffe'); x.sequence('\\uffff') == '\\ufffe\\uffff'

## sign()

returns a number that indicates the sign of *string*, which must be a
number. *string* is first formatted, just as though the operation
\"**string+0**\" had been carried out with sufficient digits to avoid
rounding. If the number then starts with **'-'** then **'-1'** is
returned; if it is **'0'** then **'0'** is returned; and otherwise
**'1'** is returned.

**Examples:**

    v=.rexx('12.3'); v.sign    ==  1
    w=.rexx('0.0'); w.sign     ==  0
    x=.rexx(' -0.307'); x.sign == -1

## space(\[n \[,pad\]\])

returns a copy of *string* with the blank-delimited words in *string*
formatted with *n* (and only *n*) *pad* characters between each word.
*n* must be a non-negative whole number. If *n* is 0, all blanks are
removed. Leading and trailing blanks are always removed. The default for
*n* is 1, and the default *pad* character is a blank.

**Examples:**

    v=.rexx('abc  def  '); v.space        == 'abc def'
    w=.rexx('  abc def '); w.space(3)     == 'abc   def'
    v=.rexx('abc  def  '); v.space(1)     == 'abc def'
    v=.rexx('abc  def  '); v.space(0)     == 'abcdef'
    v=.rexx('abc  def  '); v.space(2,'+') == 'abc++def'

## strip(\[option \[,char\]\]\])

returns a copy of *string* with Leading, Trailing, or Both leading and
trailing characters removed, when the first character of *option* is L,
T, or B respectively (these may be given in either uppercase or
lowercase). The default is B. The second argument, *char*, specifies the
character to be removed, with the default being a blank. If given,
*char* must be exactly one character long.

**Examples:**

    v=.rexx('  ab c  '); v.strip        == 'ab c'
    v=.rexx('  ab c  '); v.strip('L')   == 'ab c  '
    v=.rexx('  ab c  '); v.strip('t')   == '  ab c'
    w=.rexx('12.70000'); w.strip('t',0) == '12.7'
    x=.rexx('0012.700'); x.strip('b',0) == '12.7'

## substr(n \[,length \[,pad\]\])

returns the sub-string of *string* that begins at the *nth* character,
and is of length *length*, padded with *pad* characters if necessary.
*n* must be a positive whole number, and *length* must be a non-negative
whole number. If *n* is greater than *string***.length**, then only pad
characters can be returned. If *length* is omitted it defaults to be the
rest of the string (or 0 if *n* is greater than the length of the
string). The default *pad* character is a blank.

**Examples:**

    v=.rexx('abc'); v.substr(2)       == 'bc'
    v=.rexx('abc'); v.substr(2,4)     == 'bc  '
    v=.rexx('abc'); v.substr(5,4)     == '    '
    v=.rexx('abc'); v.substr(2,6,'.') == 'bc....'
    v=.rexx('abc'); v.substr(5,6,'.') == '......'

**Note:** In some situations the positional (numeric) patterns of
parsing templates are more convenient for selecting sub-strings,
especially if more than one sub-string is to be extracted from a string.

## subword(n \[,length\])

returns the sub-string of *string* that starts at the *nth* word, and is
up to *length* blank-delimited words long. *n* must be a positive whole
number; if greater than the number of words in the string then the null
string is returned. *length* must be a non-negative whole number. If
*length* is omitted it defaults to be the remaining words in the string.
The returned string will never have leading or trailing blanks, but will
include all blanks between the selected words.

**Examples:**

    v=.rexx('Now is the  time'); v.subword(2,2) == 'is the'
    v=.rexx('Now is the  time'); v.subword(3)   == 'the  time'
    v=.rexx('Now is the  time'); v.subword(5)   == ''

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

    v=.rexx('abbc'); v.translate('&','b')           == 'a&&c'
    w=.rexx('abcdef'); w.translate('12','ec')       == 'ab2d1f'
    w=.rexx('abcdef'); w.translate('12','abcd','.') == '12..ef'
    x=.rexx('4123'); x.translate('abcd','1234')     == 'dabc'
    x=.rexx('4123'); x.translate('hods','1234')     == 'shod'

**Note:** The last two examples show how the **translate** method may be
used to move around the characters in a string. In these examples, any
4-character string could be specified as the first argument and its last
character would be moved to the beginning of the string. Similarly, the
term:

    v=.rexx('gh.ef.abcd'); v.translate(19970827,'abcdefgh')

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

    v=.rexx('12.3'); v.trunc         == 12
    w=.rexx('127.09782'); w.trunc(3) == 127.097
    x=.rexx('127.1'); x.trunc(3)     == 127.100
    y=.rexx('127'); y.trunc(2)       == 127.00
    z=.rexx('0'); z.trunc(2)         == 0.00

## upper(\[n \[,length\]\])

returns a copy of *string* with any lowercase characters in the
sub-string of *string* that begins at the *nth* character, and is of
length *length* characters, replaced by their uppercase equivalent.

*n* must be a positive whole number, and defaults to 1 (the first
character in *string*). If *n* is greater than the length of *string*,
the string is returned unchanged.

*length* must be a non-negative whole number. If *length* is not
specified, or is greater than the number of characters from *n* to the
end of the string, the rest of the string (including the *nth*
character) is assumed.

**Examples:**

    v=.rexx('Fou-Baa'); v.upper        == 'FOU-BAA'
    w=.rexx('Mad Sheep'); w.upper      == 'MAD SHEEP'
    x=.rexx('Mad sheep'); x.upper(5)   == 'Mad SHEEP'
    x=.rexx('Mad sheep'); x.upper(5,1) == 'Mad Sheep'
    x=.rexx('Mad sheep'); x.upper(5,4) == 'Mad SHEEp'
    y=.rexx('tinganon'); y.upper(1,1)  == 'Tinganon'
    z=.rexx(''); z.upper               == ''

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

    v=.rexx('123'); v.verify('1234567890')          == 0
    w=.rexx('1Z3'); w.verify('1234567890')          == 2
    x=.rexx('AB4T'); x.verify('1234567890','M')     == 3
    y=.rexx('1P3Q4'); y.verify('1234567890','N',3)  == 4
    z=.rexx('ABCDE'); z.verify('','n',3)            == 3
    v1=.rexx('AB3CD5'); v1.verify('1234567890','m',4) == 6

## word(n)

returns the *nth* blank-delimited word in *string*. *n* must be
positive. If there are fewer than *n* words in *string*, the null string
is returned. This method is exactly equivalent to
*string***.subword(***n*,**1)**.

**Examples:**

    v=.rexx('Now is the time'); v.word(3) == 'the'
    v=.rexx('Now is the time'); v.word(5) == ''

## wordindex(n)

returns the character position of the *nth* blank-delimited word in
*string*. *n* must be positive. If there are fewer than *n* words in the
string, 0 is returned.

**Examples:**

    v=.rexx('Now is the time'); v.wordindex(3) == 8
    v=.rexx('Now is the time'); v.wordindex(6) == 0

## wordlength(n)

returns the length of the *nth* blank-delimited word in *string*. *n*
must be positive. If there are fewer than *n* words in the string, 0 is
returned.

**Examples:**

    v=.rexx('Now is the time'); v.wordlength(2)    == 2
    w=.rexx('Now comes the time'); w.wordlength(2) == 5
    v=.rexx('Now is the time'); v.wordlength(6)    == 0

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

    v=.rexx('now is the time'); v.wordpos('the')       == 3
    v=.rexx('now is the time'); v.wordpos('The')       == 0
    v=.rexx('now is the time'); v.wordpos('is the')    == 2
    v=.rexx('now is the time'); v.wordpos('is    the') == 2
    v=.rexx('now is the time'); v.wordpos('is  time')  == 0
    w=.rexx('To be or not to be'); w.wordpos('be')     == 2
    w=.rexx('To be or not to be'); w.wordpos('be',3)   == 6

## words()

returns the number of blank-delimited words in *string*.

**Examples:**

    v=.rexx('Now is the time'); v.words == 4
    w=.rexx(' '); w.words               == 0
    x=.rexx(''); x.words                == 0

## x2b()

Hexadecimal to binary. Converts *string* (a string of at least one
hexadecimal characters) to an equivalent string of binary digits.
Hexadecimal characters may be any decimal digit character (0-9) or any
of the first six alphabetic characters (a-f), in either lowercase or
uppercase. *string* may be of any length; each hexadecimal character
with be converted to a string of four binary digits. The returned string
will have a length that is a multiple of four, and will not include any
blanks.

**Examples:**

    v=.rexx('C3'); v.x2b  == '11000011'
    w=.rexx('7'); w.x2b   == '0111'
    x=.rexx('1C1'); x.x2b == '000111000001'

## x2c()

Hexadecimal to coded character. Converts the *string* (a string of
hexadecimal characters) to a single character (packs). Hexadecimal
characters may be any decimal digit character (0-9) or any of the first
six alphabetic characters (a-f), in either lowercase or uppercase.

*string* must contain at least one hexadecimal character; insignificant
leading zeros are removed, and the string is then padded with leading
zeros if necessary to make a sufficient number of hexadecimal digits to
describe a character encoding for the implementation.

An error results if the encoding described does not produce a valid
character for the implementation (for example, if it has more
significant bits than the implementation's encoding for characters).
**Examples:**

    v=.rexx('004D'); v.x2c == 'M' -- ASCII or Unicode
    w=.rexx('4d'); w.x2c   == 'M' -- ASCII or Unicode
    x=.rexx('A2'); x.x2c   == 's' -- EBCDIC
    y=.rexx('0'); y.x2c    == '\textbackslash 0'

The **d2c** method can be used to convert a number to the encoding of a
character.

## x2d(\[n\])

Hexadecimal to decimal. Converts the *string* (a string of hexadecimal
characters) to a decimal number, without rounding. If *string* is the
null string, 0 is returned.

If *n* is not specified, *string* is taken to be an unsigned number.

**Examples:**

    v=.rexx('0E'); v.x2d    == 14
    w=.rexx('81'); w.x2d    == 129
    x=.rexx('F81'); x.x2d   == 3969
    y=.rexx('FF81'); y.x2d  == 65409
    z=.rexx('c6f0'); z.x2d  == 50928

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

    v=.rexx('81'); v.x2d(2)   == -127
    v=.rexx('81'); v.x2d(4)   == 129
    w=.rexx('F081'); w.x2d(4) == -3967
    w=.rexx('F081'); w.x2d(3) == 129
    w=.rexx('F081'); w.x2d(2) == -127
    w=.rexx('F081'); w.x2d(1) == 1
    x=.rexx('0031'); x.x2d(0) == 0

The **c2d** method can be used to convert a character to a decimal
representation of its encoding.

[^1]: Unless an implementation-provided option to disallow parenthesis
    omission is in force.
