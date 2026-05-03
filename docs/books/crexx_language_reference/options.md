# The `OPTIONS` Instruction

`OPTIONS` configures file-level parsing rules and language defaults. When it is
present, it must be the first instruction in the source file.

```rexx
options levelb numeric_common comments_slash
```

## Language Level

Release 1 beta 1 documents Level B as the supported language level:

```rexx
options levelb
```

Other level names may appear in project discussions or experimental code, but
they should not be treated as release languages unless the page describing them
explicitly says so.

The compiler can also receive a default with `rxc --level levelb`. A source
file's explicit `options` line overrides that command-line default.

The `crexx` driver compiles headerless top-level scripts as Level B and imports
`rxfnsb` for convenience. Reusable modules should still write the option and
imports explicitly.

## Arithmetic Standard

The file-level arithmetic option changes parser behaviour for arithmetic
expressions and sets the default `NUMERIC STANDARD` for procedures that do not
override it.

### `numeric_common`

`numeric_common` is the Level B default. It follows common C-like precedence
and associativity choices:

- prefix minus has lower priority than power, so `-3**2` is parsed as
  `-(3**2)`
- power is right-associative, so `2**2**3` is parsed as `2**(2**3)`
- `%` is the common integer/remainder-style operator spelling in Level B

### `numeric_classic`

`numeric_classic` follows Classic REXX arithmetic parsing choices where that
mode is used:

- prefix minus has higher priority than power, so `-3**2` is parsed as
  `(-3)**2`
- power is left-associative, so `2**2**3` is parsed as `(2**2)**3`
- `//` is the Classic remainder spelling

`NUMERIC STANDARD` inside a procedure controls numeric semantics, but it does
not reparse expressions. Parser-level choices belong to file-level `OPTIONS`.

## Comment Style

The single-line comment controls are:

- `comments_hash` / `comments_nohash`
- `comments_slash` / `comments_noslash`
- `comments_dash` / `comments_nodash`

Hash comments are enabled by default so a POSIX shebang can be used:

```rexx
#!/usr/bin/env crexx
options levelb
```

Use `comments_slash` to enable `//` comments and `comments_dash` to enable
`--` comments.

Block comments use the normal REXX `/* ... */` form.

## Floating-Point Type

Level B can select how `.float` source values are treated:

- `floats_binary`: binary floating point, the default
- `floats_decimal`: decimal floating point treatment where supported

Use `.decimal` explicitly when decimal behaviour is part of a published
signature or value contract.
