# General Syntax

A program is built up out of a series of *clauses* that are composed of:
zero or more blanks (which are ignored); a sequence of tokens (described
in this section); zero or more blanks (again ignored); and the delimiter
\"**;**\" (semicolon) which may be implied by line-ends or certain
keywords. Conceptually, each clause is scanned from left to right before
execution and the tokens composing it are resolved.

Identifiers (known as symbols) and numbers are recognized at this stage,
comments (see next chapter on page \pageref{comments}) are removed, and multiple blanks (except
within literal strings) are reduced to single blanks. Blanks adjacent to
operator characters and special characters are also removed.

## Blanks and White Space

*Blanks* (spaces) may be freely used in a program to improve appearance
and layout, and most are ignored. Blanks, however, are usually
significant

- within literal strings (see below)

- between two tokens that are not special characters (for example,
  between two symbols or keywords)

- between the two characters forming a comment delimiter

- immediately outside parentheses (\"**(**\" and \"**)**\") or brackets
  (\"**\[**\" and \"**\]**\").

Tabulation (tab) and form feed
characters outside of literal strings are treated as
if they were a single blank; similarly, if the last character in a
program is the End-of-file character (EOF, encoded in ASCII as decimal
26), that character is ignored.

## Tokens {#reftokens}

The essential components of clauses are called *tokens*. These may be of
any length, unless limited by implementation restrictions, [^1] and are
separated by blanks, comments, ends of lines, or by the nature of the
tokens themselves.

The tokens are:

To illustrate how a clause is composed out of tokens, consider this
example:

    'REPEAT'   B + 3;

This is composed of six tokens: a literal string, a blank operator
(described later), a symbol (which is probably the name of a variable),
an operator, a second symbol (a number), and a semicolon. The blanks
between the \"**B**\" and the \"**+**\" and between the \"**+**\" and
the \"**3**\" are removed. However one of the blanks between the
**'REPEAT'** and the \"**B**\" remains as an operator. Thus the clause
is treated as though written:

    'REPEAT' B+3;

## Implied semicolons and continuations

A semicolon (clause end) is implied at the end of each line, except if:

1.  The line ends in the middle of a block comment, in which case the
    clause continues at the end of the block comment.

2.  The last token was a hyphen. In this case the hyphen is functionally
    replaced by a blank, and hence acts as a *continuation character*.

This means that semicolons need only be included to separate multiple
clauses on a single line.

::: shaded
**Notes:**

1.  A comment is not a token, so therefore a comment may follow the
    continuation character on a line.

2.  Semicolons are added automatically by after certain instruction
    keywords when in the correct context. The keywords that may have
    this effect are , , , ; they become complete clauses in their own
    right when this occurs. These special cases reduce program entry
    errors significantly.
:::

## The case of names and symbols

In general, cRexx is a *case-insensitive* language. That is, the names of
keywords, variables, and so on, will be recognized independently of the
case used for each letter in a name; the name \"**Swildon**\" would
match the name \"**swilDon**\".

<!-- Similarly, the lookup of external names is both case-preserving and -->
<!-- case-insensitive. If a class, method, or property is referenced by the -->
<!-- name \"**Foo**\", for example, an exact-case match will first be tried -->
<!-- at each point that a search is made. If this succeeds, the search for a -->
<!-- matching name is complete. If it does not succeed, a case-insensitive -->
<!-- search in the same context is carried out, and if one item is found, -->
<!-- then the search is complete. If more than one item matches then the -->
<!-- reference is ambiguous, and an error is reported. -->

<!-- Implementations are encouraged to offer an option that requires that all -->
<!-- name matches are exact (case-sensitive), for programmers or house-styles -->
<!-- that prefer that approach to name matching. -->

<!-- ## Hexadecimal and binary numeric symbols -->

<!-- A *hexadecimal numeric symbol* describes a whole number, and is of the -->
<!-- form *n***X***string*. Here, *n* is a simple number with no decimal part -->
<!-- (and optional leading insignificant zeros) which describes the effective -->
<!-- length of the hexadecimal string, the **X** (which may be in lowercase) -->
<!-- indicates that the notation is hexadecimal, and *string* is a string of -->
<!-- one or more hexadecimal characters (characters from the ranges \"a-f\", -->
<!-- \"A-F\", and the digits \"0-9\"). -->

<!-- The *string* is taken as a signed number expressed in *n* hexadecimal -->
<!-- characters. If necessary, *string* is padded on the left with \"**0**\" -->
<!-- characters (note, not \"sign-extended\") to length *n* characters. -->

<!-- If the most significant (left-most) bit of the resulting string is zero -->
<!-- then the number is positive; otherwise it is a negative number in -->
<!-- twos-complement form. In both cases it is converted to a number which -->
<!-- may, therefore, be negative. The result of the conversion is a number -->
<!-- comprised of the Arabic digits 0-9, with no insignificant leading zeros -->
<!-- but possibly with a leading \"**-**\". -->

<!-- The value *n* may not be less than the number of characters in *string*, -->
<!-- with the single exception that it may be zero, which indicates that the -->
<!-- number is always positive (as though *n* were greater than the the -->
<!-- length of *string*). -->

<!-- **Examples:** -->

<!--     1x8    == -8 -->
<!--     2x8    == 8 -->
<!--     2x08   == 8 -->
<!--     0x08   == 8 -->
<!--     0x10   == 16 -->
<!--     0x81   == 129 -->
<!--     2x81   == -127 -->
<!--     3x81   == 129 -->
<!--     4x81   == 129 -->
<!--     04x81  == 129 -->
<!--     16x81  == 129 -->
<!--     4xF081 == -3967 -->
<!--     8xF081 == 61569 -->
<!--     0Xf081 == 61569 -->

<!-- A *binary numeric symbol* describes a whole number using the same rules, -->
<!-- except that the identifying character is **B** or **b**, and the digits -->
<!-- of *string* must be either **0** or **1**, each representing a single -->
<!-- bit. -->

<!-- **Examples:** -->

<!--     1b0    == 0 -->
<!--     1b1    == -1 -->
<!--     0b10   == 2 -->
<!--     0b100  == 4 -->
<!--     4b1000 == -8 -->
<!--     8B1000 == 8 -->

<!-- **Note:** Hexadecimal and binary numeric symbols are a purely syntactic -->
<!-- device for representing decimal whole numbers. That is, they are -->
<!-- recognized only within the source of a program, and are not equivalent -->
<!-- to a literal string with the same characters within quotes. -->

<!-- [^1]: Wherever arbitrary implementation restrictions are applied, the -->
<!--     size of the restriction should be a number that is readily memorable -->
<!--     in the decimal system; that is, one of 1, 25, or 5 multiplied by a -->
<!--     power of ten. 500 is preferred to 512, the number 250 is more -->
<!--     \"natural\" than 256, and so on. Limits expressed in digits should -->
<!--     be a multiple of three. -->
