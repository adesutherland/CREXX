## Level B `parsecompile`

`parsecompile` validates and compiles the legacy dynamic-template format used by
`parse` and `parsestring`:

```rexx
parsecompile(template = .string,
             expose token = .string[],
             expose token_type = .int[]) = .int
```

The return value is the number of plan items. `token` contains their payloads
and `token_type` contains integer kinds:

| Kind | Meaning | Example payload |
|---:|---|---|
| 1 | variable | `name` |
| 2 | quoted literal, without its outer quotes | `,` |
| 3 | absolute position | `10` |
| 4 | signed relative position | `+8` |
| 5 | implicit word separator | the original whitespace run |
| 6 | prior-variable delimiter search | `name` from `(name)` |

For example, `first','second third` produces variable `first`, literal `,`,
variable `second`, an implicit separator, and variable `third`. Whitespace is
preserved as kind 5 only between adjacent variables; whitespace touching a
quoted literal is suppressed. Single- and double-quoted literals are accepted,
and doubled quote pairs remain doubled in the literal payload.

The recognised whitespace characters are space, tab, CR, LF, vertical tab,
form feed, and U+00A0. Positions must fit a signed 64-bit native integer.

Malformed quotes, signed positions, parenthesized searches, or oversized
positions signal `INVALID_ARGUMENTS`. Validation finishes before either exposed
array is cleared, so both arrays remain unchanged when a signal is raised.

This is a Level B runtime helper, not a Level C BIF. The compiler `PARSE` exit
does not use this legacy dynamic-template representation: it validates the
source template in the exit and emits direct instructions or a packed
`parseplan` descriptor.
