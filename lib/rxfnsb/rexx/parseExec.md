## Level B `parseexec`

`parseexec` executes the frozen stream format emitted by the compiler PARSE
exit:

```rexx
parseexec(src = .string,
          splan = .string,
          template = .string,
          debug = .int optional) = .string[]
```

Signed relative positions following a literal are anchored at the literal's
first matching character, including when a drop target occurs between the
literal and the signed position. Templates with this combination are kept on
this Level-B path because the compact kernel `parseplan` descriptor does not
carry that literal-match anchor.

This is deliberately separate from the legacy `parsecompile`/`parsestring`
format. Each stream item is encoded as `kind,length:text;`, where `length` is
the number of Unicode codepoints in `text`:

| Kind | Meaning |
|---:|---|
| 1 | assignment target (`.` is a drop target) |
| 2 | literal boundary |
| 3 | absolute position |
| 4 | relative forward position |
| 5 | relative backward position |
| 6 | implicit next-word control; payload must be `{implicit}` |

For example, `3,1:1;1,5:first;6,10:{implicit};1,6:second;` assigns the first
two blank-separated words. A drop target still occupies its aligned result
slot. Literal payloads may contain commas, colons, and semicolons because their
length, rather than punctuation, defines the payload boundary.

The decoder validates the stream as it reads it and retains only the current
item, two lookahead items, and any controls that precede the next target. It
does not materialize parallel arrays for the whole plan. This bounds normal
decoder storage independently of plan length while preserving shared-boundary
and compiler overlay semantics.

`debug` accepts `0`, `1`, or `9`. Invalid punctuation, length, kind, numeric
payload, implicit payload, target, truncation, or debug level signals
`INVALID_ARGUMENTS` using normal Level B signal handling.

`parseexec` is an internal Level B compiler-runtime helper, not a Level C BIF.
Its format is frozen by direct compiler-exit and runtime tests.
