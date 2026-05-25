# Built-in functions for \crexx{} strings and binary values

The `.string` type is central to the \rexx{} language and all its variants. This is true for \crexx{} and this implementation also contains the expected *built-in-functions*.[^bif] Level B also provides byte-oriented helpers for the `.binary` type.

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
| COPIES          | COPIES(s, n)                                 |
| DELSTR          | DELSTR(s, start, length)                     |
| DELWORD         | DELWORD(s, start, n)                         |
| INSERT          | INSERT(s, target, position, length, pad)     |
| JUSTIFY         | JUSTIFY(s, width, pad)                       |
| LEFT            | LEFT(s, n, pad)                              |
| LENGTH          | LENGTH(s)                                    |
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

[^bif]: also colloqually referred to with the jargon-like expression BIFs.
