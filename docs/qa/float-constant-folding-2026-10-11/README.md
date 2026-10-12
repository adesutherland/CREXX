# Typed float folding local repair receipt — 11 October 2026

The repair is local and uncommitted against develop `bb0d211a8d17` (full base
revision and qualified input/tool SHA-256 hashes in `manifest.json`). It is
unpublished. The separate Level L research checkout was read only.
The authoritative scope and AC-01–AC-04 acceptance record is
[`compiler/docs/float_constant_folding_worklist.md`](../../../compiler/docs/float_constant_folding_worklist.md).

## Cause and repair

Fresh current-develop Debug tools reproduce the retained finding in B and G:
`0.1 + 0.2 = 0.3` returns `0` without compiler optimization and `1` with it,
independent of assembler optimization and concrete VM. The subtraction
`(0.1 + 0.2) - 0.3 > 0.0` returns `1` without optimization and `0` with it.
Decimal equality returns `1` throughout. `before.json` retains exact commands,
outputs and pre-repair tool/source hashes; `*-before.rxas.txt` retains assembly.

Binary64 addition produces `0.30000000000000004`, bits `0x3fd3333333333334`.
The literal `0.3` has bits `0x3fd3333333333333`; the positive residual is
`5.5511151231257827e-17`. `rewrite_to_float_constant()` first stores the correct
double, then `update_string()` uses contextual display formatting, capped at
`DBL_DIG` (15). Its `0.3` text was reparsed by `string_to_type(TP_FLOAT)`, losing
one bit in this example. Both named and inferred constant propagation repeated
this reparse. RXAS float emission independently used display text instead of
the typed value, also rounding raw optimized literals under small DIGITS.

`typed_constant_to_type()` preserves the already computed/copied float payload
when its target is still float. Raw literals continue to parse their source
text normally. Typed float emission uses `%.17g` and retains a float lexical
marker for integer-looking values; unoptimized raw literals emit their original
source spelling. Display formatting, numeric contexts and effective strict/loose
string comparisons retain their owning behavior. AST clones copy the double;
inline metadata already transports it with `%.17g`/`atof`. RXAS parses it to
double and pools/deduplicates exact bits; linking preserves the shared-pool value.

`after.json` and `*-after.rxas.txt` retain the original comparison/residual/
decimal matrix after the repair: `0`, `1`, `1` in every cell.

## Permanent regression and validation

`float_constant_folding_contract` has 64 VM executions and 156 checked tool
commands: two language levels, two compiler modes, two assembler modes, both
concrete VMs, and source/RXAS/RXBIN import routes. Every image is linked before
execution. Exact output comparisons cover the original case, positive residual,
equal binary-float and decimal controls, raw literals, named/inferred constants,
integer-looking float transport, subsequent runtime arithmetic, display,
strict/loose/mixed string comparisons and local numeric contexts. CLI arguments
keep the reference addition at runtime. Assembly assertions require exact float
text, a remaining runtime `fadd`, and fewer additions after optimization.
Provider-discovery assertions prevent a binary sibling from substituting for a
requested source/RXAS route; `import-routes.json` retains the final selections.

Both isolated pre-registration Debug/ASan aggregate runs passed. Registered
final workload: normal Debug **0.50 seconds**, maintained Apple ASan **8.03
seconds**. Earlier measurements 0.75/9.49 seconds also pass. CTest uses
`RUN_SERIAL` and a generous 120-second hang-protection limit. No broad,
overnight, hosted, release, or performance-programme gate was dispatched.

The relevant existing normal suite passed 28 checks, covering numeric/decimal
optimizer safety, strict comparison code generation and runtime behavior,
mixed comparison, named constant payloads, contextual decimal literals,
inline constant-argument mutation, numeric contexts and B/G BIF contexts in
linked/unlinked compiler modes. The initial sweep passed 27 existing checks
plus the new aggregate; one expected RXAS golden failed. That golden had
required `1.000000001` to become `1.0` under DIGITS 9. Only its incorrect load
operand changed to `1.0000000010000001`, and the failed check passed on replay.
Unchanged passing checks were reused. The final new aggregate was rerun after
adding mixed-loose and integer-looking transport controls.

Fresh core tools and normal Debug tools/libraries built with **zero warnings**;
full logs are retained without filtering. The maintained ASan build passes and
reports no address-sanitizer finding, but it emits existing Apple SDK `sprintf`
deprecation warnings in unchanged sources (retained in `focused-asan-build.log`).
Apple LeakSanitizer is unavailable, so runner leak options are off for that
platform. This receipt does not claim Linux LSan or cross-platform qualification.

Retained evidence:

- `manifest.json`: exact qualified input/tool/library hashes, base revision,
  commands and normal warning counts.
- `before.json`, `after.json`, `*-before.rxas.txt`, `*-after.rxas.txt`:
  independently built current-develop reproduction and repair outputs.
- `fresh-core-build.log`, `repaired-core-build.log`,
  `normal-tools-libraries-build.log`: successful build diagnostics.
- `existing-regressions-initial.log`, `numeric-golden-retest.log`:
  complete initial results, expected golden mismatch and decisive retry.
- `focused-debug-final.log`, `focused-asan-final.log`,
  `focused-debug-commands.log`, `focused-asan-commands.log`,
  `focused-asan-build.log`, `focused-asan-environment.txt`:
  exact matrix commands/outcomes and sanitizer evidence.
- `import-routes.json`: all 12 intended provider route/mode selections.

## Research disposition and limits

This resolves the reproduced **RS-F01** binary-float compiler discrepancy and
addresses the compiler-conformance blocker cited under **RS-AC04**. The research
owner must adopt the repair and reconcile/requalify its recorded acceptance;
this task deliberately does not overwrite historical outputs or change that
separate worklist. It does not establish completion of the wider Level L work.

No remaining precision-loss failure was observed in the qualified finite-float
folding, constant propagation, raw/typed emission or imported inline paths.
Non-finite arithmetic and other platforms are outside this focused evidence;
there is no new claim that all floating-point edge behavior is qualified.

A subsequent requested [related-defect scan](related-scan/README.md) confirmed
eight open findings in integer transport/conversion, cross-type float casts,
NaN comparison, subnormal acceptance, non-finite transport, signed zero and
explicit casts bypassed by string consumers.
The original repair and its qualified inputs are unchanged. Those findings
extend the investigation beyond this receipt's original regression panel and
must be repaired before claiming wider numeric optimizer equivalence.

## Subsequent full CTest and publication

The local-only status above describes the initial repair checkpoint. Adrian
subsequently authorized the follow-up repairs and full CTest/develop publication.
See the [publication receipt](publication/README.md) for the final 3,329-test
qualification, focused correction/ASan proof and expected hosted status. The
research agent owns adoption and qualification after publication.
