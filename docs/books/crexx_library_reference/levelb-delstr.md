## Level B `delstr`

`delstr` removes a character range from a string:

```rexx
delstr(string = .string, start = .int,
       delete_length = 0) = .string
```

`start` is a positive 1-based Unicode character position. When
`delete_length` is omitted, deletion continues through the end. An explicitly
supplied zero deletes nothing. A start beyond the string also returns the source
unchanged.

Nonpositive `start` or a negative supplied `delete_length` raises
`INVALID_ARGUMENTS`.

```rexx
delstr("abcd", 3)     /* "ab" */
delstr("abcde", 3, 2) /* "abe" */
delstr("abcde", 6)    /* "abcde" */
delstr("abcde", 3, 0) /* "abcde" */
```

The source is read-only. The implementation distinguishes omission through
argument-presence metadata, computes length once, and extracts at most one
prefix and one suffix with direct VM cursor/subslice operations.

`lib/rxfnsb/tests_functional/ts_delstr.crexx` covers omitted/zero/oversized
lengths, boundary starts, empty/Unicode input, signals, and non-mutation in
optimized and unoptimized overlays.
