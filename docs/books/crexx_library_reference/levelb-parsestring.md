## Level B `parsestring`

`parsestring` executes the legacy parallel-array plan produced by
[`parsecompile`](parsecompile.md):

```rexx
parsestring(parse_string = .string,
            tokenhi = .int,
            expose token = .string[],
            expose token_type = .int[],
            expose variable = .string[],
            expose variable_content = .string[],
            template = "unknown")
```

`token` and `token_type` are borrowed read-only references. The output arrays
are cleared and populated in template-variable order, with names in `variable`
and matching fields in `variable_content`. Absolute and relative positions use
one-based source positions; literal and prior-variable searches begin at the
current parse position. Kind 5 treats space, tab, CR, LF, vertical tab, form
feed, and U+00A0 as whitespace.

For example, compiling and executing `left','middle','right` against
`one,two,three` yields `one`, `two`, and `three`. A search item such as `(left)`
uses the value already assigned to `left` as the next delimiter.

The function checks `tokenhi`, every integer kind, and all position, separator,
and search payloads before clearing the output arrays. A malformed or truncated
plan signals `INVALID_ARGUMENTS` and leaves existing outputs unchanged. The
source and both plan arrays are never changed.

The implementation scans the source directly with VM string operations and
does not copy the unconsumed source tail for each delimiter search. This is a
Level B runtime helper, not a Level C BIF.
