# Essential source metadata and condition provenance

Status: bounded implementation approved and implemented, 15 October 2026.
Qualification is recorded under CP-AC01–06 in [PLAN.md](../PLAN.md).
[PLAN.md](../PLAN.md) owns acceptance and steps. Following Adrian's direction to
get the essentials right without a wholesale redesign, this correction keeps the
current diagnostic structure. Larger improvements are recorded separately.

## Intended outcome

C and G errors name useful, truthful source locations and retain the original
fault. Correct the present metadata and re-raise defects using existing Level B
facilities and the Level L emitter. No new C/G syntax, public condition class,
IR format, source-map companion, VM API or shadow stack is proposed.

Two locations have different meanings:

- **Fault:** the module/address where the original condition arose. For a
  Classic helper failure this can be inside the runtime library.
- **Authored context:** the C/G expression whose execution caused that helper
  call. It must not replace or be presented as the helper's fault address.

The existing `.signal` getters remain the surface: `name`, `code`, `message`,
`payload`, `module`, `address`, `file`, `line`, `column`, `source`. The source
getters describe the event's fault. They do not promise a complete stack or an
application source location when the fault actually occurred in a helper.

## Existing facilities and the important limitation

| Facility | Available now |
|---|---|
| `.srcstep` | Address-associated source anchor, clause ID, provenance flags, logical filename, line, columns and source text. |
| VM-bound `.signal` | Retains the original fault module/address and payload after delivery to a branch-value handler. |
| `signalorigin "NAME",message,event` | Re-raises with a bound event's original fault coordinates. Takes a literal signal name and string message; it is not an arbitrary object-payload-preserving rethrow. |
| Existing frames / `metaloadcalleraddr` | The immediate caller can be inspected in B. The default panic reporter shows a fault and one caller. There is no general B stack walk in this proposal. |
| `sigbrv` | Unwinds deeper frames before delivering the condition to its owning handler frame. That loses the deeper call chain, although the original fault survives. |

Consequently, metadata alone cannot recover the precise authored call site after
unwinding to the current function-level Classic handler. Removing its location
state immediately would lose information. Keep that state for this increment,
remove redundant updates, and make its fallback honest. Moving handlers into
outer adapters might later make the caller frame sufficient, but that changes
handler placement and per-call costs and is not needed for this repair.

These conclusions come from the existing
[`signal.crexx`](../../../rxfnsb/rexx/signal.crexx),
[`sourceinfo.crexx`](../../../rxfnsb/rexx/sourceinfo.crexx),
[`rxvmhandlers_control.inc`](../../../../interpreter/rxvmhandlers_control.inc)
and [`rxvmintp.c`](../../../../interpreter/rxvmintp.c). The
[VM guide](../../../../docs/ai-context/RXVM_INTERPRETER.md) and
[assembler guide](../../../../docs/ai-context/RXAS_ASSEMBLER.md) define the
underlying contracts. The implementation proof is retained separately from this mechanism account.

## Implemented essential correction

1. **Use sensible source names through the existing interface.** Keep the
   compile-service `filename` argument. Preserve explicitly supplied logical
   relative names; convert physical absolute filenames to their basename at the
   compiler boundary, and have the CLI pass the basename of the opened file.
   Preserve `<source>` for memory input. No new command-line option now. Service
   callers can already pass `examples/error-c.rexx` to disambiguate names.
   Identity across linked modules remains module plus local ID, not basename
   alone. A future project mapping option can address richer CLI naming.
2. **Correct existing source records.** Use the complete single-line expression
   range, Unicode columns and CRLF-aware lines. Preserve exact/derived/synthetic
   origin distinctions: generated code must not be marked AUTHORED/EXACT.
   For a multiline span, display its first-line segment with composite
   provenance and retain the full range in IR. Synthetic work without an
   authored origin has no authored-source claim. Test that mapping cannot bleed
   from the preceding routine into generated entry code.
3. **Do not invent clause fidelity.** The present IR-origin ID is not an authored
   clause ID. Use clause zero where the backend lacks a real clause association;
   this is the documented unknown/no-clause value. Proper frontend-to-IR clause
   grouping remains open. Do not add a new map format just to close that item
   during this diagnostic repair. Full TRACE acceptance remains open too.
4. **Remove redundant state updates, not needed information.** Suppress unchanged
   straight-path source records and Classic location-register writes. Reassert
   state at branch/loop entries where predecessors can leave different state.
   Different expressions on the same line can need distinct anchors. Do not
   globally deduplicate address-bearing metadata. Retain one authored-location
   field for the existing Classic handler while the VM cannot recover that
   location from an unwound stack.
5. **Replace the misleading entry fallback.** Before any authored operation has
   been entered, describe the logical file and routine-entry context explicitly,
   such as `error-c.crexx: entering $MAIN`. This is contextual text, not an invented
   exact source line. Once an authored operation starts, the retained location
   names its actual source anchor. Remove the generic `generated Classic entry`
   presentation; keep generated provenance in metadata where appropriate.
6. **Preserve the original fault when relaying the error.** Keep the existing
   function-level fatal handler. It can format the retained authored context on
   the error path, but the final raise should use a selected literal
   `signalorigin` form with the original bound event. A B helper may format text;
   it must not perform the current ordinary `signal` re-raise which resets the
   fault to that helper. No handler relocation or new stack structure is needed.

All compiler/adapter changes remain Level B. RXAS construction continues through
Level L libraries. Numeric snapshots, call argument flags, reference cleanup,
register allocation and optimisation are outside this correction.

## Condition semantics and honest limits

The admitted names stay `CLASSIC_SYNTAX`, `DIVISION_BY_ZERO`, `CONVERSION_ERROR`
and `INVALID_ARGUMENTS`. Preserve the original name, error number/message and
module/address; any authored-context prefix is added separately on the error
path, as it is today. Prevent the local handler catching its own re-raise, retain
nonzero status, and leave ignored overflow/lost-digits policy unchanged.
`sigshalt` is not suitable for this: its VM path returns zero.

The current target adapter and Classic checked-result failures carry strings;
the four VM-bound fatal dispatch arms are tested with string messages. This
does not preserve arbitrary structured conditions, intercept
`CLASSIC_CONDITION`, or widen the current standalone fatal profile.
General inherited handlers, structured payload propagation and embedded callback
caller fields require later review. This correction preserves the original fault
inside the existing limited profile; it does not establish full condition parity.

The payload audit for this admitted profile is:

| Producer | Retained contract |
|---|---|
| `llctarget.bad` | String Classic error number/detail, raised as `CLASSIC_SYNTAX`. |
| `rexxclassicbif_checked` in `RexxClassicBifs.crexx` | `context.errorDetail()` string, including division error `RXC-LC-42.3`; the original runtime helper remains the fault. |
| Unsupported Classic expression operator | Literal string `INVALID_ARGUMENTS`; no structured object payload. |
| VM division/conversion/invalid-argument failures | Existing fatal signal name/code and string or empty message. The four generated dispatch arms have explicit string-message regression probes. |

The generated handler saves the signal selector in a scratch register, disables
its four fatal catches, and calls the formatter using a disjoint ABI window.
The bound event remains in `r0`. It then selects a literal `signalorigin` with
the formatted message and that original event. This adds a small cold dispatch
sequence; it does not replace required context with a new stack or add a hot
helper call for each source anchor. Normal ABI flags and cleanup are unchanged.

With missing/stripped metadata, report module/address rather than guessing a
source file. Static linked modules remain live in this profile. INTERPRET will
need ownership or snapshots of code/source provenance until all referencing
conditions are released, including nested fragments; numeric addresses alone
are not durable after unloading or across VM runs. That remains STEP08/AC11.

## Simple C/G example

Use the existing divide-error fixtures, with logical filenames.

`error-c.rexx`:

```rexx
options levelc
say 1
say 1 / 0
```

`error-g.rexx`:

```rexx
options levelg
say 1
say 1 / 0
```

Both print `1`, then fail on line 3 before that SAY produces output. The focused
checks retain C exit 28 with `RXC-LC-42.3` and G exit 5 with a division error.
The following summarises their diagnostic content; the VM also prints numeric
module/address information:

```text
C: error-c.rexx:3:5: RXC-LC-42.3 ... (CLASSIC_SYNTAX)
   fault: original Classic helper module/address and source, when available

G: DIVISION_BY_ZERO
   fault: error-g.rexx:3:5 — say 1 / 0
```

For C the first location is retained authored context; the fault remains the
helper's actual location. For G the failing instruction is directly attributed
to the expression. Preserve that distinction rather than forcing identical
condition names or pretending the C helper fault occurred in the source file.

The expression `1 / 0` covers columns 5 through 9, end-exclusive 10. Its
metadata uses the existing format, for example:

```rxas
.srcstep 7 0 17 "error-c.rexx" 3 5 10 "say 1 / 0"
```

Here 7 is an illustrative anchor ID, zero honestly says clause grouping is not
available, and 17 is AUTHORED | EXACT. Generated setup uses appropriate generated
or synthetic provenance. Correct clause grouping is recorded as further work,
not falsely claimed by this example.

## Further work, without blocking this correction

- Real authored clause grouping carried through rewrites and separate compiler
  artifacts; complete TRACE events and richer multi-source/multiline origins.
- Review moving error interception into outer B adapters, measuring their entry
  cost and proving caller provenance before removing the remaining location
  field. Consider both C and G helper failures, not a Classic-only design.
- Full condition payload/handler propagation and complete backtraces, only with
  an explicit contract for what survives unwinding.
- INTERPRET source/code lifetime, nested execution and retained conditions.
- The emission review's separate unused defaults/frame layout and STEP05 shared
  optimisations. None is silently closed by better diagnostic output.

The approval covers only the six corrections above and their focused tests.
PLAN.md's CP criteria and steps retain the qualification boundary. The
[backend reference](backend.md), [user guide](../../doc/user-guide.md) and
[executable examples](../examples/step04/README.md) describe the implemented path.

An explicit no-source record clears metadata at generated entries and cold
handlers. The current VM may display this as `source: :0:0:` in its one-caller
view; it no longer attributes that code to the preceding routine. Fixing that
presentation belongs to the existing VM printer, not a new provenance stack.
With linker stripping, the source line is absent and the original module/address
remains. Binary module labels can still contain an assembler input path; those
are distinct from the logical C/G source names fixed here.
