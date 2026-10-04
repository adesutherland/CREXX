# The `OPTIONS` Instruction

For Level B, a leading `OPTIONS` clause configures file-level parsing rules and
language defaults. File-level choices must be in the first instruction because
the compiler must know them before parsing the rest of the file.

```rexx
options levelb numeric_common comments_slash
```

## Language Level

The Release 1 beta line documents Level B as the supported compiler language:

```rexx
options levelb
```

Other level names identify historical, directional, or DSLSH tooling surfaces.
See [Language levels](crexx_levels.md) for the full catalogue. Do not treat a
level name as a release compiler language unless the page describing it
explicitly says so.

The compiler can also receive a default with `rxc --level levelb`. A source
file's explicit `options` line overrides that command-line default.

## Level C Compatibility

Level C compatibility is under development. Its leading, bare-word `OPTIONS`
clause can select the source level and file-level comment and numeric settings.
Those settings are fixed before parsing. A first clause that is an expression,
such as `options choose()`, is executable source; use `rxc --level levelc` when
there is no static `options levelc` selector.

Every Level C `OPTIONS` clause, including the leading clause, also executes at
its source position. Its expression is evaluated once. Later clauses cannot
change how already parsed source is recognized. With no operand, `OPTIONS` is
a no-op, matching Regina. The BYTE and UTF8 Level C profiles currently define
no runtime option words, so the runtime ignores their evaluated values,
including unknown words and IBM EBCDIC DBCS options such as `ETMODE` and
`EXMODE`. An expression's side effects still occur.

Level C defaults to Classic `/* ... */` comments and Classic numeric
precedence. A leading static clause may enable `comments_hash` or
`comments_dash`, or explicitly disable them. `comments_slash` is rejected in
Level C because `//` is the Classic remainder operator. `numeric_common` is
also rejected in Level C; use `numeric_classic` or the default.

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

For Level B, hash comments are enabled by default so a POSIX shebang can be
used:

```rexx
#!/usr/bin/env crexx
options levelb
```

Use `comments_slash` to enable `//` comments and `comments_dash` to enable
`--` comments.

Block comments use the traditional REXX `/* ... */` form.

## Floating-Point Type

Level B can select how `.float` source values are treated:

- `floats_binary`: binary floating point, the default
- `floats_decimal`: decimal floating point treatment where supported

Use `.decimal` explicitly when decimal behaviour is part of a published
signature or value contract. With the default treatment, an otherwise ordinary
dotted literal is nevertheless parsed directly from its source spelling when
an enclosing decimal assignment, argument, return, cast, constructor, or
expression supplies the expected `.decimal` type. `floats_binary` and an
explicit `.float(...)` boundary force binary treatment; a `d` suffix explicitly
selects decimal.
