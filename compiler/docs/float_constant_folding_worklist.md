# Typed float constant folding repair

## Vision and intended outcomes

Repair the optimization-equivalence defect exposed by Level L/RexxScript
qualification: binary `0.1 + 0.2 = 0.3` must be false with and without compiler
optimization on both concrete VMs. Preserve valid constant folding, the exact
typed double through propagation and RXAS/assembler transport, and existing
display, strict/loose string comparison, decimal and numeric-context behavior.
Level B and Level G share the affected backend and both require proof.

Work in the existing develop checkout and preserve unrelated work. Initial
local review delivery is retained below as history. Adrian subsequently
approved the RXAS inf/nan literal extension and full CTest followed by develop
publication. The research agent owns pull/merge and qualification after that
publication; this task does not modify its checkout, overwrite evidence or
claim wider research completion. Keep the arithmetic semantics, display
precision and valid folding: no runtime rounding, tolerance comparison or
broad optimization disablement.

## Acceptance criteria

1. **AC-01 — verified:** Reproduce with a fresh current-develop product build;
   retain B/G optimized/unoptimized outputs on rxtvm/rxbvm and explain the
   precise information loss with source/assembly evidence.
2. **AC-02 — verified:** Preserve typed float values through folding,
   propagation and RXAS/assembler transport. Original comparison, subtraction
   residual, equal-float control, and a folded value used by later runtime
   arithmetic agree across both compiler modes, both assembler modes and VMs.
   Compare a forced runtime calculation with its folded counterpart and prove
   valid folding remains enabled. Inspect relevant import/constant paths.
3. **AC-03 — verified:** Preserve user-visible numeric formatting, strict string
   comparison, loose string comparison, decimal arithmetic and local numeric
   contexts. Permanent controls and relevant existing regressions must pass.
4. **AC-04 — verified:** Add permanent focused regressions, retain exact source,
   build/tool hashes, commands, results and validation logs. Successful core
   build is warning-free; relevant normal numeric/comparison/propagation tests
   pass. Report RS-F01/RS-AC04 disposition without editing research evidence.

## Implementation steps

1. **STEP-01 — complete (AC-01):** Read owning docs/source and retained
   research evidence, verify develop synchronization, build fresh core tools,
   reproduce B/G matrix before changing compiler code. No dependency.
2. **STEP-02 — complete (AC-01, AC-02, AC-03):** Trace rewrite, conversion,
   comparison, propagation/import and emission paths. Separate typed value
   transport from contextual display. Depends on STEP-01. Stop for direction
   only if a language or architectural decision becomes necessary.
3. **STEP-03 — complete (AC-02, AC-03, AC-04):** Implement bounded value-
   preservation repair and permanent tests using existing repository patterns;
   test intended failure before repair where practical. Depends on STEP-02.
4. **STEP-04 — complete (AC-02, AC-03, AC-04):** Build core product and run
   focused B/G compiler/assembler/VM matrix plus relevant existing normal
   correctness tests. Measure any new aggregate in isolated Debug and maintained
   ASan before registration, with explicit scheduling/timeouts. Retain exact
   evidence; reuse valid unchanged results. No broad/overnight or hosted runs.
5. **STEP-05 — complete (AC-01–AC-04):** Update owning documentation and this
   worklist with cause, evidence, verified criteria and remaining limits;
   review diff and report local review delivery and research discrepancy
   disposition. Depends on STEP-04; no commit/push or research edits.

## Evidence and status

- Starting develop revision: `bb0d211a8`; clean and synchronized with remote
  develop on 11 October 2026.
- Historical finding: separate research `research/level-l/PLAN.md`, RS-F01 and
  RS-AC04; `evidence/product-rxas/final/float-reproducer/receipt.json` retains
  noopt `0` / opt `1` on both VMs. These files are read-only for this task.
- Fresh Debug core build (`cmake-build-float-repair`, parser mode/llama off)
  passes without warnings. B/G x compiler opt/noopt x assembler opt/noopt x
  rxtvm/rxbvm reproduces equality noopt `0` / opt `1`, positive residual
  noopt `1` / opt `0`, decimal control `1` throughout: `before.json` and
  `*-before.rxas.txt` under the evidence directory below.
- Cause: `rewrite_to_float_constant()` stores the exact binary64 result then
  `update_string()` formats using the numeric context (`extract_double_decimal`
  caps at `DBL_DIG`, 15), producing `0.3` from `0.30000000000000004`.
  `string_to_type(TP_FLOAT)` reparses it even when already typed float. Both
  constant-propagation paths repeat that conversion. `format_constant(TP_FLOAT)`
  emits contextual text, independently rounding transport. AST cloning copies
  the double; inline metadata already writes `%.17g` and imports with `atof`.
- Repair: `typed_constant_to_type()` skips same-type float reparsing only for
  already computed/copied constants. Raw literals still initialize their typed
  value through `string_to_type()`. Emit typed double constants with `%.17g`
  plus a float lexical marker for integer-looking results; raw unoptimized
  literals retain source spelling. Contextual `node_string` and effective-
  string comparison behavior remain unchanged.
- Validation evidence is retained under
  `docs/qa/float-constant-folding-2026-10-11/`.
- Permanent `float_constant_folding_contract`: 64 VM executions / 156 tool
  commands across B/G, both compiler modes, both assembler modes, both VMs,
  linked images and isolated source/RXAS/RXBIN imports. Reports assert intended
  provider selection. CLI inputs keep the reference arithmetic at runtime;
  RXAS assertions prove exact float transport and retained constant folding.
  Formatting, strict/loose/mixed comparisons, decimal, low digits and
  engineering/upper-case contexts pass. Final Debug 0.50 s / ASan 8.03 s;
  pre-registration aggregate passes and earlier 0.75 s / 9.49 s measurements
  support `RUN_SERIAL`, 120-second generous hang protection.
- Relevant existing normal regressions: all 28 pass after one intentional
  golden correction. `opt_numeric_safety_opt` had encoded the same defect:
  `1.000000001` under DIGITS 9 became `1.0`; it now transports
  `1.0000000010000001`. Only this failed check was repeated; unchanged passing
  checks were reused. Final matrix was repeated after adding mixed-loose and
  integer-looking float transport controls; no product code changed.
- Fresh core and normal tools/library builds have zero warnings. Maintained
  Apple ASan passes without sanitizer findings; its build retains existing
  `sprintf` deprecation warnings in unchanged code. Apple LSan is unavailable;
  this is local address-sanitizer proof, not Linux LSan/cross-platform closure.
- Exact inputs/tools, commands, logs and results:
  [validation receipt](../../docs/qa/float-constant-folding-2026-10-11/README.md).
- RS-F01: the reproduced compiler discrepancy is resolved by this local repair.
  The compiler-conformance blocker recorded under RS-AC04 is addressed; its
  research owner must reconcile/requalify the research checkout after adopting
  the repair. Research history/status files were not edited, and wider Level L
  completion is not claimed.
- Delivery status: complete within the requested repair scope, local,
  uncommitted and unpublished for review. All four criteria verified.

## Follow-up scan — 11 October 2026

Vision: audit analogous loss of typed numeric values and nearby optimization-
equivalence defects after the finite-float repair, distinguishing confirmed
reproducers from source-only concerns. Preserve the completed repair and its
evidence; keep all work local and leave the research checkout read only. This
follow-up is investigation and evidence, without changing compiler semantics.

1. **SCAN-AC01 — verified:** Inspect remaining scalar conversion and constant-
   propagation boundaries, integer/decimal equivalents, inline/contract and
   assembler/disassembler transport, and float exceptional values.
2. **SCAN-AC02 — verified:** Confirm candidates with bounded minimal B/G probes,
   comparing compiler and assembler modes on both concrete VMs where applicable.
   Retain source/tool hashes, commands and outputs; avoid broad repeated QA.
3. **SCAN-AC03 — verified:** Record stable findings with cause, severity, affected
   paths, reproducer and next action; report checked controls and limits.

1. **SCAN-STEP01 — complete (SCAN-AC01):** Trace code and owning behavior
   docs, prioritize places that reconstruct typed numbers from display strings.
2. **SCAN-STEP02 — complete (SCAN-AC02):** Run minimal probes using the existing
   qualified Debug tools; deepen only candidates that produce a discrepancy.
3. **SCAN-STEP03 — complete (SCAN-AC03):** Retain a scan receipt under the same
   QA directory and record confirmed follow-ups here. Language/design changes,
   new repairs, and wider research acceptance remain outside this scan.

### Confirmed findings — repaired locally

At scan completion, all eight findings reproduced on B and G with both
compiler modes and both assembler modes. Runtime manifestations agree on rxtvm/rxbvm; optimized
compile/assembly failures occur before execution. Source/tool hashes, complete
commands/outcomes, minimal sources, relevant RXAS and controls are retained in
the [scan receipt](../../docs/qa/float-constant-folding-2026-10-11/related-scan/README.md).
The initial repair's two core source hashes are unchanged by this investigation.
These are correctness findings, not maintained sanitizer diagnostics.

| ID / status | Priority and confirmed behavior | Owning cause / next action |
| --- | --- | --- |
| **FSCAN-01 — repaired** | High: integer folding/propagation at DIGITS 5 changes `9007199254740993` to `9007199254740992`; equality changes `1` to `0`, and a valid INT64_MAX constant can raise compiler overflow. Even `123456` used with later runtime arithmetic emits an invalid float-shaped integer immediate and fails assembly. | `rewrite_to_integer_constant()`, both propagation paths and `format_constant(TP_INTEGER)` also reuse contextual text. Preserve typed integers, use exact integer conversion/transport, and retain separate contextual display. Mirrors the original float pipeline defect. |
| **FSCAN-02 — repaired** | High: float-to-decimal conversion at DIGITS 18 changes comparison/round-trip results `1` to `0` for `1.234567890123456`. | `typed_constant_to_type()` protects float-to-float only; cross-type conversions still use 15-digit display. Match the selected decimal provider's existing `decimalFromDouble()` conversion (currently 16 digits), rather than redefining conversion precision. |
| **FSCAN-03 — repaired** | High: typed integer assignment of `1.0000000000000002` signals runtime CONVERSION_ERROR without optimization but succeeds as `1` with optimization. DIGITS 5 propagation of `1.000001` has the same mismatch. | `string_to_type(TP_INTEGER)` reparses display and accepts a `1e-15` epsilon, while VM `FTOI` requires exact float/integer equality. Repair typed conversions against current runtime rules; retain original string/literal validation deliberately. |
| **FSCAN-04 — repaired** | Medium: NaN equality changes `0` to `1`, inequality changes `1` to `0`, and `>=`/`<=` against finite values change false to true. | `compare_nodes()` treats NaN difference as three-way equality because neither `> 0` nor `< 0` holds. Preserve unordered float comparison semantics for each operator, using the current VM behavior. |
| **FSCAN-05 — repaired** | Medium: `1e-310` is accepted/executed without optimization but rejected as BAD_CONVERSION with optimization; `1e-307` is a passing control. | Shared `rx_string_to_double()` rejects every ERANGE (including representable subnormals); RXAS `atof` accepts them. Align literal folding/transport with accepted binary64 values; do not silently change loose-string parsing policy. |
| **FSCAN-06 — repaired** | Medium: a folded infinity/NaN operand used with runtime input emits `inf.0`/`nan.0` and fails RXAS; unoptimized execution succeeds. An RXAS infinity constant disassembles to `inf`/`-inf` that cannot reassemble. | Compiler/disassembler spellings are outside the current RXAS float grammar. Choose an existing representable lowering or obtain language-syntax approval before extending literal syntax; qualify both compiler emission and RXDAS round trip. |
| **FSCAN-07 — repaired** | Medium: dividing runtime `1.0` by typed `-0.0` prints `inf` without optimization and `-inf` with optimization. | Nonoptimized unary negation emits `0.0 - value`, which does not preserve the sign of zero; optimized unary negation now retains the correct negative zero. Repair the existing runtime lowering and retain signed-zero proof. |
| **FSCAN-08 — repaired** | High, both compiler modes: `say (2 as .boolean)` prints `2`, while assignment to a typed boolean correctly prints `1`; `say (1.5 as .int)` and an equivalent string assignment print `1.5` without the required checked-conversion failure. | OP_TYPE_CAST emission/folding uses the consumer's `node->target_type` and bypasses the explicit cast target in child2 when that consumer is string. Preserve the declared cast first, then perform consumer conversion; add direct SAY/string-consumer controls as well as typed destinations. |

Historical scan recommendation: repair FSCAN-01–03 as the typed numeric conversion
and transport family, with separate reviewable changes where causes differ;
address FSCAN-08 as a separate cast-lowering defect, then close FSCAN-04–07 with
focused exceptional-value tests. A required repair
plan must preserve current VM/provider conversion contracts. The 16-digit FTOD
provider policy is recorded as an existing boundary, not changed by this scan.
Normal float displays, decimal rounding, low-range/default-context integer
controls, finite assembler/disassembler round trip, and existing inline
17-digit transport supply useful retained controls. This scan is complete;
the eight repairs are now verified locally by the follow-up receipt below.
Wider optimizer-equivalence and research qualification are not claimed.

## Follow-up implementation — user authorized 11 October 2026

Vision and intended outcomes: repair all FSCAN-01–08, delivering exact typed
numeric folding/propagation and transport plus explicit casts that execute
before consumer conversions. Optimized and unoptimized B/G must agree on
values and required rejection; float comparisons retain current binary64
semantics, including unordered NaN and signed zero. Preserve existing numeric
display, strict/loose string behavior, decimal contexts and the selected
provider's 16-digit FTOD contract. Keep folding enabled. Work remains local in
the synchronized develop checkout; preserve prior/unrelated work and all
historical evidence. No publication or research-checkout changes.

1. **REP-AC01 — verified (FSCAN-01):** Exact integer folding/propagation through
   DIGITS 5/18, INT64 limits, runtime use and RXAS integer transport passes;
   formatting controls remain unchanged.
2. **REP-AC02 — verified (FSCAN-02):** Typed float-to-decimal folding matches actual
   provider conversion at small/high digits; round-trip controls agree without
   changing runtime provider precision or contextual decimal literals.
3. **REP-AC03 — verified (FSCAN-03):** Fractional float-to-int constants and propagated
   values are rejected without tolerance, matching checked runtime conversion;
   exact integer casts and original valid string conversions remain correct.
4. **REP-AC04 — verified (FSCAN-04):** All six typed float comparison operators have
   the VM's unordered-NaN results in folded and runtime cases; finite/infinity
   equality/ordering and string-comparison controls pass.
5. **REP-AC05 — verified (FSCAN-05):** Float literals including representable
   subnormals fold/transport consistently; normal-small/zero controls pass.
   Keep the existing strict string/loose numeric-key parser policy intact.
6. **REP-AC06 — verified (FSCAN-06):** Folded/imported infinity/NaN operands survive
   emission and runtime use. RXDAS non-finite float operands reassemble and
   preserve their bits using the approved RXAS non-finite literals; finite
   round trips remain unchanged. No new opcode or RXBIN format is required.
7. **REP-AC07 — verified (FSCAN-07):** Unary float negation preserves both zero signs
   in optimized/unoptimized cases, including subsequent runtime arithmetic.
8. **REP-AC08 — verified (FSCAN-08):** Explicit scalar cast executes before SAY,
   string assignment and other consumer promotion; invalid checked conversions
   cannot disappear. Typed destination, chained cast and object-cast controls
   retain their contracts and register ownership.
9. **REP-AC09 — verified:** Permanent focused regressions cover every finding in B/G
   x compiler modes x assembler modes x both supported concrete VMs, linked
   transport and relevant import paths, including success and rejection.
   Measure new/expanded aggregates in isolated Debug and maintained ASan before
   registration; declare generous timeout/scheduling from evidence.
10. **REP-AC10 — verified:** Core product build is warning-free in normal Debug;
    relevant numeric/comparison/cast/constant/binary/metadata correctness tests
    and focused maintained ASan pass. Retain exact qualified hashes, commands,
    logs/results and current acceptance status. No redundant broad/overnight
    or hosted qualification. Actual sanitizer findings follow SAN governance.

1. **REP-STEP01 — complete (REP-AC01–08):** Reconcile current AGENTS, worklist,
   synchronized checkout and retained minimal scan evidence. Remote develop
   remains at the base revision; original local repair is preserved.
2. **REP-STEP02 — complete (REP-AC01–03, REP-AC08):** Implement typed scalar
   conversion and exact integer transport; retain declared cast before consumer
   conversion in folding/emission. Depend on STEP01. Preserve provider policy;
   stop only if a language/architecture decision actually becomes necessary.
3. **REP-STEP03 — complete (REP-AC04, REP-AC05, REP-AC07):** Repair unordered float
   folding, literal-only binary64 parsing and signed-zero negation. Keep shared
   strict text parsing and runtime formatting unchanged. Depend on STEP01.
4. **REP-STEP04 — complete (REP-AC06):** Implement approved RXAS non-finite
   float literals and use them in compiler/RXDAS operands, preserving original
   opcode overloads and register/global/argument mappings. Remove the rejected
   expansion prototype. Depend on STEP01; no opcode/RXBIN-format change.
5. **REP-STEP05 — complete (REP-AC01–09):** Add minimal positive/negative permanent
   probes and exact bit/transport controls. Run focused checks during each repair;
   measure aggregate Debug/ASan, then register with explicit scheduling/timeout.
6. **REP-STEP06 — complete (REP-AC10):** Build core and prerequisites, run relevant
   existing normal correctness suite and focused ASan; diagnose any regressions,
   update only deliberately changed expectations, reuse unchanged passing proof.
7. **REP-STEP07 — complete (REP-AC01–10):** Update owning docs, exact validation
   receipt and finding statuses; inspect final diff and deliver locally for
   review with all remaining criteria visibly open until verified. No push.

Implementation evidence will be retained under
`docs/qa/float-constant-folding-2026-10-11/related-repair/`. All REP criteria are verified by the final follow-up receipt; the scan
is retained separately as historical pre-repair evidence.

### Historical REP-STEP04 syntax decision gate — superseded

The existing-instruction prototype passes ordinary non-finite arithmetic,
but generic register substitution does not preserve all original RXAS
instruction semantics. In the retained `expansion-global-proof.json`, a
callee changes a global integer payload to 42 and returns an immediate float
to that same global: original RET_FLOAT leaves 42 intact, while substituted
RET_REG overwrites it with 0. LOAD_REG_REG also aliases storage rather than
performing a scalar float load, and SAY_REG expects a string. Therefore that
prototype is not an acceptable general RXDAS transport repair.

Concrete proposal: add RXAS-only `0f` followed by exactly 16 hexadecimal IEEE
binary64 bits, e.g. `0f7ff0000000000000` (+infinity) and
`0f7ff8000000000000` (quiet NaN). Emit it only for non-finite float operands;
keep numeric display, finite emission, strict text parsing, all existing
opcodes and RXBIN layout unchanged. Sign/NaN payload and original opcode
overload remain exact. Reviewable lexer/decoder patch:
`docs/qa/float-constant-folding-2026-10-11/related-repair/rxas-bit-literal-proposal.patch`.
This proposal was pending approval at this checkpoint. Adrian subsequently
selected the readable inf/nan family below; the 0f proposal remains historical
and unapplied. REP-AC06 is now verified by the exact transport/ABI proof.

### Historical interruption checkpoint — explanation requested

Adrian interrupted the run to request a fuller explanation of the syntax
question. No approval has been given. No build/test processes remain running.
All edits remain local and uncommitted; assembler syntax is still unchanged.

- REP-STEP02/03 have provisional implementations. The new values/casts B/G x
  compiler/assembler x both-VM matrix passes, including the required failures;
  final existing correctness and sanitizer qualification remains outstanding.
- Normal Debug core tools and library prerequisites built successfully, with
  no warnings: `/tmp/crexx-related-normal-build.IjaKX9` (exit 0).
- The maintained ASan tree was configured only; these follow-up repairs have
  **not** yet run their ASan build/tests. Earlier original-repair ASan evidence
  does not qualify the changed follow-up implementation.
- REP-STEP04's generic disassembler expansion is a rejected local prototype
  because the global/return-payload proof changes output 42 to 0. It must be
  replaced or removed before final delivery. The proposed bit-literal patch is
  saved for review and remains unapplied.
- New permanent aggregate scaffold and fixtures are present. VALUES_ONLY
  measures just values/casts; the full transport contract is intentionally
  incomplete and is not registered as a passing CTest. All REP criteria stay
  open until full required evidence and the transport decision are complete.

### Accepted syntax decision — 11 October 2026

Adrian proposed `[+|-]nf`, `nan` and authorized implementation: "If so lets do
that", then corrected the spelling: "inf I meant". Use RXAS-only `inf`, `+inf`,
`-inf` for infinities and `nan` for the canonical
quiet NaN, case insensitive. For exact RXDAS transport of a different NaN sign,
payload or signaling bit, use `nan(<16 IEEE hexadecimal bits>)`; ordinary NaN
remains readable as `nan`. This is the exact-data form of the accepted NaN
literal family, preserving REP-AC06's bit requirement rather than replacing it
with canonicalization. No `0f` spelling will be introduced. Existing finite
literal transport, numeric display, strict string parsing, opcode identities
and RXBIN layout remain unchanged. Inline float field arity/identities and
finite spelling remain unchanged; the non-finite family also preserves NaN bits
in that field. Bare inf/nan RXAS operands are now reserved numeric literals,
including where a branch target or constant alias was previously an identifier.
The user-facing RXAS guide and VM specification document this consequence.


### Final local follow-up acceptance — 11 October 2026

All FSCAN-01–08 and REP-AC01–10 are repaired and verified locally. Evidence and
exact input/tool hashes are in
[related-repair receipt](../../docs/qa/float-constant-folding-2026-10-11/related-repair/README.md).
The original AC-01–04 proof and pre-repair scan remain historical and intact.

- Typed scalar conversion/propagation and emission preserve integer and float
  payloads; decimal conversions use the provider's runtime services. Explicit
  casts execute before consumer conversion. No tolerance or broad optimization
  disablement was added. Exact float literals use a local binary64 parser;
  strict string parsing and effective-string comparisons retain their policy.
- NaN comparisons are unordered, and unary negation preserves signed zero.
  Approved inf/nan operands retain original RXAS opcode overloads. RXDAS and
  inline metadata retain noncanonical NaN sign/payload/signaling bits. Decimal
  exceptional constants materialize through existing provider text conversion.
- Permanent numeric_constant_contract: 200 VM executions / 476 tool commands
  covering B/G, compiler/assembler modes, concrete VMs, success/rejection,
  linked images, isolated imports, exact inline NaN payloads and RXDAS
  binary64/LOAD_FLOAT/RET_FLOAT payload controls. Original float aggregate
  remains enabled and passes, proving retained folding and runtime reference
  arithmetic. Final aggregate measurements: Debug 1.34 s; Apple ASan 25 s.
  New CTest is RUN_SERIAL with a 300-second hang-protection limit.
- Final normal correctness: 58/58 including object-cast success/failure controls.
  Registered maintained Apple ASan: 3/3 (original float aggregate, expanded
  numeric aggregate, independent metadata contract), no sanitizer diagnostics.
  Successful normal core/prerequisite builds have zero warnings. Earlier ASan
  recompilation emitted existing Apple SDK sprintf deprecation warnings in
  unchanged functions; retained logs show them. Final negation recompilation emits the same two existing header warnings;
  normal builds remain warning-free. Apple LSan is unavailable; Linux/cross-platform
  qualification is not claimed, and no overnight/hosted gate was dispatched.
- RS-F01 is resolved by this local compiler repair, including the original
  false optimized binary-float equality. The compiler blocker under RS-AC04 is
  addressed. Research adoption/requalification and the wider Level L worklist
  remain the research owner's work. No research source/evidence/status changed.
- Delivery: existing develop at bb0d211a8d17f0b66fdd6fcb99ad6ffd849a0283,
  confirmed current against remote develop. All repair changes are local,
  uncommitted and unpublished for review. No remaining reproduced defect in
  the eight-finding scope; other providers/host platforms are not qualified by
  this local run.


### Final negation boundary check — verified after reopening REP-AC07

A binary-storage probe compares `-(argv_zero / argv_zero)` with
`-(0.0 / 0.0)`: noopt returns equal bits (1), optimized returns unequal (0).
The runtime lowering multiplies by -1, preserving/quieting its NaN operand;
folding used C unary minus, which toggles the NaN sign without arithmetic.
This is the same scalar-negation boundary as FSCAN-07. Align folding with the
emitted multiplication, using a volatile multiplier to prevent the host
Release C compiler from substituting unary minus. Add permanent exact-bit
runtime/folded proof to the numeric aggregate, then rerun the affected normal
and maintained ASan checks. REP-AC07/09/10 remain open until that proof passes;
other accepted outcomes and retained valid evidence are preserved.


Final reconciliation complete: the restored assembler source remains in place.
All REP criteria are verified, including the additional NaN-negation bit control.
Final normal 58/58 passes (numeric aggregate 1.54 s), maintained ASan 3/3 passes
in 35.03 s, with no sanitizer findings. Host Clang -O3 IR retains volatile read
and multiplication. The full original matrix and eight-finding matrix pass with
unchanged command/VM counts. Final exact source/tool hashes and all replay logs
are refreshed in related-repair/final/manifest.json; historical evidence remains
separate. Source changes after qualification are documentation/evidence only.
All work is local, uncommitted and unpublished; the research checkout was not
modified by this task.


## Full CTest and publication — user authorized 11 October 2026

Vision and intended outcomes: extend the completed local repair through the
user's requested full normal Debug CTest gate, then commit and publish the
complete code/tests/docs/evidence to develop. Preserve all earlier acceptance
criteria, unrelated work and historical evidence. The research agent will pull
and merge the published changes and qualify them after publication; this task
must not modify that checkout or claim research completion. The earlier local-
only delivery constraint is superseded by this explicit publication request.

1. **PUB-AC01 — verified:** All required artifacts for the full configured normal
   Debug CTest catalogue are prepared with a warning-free successful build.
   Retain build command/log and exact code/test/build-input hashes.
2. **PUB-AC02 — verified:** Full CTest passes with repository scheduling and
   `--parallel 30`, without new exclusions or weakened assertions. Diagnose and
   repair failures; reuse unchanged passing results and replay timeout-only
   failures once in isolation under their declared scheduling before broad
   repeats. Retain catalogue, full results/logs and any focused repair proof.
3. **PUB-AC03 — verified:** Review and commit only this repair's code, permanent
   regressions, owning documentation, plan and qualification receipts. Include
   the expected successful normal automatic publication status in the first
   substantive push; do not add a CI-status-only follow-up commit.
4. **PUB-AC04 — fulfilled by publication of these commits:** Publish the compatible commits to synchronized develop
   after PUB-AC01–03. Verify remote commit IDs and report the exact reference for
   the research agent. No release tag/master promotion or research edits.
   Hosted checks are automatic follow-up checks, not an additional manual or
   overnight gate; report actual versus expected status accurately.

1. **PUB-STEP01 — complete (PUB-AC01–04):** Read current AGENTS, authoritative
   plan and local state; remote develop remains bb0d211a8d17. Confirm explicit
   full-CTest/publication authorization and research-agent ownership.
2. **PUB-STEP02 — complete (PUB-AC01):** Build qa-prep-all in the normal Debug
   tree so all configured CTest tiers have their declared artifacts. Record
   catalogue, source/build inputs and complete diagnostics. Depends on STEP01.
3. **PUB-STEP03 — complete (PUB-AC02):** Run the full normal Debug CTest suite;
   triage any failures and run the smallest decisive repair checks. First-party
   sanitizer findings still require SAN governance. Depends on STEP02.
4. **PUB-STEP04 — complete (PUB-AC03):** Final source/diff review, retain full-gate
   receipt and delivery status; make separate reviewable commits for distinct
   repair mechanisms where practical, publishing compatible commits together.
   Depends on STEP03 passing; do not repeat valid unchanged broad proof.
5. **PUB-STEP05 — publication/ref verification (PUB-AC04):** Recheck remote develop, publish commits,
   verify remote head and provide research handoff reference. Depends on STEP04.

The full gate is explicitly requested by Adrian; this is not inferred release,
sanitizer or performance qualification. Evidence will be retained under
`docs/qa/float-constant-folding-2026-10-11/publication/`.


Publication preparation: qa-prep-all completed successfully with zero warnings.
The assembler special-decimal normalization edit had again reverted during
preparation; its exact validated content was restored and staged, and the
artifact preparation was repeated against that stable source. Adrian confirmed
that no other edits are intended. The full normal Debug CTest run is active
with 3,329 configured tests, --parallel 30, no added exclusions, and JUnit/full
log retention. Code/test/build hashes remain aligned; the research checkout was
not modified or interrupted.


Full sweep completed: all 3,329 tests executed in 2,271.82 seconds; 3,324
passed and five snapshot/diagnostic assertions failed. No timeout or sanitizer
finding occurred. Four snapshot changes are intentional: 11 synthetic
register-index integer annotations across three AST dumps, and seven strict
numeric operands materialized before string comparison. Their runtime controls
pass. Update only those verified golden differences. The fifth failure is an
unintended raw integer-literal diagnostic change; restore OVERFLOW_UNDERFLOW
on strict integer parse ERANGE for lexer-validated integer tokens, preserving
BAD_CONVERSION for invalid text/fraction casts and the INT64_MIN exception.

The correction is confined to an already-failing error path; every successful
parse/evaluation/emission path remains unchanged. Retain the full frozen sweep,
then replay all five failures and focused numeric/int64/comparison/AST controls
in normal Debug and the maintained ASan tree. Reuse the other 3,324 valid
outcomes rather than spend another 38-minute sweep on unchanged behavior. The
full catalogue must have green final outcomes before committing or publishing.


Full catalogue green: 3,324 retained valid passes plus all five failed tests
passing on final replay, covering all 3,329 tests without skips/exclusions.
The literal diagnostic now preserves OVERFLOW_UNDERFLOW at each invalid token;
the golden also removes an obsolete duplicate assignment error eliminated by
same-type payload preservation. Focused normal controls passed except that
obsolete duplicate expectation, then the five-test final replay passed 5/5.
Expanded maintained Apple ASan passed 9/9 in 39.52 s with no findings. The only
new C-code change after the full sweep selects an error message after an
already-failed parse; successful parse/evaluation/emission remains unchanged.
Evidence reuse and final outcome mapping are explicit in the publication
receipt, rather than misrepresenting the initial sweep as a single green run.


Publication delivery record: compatible commits separate typed numeric/RXAS
transport, unordered NaN comparison, signed-zero/NaN negation and explicit-cast
consumer ordering. The complete docs, permanent regressions and qualification
receipts ship together in the first substantive develop push. All 3,329 CTest
outcomes are green by the retained full sweep plus repaired replay; expanded
Apple ASan 9/9 is green. Normal automatic hosted product/smoke, optimizer-parity
and CodeQL checks are expected to succeed, not claimed observed before push.
PUB-AC04 is fulfilled on successful publication and remote-ref verification of
these commits, which is reported live in the conversation. No later CI-status-
only commit is planned. Research adoption and qualification remain with the
research agent after publication; wider research completion is not claimed.

## Open numeric semantics follow-ups

Source-reconfirmed findings below remain distinct from their historical runtime
reproductions; source review is not fresh execution or qualification. Policy
questions do not imply defects or authorize numeric redesign. Adrian owns those
decisions; the named compiler, runtime/provider and reference owners own any
separately scoped repair. The completed typed-constant, float transport and
typed-NaN repairs above remain separate.

| ID | Status and durable finding | Owner / next action |
| --- | --- | --- |
| REV-F01 | Open defect, source-reconfirmed: loose numeric-string comparison can treat `"nan" = "0"` as true because neither greater nor less is taken. | Runtime `rxvm_loose_compare_text` in [rxvmintp.c](../../interpreter/rxvmintp.c) and folded comparison in [rxcp_opt.c](../rxcp_opt.c): define accepted NaN-text semantics for all six operators, then align folding/runtime with focused regressions. |
| REV-F02 | Open optimizer-equivalence defect, source-reconfirmed: `"Infinity" as .decimal` reaches a finite-only folded validator although the decimal provider accepts special values; historical execution succeeded without optimisation and failed during optimised compilation. | [rxcp_opt.c](../rxcp_opt.c), `stringtodecimal` in [rxvmvars.h](../../interpreter/rxvmvars.h) and [mc_decimal.c](../../interpreter/rxvmplugin/rxvmplugins/mc_decimal/mc_decimal.c): align conversion admission and failure policy. |
| REV-F03 | Policy decision: finite float-to-decimal conversion still uses `%.16g`, which can lose a binary64 value such as `1.0000000000000002`. Compiler/provider parity is a separate, repaired obligation. | [mc_decimal.c](../../interpreter/rxvmplugin/rxvmplugins/mc_decimal/mc_decimal.c) and [db_decimal.c](../../interpreter/rxvmplugin/rxvmplugins/db_decimal/db_decimal.c): choose exact-value, round-trip or contextual-text conversion semantics before changing precision. |
| REV-F04 | Policy/reference mismatch, source-reconfirmed: typed integer/integer `/` selects its result type from the file numeric standard, independently of procedure `NUMERIC STANDARD`. | `OP_DIV` in [rxcp_val_type.c](../rxcp_val_type.c) and [numeric reference](../../docs/books/crexx_language_reference/numeric.md): document the static rule or explicitly approve different type-selection/inheritance semantics. |
| REV-F05 | Policy/reference decision, source-reconfirmed: explicit `floats_binary` rewrites decimal literals and `.decimal`; `floats_decimal` rewrites `.float`. Omission of a force option has different semantics. | Representation walkers in [rxcp_val_type.c](../rxcp_val_type.c), selected by [rxcp_val_orch.c](../rxcp_val_orch.c): document force-option precedence; decide separately whether explicit type intent should override it. |
| REV-F06 | Open formatting/reference mismatch, source-reconfirmed: mcdecimal special values return lowercase `inf`/`nan` before `NUMERIC CASE` handling; finite exponent formatting follows a different path. | `decimalToString` in [mc_decimal.c](../../interpreter/rxvmplugin/rxvmplugins/mc_decimal/mc_decimal.c): decide whether CASE covers special values and align implementation/reference. |
| REV-F07 | Reference defect, source-reconfirmed: the advertised combined `NUMERIC` settings syntax is absent from the grammar; separate clauses are supported. The older B/G expression `(.float(0.1) + .float(0.2) = .float(0.3))` reportedly raised `CLASS_NOT_FOUND` while separate assignment worked; that symptom is unverified against current code. | [rxcpbgmr.y](../rxcpbgmr.y) and [numeric reference](../../docs/books/crexx_language_reference/numeric.md): correct combined-syntax examples; obtain a minimal current reproducer before classifying or repairing the constructor report. |
| REV-F08 | Comparison-model decision: typed numbers, loose numeric strings above binary64 integer precision, decimal/string conversion and `RexxValue.compareEqual` versus `equals` follow different value, precision and textual paths. | Loose comparison in [rxvmintp.c](../../interpreter/rxvmintp.c), [RexxValue.crexx](../../lib/rxfnsc/RexxValue.crexx) and numeric reference: specify type-specific equality/FUZZ contracts before proposing a common comparison algorithm. |
| REV-F09 | Reference scope: distinguish type-specific DIGITS/FUZZ/FORM/CASE/STANDARD effects, default versus force options, and value conversion versus display versus exact transport. | [numeric reference](../../docs/books/crexx_language_reference/numeric.md) and [data types](../../docs/books/crexx_language_reference/data_types.md): reconcile specific claims with the owners above; do not describe all typed calculations/comparisons as decimal-context operations. |

Strict `==` compares the resulting strings exactly, not numeric payloads. Typed
B/G formatting can give distinct float payloads the same text; Classic C can
retain literal spelling. Binary64 value equality, contextual display and exact
transport are different contracts: decimal formatting cannot recover precision
already lost at a binary conversion boundary.
