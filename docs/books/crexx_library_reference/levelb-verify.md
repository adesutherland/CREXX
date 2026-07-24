## Level B `verify`

`verify` locates the first character that either is or is not in a reference
table:

```rexx
verify(string = .string, reference = .string,
       option = "N", start = 1) = .int
```

`option` is case-insensitive and only its first character is significant:

- `N` (the default) returns the first character not in `reference`.
- `M` returns the first character in `reference`.

`start` is an optional positive 1-based integer. The result is a 1-based
Unicode character position, or zero when no qualifying character exists. An
empty source returns zero. With an empty reference, Nomatch returns `start`
when it is within the source and Match returns zero.

```rexx
verify("1Z3", "1234567890")          /* 2 */
verify("AB4T", "1234567890", "M")  /* 3 */
verify("1P3Q4", "1234567890",,3)   /* 4 */
verify("é🙂x", "🙂", "Match")       /* 2 */
```

An empty/invalid option or a non-positive start raises `INVALID_ARGUMENTS`.
The source and reference are not modified. Their lengths are cached, and each
source character is checked by one native VM character-table scan; no substring
or temporary character string is allocated.
