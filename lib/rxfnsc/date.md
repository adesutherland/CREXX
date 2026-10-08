# DATE - Level C Classic BIF

Last reviewed: 2026-10-08. See the
[detailed compatibility layer](../../compiler/docs/levelc_compatibility_layer.md#824-date)
and [worklist](../../docs/planning/release-1/levelc-compatibility-worklist.md)
for current compiled mapping, evidence and known defects.

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

The caller's `RexxVariablePool` owns the clock sample. `beginClauseTime()` can
refresh it. DATE and TIME currently call `ensureClauseTime()`, which takes
the first sample while `date_time_ready` is false. No production compiler/runtime call to
`beginClauseTime()` refreshes later clauses. A linked optimized/no-opt TIME
probe confirms identical current time and elapsed zero after a one-second sleep.
DATE shares that cached sample, including its current-year window; a calendar
crossing itself was not probed. The reproduced TIME defect and the shared
DATE cache leave production clause refresh unqualified. `injectClauseTime(...)` supplies deterministic
harness values and does not prove production clock progression.

Invalid input or supplying `inoption` without the second argument records
`RXC-LC-40.19`. Standard count, omission, and option errors are recorded by the
shared CheckArgs service. The direct optimized/unoptimized harness covers every
format, sliding-window boundaries, leap-day conversion, state injection, and
errors. Current compiler lowering reaches this standalone entry, and the
compiled reference audit checks explicit-date conversion in direct/linked
optimized/no-opt execution. Those results remain valid; they do not qualify
clock refresh, every timezone or DST boundary, or wider host lifecycle.
