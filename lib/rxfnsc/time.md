# TIME - Level C Classic BIF

Last reviewed: 2026-10-21. See the
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

Compiled DATE and TIME share the current activation's lazy `RexxDateTimeState`
through `RexxBifCallContext`. `beginClauseTime()` invalidates the clause sample;
`ensureClauseTime()` freezes local/UTC time, offset and base day on first use.
Nested invocations sample independently while the existing program-root
elapsed/reset lifetime remains shared. Direct library contexts without an
activation keep the injectable `RexxVariablePool` fallback, including
`injectClauseTime(base_day, local_microseconds, utc_microseconds,
offset_microseconds)` for deterministic tests.

The research C compiler's
[CLAUSE1 contract](../../research/level-l/compilers/doc/levelc-architecture.md#clause-clock-contract)
owns clause-begin placement and bounded compiled progression qualification.
Injected samples alone do not establish real clause progression or broader
conditions/TRACE, host and platform clock behavior.

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
Classic activation clock or evidence of its compiled clause integration.
