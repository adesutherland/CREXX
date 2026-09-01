## fsayfmt

`fsayfmt` converts an FSAY template into a cREXX source expression:

```rexx
fsayfmt(template = .string) = .string
```

It is a source generator, not a runtime interpolation engine. Placeholders may
name a simple identifier or compound variable. General expressions and array
subscripts are rejected. The compiler-exit statement passes one quoted source
literal; direct calls normally pass the decoded template text.

```rexx
fsayfmt("Hello {name}")       /* 'Hello '||name */
fsayfmt("{name:10}")         /* left(name,10) */
fsayfmt("{price:8.2}")       /* format(price,8,2) */
```

The placeholder grammar is
`{variable[:alignment][width][.decimals]}`. Whitespace may replace the colon.
Unqualified text widths are left aligned. `<`, `>` and `^` select left, right
and centre alignment. Decimal formatting uses `FORMAT`; its default is right
alignment. A decimal precision may be supplied without a width as `.digits`.
Widths and decimal counts are non-negative decimal integers.

`{{` and `}}` emit literal braces. Quote characters inside the decoded template
are ordinary literal text. Generated literal segments always use single-quoted
cREXX source with embedded apostrophes doubled.

Malformed braces, placeholders, variable names, widths or decimal counts signal
`INVALID_ARGUMENTS`. The implementation decodes at most one outer source
literal and scans the resulting template once; it does not invoke the general
quote-aware library or rescan the growing output.

`lib/rxfnsb/tests_functional/ts_fsayfmt.crexx` covers source-literal decoding,
literal escaping, braces, compound names, all alignment modes, decimal formats,
Unicode text and each documented validation failure in optimized and
unoptimized Level B builds. The compiler-exit integration test additionally
executes the generated FSAY statements.
