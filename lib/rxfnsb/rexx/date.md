# Level B `date`

The typed Level B calendar extension is:

```rexx
date([oformat="" [,idate="" [,iformat="" [,osep="" [,isep=""]]]]]) = .string
```

An empty `idate` uses the current local date and ignores `iformat`. The default
format is `NORMAL`. Calendar years are the proleptic Gregorian range 1 through
9999. Every result, including BASE, UNIX, EPOCH, and JDN, is a `.string`.

This is deliberately distinct from the three-argument Classic Level C DATE
BIF documented in `lib/rxfnsc/date.md`.

## Formats

Format names are case-insensitive documented abbreviations. X variants carry
a four-digit year; the corresponding short numeric input variants map `00`
through `99` deterministically to 2000 through 2099.

| Format | Output example for 1962-03-10 | Input |
|---|---|---|
| `NORMAL` / `N` | `10 Mar 1962` | yes |
| `XNORMAL` / `XN` | `10 March 1962` | yes |
| `STANDARD` / `ST` | `19620310` | yes |
| `ORDERED` / `O` | `62/03/10` | yes, interpreted as 2062 |
| `XORDERED` / `XO` | `1962/03/10` | yes |
| `EUROPEAN` / `E` | `10/03/62` | yes, interpreted as 2062 |
| `XEUROPEAN` / `XE` | `10/03/1962` | yes |
| `GERMAN` / `G` | `10.03.62` | yes, interpreted as 2062 |
| `XGERMAN` / `XG` | `10.03.1962` | yes |
| `USA` / `U` | `03/10/62` | yes, interpreted as 2062 |
| `XUSA` / `XU` | `03/10/1962` | yes |
| `INTERNATIONAL` / `IN` | `1962-03-10` | yes |
| `QUALIFIED` / `QU` | `Saturday, March 10, 1962` | yes; weekday is validated |
| `JULIAN` / `J` | `1962069` | yes |
| `BASE` / `B` | `716308` | yes; days since 0001-01-01 |
| `UNIX` / `UN` | `-2854` | yes; days since 1970-01-01 |
| `EPOCH` / `EP` | `-246585600` | yes; seconds since 1970-01-01 |
| `JDN` | `2437734` | yes |
| `DAYS` / `D` | `069` | output only |
| `WEEKDAY` / `W` | `Saturday` | output only |
| `MONTH` / `M` | `March` | output only |
| `CENTURY` / `C` | `22714` | output only |
| `DEC` | `10-03-62` | yes, interpreted as 2062 |
| `XDEC` | `10-03-1962` | yes |
| `SORTED` / `S` | `19620310` | output only |

`S` means SORTED on output and STANDARD on input. Use `ST` when naming
STANDARD output explicitly.

## Separators and errors

`osep` and `isep` are empty or exactly one Unicode codepoint. A supplied
separator replaces the format's normal separator; SORTED normally has none.
For example:

```rexx
date("XG", "10•03•1962", "XE", "•", "•") -- 10•03•1962
```

Invalid dates, unknown or output-only input formats, malformed fields, range
errors, and overlong separators signal `INVALID_ARGUMENTS`. Inputs are scanned
directly and do not print diagnostics or normalize invalid calendar fields.

`ts_date`, `ts_date2`, and `ts_date_core` exercise every table row, the Unicode
example, Gregorian boundaries, invalid inputs, and optimized/unoptimized code.
