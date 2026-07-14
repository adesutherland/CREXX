# TIME - Level C Classic BIF

The standalone direct Level C entry point is:

```rexx
result = rexxclassicbif_time(reference context)
```

It implements `TIME([option [,time [,inoption]]])` with checklist
`oCEHLMNORS oANY oCHLMNS`. Arguments, presence information, result, and errors
use `RexxValue` and `RexxBifCallContext`; it is not routed through the
deprecated name-based compatibility controller.

| Option | Result/input form |
|---|---|
| `C` | civil `h:mmam` or `h:mmpm` |
| `E` | seconds since the activation's elapsed origin (output only) |
| `H` | hour 0 through 23 |
| `L` | `hh:mm:ss.ffffff` |
| `M` | minutes since local midnight |
| `N` or omitted | `hh:mm:ss` |
| `O` | local UTC offset in microseconds (output only) |
| `R` | elapsed seconds followed by reset (output only) |
| `S` | seconds since local midnight |

Input conversion is exact: formatting the parsed value with `inoption` must
reproduce the supplied text. `E`, `R`, and `O` cannot be conversion targets.

DATE and TIME share one `RexxDateTimeState` in the caller's
`RexxVariablePool`. `beginClauseTime()` captures and freezes local time, UTC
time, offset, and local base day. A direct BIF freezes on first use when no
sample exists. `injectClauseTime(base_day, local_microseconds,
utc_microseconds, offset_microseconds)` is the deterministic harness API.
Elapsed/reset origin persists across new clause samples in the activation.

Invalid input or supplying `inoption` without the second argument records
`RXC-LC-40.19`; converting to `E`, `R`, or `O` records `RXC-LC-40.29`.
Standard count, omission, and option errors come from shared CheckArgs.

The direct optimized/unoptimized harness covers every option and conversion,
midnight/noon civil forms, six-digit fractions, frozen state, elapsed/reset,
offset, live first-use freezing, and errors. Compiler/lowering changes remain
outside this library batch. The separate Level B VM-clock extension is
documented in `lib/rxfnsb/rexx/time.md`.
