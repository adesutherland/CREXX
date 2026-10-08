# TIME - Level C Classic BIF

Last reviewed: 2026-10-08. See the
[detailed compatibility layer](../../compiler/docs/levelc_compatibility_layer.md#856-time)
and [worklist](../../docs/planning/release-1/levelc-compatibility-worklist.md)
for current compiled mapping, evidence and known defects.

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
The elapsed/reset origin persists across explicitly refreshed or injected samples.
However, no production compiler/runtime caller invokes `beginClauseTime()` at
later clauses: `ensureClauseTime()` keeps the ready pool's first sample. A
linked optimized/no-opt probe on the reviewed baseline reads the same TIME(L)
and TIME(E)=0 after a one-second ADDRESS SYSTEM sleep; Regina advances. This
verified integration defect leaves real progression of current time, elapsed
time and reset state unqualified. The exact receipt is retained by the worklist. DATE shares this
sample, but a calendar-boundary crossing was not probed.

Invalid input or supplying `inoption` without the second argument records
`RXC-LC-40.19`; converting to `E`, `R`, or `O` records `RXC-LC-40.29`.
Standard count, omission, and option errors come from shared CheckArgs.

The direct optimized/unoptimized harness covers every option and conversion,
midnight/noon civil forms, six-digit fractions, frozen state, elapsed/reset,
offset, live first-use freezing, and errors. Compiler lowering now reaches this
standalone entry; the compiled reference audit checks explicit-time conversion
in direct/linked optimized/no-opt execution. Injected elapsed/reset checks do
not prove real multi-clause time progression. The separate Level B VM-clock
extension is documented in `lib/rxfnsb/rexx/time.md`; it is not the shared
Classic pool clock or evidence that its refresh gap is fixed.
