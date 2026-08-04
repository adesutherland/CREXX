## abbrev

`abbrev` tests a case-sensitive leading abbreviation:

```rexx
abbrev(string = .string, candidate = .string, minimum = 0) = .int
```

It returns `1` only when all of these are true:

- `candidate` is no longer than `string`;
- `candidate` contains at least `minimum` characters; and
- every candidate character equals the corresponding leading character of
  `string`.

The default minimum is zero. For an omitted minimum this produces the same
prefix result as the Classic default of the candidate length. An empty
candidate therefore matches unless a positive minimum is supplied.

`minimum` is an integer Level B argument. A negative value raises the
`INVALID_ARGUMENTS` signal rather than being silently accepted.

```rexx
abbrev("PRINT", "PRI")     /* 1 */
abbrev("PRINT", "Pri")     /* 0 */
abbrev("PRINT", "PRI", 4)  /* 0 */
abbrev("PRINT", "")        /* 1 */
```

The implementation compares codepoints directly and checks both lengths before
reading either string. `lib/rxfnsb/tests_functional/ts_abbrev.crexx` covers the
examples, short-source safety, Unicode, exact minimum boundaries, qualified
calls, and the negative-length signal.
