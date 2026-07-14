# DATE - Level C Classic BIF

The standalone direct Level C entry point is:

```rexx
result = rexxclassicbif_date(reference context)
```

It implements the Classic contract `DATE([option [,date [,inoption]]])` with
checklist `oBDEMNOSUW oANY oBDENOSU`. Arguments, presence information, result,
and errors use `RexxValue` and `RexxBifCallContext`; it is not added to the
deprecated name-based compatibility controller.

| Option | Result/input form |
|---|---|
| `B` | days since 0001-01-01 |
| `D` | day of current clause year, 1 through 365/366 |
| `E` | `dd/mm/yy` |
| `M` | full English month name (output only) |
| `N` or omitted | `d Mon yyyy` |
| `O` | `yy/mm/dd` |
| `S` | `yyyymmdd` |
| `U` | `mm/dd/yy` |
| `W` | full English weekday (output only) |

Short-year input uses the current frozen clause year and the Classic 50-year
sliding window. For a current year of 2024, `74` means 1974 and `73` means
2073. Input conversion is exact: formatting the parsed result back with the
input option must reproduce the supplied text.

The caller's `RexxVariablePool` owns the frozen clock. Future compiler lowering
can call `beginClauseTime()` at clause boundaries; direct calls freeze on first
use, while harnesses can use `injectClauseTime(...)` for deterministic values.

Invalid input or supplying `inoption` without the second argument records
`RXC-LC-40.19`. Standard count, omission, and option errors are recorded by the
shared CheckArgs service. The direct optimized/unoptimized harness covers every
format, sliding-window boundaries, leap-day conversion, state injection, and
errors. Compiler/lowering changes remain outside this library batch.
