## x2b

```rexx
x2b(hexadecimal = .string) = .string
```

The Level B `x2b` helper converts every hexadecimal digit to exactly four
binary digits. Input is case-insensitive, the empty string returns empty, and
all leading zero nibbles are retained.

Interior ASCII blanks are permitted only when an even number of hexadecimal
digits lies to their right. Leading/trailing blanks, a blank followed by an odd
right-hand group, or any non-hexadecimal character raises `INVALID_ARGUMENTS`.

```rexx
x2b("C3")     /* "11000011" */
x2b("7")      /* "0111" */
x2b("1 C1")   /* "000111000001" */
x2b("0001")   /* "0000000000000001" */
x2b("")       /* empty */
```

The implementation performs one validation scan and one conversion scan. It
uses direct character-range arithmetic and appends four table bits per digit;
it does not perform whole-value numeric conversion or call another selector.

The separate Classic Level C BIF is documented in
[`lib/rxfnsc/x2b.md`](../../rxfnsc/x2b.md). The focused native harness is
`lib/rxfnsb/tests_functional/ts_x2b.crexx`.
