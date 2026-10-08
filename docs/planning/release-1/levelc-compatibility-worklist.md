# Level C compatibility worklist

Status: active component worklist under [the Release 1 plan](../../release-1-plan.md).
Started 2026-10-03 on `develop`. The Release 1 plan owns the Beta 4 completion
contract (`R1-AC-01/02`) and the roadmap owns portfolio order. This worklist
records coverage, incremental evidence and Adrian-approved scope revisions;
it does not change the 2026-11-30 target.

**Current status, 2026-10-08.** The independent BIF checkpoint is locally
qualified on `d7b58e17d8d49979445543676c2bcd56fb1f351b` code/test inputs:
core product builds, full normal Debug 3296/3296, 3113 unique normal Release
correctness checks using retained unchanged results plus focused completion,
19 installed BIF cases and the installed host callback fixture. Focused
maintained macOS ASan receipts cover the changed native/ownership paths.
The exact commands, input/reuse proof, process/memory receipts and unrun gates
are in LC-STEP-90F below. All 70 names have an audit row; 62 have direct entries.
The eight stream BIFs, deferred Unicode/I/O compatibility and wider source/host
obligations remain open. ANSI numeric rules and the same infrastructure scope
apply to B/G as Adrian directed. This completes the admitted independent phase,
not the full 70-name contract, full Level C or Release 1 qualification.

`LC-I-01`–`LC-I-18` (SAY, DROP, assignment,
NOP, OPTIONS, IF, SELECT, DO, LEAVE, ITERATE, ARG, PROCEDURE, CALL, RETURN,
EXIT, PULL, PUSH and QUEUE) have closed
whole-instruction reviews. ARG uses the shared PARSE template executor and
one activation argument frame; the complete admitted Classic invocation and
template matrix has passing evidence in the closure receipt below. Cross-dialect
Classic/non-Classic invocation is outside this programme's scope by Adrian's
2026-10-05 clarification, except the fixed-signature Level B/G CALL entry
Adrian subsequently allowed on 2026-10-05. Future external CALL/INTERPRET and broader C host
interfaces retain their own open owners; they are not ARG closure gates. The
approved `LC-STEP-63T`
one-frame label and SIGNAL design has a
canonical AST/emitter foundation and one-body invocation route, including
first-instruction PROCEDURE eligibility, main fallthrough and a second
PROCEDURE. The whole PROCEDURE review now includes direct and indirect EXPOSE,
exact compound aliases and private-pool lifecycle. SIGNAL is closed under
`LC-AC-76`;
CALL closed under the accepted static provider boundary, and RETURN and EXIT
closed on the shared activation and implicit-main path.
PARSE closed on the shared template engine and its seven agreed sources;
`LC-I-20` ADDRESS and `LC-I-21` implicit command closed 2026-10-06 with the
approved embedded-NUL host-command exception recorded as
`LC-HOST-ADDRESS-NUL`. `LC-I-22` NUMERIC closed 2026-10-06;
`LC-I-23` SIGNAL and `LC-I-24` TRACE closed 2026-10-06. Adrian parked
`LC-I-25` INTERPRET on 2026-10-07 as **not implemented**; no product
implementation or architecture choice is approved by its research note below.
Its instruction and Release 1 acceptance remain open pending a separate
disposition. The full
compatibility, host, condition, BIF, AST and cross-consumer criteria remain
open even where a supporting slice or helper passes. The earlier grouped normal
Debug and Release Level C checkpoints passed 741/741 each for TRACE. The
earlier matched
focused macOS ASan passed 27/27 for ADDRESS plus implicit command. The earlier normal Debug Level C
checkpoint for the whole RETURN review passed 662/662 with
process memory monitoring. Focused macOS ASan passed 42/42 and final-input
Level B/G/RexxScript isolation passed 11/11.
`LC-I-15`–`LC-I-18` closed at the grouped 2026-10-06 checkpoint: normal Debug
Level C 694/694, focused macOS ASan 34/34 and Level B/G/RexxScript isolation
11/11 on commit `09d48617c` code/test inputs. Guarded normal runs left zero
child processes; the sanitizer runner exited with no remaining test processes.
The queue instructions use the shipped execution-local selected queue,
including its named-queue repository. A Level C or C host selector for that
repository remains open under `LC-AC-06`. This checkpoint does not qualify
full Level C or Beta 4.

**2026-10-07 closeout order.** Reconcile and close the remaining cross-cutting
gaps using the decision register below. Adrian will decide which still-open
items, if any, to leave open; no item becomes a Release 1 "won't implement"
exception without his individual approval and a documented user-visible
boundary. After this closeout, start a separate session for a fresh whole-
programme consistency review. Reuse the 741/741 normal Debug and Release
TRACE-input checkpoints while code/test/build inputs are unchanged; this
documentation reconciliation does not trigger another product suite.

### LC-GAP-02 BIF completion programme (2026-10-07)

**Vision and intended outcome.** Deliver a coherent running baseline for all
70 catalogued Classic BIF names through the existing shared direct-BIF path.
Presence in the compiler table is not closure: each family must cover legal
arguments and omissions, options, values, state, errors and authored source
locations against reference behavior, with explicitly labelled approved Unicode
departures. Preserve valid Unicode scalars/codepoint indexing, the fixed Latin-1
ordinal bridge, RexxScript's binary-capable RexxValue and sandbox, the static
signed CALL boundary, and current linker/VM behavior. Implement only narrowly
necessary BIF host support through approved interfaces. INTERPRET remains parked;
LC-GAP-01 and LC-GAP-03–10, compiler split/fast-pipeline proposals, wider host
closure and full Level C/Release 1 qualification remain pending.

**Adrian's infrastructure scope clarification, 2026-10-07.** Work must not
change Unicode or I/O behavior relative to the current infrastructure. Such
changes are deferred until an architectural assessment determines the required
compatibility. Unicode characters causing signals or other logic errors in Level C are
acceptable and currently undefined by Adrian’s further clarification; do not introduce a codec, host ABI, VM/linker change or compatibility
rule to suppress them. The stream provider proposal is unapproved and pending.
The same deferred Unicode/I/O boundary applies to B/G. Approved ANSI numeric
BIF rules apply across B/C/G through their existing decimal paths.
Continue independent BIFs and audit the existing admitted paths; keep deferred
stream/Unicode and wider host proof visibly open.

**Acceptance criteria.** Status records the complete criterion, with admitted-path
proof separated from deferred/unverified obligations.

- [ ] **LC-BIF-01 — inventory and contract audit:** recount the 70 names against
  current compiler/runtime/test code and reconcile every inventory row with
  family evidence; no entry/example-only closure. Inspect legal argument counts,
  omissions, types/options, reference values, failures and source identity.
- [ ] **LC-BIF-02 — streams and positioning:** CHARIN/CHAROUT/CHARS,
  LINEIN/LINEOUT/LINES, QUALIFY/STREAM implement one stream service. Verify
  defaults/named streams, EOF, independent positions, line endings, encoding,
  NUL, availability/state/commands, invalid/nonpositionable resources, close,
  failure conditions, resource cleanup and context isolation. New language,
  host ABI or architecture decisions require Adrian's approval first.
- [x] **LC-BIF-03 — selected queue:** QUEUED observes the same execution-local
  selected repository as PULL/PUSH/QUEUE, without consuming input; verify empty,
  FIFO/LIFO, named selection, omissions/errors and independent executions.
- [ ] **LC-BIF-04 — source, messages and condition:** SOURCELINE uses retained
  original source (not a runtime file reread), ERRORTEXT shares the standard
  catalog, and CONDITION fields follow admitted live producers and frame
  policy. Verify options/ranges/omissions, source lines and errors, nested and
  linked contexts, NUL/Unicode and producer/state-field behavior. Real host HALT
  remains a separately owned host gap, not a falsely passed producer.
- [ ] **LC-BIF-05 — pure/numeric/text/ordinal audit:** audit every existing
  entry, caller numeric and configuration context, argument normalization,
  all 256 ordinals and unmappable scalars; verify actual behavior/errors against
  IBM/Regina where useful. Preserve Level B/G/L and RexxScript isolation.
- [x] **LC-BIF-06 — complete product checkpoint:** freeze final code/test/build
  inputs; core build, full normal Debug CTest once across B/C/G/L, RexxScript,
  RXAS/linker/VM/native interfaces; relevant normal Release correctness suites
  across those levels once with opt/no-opt and linked execution; installed-product
  and host BIF smoke; focused maintained sanitizer for changed native/ownership
  paths. Record exact revision/input hashes, commands/results/logs and unrun
  platform gates. Bound parallelism by observed total/child memory, avoid broad
  job overlap, use temporary verbose logs, and verify child exit after long runs.
  Reuse valid unchanged evidence; no routine overnight dispatch.
- [x] **LC-BIF-07 — delivery and honest closure:** commit each coherent family
  with focused evidence and synchronized docs; final report identifies each
  commit, grouped QA, remaining defects/decisions, exact HEAD and unpublished
  status. Any blocked behavior stays open; no full Level C/Release 1 claim.

**Implementation steps.**

1. **LC-STEP-90A (LC-BIF-01/07; inventory/admitted-path audit complete):** read governing plans/guides,
   fetch/reconcile develop, recount entries, inspect retained evidence, and
   record this plan before implementation. Starting verified clean revision:
   `52fcfb21bd1acd18e8010af83ab65eaae8b7a863`; fetched origin/develop: 0 behind,
   208 ahead. Previous 741/741 Debug/Release Level C receipt qualifies TRACE
   inputs only. No existing build/test child jobs were found.
2. **LC-STEP-90B (LC-BIF-02; pending by Adrian’s 2026-10-07 direction; depends on 90A):** complete the
   stream family around one service. Existing FOPEN/FREADCDPT/FWRITE and Level B
   file cache do not expose seek/tell, independent read/write positions or a
   full Classic status/availability contract. Proposed decision: a narrow RXPA
   provider with session-owned stream handles, typed checked methods for read,
   write, positioning, availability, state and close; Level B BIFs own argument
   and Classic error handling. Reuse existing RXPA factory/method/ownership
   services; no VM opcode, linker or loader changes. Encoding remains explicit
   host text encoding, Unicode codepoint positions and NUL-safe length spans;
   raw binary Level C values remain out of scope. Do not implement this new
   host contract until approved. Adrian selected “Keep streams pending; finish
   independent BIFs” on 2026-10-07. Finish later independent steps; this is
   deferral, not a stream exception or BIF baseline closure.
3. **LC-STEP-90C (LC-BIF-03/07; implementation and focused proof complete; depends on 90A):** add QUEUED using the
   existing selected queue/configuration service and shared direct lowering;
   cover arguments, effects, selection and isolation; commit the family.
4. **LC-STEP-90D (LC-BIF-04/07; admitted source/messages/condition proof complete; depends on 90A):** retain source lines
   through compiler-generated activation data, implement SOURCELINE and catalog
   ERRORTEXT, reconcile diagnostic code range against references, and audit
   CONDITION producers/fields. Commit coherent source/message/state increments;
   seek approval for any genuinely new rule or architecture.
5. **LC-STEP-90E (LC-BIF-01/05/07; admitted-path audit complete; depends on 90A):** audit all remaining
   families, reuse valid character/ordinal receipts, add regressions and repair
   reproduced defects in family-sized commits. Direct-entry use tracking is now table-sized rather than a 64-bit mask; no
   second dispatcher was added. Deferred Unicode/resource proof remains open.
6. **LC-STEP-90F (LC-BIF-06/07; local independent checkpoint complete; depends on final independent code and
   90B deferral already recorded):** perform the single grouped product checkpoint on frozen
   final inputs, document each unrun platform gate, reconcile inventory/criteria,
   and report exact HEAD/ahead/behind and any precise blocked behavior.

**LC-STEP-90C receipt, 2026-10-07.** QUEUED now uses the existing
execution-local selected repository through RexxClassicConfig. No host selector,
Unicode/I/O transport change or second dispatcher was added. Import-use tracking
is now a table-sized array, removing the 64-name bound without shifting masks.
Direct entry count is 60/70; the ten absent names are the eight stream names,
ERRORTEXT and SOURCELINE. Focused core/runtime build passed with `--parallel 4`
(`/tmp/crexx-bif-queue-build.kH6o9W`). Final queue harness build and CTest
`-R '^(testRexxClassicBifQueued|levelc_bif_queued)' --parallel 4
--output-on-failure` passed 6/6 in 2.30s
(`/tmp/crexx-bif-queue-final.rauuyG`), covering empty/repeated count, FIFO/LIFO,
NUL on the admitted path, nested procedures, named queue selection via the
existing Level B API, excess/omitted arguments, source-anchored SYNTAX and
linked opt/no-opt. Earlier unchanged PULL/PUSH/QUEUE linked checks passed 6/6.
Harness setup failures were corrected to create a queue and use the shipped
QUERY/SET contract; there was no product regression. Build/test processes exited;
no remaining compiler/assembler/linker/VM/CTest children were observed. Whole-
product grouped and sanitizer checks remain open until STEP-90F. Wider host queue
selection remains LC-GAP-03, and Unicode compatibility remains deferred.

**LC-STEP-90D SOURCELINE receipt, 2026-10-07.** Compiler-generated source
lines use the existing source buffer, string escaping and Classic configuration;
there is no runtime file reread or new host/VM interface. Only a unit using this
BIF retains its source. Local routines share that unit; separately compiled
Classic providers retain their own unit. Source-mapped generated inputs report
source unavailable (count zero), rather than inventing an original inventory;
full mapped source identity and physical source NUL remain LC-GAP-06. The existing
Unicode infrastructure is unchanged and its undefined cases are not claimed
qualified. Argument bounds are checked before narrowing the source index.

Focused Debug build `cmake --build cmake-build-debug --target rxc rxfnsc
testRexxClassicBifSourceline --parallel 4` passed
(`/tmp/crexx-bif-source-build.g0s0DF`). After adding CRLF/no-terminal-EOL and
provider isolation assertions, `ctest --test-dir cmake-build-debug -R
'^(testRexxClassicBifSourceline|levelc_bif_sourceline|levelc_call_external_)'
--parallel 4 --output-on-failure` passed 10/10 in 28.23s
(`/tmp/crexx-bif-source-final.log`). It exercises count/text, preserved comments
and blank lines, leading-zero index, huge out-of-range index, invalid/zero index,
missing/excess operands, source-anchored SYNTAX, optimized/no-opt and linked
caller/provider isolation including binary-only provider imports. Processes
exited. Direct count is 61/70; nine names remain absent. Sanitizer and grouped
whole-product proof remain STEP-90F obligations. ERRORTEXT and the CONDITION
producer audit are still pending within STEP-90D.

**LC-STEP-90E numeric audit, implementation plan.** The retained ASCII
reproducer `SUBSTR('abc','1.0')` exits 28 with source-anchored 40.12 on
`f69ab852f`, while Regina returns `abc` (whole decimal and exponent spellings).
Failure log: `/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-bif-whole-baseline.idl3orbw/baseline.log`;
reference: `/tmp/crexx-bif-whole-repro.D0LIH6.reference.log`. Repair the shared
WHOLE normalizer by reusing the existing decimal-expansion helper, removing the
integer-spelling-only branch. Verify exact decimal/exponent values, fractional
rejection, signs/zero, caller digits independence, source errors and direct/linked
opt/no-opt. Keep WHOLENUM caller rounding and the existing configured numeric
alphabet/Unicode transport unchanged. This is a reference-backed argument
validation repair, not a new character compatibility rule. Wider integer/resource
limits stay visibly open until characterized.

**LC-STEP-90E WHOLE receipt, 2026-10-07.** Exact decimal/exponent whole
arguments now use the existing expansion algorithm in the shared datatype
module. The obsolete integer-spelling validator is removed; WHOLENUM reuses
that same expansion after its existing caller-context rounding. No numeric
alphabet, codec or I/O path changed. Debug focused build of `rxfnsc`,
`testRexxClassicBifDatatype`, `testRexxClassicBifD2x` and
`testRexxClassicBifSubstr`, `--parallel 4`, passed
(`/tmp/crexx-bif-whole-build.log`). CTest `-R
'^(testRexxClassicBif(Datatype|D2x|Substr)|levelc_bif_whole)' --parallel 4
--output-on-failure` passed 10/10 in 2.91s
(`/tmp/crexx-bif-whole-tests.log`). The compiled fixture exactly matches Regina
(`/tmp/crexx-bif-whole-reference.log`): decimal/exponent/leading-zero/sign
values, caller digits 2/fuzz 1, multiple positional/count BIFs and SOURCELINE,
ARG, fractional error 40.12 and its source line, direct and linked opt/no-opt.
Existing configured-digit tests now expect a mathematically whole decimal
through the unchanged normalization route. That assertion is not wider Unicode
qualification. Broad/product and sanitizer checks remain STEP-90F obligations.

**LC-STEP-90D ERRORTEXT receipt, 2026-10-07.** ERRORTEXT uses the existing
configure-generated diagnostic catalog with unexpanded Classic place-markers;
there is no second hand-maintained message table. N uses the shipped English
fallback. The extracted helper's 40.16 range call conflicts with its own catalog
and Regina's range-specific 40.17; the implementation uses 40.17. Numeric
validation uses the existing configured number normalizer and decimal service;
decimal subcode trailing zeros are preserved as catalog-key digits. Undefined
codes return empty, including zero. The catalog's 40.34 wording differs from
Regina but follows the project's standard catalog.

Debug build `cmake --build cmake-build-debug --target rxc rxfnsc
testRexxClassicBifErrortext --parallel 4` passed
(`/tmp/crexx-bif-message-repair-build.log`). CTest `-R
'^(testRexxClassicBifErrortext|levelc_bif_errortext)' --parallel 4
--output-on-failure` passed 6/6 in 2.00s
(`/tmp/crexx-bif-message-final.log`), covering required/count/omitted/options,
major/minor/undefined text, leading/trailing zero keys, decimal range precision,
huge exponent, source-anchored 40.17 and direct/linked opt/no-opt. Reference
receipts are `/tmp/crexx-bif-message-reference.4pBpq4.log` and
`/tmp/crexx-errortext-probe.K4P1Wt.log` (Regina 3.9.7). Compiler/build/test
processes exited. Initial harness wording/escaping and missing trailing-zero-key
retention were corrected before this pass. Direct count is 62/70; only the eight
deferred stream names are absent. Whole-product QA remains STEP-90F; entry count
alone closes no BIF.

**Approved numeric decisions, 2026-10-07.** Adrian approved the existing signed
64-bit integer range for positional/count WHOLE BIF operands, with source-anchored
40.12 outside it. Apply the check before expansion/allocation or VM conversion;
WHOLENUM radix operands remain arbitrary precision under their current contract.
Adrian also directed ANSI standard FORMAT rules. Preserve its initial
caller-DIGITS rounding and add a reduced-DIGITS proof. IBM documents that rule;
Regina 3.9.7 disagrees on the retained `DIGITS 2; FORMAT('12.3456',,3)` probe
(`/tmp/crexx-bif-numeric-audit.59_1f2if`, cREXX `12.000`, Regina `12.346`).
That difference is not a formatter defect. Adrian subsequently approved ANSI/Classic caller-DIGITS rules for ABS, MAX,
MIN, SIGN and TRUNC, and directed that B/G use the same rules.

**LC-STEP-90E numeric family steps (LC-BIF-05/07).**
1. Normalize NUM once with the existing Classic RexxValue +0 operation; inherit
   caller DIGITS/FORM through the five BIFs and variadic validator. FORMAT
   consumes that normalized text, retaining fractional zeros.
2. Apply the same initial +0 and inherited context in the existing B/G decimal
   BIFs; keep their typed arguments, return types and signals.
3. Repair DATATYPE(S) valid constant-symbol rejection through the shared
   symbol classifier; retain literal dot symbols, reject signed/blank-padded
   numbers as symbol spellings, and keep the distinct variable-name SYM rule.
   Regina reproducer: `/tmp/crexx-bif-datatype-symbol.rexx`. This repairs BIF
   classification only; it does not close the scanner/source LC-GAP-06.
4. Verify reduced/high DIGITS, ties, forms, fixed truncation, integer boundaries,
   errors and caller restoration in opt/no-opt direct and linked execution.
   Keep independent float/int families and Unicode/I/O infrastructure unchanged.

**LC-STEP-90E numeric/argument receipt, 2026-10-07.** Shared NUM normalization
now performs Classic +0 under caller DIGITS/FORM once. ABS/MAX/MIN/SIGN direct
entries reuse their common implementation; FORMAT consumes normalized text,
retaining fractional zeros. TRUNC truncates the rounded operand. B/G typed
ABS/MAX/MIN/SIGN/TRUNC/FORMAT inherit the same caller settings and round with
+0, retaining typed returns/signals. Tests requiring high precision now declare
that precision explicitly. Positional WHOLE checks reject values outside signed
64-bit before allocation/conversion; arbitrary-precision WHOLENUM stays separate.
DATATYPE(S) now accepts valid constant symbols; the shared classifier preserves
dot literals and rejects signed/blank-padded numbers as symbol spellings.

Focused Debug builds passed (`/tmp/crexx-bif-ansi-build.log`,
`/tmp/crexx-bif-numeric-shared-build.log`, `/tmp/crexx-bif-symbol-build.log`).
The final selected run passed 57/59; its two failures were obsolete SYMBOL
expectations for valid `.BAD`/`.1.2` constants. After updating only those test
expectations, both tests passed 2/2 (`/tmp/crexx-bif-symbol-final.log`). The
other 57 results remain valid (`/tmp/crexx-bif-numeric-focused-final.log` and
its command/memory JSON). Thus all 59 selected checks have passing final-input
evidence: reduced/high DIGITS, forms and restoration, ties, fixed truncation,
INT64 boundaries, huge exponents, source-anchored errors, direct/linked and
opt/no-opt B/C/G plus the common legacy entry. Peak child RSS was 685 MiB;
zero children remained and swap stayed 355.94 MiB.

The consolidated ASCII reference fixture has 104 values, matching Regina
(`/tmp/crexx-bif-reference-audit.log`). Original per-line checking-routine calls
made optimized compilation unnecessarily expensive; those two development
compiles were stopped, and the fixture now compares one complete output file.
The shorter initial version measured 56.59s isolated Debug and 358 MiB child RSS,
with no children left (`/tmp/crexx-bif-reference-isolated.log`); the final added
constant-symbol cases passed no-opt in the 59-check run. The test is serialized
with a 600s hang backstop; maintained-sanitizer measurement and all final modes
remain required before grouped QA. These are admitted ASCII contract repairs,
not Unicode/I/O, scanner, VM/linker or broader host closure. Full reference
limits and the deferred eight stream BIFs remain open.

**LC-STEP-90D CONDITION proof steps (LC-BIF-04/07).** Add direct C/D/E/I/S
and default/omitted/invalid/count checks for all seven existing condition IDs,
ON/OFF/DELAY and child isolation. Extend the existing typed CALL matrix with E;
extend existing live ADDRESS ERROR/FAILURE/NOTREADY and arithmetic LOSTDIGITS
fixtures with missing E/I/S assertions without new producers or host support.
Retain SYNTAX/NOVALUE, source and nested-restore fixtures. Real host HALT,
new stream producers, mapped source and deferred Unicode behavior remain open.

**LC-STEP-90D CONDITION receipt, 2026-10-07.** The direct BIF now has a
focused test for every C/D/E/I/S field and all seven admitted IDs, initial
empty state, default and long/lowercase options, argument errors, ON/OFF/DELAY,
extra-data preservation/reset and child isolation. Existing compiled fixtures
prove SYNTAX extra error number, NOVALUE, live ADDRESS ERROR/FAILURE/NOTREADY,
LOSTDIGITS and controlled typed CALL events; newly added assertions cover their
previously missing E/I/S fields. No producer, host ABI or VM/linker change was
made. Real host HALT remains unrun under LC-GAP-04; controlled HALT proves
handler/field behavior only.

Focused Debug build passed (`/tmp/crexx-bif-condition-build.log`); CTest regex
`^(testRexxClassicBifCondition_|testRexxActivationArguments_|levelc_condition_|levelc_call_condition_matrix_|levelc_numeric_lostdigits_|levelc_address_host_callback$|levelc_address_.*(reference|matrix|notready))`
with `--parallel 4 --output-on-failure` passed 35/35 in 17.00s
(`/tmp/crexx-bif-condition-focused.log`). The final warning-free unit/host
build and exact changed unit/host replay passed 3/3
(`/tmp/crexx-bif-condition-final-build.log`, `/tmp/crexx-bif-condition-final.log`).
Retained unchanged 32 results remain valid. Both monitored runs left zero
children, with unchanged swap; peak child RSS was 537 MiB. Grouped product and
sanitizer proof remain LC-STEP-90F. SOURCELINE source-map/physical-NUL behavior
and Unicode compatibility retain their deferred owners.

**LC-STEP-90F product-preparation repair plan, 2026-10-07.** The frozen
checkpoint's full Debug build stopped on both modes of `test_trace_exit`:
source-anchored `CLASS_NOT_FOUND` at `trace results`. Isolated unchanged replay
reproduces (`/tmp/crexx-bif-trace-prep-replay.log`); a copied source reproduces
too (`/tmp/crexx-bif-trace-isolation-log.ViuLap`). TRACE's generated helper imports
`RexxValue` from `rxfnsc`, but the class is exposed by `rexxvalue`. That source is
identical to the programme's starting revision; this is a pre-existing product
preparation defect, not numeric/Unicode behavior fallout.

Vision: restore the existing Level B/G TRACE helper's declared class import so
the requested complete product checkpoint can run. No TRACE language rule,
host interface, VM/linker or Unicode/I/O change is intended.

1. **LC-BIF-PREP-01:** the existing TRACE exit fixture compiles and executes in
   opt/no-opt with the declared class import; verify focused normal Debug.
2. **LC-BIF-PREP-02:** the same focused cases pass through the maintained
   sanitizer runner, with no first-party diagnostic; leave platform limits open.
3. **LC-STEP-90F-1:** correct the existing exit descriptor import, using the
   documented `rexxvalue` namespace; retain the existing regression fixture.
4. **LC-STEP-90F-2:** run the focused build/tests, commit the repair separately,
   freeze revised code/test/build inputs, then resume one full normal Debug and
   one Release correctness run. No broad CTest has yet started.

**LC-STEP-90F product-preparation repair receipt.** The TRACE descriptor now
imports the existing `rexxvalue` namespace. Its existing exit/results/negated-string
fixtures built and passed 6/6 normal Debug (1.00s CTest;
`/tmp/crexx-bif-trace-repair-build.log`, `/tmp/crexx-bif-trace-repair-tests.log`).
The matching maintained ASan build and 6/6 test run passed (2.34s CTest;
`/tmp/crexx-bif-trace-asan-build.log`, `/tmp/crexx-bif-trace-asan-tests.log`,
runner `20261007-223123-ctest`), without diagnostics or residual children.
`LC-BIF-PREP-01/02` are verified on macOS ASan's supported capability; Apple
LeakSanitizer remains unavailable. The earlier focused BIF ASan run passed 53/53
(478.02s; `/tmp/crexx-bif-asan-focused.log`, runner `20261007-221739-ctest`).
Its BIF code/tests remain unchanged by this descriptor-only repair. The serialized
reference audit measured 160.90s optimized and 162.56s linked optimized under
ASan, within the explicit 600s hang backstop; no-opt modes took 17.06/18.59s.
The first all-target normal build failure and isolated reproducer are retained;
no broad CTest has run yet. Resume product preparation on the revised frozen
checkpoint rather than report that failed build as passed.

**LC-STEP-90F early Debug attempt and focused repair plan.** The first broad
normal Debug attempt was interrupted after 156.45s when five failures exposed
three causes: `source_extension_direct_defaults` still expected headerless
Classic compilation to fail; both `levelc_numeric_whole` modes expected the
superseded unrounded numeric BIF values; `rxc_diagnostic_catalogs` found missing
German/Dutch `LEVELC_CALL_SIGNATURE` keys. A fourth cause is the existing
`trace_stem_sugar` regression: setter rewriting marks its whole assignment
synthetic, suppressing authored source records while retaining the value. Log:
`/tmp/crexx-bif-final-debug-ctest.log`. The interrupted process group exited with
zero remaining children; its owned empty stdin-driver lock is removed before
resuming. This attempt is not a completed or passing full Debug gate.

Vision: make the requested complete product baseline checkable without changing
language rules, I/O/Unicode transport, native ABI or VM/linker behavior. Preserve
the documented `.rexx` default, Adrian-approved numeric rounding and existing
TRACE statement/source expectations. The wider LC-GAP-06 review stays pending.

1. **LC-BIF-PREP-03:** headerless `.rexx` regression verifies successful Classic
   lowering, rather than absence of implemented Level C; numeric expected output
   agrees with ANSI initial rounding; translated catalogs contain the shipped
   static CALL diagnostic. Verify their existing focused normal tests.
2. **LC-BIF-PREP-04:** rewritten authored object/stem setter statements retain
   their source anchor on the outer CALL while helper operands stay synthetic.
   Existing results/intermediates TRACE regression must pass in normal Debug
   and maintained ASan; retain adjacent property/AST/TRACE tests.
3. **LC-STEP-90F-3:** correct only the stale fixture expectations and missing
   translation entries; update the existing source-default assertion without
   expanding its compiler workload.
4. **LC-STEP-90F-4:** preserve the original authored source through the existing
   setter rewrite using the shared source-anchor API. Do not change dispatch,
   optimizer policy, bytecode, native ABI or statement semantics. Run focused
   normal and maintained-sanitizer checks, commit the distinct repairs, freeze
   again, prepare the product and perform one complete final-input Debug run.

**LC-BIF-PREP-03 focused receipt.** The source-default fixture now supplies
valid headerless Classic source and inspects its generated Classic pool path;
its workload still has the same compiler invocations. NUMERIC's two expected
lines now use Adrian-approved initial BIF rounding and engineering scale. The
German/Dutch catalogs now contain the existing static signed CALL diagnostic.
The six affected/adjacent checks passed normal Debug 6/6 in 7.14s after the
focused product build (`/tmp/crexx-bif-qa-repair-build.log`,
`/tmp/crexx-bif-qa-repair-focused.log`). The source/provenance follow-up passed
8/8 in 27.86s (`/tmp/crexx-bif-source-after-provenance.log`). Both runs left zero
children and unchanged swap. Maintained sanitizer proof for the source-anchor
repair and the complete final-input product checkpoint remain open.

**LC-BIF-PREP-04 source-anchor repair receipt.** The existing indexed-object
setter rewrite now copies the authored assignment anchor to its outer CALL with
inherited provenance. Synthetic input nodes retain their existing provenance;
execution, method selection, optimization policy and VM/linker formats are
unchanged. The existing TRACE results/intermediates fixture verifies the missing
source lines and correct values. Source-line/linked provider isolation also
passes after the repair (6/6 adjacent checks plus 8/8 source/provider checks in
the normal Debug receipts above).

Matching focused maintained ASan preparation and 14/14 checks passed in 94.27s
(`/tmp/crexx-bif-provenance-asan-build.log`,
`/tmp/crexx-bif-provenance-asan-tests.log`, runner `20261007-225125-ctest`). No
sanitizer diagnostic was found; peak child RSS was 1.22 GiB, zero children
remained and swap stayed 347.94 MiB. `LC-BIF-PREP-03/04` are verified for the
admitted local paths. Apple LeakSanitizer and unrun platform gates are not passed.
This narrow existing-source repair does not close LC-GAP-06. The revised product
checkpoint must still complete its core preparation, full normal Debug suite,
Release correctness and installed/host smoke checks on final code/test inputs.

**LC-STEP-90F negative traceback assertion follow-up plan.** The next Debug
attempt on `15b8f9e1b` was stopped after 261.45s: four no-opt negative
interface/cast fixtures still expected only the panic line. Their output now
correctly includes the authored assignment source retained by LC-BIF-PREP-04.
Unchanged direct VM probes return the existing signal exit codes (11 for missing
factory, 6 for conversion), with exact source lines; there is no product value
or signal regression. Log: `/tmp/crexx-bif-checkpoint-debug-ctest.log`; direct
probes: `/tmp/crexx-bif-negative-{interface_no_impl,interface_match_reject_single,interface_named_factory_no_impl,type_ops_fail}.log`.
The interrupted run briefly left its Level G process-worker parent/child;
owned processes were stopped and its empty stdin driver lock removed. This is
an interrupted attempt, not a completed full gate.

Vision: make existing negative fixtures assert the correct panic, signal and
source traceback while tolerating only unstable module/address records. Preserve
native/compiler behavior and keep source evidence visible.

1. **LC-BIF-PREP-05:** all four negative fixtures compare the exact panic in
   no-opt/direct and opt/linked modes; no-opt also asserts the authored source
   line. Optimized failure output retains its existing panic-only golden; full
   source equivalence remains LC-GAP-06. Verify 8 normal
   Debug checks and matching maintained ASan checks; no diagnostic/source strip.
2. **LC-STEP-90F-5:** normalize only `at module ... address ...` in this existing
   four-fixture harness and retain source records in maintained no-opt goldens.
   No compiler invocations are added; the aggregate workload is unchanged.
   Commit test repair separately, freeze revised inputs, then complete full
   Debug and Release correctness on those final inputs.

**LC-BIF-PREP-05 receipt.** Four no-opt goldens now assert the exact authored
source as well as the panic. Optimized linked goldens keep their existing panic
output. Only unstable module/address records are normalized; source records are
never stripped. Normal Debug passed 8/8 in 1.60s
(`/tmp/crexx-bif-negative-debug-final.log`); matching maintained ASan passed 8/8
in 6.33s (`/tmp/crexx-bif-negative-asan-tests.log`, runner
`20261007-230635-ctest`) after focused runner preparation
(`/tmp/crexx-bif-negative-asan-build.log`). No sanitizer finding occurred;
peak test child RSS was 717 MiB, no children remained, and swap stayed 347.94 MiB.
The earlier opt golden mismatch was corrected by keeping mode-specific source
expectations; no product edit was needed. LC-BIF-PREP-05 is verified for these
fixtures; optimized general source equivalence remains LC-GAP-06.

**LC-STEP-90F assembly provenance follow-up.** The next Debug attempt on
`91e2a2416` was interrupted after 404.17s when four compiler goldens disagreed
with the intended source-anchor repair. In `13_stems` and
`repro_multi_tail_stems`, both modes still expected `.srcstep` flags 6
(GENERATED|SYNTHETIC), whereas authored setter clauses correctly emit flags 33
(AUTHORED|INHERITED). The third field is provenance flags, not AST statement
type. Comparing the entire four generated assembly files establishes that
only these provenance fields differ: every executable instruction and every
other metadata line is identical. Log:
`/tmp/crexx-bif-checkpoint-debug-ctest-final.log`; no timeout or sanitizer finding.
The interrupted run left no children; its owned empty stdin lock is removed.

1. **LC-BIF-PREP-06:** maintained assembly goldens assert authored/inherited
   provenance for the repaired setter clauses, while executable code is byte
   for byte unchanged. Verify four compiler goldens, their four runtime modes
   and the existing TRACE stem test in normal Debug and maintained ASan.
2. **LC-STEP-90F-6:** update only the proven flags in those four existing golden
   files. No native/compiler edit, workload expansion, language/ABI rule or
   source contract change. Retain earlier focused repair evidence; freeze the
   revised tests and complete the final full Debug run once on those inputs.

**LC-BIF-PREP-06 receipt.** The four maintained compiler goldens now contain
only the proven generated/synthetic → authored/inherited flag changes (4 setter
clauses in each `13_stems` mode, 6 in each multi-tail mode). Full assembly
comparison confirms every executable instruction and other metadata byte is
unchanged. The four compiler checks, four runtime modes and TRACE stem check
passed normal Debug 9/9 in 2.57s (`/tmp/crexx-bif-golden-debug-tests.log`) and
maintained ASan 9/9 in 8.65s (`/tmp/crexx-bif-golden-asan-tests.log`, runner
`20261007-232214-ctest`). No sanitizer diagnostic, residual children or swap
increase occurred. This is a golden synchronization for the earlier reviewed
source repair; no compiler/native code was changed. LC-BIF-PREP-06 is verified.

**LC-STEP-90F whole-product snapshot follow-up plan.** The completed-family
checkpoint's full Debug run on `902fee62a` has exposed three stale test
expectations: two ADDRESS parser snapshots predate the existing `input_binary`
factory before `input_array`; the channel metadata assertion hard-codes 660
opcodes, predating the approved SIGNALORIGIN entry at index 660 (current count
661). The relevant ADDRESS implementation, opcode table and native metadata
source are identical to the programme's starting revision. Both ADDRESS
factories remain implemented. No BIF, I/O/Unicode transport, VM/linker or native
API repair is needed. Retain this failed run as evidence; it cannot be called a
passing gate.

Vision: make existing normal-product tests reflect the current approved product
contract, preserving all runtime behavior and the independent BIF checkpoint.

1. **LC-BIF-PREP-07:** the two maintained ADDRESS AST snapshots match the current
   shipped descriptor declarations; opcode-table width is verified by the
   existing common consistency assertions, while the channel-specific test
   still verifies every channel effect/component/signal condition. Verify the
   three repaired tests and adjacent ADDRESS behavior in normal Debug and
   matching maintained ASan. No product or I/O behavior changes.
2. **LC-STEP-90F-7:** after this full run finishes, reconcile the two snapshots,
   replace only the obsolete duplicate hard-coded table-width check with the
   stable channel opcode identity, build the native test, run focused proof and
   commit one QA-fixture increment. Freeze revised final inputs; complete the
   required full normal Debug and Release correctness and installed/host smoke.

**LC-STEP-90F import-resolution repair plan, 2026-10-08.** The first
completed full Debug run on `902fee62a` ran all 3296 tests: 3280 passed and
16 failed (`/tmp/crexx-bif-checkpoint-debug-ctest-qualified.log`). Ten parser
fixtures reproduced unchanged in isolation. A native process sample
(`/tmp/crexx-bif-parser-stall.sample`) shows nested binary metadata signature
imports in class resolution; parser_tester reaches its internal cutoff and
returns a fallback lexer tree. Raising the cutoff would conceal the unresolved
import work. The existing `syntaxhighlight_parse_keywords.crexx` is the minimal
product reproducer. Six other failures are stale ADDRESS/opcode snapshots or
isolated compiler fixtures lacking the documented exit-disabled bootstrap.

Vision: restore current approved compiler import resolution and existing
whole-product QA without changing syntax, numeric policy, Unicode/I/O, host ABI,
VM/linker behavior, or performance production policy. Keep broader scopes open.

1. **LC-BIF-PREP-08:** identify the exact nested import cause and repair the
   existing resolver, retaining real contract validation and the shared loader.
   Verify the ten parser fixtures and adjacent import/cache/exit checks in
   normal Debug and maintained ASan; preserve a permanent reproducer.
2. **LC-STEP-90F-8:** trace bounded import evidence, repair the owning existing
   path, run focused normal and sanitizer proof, and commit separately from
   stale QA expectations. Any genuine architecture decision remains gated.
3. **LC-BIF-PREP-09:** all six non-parser failing fixtures retain their original
   isolation and behavioral assertions while using current descriptors, opcode
   consistency and the documented `-x` bootstrap. Verify normal Debug/ASan;
   the three compiler correctness fixtures also run in Release even though
   their existing label excludes them from the ordinary Release lane.
4. **LC-STEP-90F-9:** reconcile those six fixtures in one QA commit, freeze final
   code/test/build inputs, then complete final Debug/Release and installed/host
   qualification. The failed completed Debug gate remains retained, not passed.

**LC-BIF-PREP-08 diagnosis refinement.** Bounded diagnostic-only tracing showed
finite discovery of unrelated generated/linked modules from the shared compiler
CTest working directory, not a repeated-file cycle. The same authored parser
reproducer returns the complete semantic tree in a dedicated directory (2.08 s
monitor, versus the 12.20 s fallback in the polluted directory). Temporary
compiler tracing was removed and `rxcpfunc.c` matches HEAD byte-for-byte. No
resolver change was necessary or made. Existing parser fixtures now run in
per-case directories, retain their exact semantic assertions, parser lock and
serialized scheduling, and still discover packaged runtime/exit modules.
Broader import discovery/scalability design remains pending; this is QA input
isolation, not a production performance or architectural change.

The isolated certified BIF fixture additionally shared its consumer source root
with deliberate `rxfnsb` spoof providers. The existing approved source-root
precedence correctly selects those providers and shadows the later library
root, leaving unrelated BIF names unavailable. Copying its three ordinary
consumers to an owned separate source root restores source/binary certification
checks while keeping spoof scenarios unchanged. All three isolated compiler
fixtures use `-x --no-exe-import` and owned working directories; no exit/runtime
provider is silently injected. The six snapshot/bootstrap repairs are test-only.

Focused normal proof: 74/75 passed in the initial 22.37 s focused run; the
remaining certified case passed 1/1 in 3.97 s after source-root isolation.
Logs `/tmp/crexx-bif-qa-final-repair-debug.log` and
`/tmp/crexx-bif-certified-final-debug.log` qualify 75 unique final-input cases;
no completed child processes remained. The peak was 229.8 MiB descendants,
17203.2 MiB total, and unchanged 347.94 MiB swap. These measurements include
all 69 maintained parser fixtures, the two AST snapshots, native opcode
metadata and three source/binary/opt/no-opt compiler correctness fixtures.
Maintained sanitizer confirmation and final grouped qualification remain open.

**LC-BIF-PREP-07/08/09 focused receipt, 2026-10-08.** Matching maintained
ASan passed 73/75 in 55.53 s; the two snapshot mismatches contained only stale
`packednumeric@rxfnsg.rxbin` inline-summary diagnostics. Rebuilding the existing
`rxfnsg` prerequisite (237.60 s monitor, runner `20261008-000756-build`) removed
those stale generated inputs; the two unchanged snapshots then passed 2/2 in
1.76 s (runner `20261008-001157-ctest`). Logs:
`/tmp/crexx-bif-qa-final-repair-asan-build.log`,
`/tmp/crexx-bif-qa-final-repair-asan.log`,
`/tmp/crexx-bif-qa-snapshot-asan-rebuild.log`, and
`/tmp/crexx-bif-qa-snapshot-asan-final.log`. This verifies 75 unique final-input
focused cases in both normal Debug and maintained macOS ASan; no sanitizer
finding occurred. Peak rebuild descendants 933.1 MiB / total 17088.5 MiB,
no residual children, unchanged 347.94 MiB swap. Apple LSan is unavailable,
not passed. The existing 69 parser assertions/lock/serialization and six product
QA assertions are preserved; broader import-discovery design remains pending.
These preparation criteria are verified. Final grouped QA still remains open.

**LC-STEP-90F final-input Debug receipt / Release preparation follow-up,
2026-10-08 (recorded during the Release run; final disposition follows).**
Frozen code/test checkpoint `d7b58e17d8d49979445543676c2bcd56fb1f351b`,
tree `61c42a32455585746301a26cceb6861334dee671`; code/test/build fingerprint
excluding this receipt and the BIF guide:
`a199e7f76671a87128c1cfe008794f4e24ccc40344cab2ee9cb083a65dbf933b`.
Core/preparation `cmake --build cmake-build-debug --target all qa-prep
qa-prep-measurement --parallel 4` passed (380.46 s monitor). Full normal Debug
`ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure` passed
3296/3296 (1597.93 s CTest / 1599.28 s monitor), including 771 Level C-labelled
cases, configured B/G/L, RexxScript, RXAS/linker/VM and native interfaces. Logs
`/tmp/crexx-bif-product-final-debug-build.log` and
`/tmp/crexx-bif-product-final-debug-ctest.log`. Test peak descendant/total RSS
5108.2/21215.5 MiB, unchanged 347.94 MiB swap, no remaining children.

Release core `cmake --build cmake-build-release --target all qa-prep --parallel 4`
passed (189.14 s monitor, peak 1969.0/17984.7 MiB, no children). The ongoing
normal Release lane contains 3103 tests, not Debug's 3110 non-measurement tests:
its model cache was unset, omitting seven existing optional Llama model/lifecycle
checks. Debug has the available cache `/Users/adrian/Library/Caches/crexx/native-inference`.
Enabling that existing test-only setting after the current run will qualify the
seven checks once, with unchanged registry commands/properties and product
hashes proving reuse of other results. No model download or new provider work.

The Release run initially reproduced four Text Inspector failures (both core/TUI
modes); the completed log also failed generation and ANSI-PTY, six total. Their
staged owned `rxpp/ui/UI_NODE.rxpm` retains an old uppercase
filename on the case-insensitive host; its bytes match the authored lowercase
`ui_node.rxpm` (SHA256 `3a423c24d28730e67a73389628cf6da02acb1bd3801153b654574b771f489b05`).
Current RXPP intentionally admits lowercase package filenames, as documented in
`docs/ai-context/RXPP_PREPROCESSOR.md`; Debug's owned copy is already lowercase.
Release generated output consequently contains nine unexpanded `##UI_NODE`
statements. This is a stale build input, not a numeric BIF or new I/O rule.

Vision: restore the existing documented build-input contract and finish the
required whole-product checkpoint without changing product/test source or
repeating unchanged broad evidence.

1. **LC-BIF-PREP-10:** normalize only the owned staged macro filename to its
   declared lowercase path, regenerate the affected example artifacts, and
   pass all six failures plus the adjacent package
   checks in normal Release. Verify the generated macro expansions and retain
   the failed full-lane log; reuse unchanged Debug proof and other Release cases.
2. **LC-STEP-90F-10:** after the current run and child-exit proof, restore that
   staged input and enable the existing cached-model Release QA setting; build
   required artifacts, compare existing test command/property and product
   hashes, then run the affected Release cases, the seven added native checks
   and the three repaired compiler fixtures once. Product/ABI/Unicode/I/O rules
   and the whole Debug code/test inputs remain unchanged. Complete installed
   and host BIF smoke, final inventory, documents and unpublished-state report.

**LC-BIF-PREP-10 receipt, 2026-10-08.** The Release lane completed with
3097/3103 passing and six Text Inspector failures in 509.91 s CTest / 511.23 s
monitor. The initial four-failure update counted the core/TUI cases; the completed
lane has six including generation/ANSI-PTY. The full failed log remains
`/tmp/crexx-bif-product-final-release-ctest.log`.
Only the owned staged macro filename was normalized to the declared lowercase
path; bytes remained identical to the authored macro. Regeneration changed
nine unexpanded `##UI_NODE` occurrences into the nine expected `view.add_spec`
calls. No product/test source, I/O policy or macro language was changed.

Commands after the broad lane and child-exit proof:

```sh
cmake -S . -B cmake-build-release -DCREXX_LLAMA_TEST_MODELS=/Users/adrian/Library/Caches/crexx/native-inference
cmake --build cmake-build-release --target example_text_inspector_artifacts llama_provider_runtime_package rxllama_bridge_lifecycle rxc rxas rxlink rxbvm library --parallel 4
ctest --test-dir cmake-build-release --parallel 2 --output-on-failure -R '^(text_inspector_.*|rxllama_release_package_smoke|rxllama_bridge_(bge|smol|both)_(cpu|required-gpu)|rxllama_toolchain_lifecycle|perf2_04_(certified_call|inline_assembler_imports)|perf2_05_partial_call)$'
```

Configuration and affected build passed in 4.08/2.08 s monitor; completion panel
passed 17/17 in 38.48 s CTest / 38.69 s monitor. This covers all six failures,
one adjacent package check, seven newly enabled cached-model CPU/required-GPU
and lifecycle checks, and three compiler correctness fixtures carrying the
performance-measurement label. Logs `/tmp/crexx-bif-release-model-configure.log`,
`/tmp/crexx-bif-release-affected-build.log`, and
`/tmp/crexx-bif-release-completion-focused.log` with command/memory JSON siblings.
Peak completion descendants/total RSS 1590.0/17432.8 MiB; no children remained.
The registry comparison has **zero changed commands/properties for all 3289
previous tests**, exactly seven additions; SHA256 of ten core product/compiler
and runtime images is unchanged before/after model-cache configuration.
Proof: `/tmp/crexx-bif-release-model-registry-proof.json` and
`/tmp/crexx-bif-release-products-{before,after}-models.json`.
The final registry is 3296 tests, of which 3110 are normal correctness and 186
carry the measurement label; the three labelled compiler correctness fixtures
are qualified separately. Thus **3113 unique Release correctness checks have
passing evidence**: retained unchanged passes plus the focused completion,
not a falsely passed first broad run or a second broad suite.
LC-BIF-PREP-10 is verified. Debug code/test/build inputs were unaffected.

**LC-STEP-90F final grouped local product receipt, 2026-10-08.**

Qualified source revision: `d7b58e17d8d49979445543676c2bcd56fb1f351b`, tree
`61c42a32455585746301a26cceb6861334dee671`. SHA256 of the NUL-terminated records
from `git ls-tree --full-tree -rz <revision>` in Git's output order, excluding
exactly this worklist and `compiler/docs/levelc_classic_bifs.md`:
`a199e7f76671a87128c1cfe008794f4e24ccc40344cab2ee9cb083a65dbf933b`.
The subsequent receipt/guide commit changes only those two documents, so this
product/test/build fingerprint is the reuse condition; no broad suite is
repeated for receipt-only changes. `/tmp/crexx-bif-checkpoint-inputs.json`
retains the qualification revision, trees and superseded checkpoints.

Host: Darwin arm64, 10 logical CPUs, 24 GiB physical RAM. Ordinary Debug/Release
use Ninja, `/usr/bin/cc`, `-g` / `-O3 -DNDEBUG`, VM profiling OFF, Llama ON;
existing available model cache is enabled in both final normal configurations.
The local syntax-highlighter dependency is
`383e5daab0ffcf6ec83e02db7a01a27709dabb2c`; its unrelated dirty `AGENTS.md`
is preserved. Maintained Debug ASan uses address/frame-pointer flags and GNU
Make; its ordinary upstream Llama engine is uninstrumented. No new platform,
ABI, VM/linker or Unicode/I/O configuration was introduced.

```sh
cmake --build cmake-build-debug --target all qa-prep qa-prep-measurement --parallel 4
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure
cmake --build cmake-build-release --target all qa-prep --parallel 4
ctest --test-dir cmake-build-release --label-exclude '^performance-measurement$' --parallel 10 --output-on-failure
cmake --install cmake-build-release --prefix /tmp/crexx-bif-install.6v6bvmo_
python3 /tmp/crexx-bif-installed-smoke.py /tmp/crexx-bif-install.6v6bvmo_ /tmp/crexx-bif-installed-checks.bfbex8vo
```

The Release lane's six failed cases are repaired/requalified by PREP-10 above;
its unchanged passing results remain valid. The full normal Debug gate passed
**3296/3296** in 1597.93 s CTest / 1599.28 s monitor, including 771 Level C-labelled
cases and configured B/G/L, RexxScript, RXAS, linker, VM, native interfaces and
measurement-labelled tests. The Debug all-target/preparation build passed in
380.46 s monitor; Release core/preparation in 189.14 s. No required normal
correctness case is skipped or called passed while unrun. The final normal
Release coverage is **3113 unique checks**, with optimized/no-opt and linked
execution across the configured levels. The 183 remaining Release measurement
cases were not run; this is not a Release performance qualification.

Verbose logs: `/tmp/crexx-bif-product-final-debug-{build,ctest}.log`,
`/tmp/crexx-bif-product-final-release-{build,ctest}.log`,
`/tmp/crexx-bif-product-final-install.log`, and
`/tmp/crexx-bif-product-final-installed-smoke.log`; JSON siblings retain literal
argv, exit, elapsed time, sampled descendant/total RSS, swap and child-exit proof.
Listed commands were launched through `python3 /tmp/crexx-bif-monitor.py <log>
<command...>` with both output streams redirected to that temporary log.
Runs were serialized across broad jobs, build parallelism 4 / normal CTest 10 /
focused sanitizer and Release completion 2. Full Debug peak descendants/total
RSS was 5108.2/21215.5 MiB; Release broad peak 2159.3/17931.4 MiB. Every completed
long run left zero children, with unchanged 347.94 MiB swap. Final executable
process inventory also found no compiler/assembler/linker/VM/parser/CTest/build
processes. Only the exact owned empty stdin lock and Python cache generated by
these tests were removed; unrelated work was preserved.

Installation passed in 2.07 s monitor. Installed smoke passed **19/19 BIF cases
plus one RXVML callback lifecycle fixture** in 38.65 s monitor: B/C/G ANSI
numeric context in direct/linked optimized/no-opt modes (12), queue/source/message
linked optimized/no-opt (6), and the 104-value reference audit linked optimized
(1). Each BIF case uses the maintained `compiler/tests/levelc_queue_linked.cmake`
with all tools and runtime bytecode from the installation; it performs
`rxc -> rxas -> rxlink -> rxvm` or the maintained direct mode. The host fixture
is `compiler/tests/src/test_levelc_address_host_callback.c`, compiled against
installed static libraries and freshly compiled installed-toolchain source.
Its 12 callbacks verify existing ADDRESS/CONDITION ERROR/FAILURE/NOTREADY paths,
values and owned output/error file cleanup; the RXVML context is destroyed.
No new host provider is implemented or approved by this smoke.
Installed peak descendants/total RSS 344.4/15716.3 MiB; zero children/files remain.
Receipt with exact expanded argv and eight installed/Release product SHA256
matches: `/tmp/crexx-bif-installed-checks.bfbex8vo/receipt.json`.
Installed smoke script SHA256: `b6aacee8da273564b8461250d8b789c9baaea589dfe4ee907f96a8edfd0964b0`.

The exact installed BIF invocation is the following maintained recipe, repeated
for the 19 cases/modes listed above; NAME is `installed_` plus the source basename,
SOURCE/EXPECTED are in `compiler/tests/rexx_src`, and BINDIR/RXC/RXAS/RXLINK/RXVM
are `/tmp/crexx-bif-install.6v6bvmo_/bin` and its named executables. BUILD_DIR is
`/tmp/crexx-bif-installed-checks.bfbex8vo`. NOOPT/DIRECT are ON or OFF for the
stated modes. The host compile/run literal argv is retained in the receipt/log.

```sh
cmake -DRXC="$BINDIR/rxc" -DRXAS="$BINDIR/rxas" -DRXLINK="$BINDIR/rxlink" -DRXVM="$BINDIR/rxvm" -DBINDIR="$BINDIR" -DNAME="$NAME" -DSOURCE="$SOURCE" -DEXPECTED="$EXPECTED" -DBUILD_DIR="$BUILD_DIR" -DNOOPT="$NOOPT" -DDIRECT="$DIRECT" -P compiler/tests/levelc_queue_linked.cmake
```

**Focused maintained sanitizer evidence.** The 53-case BIF panel on
`c6e03a62d65a2dcebf474d9166a969965f88a087` remains valid for unchanged BIF
implementation/test inputs. Later changed native/source ownership and fixture
paths have matching normal Debug and ASan overlay receipts, rather than a
redundant broad sanitizer run. No first-party sanitizer finding occurred.
All commands use `tools/asan-run.sh`, test jobs 2 and `--leaks off` because
Apple LSan is unavailable; this is ASan evidence, not leak-clean proof.
The exact retained test commands are:

```sh
tools/asan-run.sh --phase ctest --regex '^(testRexxClassicBif(Datatype|Queued|Sourceline|Errortext|Format|Condition)_|level[bcg]_bif_(queued|sourceline|errortext|whole|integer_limits|numeric_context|reference_audit)|levelc_call_external_|levelc_address_host_callback$)' --test-jobs 2 --leaks off --stop-on-failure --no-live-tail --tail-lines 20
tools/asan-run.sh --phase ctest --regex '^test_trace_(exit|results|negated_string)_' --test-jobs 2 --leaks off --no-live-tail --tail-lines 10
tools/asan-run.sh --phase ctest --regex '^(source_extension_direct_defaults$|rxc_diagnostic_catalogs$|levelc_numeric_whole_(opt|noopt)$|trace_stem_sugar$|trace_event_metadata$|levelc_bif_sourceline|levelc_call_external_)' --test-jobs 2 --leaks off --no-live-tail --tail-lines 15
tools/asan-run.sh --phase ctest --regex '^(interface_no_impl|interface_match_reject_single|interface_named_factory_no_impl|type_ops_fail)_run_(noopt|opt)$' --test-jobs 2 --leaks off --no-live-tail --tail-lines 10
tools/asan-run.sh --phase ctest --regex '^((13_stems|repro_multi_tail_stems)(_run)?_(noopt|opt)|trace_stem_sugar)$' --test-jobs 2 --leaks off --no-live-tail --tail-lines 10
tools/asan-run.sh --phase ctest --test-jobs 2 --leaks off --regex '^(syntaxhighlight_.*|address_(exit_extended_parse|inline_then_parse)|rxas_optimizer_metadata|perf2_04_inline_assembler_imports|perf2_04_certified_call|perf2_05_partial_call)$' --no-live-tail --tail-lines 30
tools/asan-run.sh --phase ctest --test-jobs 2 --leaks off --regex '^address_(exit_extended_parse|inline_then_parse)$' --no-live-tail --tail-lines 20
```

| Focus | Passing normal/ASan proof | Retained receipt |
| --- | --- | --- |
| BIF/selected queue/source/message/numeric/context/host panel | focused normal family receipts above; ASan 53/53, 478.02 s | `/tmp/crexx-bif-asan-focused.log`, runner `20261007-221739-ctest` |
| TRACE descriptor import | 6/6 + 6/6, ASan 2.34 s | `/tmp/crexx-bif-trace-{repair,asan}-tests.log` |
| Authored setter provenance/source/provider paths | 6 adjacent + 8 source/provider normal; ASan 14/14, 94.27 s | `/tmp/crexx-bif-provenance-asan-tests.log` |
| Exact negative panic/source fixtures | 8/8 + 8/8, ASan 6.33 s | `/tmp/crexx-bif-negative-{debug-final,asan-tests}.log` |
| Four source provenance assembly goldens + runtime/TRACE | 9/9 + 9/9, ASan 8.65 s | `/tmp/crexx-bif-golden-{debug,asan}-tests.log` |
| Parser input isolation, AST/opcode snapshots, three compiler fixtures | 75 unique normal; 75 unique ASan (73 retained + 2 after stale prerequisite rebuild) | PREP-07/08/09 receipt above; `/tmp/crexx-bif-qa-final-repair-asan.log`, `/tmp/crexx-bif-qa-snapshot-asan-final.log` |

Build/preparation literal commands and process metrics remain in the corresponding
`*-asan-build.log.json` / `*-asan-rebuild.log.json` siblings named by the focused
receipts. Aggregate workload and serialized 600 s reference-audit timeout were
measured in normal Debug and maintained ASan; no deadline was silently shortened.
No sanitizer suppression or supported-platform leak exception was added.

**Unrun gates/capability limits.** No Linux or Windows normal/platform sanitizer,
full macOS sanitizer matrix, hosted overnight/deep/stress/build-graph/CodeQL or
release-platform gate was dispatched or called passed. Apple LSan is unsupported;
Linux LSan proof is unrun. Focused first-party ASan does not instrument the ordinary
upstream Llama engine. GTK was disabled in normal builds; real ODBC driver coverage
was disabled (configured mock/interface checks ran). Stream positioning, encoding,
EOF and resource matrices are unrun because the eight BIFs remain deferred.
Existing Unicode regressions passing do not define the currently undefined
Unicode error/logic behavior. These limits prevent full Level C/Release 1,
70-name conformance, cross-platform sanitizer-clean or release-ready claims.

**Committed delivery ledger (all unpublished).**

| Commit | Coherent increment | Focused proof |
| --- | --- | --- |
| `6eeb9c988` | QUEUED/shared direct-use tracking | 6/6 |
| `f69ab852f` | retained SOURCELINE and provider isolation | 10/10 |
| `22b6fa48f` | exact decimal/exponent WHOLE | 10/10 |
| `5f49de7e2` | shared-catalog ERRORTEXT | 6/6 |
| `04bbd17d5` | signed WHOLE limit, ANSI B/C/G numeric context, ASCII symbol audit | 59 unique final-input checks |
| `c6e03a62d` | CONDITION admitted producers/fields/frame policy | 35 unique final-input checks |
| `de676ee9c` | existing TRACE class import repair | Debug 6/6; ASan 6/6 |
| `acdced207` | source-default/numeric/catalog QA reconciliation | Debug 6/6 |
| `15b8f9e1b` | authored setter source ownership | Debug adjacent 6 + source/provider 8; ASan 14/14 |
| `91e2a2416` | negative fixture exact source assertions | Debug 8/8; ASan 8/8 |
| `902fee62a` | assembly provenance goldens | Debug 9/9; ASan 9/9 |
| `d7b58e17d` | isolated QA inputs and current metadata snapshots | 75 unique Debug and 75 unique ASan |

The final receipt/guide commit is documentation-only and preserves the frozen
fingerprint above. The last fetched source checkpoint is 0 behind / 220 ahead of
origin/develop; the receipt commit advances the unpublished ledger by one.
All work remains unpublished on develop; release/hosted gates remain unrun.
LC-BIF-03/06/07 and admitted LC-STEP-90C/D/E/F phase delivery are verified.
LC-BIF-01/02/04/05 remain open for the full contracts, with the deferred obligations
preserved rather than removed. Remaining exact BIF limits: eight absent stream
entries pending architectural/compatibility assessment; SOURCELINE mapped inputs
unavailable/count zero and physical-source NUL truncation under LC-GAP-06;
CONDITION real host HALT producer under LC-GAP-04; full resource/reference/platform
proof and undefined Unicode behavior. No admitted focused ASCII behavior failure
remains reproduced. The proposed narrow stream provider remains unapproved; no
new decision was assumed. INTERPRET, LC-GAP-01/03–10, wider host criteria and B/G
compiler split/fast-pipeline proposals remain pending.

### Remaining-gap decision register (2026-10-07)

This is the current closeout queue, not a list of approved exclusions. A
closed `LC-I-*` instruction stays closed on its tested contract while a shared
host, expression or source obligation can remain open. `LC-AC-01/04/06/08/58/59/61/72/73/75`
and `R1-AC-01/02` remain unchecked. The [reference-obligation appendix](levelc-reference-obligations.md)
is a contract inventory; dated partial-status text there must be reconciled
with the later whole-instruction receipts before the fresh review.

| ID | Remaining point and current boundary | Owner / disposition needed |
| --- | --- | --- |
| LC-GAP-01 | `INTERPRET` is recognized but not executable. `LC-87-01–05` cover exact generated Unicode source, nested groups, current frame and condition/control transfers, and bounded code lifetime. The compiled-fragment and RexxScript-inspired routes below are research, not approved designs. | `LC-I-25`, `LC-AC-59/04`, `LC-REF-062`, `R1-AC-01/02`: parked now. Later choose implementation or individually approve a Release 1 "won't implement" entry with diagnostic and documentation. |
| LC-GAP-02 | Of 70 catalogued Classic BIF names, 62 have direct compiler entries; eight do not: `CHARIN`, `CHAROUT`, `CHARS`, `LINEIN`, `LINEOUT`, `LINES`, `QUALIFY`, `STREAM`. Every name has a grouped audit/individual evidence row. QUEUED, SOURCELINE and ERRORTEXT have focused receipts in LC-STEP-90C/D; the independent whole-product checkpoint is locally qualified under LC-STEP-90F. `CONDITION` has admitted producer/field proof; real host HALT remains separately open. Streams and Unicode/I/O changes are deferred by Adrian pending architectural assessment. Source, wider host/resource and platform proof remain open; entry presence alone closes no BIF. | `LC-AC-01/04/06/73`, `LC-REF-018/019/071/072/074`: retain the independent baseline and deferred stream proposal; finish full reference/source/host/resource obligations only within Adrian's assessed compatibility direction. No wider host criterion or full 70-name closure follows from this checkpoint. |
| LC-GAP-03 | Configured command, stream, default input, queue selection and external routine services need an end-to-end host contract, including resource lifecycle and condition/result reporting. ADDRESS, implicit command, PULL/PUSH/QUEUE and CALL are instruction-closed on their admitted paths. CALL's approved static signed Level B/G boundary and unchanged linker/VM remain in force. | `LC-AC-06/04`, `LC-REF-003/015–020`: distinguish missing host APIs from closed instruction behavior; implement or explicitly disposition each required adapter. General Classic/non-Classic interoperation remains outside this programme. |
| LC-GAP-04 | Invocation modes, caller trap overrides, completion classes and an externally visible variable-pool API/access window are not fully qualified. The C-string `rxvml_run()` cannot carry an embedded-NUL argument; its length-aware entry exists, but the C-string obligation is not an approved exclusion. Real host HALT production remains open. | `LC-AC-06/04`, `LC-REF-001/004–006/021–024/057/071/073`: specify and qualify required host behavior or request precise scope decisions; preserve the existing VM/linker approval boundary. |
| LC-GAP-05 | `LC-HOST-ADDRESS-NUL` remains an approved instruction-level FAILURE diagnostic when the command host path cannot represent NUL; length-aware delivery is still a separate host-interface obligation. TRACE's practical divergences are agreed, and displayed scalar values must remain correct. PARSE EXTERNAL/NUMERIC are outside Adrian's initial Level C scope as mainframe-specific sources; their final Release 1 disposition is not yet recorded. | `LC-AC-06/04`, `LC-REF-015/018/070`: decide the remaining host-transport and final PARSE-source dispositions without reopening the closed ADDRESS/TRACE/PARSE reviews merely to gather more evidence. |
| LC-GAP-06 | Source and diagnostic equivalence need a complete scanner, encoded-source and line-identity review: a physical Level C file with embedded NUL currently compiles only the prefix silently (reproducer in `LC-STEP-87A`). Contextual symbols, numeric/quoted/radix literals, limits, reserved state, `.MN`, error catalog and source traceback remain cross-cutting. | `LC-AC-01/04/08/72`, `LC-REF-002/007/025–036/042/052/072`: repair silent source truncation independently of parked INTERPRET, then qualify the remaining source/diagnostic matrix or seek exact exceptions. |
| LC-GAP-07 | Expression semantics still need whole-family audit: power association, arithmetic precedence and numeric errors, normal/strict comparison, logical operand errors, concatenation and configuration effects across optimized/no-opt execution. `NUMERIC` as an instruction is closed; that does not close all expression consumers. | `LC-AC-04/72/73`, `LC-REF-043–046/053/054`: reconcile specific failing forms against the reference and qualify one shared expression path. |
| LC-GAP-08 | The Unicode-first scalar route and fixed Latin-1 ordinal bridge have substantial passing slices, but complete source, BIF, host and cross-consumer proof is open. `RexxValue` binary capability remains for RexxScript. Explicit Unicode BIF names/codecs and the later raw-binary boundary still require their own design decision. | `LC-AC-06/72/73/75`, `LC-REF-008–014`: complete whole-program proof; decide whether the new Unicode BIF design belongs to this Level C closure or a separately approved later scope. No implicit raw-byte behavior. |
| LC-GAP-09 | AST ownership/provenance, duplicate lowering paths and the earlier-instruction baseline have many retained receipts but no final complete crosswalk. Some coverage and reference rows still describe pre-closure slices; this register corrects the active status, not every historical receipt. | `LC-AC-08/58/61`: inspect current parser-to-emitter paths and remove real duplication or stale claims. Reserve a fresh independent consistency review for the new session after closeout. |
| LC-GAP-10 | Exact-head full compatibility and Release 1 qualification are unproved. The latest normal Level C Debug/Release 741/741 checkpoints qualify TRACE code/test inputs, not the outstanding host/BIF/source/expression surface or Beta 4 packaging/platform matrix. | `LC-AC-01/04/59`, `R1-AC-01/02`: after dispositions and product changes, run the smallest relevant grouped checks and the required candidate qualification once on final inputs; do not repeat unchanged gates per item. |

1. **LC-STEP-89A (`LC-AC-01/04/06/08/58/59/61/72/73/75`; complete 2026-10-07):** reconcile the active instruction queue, current BIF entries, accepted boundaries and open cross-cutting criteria into `LC-GAP-01–10` without treating a closed instruction as full compatibility.
2. **LC-STEP-89B (`R1-AC-01/02`; pending Adrian's decisions; depends on 89A):** record each chosen item as required work, a specifically approved and documented "won't implement" exception, or an explicitly open Release 1 blocker. Parking alone never approves an exception.
3. **LC-STEP-89C (`LC-AC-01/04/06/08/58/59/61/72/73/75`; pending 89B where scope decisions are needed):** close the agreed product gaps in coherent family-sized increments; run focused tests during implementation and one relevant grouped normal correctness checkpoint on the final changed inputs. Keep required host/platform proof open until run.
4. **LC-STEP-89D (`LC-AC-01/04/06/08/58/59/61/72/73/75`, `R1-AC-01/02`; pending 89C):** hand the exact checkout, dispositions, open criteria and retained evidence to a new session for a fresh full consistency review before any overall Level C or Release 1 completion claim.

**2026-10-06 qualification cadence revision.** Adrian directed thicker
instruction increments and one broad regression/sanitizer checkpoint across
several completed instruction implementations instead of repeatedly running
the entire matrix for each one. Finish each whole-instruction source-form,
error and runtime review with focused normal Debug tests and a coherent
commit; retain any broad or sanitizer criterion as visibly open until the
next grouped checkpoint passes on its actual final inputs. This changes
test scheduling, not Level C acceptance or instruction scope. The EXIT
review is the first increment under this cadence after one broad run had
already started. Do not repeat that broad run solely to replace an
identified stale prebuilt host fixture or reprove unaffected tests.

**2026-10-05 scope and VM decision.** Adrian initially excluded a general
interface between Classic and non-Classic Rexx from this programme, then
allowed Level C CALL to reach a Level B/G routine with a specific signature.
`LC-AC-71` therefore closes on the complete ARG instruction behavior for
admitted Classic activations; future Classic external CALL/INTERPRET forms
must use the same frame when their own instruction reviews reach them. The
larger `RexxStart`-like host invocation-mode proposal `73G.3` is separate
host work and does not gate ARG. Preserve the C-string `rxvml_run()` embedded-
NUL obligation in host-interface tracking; the already qualified length-aware
entry supplies the exact-length path. In response to an explicit VM review,
Adrian approved retaining the `signalorigin` RXAS/VM operation from
`d809ab8da` and dedicated VM `CLASSIC_CONDITION` signal 29 from `2a7ebf657`
as implementations of the approved one-frame SIGNAL direction. These are VM
contract changes, not merely compiler AST nodes. Any further architectural
shift still requires its own decision before a product edit.

**2026-10-06 CALL implementation decision.** Adrian accepted the fixed
`.void(frame=.RexxActivationArguments)` Level B/G signature and directed that
Level C use the ordinary Level B/G compiler signature check (with a Level C
diagnostic), linker and runtime behavior. Level C is a compiler compatibility
layer at this boundary. Do not edit `rxlink`, the VM, its instruction set, or
its loader for CALL without a further explicit decision. The prior
`metacheckproc`, dynamic pointer selection and special host resolver proposals
below are historical and superseded. Generate ordinary typed imports and
calls for external targets; separately compiled Level C providers need a
compiler-generated exposed entry with this signature, unique helper symbols
and no competing `.main`. Preserve the one Classic activation frame,
Unicode text boundary and optional result protocol. The compiler-only
missing-target path retains reached-only 43.1 for a target absent at caller
compilation. The later static-boundary decision below governs providers added
after compilation or omitted from the linked image.

**2026-10-06 CALL static-boundary scope decision.** Adrian accepted the
static signed boundary for Level C CALL. A provider must be visible to the
compiler when its caller is compiled and must be included in the linked
image. A provider added only after caller compilation is outside the Level C
CALL search contract; recompile the caller to bind it. If a provider that was
available at compilation is omitted from the image, retain the ordinary
core `FUNCTION_NOT_FOUND` behavior, including its existing source location
for a direct CALL or delayed external handler. This is an explicit departure
from Classic late runtime lookup and its error identity/timing, limited to
that static provider boundary. It does not change reached-only 43.1 for a
target absent at caller compilation, the 16.1 causing-clause behavior for an
absent local/delayed handler, frame/result semantics, or the remaining CALL
criteria. No linker or runtime edit is authorized by this decision.

Dated plans and receipts below preserve the state and proposals at their own
checkpoints. The status above, the whole-instruction queue and the current
coverage rows supersede their old "next", profile and bounded-slice wording.

## Vision and intended outcome

Compile and execute Classic REXX through `rxc`, `rxas`, `rxlink`, and `rxvm`
with the syntax, scalar and numeric rules, variable-pool behavior, control
flow, routines, built-in functions, conditions, diagnostics, source services,
and host interfaces described in the [compliance reference](../../../compiler/docs/levelc_compliance_reference.md)
and [BIF reference](../../../compiler/docs/levelc_classic_bifs.md).
Keep the approved Unicode-scalar/Latin-1-ordinal compatibility boundary,
source provenance, Classic error identities where applicable, and fail-closed
handling for shapes not yet proved.
Preserve Level B behavior and the separate RexxScript sandbox. An exclusion
counts only after Adrian individually approves its reason, user-visible
behavior, and documentation; unfinished work remains open.

The [Level C architecture design](../../../compiler/docs/levelc_working_architecture.md)
records the common `rxfnsc` value, variable-pool and BIF foundation with
RexxScript. Adrian confirmed this shared foundation on 2026-10-03 while
retaining the two products' distinct language and sandbox contracts.

**Approved 2026-10-04 character-model revision.** Compiled Level C scalar
strings are valid Unicode text, stored as Level B `.string` values and indexed
by codepoint. There is one ordinary character route: SAY emits text, PARSE and
character BIFs use codepoints, and no implicit BYTE/UTF8 profile selection is
required. A fixed Latin-1 ordinal bridge maps every byte `00`–`FF` to the
same-numbered Unicode scalar `U+0000`–`U+00FF`. Byte-valued Classic conversion
and bitwise BIFs reverse that bridge and signal when a required scalar exceeds
`U+00FF`; they must not silently encode a scalar as UTF8 bytes. Thus
`X2C('FF')` is `U+00FF`, `C2X(X2C('FF'))` is `FF`, and text output of that value
uses the selected host text encoding rather than promising a raw `FF` byte.
Ordinary character operations may accept all Unicode scalars. Explicit
Unicode BIFs/codecs will be designed in a later approved language step; raw
binary values and I/O are later, separately scoped facilities. Preserve the
shared `RexxValue` class, its binary storage and numeric caches for RexxScript
and future APIs; Level C's visible scalar contract selects valid text without
removing those capabilities. RexxScript retains its own evaluator and sandbox.
This is an approved departure from byte-exact Classic behavior, including
UTF8-source byte positions and host raw-byte output; reference comparisons
must label those differences rather than call them parity. The former
BYTE-default/opt-in-UTF8 profile proposal and LC-STEP-73H implementation route
are superseded. Existing profile tests remain historical regression evidence
until their replacements are qualified. `LC-AC-04/06/57/59/71` and the
instruction receipts below must be assessed against this revision.

Historical delivery sequence: the first increments were a coverage inventory,
then executable `IF/THEN/ELSE`, then simple `DO ... END`. They did not complete the Beta 4
contract. Later increments are selected from the open coverage rows rather
than redefining compatibility around the first slices.
Adrian prioritized closing the high-risk AST tree-manipulation path early on
2026-10-03. Structural lowering and its invariant evidence now precede broad
BIF and host-service expansion; the full compatibility contract is unchanged.
After the four-item PARSE probe, Adrian rejected target-count-specific
lowering as the lasting design. `LC-AC-50` removes that arbitrary boundary for
direct word/dot templates before work resumes on positions and patterns.
The subsequent variable-list increments put symbol resolution and indirect-list
execution in the shared Classic pool. Level C preserves source order and
captures the value of a parenthesized reference at its position; the runtime
interprets that subsidiary list. This keeps variable semantics available
to RexxScript without sharing the two products' statement parsers.

On 2026-10-03 Adrian changed the delivery unit: future implementation is
planned by **whole instruction at minimum**, with cross-cutting value,
variable, expression, BIF and lifecycle foundations planned together where
they serve several instructions. Fine-grained cases remain regression tests;
they are not independent feature-completion claims. An instruction stays open
until its valid forms, errors, nested contexts, source/evaluation order,
configuration and relevant host lifecycle are proved or a specific exception
is approved. A reviewable instruction may require several commits, but its
acceptance contract is set before those commits. Historical bounded criteria
below retain their evidence and IDs; their checkmarks do not close a whole
instruction. The [implementation review](../../../compiler/docs/levelc_working_architecture.md#2026-10-03-implementation-review-simplify-before-expansion)
records the simplification candidates and approval gates.
The instruction queue below is strict: one active instruction at a time.
Inventory its complete grammar/reference contract, review and simplify its
implementation, finish the missing forms, qualify it, close it, then start
the next row. Significant work, missing infrastructure or an awkward
implementation is not infeasibility and stays in scope. Reserve an
"infeasible" finding for a fundamental language feature or capability that
cannot be delivered without a serious compromise elsewhere. Record the
conflict, alternatives, affected behavior and evidence; the instruction
remains open until Adrian decides and approves the specific disposition.
Cross-cutting repairs are made when required by the active instruction and
are verified against all affected consumers.
On 2026-10-05 the LC-I-11 ARG invocation audit identified the per-label
procedure partition as a shared blocker for Classic labels without
`PROCEDURE`. The approved LC-STEP-63T frame/label architecture was therefore
an ARG/PROCEDURE/CALL dependency. Its one-body implementation enabled the
later ARG closure; it does not mark LC-I-23 SIGNAL complete or close the
PROCEDURE/CALL rows.

## Acceptance criteria

- [ ] **LC-AC-01 — complete inventory (R1-AC-01):** every statement,
  expression, BIF, numeric context, variable-pool/stem, procedure/call,
  PARSE, condition/signal, source/TRACE, and host-interface feature in the
  references has a row with implementation state, evidence or open work, and
  a platform/configuration boundary. Proposed exceptions remain open until
  individually approved. Verify by reference-to-matrix reconciliation.
- [x] **LC-AC-02 — IF execution:** supported `IF expression THEN instruction
  [ELSE instruction]` evaluates the condition once, selects the correct arm,
  binds `ELSE` correctly in nested forms, and enforces Classic exact logical
  values. Unsupported branch statements still fail closed. Verify with
  Classic reference output/error comparisons, positive and negative CTests,
  opt/no-opt runs, lowered-tree inspection, and `rxc`/`rxas`/`rxlink`/`rxvm`
  execution.
- [x] **LC-AC-03 — simple DO execution:** supported `DO ... END` executes its
  body once with correct statement order and nested `IF`/`DO` behavior;
  broader controlled/repetitive forms and named transfers remain open until
  their semantics are separately proved. Verify with Classic reference comparisons,
  positive and negative CTests, opt/no-opt, lowered-tree inspection, and the
  full toolchain.
- [x] **LC-AC-05 — NOP execution:** a childless Classic `NOP` has no visible
  effect in main, local procedures, and supported `IF`/`DO` bodies; adjacent
  statements retain their order and unsupported statement shapes still fail
  closed. Verify against the Classic interpreter, focused optimized/no-opt
  runs, the relevant normal Level C suite, and linked-image execution.
- [ ] **LC-AC-06 — shared runtime contract:** compiled Level C and RexxScript
  use the same `rxfnsc` `RexxValue`, `RexxVariablePool`, and overlapping Classic
  BIF implementations with their own caller contexts. Verify shared BIF
  argument/error behavior from both products, RexxScript sandbox isolation,
  Level C visible-pool behavior, and Unicode/Latin-1 conversion boundaries with
  focused cross-consumer integration evidence. RexxScript's string-oriented
  evaluator and allow-list remain product-specific.
- [ ] **LC-AC-08 — AST lowering closure:** every parsed Level C instruction,
  expression and program-structure shape is explicitly mapped to canonical
  lowering, a source-level diagnostic, or a visibly open feature row. Accepted
  rewrites preserve parent/sibling ownership, generated scope and symbol
  validity, evaluation order, source anchors, and opt/no-opt behavior, with no
  Level C-only nodes surviving the lowering boundary. Verify with a parser-node
  crosswalk, structural validation, nested target-tree probes, focused runtime
  equivalence and the normal compiler suite. Keep this criterion open until
  the structural shape families are closed; a first verifier is only a gate.
- [x] **LC-AC-09 — bounded SELECT execution:** parsed `SELECT`/ordered `WHEN`
  clauses and an optional `OTHERWISE` lower to a lazy canonical branch chain.
  Only the selected arm executes; later conditions are skipped, nesting and
  local procedures retain source order, a nonlogical `WHEN` reports `34.2`,
  and an unmatched SELECT without OTHERWISE reports `7.3` with its source
  line. Verify against the Classic reference for successful paths, optimized
  and no-opt execution, negative runtime checks, lowered-tree inspection,
  the normal correctness suite, and linked-image execution.
- [x] **LC-AC-10 — literal counted DO slice:** a parsed `DO` with one
  non-negative integer literal repetition count executes its supported body
  exactly that many times, including zero and nested groups, with opt/no-opt
  parity and valid lowered ownership/source anchors. Dynamic counts, controlled
  variables, conditions, `LEAVE` and `ITERATE` remain open. Verify against
  Regina, a targeted AST probe, focused execution and the normal Level C
  suite, then commit this increment separately.
- [x] **LC-AC-11 — loop transfer through generated AST blocks:** childless
  `LEAVE` and `ITERATE` in a supported literal counted `DO` target the nearest
  source repetitive loop even when nested in Classic simple DO, IF or SELECT
  structures. Generated branch/group wrappers must not capture those
  transfers; named transfer forms remain open. Verify a Regina-versus-canonical
  counterexample, nested opt/no-opt behavior, target-tree association, normal
  compiler regressions and linked execution.
- [x] **LC-AC-12 — bounded DO FOREVER:** a parsed childless `REPEAT` marked
  FOREVER lowers to a canonical indefinite loop, and childless LEAVE/ITERATE
  still target that source loop across generated blocks. Verify Regina output
  for guarded nested and procedure forms, opt/no-opt parity, source anchors,
  normal correctness and linked execution. Dynamic counts, controlled
  variables, combined conditions and named transfers remain open.
- [x] **LC-AC-13 — bounded DO WHILE:** a parsed `DO WHILE expression`
  reevaluates a supported, setup-free condition before each iteration,
  executes zero times for an initially false condition, and preserves
  childless LEAVE/ITERATE binding through generated blocks. Nonlogical
  conditions report `34.3`; expressions requiring per-iteration setup have
  separate criteria. Verify Regina output, opt/no-opt and linked execution, a
  lowered-tree probe, focused normal/Debug regressions and the relevant
  correctness suite.
- [x] **LC-AC-14 — bounded DO UNTIL:** a parsed `DO UNTIL expression`
  executes its supported body before the first condition check, then checks
  after each iteration, including an ITERATE. Childless LEAVE/ITERATE keep
  their nearest source loop through generated blocks; a nonlogical condition
  reports `34.4`. Only setup-free expressions are admitted in this slice.
  Verify Regina behavior, opt/no-opt and linked execution, a lowered-tree
  probe, focused normal/Debug regressions and the relevant correctness suite.
- [x] **LC-AC-15 — literal count with condition:** parsed `DO integer
  WHILE expression` and `DO integer UNTIL expression` combine the bounded
  count with the correct entry or end check. Zero count executes no body;
  LEAVE/ITERATE retain the nearest source loop through generated blocks;
  `34.3`/`34.4` remain contextual. Accept only non-negative literal counts
  and setup-free conditions. Verify Regina, optimized/no-opt and linked
  execution, source-anchored tree shape, focused normal/Debug regressions and
  the relevant correctness suite.
- [x] **LC-AC-16 — eager Classic logical operands:** Level C `&` and `|`
  evaluate their left and right operands once each, in source order, even
  when the left result determines the Boolean answer. Side effects and
  contextual invalid-value identities `34.5`/`34.6` are preserved; existing
  Boolean results and opt/no-opt output remain stable. Verify a Regina
  side-effect reproducer, left/right operand errors, nested expressions,
  shared RexxValue behavior, linked execution and the relevant normal/Debug
  suites. Keep unrelated operator and condition families open.
- [x] **LC-AC-17 — direct WHILE with setup expressions:** a parsed direct
  `DO WHILE expression` reevaluates any supported condition whose lowering
  creates setup statements on every loop entry, preserving Classic eager operand
  order, BIF argument frames, compound-tail lookup, and exact logical errors.
  Existing childless LEAVE/ITERATE retain their source loop. Verify against
  Regina with bounded fixtures, opt/no-opt and linked execution, a canonical
  block-expression tree and source anchors, focused normal/Debug regressions
  and the relevant correctness suite. UNTIL and combined setup-bearing
  headers have separate criteria for their end-check and limit proof.
- [x] **LC-AC-18 — direct UNTIL with setup expressions:** a parsed direct
  `DO UNTIL expression` executes its body before the first condition, then
  reevaluates any supported setup-bearing condition after each ordinary or
  ITERATE path. Preserve eager operand order, BIF argument frames, source
  LEAVE/ITERATE ownership and exact `34.4` errors. Verify against Regina with
  bounded fixtures, opt/no-opt and linked execution, a source-anchored
  canonical block-expression tree, focused normal/Debug regressions and the
  relevant correctness suite. Count-plus-condition setup has a separate
  criterion for its limit proof.
- [x] **LC-AC-19 — literal count with setup-bearing condition:** parsed
  `DO integer WHILE expression` and `DO integer UNTIL expression` execute
  supported setup-bearing conditions at the respective entry/end checks,
  subject to the literal count limit. A zero count evaluates neither body nor
  condition setup; eager operand effects, BIF argument frames, source
  LEAVE/ITERATE ownership and exact `34.3`/`34.4` errors hold. Verify Regina
  timing, opt/no-opt and linked execution, source-anchored FOR plus
  BLOCK_EXPR tree shape, focused normal/Debug regressions and the relevant
  correctness suite. Dynamic counts and controlled headers remain open.
- [x] **LC-AC-20 — FOREVER with condition:** parsed `DO FOREVER WHILE` and
  `DO FOREVER UNTIL` execute with Classic entry/end check timing, including
  setup-bearing expressions. WHILE can skip its body; UNTIL executes once
  before checking. Preserve eager side effects, BIF frames, nearest-loop
  LEAVE/ITERATE ownership, local procedure scope and `34.3`/`34.4` errors.
  Verify bounded Regina output, opt/no-opt and linked execution, canonical
  REPEAT without FOR plus source-anchored condition tree, focused normal/Debug
  regressions and the relevant correctness suite.
- [x] **LC-AC-21 — bounded dynamic repetition count:** parsed direct
  `DO expression` evaluates a supported count expression once at loop entry,
  then executes its body the resulting number of times, up to the currently
  supported signed 32-bit whole-count limit. Zero skips the body, mutation of
  source variables does not change the count, BIF/local-call side effects run
  once, and childless LEAVE/ITERATE target the source loop. Invalid runtime
  counts use contextual `26.2` without silent truncation. Verify Regina for
  valid counts, opt/no-opt and linked execution, canonical FOR with a
  source-anchored setup block when needed, focused RexxValue/normal/Debug
  regressions and the relevant correctness suite. Dynamic count plus a
  condition has a separate criterion; controlled headers and wider numeric
  bounds remain open.
- [x] **LC-AC-22 — bounded dynamic count with condition:** parsed
  `DO expression WHILE expression` and `DO expression UNTIL expression`
  evaluate a supported count once, then check a supported condition at its
  Classic entry/end point within the count limit. Zero count skips condition
  setup and body; count-side effects run once, condition-side effects run at
  each required check; childless LEAVE/ITERATE keep source ownership and
  contextual `26.2`/`34.3`/`34.4` errors. Verify bounded Regina output,
  opt/no-opt and linked execution, canonical FOR plus WHILE/UNTIL with
  independent setup blocks and source anchors, focused normal/Debug tests and
  the relevant correctness suite. Controlled headers and wider count values
  remain open.
- [x] **LC-AC-23 — bounded controlled TO loop:** parsed
  `DO scalar = non-negative integer TO non-negative integer` initializes the
  visible Classic variable pool, checks the bound before each body, advances
  the current pool value by one after each ordinary or ITERATE path, and
  leaves the correct post-loop or LEAVE value. A body assignment to the
  control variable affects the next step; a nonnumeric control value reports
  contextual `41.6`. Preserve nested source LEAVE/ITERATE ownership, local
  procedure scope, and opt/no-opt behavior. Verify bounded Regina output,
  linked execution, canonical WHILE plus end-check BLOCK_EXPR tree shape,
  focused runtime/Debug tests and the relevant correctness suite. BY, FOR,
  dynamic endpoints, named transfer, and combined conditions remain open.
- [x] **LC-AC-24 — bounded controlled BY step:** parsed scalar
  `DO name = non-negative integer TO non-negative integer BY signed integer`
  accepts TO/BY in either parsed order. A negative step checks the lower
  bound and advances downward; a zero or positive step checks the upper
  bound. The visible pool variable advances after ordinary and ITERATE paths,
  preserves body mutation, and retains the correct value after normal exit
  or LEAVE. Prove guarded zero-step behavior, opposite-direction zero-entry,
  nested transfers, and opt/no-opt parity. Verify against Regina, linked
  execution, source-anchored canonical WHILE/UNTIL tree shape, focused
  normal/Debug tests and the relevant correctness suite. Dynamic BY, FOR,
  dynamic endpoints, named transfer and combined conditions remain open.
- [x] **LC-AC-25 — bounded controlled TO with FOR:** parsed scalar
  `DO name = non-negative integer TO non-negative integer [BY signed integer]
  FOR non-negative integer` applies a signed 32-bit bounded FOR limit alongside the TO
  direction check under one source loop, regardless of parsed modifier order.
  A zero FOR count initializes the visible control variable and skips the
  body; a positive limit advances the pool value after each body or ITERATE,
  including the final count-limited body. Body mutation and LEAVE retain
  Classic final values. Verify Regina output, opt/no-opt and linked execution,
  canonical FOR plus WHILE/UNTIL tree shape, focused normal/Debug checks and
  the relevant correctness suite. Dynamic FOR, FOR without TO, dynamic
  controlled endpoints, named transfer and combined conditions remain open.
- [x] **LC-AC-26 — bounded controlled FOR without TO:** parsed scalar
  `DO name = non-negative integer [BY signed integer] FOR non-negative integer`
  applies a signed 32-bit bounded repetition limit without an entry TO check.
  The visible pool variable initializes even for zero count, advances after
  each ordinary or ITERATE path, and retains the Classic final or LEAVE
  value; body mutation and both BY clause orders behave as in Regina.
  Verify bounded Regina output, opt/no-opt and linked execution, canonical
  FOR plus UNTIL/BLOCK_EXPR tree shape with no synthetic WHILE, focused
  normal/Debug tests and the relevant correctness suite. Unbounded controlled
  DO, dynamic FOR/BY/start, named transfer and combined conditions remain open.
- [x] **LC-AC-27 — named controlled-loop transfer:** named `LEAVE name` and
  `ITERATE name` resolve a case-insensitive name to the matching active,
  supported controlled DO, including an outer loop across nested controlled
  loops, simple DO groups, IF and generated branch blocks. ITERATE reaches
  that loop's end step and next entry check; LEAVE skips its end step. The
  visible pool values, skipped statements and matched outer target agree
  with Regina, while invalid names retain `28.3`/`28.4` diagnostics. Verify
  bounded Regina output, opt/no-opt and linked execution, source-anchored
  lowered target associations, focused normal/Debug tests and the relevant
  correctness suite. Named transfer to wider unsupported controlled shapes
  remains fail-closed.
- [x] **LC-AC-28 — bounded controlled WHILE entry:** parsed scalar controlled
  `DO name = literal [TO literal] [BY signed literal] [FOR literal]
  WHILE expression` with at least one TO or FOR applies a supported WHILE
  condition at each permitted entry after the FOR and TO guards. A zero FOR
  or failed TO guard does not
  evaluate WHILE or its setup; ITERATE advances the visible control variable
  before the next entry check, while LEAVE skips that advance. Preserve named
  and childless source-loop ownership, contextual `34.3`, local procedure
  scope, and opt/no-opt behavior. Verify bounded Regina output and errors,
  linked execution, a source-anchored canonical entry BLOCK_EXPR tree,
  focused Release/Debug checks and the relevant correctness suite. Controlled
  UNTIL and dynamic controlled clauses remain open.
- [x] **LC-AC-29 — bounded controlled UNTIL end check:** parsed scalar
  controlled `DO name = literal [TO literal] [BY signed literal]
  [FOR literal] UNTIL expression` with at least one TO or FOR evaluates a
  supported UNTIL condition after each executed body and on ITERATE, before
  advancing the visible control variable. A true UNTIL leaves the current
  value unadvanced; a false UNTIL advances it, including after the final
  FOR-limited body. Zero FOR and failed TO entry guards skip the condition.
  LEAVE skips both condition and step. Preserve setup timing, named and
  childless loop ownership, contextual `34.4`, local scope, and opt/no-opt
  behavior. Verify bounded Regina output and errors, linked execution,
  source-anchored canonical end BLOCK_EXPR tree, focused Release/Debug checks
  and the relevant correctness suite. Dynamic controlled clauses remain open.
- [x] **LC-AC-30 — guarded unbounded controlled DO:** parsed scalar
  `DO name = non-negative integer [BY signed integer]` with optional supported
  WHILE or UNTIL condition runs without a synthetic TO or FOR limit. The
  visible pool variable advances after ordinary and ITERATE paths, unless a
  true UNTIL or LEAVE exits before the step; WHILE checks at entry. Guarded
  fixtures prove termination, body mutation, named and childless transfers,
  local scope, final control values and opt/no-opt parity against Regina.
  Verify canonical REPEAT with no FOR and no synthetic WHILE for bare/BY
  forms, source anchors, linked execution, focused Release/Debug checks and
  the relevant correctness suite. Dynamic controlled clauses remain open.
- [x] **LC-AC-31 — captured dynamic TO endpoint:** parsed scalar controlled
  `DO name = non-negative integer TO expression` with optional signed literal
  BY, bounded literal FOR, and a supported WHILE or UNTIL condition evaluates
  a supported TO expression exactly once before assigning the new visible
  control variable, so it observes any previous value. Later mutation of
  its source variables does not change
  the endpoint; a nonnumeric endpoint reports contextual `41.4` at setup,
  including when FOR is zero. FOR-zero still evaluates the TO expression at
  loop setup as Regina does. Preserve
  nested/named transfer ownership, condition timing, source anchors and
  opt/no-opt behavior. Verify bounded Regina output and errors, a captured
  canonical value in the lowered tree, linked execution, focused
  Release/Debug checks and the relevant correctness suite. Dynamic start,
  BY and FOR remain open.
- [x] **LC-AC-32 — captured dynamic BY step:** parsed scalar controlled
  `DO name = non-negative integer [TO literal-or-expression]
  BY expression [FOR literal]` accepts a supported BY expression evaluated
  and validated exactly once before the new visible control assignment.
  Dynamic TO and BY clauses evaluate in their written order, both seeing
  any prior control value; later source mutation changes neither capture.
  A nonnumeric BY reports contextual `41.5` at setup, including FOR-zero.
  Preserve positive, negative and guarded zero steps, TO direction,
  ITERATE/LEAVE timing, optional WHILE/UNTIL, local scope and opt/no-opt
  behavior. Verify bounded Regina output and errors, hidden canonical
  captures with source order, linked execution, focused Release/Debug checks
  and the relevant correctness suite. Dynamic start and FOR remain open.
- [x] **LC-AC-33 — captured dynamic controlled FOR count:** parsed scalar
  controlled `DO name = non-negative integer [TO literal-or-expression]
  [BY signed-literal-or-expression] FOR expression` evaluates a supported
  FOR expression once in written clause order with TO/BY, before the new
  visible control assignment. The resulting non-negative whole count is
  bounded to signed 32-bit and reports contextual `26.3` for invalid or
  out-of-range values. Zero still evaluates and validates every header
  clause but skips the body and WHILE/UNTIL checks; positive counts preserve
  the final pool value and ITERATE/LEAVE timing even if source variables
  change. Verify bounded Regina output/errors, opt/no-opt, captured canonical
  FOR count and source anchors, linked execution, focused Release/Debug
  checks and the relevant correctness suite. Dynamic start and wider counts
  remain open.
- [x] **LC-AC-34 — loop-end call argument preservation:** optimized
  canonical controlled loops preserve captured by-value arguments across a
  method call and every later iteration, including when a zero-count loop
  precedes them. Calls within canonical UNTIL end checks use a disjoint
  argument window so their receiver and by-value argument slots cannot
  overwrite a value needed at the next entry. Other call affinity shapes
  retain their existing behavior. Verify the isolated
  literal-FOR/dynamic-BY counterexample in opt and no-opt, inspect emitted
  call marshalling, run the relevant normal compiler checks, and execute
  through the full toolchain. This is a general compiler correctness repair
  exposed by Level C, independent of dynamic FOR support.
- [x] **LC-AC-35 — captured dynamic controlled start:** parsed scalar
  `DO name = expression` with the already supported TO/BY/FOR and optional
  WHILE/UNTIL clauses evaluates a supported start expression exactly once
  before the other header expressions, while they still see the previous
  visible control value. It captures that result before later side effects,
  validates TO/BY/FOR in written order, then validates the captured start as
  numeric with contextual `41.6` before assigning the visible pool. Invalid
  later clauses retain their own prior error and evaluation timing. Zero FOR,
  entry/end checks, named transfer, local scope, final control value, source
  anchors and opt/no-opt behavior remain correct. Verify Regina output and
  error-side-effect order, a canonical lowered-tree probe, focused
  Release/Debug regressions, the relevant correctness suite and linked
  execution. Wider expression shapes and complete compatibility remain open.
- [x] **LC-AC-36 — empty quoted string lowering:** parsed Classic `''` and
  `""` literals lower to canonical empty string values with their source
  anchors, in SAY, assignment, concatenation and supported call contexts.
  They compile, assemble, link and execute with Regina-equivalent output in
  optimized and no-opt modes. Numeric use reports its contextual runtime
  error rather than a compile-time unsupported-shape error. Verify a minimal
  counterexample, focused positive and negative regressions, lowered-tree
  inspection, the relevant normal compiler suite and linked execution.
- [x] **LC-AC-37 — function call after a concatenated operand:** a Classic
  function call immediately followed by `(` remains a `FUNCTION` term when
  it follows another expression operand under blank concatenation, including
  a SAY literal and a variable. Its arguments, effects and result agree with
  Regina in opt/no-opt and linked execution. A blank between the function
  name and `(` continues to mean a symbol followed by a parenthesized term,
  as Regina does. Verify minimal reference counterexamples, raw and lowered
  AST shape, supported nested arguments, focused parser/runtime checks and
  the relevant normal compiler suite. Broader call syntax remains open.
- [x] **LC-AC-38 — direct ARG uppercase binding:** a supported direct
  local-procedure `ARG` binding applies Classic `PARSE UPPER ARG` conversion
  to each supplied argument before storing it in the callee's visible pool.
  The caller's value remains unchanged, scalar target order and exposure
  still work, and the shared Classic TRANSLATE implementation supplies the
  then-current Level C BYTE default mapping. Unicode uppercase behavior is
  open under LC-AC-72/74. Verify lowercase
  and mixed-case Regina
  examples, opt/no-opt and linked execution, canonical argument-frame and
  pool-set tree shape, focused shared-runtime checks and the relevant normal
  compiler suite. Wider parse templates and external-call lifecycle remain
  open.
- [x] **LC-AC-39 — single-target PARSE source and template lowering:**
  `PARSE VAR scalar target` and `PARSE VALUE expression WITH target`, each
  with one direct scalar target and optional `UPPER`, evaluate their source
  once and write the target through the Classic visible pool. The source
  variable and caller state retain their values unless the target aliases
  them. The optional uppercase path uses shared Classic TRANSLATE with the
  then-current BYTE default. Unicode source values remain under LC-AC-72.
  Other parse sources, multiple targets, patterns,
  positions and comma templates remain fail-closed. Verify Regina output,
  main/procedure and self-target behavior, opt/no-opt, raw and canonical tree
  shape, focused negatives, normal compiler checks and linked execution.
- [x] **LC-AC-40 — three-target implicit-word PARSE:** `PARSE VAR scalar
  first second tail` and `PARSE VALUE expression WITH first second tail`,
  with direct scalar targets and optional UPPER, use the existing `parsewords3`
  VM primitive through a shared `RexxValue` method. The source is evaluated
  and captured once before ordered pool writes; the first two targets receive
  words and the third receives the unparsed remaining tail, preserving blanks,
  source aliases and repeated target order. Other template counts and
  patterns remain guarded. Verify Regina across leading/repeated/trailing
  blanks, short input, aliases, opt/no-opt, canonical tree, shared-method
  behavior and linked execution, then commit separately.
- [x] **LC-AC-41 — abutted expression concatenation:** when adjacent Classic
  expression terms touch in the source, the parser emits abuttal `OP_CONCAT`
  rather than blank `OP_SCONCAT`, including literal/symbol chains and
  parenthesized or call terms whose punctuation is outside their operand AST.
  Comment-only gaps act as abuttal, while spaced terms remain `OP_SCONCAT`;
  explicit `||` retains its existing path.
  Verify minimal Regina counterexamples, raw and lowered AST, opt/no-opt,
  parser/highlighter checks, the normal Level C suite and linked execution.
  This closes `LC-FIND-04`; broader expression conformance remains open.
- [x] **LC-AC-42 — decimal fraction source tokens:** Classic constants with
  a decimal point and digits on at least one side (`1.5`, `1.`, `.5`) form one
  source numeric AST operand and retain their exact spelling as a `RexxValue`.
  They work in ordinary expressions, PARSE VALUE expressions and arithmetic;
  fractional DO/FOR counts still raise their existing contextual errors.
  Verify Regina output, raw/canonical tree, opt/no-opt, focused invalid-count
  tests, the normal Level C suite and linked execution. Exponents are covered
  by `LC-AC-43`; digit-starting constant symbols are covered by `LC-AC-44`.
- [x] **LC-AC-43 — exponent numeric source tokens:** valid Classic numeric
  exponent forms with integer, trailing-dot or fractional mantissas and an
  optional exponent sign form one numeric source operand. Their displayed
  constant spelling uses uppercase `E`; arithmetic and PARSE VALUE consume
  the same value, and whole-number DO/FOR checks retain contextual behavior.
  Verify Regina and a failing baseline, raw/canonical tree, opt/no-opt,
  focused count errors, the normal Level C suite and linked execution.
  Digit-starting nonnumeric constants are covered by `LC-AC-44`;
  leading-period symbols remain open under `LC-AC-04/08`.
- [x] **LC-AC-44 — digit-starting constant symbols:** nonnumeric Classic
  constants beginning with a digit, including letter and repeated-period
  forms, remain one source `CONST_SYMBOL` operand. Their value is the uppercase
  source spelling, independent of the variable pool; expressions, PARSE VALUE
  and adjacent concatenation preserve it. Numeric constants retain the
  `INTEGER`/`DECIMAL` path, and leading-period symbols remain separate work.
  Verify Regina, raw/canonical tree, opt/no-opt, parser/highlighter checks,
  the normal Level C suite and linked execution.
- [x] **LC-AC-45 — two-target implicit-word PARSE:** `PARSE VAR scalar
  first rest` and `PARSE VALUE expression WITH first rest`, with optional
  UPPER and direct scalar targets, capture the source once before any pool
  writes. The first target receives the first blank-delimited word; the
  second receives the remaining source after exactly one separator, retaining
  further blanks. Source aliases and repeated targets follow authored write
  order. Verify Regina across leading/repeated/trailing blanks, short input,
  aliasing, local scope, shared value method, opt/no-opt, raw/canonical tree,
  normal Level C suite and linked execution. Other template shapes remain
  guarded; broader Unicode proof remains under `LC-AC-04/72`.
- [x] **LC-AC-46 — three words with final PARSE drop:** `PARSE VAR scalar
  first second third .` and the corresponding `PARSE VALUE expression WITH`
  form, with optional UPPER, use the existing `parsewords3d` primitive through
  shared `RexxValue`. The three scalar targets receive the first three words
  while the final dot discards the remaining template value; no dot binding
  reaches the pool. Source capture precedes ordered writes. Verify Regina
  across whitespace, short input, source aliases, repeated targets, local
  scope, shared method opt/no-opt, raw/canonical tree, normal Level C suite
  and linked execution. Other dot placements were carried to `LC-AC-47`;
  wider templates remain guarded.
- [x] **LC-AC-47 — bounded PARSE dot placeholders:** two- and three-item
  `PARSE VAR`/`PARSE VALUE` templates containing one or more `.` placeholders
  and at least one direct scalar target use the already shared word/tail split
  methods. Each dot consumes its corresponding word or tail without a pool
  write; scalar targets retain authored write order. The source is captured
  once before any writes, with optional UPPER through shared TRANSLATE.
  Verify Regina for leading, internal, and final dots, whitespace, short
  input, aliases/repeated targets, one-time VALUE evaluation, local scope,
  opt/no-opt, raw/canonical AST, normal Level C and linked execution. Four-item
  templates with internal dots were carried to `LC-AC-49`; other template
  shapes remain guarded.
- [x] **LC-AC-48 — all-dot direct PARSE templates:** one-, two- and
  three-item `PARSE VAR`/`PARSE VALUE` templates consisting only of `.`
  placeholders consume the source without any visible pool assignment.
  A VALUE source expression executes exactly once, including when its result
  is otherwise unused; optional UPPER and local procedure scope preserve the
  same source evaluation boundary. Verify Regina side effects and unchanged
  pool values, opt/no-opt, raw/canonical AST with no dot pool binding, focused
  negatives, normal Level C and linked execution. Four-item all-dot forms
  were carried to `LC-AC-49`; wider templates remain guarded.
- [x] **LC-AC-49 — four-item direct PARSE templates:** `PARSE VAR` and
  `PARSE VALUE` with four direct scalar/dot items consume the first three
  words and assign the remaining tail to a fourth scalar, or discard it when
  the fourth item is a dot. Dot placeholders at earlier positions suppress
  only their corresponding pool writes. A shared `RexxValue` method composes
  existing `parsewords3` splitting for a fourth scalar; the established
  `parsewords3d` method handles a final dot. Preserve source capture before
  ordered writes and optional UPPER. Verify Regina whitespace, short input,
  aliases/repeated targets, once-evaluated VALUE, local scope, shared method
  opt/no-opt, raw/canonical tree, normal Level C, RexxScript and linked
  execution. Five-item and pattern templates remain guarded.
- [x] **LC-AC-50 — generic direct word/dot PARSE templates:** `PARSE VAR`
  and `PARSE VALUE` accept a nonempty template of any representable number of
  direct scalar or `.` items, with optional UPPER. A shared `RexxValue`
  operation returns one result per item: the first `n-1` results are words
  and the final result is the unparsed tail; dots suppress only their pool
  writes. The source and result vector are captured before scalar writes,
  preserving aliases, repeated targets, local scope and once-only VALUE
  effects. The compiler has no target-count dispatch or arbitrary four-item
  guard. Verify Regina at counts 1-4 and beyond four, whitespace, short
  input, dot positions, opt/no-opt, shared runtime and RexxScript behavior,
  raw/canonical AST, the normal Level C suite and linked execution. Patterns,
  positions, comma templates and other source forms remain open under their
  own criteria.
- [x] **LC-AC-51 — static mixed PARSE template plan:** `PARSE VAR` and
  `PARSE VALUE` accept a nonempty single template with any representable
  sequence of direct scalar/dot targets, literal patterns, and literal
  absolute/relative positions. The lowerer validates the parsed AST and
  serializes its items to the VM's frozen `parseplan` descriptor; no template
  length or item-order dispatch governs acceptance. One source snapshot and
  one plan execution precede ordered visible pool writes. Dot targets
  consume fields without writes; aliases, repeated targets, UPPER, local
  scope and once-only VALUE effects retain their established behavior.
  Direct word-only templates may keep their shared generic fast path.
  Verify literal/position interaction, leading/trailing/repeated delimiters,
  absent patterns, backward/zero/out-of-range positions, ASCII/UTF8 boundaries,
  opt/no-opt, raw/canonical AST, Regina equivalence, the normal Level C suite
  and linked execution. Dynamic pattern/position operands, comma templates
  and other source forms remain open for subsequent steps.
- [x] **LC-AC-52 — backward PARSE plan cursor repair:** a frozen `parseplan`
  relative negative control leaves the next target at the moved cursor after
  storing the preceding field, matching Regina and preserving the existing
  Level B exit and Level C AST paths. Verify the minimal `3 a +2 b -1 c`
  counterexample in both products, focused opt/no-opt VM and exit tests,
  unchanged forward/absolute controls, the relevant normal correctness
  suite, and linked execution. This repairs `LC-FIND-06`.
- [x] **LC-AC-53 — direct stem and compound DROP:** a direct Classic `DROP`
  list accepts scalar, stem and supported compound symbols in authored order.
  Compound tail components are evaluated once against the visible pool,
  preserving substituted text case, before the selected tail is dropped; a
  stem drop removes its default and tails, while an individual compound drop
  reads as its expanded symbol even when the stem has a default. Exposed stems
  affect their parent binding and local unexposed stems remain local. The
  shared stem and pool classes own dropped-tail and default lifecycle, while
  the compiler emits canonical pool calls in source order. Verify Regina for
  tail substitution, dropped-tail reads, stem reset, reassignments, list order
  and scope; focused pool tests, opt/no-opt, raw/canonical AST, normal Level C
  and RexxScript regressions, and linked execution.
  Parenthesized indirect `DROP` was subsequently completed under `LC-AC-56`.
- [x] **LC-AC-54 — shared Classic stem lifecycle:** assigning a stem default
  replaces prior explicit and dropped tails; dropping one tail records a
  tombstone that reads as the expanded symbol until reassigned or reset by a
  stem assignment. Tail substitution preserves the substituted value's case
  for lookup and unresolved display. The shared `RexxStem` and
  `RexxVariablePool` implement these rules for both Level C and RexxScript.
  Verify minimal Regina counterexamples, focused runtime tests, optimized and
  no-opt execution, relevant cross-consumer regressions and linked delivery.
- [x] **LC-AC-55 — shared one-symbol DROP operation:** for a validated Classic
  variable symbol, the shared pool resolves a compound tail once against the
  current visible bindings, then drops the resulting scalar, entire stem, or
  one case-preserved tail. It preserves exposure and dropped-tail semantics.
  Verify Regina examples, focused optimized/no-opt pool regressions, linked
  execution and unchanged Level C/RexxScript consumers.
- [x] **LC-AC-56 — indirect DROP subsidiary lists:** a parenthesized `DROP`
  reference reads its variable at that point in the authored list, splits its
  value into subsidiary variable names and drops those names from left to
  right through the shared pool, including scalar, stem and compound items.
  Invalid words are ignored while later valid words still execute, following
  Regina as Adrian selected on 2026-10-03. Direct items before and after the
  reference retain their order. Verify source/canonical AST shape, Regina for
  valid and invalid lists and scope, focused opt/no-opt, normal correctness
  and linked execution.
- [x] **LC-AC-57 — complete SAY instruction:** `SAY [expression]` evaluates an
  expression once when supplied and writes its string value, or an empty line
  when omitted, through the configured default output. Main, nested and local
  procedure contexts preserve output order, source anchors and relevant
  output/error lifecycle. Verify the grammar's expression and childless forms,
  Regina output, side-effect order, opt/no-opt, AST, normal correctness,
  linked delivery and configured host output. The already tested childless
  fixture is one case in this instruction contract, not a separate slice.
  The 2026-10-04 Unicode-first decision reopens output qualification: prove
  mapped `U+0080`–`U+00FF`, non-Latin-1 text, embedded NUL, host text encoding
  and errors without claiming raw-byte output. Earlier expression, ordering,
  callback and AST evidence remains valid for its tested inputs.
- [ ] **LC-AC-58 — simpler Level C implementation:** the active lowering has
  one context-aware instruction dispatch, one reviewed variable-operation
  ownership boundary, and one general PARSE execution path. DO header
  validation and lowering share a checked representation; a dedicated AST
  node is used only if its total validation/emitter cost is lower than the
  canonical rewrite. Preserve evaluation order, source anchors, scopes,
  loop associations, errors, opt/no-opt behavior and RexxScript isolation.
  Verify a before/after code-path inventory, focused structural invariants,
  retained functional corpus, linked execution and cross-consumer tests.
  Specific architecture changes require Adrian's approval before code edits.
- [ ] **LC-AC-59 — instruction-level delivery:** each `LC-I-*` row in the
  instruction programme below is individually closed only after its complete
  reference and parser-form matrix, diagnostics, interaction/lifecycle cases,
  relevant Unicode/Latin-1 and host evidence, optimized/no-opt parity, and full
  toolchain proof are recorded, or a specific exception is approved. Preserve
  each existing bounded test as regression evidence. Verify against the
  compliance reference, parser grammar, reference-obligation appendix,
  tests and exact candidate revision.
- [x] **LC-AC-60 — one length-aware SAY output callback:** the VM, RXVML and
  RXPA expose only a `(const char *, size_t)` custom SAY callback. The compiler
  exit bridge and in-tree hosts use it; the terminated-text callback and its
  legacy-only test are removed. Default and custom output preserve embedded
  NUL and SAY's newline, while output failures retain their signal identity.
  Verify symbol/call-site inventory, focused host and console tests, both VM
  modes, core build and updated public documentation. Adrian approved removing
  the old callback API on 2026-10-04; this is a native host ABI change.
- [ ] **LC-AC-61 — reviewed prior-instruction baseline:** before SIGNAL work,
  audit every Level C instruction with earlier implementation work against its
  full parser/reference forms, shared foundations, AST/ownership and emitted
  behavior. Remove avoidable duplicate or case-specific paths; retain focused
  regression coverage. Close an instruction only after its complete contract
  and normal/toolchain evidence are recorded, and list any remaining work
  explicitly. Verify the per-instruction receipts and code-path inventory;
  significant work does not count as an infeasible exception.
- [x] **LC-AC-62 — complete DROP instruction:** every parsed direct or
  parenthesized DROP list item executes in authored order in main, nested and
  local contexts. Direct scalar, stem and arbitrary-component compound names
  use the same shared pool operation as indirect subsidiary words; exposed
  aliases, substitution, dropped-tail/default lifecycle and source anchors
  agree with Regina. Invalid subsidiary words are ignored as Adrian chose;
  invalid source forms retain their compiler diagnostics. Verify multi-part
  and mixed-list Regina probes, optimized/no-opt, source/canonical AST,
  focused pool and RexxScript checks, normal Level C correctness and linked
  toolchain execution. No fixed tail-component or list-length limit remains.
  The 2026-10-04 revision reopens the Unicode subsidiary-word, configured
  blank and symbol-classification cases; prior pool-order evidence is retained.
- [x] **LC-AC-63 — complete assignment instruction:** every parsed valid
  scalar, stem and arbitrary-component compound target takes its Classic
  value in main, nested and local contexts. Compound name substitution follows
  evaluation of the right-hand expression, including a call that mutates a
  tail component. The shared pool owns substitution and final classification,
  stem reset and exposure behavior. An omitted expression assigns the empty
  string, as in Regina and IBM's VM dialect; source-invalid targets retain their
  diagnostics. Verify against Regina, optimized/no-opt and source/canonical
  trees, focused pool and RexxScript checks, normal Level C correctness and
  linked toolchain output, with Unicode values where relevant. The 2026-10-04
  revision reopens Unicode scalar and compound-tail proof while retaining
  prior evaluation-order and pool ownership evidence.
- [x] **LC-AC-64 — complete NOP instruction:** a childless Classic `NOP`
  preserves adjacent statement order and has no visible effect in main,
  selected IF/SELECT arms, DO bodies and local procedures. Text after `NOP`
  in the same clause reports the Classic `21.1` syntax identity instead of
  being silently discarded. Verify Regina positive and negative behavior,
  source/canonical trees, optimized/no-opt, normal Level C correctness and
  linked execution. Shared label/TRACE/condition lifecycle stays explicitly
  open under `LC-AC-04/08` and `LC-I-23/24`, as for SAY.
- [x] **LC-AC-65 — complete DO instruction:** simple grouping and every valid
  counted, FOREVER and controlled loop form execute with Classic setup,
  entry, body, end-check and step order in main, nested and local contexts.
  Scalar and compound control names use the shared variable pool, including
  substitution again when a tail variable changes during a loop. TO, BY and
  FOR modifiers are accepted in their legal orders and evaluated once in
  source order; WHILE and UNTIL retain their distinct check points. Counts
  have no compiler-imposed 32-bit bound; reference-dialect limits and actual
  numeric representation limits must be characterized rather than silently
  treated as a language rule. END name, LEAVE/ITERATE associations, invalid
  forms and Classic error identities are checked. Verify with source and
  canonical AST ownership/association inspections, Regina/IBM reference
  cases, optimized/no-opt and normal correctness, focused shared-runtime
  checks, and linked toolchain execution. Shared labels, traps, TRACE and
  host configuration remain open under their own criteria.
- [x] **LC-AC-66 — complete OPTIONS instruction:** a leading source directive
  selects the language and file-level scanner/parser options over a CLI
  default, with documented defaults and clear handling of conflicting words.
  Every executable `OPTIONS` form evaluates its Classic expression once at its
  source position in main, selected/nested arms and local routines, passing
  the exact-length value to the shared processor service. The approved
  Unicode-first policy recognizes no runtime words, so the service
  ignores the value without scanning it; if runtime words are later added,
  that service must process uppercase blank-delimited words in order and ignore
  unknown words. Header words must survive
  as source-owned AST input even though canonical Level B imports and defaults
  are generated. Empty operand, IBM DBCS-specific words and the boundary
  between compile-time and runtime words require the LC-STEP-67B decision
  before implementation. Verify source and canonical AST, source/CLI and
  comment/numeric option cases, dynamic expression side effects, invalid
  forms and recovery, optimized/no-opt, relevant normal Level C and Level B
  regressions, the approved Unicode character model, and linked execution.
- [x] **LC-AC-67 — complete IF instruction:** `IF expression THEN instruction
  [ELSE instruction]` retains the Classic clause and nearest-eligible-ELSE
  rules in main, nested DO/SELECT, and local routines. It evaluates the
  condition once before either arm, accepts exactly logical `0` or `1` with
  the correct Classic error and source location otherwise, executes only the
  selected arm, and preserves source ownership through canonical lowering.
  Every legal instruction form can occupy an arm without an IF-specific
  restriction; an arm whose instruction has its own open `LC-I-*` row remains
  explicitly open under that row rather than being claimed as delivered by
  IF. Missing condition, THEN or arm, misplaced delimiter/label, stray ELSE,
  and nested recovery must follow Classic clause rules and produce the
  applicable diagnostic. Verify
  with an IBM/Regina reference matrix, source and lowered-tree inspection,
  focused positive and negative CTests, optimized/no-opt execution, relevant
  normal correctness and shared runtime checks, and linked execution.
- [x] **LC-AC-68 — complete SELECT instruction:** `SELECT` contains one or more
  ordered `WHEN expression THEN instruction` arms and an optional `OTHERWISE`
  instruction list, ending with `END`. Evaluate each condition at most once in
  source order until the first exact logical `1`; skip later conditions and
  unchosen arms. Execute the chosen instruction or complete OTHERWISE list in
  main, nested DO/IF/SELECT and local routine contexts. Raise the Classic
  logical-value error at the active WHEN and `7.3` at the SELECT when no arm
  matches without OTHERWISE. Preserve source ownership through lowering and
  normal statement dispatch for all legal arm instructions; instruction-owned
  open behavior stays with its own row. Diagnose missing/stray WHEN, THEN,
  arm, OTHERWISE/END placement, named END, and malformed nesting with Classic
  clause identities and recovery. Verify IBM/Regina reference cases, source
  and canonical AST, focused valid/invalid tests, optimized/no-opt execution,
  relevant normal correctness and shared runtime checks, and linked toolchain
  execution. Do not close the criterion with a SELECT-specific restriction.
- [x] **LC-AC-69 — complete LEAVE instruction:** childless `LEAVE` exits the
  innermost active repetitive DO and named `LEAVE symbol` exits the innermost
  active controlled DO whose authored control symbol matches, ignoring case
  but without compound-tail substitution. Simple DO, IF and SELECT wrappers
  do not change the target; leaving nested loops skips their remaining
  bodies, end checks and steps and preserves each visible control value at
  the point of transfer. Local routine calls cannot leave an inactive caller
  loop. Ordinary compiled Level C diagnoses a source-provable invalid loop
  target at compile time (`28.1` or `28.3`). With leading static `OPTIONS
  LEVELC STRICTC`, a syntactically valid but invalid-target LEAVE in an
  unselected branch compiles and does not signal; reaching it raises the
  contextual `28.1` or `28.3` at runtime. Level B/G retain their existing
  compile-time checks. Diagnose malformed names and extra same-clause text
  at compile time with Classic identity and source position. Preserve source/canonical AST ownership
  and one shared transfer/binding path without a LEAVE-specific loop rewrite.
  Verify IBM/Regina reference cases, optimized/no-opt, main/nested/local
  output, invalid-source fixtures, source and canonical trees, relevant normal
  Level C and shared runtime checks, and linked toolchain output. Shared
  SIGNAL/INTERPRET loop invalidation and TRACE lifecycle remain open under
  their own instruction rows and LC-AC-04/08.
- [x] **LC-AC-70 — complete ITERATE instruction:** childless `ITERATE`
  continues the innermost active repetitive DO, and `ITERATE symbol`
  continues the innermost active controlled DO whose authored control symbol
  matches without compound-tail substitution. It ends any intervening inner
  loops, skips the remainder of the selected body, and runs that loop's
  normal end processing: an applicable UNTIL check before the visible
  control step, count/step processing, then the next entry check. IF, SELECT
  and simple DO wrappers do not change the target. Local routines cannot
  iterate an inactive caller loop. Ordinary Level C diagnoses a
  source-provable invalid target at compile time (`28.2` or `28.4`);
  leading static `OPTIONS LEVELC STRICTC` defers a syntactically valid
  invalid target to an Error 28 signal only if reached. Level B/G checks
  remain unchanged. Malformed names and extra same-clause text fail at
  compile time with Classic identity and source position. Preserve one
  shared transfer/binding path, source/canonical AST ownership, and the
  existing emitter. Verify IBM/Regina reference cases, optimized/no-opt
  output for all loop kinds and nested/local contexts, invalid forms,
  source/canonical trees, relevant normal and shared runtime checks, and
  linked execution. SIGNAL/INTERPRET invalidation, TRACE and condition
  traps remain under their own instruction rows and LC-AC-04/08.
- [x] **LC-AC-71 — complete ARG instruction:** Classic `ARG [template_list]`
  behaves as `PARSE UPPER ARG`: it retrieves the active program or routine's
  argument strings without changing them, uppercases each source before
  parsing, and applies comma-separated templates in positional order.
  Templates support the same variable/dot, blank-word, literal and position
  patterns as PARSE, with no arbitrary template count limit. Repeated ARG
  instructions reread the same activation arguments; missing or omitted
  source positions parse as empty strings, while an explicit empty string
  remains present for argument-existence queries. Preserve source-order
  assignments through the visible pool, including compound targets and
  PROCEDURE EXPOSE aliases. Support main-program and internal routine
  activations and direct Classic subroutine/function calls that the compiler
  currently admits. Later external CALL and INTERPRET implementations must
  populate the same activation argument frame and qualify their own invocation
  paths under their instruction rows. Broader C host invocation modes retain
  a separate host owner; the later fixed-signature Level B/G CALL entry is
  owned by CALL, while general cross-dialect invocation remains outside the
  agreed programme scope. Neither is an ARG instruction closure gate.
  Diagnose malformed templates and illegal instruction placement with
  Classic identity and source position. Verify IBM/Regina examples,
  omitted-versus-empty behavior, exact-length strings and Unicode codepoint
  behavior under LC-AC-72, source/canonical AST, optimized/no-opt,
  relevant normal/shared runtime checks and linked toolchain execution.
  ARG built-in function behavior remains under the BIF row, but its shared
  activation-state dependency must be reviewed here.
- [x] **LC-AC-07 — scalar pool read and DROP slice:** an uninitialized or
  dropped scalar reads as its uppercase Classic symbol, direct scalar `DROP`
  affects the current visible pool (including a procedure's exposed alias),
  and reassignment restores its value. This initial scalar slice preceded
  direct stem/compound support under `LC-AC-53`; indirect `DROP` remains open.
  Verify against the Classic
  interpreter, optimized/no-opt and linked execution, runtime pool tests,
  and the relevant Level C and RexxScript correctness suites.
- [ ] **LC-AC-04 — full compatibility (R1-AC-02):** every required matrix row
  has executable behavior and documentation, or an individually approved
  exception with its diagnostic and user-visible limit. Verify reference
  equivalence within the approved Unicode-first boundary, documented Classic
  departures, supported-platform Unicode/Latin-1 behavior,
  optimized/no-opt parity, errors/conditions, lifecycle, toolchain, and
  packaging evidence on the exact Beta 4 candidate. This remains open after
  the first control-flow increments.
- [ ] **LC-AC-72 — one Unicode Level C scalar route:** every Level C literal,
  expression, variable, activation argument, PARSE field and returned BIF value
  used as a Classic scalar is valid UTF8 text; ordinary character units are
  codepoints with no runtime BYTE/UTF8 selector. `RexxValue` retains its
  binary and numeric capabilities for other consumers. Verify NUL, Latin-1,
  supplementary characters, opt/no-opt, linked output and RexxScript/Level B
  cross-consumer tests; inspect generated code for unwanted binary-to-text
  conversion of byte-valued BIF results.
- [ ] **LC-AC-73 — Latin-1 ordinal conversion bridge:** each of the 256 byte
  values round-trips through `X2C`/`C2X` as `U+00XX`; `D2C`, `C2D`, bitwise,
  `XRANGE` and hex/binary literals follow the same mapping where applicable.
  A byte-valued operation receiving a scalar above `U+00FF` raises the agreed
  conversion signal, while ordinary character operations accept it. Verify
  complete 256-value and out-of-range cases, nested BIFs, CTest, linked image
  and host text output. The selected error identity is `RXC-LC-23.1` through
  the existing BIF context, yielding the ordinary Classic syntax signal in
  compiled Level C. It denotes a scalar invalid for this byte conversion,
  not invalid Unicode. This decision is recorded before the first code edit.
- [x] **LC-AC-74 — closed-instruction Unicode review:** reconcile each
  formerly closed LC-I-01–10 instruction against the new scalar, source,
  symbol, host and error contract. Retain unaffected structural receipts;
  reopen and requalify affected instruction-owned behavior, including SAY,
  DROP and assignment. Verify the reviewed table below, targeted Unicode
  regressions and one relevant normal correctness suite after code changes.
- [ ] **LC-AC-75 — explicit Unicode and later binary boundary:** document the
  proposed new Unicode BIF names, input/output semantics, errors and Level
  B/G/RexxScript reuse for Adrian's separate syntax approval. Keep raw binary
  values/I/O as a separately planned future facility with no implicit
  conversion into Level C text. Verify the accepted BIF design and explicit
  binary deferral in architecture, language and release documents.
- [x] **LC-AC-76 — complete SIGNAL instruction:** direct symbol and quoted
  label targets, evaluated `VALUE expression`, and ON/OFF condition forms
  with default or named labels follow Classic clause, label and invocation
  rules. Evaluate a VALUE expression once, resolve label spelling without
  losing Unicode text, raise source-anchored 16.1 for a reached missing
  target, and diagnose malformed forms with Classic identity and location.
  Direct and trapped transfers update SIGL to the causing clause; SYNTAX
  traps set RC to the error number and preserve the underlying diagnostic.
  Install, disable, inherit and restore condition policy per activation;
  delivery is one-shot until re-enabled. Transfers discard crossed DO,
  selection, reference and handler state while preserving caller frames and
  the visible variable pool. Verify IBM/Regina reference cases, all admitted
  conditions and legal nested contexts, raw/canonical AST, optimized/no-opt,
  source metadata, linked output, normal Debug/Release Level C and Level
  B/G/RexxScript isolation. Exercise each condition handler with a controlled
  typed event where its real producer is not yet available. The real host HALT
  producer remains open under `LC-AC-06`; its absence does not block SIGNAL
  instruction closure.
- [x] **LC-AC-77 — TRACE instruction, approved practical scope:** bare,
  alphabetic, prefix, numeric and `VALUE` forms use one activation-local
  option state with source-anchored errors. The compiled `TRACE()` BIF observes
  and updates that state. Supported source, command, result and intermediate
  records contain the correct Unicode scalar values and source locations;
  unsupported records must be absent rather than display a false value.
  Command inhibition and ordinary execution remain correct. Preserve the
  existing Level B/G/RexxScript trace behavior. Document any remaining Classic
  divergence, including interactive, numeric, SCAN or record-coverage gaps,
  with a bounded probe and user-visible effect; these divergences do not block
  this instruction's closure under Adrian's 2026-10-06 direction. Verify
  focused reference cases, opt/no-opt linked output, one normal Debug and
  Release Level C checkpoint, and relevant Level B/G/RexxScript isolation.
  Keep full Level C and Release 1 criteria open until their own matrices pass.

## Implementation steps

**Unicode-first revision steps (approved character model; 2026-10-04).**
These supersede the unimplemented LC-STEP-73H profile bridge while preserving
its historical IDs and probe evidence. New BIF syntax and any further language
choice still require Adrian's approval.

1. **LC-STEP-88A (LC-AC-72/74/75; complete 2026-10-04):** record the approved scope change
   in this authoritative worklist, release plan and architecture/reference
   docs; audit all ten formerly closed instructions and retain valid evidence.
   No code dependency.
2. **LC-STEP-88B (LC-AC-72/73; complete 2026-10-04; depends on 88A):** implement one shared
   Latin-1 ordinal conversion service and the complete conversion/bitwise
   BIF family, including the selected conversion signal; verify 256 round
   trips, failures and RexxScript consumers before committing the increment.
3. **LC-STEP-88C (LC-AC-72/73; complete 2026-10-04; depends on 88B):** make Level C hex/binary
   literals, expression argument flow and resulting `RexxValue` objects use
   Unicode text consistently; keep the shared class's binary capability.
   Remove the now-unused conditional UTF-8 byte-to-value helper. Inspect
   canonical AST/emitter output and verify opt/no-opt and linked runs.
   `88C-1` decode source byte literals once into ordinals while preserving
   Level B/G AST and emitter behavior; `88C-2` build a source-anchored Unicode
   STRING constant for the Level C `RexxValue` factory, accepting the parser's
   STRING and BINARY byte-literal forms; `88C-3` cover `00`, `FF`, valid UTF-8
   byte sequences such as `C3A9`, binary suffixes, calls and PARSE patterns;
   `88C-4` remove the unused fallback helper and qualify source/canonical
   trees, opt/no-opt and linked execution. The 2026-10-04 pre-edit probe at
   `/tmp/crexx-literal-probe.VZbdvX` shows `'FF'x` rejected as an unsupported
   main statement and `C2X('C3A9'x)` incorrectly returning `E9`.
4. **LC-STEP-88D (LC-AC-57/62/63/72/74; complete 2026-10-04; depends on 88C):** requalify SAY,
   DROP and assignment as whole instructions for Unicode, including host text
   output, indirect lists and compound substitutions. Retain previously
   valid structural evidence; close each row only after focused and normal
   correctness checks. `88D-1` proves Level C SAY output through default and
   configured host routes for NUL, mapped high ordinals and non-Latin-1 text,
   then closes LC-I-01 (complete 2026-10-04). `88D-2` audits the shared pool classifier and Unicode
   direct/indirect DROP names and blanks, then closes LC-I-02 (complete
   2026-10-04). `88D-3` proves
   Unicode scalar values and compound-tail substitution through the same
   pool, then closes LC-I-03 (complete 2026-10-04). Commit and report each
   instruction separately.
5. **LC-STEP-88E (LC-AC-04/59/71/72/73/74; depends on 88D):** finish the
   Unicode character BIF and ARG/PARSE audit, then continue the strict
   instruction queue. Use one codepoint template engine; remove obsolete
   profile-selection assumptions and qualify affected shared consumers.
   `88E-1` (complete 2026-10-05) audits the complete implemented character BIF family as one
   Unicode text boundary, with codepoint positions, mapped ordinals, return
   types and RexxScript isolation. `88E-2` reviews ARG as a whole instruction
   for Unicode uppercasing, codepoint templates, activation modes and host
   entry. `88E-2A` (LC-AC-71/59; complete 2026-10-04; part of 88E-2;
   no new architecture gate)
   reuses the existing checked direct-CALL lowering and activation frame for
   a CALL inside a local procedure, including nested arms, so ARG can read
   that callee's arguments. Prove source/canonical trees, opt/no-opt, linked
   execution and normal Level C correctness. `88E-2B` (LC-AC-08/59/71;
   complete 2026-10-05 after 88E-2A) replaces direct CALL's flat `simple_tail` token list
   with the parser's existing expression/omission list, then uses the one
   general Level C expression validator and lowerer for actuals. Preserve
   omitted slots, once-only source-order evaluation, fresh frames, and CALL
   ON/OFF syntax. Prove nested ARG BIF actuals, recursive calls, malformed
   tails, source/canonical trees, opt/no-opt, linked output and normal
   correctness. CALL's other forms stay open.
   `88E-3` reviews PARSE as a whole instruction across its admitted
   source types and templates. Keep their open reference/condition obligations
   visible, and commit each qualified unit separately.
6. **LC-STEP-88F (LC-AC-75; depends on 88E for stable semantics):** propose
   explicit Unicode BIF APIs for separate language approval. Plan raw binary
   values/I/O separately rather than adding them implicitly to the scalar
   path.

**2026-10-04 LC-STEP-88B receipt.** `RexxClassicEncoding` now owns the
reversible `U+00XX`/byte ordinal bridge. Generated Level C config selects its
text path; direct BYTE consumers and `RexxValue` binary storage remain
available to RexxScript. `C2X`, `X2C`, `C2D`, `D2C`, `BITAND`, `BITOR`,
`BITXOR`, and `XRANGE` use the bridge and return text on the Level C path.
Inputs above `U+00FF` report `RXC-LC-23.1` through the BIF context and
compiled `CLASSIC_SYNTAX` signal. The focused Debug tests passed 14/14
(`/tmp/crexx-unicode-focused.5t4oPd`), including all 256 `X2C`/`C2X`
ordinals and optimized/no-opt compiled round trips. The normal Debug Level C
suite passed 446/446 (`/tmp/crexx-unicode-levelc.cyeXh5`); RexxScript,
`RexxValue`, shared BIF, and compiled out-of-range checks passed 10/10
(`/tmp/crexx-unicode-cross.Lc9jtS`). The linked image printed the expected
`0`, `FF`, `255`, `FF`, `16`, `FEFF00`, `1`
(`/tmp/crexx-unicode-linked.m5iGde`). `LC-AC-72/73` remain open for literal
flow, host text, and wider character-operation proof in STEP-88C–88E.

**2026-10-04 LC-STEP-88C receipt.** The common front end still classifies a
decoded source byte literal as STRING or BINARY according to UTF-8 validity;
Level C lowering accepts both, reads the source digits once, and emits a
source-anchored UTF-8 STRING constant for the existing `RexxValue` factory.
The same ordinal decoder serves PARSE literal templates. `'FF'x` and
`'11111111'b` now map to `U+00FF`; `'C3A9'x` maps to two scalars, and ordinary
`'é'` remains one. No AST node type or emitter case was added. The unused
conditional byte-to-value helper and its build dependencies were removed.
The first broad run exposed an unintended collision with constant symbols
ending in `B`/`X`, repaired by restricting suffix detection to literal AST
types, and two expected SAY byte-fixture changes. Seven targeted repairs passed
(`/tmp/crexx-literal-repair-focused.GqL3JF`); the exact corrected Debug Level C
suite passed 451/451 (`/tmp/crexx-literal-levelc-final.i3ys3X`), including
optimized/no-opt literal execution and lowered-tree checks. RexxScript,
`RexxValue`, shared and direct BIF checks passed 16/16
(`/tmp/crexx-literal-cross.PDUwOq`). A linked image emitted the same 12-line
fixture output (`/tmp/crexx-literal-linked.coWgYC`). `LC-AC-72/73` remain open
for the Unicode review of formerly closed instructions, ARG/PARSE character
behavior beyond these literals, and the host boundary.

**2026-10-04 LC-STEP-88D-1 SAY receipt.** The previously qualified two parser
forms, one canonical SAY lowerer/emitter path, source anchors, once-only
evaluation, output order, diagnostics and length-aware VM route are unchanged.
The expanded optimized/no-opt Level C byte fixture now proves embedded NUL,
mapped `U+0080`/`U+00FF`, non-Latin-1 BMP and supplementary text, BIF-produced
mapped text, source byte-literal ordinals and childless output. A new compiled
Level C RXVML entry proves the exact same UTF-8 spans and six callback calls;
the existing Level B/Level G callback, context-isolation and output-error
receipts remain valid. Ten focused checks passed
(`/tmp/crexx-say-focused.Sms7jz`); the Debug Level C suite passed 452/452
(`/tmp/crexx-say-levelc.M6KK8Z`). A `rxc`/`rxas`-built fixture linked with
`rxlink` and executed with `rxvm` emitted the expected bytes
(`/tmp/crexx-say-linked.p6jm66`). `LC-AC-57` and `LC-I-01` close for the
approved Unicode text contract. Shared character BIFs, raw binary I/O,
trapped conditions and complete Level C qualification remain in their own
open criteria.

**2026-10-04 LC-STEP-88D-2 DROP receipt.** The prior whole-instruction proof
for authored item order, direct scalar/stem/arbitrary compounds, source and
canonical trees, local/exposed pools and Regina's ignore-invalid-word rule is
retained. Indirect `DROP` now receives the existing activation configuration
reference. One pool helper uses the shared Classic Unicode text scanner for
ordinary and configured blanks, then the same configuration for subsidiary
symbol classification; it creates no BYTE-default configuration. The focused
pool test proves configured Greek letters, an extra separator, nonbreaking
space, an invalid emoji word and continuation to later valid names. A compiled
Level C fixture proves Unicode whitespace and invalid-word continuation in
optimized/no-opt modes. An initial build exposed a missing member dependency
and the first focused run exposed a config object/reference type mismatch;
both integration errors were repaired before qualification. Ten focused pool,
DROP and tree checks passed (`/tmp/crexx-drop-focused-fix.qdv7sU` plus the
optimized pool test); nine RexxScript checks passed
(`/tmp/crexx-drop-cross.L64JPf`), and the Debug Level C suite passed 452/452
(`/tmp/crexx-drop-levelc.WzlRuO`). The linked fixture preserved the expected
Unicode-list output (`/tmp/crexx-drop-linked.Ub2di4`). `LC-AC-62` and
`LC-I-02` close; shared host configuration and full Level C qualification
remain open.

**2026-10-04 LC-STEP-88D-3 assignment receipt.** The prior whole-instruction
proof for the parser's RHS/empty forms, scalar/stem/arbitrary compound targets,
RHS-before-substitution, nested/local execution, exposure, source/canonical
trees and one `setSymbolValue` pool path remains valid. The expanded
whole-instruction fixture proves a NUL-bearing scalar via `C2X`, Unicode BMP
and supplementary values, a compound tail containing mapped `FF`, NUL and
`80` ordinals, a Unicode RHS call that changes the target tail before the
pool write, and an exposed local Unicode write. The first expected-output
attempt incorrectly read a case-preserved tail without setting its component
variables; correcting the fixture yielded six focused passes
(`/tmp/crexx-assignment-focused-fix.jxsRdY`). The linked image emitted the
expected Unicode lines (`/tmp/crexx-assignment-linked.vCgwa3`); the normal
Debug Level C suite passed 452/452 (`/tmp/crexx-assignment-levelc.qcDNmx`).
No product implementation change was needed. `LC-AC-63` and `LC-I-03` close.
Together with the earlier ten-instruction impact audit and the SAY/DROP
receipts, this completes `LC-AC-74` and `LC-STEP-88D`. Unicode character BIFs,
ARG/PARSE and full Level C qualification remain open under their own criteria.

**2026-10-04 LC-STEP-88E-1 checkpoint.** The shared direct BIF sweep passed
106/110 first; four `SYMBOL`/`VALUE` failures came from July fixtures storing
only uppercase `A.X` while expecting the substituted lowercase `A.x` value.
Regina confirmed these are distinct case-preserved tails. The fixtures now
store and assert both keys; all four repaired opt/no-opt tests pass
(`/tmp/crexx-symbol-value-focused.1AysO6`), with the unchanged 106 direct
BIF results retained (`/tmp/crexx-character-bifs.2n9Vy2`). A compiled
31-result Unicode matrix covers character length/position, search, edits,
word scanning with U+00A0, case conversion and text results in optimized and
no-opt modes. Eight focused checks passed
(`/tmp/crexx-character-focused.kQE6rh`); the linked matrix matched the
unlinked output byte for byte (`/tmp/crexx-character-linked.6SjH5G`), and the
Debug Level C suite passed 454/454 (`/tmp/crexx-character-levelc.jssCD8`).
The character BIF text paths and direct BYTE isolation are evidenced, but
`TRANSLATE(source, output_table)` was the remaining implemented character-BIF
gap: the Unicode route reported `40.1` for the omitted input table. Adrian
approved U+0000–U+00FF implicit input ordinals on 2026-10-05, with higher
Unicode scalars unchanged. The text route now calculates the ordinal position
without allocating a table and uses the existing output/pad codepoint path.
The direct shared BIF harness covers NUL, U+0001, U+00FF, higher scalars,
short/empty outputs and BYTE isolation in optimized/no-opt modes; four
focused checks passed (`/tmp/crexx-translate-focused.ZaCYsL`). The compiled
Level C fixture passes both modes and a linked image with identical expected
output (`/tmp/crexx-translate-linked.fivHuQ`). The normal Debug Level C suite
passed 462/462 (`/tmp/crexx-translate-levelc.GcQqMo`). `88E-1` completes
the implemented character-BIF Unicode audit; unimplemented BIF services,
full reference semantics and host configuration remain open under LC-AC-04/06.

**2026-10-04 LC-STEP-88E-2A ARG checkpoint.** The Unicode ARG probe first
exposed that a local procedure could read its own ARG frame but could not
make a direct CALL (`unsupported procedure statement`). Procedure validation
and lowering now invoke the existing checked direct-CALL path also used by
main; no new AST node, activation implementation, or emitter case was added.
The new whole-instruction fixture covers codepoint positional templates,
Unicode uppercase, a Latin-1 byte literal, unchanged raw ARG BIF values,
repeated ARG, a local function and a local CALL nested in a selected IF arm.
The host-entry regression covers Unicode values over two native arguments;
the earlier main/local omission, empty, NUL, pattern, dynamic-position and
error checks are retained. The source/canonical tree check confirms the
authored nested CALL and a source-anchored canonical call with the existing
pool, configuration and fresh activation frame. Focused ARG checks passed
16/16 (`/tmp/crexx-arg-focused-fix.vlpFE9`), the nested fixture and tree
recheck passed 3/3 (`/tmp/crexx-arg-nested.SSr7U3`,
`/tmp/crexx-arg-treecheck.Xbpeez`), and the linked image produced the five
expected Unicode lines (`/tmp/crexx-arg-linked.NGQdTi`). The normal Debug
Level C suite passed 456/456 (`/tmp/crexx-arg-levelc.3YarlU`). The initial
new fixture failure is retained in `/tmp/crexx-arg-focused.RpgPlX` and was
repaired before qualification. `RexxValue` and RexxScript implementation
were not changed. LC-AC-71 and LC-I-11 stay open for the remaining invocation
and full-reference audit; unsupported expression actuals such as `CALL nested
ARG(1),,ARG(2)` and labels without PROCEDURE remain with the open CALL and
PROCEDURE rows, with their ARG activation consequences to be reconciled before
closure. LC-AC-59/61 remain open for the full instruction programme.

**LC-STEP-88E-2B approved design (proposed 2026-10-04; approved by Adrian
2026-10-05).** The
minimal `CALL relay ARG(1),,ARG(2)` probe in
`/tmp/crexx-arg-call-expr.LTSEjA` fails with `unsupported CALL argument
expression`. The raw tree shows why: direct CALL's `simple_tail` contains
`LITERAL arg`, `TOKEN (`, `INTEGER 1`, `TOKEN )` and comma tokens, whereas
normal function expressions already have a `FUNCTION` node with expression
children. A provisional lowerer-only attempt admitted general expression
nodes but could not repair that missing parser structure; it also revealed
that the old CALL string token retains source quotes and needs its special
decoder. That unqualified attempt and a failing new CTest registration were
removed. The prior qualified product code and tests remain intact.

1. **LC-88E-2B-01 (LC-AC-08/71):** parse direct CALL actuals using the
   existing `levelc_call_args` expression/comma grammar, with `NOVAL` for
   omissions under one `ARGS` node; keep CALL ON/OFF and recovery branches
   separate. Verify a raw AST matrix for no actuals, leading/middle/trailing
   omissions, quoted/hex literals, nested calls, operators and malformed
   parentheses/commas. Retain source anchors and Classic diagnostics.
2. **LC-88E-2B-02 (LC-AC-59/71; depends on 01):** remove the flat-tail
   validator/lowerer and pass each expression through the existing
   `levelc_expr_supported`/`levelc_lower_expr` path, appending a presence
   flagged value into the already approved activation frame immediately
   after that actual's setup. An omitted position appends absent/empty.
   Preserve one evaluation in source order, including side effects.
3. **LC-88E-2B-03 (LC-AC-08/59/71; depends on 02):** prove opt/no-opt,
   nested and recursive ARG/CALL frames, source/canonical trees, linked
   execution, malformed forms, existing CALL/ARG and Level B/G regressions,
   and one relevant normal Level C suite. Commit the qualified instruction
   dependency separately. Keep external/condition CALL under LC-I-13.

This is a source grammar/AST shape change even though it admits established
Classic syntax rather than inventing a new rule. Adrian approved it on
2026-10-05. A lowerer-side token mini-parser would duplicate the existing
expression grammar and create another special path; retaining the current
guard would leave valid CALL/ARG activation behavior unfinished. `RexxValue`
has no absent-value state; `RexxActivationArguments` already records slot
presence separately, so omitted actuals use that existing mechanism.

**2026-10-05 LC-STEP-88E-2B receipt.** Direct CALL now parses an `ARGS` child
of ordinary expression and `NOVAL` nodes; a call with no actuals has no `ARGS`
child. The old flat-tail validator, comma scanner and literal decoder are
removed. The general expression validator/lowerer populates the existing
presence-flagged activation frame in source order. The permanent fixture
proves nested `ARG(1)`, recursive frames, once-only side effects, middle and
edge omissions, quoted and hex literals, arithmetic, and no-argument calls in
optimized and no-opt modes. Raw/canonical tree inspection and `35.1` malformed
expression diagnostics pass; the linked image produces the expected 14 lines
(`/tmp/crexx-call-linked.6my6Ae`). Four focused CTests passed
(`/tmp/crexx-call-tree-test3.wHgLoF`), and the normal Debug Level C suite
passed 460/460 (`/tmp/crexx-call-levelc.YZ8tH7`). CALL's other forms and the
remaining ARG invocation/reference audit stay open under LC-I-13 and LC-AC-71.

1. **LC-STEP-01 (LC-AC-01):** reconcile the two references, the compiler's
   Classic BIF recognition inventory, runtime modules, compiler lowering,
   and registered tests into the matrix below. Link this worklist from the
   Release 1 plan. Commit this inventory increment. Detailed compatibility
   choices and exceptions require Adrian's decision.
2. **LC-STEP-02 — complete (LC-AC-02):** create a small Classic `IF` reproducer and
   target-shape baseline; implement guarded AST lowering using the existing
   remap builder, preserving one condition evaluation and source anchors.
   Add focused positives, negatives, and reference/opt/no-opt/toolchain
   evidence. Commit only when the normal correctness gate passes.
3. **LC-STEP-03 — complete (LC-AC-03; depends on STEP-02):** add the simple `DO` shape
   and nested cases, retain fail-closed gates for loop variants, qualify as
   above, then commit separately.
4. **LC-STEP-04 (LC-AC-01/04; depends on the approved matrix):** select and
   implement the remaining feature rows in reviewable increments, committing
   each qualified increment. Get Adrian's approval before any new language
   syntax, compatibility exception, or architectural change.
5. **LC-STEP-05 (LC-AC-04):** close the exact-candidate matrix with reference,
   correctness, supported-platform, package and release evidence; update
   user and compiler documentation. This is a release gate, not a substitute
   for normal focused development checks.
6. **LC-STEP-06 — complete (LC-AC-05; depends on STEP-03):** recognize only the childless
   Classic `NOP` AST in the supported statement validator, emit a canonical
   source-anchored `NOP` in main and procedure contexts, and compare a nested
   fixture with the Classic interpreter. Run focused opt/no-opt and normal
   Level C tests, verify the linked image, then commit this bounded increment.
7. **LC-STEP-07 — complete (LC-AC-06):** reconcile the existing Level C and
   RexxScript implementation with one current architecture design. Record the
   shared value/pool/BIF ownership, both call paths, sandbox boundary, and
   acceptance evidence in this worklist and the architecture document.
8. **LC-STEP-08 (LC-AC-06; depends on STEP-07):** qualify overlapping BIFs and
   pool operations in reviewable cross-consumer increments. Use the shared
   `rxfnsc` implementation, test both adapters and isolation, and commit each
   qualified increment. Keep any observed behavior gap visibly open. Prioritize
   AST closure steps before broad BIF or host expansion.
9. **LC-STEP-09 — complete (LC-AC-07; serves LC-AC-06; depends on STEP-07):** retain the
   Regina and compiled reproducer for an unset scalar read; use the shared
   pool's `symbolValue()` for scalar reads and lower guarded direct scalar
   `DROP` through that pool. Compare main/procedure/IF/DO cases and unsupported
   forms with the Classic reference, then run focused and normal regressions
   before committing this separate increment.
10. **LC-STEP-10 — complete (LC-AC-08; depends on STEP-09):** inventory parser-emitted
    structural node families and their lowering disposition. Add a production
    boundary check that rejects invalid parent/sibling ownership or surviving
    Level C-only nodes before canonical validation; exercise it on nested
    accepted shapes and retain debug tree evidence. Commit the verifier and
    crosswalk as one reviewable increment.
11. **LC-STEP-11 — complete (LC-AC-08/09/04; depends on STEP-10):** open the next Classic
    control-flow tree shape, starting with `SELECT/WHEN/OTHERWISE`, only after
    confirming its parsed AST and reference condition behavior. Prove branch
    order, nested arms, source anchors, opt/no-opt and missing-match behavior;
    keep unproved forms fail-closed. Commit separately.
12. **LC-STEP-12 (LC-AC-08/04; depends on STEP-11):** continue structural
    families such as repetitive/controlled `DO`, `LEAVE`/`ITERATE`, routine
    boundaries, PARSE templates and condition branches in risk-sized commits.
    Close `LC-AC-08` only after the full parser-node crosswalk and structural
    invariants are evidenced; a runtime or host dependency remains an open
    criterion rather than an implicit exclusion.
13. **LC-STEP-13 — complete (LC-AC-08/10; depends on STEP-11):** admit the parser's
    `DO > REPEAT > FOR > INTEGER` form only for a proven non-negative literal
    count representable in the canonical integer builder. Reuse a neutral
    counted-DO builder, validate body statements recursively, keep other
    repetition headers fail-closed, compare zero/nested counts with Regina,
    and qualify opt/no-opt, tree shape, linked execution and the normal suite.
    This is one bounded part of STEP-12, not closure of all loop forms.
14. **LC-STEP-14 — complete (LC-AC-08/11; depends on STEP-13):** reproduce the source-loop
    versus generated-wrapper association hazard. Map each accepted source
    counted loop to a hidden canonical control symbol, and lower childless
    LEAVE/ITERATE to named canonical transfers targeting that symbol through
    the existing association machinery. Validate source nesting before
    lowering, preserve source anchors and loop state, reject named/unsupported
    transfer shapes, and qualify focused/normal/linked execution. Keep global
    `ast_do()` semantics unchanged.
15. **LC-STEP-15 — complete (LC-AC-08/12; depends on STEP-14):** recognize the parser's
    `DO > REPEAT("forever") > INSTRUCTIONS` form. Reuse the hidden-control
    mapping and canonical association path from STEP-14, omitting the FOR
    count in the neutral builder. Keep every other new repetition/condition
    shape fail-closed. Qualify with bounded-termination fixtures and commit
    separately.
16. **LC-STEP-16 — complete (LC-AC-08/13; depends on STEP-15):** admit the parser's
    `DO > WHILE > INSTRUCTIONS` shape only when its condition lowers without
    setup statements. Add the WHILE condition to the neutral controlled-loop
    AST, reuse hidden source-loop transfer binding, validate exact logical
    `34.3` behavior in the shared value class, and keep UNTIL, combined headers
    and setup-requiring expressions fail-closed. Qualify condition timing,
    zero-entry, transfer targets, optimized/no-opt, source anchors, linked
    execution, and normal correctness before committing the increment.
17. **LC-STEP-17 — complete (LC-AC-08/14; depends on STEP-16):** use the existing canonical
    UNTIL node in the neutral controlled-loop builder only after confirming
    its post-body check and ITERATE path against the Classic reference. Reuse
    the setup-free condition guard and hidden source-loop binding, add exact
    `34.4` validation in shared RexxValue, and keep combined headers and
    setup-bearing expressions fail-closed. Qualify and commit separately.
18. **LC-STEP-18 — complete (LC-AC-08/15; depends on STEP-17):** admit the parser's
    `DO > REPEAT > WHILE/UNTIL > INSTRUCTIONS` shape only for a proven literal
    count and setup-free condition. Place FOR and the condition under one
    controlled canonical REPEAT; verify count/condition ordering and both
    transfer paths with bounded Classic reference fixtures. Keep dynamic
    counts, controlled variables and setup-bearing expressions fail-closed.
    Qualify and commit this combined-header increment separately.
19. **LC-STEP-19 — complete (LC-AC-08/16; depends on STEP-18):** replace the Level C
    branch materialisation of `&`/`|` with left-to-right eager lowering.
    Capture the left `RexxValue` before any right setup statements, then
    evaluate the right once and call shared contextual logical methods.
    Prove side effects and `34.5`/`34.6` against Classic references, preserve
    source anchors and scope, run focused/normal/linked/Debug evidence, then
    commit this semantic repair separately from the condition-block work.
20. **LC-STEP-20 — complete (LC-AC-08/17; depends on STEP-19):** for direct `DO WHILE`,
    lower a supported setup-bearing condition into a canonical `BLOCK_EXPR`
    whose instructions are the condition prelude followed by `LEAVE WITH`
    the contextual logical value. Preserve the existing direct expression
    path when no setup is generated. Prove the block expression executes on
    every entry, its generated names remain in scope, and inner LEAVE WITH
    does not disturb source LEAVE/ITERATE. Keep UNTIL/combined setup-bearing
    forms fail-closed, qualify and commit this structural increment.
21. **LC-STEP-21 — complete (LC-AC-08/18; depends on STEP-20):** admit supported
    setup-bearing conditions for direct `DO UNTIL` and reuse the canonical
    `BLOCK_EXPR` condition builder from STEP-20. Prove the generated prelude
    runs only after the body and on ITERATE, with the correct source loop
    association and `34.4` identity. Remove the obsolete unsupported fixture,
    keep combined headers fail-closed, qualify and commit this increment.
22. **LC-STEP-22 — complete (LC-AC-08/19; depends on STEP-21):** allow supported
    setup-bearing WHILE/UNTIL conditions under the existing literal-count
    canonical REPEAT. Reuse the condition `BLOCK_EXPR` builder while retaining
    the no-setup direct path. Confirm zero-count suppression, limit-boundary
    check timing, ITERATE and LEAVE behavior against Regina. Remove the
    obsolete combined-setup negative fixture, retain dynamic/controlled
    guards, qualify and commit this increment.
23. **LC-STEP-23 — complete (LC-AC-08/20; depends on STEP-22):** recognize the parser's
    childless `REPEAT("forever")` followed by WHILE/UNTIL, and omit FOR in the
    existing canonical controlled-loop builder. Reuse condition block
    expressions and source-loop transfer binding. Prove zero-entry versus
    post-body semantics, ITERATE and LEAVE, remove the obsolete negative
    fixture, qualify and commit this structural increment.
24. **LC-STEP-24 — complete (LC-AC-08/21; depends on STEP-23):** add a contextual
    `RexxValue` whole-count conversion and permit supported direct repetition
    expressions. Lower count setup under the canonical FOR as a BLOCK_EXPR so
    it runs once, reuse the neutral builder and hidden source-loop binding,
    and retain fail-closed guards on combined dynamic and controlled forms.
    Prove valid whole spellings, zero, mutation, BIF side effects and `26.2`
    boundaries against the references; qualify and commit separately.
25. **LC-STEP-25 — complete (LC-AC-08/22; depends on STEP-24):** permit the already
    bounded dynamic FOR conversion and setup block alongside a supported
    WHILE/UNTIL condition under one canonical REPEAT. Prove runtime setup
    order and zero-count suppression against Regina, including BIF/local
    calls and transfer paths. Remove the obsolete dynamic-combined negative
    fixture, keep controlled headers fail-closed, qualify and commit.
26. **LC-STEP-26 — complete (LC-AC-08/23; depends on STEP-25):** recognize only the
    source REPEAT assignment plus literal TO shape for a scalar pool variable.
    Lower the initial pool assignment before one canonical REPEAT; use its
    WHILE entry check for the current pool value against TO and an UNTIL
    end-check BLOCK_EXPR to advance the pool value and return false. Extend
    the neutral loop builder only as needed for both checks. Use contextual
    RexxValue numeric validation and the existing hidden source-loop binding.
    Prove body mutation, zero-entry, final values, ITERATE/LEAVE and nested
    ownership against Regina. Retain fail-closed guards on other controlled
    shapes, qualify and commit separately.
27. **LC-STEP-27 — complete (LC-AC-08/24; depends on STEP-26):** extend the controlled
    clause crosswalk to one literal signed BY and one literal TO in either
    parser order. Reuse the pool-backed canonical WHILE/UNTIL loop and add
    contextual RexxValue step comparison/advance methods while preserving
    the default BY-one behavior. Prove negative, positive and guarded zero
    steps, mutation, final values and ITERATE/LEAVE against Regina; keep
    dynamic BY/FOR and wider headers fail-closed, qualify and commit.
28. **LC-STEP-28 — complete (LC-AC-08/25; depends on STEP-27):** recognize one bounded
    literal FOR alongside the existing literal TO and optional literal BY,
    in any parsed modifier order. Feed its count to the canonical REPEAT FOR
    child while retaining the pool-backed WHILE entry and UNTIL end step.
    Prove zero and positive counts, direction, final-value and transfer
    timing against Regina. Keep dynamic FOR and FOR-without-TO fail-closed,
    qualify and commit separately.
29. **LC-STEP-29 — complete (LC-AC-08/26; depends on STEP-28):** admit the existing
    bounded controlled FOR header without TO, optionally with signed literal
    BY. Generalize the controlled clause crosswalk and omit the canonical
    WHILE child when TO is absent, retaining FOR and the pool-advancing UNTIL
    end block. Prove zero count, final values, mutation, ITERATE/LEAVE and
    nested scope against Regina, then qualify and commit separately.
30. **LC-STEP-30 — complete (LC-AC-08/27; depends on STEP-29):** map a named transfer's
    source variable to a supported active controlled DO during validation and
    to that DO's hidden canonical binding during lowering. Reuse existing
    LEAVE/ITERATE nodes through the emitter, preserving source anchors and
    transfer timing across nested/generated blocks. Keep the front-end
    `28.3`/`28.4` invalid-name checks; prove outer and inner targets against
    Regina, qualify and commit separately.
31. **LC-STEP-31 — complete (LC-AC-08/28; depends on STEP-30):** admit a supported
    controlled WHILE header and combine its entry condition with the existing
    TO guard lazily under one canonical WHILE, preserving FOR-before-TO-before-
    WHILE timing. Reuse the canonical end-step block and source binding,
    including setup-bearing conditions and exact `34.3`. Keep UNTIL and
    dynamic controlled clauses fail-closed, prove Regina/tree/toolchain and
    normal/Debug checks, then commit separately.
32. **LC-STEP-32 — complete (LC-AC-08/29; depends on STEP-31):** admit a supported
    controlled UNTIL header by combining its condition and the existing pool
    advance under the canonical end-check BLOCK_EXPR. Check the condition
    first, return true without stepping, or step and return false; retain
    FOR/TO entry guards and hidden source-loop bindings. Prove ITERATE,
    LEAVE, final count timing, setup and `34.4` with Regina and normal/Debug
    checks, then commit separately.
33. **LC-STEP-33 — complete (LC-AC-08/30; depends on STEP-32):** allow the parser's
    controlled assignment header without TO/FOR, with optional literal BY
    and supported WHILE/UNTIL. Reuse the pool-backed canonical REPEAT and
    end step, omitting FOR and any synthetic entry guard. Keep guarded
    reference fixtures and bounded command execution; prove transfer,
    condition, scope, tree, toolchain and normal/Debug behavior, then commit.
34. **LC-STEP-34 — complete (LC-AC-08/31; depends on STEP-33):** accept a supported
    dynamic TO expression, lower its setup once before the initial pool set,
    validate its numeric value there, copy the resulting RexxValue into a
    hidden canonical local, and use that
    local in the existing WHILE TO check. Keep the literal TO fast path and
    dynamic start/BY/FOR guards. Prove Regina timing, mutation, `41.4`,
    source-anchored ownership and normal/Debug/toolchain behavior, then
    commit separately.
35. **LC-STEP-35 — complete (LC-AC-08/32; depends on STEP-34):** admit supported
    dynamic BY expressions, lower dynamic TO/BY clause setup in parsed
    source order before the initial pool set, validate and copy each value
    into a hidden canonical local, and reuse the existing TO comparison and
    end-step methods. Prove signed/zero steps, source mutation, FOR-zero
    `41.5`, nesting and condition timing against Regina, then qualify and
    commit separately.
36. **LC-STEP-36 — complete (LC-AC-08/33; depends on STEP-35):** admit a supported
    dynamic controlled FOR expression, evaluate and validate it once in the
    parsed TO/BY/FOR order before the initial pool set, and feed its hidden
    canonical int to the existing REPEAT FOR child. Add shared RexxValue
    contextual `26.3` conversion; prove zero/positive count, mutation,
    transfers, conditions, source anchors, linked execution and normal/Debug
    checks, then commit separately.
37. **LC-STEP-37 — complete (LC-AC-34; depends on STEP-35, blocks STEP-36):** retain a
    minimal existing-syntax reproducer for the call-window affinity alias
    exposed by captured BY across a loop back edge. Use a disjoint call window
    for canonical UNTIL end-check calls while retaining normal affinity for
    other call sites; prove the counterexample and the existing WHILE path in
    opt/no-opt runtime and emitted marshalling, then run the
    relevant normal/Debug checks and commit the independent repair.
38. **LC-STEP-38 — complete (LC-AC-08/35; depends on STEP-36):** capture a supported
    dynamic start into a hidden canonical `RexxValue` before walking the
    remaining header clauses. Keep their existing written-order validation,
    then validate start with shared contextual `41.6` immediately before the
    pool assignment. Feed that checked value into the existing canonical
    pool-set and controlled-loop builders; add no AST node unless those
    builders cannot express the required ownership or timing. Prove reference
    parity, error precedence, tree shape, opt/no-opt and linked behavior,
    qualify the relevant normal suites, and commit this increment separately.
39. **LC-STEP-39 — complete (LC-AC-08/36; depends on STEP-38):** preserve the parser's
    zero-length `STRING` payload when creating the shared `RexxValue` literal,
    without relaxing missing-payload checks on other node types. Retain the
    existing canonical string factory and emitter path; inspect the source
    anchor and opt/no-opt value flow, prove Regina and contextual numeric
    errors, run the relevant normal checks, and commit separately.
40. **LC-STEP-40 — complete (LC-AC-08/37; depends on STEP-39):** admit an adjacent
    function-call term in the Level C continuation expression grammar, using
    source token positions to preserve the spaced symbol-plus-group meaning.
    Build the existing `FUNCTION` AST node and canonical BIF/local-call path;
    add no new emitter node. Reproduce both spellings against Regina, inspect
    raw and lowered trees, qualify focused and normal checks, and commit this
    parser/AST increment separately.
41. **LC-STEP-41 — complete (LC-AC-06/08/38; depends on STEP-40):** route each supported
    ARG binding's captured `RexxValue` through the existing shared Classic
    TRANSLATE BIF context before the canonical pool set. Keep caller argument
    copies and target order intact; reuse the existing argument-frame builder
    and source anchors. Prove Regina and shared BYTE-default behavior,
    opt/no-opt and linked execution, qualify normal checks, and commit this
    binding increment separately.
42. **LC-STEP-42 — complete (LC-AC-08/39; depends on STEP-41):** validate the parser's
    `PARSE` option, source and nested template-list ownership for direct
    single-target `VAR` and `VALUE` forms. Lower source evaluation and optional
    shared TRANSLATE into canonical pool assignment with source anchors;
    reject all other template shapes. Prove Regina, opt/no-opt, nested
    main/procedure, target-tree and linked behavior, then commit separately.
43. **LC-STEP-43 — complete (LC-AC-06/08/40; depends on STEP-42):** wrap the existing
    `parsewords3` opcode in a shared `RexxValue` result method, retaining its
    exact two-word-plus-tail contract. Extend the Level C PARSE tree guard to
    only three direct scalar targets, capture the source once, call the shared
    method and write each result through the visible pool in authored order.
    Reuse shared TRANSLATE for optional UPPER. Prove reference, shared runtime,
    canonical AST, opt/no-opt and linked behavior before the separate commit.
44. **LC-STEP-44 — complete (LC-AC-08/41; depends on STEP-43):** infer the missing
    concatenation operator from the physical token span between the last
    represented token of the left AST and the first represented token of the
    right AST. Include intervening parentheses and call punctuation in the
    touch check, and apply it to the three Level C implicit-concatenation
    grammar paths. Preserve the explicit operator and spaced cases, prove
    Regina and canonical runtime equivalence, then commit this parser/AST
    correction separately.
45. **LC-STEP-45 — complete (LC-AC-08/42; depends on STEP-44):** scan bounded decimal
    fraction forms as one existing numeric token and represent them with the
    existing source `DECIMAL` AST node. Extend the Level C parser's operand
    contexts and lower that node through the same shared `RexxValue` string
    path as other Classic constants. Retain integer-only fast paths for DO/FOR
    counts and prove contextual errors, tree shape and linked equivalence
    before a separate commit.
46. **LC-STEP-46 — complete (LC-AC-08/43; depends on STEP-45):** extend the numeric
    scanner pattern to exponent forms with required exponent digits, emit the
    existing `DECIMAL` source AST node, and normalize only the exponent letter
    to Classic uppercase when constructing the shared `RexxValue` string.
    Check direct expressions, arithmetic, PARSE VALUE, count validation,
    canonical lowering and linked output before a separate commit.
47. **LC-STEP-47 — complete (LC-AC-08/44; depends on STEP-46):** scan digit-starting
    constant symbols after complete numeric forms, map them to the existing
    `CONST_SYMBOL` AST node and lowercase-to-uppercase Classic literal path,
    and retain numeric and invalid-assignment classification. Prove the raw
    symbol and canonical shared-value tree, Regina parity, parser/highlighter
    behavior and linked output before a separate commit.
48. **LC-STEP-48 — complete (LC-AC-06/08/45; depends on STEP-47):** derive the first
    word using the existing shared `parsewords3` VM wrapper, then retain the
    exact two-target remainder from the captured source in a shared
    `RexxValue.parseWordAndRest()` method. Extend the validated nested PARSE AST
    slice to two targets, capture the returned pair before ordered pool
    writes, and keep optional shared TRANSLATE before splitting. Prove
    reference behavior, shared method tests, canonical lowering and linked
    execution before the separate commit.
49. **LC-STEP-49 — complete (LC-AC-06/08/46; depends on STEP-48):** wrap the existing
    `parsewords3d` VM instruction in a shared `RexxValue` method, validate
    exactly three direct scalar targets followed by a final dot in the
    nested Level C template AST, and capture all three outputs before pool
    writes. Keep optional shared TRANSLATE and fail closed for other dot
    placements. Prove Regina, shared method, canonical AST, opt/no-opt and
    linked behavior before a separate commit.
50. **LC-STEP-50 — complete (LC-AC-08/47; depends on STEP-49):** validate `.` as a
    placeholder in two- and three-item nested PARSE templates while retaining
    the existing final-dot four-item slice. Reuse the shared two-/three-result
    split methods, suppress pool writes for dots, and preserve capture before
    scalar writes. Compare reference behavior and source-anchored raw/canonical
    trees, run opt/no-opt, normal Level C, and linked checks, then commit
    separately.
51. **LC-STEP-51 — complete (LC-AC-08/48; depends on STEP-50):** allow all-dot direct
    templates of one to three items, with a hidden canonical capture for the
    single-dot VALUE source and the existing shared split captures for two or
    three dots. Emit no pool writes, retain source anchors, and prove
    side-effect, scope, optimizer and linked parity before a separate commit.
52. **LC-STEP-52 — complete (LC-AC-06/08/49; depends on STEP-51):** compose the shared
    `parsewords3`/`parseWordAndRest()` operations into a four-result
    `RexxValue` method, admit four direct scalar/dot items in the nested PARSE
    AST, select the existing final-dot method when appropriate, and skip only
    dot pool writes. Replace newly obsolete four-item negative fixtures with
    a five-item guard, then prove reference, shared-consumer, structural,
    opt/no-opt, normal Level C and linked behavior before a separate commit.
53. **LC-STEP-53 — complete (LC-AC-06/08/50; depends on STEP-52):** replace the
    target-count dispatch with one shared word/dot result-vector operation
    that chains the certified `parsewords3` primitive through
    `parseWordAndRest()`. Validate an arbitrary nonempty direct target list,
    pass its count once, capture all results, and write non-dot targets in
    order. Replace obsolete five-item negatives with a distinct unsupported
    template family. Prove reference, shared-consumer, structural, opt/no-opt,
    normal Level C and linked behavior before a separate commit.
54. **LC-STEP-54 — complete (LC-AC-08/51; depends on STEP-53):** map the parser's mixed
    template AST to a validated, count-independent item sequence and compile
    static items into the frozen `parseplan` descriptor. Reuse the existing
    canonical assembler node and string-array result shape where possible;
    add a dedicated node only if the existing emitter boundary cannot
    represent the descriptor safely. Capture source and results before pool
    writes, compare with Regina for pattern/position interaction and Unicode,
    inspect raw/canonical ownership, run focused Debug/Release and the normal
    Level C suite, and retain linked-image evidence before committing.
55. **LC-STEP-55 — complete (LC-AC-52; discovered during STEP-54):** retain the
    Level B exit and Regina counterexample, repair the frozen VM plan's
    backward cursor without changing the descriptor format, prove focused
    opt/no-opt and normal correctness, then commit the shared runtime cause
    separately from Level C mixed-template lowering.
56. **LC-STEP-56 — complete (LC-AC-06/53/54; depends on STEP-09):** repair shared stem
    assignment/reset and dropped-tail state, preserve substituted tail case,
    and add pool operations for non-mutating compound reads and one-tail drops.
    Prove the discovered reference counterexamples and focused cross-consumer
    behavior, then commit this distinct runtime cause.
57. **LC-STEP-57 — complete (LC-AC-08/53; depends on STEP-56):** broaden the Level C
    direct-list guard and lowerer to call shared scalar/stem/compound pool
    operations in source order, and replace obsolete negative fixtures with
    Regina-based focused and linked proof. Retain parenthesized indirect lists
    as a separately guarded shape; run the normal Level C and RexxScript
    checks before committing.
58. **LC-STEP-58 — complete (LC-AC-06/55; depends on STEP-56):** implement one validated
    Classic symbol drop in `RexxVariablePool` using its existing name resolver
    and scalar/stem/tail operations. Prove substitution and exposure with
    focused pool tests, optimized/no-opt, linked execution and cross-consumer
    checks, then commit the runtime primitive separately.
59. **LC-STEP-59 — complete (LC-AC-08/56; depends on STEP-57/58):** add shared subsidiary-list interpretation and lower the
    parser's `VAR_REFERENCE` node to a canonical helper call in source order.
    Ignore invalid subsidiary-list words as approved; verify that behavior,
    reference behavior, raw/canonical trees,
    opt/no-opt, normal Level C and linked execution before committing.
60. **LC-STEP-60 — checkpoint complete (LC-AC-08/57; folded into STEP-63):**
    incorporate childless `SAY` into an instruction-level fixture covering
    expression side effects, output order, nested execution and local routines.
    Regina, opt/no-opt, tree, normal Level C and linked evidence are recorded
    under the review below. This checkpoint was not a feature-completion
    claim; whole SAY closed later under STEP-63F.

61. **LC-STEP-61 — review complete (LC-AC-58/59):** inspect the active parser,
    lowerer, remap builders, shared pool/value/BIFs, VM PARSE path, tests and
    coverage records. Record specific duplication and safe foundations in the
    architecture review; retain all prior criteria and evidence.
62. **LC-STEP-62 (LC-AC-58/59; depends on STEP-61):** apply the same strict
    review gate to each instruction before implementation: map all parsed
    forms and reference obligations, identify duplicate paths and missing
    semantics, select the simplest implementation, and identify decisive
    existing or missing tests. Obtain approval before a particular language
    or architecture decision. Refactor only the active instruction and any
    required shared foundation in reviewable commits. Measure any new
    aggregate CTest in isolated Debug and sanitizer builds before registering
    it. Close the instruction with an evidence receipt. A fundamental conflict
    requiring a serious compromise remains open while Adrian decides its
    individually proposed disposition; ordinary implementation effort never
    qualifies for that path.

### Whole-instruction programme

The following rows are semantic delivery units and implementation steps for
`LC-AC-59`. A row is **open** until its own full instruction contract is
evidenced; closed rows are marked explicitly, and current bounded behavior is
retained in the coverage matrix.
Work through the rows in order, with only one active row; changing the order
requires a recorded reason and must not turn a partial row into a closure.
On 2026-10-04 Adrian prioritized the high-risk DO AST work before the remaining
prior-instruction audit and SIGNAL. LC-I-08 closed after its whole-instruction
review; LC-I-05 OPTIONS, LC-I-06 IF and LC-I-07 SELECT subsequently closed.
LC-I-09 LEAVE and LC-I-10 ITERATE then closed; LC-I-11 ARG closed after the
2026-10-05 whole-instruction receipt below. The
priority change did not count any row as reviewed or closed before its own
evidence receipt.
Each step includes parser-form inventory, reference cases, invalid forms,
main/procedure/nested execution where legal, opt/no-opt, source/AST checks,
relevant runtime and host-text checks, and linked delivery. The relevant
normal correctness suite runs once per coherent instruction checkpoint, with
focused checks during its development. A structural change affecting all
instructions calls for the full Level C suite at that checkpoint. Reuse
unchanged evidence and leave overnight assurance to its scheduled lanes.

| Unit | Step | Complete instruction obligation and principal dependency |
| --- | --- | --- |
| LC-I-01 SAY — closed 2026-10-04 | LC-STEP-63F; LC-STEP-88D-1 | `LC-AC-57`: expression, ordering and callback evidence retained; NUL, mapped high ordinals and non-Latin-1 output now pass default, configured host, optimized/no-opt and linked checks. |
| LC-I-02 DROP — closed 2026-10-04 | LC-STEP-64B; LC-STEP-88D-2 | `LC-AC-62`: prior list order, arbitrary compounds, exposure and Regina invalid-word policy retained; activation configuration, Unicode words and blanks now pass focused pool, opt/no-opt, linked and normal Level C checks. |
| LC-I-03 assignment — closed 2026-10-04 | LC-STEP-65B; LC-STEP-88D-3 | `LC-AC-63`: prior RHS order and shared-pool ownership retained; Unicode scalar values, NUL/high-ordinal tails, local exposure and RHS-before-substitution now pass opt/no-opt, linked and normal Level C checks. |
| LC-I-04 NOP — closed 2026-10-04 | LC-STEP-66B | `LC-AC-64`: childless behavior and invalid tails in accepted statement contexts; shared label/TRACE lifecycle remains under its own open criteria. |
| LC-I-05 OPTIONS — closed 2026-10-04 | LC-STEP-67A–67D | `LC-AC-66`: static source header, executable expression at each source point, no-op empty form, unknown-word policy and configuration ownership. The approved Unicode model defines no runtime profile word. Shared condition, TRACE and host obligations remain open in their own rows. |
| LC-I-06 IF — closed 2026-10-04 | LC-STEP-68A–68C | `LC-AC-67`: all arm positions without IF-specific rejection, nearest ELSE, condition/error and nesting behavior. Each arm's instruction semantics remain with its owner row; shared condition/trap lifecycle remains open in its own criteria. |
| LC-I-07 SELECT — closed 2026-10-04 | LC-STEP-69A–69C | `LC-AC-68`: WHEN/OTHERWISE forms, arm instructions, evaluation and no-match/error lifecycle; IF and statement dispatch. Shared helper-stack source reporting and TRACE/trap lifecycle remain open under LC-AC-08/04. |
| LC-I-08 DO — closed 2026-10-04 | LC-STEP-70A–70D | `LC-AC-65`: simple, counted, controlled scalar/compound, FOREVER, WHILE/UNTIL and legal combinations without an arbitrary count limit; one checked header and reviewed loop representation. Shared NUMERIC/condition/TRACE/host lifecycle remains open in its own rows. |
| LC-I-09 LEAVE — closed 2026-10-04 | LC-STEP-71A–71C | `LC-AC-69`: unnamed/named targets, nesting, state and errors with the approved default/STRICTC timing distinction; DO. |
| LC-I-10 ITERATE — closed 2026-10-04 | LC-STEP-72A–72C | `LC-AC-70`: unnamed/named targets, end-step timing, nesting and errors across all legal loops through the shared DO/LEAVE path. |
| LC-I-11 ARG — closed 2026-10-05 | LC-STEP-73A–73I | `LC-AC-71`: complete `PARSE UPPER ARG` semantics on admitted Classic activations, arbitrary comma templates, omitted/empty positions, Unicode and diagnostics through one shared PARSE executor and argument frame. CALL later reused the frame under LC-I-13; INTERPRET and host modes retain separate owners. |
| LC-I-12 PROCEDURE — closed 2026-10-05 | LC-STEP-74A–74D | `LC-74-01–05`: first-instruction and diagnostics, private pool, direct/indirect scalar/stem/exact compound EXPOSE, source order, nested/recursive alias lifetime, Unicode and isolation. CALL/RETURN later closed under LC-I-13/14; EXIT and shared full Level C lifecycle remain open under their own rows. |
| LC-I-13 CALL — closed 2026-10-06 | LC-STEP-75A–75E | `LC-75-01–06`: whole syntax/error, local/BIF/external signed resolution, ARG and result lifecycle, linked and native host execution, and four-condition delayed policy/handler delivery pass under the accepted static provider boundary. ADDRESS, stream and host HALT producers retain their own open rows; this CALL closure does not qualify them. |
| LC-I-14 RETURN — closed 2026-10-06 | LC-STEP-76A–76C | `LC-76-01–05`: bare/value syntax and anchored errors, one-body local function/subroutine lifecycle, result presence/drop, outermost status through the implicit main and native host, optimized/no-opt direct/linked proof. EXIT and host result exchange remain separately open. |
| LC-I-15 EXIT — closed 2026-10-06 | LC-STEP-77A–77D complete | `LC-77-01–06`: explicit termination across main/internal/external frames, result and host paths, source errors and Regina physical EOF pass focused and grouped qualification. |
| LC-I-16 PULL — closed 2026-10-06 | LC-STEP-78A–78D complete | `LC-78-01–05`: one selected-queue/default-input acquisition and shared PARSE UPPER template executor; source, error, EOF, queue interaction and grouped qualification pass. Host selection API remains under LC-AC-06. |
| LC-I-17 PUSH — closed 2026-10-06 | LC-STEP-79A–79D complete | `LC-79-01–04`: optional expression/bare null, source/errors, exact text, front insertion and nested/recursive lifecycle pass with PULL/QUEUE and grouped qualification. Host selection API remains under LC-AC-06. |
| LC-I-18 QUEUE — closed 2026-10-06 | LC-STEP-80A–80D complete | `LC-80-01–04`: optional expression/bare null, source/errors, FIFO tail and PUSH/PULL interaction pass after shared rxfnsb active-count repair and grouped qualification. Host selection API remains under LC-AC-06. |
| LC-I-19 PARSE — closed 2026-10-06 | LC-STEP-81A–81E complete | `LC-81-01–07`: all seven agreed sources, UPPER, arbitrary/comma templates, patterns, positions, errors, source services and frame behavior pass one shared executor and qualified checkpoint. EXTERNAL and NUMERIC are explicitly outside initial Level C scope. |
| LC-I-20 ADDRESS — closed 2026-10-06 | LC-STEP-82A–82E complete | Selection/swap/transient command, WITH resources, frame state, RC/conditions and configured host path pass opt/no-opt, linked, callback and grouped qualification. The approved embedded-NUL host-command exception remains open as `LC-HOST-ADDRESS-NUL` under `LC-AC-06`. |
| LC-I-21 implicit command — closed 2026-10-06 | LC-STEP-83A–83D complete | Expression-only commands evaluate once, then use the active invocation's ADDRESS environment and lasting connections through the shared adapter. Forms/errors, source warnings, linked opt/no-opt, nested/external and native host conditions passed grouped normal and focused sanitizer checks. `LC-HOST-ADDRESS-NUL` remains open under LC-AC-06. |
| LC-I-22 NUMERIC — closed 2026-10-06 | LC-STEP-84A–84D complete | `LC-84-01–05`: executable DIGITS/FORM/FUZZ, source errors, once-only evaluation, activation-local lifetime, arithmetic/display/BIF effects and LOSTDIGITS pass opt/no-opt and grouped qualification. Full Level C and cross-cutting criteria remain open. |
| LC-I-23 SIGNAL — closed 2026-10-06 | LC-STEP-85A–85D complete | `LC-AC-76`/`LC-85-01–05`: direct/quoted/VALUE branches, seven ON/OFF identities, source-ordered frame labels, one-shot and activation-local delivery, condition state, malformed and missing-target diagnostics, linked and Debug/Release proof. Real host HALT production stays under `LC-AC-06`. |
| LC-I-24 TRACE — closed 2026-10-06 | LC-STEP-86A–86E | `LC-AC-77` practical scope: activation-local options and `TRACE()`, correct displayed Unicode/NUL values, authored source, command inhibition and diagnostics pass linked opt/no-opt and grouped Debug/Release. Interactive, numeric skip/suppress, SCAN and incomplete event coverage are recorded divergences below. |
| LC-I-25 INTERPRET — parked, not implemented 2026-10-07 | LC-STEP-87A research retained; 87B–87E parked | Parser recognizes the form but lowering rejects it. `LC-87-01–05`, `LC-AC-59/04` and `R1-AC-01/02` remain open; this is not an approved "won't implement" exception. |

**LC-I-24 TRACE plan — vision and intended outcome, 2026-10-06; scope revised
by Adrian 2026-10-06.** A Classic program can select, query and reset tracing
at execution time, inspect correct source and scalar values, and suppress or
examine host commands without changing ordinary program semantics. Remaining
Classic trace differences with little practical value may be documented and
closed rather than expanded into compiler/runtime work. Level C should reuse
the established certified TRACE exit, `.srcstep`/`.traceevent` metadata and
breakpoint handler rather than grow a separate trace engine. The Unicode-first
text contract applies to source, values and host output. Existing Level B/G,
RexxScript, output-sink and namespace-extension behavior remains supported.
The linker and VM are outside this plan's edit authority; a discovered need to
change either requires a concrete review with Adrian before that edit. The
companion `TRACE()` BIF must share the observable Classic state. This is one
whole-instruction review, with coherent implementation commits and one grouped
normal correctness checkpoint per final code/test input set.

1. **LC-86-01 — forms and diagnostics (closed):** bare reset, alphabetic words
   and abbreviations, static and `VALUE` options, `?`/`!` prefixes and signed
   numeric settings preserve source, once-only evaluation and source-anchored
   errors. Document unsupported `SCAN` explicitly. Verify reference cases,
   AST shape and opt/no-opt compile and execution.
2. **LC-86-02 — shared activation state (closed):** option, prefix and inhibition
   state inherit into calls and restore to callers. `TRACE()` sees and changes
   the same state as the instruction; `OFF` and bare reset clear prefixes as
   specified. Document interactive/numeric behavior that is not implemented.
   Verify nested/recursive and host entry cases without a second state path.
3. **LC-86-03 — observable trace records (closed):** supported Classic and cREXX
   modes/sinks emit correct source, command, result and intermediate values;
   never print a fabricated or stale scalar. Record any missing Classic events
   as a specific divergence. Verify Unicode values and full
   `rxc`→`rxas`→`rxlink`→`rxvm` output in optimized and no-opt builds.
4. **LC-86-04 — execution controls (closed with recorded divergences):** `!` inhibits subsequent host
   commands while setting `RC` as the Classic contract requires. Probe and
   document the exact unimplemented interactive `?`, numeric skip/suppress
   and `SCAN` effects; ensure accepted options do not silently corrupt
   execution or report wrong values.
5. **LC-86-05 — coherent isolation and closure (closed):** use one trace option
   parser/state path and one event route across Level C and existing consumers.
   Keep source and diagnostic docs current; run focused checks while building,
   then one relevant normal Debug and Release Level C checkpoint on final
   code/test inputs, plus Level B/G/RexxScript isolation. Record concise
   evidence and commit the whole instruction only when LC-AC-77 passes.

1. **LC-STEP-86A (`LC-86-01–05`; complete):** reconcile the IBM/Regina reference,
   existing exit/runtime/metadata and Level C AST; inventory every legal form,
   error and producer/output path before implementation.
2. **LC-STEP-86B (`LC-86-01–02`; complete):** connect the Level C source
   node to the existing trace route, unify option parsing and activation state
   with `TRACE()`, and cover forms/errors in main and called bodies.
3. **LC-STEP-86C (`LC-86-03`; complete to approved scope):** deliver authored
   source, supported command/result/intermediate metadata and formatting
   through the existing trace handler, preserving optimized value safety;
   record absent label and expression records.
4. **LC-STEP-86D (`LC-86-04`; complete to approved scope):** finish command inhibition,
   probe interactive/numeric/SCAN behavior and record bounded divergences.
5. **LC-STEP-86E (`LC-86-01–05`; complete):** group focused and
   normal qualification, update architecture/reference docs and commit the
   coherent whole-instruction result. Keep all unverified criteria open.

**LC-STEP-86C value finding, repaired and qualified.** The
first linked `TRACE R/I` probe showed generated helper events and read
`RexxValue` objects as strings, printing false empty values. An authored
clause marker, typed Classic scalar events and handler `asString()` read now
print the actual value; generic helper events are filtered. A later NUL probe
found that capturing assignment value after call marshalling could print an
empty string; capture now precedes marshalling and prints `\x00`. The focused
linked opt/no-opt matrix checks numeric, NUL and Unicode values, compound
variables, PARSE, calls, frame restore and `TRACE()`; remaining event omissions
are listed in the TRACE requirements note. No VM or linker edit was made.

**LC-I-24 closure, 2026-10-06.** Final-input focused TRACE and existing
Level B/G/RexxScript isolation passed 41/41; normal Debug and Release Level C
passed 741/741 each. Linked opt/no-opt output is compared exactly by
`levelc_trace_values_*`; unsupported static SCAN and invalid dynamic options
have stable diagnostics. The focused `!C` host probe inhibited a command,
set RC zero, and `!` reenabled execution. Approved divergences: `?` updates
state without prompting; nonzero numeric options do not skip or suppress;
SCAN is rejected; `L` has no label-pass records; compound-name,
final-expression and some optimized expression records may be absent.
All emitted scalar values must be correct, including NUL displayed as
`\x00`; unavailable events are omitted. The `N`/`E`/`F` host-condition
classification remains coarse as documented. Full Level C, host-interface
and Release 1 criteria remain open under their own IDs.

**LC-I-25 INTERPRET plan — parked 2026-10-07, not implemented.** The
2026-10-06 research and proposed architecture below are retained for the
later decision, not approved for implementation. `LC-87-01–05` and
`LC-STEP-87B–87E` remain open; no further INTERPRET implementation is in the
current closeout. Parking does not close `LC-I-25`, `LC-AC-59/04` or the
Release 1 acceptance criteria.

**Vision and intended outcome recorded 2026-10-06.**
`INTERPRET expression` executes Unicode source built at run time with the
same Classic compiler grammar and the current invocation's pool, arguments,
numeric/ADDRESS/TRACE settings, condition policy and host context. It must
support complete nested instruction groups and recursive INTERPRET, and let
RETURN, EXIT, SIGNAL and local calls have their Classic effect. It must not
silently truncate an embedded NUL or turn malformed generated text into a
different valid program. Preserve the accepted static signed boundary for
ordinary external CALL, and existing VM/linker behavior unless Adrian
approves a specific change. This is one whole-instruction review; no
constant-only substitution or second partial Rexx interpreter is closure.

1. **LC-87-01 — source and diagnostics (open):** evaluate the expression once,
   interpret its exact Unicode text with implied trailing delimiter, and
   diagnose missing expression, invalid generated syntax, invalid Unicode or
   embedded NUL without truncation. Verify Regina/IBM cases, source location,
   SYNTAX trap, opt/no-opt and linked execution.
2. **LC-87-02 — complete generated instruction group (open):** accept the
   ordinary Level C instruction and expression grammar, complete nested
   `DO`/`SELECT` groups and nested INTERPRET. Reject label clauses inside
   interpreted text per the Classic reference; outer loops are inactive for
   LEAVE/ITERATE. Verify positive and malformed group matrices.
3. **LC-87-03 — caller context (open):** generated instructions use the active
   pool and ARG frame, preserve PROCEDURE/exposure rules, inherit and update
   NUMERIC, ADDRESS, TRACE and condition settings, and use the configured
   host/queue services. Verify nested local/recursive calls and host entry.
4. **LC-87-04 — control transfer (open):** RETURN/EXIT, SIGNAL to outer labels,
   local CALL and condition handlers have the same caller-visible effect as
   inserted clauses; crossed scopes and trapped SYNTAX clean up correctly.
   CALL ON ERROR/FAILURE/HALT may interrupt interpreted execution and resume
   it after the handler returns, while SIGNAL ends that execution. Verify
   first-label precedence, SIGL/RC, result presence and nested traps.
5. **LC-87-05 — lifecycle and integration (open):** generated code is compiled
   through one Level C compiler path, loaded/executed safely using a defined
   cache/ownership model, and available in the required installed product.
   Verify repeated and concurrent use, bounded memory, failure cleanup,
   linked image and Level B/G/RexxScript isolation. Keep full Level C and
   Release 1 criteria open until separately verified.

1. **LC-STEP-87A (`LC-87-01–05`; research checkpoint retained, parked):** inventory reference syntax,
   transfers/errors and current compiler/loader capabilities with bounded
   Regina and linked probes; record divergences before choosing semantics.
2. **LC-STEP-87B (`LC-87-03–05`; parked, decision open; depends on 87A):** prove a coherent
   compiler-to-execution design on existing APIs. Any new runtime compilation
   service, VM/linker edit or other architectural shift is a decision gate
   requiring Adrian's approval before implementation.
3. **LC-STEP-87C (`LC-87-01/02/05`; parked; depends on 87B):** connect runtime source
   to the existing Level C parser/lowerer and reusable generated-code loader;
   retain exact source length, errors and cleanup.
4. **LC-STEP-87D (`LC-87-03/04`; parked; depends on 87C):** integrate the current
   activation/frame, label transfers, local calls and condition lifecycle.
5. **LC-STEP-87E (`LC-87-01–05`; parked; depends on 87C/D):** run focused reference
   and lifecycle checks during development, then one coherent normal Debug
   and Release Level C checkpoint, update architecture docs and commit.

**LC-STEP-87A reference and execution-path findings, 2026-10-06.**
[IBM's INTERPRET rule](https://www.ibm.com/docs/en/cics-ts/6.x?topic=instructions-interpret)
processes one evaluated source group in the current invocation,
with an implied final delimiter; an incomplete DO/SELECT is a syntax error,
and label clauses are prohibited. Regina executes ordinary assignments,
ARG, complete DO groups, RETURN from the containing routine, SIGNAL to an
outer label and a first-instruction PROCEDURE in a called routine. A separate
Regina probe confirmed a generated local CALL with ARG and returned RESULT,
nested INTERPRET, and an empty generated string as a no-op
(`/tmp/crexx-interpret-local-call.H5I7yH`). Its
acceptance of a label inside interpreted text is an implementation quirk:
both the IBM rule and Regina's manual prohibit that form. A Regina probe of
LEAVE in interpreted text inside an outer DO raised Error 28 because the outer
loop is inactive. A malformed generated IF reached SIGNAL ON SYNTAX with RC
14 and SIGL at the authored INTERPRET clause. Regina silently truncates
generated source at embedded NUL; Level C must instead inspect the exact
length and issue a controlled source error under its Unicode text contract.
IBM's [condition reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=reference-conditions-condition-traps)
also requires an enabled CALL ON ERROR/FAILURE/HALT
to interrupt and later resume interpreted execution; SIGNAL abandons it.
The bounded Regina ERROR probe printed `first`, `handler 1 2`, `second`,
`after` in that order (`/tmp/crexx-interpret-calltrap.XXXXXX.log`), confirming
that an external command condition resumes at the next generated clause.
Reference probes and outputs are retained in `/tmp/crexx-interpret-reference.*`,
`/tmp/crexx-interpret-syntax.*`, `/tmp/crexx-interpret-procedure.*`,
`/tmp/crexx-interpret-leave.*`, and `/tmp/crexx-interpret-nul.*`.

The current Level C parser recognizes `INTERPRET expression` but lowering
rejects the source-only node. `rxclib` and `rxaslib` exist, but there is no
runtime source-to-RXBIN service or fragment compiler mode. The existing
`loadmodule()` operation can load a provider while bytecode is executing:
an isolated Debug probe compiled a fragment provider with the ordinary tools,
loaded it from a host procedure, and changed the host's shared
`RexxVariablePool` (`/tmp/crexx-interpret-load-run.m9iNHW`, output `changed`).
That proves shared object access, not caller-frame semantics. The current
compiler also accepts a physical Level C source file containing NUL and
silently stops at that byte: `/tmp/crexx-interpret-nulsource.EO6rmc` compiled
with exit zero, but its RXAS contained only the preceding SAY. The runtime
source service needs an exact-length NUL check before the normal compiler;
the general file-source scanner issue stays visible under `LC-AC-72/08`.
The active compiler body keeps its pool, local-label associations and handler branches
in generated locals and same-procedure `FRAME_*` nodes; a late-loaded method
cannot directly branch to those labels, RETURN from that body or call its
local labels. The VM loader offers no public per-module unload operation, so
uncached distinct source texts also remain resident in one VM context.

**LC-STEP-87B architectural proposal — unapproved and parked.** The proposed compiled
route is a runtime source-to-RXBIN service using the existing Level C grammar
and assembler, a compiler-generated fragment entry that receives the current
activation/pool/context, and a generated caller-side transfer dispatcher.
The first implementation should use a dedicated child-process compiler
service with argument-vector invocation and isolated temporary inputs:
`rxcmain()` still has process-exit error paths and is not an embeddable
per-call API, while `rxclib`/`rxaslib` can back that helper. No source text
or dynamic path is interpolated into a shell command. The fragment provider
is selected by a content and caller-contract key so multiple interpreted
texts cannot accidentally bind the same late-loaded interface factory.
An isolated two-provider Debug probe loaded both providers and selected them
by distinct `match` keys in reverse order, printing `B`, `A`
(`/tmp/crexx-interpret-selector.AResi9`). This proves the current factory
selector can disambiguate loaded fragments; it does not prove the complete
INTERPRET frame or module reclamation contract.
The fragment must report RETURN/EXIT/SIGNAL and invoke caller-local labels
through that dispatcher rather than create a second interpreter or treat its
own VM call frame as the Classic invocation. It must propagate conditions and
configuration changes through the shared activation and restore caller state
on escape. In particular, PROCEDURE can replace the visible pool during
interpreted execution, so the parent body must reload its pool link after
the fragment returns; merely sharing the pool object is insufficient.
Activation-local NUMERIC/ADDRESS/TRACE settings can use the existing shared
activation, but caller-frame SIGNAL handlers require an explicit rebind after
fragment execution. A content-addressed cache avoids recompiling repeated text; a
module-lifetime policy must handle distinct texts and concurrent contexts.
This adds a runtime compilation service and a new compiler fragment mode;
neither is covered by prior frame/CALL approvals. A truly reclaimable cache
would additionally need a reviewed VM module-lifecycle API. Adrian must
approve the concrete architecture before product edits. No linker change is
proposed. A partial static substitution, unbounded silent cache or separate
statement evaluator would not close `LC-I-25`.

**RexxScript alternative requested for review, 2026-10-06 — unapproved and parked.**
The shipped RexxScript evaluator is a useful model for per-instance source,
program-counter, block-stack, error and resource ownership. It also shares
`RexxValue`, `RexxVariablePool` and compatible BIF implementations with Level C.
Its current parser splits statements itself, uses a separate string-variable
array mirrored to a sandbox pool, evaluates arithmetic left-to-right, captures
SAY output, and returns a result array. Its deliberate sandbox omits Classic
CALL, ARG, PROCEDURE, ADDRESS, PARSE, NUMERIC, TRACE, conditions, compound
symbols and real INTERPRET. In contrast, interpreted Level C text must use
the already qualified Level C grammar and *live* activation/pool/host state,
and must transfer or resume control in the caller's compiled body. Passing
the text directly to RexxScript would change language behavior and cannot
close `LC-I-25`.

| Candidate | Reuse | Main new work and risk | VM/linker impact |
| --- | --- | --- | --- |
| Compile generated Level C text to a late-loaded fragment | Reuses the existing Level C parser, validator, lowerer, BIF paths and bytecode execution; isolated loader/factory probes pass. | Runtime compiler service; caller-frame bridge for pool replacement, local CALL, RETURN/EXIT, SIGNAL and resumable CALL ON; safe fragment cache and reclamation. | Linker unchanged; targeted VM module-lifecycle support may be required for bounded residency. |
| RexxScript-inspired Classic interpreter | Reuses the evaluator's instance/PC/block ownership pattern and shared `rxfnsc` objects. Avoids loading a distinct bytecode module for each source. | Replace its parsing with the Level C grammar and implement the entire Classic statement/condition/control matrix against the live activation. Maintaining semantic parity with the compiled lowerer creates a second execution path unless both are refactored to one shared execution plan. | Could leave VM/linker unchanged, but requires a major compiler/runtime architecture shift and RexxScript sandbox isolation proof. |
| Text substitution or direct RexxScript delegation | Little new machinery. | Fails dynamic syntax, current-frame transfers and/or established Classic semantics. | None; rejected as completion route. |

Before either implementation, Adrian's choice must cover which semantic engine
owns interpreted code and whether the VM may gain module reclamation. If the
RexxScript-inspired route is selected, a bounded design proof must show how
the Level C parser feeds an execution plan shared with compiled lowering, or
explicitly justify and qualify the second full Classic executor. The existing
RexxScript sandbox remains a separate product contract in either route.

**LC-STEP-88A closed-instruction review.** The former receipts remain evidence
for their tested forms; the changed character contract affects instruction
ownership as follows:

| Instruction | Unicode-first effect | Disposition |
| --- | --- | --- |
| SAY | Text from mapped byte values and non-Latin-1 scalars now goes through host text output; exact raw-byte expectations no longer describe the contract. | Closed under LC-AC-57/88D-1; length-aware callback, expression order and new host/default Unicode proof retained. |
| DROP | Indirect words and compound substitutions may contain Unicode; the pool uses the activation's text configuration for splitting and classifying subsidiary words. | Closed under LC-AC-62/88D-2; direct/indirect ordering and invalid-word policy retained. |
| Assignment | Scalar payloads and compound-tail substitutions can contain Unicode; the pool stores `RexxValue` without a byte conversion. | Closed under LC-AC-63/88D-3 for visible Unicode values/tails; RHS-order proof retained. |
| NOP | No scalar or character operation. | Closed receipt retained. |
| OPTIONS | Runtime words remain unrecognized and evaluated expression value is not scanned; no profile word is added. | Closed receipt retained; obsolete profile wording will be removed. |
| IF and SELECT | Exact logical `0`/`1` checks and branch/arm ownership do not use byte indexing. | Closed structural receipts retained; shared expression/condition work remains open. |
| DO | Loop setup, state and numeric checks are independent of character units; compound names share the reopened pool audit. | Closed loop receipt retained; pool, NUMERIC and condition obligations remain open. |
| LEAVE and ITERATE | Named/unnamed transfer and loop-state mechanics do not use character units. | Closed receipts retained; shared DO/pool obligations remain open. |

ARG and PARSE are already open. Their pending configuration proof changes to
Unicode codepoint positions and mapped conversion values. No previously
closed receipt proves the new high-character output or byte conversion rules.

Expression grammar, BIFs, variable semantics, source/character configuration,
conditions and host adapters are cross-cutting foundations under
`LC-AC-01/04/06/08`; they are not silently completed by an instruction row.
Before closing any `LC-I-*`, reconcile its reference-obligation rows and the
parser's accepted forms. An unresolved instruction-specific dependency keeps
the row open; shared foundations retain their independent acceptance criteria.

### 2026-10-04 delivery correction and plan

**Vision and intended outcome.** Keep one length-aware SAY output interface
across the VM and native hosts, including compiler exits and RXPA, so valid
NUL text cannot be truncated by an obsolete callback. Close the SAY
instruction for its own evaluation, output, diagnostic and host contract,
without treating every cross-cutting BIF, external-call, TRACE or SIGNAL
feature as a SAY prerequisite. Preserve those full Level C obligations under
their existing criteria and instruction rows. Then audit and complete the
previously implemented instruction families, including DO, to establish a
simple, evidenced baseline before beginning SIGNAL. Adrian explicitly
approved this order and the callback API removal on 2026-10-04.

**Acceptance:** `LC-AC-57` (whole SAY), `LC-AC-59` (instruction receipts),
`LC-AC-60` (single callback) and `LC-AC-61` (prior-instruction baseline)
above remain independently checkable; overall `LC-AC-04/06/08` remain open
until their full contracts are proved. Missing BIF services and external
resolution are cross-cutting open work, not SAY-specific exceptions. Classic
trap delivery and label/activation flow belong to `LC-I-23 SIGNAL` and its
dependent CALL/condition contracts; `LC-STEP-63T` below is retained as an
unapproved design proposal for that later review, not a SAY closure gate.

**Implementation steps:**

1. **LC-STEP-63E (LC-AC-60; complete; depends on 63B):** replace the
   terminated callback surface in VM, RXVML, RXPA and compiler exits with the
   existing byte-span signature. Migrate in-tree registrations, remove the
   legacy-only test branch, update host documentation, build and run focused
   output/context/plugin checks; commit this interface increment.
2. **LC-STEP-63F (LC-AC-57/59; complete; depends on 63E):** finish the SAY-only
   reference/parser/AST/output/error audit, retain focused Regina, opt/no-opt,
   full toolchain and normal correctness evidence, then close `LC-I-01` and
   `LC-AC-57` only if no SAY-specific gap remains; commit its receipt.
3. **LC-STEP-62B (LC-AC-61/58/59; depends on 63F):** inventory all earlier
   touched instruction rows, inspect implementation shape and duplication,
   reconcile complete reference and parser obligations, and work through
   their existing `LC-STEP-64` onward rows in reviewable whole-instruction
   checkpoints. Record closed and still-open rows with exact evidence. Do not
   begin `LC-I-23 SIGNAL` until this baseline is clear and its architecture
   gate is approved.

2026-10-04 prior-instruction audit at the DROP checkpoint (`7f0f08b92`):
the parser grammar, Classic compliance reference, lowerer validation and
lowering, worklist crosswalk and existing tests were reconciled. This is a
baseline inventory, not closure of the open rows. The current code has two
parallel main/procedure validators and lowerers for most instructions. Each
whole-instruction review must reduce that duplication where practical without
losing the genuinely different routine rules.

| Earlier instruction | Implementation shape at the 2026-10-04 audit | Open review or work recorded then |
| --- | --- | --- |
| SAY, DROP | Whole instruction closed in LC-I-01/02 | Shared TRACE, SIGNAL and host text services remain separate open criteria. |
| Assignment | Scalar and one-component compound targets use different compiler-selected pool methods; stem and multi-component targets are rejected | Evaluate RHS before pool substitution, then use one pool-owned assignment operation; prove stem/exposure and calls that mutate tail variables. Active LC-I-03. |
| NOP | Childless parser node and one no-op lowerer in main/procedure, including nested bounded arms | Reconcile clause and TRACE hooks and malformed source before LC-I-04 closure. |
| OPTIONS | Parser emits REXX_OPTIONS; acceptance of the node is broad, with option handling in programme setup | Inventory option words and unknown policy, test source/configuration lifecycle. |
| IF, SELECT | Guarded trees lower to canonical branches; validation/lowering dispatch repeats between main and procedure | Audit every legal arm and diagnostic, nearest ELSE, ordering and no-match lifecycle; simplify common dispatch. |
| DO | Many proven bounded header slices, but header validation and lowering independently interpret positional REPEAT/condition/body children and synthesize loop blocks | Normalize one checked header, then compare canonical construction with a dedicated node/emitter route for total simplicity. Remove arbitrary count bound; prove all legal header combinations and associations. Architecture decision remains gated. |
| LEAVE, ITERATE | Hidden loop targets and canonical transfers support bounded unnamed and named forms | Audit targets, nested loops, invalid placement and step timing with the complete DO representation. |
| ARG, PROCEDURE | Local routine slice with direct ARG and limited EXPOSE/pool lifecycle | Complete templates, omitted arguments, direct/indirect exposure and routine context. |
| CALL, RETURN, EXIT | Internal calls and bounded return/exit paths; main and procedure rules differ | Complete resolution, optional values, traps and activation/fallthrough lifecycle; SIGNAL architecture may affect these rows. |
| PARSE | Direct word templates and VM parseplan are separate execution paths; VAR/VALUE source subset | Design one execution representation before completing sources, patterns, dynamic positions and comma templates. |

This records why the bounded DO acceptance checks do not close LC-I-08. The
order stays assignment, NOP, OPTIONS, IF, SELECT, DO, then transfers and
routines; DO's representation review starts before its implementation, so
the earlier AST risk is visible rather than hidden by passing slice tests.

**LC-I-03 assignment plan — vision and outcome.** One parsed Classic
assignment target produces one pool operation with full scalar, stem and
compound semantics. The compiler evaluates the authored expression before
the pool substitutes the target name, preserving its source location; the
shared pool implements the final write for Level C and RexxScript consumers.
This completes the instruction without adding a special case for each tail
shape.

The first Regina whole-instruction probe contradicted the review's earlier
pre-RHS capture assumption: `items.key.part=change()` wrote the name derived
from `key` and `part` *after* `change()` mutated them. The previous compiler
lowering captured one tail component before the RHS. The correction follows
the observed Classic order and makes the compiler path smaller; it does not
introduce a new language rule or runtime interface.

1. **LC-STEP-65A (LC-AC-58/63; complete; depends on 62B):** retain a whole-instruction
   Regina fixture for scalar/stem/compound names, multi-component substitution,
   default reset, exposure and RHS side effects. Lower every valid assignment
   through the existing `setSymbolValue` operation after evaluating the RHS;
   remove compiler-only tail materialization and preserve source-invalid
   diagnostics. Build and run
   focused compiler, pool and cross-consumer checks; commit the implementation.
2. **LC-STEP-65C (LC-AC-63; complete; depends on 65A):** accept an omitted assignment
   expression as an empty string in the Classic grammar, preserving the
   authored source anchor and existing invalid-target diagnostics. Add a
   reference case in scalar, stem, compound and nested contexts, plus parser
   recovery checks; build and run focused tests, then commit this grammar
   increment. This is within Adrian's approved complete-Classic-assignment
   scope, not a new cREXX syntax choice.
3. **LC-STEP-65B (LC-AC-59/63; complete; depends on 65A/65C):** reconcile all parser target
   forms and reference obligations, check optimized/no-opt tree/source and
   runtime equivalence, normal Level C correctness and linked toolchain output;
   record exact evidence and close LC-I-03 only if its contract is met.

2026-10-04 LC-STEP-65A assignment simplification: the old compiler-side
stem/tail splitter and its single-component guard are gone. All validated
targets lower through `RexxVariablePool.setSymbolValue` after RHS evaluation;
the method already handles scalar, stem default, arbitrary compound tails and
exposed aliases. The Regina whole-instruction probe output is retained at
`/tmp/crexx-assignment-regina.6uqdnk`; the new fixture matches it in optimized
and no-opt modes, including the side-effecting RHS that distinguishes the
old incorrect order. Invalid numeric-start target `1bad` retains `31.1` at
line 2. Release core build passed (`/tmp/crexx-assignment-build2.lVfDyb`),
focused compiler/pool tests passed 10/10
(`/tmp/crexx-assignment-focused2.zYYiGi`), and RexxScript Runtime/Compat
tests passed 4/4 (`/tmp/crexx-assignment-rexxscript.dpfRmX`). Normal Level C,
source/canonical AST and linked delivery remain for LC-STEP-65B; the whole
 instruction is still open at this implementation checkpoint.

2026-10-04 reference reconciliation found an additional valid assignment
form after STEP-65A: Regina executes `name=` as an empty-string assignment,
also documented for IBM VM REXX. The current parser reports `21.1` and
`PARSE_FAILURE` for it (`/tmp/crexx-assignment-empty.75Bo2X`); the reference
probe is `/tmp/crexx-assignment-empty-regina.9wjBsC`. LC-STEP-65C keeps
LC-I-03 open until this form is implemented and qualified.

2026-10-04 LC-STEP-65C empty-assignment implementation: the grammar retains
an `ASSIGN` with only its target for `name=`, including a numeric-start bad
target; the lowerer constructs one empty `RexxValue` and uses the same pool
write. The permanent fixture covers scalar, compound, stem, IF and local
procedure contexts. Its output matches Regina
(`/tmp/crexx-assignment-regina3.7hjdiG`); invalid targets with and without
RHS both report `31.1`. Release product build passed
(`/tmp/crexx-assignment-empty-build.IzM8RA`), focused checks passed 4/4
(`/tmp/crexx-assignment-empty-focused.ot6dhD`), the normal Release Level C
suite passed 316/316 (`/tmp/crexx-assignment-empty-levelc.8J5Ayw`), and the
linked toolchain output matched Regina's 216 bytes
(`/tmp/crexx-assignment-linked3.r8sqKM`). The AST debug proof retains the
authored source anchor and empty string at scalar, compound, stem and local
sites (`/tmp/crexx-assignment-empty-tree.XuTvlU`). The final tree test adds
 the scalar/local anchor assertions; the whole-instruction closure review is
 still LC-STEP-65B.

2026-10-04 LC-STEP-65B whole-assignment closure at implementation commit
`0a746c981`: the grammar has one valid assignment production with an RHS and
one without; numeric-start recovery variants preserve `31.1`. Contextual
keyword targets, scalar/stem/arbitrary-component compound names, empty RHS,
main/IF/DO/local execution, BYTE value length, stem default reset, exposed
aliases and the Regina RHS-before-substitution case are in the single
instruction fixture. `levelc_pool_statement_supported` accepts exactly those
shapes and one shared `setSymbolValue` lowerer handles them. The canonical
tree has no surviving Level C-only nodes; its source anchors include the
side-effecting assignment and empty main/local writes. Optimized/no-opt
execution matches Regina; the linked 216-byte result is identical
(`/tmp/crexx-assignment-linked3.r8sqKM`). The Release Level C sweep passed
316/316 on the implementation inputs
(`/tmp/crexx-assignment-empty-levelc.8J5Ayw`); the later, stricter
empty-assignment tree assertion passed separately without a product-code
change (`/tmp/crexx-assignment-empty-anchor-retest.UCsFUv`). Focused pool
and RexxScript checks from 65A remain valid because their code/test inputs
 did not change. This closes `LC-AC-63` and `LC-I-03` only; `LC-AC-04/06/08/61`
 and the wider TRACE/SIGNAL, host text and external API contracts stay open.

**LC-I-04 NOP plan — vision and outcome.** Preserve one source-anchored NOP
through the normal canonical emitter and make invalid same-clause tails fail
with the Classic diagnostic. The existing no-op behavior and broader nested
control flow tests are the positive baseline. The pending single-activation
label design and TRACE hooks are shared capabilities that apply to every
statement; they remain visible open work rather than NOP-specific omissions.

1. **LC-STEP-66A (LC-AC-64; complete; depends on 65B):** compare childless, selected
   IF/SELECT, DO and local NOP with Regina; add a source-level `21.1` recovery
   path for extra same-clause tokens, including variable and nonvariable
   starts. Retain a focused invalid-source regression and source-tree check;
   build and commit the parser repair.
2. **LC-STEP-66B (LC-AC-59/64; complete; depends on 66A):** reconcile the grammar and
   reference, check the existing positive opt/no-opt/linked cases, new negative
   diagnostics, canonical NOP/source anchors and the relevant normal Level C
   suite; close the instruction-specific row only if all pass. Keep shared
   label/TRACE obligations open with their own evidence.

2026-10-04 NOP audit reproducer: `NOP extra` compiled with a raw NOP node and
silently dropped `extra` (`/tmp/crexx-nop-audit.5EO6LD` and
`/tmp/crexx-nop-invalid-tree.rwYLGW`), while Regina reports `21.1`
(`/tmp/crexx-nop-reference.wClRwy`). `mark: nop` currently fails compilation
because the lowering plan requires a `PROCEDURE` after a local label; Regina
runs it. That label/fallthrough defect is shared across statement kinds and
remains open under `LC-AC-08` and the pending SIGNAL architecture gate. It
 does not authorize bypassing or weakening that later work.

2026-10-04 LC-STEP-66A NOP parser repair: the grammar now creates a source
`21.1` diagnostic for a variable or nonvariable token after `NOP` in the same
clause, preserving the tail for recovery. The canonical childless NOP and its
source anchor are unchanged. Invalid `NOP extra` and `NOP 'quoted'` now fail
at their authored tokens in the permanent fixture. Focused nested/SELECT/
WHILE/UNTIL, negative and tree tests passed 16/16
(`/tmp/crexx-nop-focused.oggHWy`); the dedicated NOP source-anchor tests
passed 4/4 (`/tmp/crexx-nop-tree-tests.r3M4Pr`). The Release Level C suite
passed 318/318 (`/tmp/crexx-nop-levelc-suite.9Ykdll`) and the linked
toolchain matched Regina's 20 bytes (`/tmp/crexx-nop-linked.CFAj8d`). The
 shared labeled-clause and TRACE obligations remain open.

2026-10-04 LC-STEP-66B NOP closure at implementation commit `c7469d9da`:
the only valid NOP parser production is childless; two recovery productions
consume and diagnose invalid variable/nonvariable tails with `21.1` at the
first extra token. The same codepath rejects keyword, numeric and bracket
starts (`/tmp/crexx-nop-tail-audit.WR8ZSJ`). One source-anchored canonical
NOP is emitted, with main, IF and DO, plus local anchors checked by the tree
test (`/tmp/crexx-nop-tree.TiFq9z` and
`/tmp/crexx-nop-tree-tests.r3M4Pr`). Existing SELECT, WHILE and UNTIL
fixtures also execute nested NOP in opt/no-opt; focused tests passed 16/16.
Regina, optimized/no-opt and linked output agree on `before`, `middle`,
`after`, with exact linked proof at `/tmp/crexx-nop-linked.CFAj8d`. The
normal Release Level C suite passed 318/318 on the implementation inputs
(`/tmp/crexx-nop-levelc-suite.9Ykdll`). No NOP-specific runtime helper or
case-by-case emitter path was added. This closes `LC-AC-64` and `LC-I-04`;
the shared label/fallthrough, TRACE, condition and host text contracts
remain open under `LC-AC-04/08` and their instruction rows.

**LC-I-08 DO plan — vision and outcome.** Replace the accumulated DO header
cases with one checked description and one supportable loop-state path, while
keeping the compiler's established canonical `DO` emitter and its transfer
associations if that route survives the decision review. Every valid Classic
DO form and timing rule must remain visible in the whole-instruction contract
`LC-AC-65`. The shared pool must own control-name resolution, as it now owns
assignment and DROP. A count representation must avoid a compiler-specific
32-bit cutoff; it must preserve the numeric values the chosen Classic profile
can represent. This work precedes SIGNAL because loop AST and activation
semantics are a high-risk foundation. At this DO checkpoint, IF, SELECT and
OPTIONS were still open.

1. **LC-STEP-70A (LC-AC-61/65; complete review; depends on 66B):** inventory
   the DO parser and current validator/lowerer/emitter paths, run focused
   reference probes, compare a shared runtime-state design using canonical
   `DO` with a dedicated AST/emitter node, and record the architecture gate.
   This is a read-only implementation review, not DO closure.
2. **LC-STEP-70B (LC-AC-65; complete; depends
   on 70A):** normalize source DO headers once into a checked description used
   by validation and lowering. Preserve source anchors, emitted loop-target
   association and invalid-form diagnostics; add structural assertions where
   they distinguish ownership or control timing.
3. **LC-STEP-70C (LC-AC-65; approved, complete; depends on 70B):** implement the
   chosen common loop-state service in `rxfnsc`, route counted and controlled
   forms through it and the shared pool, and remove redundant compiler header
   branches and arbitrary `int` conversion. Qualify focused reference,
   runtime, AST, optimizer and transfer cases in reviewable commits. Preserve
   Classic numeric display scale for controlled start and step through one
   shared `RexxValue` operation; reference probes show that generic decimal
   addition currently strips it (`/tmp/crexx-do-scale.q3LHLn`).
4. **LC-STEP-70D (LC-AC-59/61/65; complete; depends on 70C):** reconcile all
   valid parser forms and Classic errors, main/local/nested execution,
   optimized/no-opt and canonical/source trees, normal Level C correctness,
   RexxScript consumers of changed shared services and linked delivery. Close
   LC-I-08 only when the whole contract passes; return to the skipped
   instruction rows before the SIGNAL architecture decision.

2026-10-04 LC-STEP-70A evidence and open questions: `levelc_do_supported`
and `levelc_lower_do` independently decode the positional source children.
`levelc_controlled_header_supported` accepts only scalar targets and is
called again during lowering. The latter constructs captures, synthetic
WHILE/UNTIL and a `BLOCK_EXPR` for sequencing, while
`rxcp_remap_create_controlled_do` and `rxcp_emit_flow.c` already provide a
canonical loop with LEAVE/ITERATE association and cleanup. `RexxValue` count
methods and the literal fast path convert to `int` and reject values above
2147483647. A Regina probe of `DO a.i=1 TO 2` that changes `i` in the body
shows the later step writes the newly substituted control name: output
`body=1` then `after=1|11`; the current compiler rejects the header
(`/tmp/crexx-do-design.sjBeq8`). A
Regina `DO 2147483648` probe reports `26.2`, establishing that Regina itself
has a count limit, but this does not justify a compiler-imposed limit as the
portable language contract. The exact reference/profile range and overflow
diagnostics remain to be resolved in 70C. The design comparison and proposed
invariants are in the [architecture review](../../../compiler/docs/levelc_working_architecture.md#2026-10-04-do-architecture-decision-proposal).

Adrian approved the checked-header, shared `rxfnsc` loop-state service and
existing canonical DO emitter architecture on 2026-10-04. Regina timing
probes (`/tmp/crexx-do-timing.Vcjwp5`) confirm that start and TO/BY/FOR
expressions see the pre-loop control value and execute in written order,
that UNTIL exits before stepping, and that ITERATE reaches the UNTIL check.
LC-STEP-70B records one checked descriptor per source DO in the lower plan;
the validator, lowerer and named transfer checks use it instead of decoding
the header repeatedly. It caches controlled literal classifications and the
source-ordered modifier nodes; no new emitter node or runtime behavior was
added. The focused DO set passed 38/38
(`/tmp/crexx-do-header-focused2.sviu4y`) and the Release Level C suite passed
318/318 (`/tmp/crexx-do-header-levelc3.NzQvDI`) on this code input. LC-I-08
and `LC-AC-65` remain open for the shared loop-state and whole-instruction
qualification.

2026-10-04 LC-STEP-70C implementation: `RexxDoState` owns source-ordered
header captures, exact decimal-string count progress, entry checks, and pool
control stepping. The lowerer calls that state from the existing canonical
DO entry/end positions, accepts compound controls, and removes the old
per-form captures, literal special cases and 32-bit count conversion. The
shared `RexxValue.controlNumericAdd` keeps Classic decimal display scale
for controlled initialization and stepping. Regina reference output for
compound tail changes and numeric scale matches optimized/no-opt fixtures;
the DO matrix passed 93/93 (`/tmp/crexx-do-matrix-final.Mx66qp`), new scale
fixture 2/2 (`/tmp/crexx-do-scale-fixture-test.SNVAs1`), and final shared
`RexxValue`/`RexxDoState` plus RexxScript consumers 8/8
(`/tmp/crexx-do-consumers-final.iqVG98`). The linked `rxc`/`rxas`/
`rxlink`/`rxvm` scale fixture matches Regina byte for byte
(`/tmp/crexx-do-linked.TRn3C1`). The final exact-input Release Level C
suite passed 321/321 (`/tmp/crexx-do-levelc-final.QVsi4e`). The source
and lowered tree inspection in `/tmp/crexx-do-tree.p8XeAP` shows separate
state objects in nested canonical loops, with `ITERATE outer` bound to the
outer generated loop target. The whole-instruction LC-STEP-70D audit remains
required before
`LC-AC-65` or LC-I-08 can close.

LC-STEP-70D first audit finding: the three duplicate controlled modifiers
(`TO`, `BY`, `FOR`) reached the checked-header validator but fell through to a
generic unsupported-shape result, while Regina reports `27.1` for each.
An empty `TO` operand also recovered by consuming a following body expression
and compiled. The reproductions and raw tree are retained at
`/tmp/crexx-do-invalid-audit.DKBebx` and
`/tmp/crexx-do-to-explicit-tree.4ihzVy`. The repair emits `27.1` at each
repeated modifier and `35.1` for empty TO/BY/FOR/WHILE/UNTIL operands, keeping
the following body clause in the source tree. Regina uses its generic `64.1`
parser identity for the latter; the Level C identity follows the existing
standard invalid-expression mapping. They were defects, not approved
compatibility exceptions. The audit also removed the now-unreferenced bounded
`RexxValue` count methods
and one-line control wrappers. Their 32-bit contracts are superseded by
`RexxDoState`; retained older worklist receipts remain historical evidence.

2026-10-04 LC-STEP-70D closure receipt: the parser forms were checked against
the grammar (`compiler/rxcpcgmr.y`): grouping, counted, FOREVER, bare
WHILE/UNTIL, counted plus WHILE/UNTIL, and controlled scalar/compound with
TO/BY/FOR in any legal order and optional WHILE/UNTIL. The existing focused
runtime/tree matrix covers these in main, nested and local procedure contexts,
including LEAVE/ITERATE, zero-entry and end-step behavior. New Regina probes
and optimized/no-opt fixtures cover all six modifier orders and a compound
tail changed during header evaluation (`/tmp/crexx-do-order.2HqazG`,
`/tmp/crexx-do-tail-header.xKk77V`). Invalid-source fixtures now cover
duplicate and empty modifiers, missing END and illegal END names in addition
to existing numeric/logical/transfer errors; parser-mode tests verify that
diagnostics and the following body survive recovery. The lowered/source tree
inspection (`/tmp/crexx-do-closure-tree.ZVjME7`) and the boundary test show distinct
nested state and canonical target ownership. The exact-input Release Level C
suite passed 335/335 (`/tmp/crexx-do-closure-final-suite.0Uybf3`), shared
value/DO-state/RexxScript consumers passed 8/8
(`/tmp/crexx-do-close-consumers.bdUe3h`), and a linked four-toolchain run of
all six orders matched Regina byte for byte
(`/tmp/crexx-do-order.2HqazG`). `LC-AC-65` and LC-I-08 close. The memory cost
of representing an extreme exponential count, and the decNumber source
exponent limit, are recorded in the architecture document; neither is an
approved language exception. Shared NUMERIC, condition, TRACE and host
profile behavior remains open under its separate criteria and instruction
rows. At this checkpoint LC-I-05 OPTIONS was next, followed by IF and SELECT
before SIGNAL.

**LC-I-05 OPTIONS plan — vision and intended outcome.** A Classic programme
must be able to request cREXX source-language defaults before parsing and to
execute `OPTIONS` at its authored location with Classic expression and
unknown-word behavior. The first clause currently carries both roles. The
source pre-scan consumes a static header for level, comment and numeric
settings, but the Level C grammar accepts only a list of symbols and the
lowerer discards every parsed `REXX_OPTIONS` node while emitting fixed
`levelb comments_dash numeric_classic` and imports. Close the whole instruction
with a single checked ownership path for file-level settings and a source-
anchored executable path. Preserve Level B, RexxScript, BYTE/UTF8 and the
generated canonical emitter. Do not treat unrelated TRACE/condition or host
configuration work as closed by this instruction.

1. **LC-STEP-67A (LC-AC-59/66; complete; depends on DO closure):** inventory
   pre-scan, Level C grammar, source and canonical trees, validation, emitted
   options, runtime configuration and tests. Compare the expression/word rule
   with the IBM reference and Regina, including an empty operand; identify
   duplicate or discarded behavior and record a concrete architecture choice.
2. **LC-STEP-67B (LC-AC-66; approved 2026-10-04; depends on 67A):** Adrian
   approves the source/runtime split, the empty-operand dialect choice, the
   cREXX policy for IBM's EBCDIC DBCS-specific ETMODE/EXMODE words, and the
   Level C line-comment policy where `comments_slash` collides with the
   Classic `//` remainder operator, and whether `numeric_common` is legal in
   a Classic-precedence source. Approval
   is required before compiler or runtime architectural changes. The proposed
   split keeps static, leading `level*`, comment and numeric words as file
   directives in the existing pre-scan; parses every Level C `OPTIONS` clause
   as a Classic expression; and lowers it to one shared `rxfnsc` processor
   service that evaluates at runtime, handles applicable words in order and
   ignores unknown words. Generated Level B options/imports remain a private
   canonical header; source choices must not be silently overwritten.
3. **LC-STEP-67C (LC-AC-66; complete at `8cd24c317`; depends on approved 67B):** implement the
   reviewed parser, validation, lowering and shared processor service in
   reviewable commits, with focused source/AST, opt/no-opt, runtime and
   negative tests. Keep the byte-length value model and profile ownership
   explicit; avoid separate per-word lowering branches.
4. **LC-STEP-67D (LC-AC-59/61/66; complete; depends on 67C):** reconcile all
   legal and invalid forms, inspect source and canonical ownership/anchors,
   compare reference output, check source/CLI and option ordering, run focused
   Level B/RexxScript consumers and one relevant normal correctness suite,
   prove linked delivery, update architecture/reference docs and close LC-I-05
   only on a complete evidence receipt.

2026-10-04 LC-STEP-67A audit: the pre-scan's recognized static words are the
six `level*` selectors, hash/slash/dash comment enable/disable pairs, and
`numeric_common`/`numeric_classic`; `srcmap` has a separate source marker.
`floats_binary`/`floats_decimal` are validated later by Level B but not by the
pre-scan, and are not yet an established Level C source contract. The Level C
parser forces Classic numeric mode and builds `REXX_OPTIONS` from zero or more
bare symbols; its lowering plan accepts and then drops those nodes, and
`levelc_build_options` supplies a fixed Level B header. The IBM TSO/E REXX
reference specifies `OPTIONS expression`, evaluation into uppercase words in
order, and ignoring unknown words. Its recognized ETMODE/NOETMODE and
EXMODE/NOEXMODE words are tied to EBCDIC DBCS shift-out/shift-in behavior,
with ETMODE subject to a first-instruction rule. Regina executes dynamic
variables and expression concatenation, ignores unknown words, and accepts
bare `OPTIONS` (`/tmp/crexx-options-audit.JlwjGf`); IBM's grammar requires an
expression. Neither source/runtime ownership nor the dialect difference is
silently resolved by this audit. The present code does not have a Level C
runtime options service. The Level C scanner currently handles only nested
block comments, irrespective of the pre-scan comment flags. A source probe
with `options levelc numeric_common comments_dash` still parses a following
`--` line as minus operators (`/tmp/crexx-options-current.E1m8tV`). The same
probe shows the Level C parser forces Classic numeric mode, while a dynamic
`OPTIONS word` and an expression form have no lowered execution at all. Since
`//` is the Classic remainder operator, enabling Level B's slash comments
would change a valid Level C expression; that conflict needs an explicit
language choice. `numeric_common` is likewise recognized by the pre-scan but
overwritten when `rexcpars` forces Classic numeric mode, so it also requires
an explicit Level C policy. Placement probes show nested `OPTIONS word` fails
only during lowering and a local-routine `OPTIONS` is rejected by its
statement validator (`/tmp/crexx-options-placement.hpkypp`). LC-STEP-67B is
the next gate.

2026-10-04 LC-STEP-67B decision: Adrian approved the recommended set. A
leading bare-word source header selects compiler level and compatible
file-level lexical settings; every Level C OPTIONS clause remains an
executable expression evaluated at its source position. Bare OPTIONS is a
Regina-compatible no-op. Unknown runtime words, including IBM's EBCDIC
DBCS-specific ETMODE/EXMODE family in the then-proposed cREXX BYTE/UTF8
profiles, are ignored. The later Unicode-first decision supersedes those
profile words without changing the unknown-runtime-word policy.
Level C defaults to Classic block comments; explicit hash/dash line-comment
switches may be used. `comments_slash` is rejected so `//` remains Classic
remainder, and `numeric_common` is rejected so Level C keeps Classic numeric
precedence. These rejected source switches require clear diagnostics rather
than silent pre-scan acceptance. No new runtime profile option is implied.

2026-10-04 LC-I-05 OPTIONS closure receipt, implementation
`8cd24c317abdbf670128fd4d49925f5a1d4e36cd`: the first static bare-word
clause is the sole file-level directive; the pre-scan only selects compatible
source settings and does not share later validation's option-seen flags. The
parser retains every source `REXX_OPTIONS` expression, including the first,
and the canonical lowerer routes each one through a single source-anchored
`RexxClassicConfig.applyOptions` call. The generated Level B header remains
private. The service returns without parsing its exact-length argument because
the approved BYTE and UTF8 profiles have no recognized runtime words; the
expression still executes once and unknown words remain harmless. This is the
approved present processor contract, not an exception to a supported option.
Source `comments_hash`/`comments_dash` are honored, block-only comments are
the Level C default, and `comments_slash`, `numeric_common`, and conflicting
comment settings receive `INCOMPATIBLE_OPTIONS` diagnostics. Source and
canonical `-d2` inspection showed the original nodes and generated calls at
first, nested, loop and local-routine anchors
(`/tmp/crexx-options-ast-final.pTGafi`); malformed-expression recovery retained a
following SAY node (`/tmp/crexx-options-recovery.opjB4d`). Regina agreed on
`levelc_options_instruction.rexx` (`6`, `1`) and the dynamic first-clause
fixture (`1`). Focused OPTIONS CTest passed 11/11; direct
`testRexxClassicBifs_{noopt,opt}` checks passed for unknown embedded-NUL
values in both BYTE and UTF8 profiles; optimized/no-opt RexxScript consumers
passed 10/10. The final Release Level C suite passed 346/346
(`/tmp/crexx-options-levelc-closure.QqJHit`) after the final runtime edit.
The 169/169 Release smoke run (`/tmp/crexx-options-smoke-final.5SoDsm`)
qualified the final compiler pre-scan; the subsequent runtime no-op reduction
was covered by the final focused and Level C runs. `rxc` → `rxas` → `rxlink`
→ `rxvm` on the final source/runtime image exited zero and printed `6`, `1`
(`/tmp/crexx-options-closure-link-log.PRQgPW`). `git diff --check` passed.
No infeasible feature or approved compatibility exception was needed. Proceed
to LC-I-06 IF; LC-AC-59/61 remain open until their remaining rows close.

**LC-I-06 IF plan — vision and intended outcome.** Classic `IF` should select
exactly one authored instruction through the same statement path used outside
the branch, with no special-case list of allowed arm kinds. Clause boundaries,
nearest ELSE, exact logical values, errors and source anchors should match the
[IBM REXX IF reference](https://www.ibm.com/docs/SSGMCP_5.5.0/reference/rexx/if.html)
and Regina where its behavior agrees. The current grammar builds a
condition/THEN/optional ELSE tree, and the lowerer has one recursive IF helper
but separate main/procedure dispatch. The earlier `LC-AC-02` pass was bounded:
its unsupported-arm fixture fails because the PARSE template is not yet
globally supported. This review must identify any IF-specific limitations,
remove avoidable duplication without inventing special cases, and leave other
instructions' open semantics under their own rows. Preserve Level B and
RexxScript behavior and the established canonical IF emitter.

1. **LC-STEP-68A (LC-AC-59/67; complete; depends on OPTIONS closure):** inventory
   every parser IF/THEN/ELSE form, token adapter, diagnostics, source and
   canonical AST shape, main/procedure dispatch, logical service, and retained
   tests. Compare IBM and Regina for clause placement, dangling ELSE, empty
   arms, exact truth values and error identity. Classify each failing arm as
   IF-specific or as work owned by its statement row. Record any architecture
   decision before editing compiler logic.
2. **LC-STEP-68B (LC-AC-67; complete at `c8508ee69`; depends on 68A and any
   required Adrian approval):** repair the complete IF-owned path in one coherent
   implementation increment. Keep a single source-anchored canonical IF
   lowering path and use normal statement dispatch for arms. Add focused
   positive, negative, AST and opt/no-opt tests for the full IF matrix;
   preserve or replace earlier bounded fixtures.
3. **LC-STEP-68C (LC-AC-59/61/67; complete; depends on 68B):** inspect source and
   lowered trees, reconcile all legal/invalid forms and error recovery, compare
   reference output, run focused runtime/RexxScript consumers and the relevant
   normal correctness suite, prove `rxc`/`rxas`/`rxlink`/`rxvm`, update the
   architecture/reference docs, and close LC-I-06 only with an exact-commit
   evidence receipt. If any IF-owned capability is absent, leave LC-AC-67
   open; significant implementation work is not an infeasibility exception.

2026-10-04 LC-STEP-68A initial IF audit: the grammar owns a three-child
condition/THEN/ELSE source shape and already recognizes nested IF, DO and
SELECT arms; the lowerer has one recursive IF helper and uses the ordinary
main/procedure statement paths for each arm. `RexxValue.logicalIfValue`
enforces exact `0`/`1` with `34.1`. A Regina and compiled seven-line matrix
matched on nearest ELSE, false outer IF, one-time side effects and a DO arm
(`/tmp/crexx-if-matrix.DsQOqP`). Semicolons and labels between THEN and its
following instruction are legal in both; they are not dummy instructions.
The older unsupported-arm fixture fails for a PARSE template not supported
outside IF (`/tmp/crexx-if-arm-baseline.Lr24aW`), so it is not an IF-specific
capability limit. Invalid probes found a concrete IF-owned gap: a trailing
THEN, including one followed only by separators or a label, reports generic
`21.1` and duplicate `PARSE_FAILURE` rather than Regina's `14.3` (case logs
under `/tmp/crexx-if-arm-invalid.LhlEgo` and
`/tmp/crexx-if-more-invalid.Z3OvFR`). A trailing ELSE already reports `14.4`.
The Level C parser calls its fallback on total parse failure, then the
top-level driver calls the generic fallback again even when diagnostics exist;
the duplicate conflicts with the driver's stated last-resort contract.
Repair this existing ownership path and the missing-THEN-arm diagnosis before
qualification. Empty parentheses after IF remain a general expression-parser
case under `LC-AC-04/08`; do not mark them as an IF-only success.

2026-10-04 LC-I-06 IF closure receipt, implementation
`c8508ee690a6ec9e5c20cfa596f18c7d9305e799`: the existing source grammar,
single IF lowerer and canonical IF emitter handled normal and nested arms; no
new AST node or arm whitelist was needed. The IF-owned repair classifies a
trailing THEN after optional separators/labels as `14.3`, and the driver no
longer adds generic `PARSE_FAILURE` after Level C's own fallback diagnosis.
Trailing ELSE remains `14.4`; the already implemented `logicalIfValue` keeps
exact `0`/`1` and `34.1`. The whole-instruction fixture exercises assignment,
DROP, PARSE, CALL, OPTIONS, SAY/NOP, DO, SELECT, LEAVE/ITERATE, local RETURN,
separator/label placement and nested ELSE; Regina and optimized/no-opt cREXX
both print `red blue new`, `1`, `2`, `select-if`, `local`, `label`, `outer`.
Source and canonical `-d2` trees retain nested IF ownership, one logical
check per condition and authored arm anchors (`/tmp/crexx-if-ast.S1mcYT`).
Nested missing-THEN recovery retains the following SAY in the source tree
(`/tmp/crexx-if-nested-recovery-ast.CaydAb`). The 19/19 focused IF matrix,
including missing condition/THEN/arm, stray ELSE, invalid logical value,
opt/no-opt and tree shape, passed. The exact Release Level C suite passed
356/356 (`/tmp/crexx-if-levelc-final.jsTViV`); shared `testRexxValue` and
RexxScript checks passed 12/12. Linked `rxc` → `rxas` → `rxlink` → `rxvm`
execution exited zero with the same seven lines
(`/tmp/crexx-if-link-log.fGjSDR`). `git diff --check` passed. The older
unsupported-arm fixture still identifies a PARSE template limitation owned
by LC-I-19; a syntactic final RETURN remains required by the open routine
row. `IF ()` parsing and wider expression syntax remain under LC-AC-04/08.
Shared trap/condition lifecycle remains open under SIGNAL and its foundation
criteria. No IF-specific infeasible feature or approved compatibility
exception was needed. Proceed to LC-I-07 SELECT.

**LC-I-07 SELECT plan — vision and intended outcome.** Classic `SELECT` should
evaluate ordered WHEN conditions and execute exactly the first selected arm,
or its optional OTHERWISE list, using the same instruction dispatch as outside
the selection. The full source form, errors, nesting, source ownership and
linked execution should agree with the
[IBM REXX SELECT reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=ki-select)
and Regina where they agree. The existing source grammar and canonical IF
lowering are the intended foundation; review and repair the whole instruction
rather than accumulating bounded cases or new per-arm handling. Shared
expression, condition/trap and individual arm-instruction obligations remain
in their owning open rows. Preserve Level B and RexxScript behavior.

1. **LC-STEP-69A (LC-AC-59/68; complete; depends on IF closure):** inventory the
   full SELECT grammar, token adapter, validation and fallback diagnostics,
   source/canonical AST, main/procedure dispatch, logical/no-match services,
   and retained tests. Probe IBM/Regina valid and invalid forms, including
   ordering, separators, empty OTHERWISE, nesting, END and error location.
   Classify gaps by owner and record any architecture or language decision
   before compiler edits.
2. **LC-STEP-69B (LC-AC-68; complete; depends on 69A and any required Adrian
   approval):** repair SELECT-owned gaps in one coherent implementation
   increment, keeping one source-anchored lowering path and ordinary statement
   dispatch for arms. Add a whole-instruction fixture plus focused invalid,
   AST and optimized/no-opt coverage; replace bounded assertions where useful.
3. **LC-STEP-69C (LC-AC-59/61/68; complete; depends on 69B):** inspect source and
   canonical trees, reconcile every valid/invalid form and error recovery,
   compare reference output, run focused shared-runtime/RexxScript and relevant
   normal correctness checks, prove `rxc`/`rxas`/`rxlink`/`rxvm`, update the
   architecture/reference docs, and close LC-I-07 only with an exact-commit
   evidence receipt. If any SELECT-owned capability remains absent, keep
   LC-AC-68 open; significant work is not an infeasibility exception.

2026-10-04 LC-STEP-69A audit: the existing parser builds `SELECT >
INSTRUCTIONS > WHEN* [OTHERWISE]`; the lowerer builds an ordered canonical IF
chain and passes arm statements through the ordinary main/procedure dispatcher.
The shared `logicalWhenValue` and `rexxvalue_select_missing` services supply
`34.2` and `7.3`. A Regina/compiled matrix matched first-true selection,
condition laziness, nested IF/SELECT/DO, a multi-instruction OTHERWISE, and
an omitted OTHERWISE after a true arm (`/tmp/crexx-select-audit-_z_7gpbm`).
IBM's reference explicitly permits an empty OTHERWISE list; Regina accepts
it, but cREXX currently reports `21.1`. Debug parser trace
(`/tmp/crexx-select-empty-parse.dwwNEZ`) shows the token adapter discarding
the clause boundary after OTHERWISE, leaving END where the grammar expects an
EOC. A SELECT containing only OTHERWISE currently reports `7.1`; Regina
distinguishes it as `7.2`. Missing WHEN condition and missing THEN arm fall
back to generic or misleading diagnostics; the source grammar already has
corresponding recovery forms for the analogous IF case. Repair these existing
parser/diagnostic ownership paths, retain the one canonical lowerer, and
characterize wider expression/parser failures under LC-AC-04/08 rather than
adding SELECT-only runtime cases. No new language syntax or architecture
decision is needed for this repair.
The arm inventory additionally found that `OTHERWISE SELECT` on the same
clause is valid in Regina but rejected because the first-instruction parser
rule omits SELECT (`/tmp/crexx-select-first-z0ulgu7f`). The grammar also
stores that first instruction outside its OTHERWISE `INSTRUCTIONS` child,
forcing two lowerer paths for one list. Admit the existing SELECT instruction
in the common first-instruction production and normalize this source AST to a
single ordered list; this is a representation cleanup within the documented
SELECT-to-IF design, not a new language or runtime architecture.
An additional invalid-header probe found `SELECT 1` and `SELECT foo` silently
accepted: Lemon records a syntax error for the extra same-clause token but
recovers to a valid SELECT tree without a diagnostic. Regina reports `21.1`.
Give the invalid header an explicit source grammar production and source
diagnostic, retaining the following WHEN tree for recovery. This is a
SELECT-owned correction; broader silent parser recovery remains a general
front-end audit concern under LC-AC-04/08.
The same silent Lemon recovery also drops an unexpected SAY before, between,
or after WHEN arms while accepting the remaining SELECT
(`/tmp/crexx-select-body-mfdxp643`); Regina reports `7.2` at the first such
clause. The grammar's existing `%syntax_error` record is overwritten by later
recovery tokens and ignored when a usable AST remains. Keep the first syntax
error and diagnose an otherwise undiagnosed unexpected instruction at a SELECT
clause boundary as `7.2`. Existing grammar diagnostics continue to take
precedence. Broader parser recovery is kept under its own instruction and
expression owners, because legal empty assignments and labels also appear in
the parser's syntax-error record.
Regina also accepts labels before and between WHEN clauses. Lemon had been
recovering past them without source AST ownership. Retain those LABEL nodes in
the SELECT list while the current canonical lowering skips their trace-only
execution role; shared TRACE and SIGNAL label semantics remain open in their
own rows. Null clauses already pass without a label-specific route.

2026-10-04 LC-STEP-69B/69C receipt: `dde8909aa` implements the complete
SELECT parser/AST/diagnostic repair while retaining one canonical IF-chain
lowerer and ordinary arm dispatch. `levelc_select_instruction.rexx` matches
Regina byte-for-byte for first-true selection, lazy conditions, empty and
same-clause OTHERWISE, labels around WHEN, nested SELECT/IF/DO, local calls,
and selected instruction side effects. Source-tree and canonical-tree CTests
check the normalized OTHERWISE list, retained labels, branch chain and pool
calls. Focused SELECT/IF/assignment checks passed 41/41
(`/tmp/crexx-select-focused6.M6NBDf`); final optimized/no-opt `34.2` and
`7.3` assertions passed 4/4 (`/tmp/crexx-select-error-tests.g6bkrm`).
Shared RexxValue/RexxScript checks passed 12/12
(`/tmp/crexx-select-shared-tests.rFKyS7`), and the normal Release Level C
suite passed 378/378 (`/tmp/crexx-select-levelc-qa.GyVd61`). A linked image
from `rxc`/`rxas`/`rxlink`/`rxvm` matched Regina exactly
(`/tmp/crexx-select-linked-run.LKlXnF`,
`/tmp/crexx-select-regina.nKOSP7`). `git diff --check` passed before commit;
the subsequent architecture note changed documentation only. The active
WHEN's nonlogical-value error reports its WHEN source line in both modes.
The no-match error includes the SELECT line in its `7.3` message; VM stack
source can report the last WHEN or an inlined shared helper instead, a
cross-cutting source-stack obligation under `LC-AC-08/04`, not a SELECT-only
runtime restriction. Shared label TRACE and condition-trap behavior remain
with `LC-I-23/24`; arm-instruction semantics remain with their owner rows.
No SELECT-specific infeasibility or approved compatibility exception was
needed. `LC-AC-68` and LC-I-07 close; `LC-AC-04/08/59/61` stay open for the
remaining programme. Next is LC-I-09 LEAVE.

**LC-I-09 LEAVE plan — vision and intended outcome.** Every parsed Classic
`LEAVE` form should use the existing source-loop binding and canonical loop
transfer path, selecting the correct active loop and leaving its visible
control state unchanged. The grammar must reject malformed or trailing source
text instead of recovering to a valid partial instruction. Match IBM's
[LEAVE reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-leave)
and Regina's executable/error behavior, while preserving Level B, DO,
ITERATE and RexxScript behavior. Do not add a separate LEAVE emitter path.

1. **LC-STEP-71A (LC-AC-59/69; complete review; depends on SELECT closure):** inventory
   grammar/token recovery, source and canonical AST, static diagnostics,
   transfer binding and emitter, reference valid/invalid forms, and retained
   DO/LEAVE/ITERATE tests. Reproduce any mismatch before editing, record
   whether it belongs to LEAVE or a shared lifecycle owner, and identify any
   decision gate before an architecture or language change.
2. **LC-STEP-71B (LC-AC-69; complete; depends on 71A and Adrian's
   `STRICTC` direction):** retain default static target diagnostics; add a
   central leading-header `STRICTC` flag for Level C and lower invalid-target
   LEAVE/ITERATE to one shared `RexxDoState` runtime error service only in
   that mode. Keep syntax diagnostics static and valid transfers on the
   canonical binding path. Add a complete optimized/no-opt fixture and
   focused default, strict, invalid and AST checks.
3. **LC-STEP-71C (LC-AC-59/61/69; complete; depends on 71B):** compare IBM and
   Regina, inspect source/canonical associations, run focused and relevant
   normal correctness plus shared-consumer checks, prove linked output,
   update architecture/reference docs, and commit an exact-revision receipt.
   Close LC-I-09 only when its own contract is proved; keep shared SIGNAL,
   INTERPRET, TRACE and condition lifecycle in their owner rows.

2026-10-04 LC-STEP-71A initial audit: the parser already builds `LEAVE` with
zero or one source `VAR_SYMBOL`; the common LEAVE/ITERATE validator resolves
the nearest source repetitive DO or a named controlled ancestor, and the
lowerer emits a canonical transfer to that binding's hidden loop control.
IBM specifies that a name is constant, matches the authored control symbol
except case, never substitutes compound components, and chooses the innermost
matching active loop. Regina/cREXX probes agree on compound names and reject
routine-to-caller transfer. Three LEAVE-owned gaps are reproduced: `LEAVE i j`
is silently accepted by cREXX but Regina reports `21.1`; a malformed numeric,
parenthesized or dot target reports `20.2` in cREXX but Regina reports `20.1`;
and a named LEAVE with no repetitive loop reports `28.3` in cREXX but Regina
reports `28.1`. The errors are ordinary Classic fidelity corrections within
the current grammar/diagnostic architecture. The `20.2` route is shared with
ITERATE, so the same correction must be checked for that consumer without
prematurely closing LC-I-10. SIGNAL/INTERPRET invalidation remains with those
instruction owners. These source-form repairs need no new architecture decision.

Further LC-STEP-71A runtime probe: Regina prints `ok` for `if 0 then leave`,
`if 0 then leave i`, and an unselected LEAVE in a simple DO, whereas cREXX
rejects all three with static `28.1`. This is a LEAVE-owned timing gap, not an
infeasible capability. The proposed architecture adjustment is to keep source
syntax errors (`20.1`, `21.1`) static, retain the existing canonical transfer
for a valid source-loop binding, and lower a syntactically valid invalid-target
LEAVE/ITERATE to one shared `RexxDoState` runtime error service that signals
the contextual `28.1`/`28.2` or `28.3`/`28.4` only if reached. This uses the
already imported DO runtime and ordinary statement dispatch; no new AST or
emitter instruction is required. The shared transfer validator and lowerer
must remove their static target rejection, and the affected invalid-compile
tests become invalid-runtime tests. A dead-branch fixture plus reached-error
fixtures in opt/no-opt, normal Level C, RexxDoState/RexxScript and linked
execution will prove timing and isolation. This changes the existing static
validation architecture and is gated on Adrian's approval under AGENTS.md.

2026-10-04 accepted policy: Adrian clarified that the question concerns
**compiled** Level C. `STRICTC` means closer Classic REXX behavior. The
current Level B compiler rejects `if 0 then leave` outside a loop
(`NOT_IN_LOOP`) and accepts it inside one. Regina accepts an unselected
outside-loop LEAVE, and IBM documents Error 28 as an error while *running* a
compiled REXX program; the exact IBM dead-branch compilation case is not yet
proved. Adrian chose the opposite mapping from the earlier proposal:
ordinary Level C keeps compile-time rejection for source-provable invalid
LEAVE/ITERATE targets, while a leading static `OPTIONS LEVELC STRICTC` seeks
closer Classic behavior by compiling syntactically valid transfers and
signalling `28.*` only if an invalid transfer executes. This is the accepted
default-profile compatibility exception for these instructions. `STRICTC`
does not change malformed-syntax diagnostics or suppress an executed signal;
Level B/G remain unchanged. The flag is a central policy point for subsequent
instruction reviews, not a claim that all Classic behaviors already have a
strict-mode implementation. This decision explicitly revises LC-AC-69.

2026-10-04 source-form checkpoint: `b0df4e3a5` corrects malformed-name
`20.1`, extra-token `21.1`, and the no-loop versus unmatched-name subcode in
the shared LEAVE/ITERATE diagnostic scanner, with a whole LEAVE fixture and
focused error tests. Regina and compiled optimized/no-opt output match for
counted, FOREVER, WHILE, UNTIL, named outer/innermost, compound authored
name, simple group, IF/SELECT and local routine cases
(`/tmp/crexx-leave-regina.OXJPvX`). Source/canonical tree inspection retains
named LEAVE nodes and targets (`/tmp/crexx-leave-tree-log.iubjNM`). Focused
regressions passed 22/22 (`/tmp/crexx-leave-focused-retry.5FvvsG`). The
Release Level C run passed 384/385; its sole failure was an old highlighter
assertion expecting `20.2`. `b4e44cdda` updated only that test assertion,
which passed in isolation (`/tmp/crexx-leave-static-levelc.qCAehB`,
`/tmp/crexx-leave-highlighter-retry.jTabiN`). No product regression was found.
The runtime-timing part of LC-AC-69 and LC-I-09 remains open for implementation
and qualification under the accepted policy; this checkpoint is not LEAVE
closure.

2026-10-04 LC-I-09 LEAVE closure: `1382bdc9b` records Adrian's accepted
default/`STRICTC` policy; `2aad4ec65` implements the central leading-header
flag, preserves default static 28.x diagnostics, and lowers only invalid
strict-mode source targets to one shared `RexxDoState` error service. Valid
transfers keep the existing canonical loop binding; a lost binding remains a
compiler failure. Syntax 20.1/21.1 remains static. Raw and canonical transfer
trees are checked by `levelc_strictc_transfer_source_tree` and the existing
LEAVE tree/whole-instruction fixtures. Final Release build of `rxc`, `rxas`,
`rxlink`, `rxvm`, and `rxfnsc` passed (`/tmp/crexx-strictc-final-build.nF5lFX`);
the normal Level C suite passed 403/403
(`/tmp/crexx-strictc-final-levelc.ivizDu`), and shared RexxValue,
RexxDoState, RexxScript, CLI and linked-consumer checks passed 14/14.
Optimized/no-opt strict-mode fixtures cover dead and reached LEAVE/ITERATE,
unmatched names, local routine isolation and malformed syntax. Regina matches
the dead-branch output and runtime 28.1/28.2/28.3/28.4 cases. Level B/G still
reject `if 0 then leave` outside a loop with `NOT_IN_LOOP` in direct probes.
The final linked `rxc`→`rxas`→`rxlink`→`rxvm` run printed `body 1`/`after 2`
for dead transfers and signalled `RXC-LC-28.1` at source line 2 for a reached
LEAVE (`/tmp/crexx-strictc-final-linked.e0HyA7`). No LEAVE-owned infeasible
feature or exception remains beyond the explicitly accepted default-profile
timing distinction. `LC-AC-69` and LC-I-09 close; shared SIGNAL/INTERPRET
lifecycle and `LC-AC-04/08/59/61` remain open under their own owners. Next is
LC-I-10 ITERATE for a separate whole-instruction review and closure.

**LC-I-10 ITERATE plan — vision and intended outcome.** Complete the whole
Classic instruction through the existing source-loop association and canonical
loop emitter, so ITERATE consistently skips the selected body's remainder,
ends inner loops, and performs the selected loop's ordinary end check and
advance. Use the approved default/`STRICTC` timing policy from LEAVE and the
shared runtime Error 28 service. Preserve Level B/G, DO, LEAVE and RexxScript
behavior. Compare the [IBM ITERATE reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-iterate)
and Regina, recording any true cross-cutting lifecycle gap under its owner.

1. **LC-STEP-72A (LC-AC-59/70; complete review; depends on LEAVE closure):**
   inventory grammar and recovery, raw/canonical AST, transfer binding and
   emitter, end-check/step order across counted, controlled, FOREVER,
   WHILE/UNTIL and combinations, named nesting, routine isolation, default
   and `STRICTC` errors. Reproduce any mismatch before editing and identify
   its instruction owner and any new decision gate.
2. **LC-STEP-72B (LC-AC-70; complete evidence-only; depends on 72A):** repair ITERATE-owned gaps in one
   coherent instruction increment using the established transfer machinery;
   add a complete optimized/no-opt fixture and focused error/tree checks.
   If the review finds no code gap, add only the missing whole-instruction
   evidence and document that conclusion.
3. **LC-STEP-72C (LC-AC-59/61/70; complete; depends on 72B):** compare IBM/Regina results,
   inspect source/canonical ownership, run focused and relevant normal and
   shared-consumer checks, prove linked execution, update architecture and
   reference docs, and commit the exact-revision receipt. Close LC-I-10 only
   when its own contract is proved. Shared SIGNAL/INTERPRET, TRACE and
   condition lifecycle remain with their owner rows.

2026-10-04 LC-STEP-72A initial audit: ITERATE shares the LEAVE/ITERATE source
grammar, 20.1/21.1 recovery, static 28.x validator, source-loop association,
canonical target emitter and approved `STRICTC` runtime-error service. IBM's
ITERATE reference says the selected loop's control variable is incremented
and tested as usual and inner active loops end; IBM IRX0028I confirms an
inactive caller loop cannot be crossed by an internal routine. Existing DO
tests cover counted, WHILE/UNTIL and named transfers but no complete ITERATE
fixture. A whole-instruction probe covering counted, FOREVER, WHILE, UNTIL,
TO/BY, FOR, named outer, compound authored control, SELECT/simple-DO wrappers
and a local routine matches Regina exactly in optimized execution
(`/tmp/crexx-iterate-probe.hvoWVy`). No ITERATE-owned code mismatch is yet
reproduced. The remaining 72A review should verify combined controlled
end-check/step timing, duplicate-name nearest binding, syntax/error matrix,
 no-opt and source/canonical trees before deciding whether 72B needs product
 code or only regression coverage.

2026-10-04 LC-I-10 ITERATE closure: `86727645a` adds one complete instruction
fixture and focused invalid-source checks; no ITERATE product code change was
needed. The whole fixture agrees byte-for-byte with Regina in optimized and
no-opt execution across counted, FOREVER, WHILE, UNTIL, TO/BY, FOR, combined
controlled end conditions, named outer and innermost duplicate controls,
compound authored control, IF/SELECT/simple DO wrappers and a local routine
(`/tmp/crexx-iterate-matrix.U34zSg`). The CTest matrix confirms default
compile-time 28.2/28.4, `STRICTC` runtime 28.2/28.4, routine isolation and
static 20.1/21.1 syntax errors; Regina confirms the same reached-error and
syntax subcodes while accepting the explicitly approved default-profile
dead-branch exception. Raw ITERATE nodes, authored names and canonical target
nodes were inspected (`/tmp/crexx-iterate-tree.YptUAM`), with canonical shape
retained in `levelc_iterate_instruction_tree`. Focused ITERATE regressions
passed 18/18 (`/tmp/crexx-iterate-focused.uxsjHa`); the normal Release Level C
suite passed 415/415 (`/tmp/crexx-iterate-levelc.3TCXZL`). The unchanged
shared RexxDoState/RexxValue/RexxScript code and its 14/14 qualified checks
from LEAVE were reused. Linked `rxc`→`rxas`→`rxlink`→`rxvm` output matches
Regina (`/tmp/crexx-iterate-linked.dvHEz1`); direct Level B/G probes still
reject outside-loop `if 0 then iterate` with `NOT_IN_LOOP`. IBM's
[ITERATE reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-iterate)
and [IRX0028I explanation](https://www.ibm.com/docs/en/zos/2.5.0?topic=irx-irx0028i)
support the end-step and inactive-routine contracts. No ITERATE-owned
infeasible feature or further compatibility exception was found. LC-AC-70 and
LC-I-10 close; shared SIGNAL/INTERPRET, TRACE and condition lifecycle remain
open under LC-AC-04/08/59/61 and their instruction rows. Next is LC-I-11 ARG.

**LC-I-11 ARG plan — vision and intended outcome.** Make Classic ARG a whole
instruction rather than a fixed procedure-parameter binding. The user should
be able to parse the active activation's unchanged argument strings with
Classic uppercase behavior in a main program or routine, using the same
template rules, variable pool and argument-presence semantics as PARSE ARG.
Retain Level B's typed `arg` and RexxScript's distinct call interface. The
source and canonical AST must remain explicit and supportable; no arbitrary
template or argument count limit is acceptable.

1. **LC-STEP-73A (LC-AC-59/71; complete; depends on ITERATE closure):**
   inventory IBM/Regina forms, parser and raw AST shapes, current fixed
   procedure ARG lowering, PARSE ARG plan/runtime, CALL activation storage,
   main and routine invocation modes, omitted/empty arguments, errors and
   source positions. Reproduce gaps and identify shared owners before edits.
2. **LC-STEP-73B (LC-AC-71; complete after approved design; depends on 73A):** choose the simplest complete
   lowering/runtime contract, preferring the existing PARSE plan and shared
   activation state where they fit. If that choice changes architecture,
   record the concrete design and pause for Adrian's required approval before
   editing compiler logic. Do not classify substantial work as infeasible.
3. **LC-STEP-73C (LC-AC-71; complete; depends on 73B and its decision gate):** implement
   the whole instruction and remove obsolete fixed-template duplication;
   cover main and routine forms, repeated reads, templates, ordering,
   uppercasing, missing/omitted positions, and all admitted call modes.
   Preserve Level B/G and RexxScript contracts.
4. **LC-STEP-73D (LC-AC-59/61/71; complete; depends on 73C):** compare IBM/Regina,
   inspect raw/canonical trees, run focused and relevant normal/shared
   regressions, prove linked output, update reference/architecture docs, and
   commit an exact-revision receipt. Close LC-I-11 only when all AC-71
   behavior and error paths are verified; keep CALL/BIF/host lifecycle gaps
   visibly open under their owners as well.
5. **LC-STEP-73E (LC-AC-06/71; complete after the activation frame):**
   implement the adjacent Classic `ARG()` BIF against that same activation
   state, with zero/one/two-argument forms, E/O existence tests, omissions,
   and standard BIF errors. Compare IBM and Regina, exercise main/routine and
   optimized/noopt calls, and retain shared `rxfnsc` regression evidence.
   Keep the BIF inventory row separate from ARG instruction closure.
6. **LC-STEP-73F (LC-AC-71; complete after approved decision gate):** map a
   failed dynamic numeric PARSE/ARG position to Classic Error 26.4 while
   preserving the existing Level B `parseplan` conversion signal. The proposed
   version-2 descriptor flag and VM handler change below require Adrian's
   approval before implementation.
7. **LC-STEP-73G (LC-AC-71; complete for ARG after 73F):** qualify supported host ARG
   entry and repeated activation lifecycles, audit the actual invocation and
   approved Unicode-first boundary, including codepoint templates and
   Latin-1 ordinals. The earlier BYTE/UTF8 profile-selection proposal was
   superseded by the 2026-10-04 character-model decision. Keep non-admitted
   CALL/host entry modes with their owning instruction or host-service row.
   The 2026-10-05 scope clarification below supersedes the former rule that
   those future modes prevented ARG instruction closure.
   **LC-STEP-73G.1 (LC-AC-06/71; complete 2026-10-05):** add a length-aware
   native main-entry argument variant over the existing `rxvml` value array,
   keeping `rxvml_run()` as the terminated-string convenience route. Accept
   valid Unicode text including embedded NUL, preserve exact lengths into
   `ARG(n)` and uppercase template parsing, reject malformed UTF8 and invalid
   pointer/length pairs, and prove consecutive runs on one host context.
   This is an additive host entry using the current activation and
   `rxvml_set_str()` paths; it changes no source syntax or frame architecture.
   Keep the C-string route's inherent limit explicit and audit higher host
   wrappers under LC-AC-06 rather than calling it an exclusion.
   **LC-STEP-73G.2 (LC-AC-06/71; complete 2026-10-05; depends on 73G.1):**
   carry exact UTF8 byte lengths through both public `crexxsaa` source and
   RXBIN entries, including uncached, cache-miss and cache-hit execution,
   using the same `rxvml` argument array and one source-cache implementation.
   Retain the existing C-string entries as convenience wrappers. Verify
   embedded NUL, Unicode, malformed UTF8, invalid pointer/length pairs,
   repeated context use and legacy calls with focused host tests; update the
   public host guide and reference obligation. This is an additive C API, not
   a new Classic invocation model.
   **LC-STEP-73G.3 — separate host invocation design proposal (LC-AC-06;
   review parked 2026-10-05, outside ARG closure):** the length-aware entries complete transport of
   *present* Unicode arguments for today's command-style main entry, but they
   do not implement the larger Classic host start contract. In particular,
   `rxvml_run_internal()` builds an array in which every position is present;
   a null pointer with zero length becomes an explicit empty string. The
   generated Level C main copies each item using `appendText`, receives no
   command/function/subroutine call kind, and its wrapper discards the body
   result. The body currently has a void return and main `EXIT expression` is
   still an open LC-I-15 shape. `crexxsaa` exposes source/RXBIN execution and
   command-environment callbacks, with no `RexxStart()`-style result/condition
   or in-store source entry. These are real host and cross-instruction gaps;
   the C-string interface's NUL limit remains an obligation, not an exclusion.

   **Vision:** a C host can start a Level C program in each agreed Classic
   invocation mode with exact Unicode argument spans and omitted-slot state,
   receive a typed completion/result or condition, and reuse one context
   without leaking activation state. The implementation should use the same
   activation, pool, config and source-cache paths as CLI/internal calls and
   preserve existing `rxvml`/`crexxsaa` consumers. The public compatibility
   ABI choice is pending Adrian's clarification; do not infer historical
   `RexxStart()` source compatibility from the current `crexxsaa` name.

   1. **LC-73H-01:** host supplied omitted, empty and nonempty positions are
      distinguishable by ARG(), ARG templates and nested calls, including
      embedded NUL and Unicode; verify source/RXBIN, cache miss/hit and repeat
      runs against reference and host C fixtures.
   2. **LC-73H-02:** command, function and subroutine entry carry their mode to
      Classic source services and return obligations; verify `PARSE SOURCE`,
      ARG, optional versus required result, EXIT/RETURN behavior and error
      identity at authored source positions.
   3. **LC-73H-03:** the agreed public host entry exposes result text and
      condition/completion state with explicit lengths and ownership; verify
      success, untrapped error, invalid input and resource/lifecycle paths.
   4. **LC-73H-04:** existing C-string and length-aware entries, cache behavior,
      ADDRESS callbacks, Level B/G, RexxScript and the installed ABI remain
      compatible; verify focused C consumers and relevant normal correctness.

   1. **LC-STEP-73G.3A (LC-73H-01–04; active):** inventory the local REXXSAA
      header, IBM/Regina invocation behavior, current `rxvml`/`crexxsaa`
      ownership, main-frame lowering and EXIT/RETURN dependencies; retain a
      reference matrix before product edits.
   2. **LC-STEP-73G.3B (LC-73H-01–04; decision gate after 3A):** settle whether
      the public target is an extended modern `crexxsaa` facade with Classic
      equivalent semantics or historical source-compatible `RexxStart`/
      `RXSTRING` entry points, including result ownership and any callback
      scope. Record Adrian's choice before changing the public ABI or main
      frame contract.
   3. **LC-STEP-73G.3C (LC-73H-01/02; depends on 3B):** carry typed host
      argument presence and entry mode through one VM main activation without
      a parallel ARG engine; implement needed shared RETURN/EXIT result state
      with their instruction owners.
   4. **LC-STEP-73G.3D (LC-73H-03/04; depends on 3C):** implement the selected
      host facade over the same source-cache/load/run path, preserving legacy
      entries and explicit buffer ownership.
   5. **LC-STEP-73G.3E (LC-73H-01–04; depends on 3D):** compare reference
      modes, run focused host/AST/linked tests, the relevant normal correctness
      suite once on exact inputs, update the public guide, and commit a
      coherent receipt. Keep LC-AC-71 open until this and other applicable
      invocation paths pass; keep broader host callbacks under LC-AC-06.

   **Scope disposition, 2026-10-05:** `73G.3A–E` and `LC-73H-01–04` are
   retained as a historical host proposal and reference inventory, not active
   ARG implementation steps or `LC-AC-71` gates. No public host ABI or main
   frame-mode work follows from this proposal without a separate decision.
   Cross-dialect Classic/non-Classic invocation is outside the agreed scope;
   external Classic CALL/INTERPRET and actual host-interface obligations keep
   their separate programme owners.

   **2026-10-05 73G.3A reference receipt.** A guarded C `RexxStart` probe
   against installed Regina, retained as ignored
   `cmake-build-debug/levelc_arg_regina_host_probe.c` and
   `cmake-build-debug/levelc-arg-regina-host-reference.log`, used one command
   argument and three function/subroutine positions with the middle position
   omitted. Command entry reported `PARSE SOURCE` mode COMMAND, ARG count 1
   and existence `100`; function/subroutine entries reported their respective
   modes, ARG count 3 and existence `101`. Each successful entry returned an
   exact three-byte `RET` result. A three-argument COMMAND call was rejected
   by Regina with status 3, so mode-specific host argument constraints need
   explicit reference qualification. The bundled legacy `rexxsaa.h` defines
   call-kind constants `1/2/4`, while the installed Regina header uses
   `0/1/2`; a public historical ABI requires a specified compatibility
   target and must not copy constants casually. A second guarded Regina probe
   of bare `RETURN` supplied no result in any mode but returned success even
   for RXFUNCTION; [IBM's function reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=functions-subroutines)
   requires a function result, as the Level C compliance reference does.
   Preserve this divergence in the host matrix and follow the agreed IBM
   function contract unless Adrian approves a different choice. All guarded
   runs left zero processes. The second probe is retained in
   `cmake-build-debug/levelc-arg-regina-host-bare-reference.log`.
   [ooRexx's API reference](https://www.oorexx.org/docs/pdf/rexxapi.pdf)
   also documents mode and result ownership. This is reference evidence, not
   cREXX product qualification.
8. **LC-STEP-73I (LC-AC-71; complete as a coherence review 2026-10-05;
   depends on 73C/63T-3):**
   review ARG as one compiler-to-host contract before closure. Trace every
   admitted argument producer through one activation frame, both ARG
   consumers, the common PARSE executor, source/canonical AST, diagnostics,
   and lifecycle. Consolidate the duplicate direct-CALL/local-function
   actual-argument binding loops so omission and once-only source-order
   evaluation have one implementation. Verify the resulting opt/no-opt,
   recursive, linked, host, parser and shared-runtime evidence once on the
   coherent code/test checkpoint. Give LC-AC-71 an explicit pass/fail verdict;
   keep any unproved invocation or host boundary open with its owning step.

**2026-10-05 LC-STEP-73G.1/73G.2/73I coherence verdict.** One activation
object owns ordered text values and omission bits. Main execution from CLI,
`rxvml` and `crexxsaa` reaches the same VM `.string[]` and generated main
frame; direct CALL and local function expressions now share one actual-binding
loop. ARG and ARG() read that unchanged frame, and ARG uses the PARSE-owned
template executor and visible pool writes. The public native run entries have
additive byte-length variants, so valid Unicode arguments with embedded NUL
reach the same activation without truncation. The two `crexxsaa` source
entries share one cache/miss/hit implementation, and the two RXBIN entries
share one load/run implementation. The older terminated-string APIs remain
convenience entries. This is argument transport in the current host model;
Classic `RexxStart()`-equivalent invocation modes and services remain open.

| LC-AC-71 facet | Review verdict / retained evidence |
| --- | --- |
| Templates, repeat reads, positional omission and Unicode uppercasing | Passing optimized/no-opt, parser, codepoint, linked and shared-runtime fixtures from the prior ARG checkpoints; the 2026-10-05 558-test normal Debug suite includes them. |
| Main, direct CALL, local function and frame-local label paths | Passing invocation fixture, one-body frame tests, source/canonical tree and linked evidence; the duplicate actual-binding loops were consolidated in `levelc_append_call_actuals()`. |
| Native host exact lengths and lifecycle | Passing `levelc_arg_host_entry` and `crexxsaa_arg_lengths`: embedded NUL, Unicode, invalid UTF8/pointer/length, uncached source, cache miss/hit, direct RXBIN, legacy entry and consecutive context use. |
| Other invocation and host modes | **Open:** external routine/indirect CALL and INTERPRET remain unimplemented under their instruction owners; Classic host invocation classes, trap overrides and pool APIs remain under LC-AC-06/LC-REF-001/005/016/021–024. They are not ARG exclusions. |

Therefore **LC-AC-71 and LC-I-11 remain open**. The coherent implementation
for admitted current paths passes; the whole instruction cannot close while
its applicable invocation contract is unproved. The final guarded product
build (`rxc`, `rxas`, `rxlink`, `rxvm`, `crexxsaa` and host fixtures) passed,
focused `crexxsaa_arg_lengths`, `crexxsaa_status`, `crexxsaa_variables` and
`levelc_arg_host_entry` passed 4/4, and the normal Debug Level C suite passed 558/558 on these
code/test/build inputs. Logs are
`cmake-build-debug/levelc-arg-coherence-final-build.log`,
`cmake-build-debug/levelc-arg-coherence-final-focused.log` and
`cmake-build-debug/levelc-arg-coherence-final2-debug-qual.log`.
The guarded broad run peaked at about 4366 MiB descendant RSS, left zero
processes, and the post-run process inventory was empty. This evidence is a
development checkpoint, not full Level C qualification.

2026-10-05 LC-I-11 whole-instruction audit refresh: the current compiler
admits main execution from the CLI or `rxvml_run()`, direct internal CALL,
local function calls, nested/recursive invocations, and frame-local label
transfers. External routine resolution, indirect CALL and INTERPRET remain
unimplemented under their owning instruction/host rows. IBM defines ARG as
`PARSE UPPER ARG` and permits it wherever an ordinary instruction is legal;
it has no PROCEDURE-like first-instruction restriction. Its legal templates
include blank words, dots, literal and variable patterns, static and dynamic
positions, empty and arbitrarily many comma segments. The existing source
tree, shared `parseplan` executor, activation-local arguments/BIF, exact-length
internal strings, Unicode codepoints, compound/exposed writes, opt/no-opt,
linked and host checks cover these categories. A fresh Regina/compiled probe
also agrees for IF/SELECT ARG, label fallthrough, `SIGNAL` within the frame,
zero-argument CALL, function entry and `ARG(sep)` keyword adjacency; make
this permanent before closure. The C-string host API's embedded-NUL limit
remains an open host-interface obligation, not an ARG exception.

The invalid-template audit found a specific gap: `ARG x =`, `ARG x +` and
`ARG x -` report a compiler 21.1 at the following clause instead of Classic
38.1 at the authored template. Empty or unclosed parenthesized patterns also
currently use 19.7/46.1 where Regina and IBM's invalid-template category
report Error 38. Repair the shared ARG/PARSE grammar diagnostics and add
source-anchored negative checks, then run the relevant parser and Level C
correctness suite once for the coherent code/test checkpoint. Keep LC-AC-71
and LC-I-11 open until this and the invocation matrix are verified.

**2026-10-05 LC-STEP-73D invocation and diagnostic checkpoint.** The
`levelc_arg_instruction_invocation*` optimized/no-opt fixture matches Regina
for IF/SELECT placement, repeated reads after label fallthrough and SIGNAL,
omitted and explicit-empty positions, zero-argument CALL, local function entry
and `ARG(sep)` keyword adjacency. The shared ARG/PARSE grammar now reports
38.1 at the authored operator for incomplete `=`, `+` and `-` positional
patterns. Empty, non-symbol and unclosed parenthesized patterns also use
Classic invalid-template 38.1 rather than the earlier 19.7/46.1 forms.
Regina and compiler probes for these error categories are retained in
`cmake-build-debug/levelc-arg-error-audit-after.log`; permanent source-position
checks are `levelc_arg_missing_positions`, `levelc_arg_malformed` and the
affected syntax-highlighting fixtures. The focused affected checks passed
10/10, the four core product targets built, and the normal Debug Level C
suite passed 557/557 on the code/test inputs of this checkpoint. The guarded
broad run peaked at about 2227 MiB descendant RSS and left no processes;
the post-run process inventory was empty. Logs are
`cmake-build-debug/levelc-arg-audit-focused.log`,
`cmake-build-debug/levelc-arg-audit-final-focused.log`,
`cmake-build-debug/levelc-arg-audit-product-build.log` and
`cmake-build-debug/levelc-arg-audit-debug-qual.log`.
This is a coherent ARG review increment, not LC-AC-71 closure. The public
`rxvml_run()` C-string argument vector still cannot carry embedded NUL and
the applicable host-interface and unresolved invocation audit remain open;
external CALL and INTERPRET have their later instruction owners.

2026-10-04 LC-STEP-73A initial audit: IBM defines ARG as `PARSE UPPER ARG`;
comma-separated templates consume successive argument strings, each call
reads the active activation again, and missing templates/sources are legal.
Regina's `call probe 'a b',,'c d'` with two ARG instructions produces
`first A B`, an empty middle source, `third C D`, and a second parse of the
same sources (`/tmp/crexx-arg-reference.TdhaJv`). Current Level C rejects
main `ARG` (`unsupported main statement`), multi-item templates (`unsupported
ARG template`), repeated routine ARG (`ARG must be first`), and omitted CALL
arguments (`CALL argument count mismatch`). The existing fixed procedure
signature derives arity from its first ARG and passes one `RexxValue` formal
per template; it cannot represent Classic activation argument presence or
later rereads. Raw AST for `ARG x,,z` has only two inner `TEMPLATES` nodes:
the shared parser drops the empty comma position
(`/tmp/crexx-arg-ast-log.B0BW1I`). The existing PARSE path has a compiler
`parseWordTemplate` fast path and a VM `parseplan` descriptor path, but guards
comma templates and dynamic operands; VM `parseplan` version 2 already has
indexed dynamic references. These are implementable foundation gaps, not an
infeasibility or accepted exception. Routine labels without `PROCEDURE` are
also rejected by the current Level C slice and must be assigned to the
PROCEDURE/CALL lifecycle owner while ARG is completed.

**LC-STEP-73B architecture proposal for Adrian's approval:** Preserve every
comma position in the existing outer/inner `TEMPLATES` AST by representing an
empty segment with an empty inner `TEMPLATES` node; no new node type or emitter
instruction is needed. Replace ARG-derived fixed formal arity with one shared
`RexxActivationArguments` frame containing ordered `RexxValue` strings and
presence flags. Generated main code fills it from the VM's `-a`/host argument
array; internal function and subroutine calls fill a fresh frame after
evaluating actuals once, preserving omitted versus explicit empty arguments.
The frame is passed into each generated routine alongside the existing pool
and configuration, so ARG can run repeatedly at any legal point and the
Classic ARG BIF can inspect the same activation later. Lower each ARG template
through one extracted PARSE template executor: capture and uppercase that
source, apply the existing VM `parseplan` descriptor, then write captured
fields in order through the visible pool. Extend that descriptor construction
to its existing version-2 dynamic references and comma segments; migrate
Level C's word-only PARSE lowering into the same path only after the PARSE
regressions and shared `RexxValue` API consumers are checked. This replaces
fixed ARG binding rather than layering new special cases. Level B typed ARG
and RexxScript's separate parser remain unchanged. The proposed activation
frame, parser AST preservation and PARSE lowering consolidation are an
architectural shift; AGENTS.md requires Adrian's approval before those
compiler edits. No product code change for ARG has been made at this gate.

2026-10-04 decision: Adrian approved LC-STEP-73B after reviewing reuse of the
existing Level C PARSE capabilities. LC-STEP-73A and 73B are complete;
LC-STEP-73C is active. The approved implementation extracts one Level C
template lowering path from PARSE for both PARSE and ARG, using the existing
VM `parseplan` executor for general templates. ARG supplies an activation
argument source and Classic uppercasing; PARSE retains its source selection.
The existing `RexxValue.parseWordTemplate` helper and its regression evidence
must be checked for parity and external consumers before changing the
compiler's word-only path or removing the method. This decision introduces no
new source syntax or compatibility exception.

LC-STEP-73C parser checkpoint: the Level C grammar now retains empty inner
`TEMPLATES` nodes for middle, leading and trailing comma positions. The
`levelc_arg_template_source_tree` regression proves all three source shapes;
the focused PARSE syntax highlighting checks pass. ARG lowering and LC-AC-71
remain open.

LC-STEP-73C runtime frame checkpoint: `RexxActivationArguments` now owns
ordered values and per-slot presence for one activation. The focused
`testRexxActivationArguments` opt/noopt tests prove omitted, explicit empty,
missing and repeat-read behavior. Compiler-generated main/routine population,
ARG/PARSE consumption, and LC-AC-71 remain open.

LC-STEP-73C call bridge checkpoint: generated main captures the VM argument
array once, and direct CALL/local function sites build a fresh ordered frame
from evaluated actuals. Simple ARG instructions can read the frame repeatedly
in main and routines. The `levelc_arg_frame_main_*` and
`levelc_arg_frame_calls*` opt/noopt tests prove main positions, omitted CALL
slots, explicit empty values, leading/trailing omissions and repeated reads;
the prior procedure runtime and updated canonical tree-shape tests pass.
General ARG templates, PARSE path reuse, ARG BIF and broader invocation/error
qualification remain open. The fixed ARG-derived procedure signature is gone.

LC-STEP-73C shared template checkpoint: one compiler executor now lowers the
supported PARSE and ARG template items through the VM `parseplan` operation.
Word-only templates no longer use a separate `parseWordTemplate` compiler
path; that public runtime method remains for library consumers. ARG traverses
all comma segments, preserving empty positional slots, and writes non-dot
captures through the visible variable pool in authored order. Static and
dynamic literal/position plans use the VM's version-1 and version-2 formats;
version-2 operands read the visible pool or an earlier completed capture.
The `levelc_arg_static_templates*` and `levelc_arg_dynamic_templates*`
opt/noopt fixtures match Regina, including compound/exposed targets and
captured dynamic operands. The prior parser-only `levelc_arg_patterns` fixture
now runs through the full toolchain in both modes. Three former negative tests
for missing/omitted call arguments and dynamic PARSE were replaced by positive
runtime checks because the compiler now accepts those shapes. ARG BIF, broader call modes, errors, BYTE/UTF8 and
linked-image qualification remain open, so LC-AC-71 remains open.

LC-STEP-73E BIF checkpoint: the standalone `RexxClassicBifArg` helper reads
the same activation frame as the ARG instruction. `ARG()` reports the last
explicitly supplied position, while `ARG(n)` retains exact source case and
`ARG(n,'E'/'O')` distinguishes omitted and explicit empty values. The direct
compiler entry reuses the standard BIF argument/context/result-check path and
passes the activation frame as its one additional input. [IBM's ARG BIF
reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=functions-arg-argument)
count/E/O rules and Regina's trailing-omission behavior match the opt/noopt
`levelc_arg_bif*` and `testRexxClassicBifArg*` checks. Invalid index and
option cases signal Classic SYNTAX with authored source positions. Broader
profile, host-entry and BIF reference proof remain open.

LC-STEP-73D qualification checkpoint: main `-a` arguments retain their raw
case for `ARG(n)` while ARG instruction uppercases parsed fields;
`levelc_arg_exact_bytes*` proves an embedded NUL survives both paths;
`levelc_arg_linked` runs a dynamic template from one sealed image;
`levelc_arg_malformed` retains `RXC-LC-46.1` and source position; and the
dynamic canonical tree and twelve-target nested IF/DO cases pass. These
checks join the earlier opt/noopt ARG/PARSE and shared runtime evidence.
LC-AC-71 remains open for the applicable configuration/lifecycle boundary.

**LC-STEP-73F decision proposal — dynamic position error identity.** Regina
raises Error 26.4 for `arg first =(pos) second` when `pos='Q'`. The current
Level C generated `parseplan` instead raises VM `CONVERSION_ERROR` at the
authored ARG clause. The VM's version-2 compact descriptor reserves two header
bytes; assign bit 0 of byte 10 as a Classic numeric-error policy flag, leaving
the current zero value and Level B PARSE exit unchanged. Level C would set the
flag on its version-2 plans. On a dynamic-position conversion failure, the VM
handler would raise `CLASSIC_SYNTAX` with `RXC-LC-26.4` and the authored source
location for flagged plans, retaining `CONVERSION_ERROR` for unflagged plans.
Reject unknown header bits. This adds no AST node or second template executor.
The VM descriptor/header contract and condition mapping are an architectural
shift under AGENTS.md. Adrian approved this exact flag and signal design on
2026-10-04 before code edits.

- [x] **LC-73F-01:** invalid dynamic numeric ARG/PARSE positions yield Classic
  `26.4`, source position and prior output in optimized/noopt Level C runs.
- [x] **LC-73F-02:** existing unflagged Level B version-2 plans still yield
  `CONVERSION_ERROR`; other valid static/dynamic plans and linked images retain
  their current output.
- [x] **LC-73F-03:** the VM descriptor documentation, focused runtime tests
  and Level C architecture record describe the single flagged policy.

1. **LC-STEP-73F.1 (complete; LC-73F-01/02; approved):** extend version-2
   descriptor validation to accept only the Classic policy bit, and select the
   existing `CLASSIC_SYNTAX` signal with `26.4` on flagged conversion failure.
2. **LC-STEP-73F.2 (complete; LC-73F-01/03; depends on 73F.1):** set the policy bit in
   Level C version-2 descriptors, preserving one PARSE/ARG lowering path.
3. **LC-STEP-73F.3 (complete; LC-73F-01/02/03; depends on 73F.2):** compare Regina,
   run flagged/unflagged negative and positive tests in normal Debug and the
   relevant Level C suite, check linked execution, update docs and commit.

LC-STEP-73F evidence: Regina reports 26.4 for invalid dynamic ARG and PARSE
positions. `levelc_arg_bad_dynamic_position*` and
`levelc_parse_bad_dynamic_position*` prove the Classic signal, authored line
and prior output in optimized/noopt runs. The direct version-2 Level B
`nr14_unflagged_parseplan` fixture retains `CONVERSION_ERROR`, while
`nr14_unknown_parseplan_flag` rejects unknown bits; both VM variants pass the
frozen-plan contract. The positive dynamic ARG cases and sealed linked image
pass. Normal Debug core build passed. Eight focused tests passed together.
The broad Level C run passed 438 of 441 tests; its two old negative tests
expected nested dynamic PARSE to remain unsupported and are now four passing
optimized/noopt IF/DO runtime tests, with Regina output comparison. The third
broad exception was an unbuilt SAY host test; it passed after building its
target. Together the retained broad run and five targeted repairs cover all
443 tests in the revised Level C set. ARG instruction closure still needs
the LC-AC-71 configuration and invocation-lifecycle audit.

LC-STEP-73G host-entry checkpoint: the native `rxvml_run()` entry reaches
generated Level C main with its argument vector. `levelc_arg_host_entry`
loads the same module in one host context and runs two arguments, one argument,
one explicit empty argument and zero arguments in sequence. `ARG()` count,
`ARG(n,'E')`, raw `ARG(n)`, and uppercased instruction fields reset for each
activation; the host uses the length-aware SAY callback. The focused host test
passes in normal Debug. CLI main entry remains covered in optimized/noopt
`levelc_arg_frame_main_*`; local CALL frames and omissions remain covered by
`levelc_arg_frame_calls*`. `rxvml_run()` accepts terminated C strings, so it
cannot carry an embedded NUL; internal CALL exact-byte behavior is covered
separately by `levelc_arg_exact_bytes*`. At this checkpoint, generated Level C
main still constructed a BYTE-default configuration. The approved Unicode-first
route superseded that profile proposal; current ARG Unicode and host proof is
recorded under LC-STEP-88E-2A. Full invocation and host lifecycle proof remains
open under LC-AC-71 and the shared LC-AC-04/06 configuration contract.
The host fixture initially used `CALL nested ARG(1),,ARG(2)` and was narrowed
to isolate host ARG entry behavior while direct CALL expressions were open.
LC-STEP-88E-2B now admits that expression/omission shape in compiled Level C,
including recursive frames; it remains a CALL dependency receipt rather than
a completed host or full ARG invocation audit.

**2026-10-05 LC-I-11 whole-instruction closure receipt (supersedes the
earlier open verdict).** Adrian separated cross-dialect and future host
invocation work from the Classic ARG instruction on 2026-10-05. The complete
ARG source form is `ARG [template_list]`, semantically `PARSE UPPER ARG`.
The parser retains every empty comma position; validation and lowering walk
arbitrarily many segments and use the same `parseplan` executor as PARSE.
There is one `RexxActivationArguments` owner per main or direct internal
subroutine/function invocation; ARG rereads its values without consuming
them, and `ARG()` checks the same presence bits. The compiler has one
actual-argument binding loop and no first-ARG signature binding or separate
word-only template lowerer. This is the coherence review for the whole
instruction, not another ARG implementation slice.

| ARG obligation | Closure evidence |
| --- | --- |
| Bare form, word/dot/variable/literal/dynamic patterns, absolute/relative positions, empty/many comma segments | `levelc_arg_patterns*`, `levelc_arg_static_templates*`, `levelc_arg_dynamic_templates*`, `levelc_arg_nested_long*`, source-tree and canonical-tree fixtures pass; shared PARSE `parseplan` path inspected. |
| Uppercasing, Unicode codepoint positions, exact values, omitted versus explicit empty, repeat reads and pool writes | `levelc_arg_unicode*`, `levelc_arg_exact_bytes*`, `levelc_arg_bif*`, `testRexxActivationArguments*`, frame and EXPOSE fixtures pass; `ARG(n)` retains original case while ARG fields uppercase. |
| Main, local CALL/function, nested/recursive and legal statement placement | `levelc_arg_frame_main_*`, `levelc_arg_instruction_invocation*`, `levelc_arg_frame_lifecycle*`, recursive shared/private and direct expression-actual fixtures pass optimized/no-opt; linked ARG and frame tests pass. |
| Malformed templates and reached dynamic position errors | Authored 38.1 template checks and `levelc_arg_bad_dynamic_position*` 26.4 source checks pass; IBM/Regina error categories and instruction placement were reviewed. |
| Native transport for admitted main entry | `levelc_arg_host_entry` and `crexxsaa_arg_lengths` pass Unicode, embedded NUL, invalid UTF8/pointer-length, repeated context and cache paths; the old C-string convenience entry's NUL limit remains tracked under the host interface. |

The guarded product build and focused ARG/host checks passed at the
`9bbdc740f` code checkpoint. Later code/test changes through `3a8c3399a`
passed the normal Debug Level C suite **594/594**, including the ARG, AST and
linked fixtures, plus Level G/RexxScript isolation **10/10**; exact-input
logs are `cmake-build-debug/levelc-signal-implicit-final-build.log`,
`cmake-build-debug/levelc-signal-forms-debug-qual.log` and
`cmake-build-debug/levelc-signal-forms-isolation.log`. The subsequent `45cc83ec4` commit and
this receipt change documentation only. The guarded qualification peaked at
about 4633 MiB aggregate RSS and left zero child processes. `LC-AC-71` and
`LC-I-11` are **closed** for the agreed Classic ARG instruction. External
Classic CALL/INTERPRET must integrate the same frame under their later rows;
cross-dialect calls are out of scope, and host invocation-mode expansion has
a separate owner. PARSE, PROCEDURE, CALL, RETURN, SIGNAL, full Level C and
Release 1 remain open.

**LC-I-12 PROCEDURE plan — vision and intended outcome, 2026-10-05.** A
Classic internal call may execute `PROCEDURE [EXPOSE variable-list]` as its
first processed instruction. It then has a private variable generation while
each exposed scalar, stem or exact compound variable aliases the immediate
caller's pool. A parenthesized list reference exposes its own variable first,
reads its value in the newly visible pool, then exposes the named words in
source order. Nested and recursive calls must bind to their actual callers;
RETURN restores the caller's view. Preserve the approved Unicode scalar and
case-preserved substituted-tail model and the separate Level B/G and
RexxScript contracts. This is one whole-instruction review; CALL/RETURN and
external invocation remain open under their own rows.

1. **LC-74-01 — legal source and diagnostics:** bare PROCEDURE and EXPOSE
   lists of direct simple, stem, compound and parenthesized names have the
   documented raw/canonical AST and no arbitrary list-length limit. Empty
   EXPOSE, malformed parentheses, nonvariable items, invalid tail keywords,
   main-program execution, a late or second execution and nested instruction
   placement report the applicable Classic identity at the authored clause.
   Verify parser trees, compiler diagnostics and runtime 17.1 against the
   [IBM PROCEDURE reference](https://www.ibm.com/docs/en/zos/3.1.0?topic=instructions-procedure)
   and [Regina manual](https://rexxinfo.org/reference/articles/regina.pdf).
2. **LC-74-02 — private pool and direct aliases:** without PROCEDURE an
   internal call shares its caller's pool; with it, unexposed values are
   private and direct scalar/stem/compound EXPOSE aliases the caller's exact
   binding. Resolve compound tails at their source-list position, preserving
   Unicode and case in substituted tails; later changes to a tail variable
   do not retarget the alias. A stem-wide assignment or DROP must also update
   each individually exposed tail in the caller, while other tails stay
   private. Verify read, write, DROP, stem default/tail, duplicate and
   source-order interactions against reference cases.
3. **LC-74-03 — indirect EXPOSE:** a `(symbol)` list exposes that symbol
   first, captures its current text, then validates and applies subsidiary
   words in order through the current private pool. Check empty, Unicode
   blanks, invalid names, direct/indirect mixtures and compound dependencies
   with a reference matrix. Reuse the shared variable-list classifier and
   pool machinery; do not add a PROCEDURE-only token parser.
4. **LC-74-04 — invocation lifetime:** main fallthrough cannot execute
   PROCEDURE; an internal activation may execute it only as its first
   processed instruction and at most once. Labels and comments before it,
   nested calls, recursion, source-order fallthrough, SIGNAL and RETURN keep
   the correct pool and source/error lifetimes. Verify optimized/no-opt and
   linked execution with authored 17.1 and no state leakage.
5. **LC-74-05 — delivery and isolation:** simplify any duplicated direct or
   indirect exposure path, retain source documentation tags, qualify focused
   pool/compiler cases during development, then run the core product build,
   relevant normal Debug Level C suite and affected Level B/G/RexxScript
   checks once on exact code/test/build inputs. Record the reference matrix,
   structural evidence, test logs, remaining adjacent instruction owners and
   a coherent commit before closing LC-I-12.

1. **LC-STEP-74A (LC-74-01–04; complete 2026-10-05):** inventory the full grammar and
   current guard, source/canonical tree, shared pool alias operations and
   retained first-instruction evidence. Probe direct/indirect ordering,
   compound-tail identity and invalid forms in Regina before product edits.
2. **LC-STEP-74B (LC-74-02/03; complete 2026-10-05; depends on 74A):** select one pool-owned
   exposure path for direct and indirect names. The existing compiler guard
   accepts only direct scalar/stem targets; the existing pool alias methods
   normalize whole names and need review for case-preserved compound tails.
   If exact compound aliases require a new runtime representation or other
   architectural shift, record the design and obtain Adrian's approval
   before implementation. Preserve the completed DROP/assignment contracts.
3. **LC-STEP-74C (LC-74-01–04; complete 2026-10-05; depends on 74B and its decision gate):**
   implement the complete source-ordered EXPOSE path, remove obsolete
   scalar/stem-only guards, and repair shared pool operations needed for
   exact aliases. Keep the first-processed-instruction runtime check in the
   existing activation frame. Add permanent positive and negative reference
   regressions, raw/canonical tree checks and linked proof.
4. **LC-STEP-74D (LC-74-01–05; complete 2026-10-05; depends on 74C):** audit the whole instruction
   against the reference and AST matrix, run the relevant normal correctness
   suite once with guarded memory/process monitoring, update architecture and
   reference docs, commit one coherent PROCEDURE increment and close the row
   only when every LC-74 criterion passes. Keep CALL/RETURN/SIGNAL and full
   Level C/Release 1 visibly open under their own criteria.

**LC-STEP-74A initial reference and implementation receipt, 2026-10-05.**
The [Regina language reference](https://rexxinfo.org/reference/articles/regina.pdf)
specifies `PROCEDURE [EXPOSE varref ...]`, where each `varref` is a symbol or
parenthesized symbol. It says an indirect reference is itself exposed before
its value is split into names, all entries bind left-to-right after the new
private generation is selected, and a compound tail's resolved name remains
the alias target if its substitution variable later changes. Guarded Regina
probes in `cmake-build-debug/levelc-procedure-regina-reference.log` and
`levelc-procedure-alias-reference.log` confirm indirect list words `one` and
`two` alias the caller while invalid `/` is ignored; unexposed `ghost`
stays private. An exposed `A.b` survives DROP/reassignment without modifying
distinct `A.B`; reversing `key A.key` to `A.key key` changes the alias target.
Each guarded probe exited with zero residual processes. A guarded current
compiler probe fails at its explicit `compound PROCEDURE EXPOSE is outside
slice` guard. The raw parser tree already retains direct `VAR_TARGET` and
indirect `VAR_REFERENCE` entries in source order under one `ARGS` node;
`cmake-build-debug/levelc-procedure-source-tree.log` retains the tree.
The shared pool's current `exposeValue`/`exposeStem` helpers normalize whole
names, which would merge case-distinct substituted compound tails. The
[IBM IRX0017I description](https://www.ibm.com/docs/en/zos/3.1.0?topic=irx-irx0017i)
also confirms that `PROCEDURE` must be the first instruction executed after
an internal call. Thus `IF 1 THEN PROCEDURE` has already executed IF and is
invalid in this programme's IBM-led contract, even though Regina permits
some later placements; the existing source-position 17.1 fixture covers it.
The
proposed `74B` path adds an exact compound-alias lookup owned by
`RexxVariablePool`, extends `RexxPoolAlias` to target the caller's exact
stem/tail, and makes direct/indirect EXPOSE use one pool service. Pool
`symbolValue`, `symbolHasValue`, `setSymbolValue` and `dropSymbol` will consult
that map using the fully resolved case-preserved compound name before the
ordinary local stem. Fixed-stem/tail parent operations must follow an alias
chain in nested procedures without re-substituting its tail. A separate
compound map avoids changing the existing case-insensitive scalar/stem keys;
the alias belongs only to the new private pool and expires with its
activation. A third guarded Regina probe in
`cmake-build-debug/levelc-procedure-nested-reference.log` confirms two nested
EXPOSE levels update the original lowercase tail while the uppercase tail
is unchanged. This is a
shared pool representation change, so no product edit will implement it
before Adrian's decision. The VM ISA and approved frame architecture need no
change.
**LC-STEP-74B decision, 2026-10-05:** after reviewing this concrete design,
Adrian instructed the programme to continue. This approves the pool-owned
case-preserving exact compound-alias map and one direct/indirect EXPOSE service
described above. It does not approve a new VM operation or a change to the
Classic/non-Classic invocation boundary. LC-STEP-74C may implement this
representation; all LC-74 acceptance criteria remain open until qualified.
Reusing an ordinary scalar alias would uppercase and merge `A.b` with
`A.B`; exposing the whole stem would wrongly share every tail. The exact
compound map is the smallest reviewed representation that preserves both
existing scalar/stem aliases and the reference's per-tail behavior.
Further guarded Regina probes in
`cmake-build-debug/levelc-procedure-compound-stem-setall-reference.log` and
`levelc-procedure-compound-stem-dropall-reference.log` show that assigning
or dropping `A.` in the private procedure also assigns or drops the
individually exposed caller tail `A.b`, while unexposed caller tail `A.B`
remains unchanged. The same pool-owned alias map must participate in those
stem-wide operations; alias lookup only on individual reads and writes would
be incomplete. Both probes exited with zero residual processes.

**LC-STEP-74D whole-PROCEDURE closure receipt, 2026-10-05.** The approved
pool-owned exact compound-alias map preserves `A.b` separately from `A.B`,
stores a fixed resolved caller name, and follows nested alias chains. One
`exposeSymbol` method handles scalar, stem and compound direct entries;
`exposeIndirect` exposes its own reference before reading and walking its
captured list. Indirect EXPOSE and DROP use the same Unicode/configured-blank
variable-list cursor and invalid-word classifier. Whole-stem assignment and
DROP propagate to individually exposed caller tails. The compiler removed its
compound/direct-only guard and emits source-anchored calls in list order from
the existing raw `VAR_TARGET`/`VAR_REFERENCE` tree. No new VM operation or
frame architecture change was needed. Inspection of the recursive
`variable_list` grammar and unbounded ARGS walk found no fixed exposure count.

The Regina-matched permanent matrix covers bare private and shared pools,
direct and indirect lists (including empty, duplicate and invalid entries),
source-order substitution, distinct case-preserved compound tails, stem
defaults/reset/DROP, nested aliases, recursion, a same-frame SIGNAL transfer,
ARG after the pool transition and a Unicode substituted tail. Guarded Regina
receipts are `cmake-build-debug/levelc-procedure-whole-regina-final.log`,
`levelc-procedure-lifecycle-regina.log` and
`levelc-procedure-unicode-regina.log`; the earlier stem-wide and invalid-form
reference logs above remain applicable. The permanent source/canonical tree
test verifies direct and indirect shapes and no surviving Level C PROCEDURE
node. Source-invalid forms report authored 20.1/25.17; existing main, late,
second and nested execution tests retain 17.1 at their causing clauses.

The guarded core product build passed in
`cmake-build-debug/levelc-procedure-core-build.log`. The normal Debug Level C
suite passed **602/602** in `cmake-build-debug/levelc-procedure-debug-final.log`
after two legacy tree assertions were updated from the retired
`exposeValue`/`exposeStem` compiler calls to `exposeSymbol`. The earlier
600/602 run failed only those stale structural assertions; each passed alone
after correction. The final suite peaked at **4962.9 MiB** aggregate RSS
and left zero child processes. Affected Level B/G and RexxScript isolation
passed **10/10** in `cmake-build-debug/levelc-procedure-isolation.log`.
`LC-74-01–05` and `LC-I-12` are closed. CALL, RETURN, EXIT, PARSE, SIGNAL,
full Level C and Release 1 criteria remain open. The next strict queue row
is LC-I-13 CALL.

**LC-I-13 CALL plan — vision and intended outcome, 2026-10-05.** A Classic
`CALL` instruction invokes an internal label, shared built-in, or external
Classic routine using one source-ordered argument and result contract. A
quoted target bypasses internal labels. `CALL ON/OFF` installs the four
callable condition traps; a delivered trap calls its target at a clause
boundary, then resumes the interrupted activation with its caller's policy
intact. The existing one-body frame, activation arguments, shared BIFs and
condition event are the foundation. Preserve Unicode scalar arguments and
results, the fixed Latin-1 ordinal bridge for byte BIFs, exact source
positions, Level B/G and RexxScript isolation, and the newly permitted
fixed-signature Level B/G entry while excluding arbitrary cross-dialect
calls. This is one whole-instruction review;
RETURN, EXIT, ADDRESS and SIGNAL retain their own instruction rows while
their shared dependencies are implemented here where CALL needs them.

1. **LC-75-01 — syntax and diagnostics (closed 2026-10-06):** direct symbol and quoted-string
   targets, no arguments, arbitrary source-ordered expression/omission lists,
   `ON/OFF ERROR|FAILURE|HALT|NOTREADY` and optional `ON ... NAME target`
   retain the authored AST. Bare/bad CALL, malformed argument expressions,
   bad condition/NAME tails and targets absent at caller compilation report
   the applicable Classic identity at the causing clause. A signed provider
   available at compilation but omitted from the image retains the ordinary
   core `FUNCTION_NOT_FOUND` result and source location under the accepted
   static boundary. Verify parser/canonical trees,
   source diagnostics, and the [IBM CALL reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=instructions-call).
2. **LC-75-02 — routine resolution and arguments (closed 2026-10-06):** an unquoted target
   resolves a callable local label before a shared BIF and then an external
   Classic routine; a quoted target skips the local label. Evaluate each
   supplied expression once, left to right, preserve omitted versus explicit
   empty positions, and give each invoked Classic routine its own activation.
   An internal call shares its caller's pool until PROCEDURE changes that
   invocation; external Classic routines start an implicit private pool and
   default internal settings while host services remain available. Test
   collisions, missing targets, nested calls,
   recursion, procedure exposure, Unicode/NUL and linked execution. A
   general cross-dialect adapter is outside this criterion; the specifically
   signed Level B/G entry is included by Adrian's later clarification.
3. **LC-75-03 — result lifecycle (closed 2026-10-06):** internal, BIF and external subroutine
   returns share one result-presence path. Returned text sets both `RESULT`
   and `.RESULT`; a value-less completion drops them. An explicit empty
   string counts as a result. CALL does not assign `RC`. Nested calls and
   aliases must observe the correct sequence; expression calls still enforce
   their separate required-value rule. Verify reference output and
   optimized/no-opt, source and linked tests.
4. **LC-75-04 — external Classic routine lifecycle (closed 2026-10-06):** find a separately
   compiled Classic provider through the compiler's source/binary import
   inventory and invoke it when included in the linked image,
   pass the same argument frame and call-kind state, return its optional
   result, and restore the caller's state and source/error identity across
   success, missing target and nested external calls. Keep the active host
   environment, streams, traps and variable-pool API available to the
   external adapter without exposing ordinary caller variables to the new
   Classic programme. Verify packaged and configured-host execution with
   the full toolchain. Providers appearing only after caller compilation are
   outside the accepted static signed boundary; recompile the caller to bind
   one. An available provider omitted from the image retains ordinary core
   `FUNCTION_NOT_FOUND` behavior. A new loader/VM boundary would require a
   separate architectural decision.
5. **LC-75-05 — delayed condition calls (closed 2026-10-06):** `CALL ON/OFF` and `SIGNAL ON/OFF`
   replace one another for a condition; the handler target defaults to the
   condition name or follows `NAME`. ERROR, FAILURE, HALT and NOTREADY use
   delayed clause-boundary delivery, suppression or buffering while a trap is
   active as specified by the [IBM condition reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=reference-conditions-condition-traps).
   A handler is an ordinary subroutine with correct ARG/RESULT, `SIGL`,
   CONDITION data and caller-policy restoration. Prove re-enable/OFF,
   nested/recursive calls, local/BIF/external handler targets, and the
   available event producers; keep missing ADDRESS, I/O and host event
   producers open with their owner rows rather than claiming them here. A
   handler provider omitted from the linked image follows the accepted
   static-boundary core error behavior.
6. **LC-75-06 — coherent delivery (closed 2026-10-06):** replace overlapping CALL/BIF/function
   dispatch and result paths with reviewed shared helpers, retain source
   documentation tags, add a complete reference and negative matrix, and
   run focused compiler/runtime tests during development. On final code/test
   inputs run the core product build, relevant normal Debug Level C suite,
   affected Level B/G/RexxScript checks, and opt/no-opt and linked cases
   once. Record exact logs and process-memory/exit status, update the
   architecture/reference/coverage rows, commit coherent increments and
   close LC-I-13 only when LC-75-01–06 pass.

1. **LC-STEP-75A (LC-75-01–05; complete):** inventory every parser form,
   resolution/result/error rule, existing lowerer/BIF/VM/host mechanism and
   retained CALL evidence. Build guarded Regina reference probes for
   collisions, omissions, results, conditions and external routines. Keep
   the matrix and open owners in this worklist before product edits.
2. **LC-STEP-75B (LC-75-02–04; complete; depends on 75A):** design one invocation
   resolver and result-presence path around the existing Classic activation.
   Specify the accepted compile-time import and linked call boundary,
   including paths, source identity, Unicode lengths and lifetime. Reuse the
   existing module system. Obtain Adrian's decision before any new loader or
   VM architecture; already approved frame/AST work needs no repeat approval.
3. **LC-STEP-75C (LC-75-01–04; complete; depends on 75B and its decision gate):**
   implement local/BIF/external calls, quoted bypass, ordered actuals,
   optional result writes/drops and errors through shared compiler/runtime
   paths. Add permanent source, AST, opt/no-opt and linked regressions.
4. **LC-STEP-75D (LC-75-05; complete; depends on 75C):** implement CALL ON/OFF and
   delayed condition-call state on the same activation/frame model; exercise
   available condition producers and handler lifetime without pre-emptively
   closing ADDRESS, I/O, SIGNAL or host instruction owners.
5. **LC-STEP-75E (LC-75-01–06; complete; depends on 75C/75D):** audit the entire CALL
   reference matrix, run the relevant normal suite once with guarded child
   and memory monitoring, update documentation and evidence, commit the
   coherent whole-instruction result, then start LC-I-14 RETURN. Keep full
   Level C and Release 1 criteria open until their own proofs pass.

**LC-STEP-75A reference and implementation inventory, in progress
2026-10-05.** The [IBM CALL reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=instructions-call)
confirms local/BIF/external order, quoted bypass of local labels, evaluated
arguments and omission positions. The [IBM external-routine reference](https://www.ibm.com/docs/en/SSLTBW_2.3.0/pdf/ikja300_v2r3.pdf)
specifies an implicit private procedure for an external Rexx programme,
including hidden caller variables and default internal settings. The
[IBM condition reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=reference-conditions-condition-traps)
requires CALL traps at clause boundaries, a DELAY state during the handler,
replacement by later CALL/SIGNAL ON/OFF, and restoration on internal return.
The current parser has direct symbol/string targets, ordered structured
argument expressions and omissions, and ON/OFF/NAME recovery nodes. The
lowerer currently accepts only an unquoted local label as a direct CALL; its
separate expression path chooses a local function or one of the shared BIF
entries. The local path creates the approved activation frame but discards its
optional return. `RexxActivationArguments` already holds argument presence,
the call kind, return presence/value and inherited SIGNAL policy.

| Reference probe, guarded in `cmake-build-debug/` | Observed behavior | Remaining implementation owner |
| --- | --- | --- |
| `levelc-call-reference-main.log`, `levelc-call-reference-dotresult.log` | Local `LENGTH` shadows the BIF; quoted uppercase `LENGTH` reaches the BIF; three positions include a middle omission and explicit empty value; a value return sets `RESULT` and `.RESULT`, bare RETURN drops both, and ordinary CALL leaves the pre-set `RC=77`. | Unified CALL resolver, BIF/local invocation and optional result update. |
| `levelc-call-reference-quoted-case.log` | Quoted lowercase `length` does not resolve the uppercase built-in on Regina; quoted target spelling must not be blindly uppercased. | Preserve decoded quoted target spelling and check the configured external adapter. |
| `levelc-call-reference-external.log` | A sibling `callext.rexx` receives a separate argument frame and returns `a-ok`/`b-ok` into `RESULT` for unquoted and quoted uppercase calls. | External Classic configuration adapter and entry ABI. |
| `levelc-call-reference-external-state.log` | External entry sees `shared` as unassigned and default `DIGITS=9`; after it changes both, its caller still sees `shared=caller` and `DIGITS=20`, plus the returned result. | Private external programme state with preserved caller lifecycle. |
| `levelc-call-reference-condition.log`, `levelc-call-reference-policy.log` | ERROR from a host command calls a no-argument handler at the clause boundary, reports CONDITION `C=ERROR`/`I=CALL` and the raising `SIGL`, then resumes. A nested call's OFF does not remove its caller's ON; the caller handler runs after return. | Delayed CALL ON/OFF state and clause-end delivery; ADDRESS remains the real command producer owner. |
| `levelc-call-reference-quoted-handler.log` | Regina accepts a quoted `NAME 'HANDLER'`, consistent with the existing parser's target shape; platform reference variants may differ. | Preserve the accepted source form and test the named handler route. |
| `levelc-call-reference-delay-state.log`, `levelc-call-reference-delay-nested.log` | A CALL trap sees `CONDITION('S')=DELAY`, `CONDITION('I')=CALL`, zero arguments and the causing `SIGL`; its returned value leaves the prior `RESULT` intact. The trap handles a later ERROR again, suppresses ERROR raised while already handling it, and a later SIGNAL ON replaces CALL ON with immediate transfer. The interrupted caller's prior CONDITION state is restored after the handler returns. | Per-activation delayed state, source-preserving clause checkpoint, handler activation and CALL/SIGNAL replacement. |
| `levelc-call-reference-missing-trap.log` | A missing delayed `CALL ON ERROR NAME` target raises `16.1` at the condition-raising `ADDRESS` clause (line 2), not at the preceding policy clause (line 1). | The compiled dispatcher must retain dynamic causing-clause source identity when its selected handler is absent; this remains open. |
| `levelc-call-fallthrough-full-regina.log`, `levelc-call-fallthrough-matrix-focused-v2.log` | A local CALL whose label has no PROCEDURE or explicit RETURN falls through to end-of-programme and returns without a value; the caller drops RESULT. The permanent resolution/result fixture now includes this case. Its full expected output matches Regina byte for byte and the focused source/linked/opt/no-opt matrix passes 4/4 after CMake regeneration. | Retain it in the complete CALL result lifecycle; whole CALL and external routines remain open. |
| `levelc-call-reference-off-name.log`, `levelc-call-reference-on-extra.log` | `CALL OFF ERROR NAME foo` fails with `21.1` at NAME; `CALL ON ERROR NAME foo bar` fails with `19.3` at the extra symbol. | The Level C parser must reject surplus CALL policy tails instead of silently recovering and emitting a valid policy. |

Each guarded probe exited with zero residual child processes. These are
reference receipts, not product tests or CALL closure.

**LC-STEP-75A surplus-tail repair plan.** A minimal guarded compiler probe
currently accepts both surplus-tail forms above: Lemon records a recoverable
syntax error but the recovered CALL AST loses the unexpected token. Add
explicit CALL policy/NAME-tail grammar recovery that anchors `21.1` and
`19.3` at those tokens, preserving the authored AST error and following
clause. Retain a normal compile-diagnostic regression for both forms, check
the neighbouring valid policy forms and optimizer-independent runtime cases,
then qualify the final code/test inputs with the normal Level C suite. This is
a parser diagnostic repair under `LC-75-01`, not a new language form.

**LC-STEP-75A remaining diagnostic matrix plan (LC-75-01; active).** Inventory
the grammar's missing/bad direct target, malformed expression list, missing
or invalid ON/OFF condition, missing/bad/extra ON NAME target and OFF tail
forms against guarded Classic reference probes. Add one permanent compile
fixture that checks each Classic identity and authored source location while
retaining following-clause recovery; repair any mismatched grammar path
without adding a new CALL form. Check adjacent valid policy and call forms,
then qualify with focused compiler tests and the relevant normal correctness
suite once at the coherent code/test checkpoint. This completes the syntax
and diagnostic submatrix but cannot close `LC-75-01` until runtime external
missing-target errors are also source-anchored.

**LC-STEP-75A constant-symbol correction plan (LC-75-01/02; complete as a
CALL grammar checkpoint, not CALL closure).** The
reference audit found that `CALL 7` is a legal direct target: Regina calls a
local `7:` label, and the IBM CALL reference allows a constant symbol as the
routine name. Guarded Regina also calls local `1.2:`, `.5:`, `7E2:` and `7dogs:`
labels from their unquoted CALLs. The initial diagnostic fixture incorrectly
treated `CALL 7` as `19.2`; the earlier source-recovery fixture has the same
outdated assumption. Before a product edit, split direct routine targets from
the stricter ON/NAME handler-target grammar so integer, decimal and
digit-starting constant symbols enter the existing local/BIF/external
resolution path, while numeric `NAME` still reports `19.3`. Preserve literal
spelling, quoted local bypass and source order. Replace the invalid numeric
case with a genuinely invalid punctuation target, add permanent positive
local-target reference/opt/no-opt/linked proof, retain following-clause
diagnostic recovery, and run focused checks before one final normal Debug
Level C checkpoint. This is a correction to the reference-defined CALL form,
not a new language direction or a numeric-only dispatch implementation.

**LC-STEP-75A surplus-tail receipt, 2026-10-05.** The Level C grammar now
retains unexpected text after `CALL OFF condition` as an authored `21.1` AST
error and after `CALL ON condition NAME target` as an authored `19.3` AST
error. The permanent four-case compile regression covers symbol and numeric
OFF tails, plus symbol and quoted-string ON/NAME tails, with exact source
locations. A quoted lowercase handler also passes the controlled compiled
CALL trap test; the IF/WHEN/DO fixture now has its handler mutate source
variables after their branch results are captured. Guarded final-input
focused Debug passed **8/8** in `cmake-build-debug/levelc-call-tail-focused.log`;
focused ASan with macOS leak detection off passed **7/7** in
`cmake-build-debug/levelc-call-tail-asan-focused-guard.log`. The normal Debug
Level C suite passed **613/613** in
`cmake-build-debug/levelc-call-tail-final-levelc.log` (peak **4909.2 MiB**
aggregate RSS); Level B/G and RexxScript isolation passed **10/10** in
`cmake-build-debug/levelc-call-tail-isolation.log`. All guarded builds/tests
left zero residual children. Direct/external missing targets, dynamic trap
source identity, real producers and the remaining CALL reference matrix still
keep `LC-75-01–06` and LC-I-13 open.

**LC-STEP-75A direct diagnostic, constant-symbol and fallthrough checkpoint,
2026-10-05.** The single compile fixture
`levelc_call_diagnostic_matrix.rexx` checks eight source-anchored errors in
one recovered programme: bare CALL, invalid punctuation target, missing and
invalid ON/OFF conditions, and missing or invalid ON/NAME targets. Bare CALL
now fills its existing `19.2` token detail with
`end-of-clause`; it previously printed the unexpanded `{token}` placeholder.
The separate surplus-tail and malformed-expression tests retain the other
CALL grammar errors. Regina confirms these identities and locations. The
earlier numeric `19.2` expectation was removed: the scanner now recognizes
numeric and digit-starting constant-symbol labels, and direct CALL routes
integer, decimal, leading-dot, exponent and other constant-symbol targets
through the ordinary local-label resolver. ON/NAME retains its distinct
numeric `19.3` error. A signed-exponent CALL may resolve externally, but a
signed-exponent label is not admitted; Regina likewise rejects `7E+2:`.
The permanent local-result fixture also covers a label without PROCEDURE or
RETURN: falling through to the programme end drops RESULT. Its expected
output, including all five constant-symbol local calls, matches guarded
Regina byte for byte in `cmake-build-debug/levelc-call-constant-full-regina-v3.log`.
Source, linked, optimized and no-opt focused execution pass. The guarded
focused normal Debug matrix passed **8/8** in
`cmake-build-debug/levelc-call-constant-focused-v3.log`; focused ASan passed
**8/8** with macOS leak detection off in
`cmake-build-debug/levelc-call-constant-asan-focused-v3-guard.log`.
The guarded final-input core product build passed in
`cmake-build-debug/levelc-call-constant-final-core-build.log`.
The normal Debug Level C suite passed **618/618** in
`cmake-build-debug/levelc-call-constant-final-levelc.log` (peak **5115.0 MiB**
aggregate RSS). Affected Level B/G and RexxScript isolation passed **10/10**
in `cmake-build-debug/levelc-call-constant-isolation.log`. All guarded runs
left zero residual child processes. Only documentation changed after the
final code/test checkpoint. Runtime
missing-target diagnostics, external routines and the rest of CALL remain
open.

**LC-STEP-75A leading-period CALL correction plan, 2026-10-05
(LC-75-01/02/06; complete).** IBM's token rules classify period-starting
symbols as constants. Guarded Regina execution accepts local `CALL .foo`,
`CALL ..foo`, `CALL .5abc` and `CALL .`, with corresponding labels
(`cmake-build-debug/levelc-call-period-matrix-regina.log`). The current `rxc`
rejects `.foo` at the CALL and label (`levelc-call-leading-period-rxc-v3.log`).
Regina rejects `CALL ON ERROR NAME .foo` as `19.3`
(`levelc-call-period-name-regina.log`), so the existing stricter NAME grammar
must remain. This fills a legal direct CALL target within the approved Classic
contract; it does not introduce a new language form.

1. `75A-P1` (LC-75-01): recognize the complete period-starting symbol as one
   token, including repeated periods and a single-period label, while
   retaining numeric `.5`, PARSE's placeholder `.`, and NAME rejection.
   Preserve the five reserved period symbols (`.MN`, `.RESULT`, `.RC`, `.RS`,
   `.SIGL`) as pool-valued symbols in expression contexts; a plain constant
   classification would break the existing `.RESULT` CALL result contract.
   Admit the standalone `.` token only as a direct CALL routine target.
2. `75A-P2` (LC-75-01/02; depends on P1): route those target spellings through
   the existing local-label resolver and source-order fallthrough path; prove
   Regina-matched optimized/no-opt and linked output, raw/canonical shape,
   and negative NAME/placeholder behavior with permanent regressions.
3. `75A-P3` (LC-75-06; depends on P2): run focused Debug and maintained
   sanitizer checks, core build and the relevant normal Level C suite on the
   final code/test inputs, plus affected Level B/G and RexxScript isolation;
   retain guard memory/child receipts, update architecture and commit the
   coherent CALL correction. Keep whole CALL open for external resolution and
   remaining trap/reference obligations.

**75A-P1–P3 receipt, 2026-10-05.** The scanner now keeps period-starting
routine/label spellings whole, with `.5` still numeric and the single `.`
still a PARSE placeholder outside direct CALL. The five reserved period
symbols remain pool-valued; the initial focused run exposed and repaired a
`.RESULT` classification regression before qualification. Direct CALL's
standalone `.` uses the same literal-label resolver as `.foo`, `..foo`,
`.5abc` and reserved `.RESULT`. The stricter ON/NAME grammar rejects ordinary
period constants, while Regina accepts reserved `.RESULT` there. The expanded
source, opt/no-opt, linked and raw/canonical tree fixtures match Regina
byte-for-byte (`cmake-build-debug/levelc-call-period-full-regina-v2.log`).
The negative matrix also retains following-clause recovery and source
locations. PARSE dot tests were included to check the shared scanner.
The final-input guarded Debug focused run passed **11/11** in
`cmake-build-debug/levelc-call-period-focused-v4.log`; the maintained macOS
ASan focused run passed **11/11** with unsupported leak detection off in
`cmake-build-debug/levelc-call-period-asan-focused-guard.log`. The guarded
core build passed in `cmake-build-debug/levelc-call-period-reconfigure.log`.
The normal Debug Level C suite passed **618/618** in
`cmake-build-debug/levelc-call-period-final-levelc.log` (peak **4894.2 MiB**
aggregate RSS), and affected Level B/G/RexxScript checks passed **10/10** in
`cmake-build-debug/levelc-call-period-isolation.log`. Every guarded build and
test left zero residual child processes. The wider period-symbol expression
and error matrix remains open under `LC-AC-04/08`; external CALL, real
condition producers, runtime missing-target behavior and the rest of
`LC-75-01–06` keep LC-I-13 open.

**LC-STEP-75A encoded CALL target and binary-grouping plan, 2026-10-05
(LC-75-01/02/06; complete).** A quoted hex or binary string is a Classic CALL
name. Guarded Regina calls the shared `LENGTH` BIF through both encodings and
returns `hex=3`/`binary=4` (`cmake-build-debug/levelc-call-encoded-regina.log`).
With no embedded blanks, the current full cREXX toolchain gives the same
output (`levelc-call-encoded-noblank-{rxc,assemble,link,run}.log`). The
source-valid byte-grouped binary form instead fails at its second separator
with `15.2` (`levelc-call-encoded-rxc.log`): the common lexical validator
accepts four-bit groups only, while the [IBM token rule](https://www.ibm.com/docs/en/zvm/7.3.0?topic=syntax-tokens) admits four- or
eight-bit groups and a shorter first group. This is a shared literal
validation repair required by the active CALL target form, not a new syntax
choice. A function/literal, PARSE and RexxScript isolation check must prevent
cross-consumer fallout.
Regina's guarded grouping matrix accepts nibble, byte, mixed and partial
groups (`levelc-binary-grouping-regina.log`), also accepts a five-bit initial
group (`levelc-binary-grouping-bad-first-regina.log`), and rejects a five-bit
later group as `15.2` (`levelc-binary-grouping-bad-later-regina.log`). The
initial group remains permissive, preserving its existing padding behavior.

1. `75A-B1` (LC-75-01/02): establish Regina/reference valid and invalid
   grouping cases, including byte, nibble, mixed, initial partial and bad
   later five-to-seven-bit groups. Regina also accepts a five-bit initial
   group, so retain the current first-group padding behavior. Keep hex
   grouping and the existing `15.2`
   source diagnostic stable.
2. `75A-B2` (LC-75-01/02; depends on B1): use one reviewed binary grouping
   validator for all Level C literal contexts, then add a permanent CALL
   hex/binary target fixture with source-order arguments, opt/no-opt, linked
   output and target-tree proof. Add negative source diagnostics and relevant
   expression/PARSE checks without a CALL-specific decoding path.
3. `75A-B3` (LC-75-06; depends on B2): run focused normal and maintained
   sanitizer checks, final-input core build and normal Level C suite once,
   plus affected Level B/G/RexxScript isolation; retain guard exit/memory
   evidence, update docs and commit. Whole CALL stays open for external
   resolution, missing targets and condition/source lifecycle.

**75A-B1–B3 receipt, 2026-10-05.** One shared Level C source validator now
accepts four- and eight-bit noninitial binary groups, including mixed groups,
without changing hex pairs or the initial group's left padding. The
source-invalid five- and six-bit later groups retain `15.2` at their literal
positions in one recovered source. The expanded
CALL fixture checks grouped hex and binary quoted BIF names, ordered actuals,
mixed/partial literal expressions and PARSE VALUE through the same ordinal
route. Its expected output matches guarded Regina byte-for-byte
(`cmake-build-debug/levelc-call-encoded-final-regina.log`), with source,
optimized/no-opt, linked and tree coverage. The final-input guarded normal
Debug focused matrix passed **7/7** in
`cmake-build-debug/levelc-call-encoded-final-focused.log`; maintained macOS
ASan passed **7/7** with unsupported leak detection off in
`cmake-build-debug/levelc-call-encoded-asan-focused-guard.log`. The guarded
core build passed in `cmake-build-debug/levelc-call-encoded-final-core-build.log`.
The normal Debug Level C suite passed **619/619** in
`cmake-build-debug/levelc-call-encoded-final-levelc.log` (peak **5218.2 MiB**
aggregate RSS); affected Level B/G/RexxScript isolation passed **10/10** in
`cmake-build-debug/levelc-call-encoded-isolation.log`. All guarded processes
exited without residual children. This closes the encoded direct CALL target
and shared binary-grouping submatrix, not LC-I-13 or the full Level C literal
matrix. External Level C/fixed-signature B/G resolution, missing targets and
condition/source lifecycle remain open.

**LC-STEP-75E local invocation coherence receipt, 2026-10-05.** Expression
functions, ordinary CALL and delayed local handlers now use one
`levelc_build_local_invocation` builder for frame creation, pool/config
binding, source-ordered actuals, entry selection and the compiled body call.
Expression lowering reuses its already resolved local target. Distinct
epilogues remain explicit: a function requires a returned value, ordinary
CALL writes/drops `RESULT` and `.RESULT`, and a delayed handler ignores its
return. The guarded focused normal Debug checks passed **15/15** in
`cmake-build-debug/levelc-call-local-common-focused.log`; focused ASan with
macOS leak detection off passed **8/8** in
`cmake-build-debug/levelc-call-local-common-asan-focused-guard.log`.
The final-input normal Debug Level C suite passed **613/613** in
`cmake-build-debug/levelc-call-local-common-final-levelc.log` (peak
**4642.1 MiB** aggregate RSS); Level B/G and RexxScript isolation passed
**10/10** in `cmake-build-debug/levelc-call-local-common-isolation.log`.
Every guarded build/test run left zero residual children. This closes one
duplicate compiler path under `LC-75-06`, not the whole criterion or CALL.

The missing-handler source review found an existing VM `signalorigin` route
used by immediate SIGNAL ON SYNTAX. It requires a VM-bound runtime signal
object carrying the raising module/address. The current delayed CALL queue
retains a Unicode description and source line, but no bound origin object;
its dispatcher can therefore set `SIGL` correctly while its unhandled `16.1`
still points at generated dispatch code. A producer/origin transport review
must resolve that gap before claiming `LC-75-01/05`; a numeric line alone is
insufficient for exact runtime source identity.

The VM can autoload
precompiled bytecode from exact package stems and supports nested host calls;
`crexxsaa` can compile/cache and run source. Neither existing top-level
entry carries omitted positions and an optional Classic result across an
external CALL, and ordinary bytecode autoload does not discover/compile
Classic source. LC-STEP-75B must define that adapter before external product
edits. `LC-I-13`, LC-75-01–06, full Level C and Release 1 remain open.

**LC-STEP-75B historical broad external architecture proposal, superseded for
the initial target set.** The following configuration-owned native provider
and source-cache proposal predates Adrian's fixed-signature Level B/G scope
clarification. It is retained as design history and is not the active CALL
implementation route; the descriptor-safe same-context proposal below is the
current decision gate.
Keep one compiler-side resolver: an unquoted local label wins, otherwise an
exact-case shared BIF wins, otherwise route to `Config_ExternalRoutine`;
quoted targets skip only the local search. Capture arguments and omission
flags once in the existing activation frame. Local and external Classic
subroutines report optional return presence through that frame; the BIF
adapter always supplies a result. One CALL epilogue updates or drops
`RESULT`/`.RESULT`; a condition-handler call deliberately preserves the
prior RESULT instead. The guarded Regina
`levelc-call-reference-trap-result.log` confirms that a handler returning
`handled` leaves its caller's existing `RESULT=old` intact.

Add a **configuration-owned external routine service** with a versioned
length-aware request/result contract: exact target UTF8 bytes and length,
source location, subroutine/function kind, ordered argument UTF8 spans and
presence bits, caller environment/stream and variable-pool access handle;
response distinguishes not found, optional returned Unicode text, and
condition/error. The C-string convenience surfaces do not define this ABI.
The default Classic provider would search the current program's directory,
then configured module roots in order, for exact-name precompiled `.rxbin`
and `.rexx` source, without falling back to an OS command. A source hit uses
the existing `crexxsaa` compiler/cache path; both source and bytecode hits
load into the current VM context and invoke a dedicated Classic entry with
the same presence-bearing frame. That entry creates its own pool and default
internal settings, while host services remain available, then records the
optional result. Host-registered providers can supply a native external
routine ahead of file search without changing CALL syntax. Reuse existing
module load and nested-call machinery; add no RXAS opcode. Both the ordinary
`rxvm` driver and `crexxsaa` must register the same core service, with the
source compiler/cache configured when available. An independent VM context
or `rxvml_run()` string-only argv would lose omissions, return presence or
host state, so neither is the proposed call path. If a safe same-context
nested load/call needs a new VM contract after prototype, return that exact
change for a separate decision before editing it. This proposal implements
the already specified `Config_ExternalRoutine` boundary but adds a public
runtime/host entry ABI and default search policy, so approval is required
under `AGENTS.md`; no product edit has implemented it yet.

**LC-STEP-75B scope clarification, 2026-10-05.** Adrian now permits Level C
CALL to reach other Level C routines and Level B/G routines with a specific
signature. This supersedes the earlier blanket cross-dialect exclusion only
for that explicitly typed Level B/G entry; arbitrary Level B/G calls and a
general Classic/non-Classic adapter remain outside this instruction review.
Adrian reconfirmed this as the initial CALL target boundary on 2026-10-05;
the proposed callable uses the same presence-bearing Classic activation frame,
so argument omission, explicit empty values and optional results can retain
one contract across local and admitted external calls. The exact B/G signature,
provider lookup and VM check below remain a separate architectural decision.
The initial external CALL target set is these two programme types; the
earlier host-registered native-routine extension is not part of this first
stage. Host services used by a called Level C programme remain required by
`LC-75-04`.
When a reached CALL finds no local label, BIF or permitted provider, the
Classic routine-not-found identity is `43.1` at that clause, as described by
the [IBM routine-not-found reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=irx-irx0043i).
The simplified target set supplies no OS command fallback. This establishes
the diagnostic target for `LC-75-01/04`; its runtime implementation and
source proof are still open.
The external Classic search/load and shared host-state requirements in
`LC-75-02/04` remain open. Specify and review the Level B/G entry signature
and the same-context invocation API before implementing that boundary; the
proposal above is not yet an approved loader or VM contract.

**LC-STEP-75B module identity probe.** A guarded `rxlink` attempt with two
ordinary compiled Level C images stopped before execution with `conflicting
callable .main` in
`cmake-build-debug/levelc-external-link-probe.log`; both images export the
same generated main and body names. Existing bytecode autoload resolves an
unresolved callable from an exact packaged `.rxbin` stem, but cannot solve
duplicate exports or discover/compile `.rexx` source. An external Level C
provider therefore needs a distinct routine entry and unique generated
symbols, with a stated source/package naming rule and private activation
setup. This is design evidence, not external CALL acceptance.
The existing VM resolves ordinary callable imports during module linking;
compiling an unknown CALL as a mandatory direct import would fail before an
unreached clause could be skipped. The whole CALL error contract therefore
also needs a reached-clause resolver that can report absence at runtime.
The VM already has `metaloadmodule`, `metaloadedprocs` and `dcall` for
same-context runtime loading, procedure enumeration and pointer calls. These
may avoid a new VM opcode, but bare enumeration does not verify the fixed
Level B/G signature and ordinary providers still collide on `.main`.
The exact safe descriptor check, callable entry and search/ownership contract
remain the architectural decision gate; no new loader API is assumed here.
Guarded prototype evidence now narrows that gate. A Level B/G module exposing
`levelcextprobe.callentry` as `.void(frame=.RexxActivationArguments)` compiled,
assembled and linked beside an ordinary Level C image without a second
`.main` (`cmake-build-debug/levelc-bg-signature-compile-v2.log`,
`levelc-bg-signature-assemble-v2.log`, `levelc-bg-signature-link.log`). A
hand-isolated Level C body with a unique generated symbol and no `.main`
assembled and linked beside that image; executing the linked main retained
its expected output (`levelc-provider-shape-assemble.log`,
`levelc-provider-shape-link.log`, `levelc-provider-shape-run.log`). Finally,
a hand-authored RXAS probe used `metaloadmodule`, `metaloadedprocs` and
`dcall` to invoke the fixed-signature Level B/G entry during execution in
the same VM; its observable `callentry/entered/called` output is retained in
`levelc-bg-dynamic-call-run-v2.log`. These artifacts live only in the ignored
build directory. They prove VM primitive feasibility, not a compiler CALL
path, safe runtime signature validation, source discovery, or external
activation semantics. The production resolver must verify the descriptor
before calling a selected pointer.
An exposed-only enumeration variant also invoked that B/G entry and reported
its qualified key `levelcextprobe.callentry` in
`cmake-build-debug/levelc-bg-exposed-dynamic-run.log`. Compiling the same
provider with uppercase authored namespace/procedure spelling still emitted
the lowercase qualified metadata key
(`cmake-build-debug/levelc-bg-uppercase-compile.log`); the resolver must
distinguish Classic quoted target spelling for file search from the compiler's
normalized callable key.

**LC-STEP-75B compiler-only external CALL plan — active 2026-10-06.**
The intended outcome is one Classic CALL resolver that hands its existing
presence-bearing activation frame to either a local body, shared BIF or an
ordinary imported callable. The external ABI is an exposed Level B/G
`.void` procedure with exactly one by-value
`.RexxActivationArguments` argument. It may set an optional `RexxValue`
return on the frame. A separately compiled Level C routine supplies the same
callable ABI through a generated wrapper, with its own initial variable pool
and configuration, while the invocation body remains the approved one-frame
implementation. The compiler uses the existing signature validator and emits
the same import/call metadata as Level B/G; only the Level C error message and
lowering are specific. The ordinary assembler, linker and runtime remain
unchanged.

1. **75B-C1 (`LC-75-01/02/04`, complete):** inspect source and binary import
   discovery, typed callable resolution, Level C programme generation and
   existing result helpers. Prove with ignored-build probes that a generated
   import can bind an exposed compatible callable, that an incompatible
   signature is rejected by the compiler, and that an ordinary linked call
   returns its optional frame result. Record the exact naming and package
   rule before product edits; avoid a second dynamic resolver.
2. **75B-C2 (`LC-75-02/04/06`, active):** generate a unique exposed
   provider entry for a separately compiled Level C source without exporting
   another `.main`. Its wrapper constructs a private pool/default Classic
   configuration, passes the incoming frame into the one compiled body and
   preserves source metadata. Qualify linked and nested Level C providers,
   argument presence, Unicode/NUL and optional returns. This is compiler
   lowering only; any new source syntax or non-compiler contract needs a
   separate decision.
3. **75B-C3 (`LC-75-01/02/03/04/06`, active; depends on C1/C2):** resolve a nonlocal
   CALL to an ordinary exposed import, reuse the existing actual/frame and
   `applyCallResult` paths, and map the existing B/G signature error to a
   Level C diagnostic. Verify local-before-BIF-before-external order, quoted
   local bypass and case behavior, invalid signatures, source/canonical AST,
   opt/no-opt and unchanged ordinary linker/VM execution.
4. **75B-C4 (`LC-75-01/04/05`, active; depends on C3):** check static import failure
   against the retained reached-only missing-target and delayed-handler
   reference cases. Seek a narrow language-compatibility decision only if
   the approved unchanged linker/runtime path cannot meet those timing
   obligations through compiler lowering. Keep failed cases visibly open;
   do not convert them into an unapproved exception.
5. **75B-C5 (`LC-75-01–06`, open; depends on C2–C4):** finish the whole CALL
   reference matrix and delayed handler variants, run focused development
   checks and then one relevant normal Debug correctness suite on final
   code/test inputs. Retain ordinary linked delivery, Level B/G/RexxScript
   isolation and process-exit evidence; close the instruction only when each
   criterion is proved or explicitly revised by Adrian.

**75B compiler-only implementation checkpoint, 2026-10-06 (CALL still open).**
The generated Level C provider has a same-stem exposed entry and no extra
`main`; two separate Level C providers link together without helper-name
collisions. Direct external CALL reuses the local activation/argument builder
and result-presence writer. A fixed-signature Level B/G provider uses the
same ordinary import and link path. The permanent
`levelc_call_external_opt/noopt` cases compile all providers and consumer,
assemble and link them with unchanged `rxlink`, then run unchanged `rxvm`.
They cover nested Level C providers, source-ordered omitted and Unicode/NUL
actuals, private external pool, optional result presence/drop, local label
precedence, quoted bypass and fixed Level B/G entry. Negative compilation
checks reject wrong parameter type, return type, arity and reference mode
with `LEVELC_CALL_SIGNATURE`. The new AST marker is initialized and copied;
the first isolated macOS ASan run exposed its missing initialization as
spurious diagnostics, corrected before retained sanitizer success. The
guarded core build passed in
`cmake-build-debug/levelc-call-external-core-final.log`. Focused CALL and
source-import checks passed **27/27** in
`cmake-build-debug/levelc-call-external-focused-final.log`. The normal Debug
Level C suite passed **623/623** in
`cmake-build-debug/levelc-call-external-levelc-final.log`, peaking at
**4560.5 MiB** aggregate RSS and leaving zero child processes. Level B/G and
RexxScript isolation passed **10/10** in
`cmake-build-debug/levelc-call-external-isolation-final.log`. The focused
macOS ASan build and external CALL tests passed **2/2** through
`tools/asan-run.sh` in
`cmake-build-debug/levelc-call-external-asan-build-final-guard.log` and
`cmake-build-debug/levelc-call-external-asan-ctest-final-guard.log`, peaking
at **684.9 MiB** with zero residual processes. Apple LeakSanitizer is
unsupported; the ASan test run used `--leaks off`. These are exact current
code/test-input development receipts, not whole CALL or Level C closure.

**75B-C4 initial static-resolution finding (superseded in part by C4a).** The guarded unreachable
`IF 0 THEN CALL no_such_external` probe fails compilation with
`#NOT_A_FUNCTION` at line 2
(`cmake-build-debug/levelc-call-missing-external-static.log`), whereas the
retained Classic reference requires 43.1 only on a reached call. Routing an
unknown delayed handler through an ordinary import likewise moved the
existing 16.1 runtime error to a compile failure in the trial focused run;
the trial was removed and the missing-handler test passed again in
`cmake-build-debug/levelc-call-missing-recheck.log`. C4a below repairs the
compile-time-absent case through compiler lowering. A provider appearing only
after compilation and an available-but-unlinked provider still need whole
CALL review under the unchanged linker/runtime constraint.

**75B-C4a compiler-only missing-provider plan (active).** Reuse the
compiler's existing ordered source/binary import inventory to distinguish a
namespace absent at compile time from an available provider with an invalid
signature. For an absent direct target, lower to an authored-clause runtime
43.1 error only if reached; for an absent delayed handler, keep the current
authored raising-clause 16.1 route. An available provider continues through
the fixed typed import and ordinary signature checker. Prove unreachable and
reached direct calls, missing and available delayed handlers, opt/noopt and
linked execution without changing `rxlink` or the VM. Do not silently extend
this to a provider discovered after compilation: that search/lifecycle case
remains an explicit `LC-75-04` decision or proof obligation.

**75B-C4a compiler-only implementation, qualified as a bounded CALL increment.** The
CALL lowerer checks the compiler's existing ordered import inventory before
constructing an ordinary typed import. A source candidate is matched by its
declared namespace rather than a misleading filename; a binary candidate
uses its module stem. For a compile-time-absent direct target, the generated
clause evaluates actual expressions and raises source-anchored `43.1` only
when reached. A missing delayed handler retains the raising-clause `16.1`
path. A present external delayed handler uses the same activation-frame
builder with zero actuals, `CONDITION()` state and ignored handler return, so
the caller's prior RESULT remains. Explicit CLI imports remain under the
ordinary resolver because their exposed names may differ from module names.
Permanent opt/noopt linked fixtures cover reached and unreachable missing
CALLs, declared-namespace aliases, source and binary-only providers, and
external delayed-handler dispatch through a controlled event. No `rxlink` or
VM files changed. The guarded full Debug product build passed in
`cmake-build-debug/levelc-call-availability-core-final.log`; focused CALL and
source-import tests passed **43/43** in
`cmake-build-debug/levelc-call-availability-focused-final.log`, peaking at
**2771.8 MiB** and leaving zero child processes. The final-input normal Debug
Level C suite passed **625/625** in
`cmake-build-debug/levelc-call-availability-levelc-final.log`, peaking at
**5134.3 MiB** with zero residual processes. Level B/G and RexxScript
isolation passed **10/10** in
`cmake-build-debug/levelc-call-availability-isolation-final.log`. The
materially expanded external-CALL aggregate took **10.25 seconds** isolated
in normal Debug and also passed isolated macOS ASan; `RUN_SERIAL` and the
300-second backstop remain appropriate. The focused ASan build passed in
`cmake-build-debug/levelc-call-availability-asan-build2-guard.log`.
On final test inputs the expanded external aggregate passed isolated macOS
ASan optimized and no-opt runs in
`cmake-build-debug/levelc-call-external-binary-asan-isolated.log` and
`cmake-build-debug/levelc-call-external-noopt-asan-final.log`; the missing
target opt/noopt tests passed in
`cmake-build-debug/levelc-call-availability-asan-focused2-guard.log`.
These used `tools/asan-run.sh --leaks off` because Apple LSan is unsupported;
each guard reported zero residual processes. The expanded aggregate peaked
at **810.9 MiB** optimized and **811.1 MiB** no-opt under ASan.
The late-provider and available-but-unlinked timing cases,
external-handler `SIGL` reference interpretation, and real condition
producers remain open under `LC-75-04/05`; the controlled event does not
qualify ADDRESS or host event creation.
The [IBM CALL reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=instructions-call)
specifically places the incoming line in the caller's variable environment
for an *internal* routine. The ignored-build Regina probe
`cmake-build-debug/levelc_call_sigl_probe/` observed unassigned `SIGL` in a
direct external routine; its external `CALL ON` attempt instead failed with
`16.1`, so it cannot settle the external trap's private-pool `SIGL` rule.

**75B-C5a whole-CALL coherence plan (complete).** Keep the intended
outcome and numbered `LC-75-01–06` acceptance criteria above unchanged. This
review checks the external handler's caller/private-pool `SIGL` behavior
against the IBM CALL/condition wording, exercises both directions of
CALL/SIGNAL policy replacement, and audits the complete CALL
source/error/result matrix against retained reference probes. Record each
unmatched form. The compiler-only path retains the ordinary linker and VM.

1. **75B-C5a-1 (`LC-75-03/05`, complete):** add focused linked optimized/no-opt
   observations for external handler private state, caller `SIGL`, RESULT and
   CALL-after-SIGNAL replacement; check SIGNAL-after-CALL policy through its
   existing activation API, then compare the inference with the IBM reference.
2. **75B-C5a-2 (`LC-75-01/02/04`, boundary decision complete):** retain exact
   late-provider and available-but-unlinked receipts, finish the direct and
   delayed negative matrix, and record Adrian's static-boundary scope decision
   under the fixed compile-time signature and unchanged core behavior.
3. **75B-C5a-3 (`LC-75-06`, bounded qualification complete):** measure the expanded nested aggregate in normal Debug
   and maintained ASan, run focused checks during edits and one relevant normal
   correctness suite on final code/test inputs, then update architecture and
   coverage. Keep LC-I-13 and full Level C open for any unverified criterion.

**75B-C5a reference and runtime boundary receipt, 2026-10-06 (static decision recorded).** A
guarded compile/assemble/link/execute probe in
`cmake-build-debug/levelc_call_late_probe/` established three outcomes with
the unchanged core: a provider added only after caller compilation does not
replace the emitted 43.1 clause even when linked; a provider known at
compile time but omitted from the image passes `rxlink` and raises VM
`FUNCTION_NOT_FOUND` at the reached CALL; the same caller linked with its
provider returns `LATE`. The guarded probe peaked at **283.4 MiB** and left
zero children. A separate controlled external-handler image omitted its
provider; `rxlink` succeeded and `rxvm` raised `FUNCTION_NOT_FOUND` at the
authored `CALL ON` policy line rather than 16.1 at the event's NOP line
(`cmake-build-debug/compiler/tests/levelc_call_external/opt/`
`external-handler-unlinked-run.log`). Adrian accepted these precise
static-boundary departures above; the other `LC-75-01/04/05` obligations
remain open. This does not authorize a linker/runtime edit.

The permanent external-handler fixture now asserts a fresh zero-argument
frame, CONDITION `ERROR|CALL|DELAY`, ignored handler return, private-pool
`SIGL` unassigned, caller RESULT preserved and caller `SIGL` at the causing
line. It also proves CALL after SIGNAL selects CALL. A second linked source
with SIGNAL after CALL rejects a controlled delayed CALL queue; the existing
activation unit separately proves `setSignalPolicy` replaces an active CALL
slot and a nested child's policy change does not mutate its caller. Both
optimized and no-opt external aggregates passed focused Debug **2/2** in
`cmake-build-debug/levelc-call-c5a-override-focused2.log`, peaking at
**330.5 MiB** with zero residual processes.
The materially expanded aggregate passed macOS ASan optimized/no-opt
**2/2** in `cmake-build-debug/levelc-call-c5a-asan-focused-guard.log`,
taking **26.22/25.51 seconds** in isolation, peaking at **811.2 MiB** and
leaving zero children. `RUN_SERIAL` and the 300-second hang backstop remain
appropriate. The unchanged activation unit passed Debug **2/2** in
`cmake-build-debug/levelc-call-c5a-activation-focused.log`; its SIGNAL/CALL
replacement and nested-policy assertions were already present. The commit
`14fa28f8e` core build and Level B/G/RexxScript isolation remain valid because
only CALL test inputs and documentation changed after them.
The final-input normal Debug Level C suite passed **625/625** in
`cmake-build-debug/levelc-call-c5a-levelc-final.log`, taking **163.02
seconds**, peaking at **5713.3 MiB** and leaving zero residual processes.
The private-pool `SIGL` expectation is inferred from IBM's internal-routine
caller-environment wording plus Regina's direct-external observation;
Regina's external CALL ON
probe returns 16.1 instead of entering that handler, so exact external-trap
reference equivalence stays open. The CALL diagnostic, local/BIF/result,
four-condition, source-boundary and missing-handler matrices already have
permanent focused evidence above; no missing legal parser form was found in
this pass. Real event producers still belong to their instruction/host rows.

**LC-STEP-75E whole-CALL closure review (complete, 2026-10-06).** The intended
outcome is one complete CALL implementation under `LC-75-01–06`, including
the accepted static provider boundary, ordinary and delayed calls, source
errors, results and activation state. The numbered criteria above remain the
pass conditions; the ADDRESS, streams and host owners must still create their
own real condition events before full Level C can close.

1. **75E-1 (`LC-75-01–04`, complete):** reconcile every legal source form, diagnostic,
   resolver precedence, external signature and result path with the retained
   reference and permanent opt/no-opt linked tests. Record the static-boundary
   departures explicitly; identify any unverified form.
2. **75E-2 (`LC-75-05`, complete):** finish the delayed-handler lifecycle audit,
   including re-enable/OFF, nested isolation, repeated delivery, active-trap
   suppression/buffering, external private state and causing source identity.
   Use the smallest controlled event reproducer for any integration gap;
   preserve real producer obligations with their instruction/host owners.
3. **75E-3 (`LC-75-06`, complete; depends on 75E-1/2):** if product or test inputs
   change, run focused normal and maintained sanitizer checks, one normal
   Level C correctness suite and affected isolation on the final inputs.
   Reuse unchanged build/test evidence, update architecture/coverage and
   commit the coherent review. Close LC-I-13 only if all `LC-75-01–06`
   checks pass, then start LC-I-14 RETURN.

**75E whole-instruction evidence audit, 2026-10-06 (closed).**
The source forms and errors in `LC-75-01` have permanent direct, quoted,
constant-symbol, binary/hex-target, omission, malformed-expression,
surplus-policy-tail and reached/unreached missing-target tests. Local-label
precedence, BIF bypass, recursive/internal frames, PROCEDURE isolation,
source-ordered actuals, Unicode/NUL and linked external Level C and signed
Level B/G calls cover `LC-75-02`. The one result epilogue is exercised for
value, empty and absent returns, nested calls and unchanged `RC` under
`LC-75-03`. The external provider wrapper constructs its own pool and
`RexxClassicConfig`; its direct and nested linked tests, source/binary-only
discovery and signature rejection cover the static `LC-75-04` boundary.
The new `levelc_call_host_entry` test loads a separately compiled provider
and caller into one native `rxvml` context, checks the host byte-length SAY
callback and result under Unicode and embedded NUL arguments, and repeats
the entry without changing the linker or runtime.

For `LC-75-05`, retained controlled-event tests cover four conditions,
default/named local, BIF and external targets, ON/OFF and CALL/SIGNAL
replacement, re-enable/repeated delivery, nested policy restoration,
DELAY/CONDITION/ARG0, preserved RESULT, IF/WHEN/DO/transfer checkpoints,
and causing-clause 16.1 for an absent handler. A new compiled-body HALT
fixture raises a second event inside the first delayed handler; optimized and
no-opt output is exactly `caught=initial|3`, `inside`,
`caught=buffered|8`, `after`. The focused normal Debug test passed **2/2**
in `cmake-build-debug/levelc_call_whole_probe/debug-ctest.log` (2.92 seconds,
zero residual processes); maintained macOS ASan passed **2/2** in
`cmake-build-debugasan/asan-logs/20261006-090058-ctest/ctest.log` (5.34
seconds, leak detection off because Apple LSan is unavailable). The new
host test passed Debug **1/1** in
`cmake-build-debug/levelc_call_whole_probe/host-test-debug.log` and macOS
ASan **1/1** in
`cmake-build-debugasan/asan-logs/20261006-091011-ctest/ctest.log`.
The guarded Debug host target build completed in
`cmake-build-debug/levelc_call_whole_probe/host-build-debug.log`; the
maintained ASan target build completed in
`cmake-build-debugasan/asan-logs/20261006-090649-build/build.log`.
Both new test paths finished without residual compiler, assembler, linker,
VM or CTest processes. The nested aggregate is serialized with a 300-second
hang backstop; its isolated Debug and ASan durations above justify that
scheduling. ADDRESS, streams and host HALT must later create real events
under their own rows.

The external-handler private-pool `SIGL` interpretation follows the
[IBM CALL reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=instructions-call),
which assigns an internal routine's incoming line in its caller's variable
environment, and the [IBM condition reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=reference-conditions-condition-traps),
which assigns the causing clause at the current subroutine level. A new
external Classic programme has an implicit private pool; Regina's direct
external CALL observes its `SIGL` unassigned. Regina's external CALL ON
attempt returns 16.1, so this is a documented-rule inference rather than
byte-for-byte reference output for an external trap. The compiled external
handler test asserts unassigned private `SIGL` and caller `SIGL` at the
causing clause. No source form or live CALL-owned implementation gap was
found in this audit. The final-input normal Debug Level C suite passed
**628/628** in `cmake-build-debug/levelc_call_whole_probe/levelc-final.log`
in **171.12 seconds**, peaking at **5104.1 MiB** aggregate RSS and leaving
zero residual processes. The unchanged `14fa28f8e` core product build and
Level B/G/RexxScript isolation remain valid: this closure added tests and
documentation only, without changing product source or the earlier isolation
inputs. The current Debug host target build and focused sanitizer receipts
above qualify the new test/build inputs. `LC-75-01–06` and LC-I-13 close on
this combined evidence; full Level C and Release 1 remain open.

**LC-I-14 RETURN plan — vision and intended outcome, 2026-10-06.** Compile
the complete Classic RETURN instruction on the approved one-body invocation
model. A RETURN must finish the current internal subroutine or function,
evaluate an optional expression before private variables are released,
restore the caller's pool and settings, and deliver exactly the required
value/presence semantics. With no internal routine active, RETURN must have
the same programme-completion effect as EXIT. This review includes Unicode
and embedded-NUL scalar results and source-anchored errors through `rxc`,
`rxas`, `rxlink`, `rxvm` and the admitted native host entry. It does not close
the separate whole EXIT instruction, host-interface obligations or full
Level C/Release 1 criteria. The [IBM RETURN reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-return)
is the normative instruction contract. Preserve the existing linker and
runtime unless Adrian separately approves an architectural change.

1. **LC-76-01 — syntax and diagnostics (verified):** bare RETURN and RETURN with
   one Classic expression retain their authored AST, source position and
   contextual keyword behavior in main, called routines and nested IF/DO/
   SELECT arms. Invalid or surplus expressions report the applicable Classic
   error at the causing token or clause. Verify reference probes, source and
   canonical trees, optimized/no-opt diagnostics and recovery.
2. **LC-76-02 — called routine lifecycle (verified):** RETURN exits exactly the
   current invocation after its expression is evaluated once, restores its
   caller's pool, exposed aliases, condition policy and saved settings, and
   keeps the caller's source/loop state. Test shared and PROCEDURE-private
   pools, nested/recursive calls, fallthrough, branches and delayed traps in
   optimized/no-opt linked output.
3. **LC-76-03 — result and function rules (verified):** a returned expression,
   including empty text and embedded NUL, is present; bare subroutine RETURN
   drops `RESULT` and `.RESULT`, while an expression sets them without
   changing `RC`. A reached bare RETURN from a function raises Classic 45.1
   at RETURN; function falloff without a value raises 44.1 at its call site.
   Verify source-anchored failures, once-only evaluation, Unicode and the
   direct/linked/native host paths.
4. **LC-76-04 — top-level completion (verified):** with no active internal
   routine, bare and valued RETURN have EXIT-equivalent programme completion,
   including the defined result/status visible to the current VM and native
   host entry. They must not resume a stale local CALL or run later clauses.
   Establish exact reference and existing host-interface behavior before any
   product edit; keep full EXIT forms under LC-I-15.
5. **LC-76-05 — coherent qualification (verified):** use the shared activation
   result/pool path without a duplicate RETURN implementation, retain source
   docs, and run focused normal Debug and maintained sanitizer checks during
   edits. On final code/test inputs run the core product build, relevant
   normal Debug Level C suite, affected Level B/G/RexxScript isolation and
   opt/no-opt linked and host cases once. Record exact logs, exit and process
   evidence, update architecture/coverage, commit coherent increments, and
   close LC-I-14 only when LC-76-01–05 pass.

1. **LC-STEP-76A (`LC-76-01–04`, complete):** inventory parser/AST, validator,
   one-body lowerer, activation result/pool operations, main-wrapper return
   and host completion behavior; run guarded reference and minimal compiled
   probes for bare/valued, nested/function and top-level cases. Record the
   matrix and any genuine architectural decision gate before editing product
   code.
2. **LC-STEP-76B (`LC-76-01–04`; complete after 76A):** implement the complete
   RETURN contract through one frame-aware lowering/completion path, reusing
   existing EXIT-compatible completion where appropriate. Add permanent
   source/AST/diagnostic and optimized/no-opt linked/host regressions. Obtain
   Adrian's decision first only if 76A proves a new runtime/linker contract
   or language choice necessary.
3. **LC-STEP-76C (`LC-76-05`; complete after 76B):** qualify final inputs once,
   reconcile the whole RETURN matrix and docs, commit the coherent instruction
   result, then start LC-I-15 EXIT. Leave every unverified criterion open.

**LC-STEP-76A reference and implementation review, 2026-10-06.** The
[IBM RETURN instruction](https://www.ibm.com/docs/en/zos/3.1.0?topic=instructions-return)
requires `RETURN [expression]` in both main and internal routines. Without
an active internal invocation it completes the program like EXIT. In a
subroutine it evaluates the expression before discarding a PROCEDURE pool,
then sets `RESULT`, or drops `RESULT` when omitted; a function requires a
result. The [IBM EXIT instruction](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-exit)
specifies a text result, with command-interface conversion to an acceptable
integer status as a separate host operation. Guarded Regina probes in
`cmake-build-debug/levelc_return_reference/` confirm bare main RETURN stops
later clauses with status zero, `RETURN 7` exits with status seven, and a
called label returns to its caller before main `RETURN 9` exits with nine.
Regina's command entry returns zero for a nonnumeric main result. The same
directory retains Level C pre-change compile probes showing that main bare
and valued RETURN both fail as `unsupported main statement`.

The parser already creates one `RETURN` AST with an optional expression and
source anchor; the shared validator/lowerer restricts it to label bodies.
The compiled body is one frame per invocation and already records optional
results in `RexxActivationArguments`, with 45.1 at a reached bare function
RETURN and 44.1 on function falloff. The main wrapper calls that body but
currently returns void, losing an outermost valued result. `rxvml_run` and
`crexxsaa` expose an integer program status; the CLI main signature also
requires `.int` or `.void`. The agreed Classic/nonclassic interface boundary
leaves a general text result host API open under its own host criterion. This
instruction will retain the full Unicode scalar inside the activation and
convert it only for the existing integer main status. The implicit main
wrapper infers `.int` from its final status RETURN and retains its hidden
argv access; one activation status method keeps the linker and VM intact;
the body continues to use its single optional-result path. This is the
LC-STEP-76B implementation direction, not a change to the host API contract.

The malformed-tail review found that the original parser silently accepted
`RETURN 1, 2` and `RETURN 1 )` as `RETURN 1`; it also discarded a final
line-continuation comma. Guarded Regina probes reject each form. The grammar
now keeps a diagnostic child for surplus comma/closing parenthesis, and the
adapter reintroduces a final continuation comma at its authored position so
the same 35.1 path can reject it. A valid `RETURN 1,` followed by `+2`
remains a single expression and exits with status three. The guarded pre-fix
and post-fix logs are in `cmake-build-debug/levelc_return_reference/`.

The native-host probe also exposed an independent load-order obligation:
`rxvml_run` returns seven for the compiled main when `library`, `classlib`
and `rxfnsc` are explicitly loaded before that module, while the minimal
base-library load used by `crexxsaa_run_source` returned zero for the same
source. The explicit-load native entry is covered here; `crexxsaa` source
entry and general Classic result exchange remain open under LC-AC-06. No
linker, VM or `crexxsaa` product change is authorized by this RETURN review.

The host-status edge probe found a conversion panic for whole decimal text
(`3.0`) and for values beyond `.int`, because `DATATYPE(..., "W")` validates
Classic wholeness rather than machine range or cast syntax. Regina returned
status three for `3.0` and zero for an out-of-range whole value. The single
activation status method now converts with a 20-digit decimal context,
checks the signed 64-bit range, and returns zero when it cannot supply the
current host's integer status. Permanent optimized/no-opt cases cover decimal,
negative, maximum, overflow and enormous exponent values; the full
`RexxValue` remains available inside the activation before the host boundary.
Guarded exploratory logs are in `cmake-build-debug/levelc_return_reference/`.

**LC-STEP-76C whole-instruction closure, 2026-10-06.** The generated explicit
main header initially suppressed the compiler's implicit-main argv marker.
The first broad run found four existing ARG/CALL entry failures; all other
656 Level C tests passed. The repair keeps the generated main implicit and
lets the compiler infer `.int` from its final status RETURN, preserving hidden
argv access. Existing ARG/CALL host and main tests then passed 5/5 in
`cmake-build-debug/levelc_return_reference/arg-call-regression-focused.log`.
A permanent combined main-ARG-plus-`RETURN 7` test passes in both optimizer
modes. The raw/lowered/fixup tree test checks the main at the stage where the
compiler creates it. The final core Debug build passed in
`cmake-build-debug/levelc_return_reference/post-regression-fixture-build.log`.
The final normal Debug Level C suite passed **662/662** in 210.17 seconds at
`cmake-build-debug/levelc_return_reference/final-levelc-suite-2.log`, peaking
at 5696.0 MiB monitored group RSS and leaving zero residual processes.
Final-input Level B/G/RexxScript isolation passed **11/11** in
`cmake-build-debug/levelc_return_reference/final-isolation-2.log`.
The maintained macOS ASan targeted build passed at
`cmake-build-debugasan/asan-logs/20261006-103857-build/build.log`, followed
by focused **42/42** at
`cmake-build-debugasan/asan-logs/20261006-104349-ctest/ctest.log` with
Apple-unavailable leak detection off. No linker or VM product source changed.
`LC-76-01–05` and LC-I-14 close on this evidence. EXIT, PARSE, the remaining
instruction queue, host-interface obligations and full Level C/Release 1
qualification stay open.

**LC-I-15 EXIT plan — vision and intended outcome, 2026-10-06.** Complete
Classic EXIT as the unconditional termination of the current REXX program,
including when an internal subroutine, function, nested call, loop or
condition handler is active. Evaluate its optional expression once in the
active variable pool, preserve the full Unicode/embedded-NUL scalar as the
program result, and unwind to the program boundary without resuming an
interrupted caller or running later clauses. An external Level C program
called through the approved signed CALL boundary has its own program
boundary: its EXIT returns to its caller with optional result presence,
without ending the caller's program. Reuse the one-body activation model
and existing integer host-status bridge; retain the broader host text-result
API, real condition producers, full Level C and Release 1 as open criteria.
No linker or VM product change is authorized. The
[IBM EXIT reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-exit)
is the reference for explicit EXIT. Adrian chose Regina's caller-return
behavior for physical end-of-program on 2026-10-06, resolving its conflict
with the IBM wording and preserving the closed LC-I-13 CALL fallthrough case.

1. **LC-77-01 — source forms and errors (verified focused):** bare EXIT and EXIT with
   one expression retain authored AST/source anchors and contextual keyword
   behavior in main, local labels and nested IF/DO/SELECT arms. Missing or
   surplus expressions fail at the causing source token/clause. Verify
   reference probes, source/canonical trees, optimized/no-opt diagnostics
   and parser recovery.
2. **LC-77-02 — result and main completion (verified focused):** bare EXIT supplies no
   result, and valued EXIT supplies the exact once-evaluated Unicode scalar,
   including empty text and embedded NUL, before the active pool disappears.
   Main completion stops subsequent clauses, exposes the current VM/native
   host status through the existing bridge and does not alter prior RC or
   unrelated settings. Verify direct, linked and native entry cases.
3. **LC-77-03 — internal termination (verified focused):** EXIT inside any internal
   invocation terminates that program, unwinding every active internal
   CALL/function/handler frame without delivering an intermediate CALL
   RESULT, evaluating later actuals or expressions, or resuming the caller.
   Verify shared and PROCEDURE-private pools, nested/recursive calls,
   loops, branches, delayed traps and optimizer parity.
4. **LC-77-04 — external program boundary (verified focused):** an external Level C
   provider's EXIT completes that provider program and returns its optional
   scalar through the approved signed CALL subroutine boundary; the caller
   continues with its own pool and condition policy. The reference's deferred
   bare external-function error is recorded for the separately open external
   function-expression service. Keep linker/VM behavior and cross-dialect
   exclusions as already agreed.
5. **LC-77-05 — physical fallthrough (verified):**
   top-level end completes with no result; an active internal routine that
   runs off the physical end returns without a value to its caller. This is
   Adrian's explicit 2026-10-06 choice over IBM's whole-program wording.
   Verify main, subroutine, function and external-program cases and retain
   the earlier CALL fallthrough behavior.
6. **LC-77-06 — coherent qualification (verified 2026-10-06):** implement one program
   completion state on the shared activation/frame path, retain docs and
   source tags, and pass focused normal Debug checks during development.
   At the next grouped checkpoint, run the maintained focused sanitizer,
   the relevant normal Level C suite, Level B/G/RexxScript isolation and
   optimized/no-opt linked/native host cases on their final code/test inputs.
   Record exact logs, memory/process evidence and remaining host obligations.
   Commit the complete instruction implementation now; close LC-I-15 only
   when LC-77-01–06 are verified at that grouped checkpoint.

1. **LC-STEP-77A (`LC-77-01–05`, reference review complete):** inventory IBM and Regina
   syntax/result/EOF/error behavior; inspect parser/AST, current bare-main
   EXIT, local and external activation parentage, all CALL expression and
   handler boundaries, and native host completion. Retain guarded minimal
   reference and pre-change compiled probes. The EOF choice is recorded above.
2. **LC-STEP-77B (`LC-77-01–04`; complete):** carry one optional
   program result and termination request through the shared activation
   graph, with a distinct external-program boundary. Lower explicit EXIT
   through that path and insert required post-call/handler/function guards
   before any further expression or RESULT use. Add permanent whole-form,
   source-tree, error, direct/linked and host regressions.
3. **LC-STEP-77C (`LC-77-05`; complete):**
   implement and test the chosen physical-end behavior, reconcile the
   earlier CALL fallthrough receipt and docs, and preserve all remaining
   accepted CALL semantics.
4. **LC-STEP-77D (`LC-77-06`; complete 2026-10-06):** commit the focused-
   qualified whole EXIT implementation, proceed to LC-I-16 PULL, then
   qualify the grouped final code/test inputs once and reconcile the full
   EXIT matrix and open criteria before claiming LC-I-15 closure.

**LC-STEP-77A reference and implementation review, 2026-10-06.** Guarded
Regina probes in `cmake-build-debug/levelc_return_reference/exit_matrix/`
and `exit-reference-matrix.log` show bare/valued EXIT from main, a local
subroutine, nested PROCEDURE-private calls and a function expression stop
the program immediately; the interrupted SAY/function/caller never
continues. Bare internal-function EXIT ends the program without a 45.1.
An external provider's EXIT 7 returns the value to a calling subroutine or
function while the caller continues; bare external-function EXIT raises
44.1 at the caller, and bare external-subroutine EXIT drops RESULT. The
current Level C compiler accepts bare main EXIT but rejects valued main
EXIT and every local EXIT, as retained in `exit-prechange-matrix.log`.
Malformed EXIT tails behave like RETURN tails: Regina rejects comma and
stray close with parser 64.1, while an invalid expression gives 35.1.
The source grammar already emits one EXIT node with an optional expression;
its surplus-tail recovery needs the same anchored review as RETURN.

The approved activation already links each internal frame to its parent for
condition policy, and the provider path passes a fresh activation to a
separate compiled Level C routine. The EXIT implementation can share one
program-completion record by reference from the activation root to internal
frames, reset that root at an external signed CALL, and make every generated
local-call, function-expression and delayed-handler boundary return early
when that root requests EXIT. This carries the exact optional RexxValue
without changing linker or VM operations or duplicating source result
lowering. The physical EOF rule remains under LC-77-05: the
[IBM EXIT instruction](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-exit)
says running off the physical end is EXIT, while guarded Regina
`exit-falloff-reference.log` and the closed CALL fixture resume an internal
caller. Adrian chose the Regina caller-return behavior on 2026-10-06. The
existing terminal frame RETURN path implements that choice; permanent
whole-form and external-program fallthrough checks will qualify it.

**LC-STEP-77B explicit-EXIT development checkpoint, 2026-10-06.** The
activation now shares one program-root exit flag and optional `RexxValue`
result across internal frames. External signed Level C CALL resets a fresh
frame as the provider's program root. The shared lowerer accepts EXIT in
main and internal clauses, evaluates its expression once, records result
presence, and returns from the current body. Generated local CALL,
function-expression and delayed-handler boundaries check the root before
resuming work or consuming CALL results. The parser retains surplus comma
and close tokens as anchored 35.1 errors while valid comma continuation
still evaluates. Source-tree evidence retains the authored EXIT nodes and
their canonical completion calls. No linker or VM product source changed.

The guarded focused `levelc_exit_*` CTest matrix passed 14/14 in
`cmake-build-debug/levelc_return_reference/exit-ctest-focused-2.log`
(peak group RSS 734.3 MiB; no residual processes). It covers optimized and
unoptimized direct/linked main, recursive local, private pool, nested
expression/actual, loop, external provider, delayed CALL handler, parser
errors, source trees and continuation. The provider result test observes
Unicode plus embedded NUL, optional empty/bare values, unchanged caller RC,
and an expression evaluated once. The native `rxvml_run` entry's status 7
passed separately in `exit-host-test.log`. The maintained macOS ASan build
passed in `cmake-build-debugasan/asan-logs/20261006-111510-build/build.log`;
the focused EXIT matrix passed 14/14 with Apple-unavailable leak detection
off in `20261006-111852-ctest/ctest.log`. Its measured longest aggregate was
22.85 seconds in normal Debug and 34.29 seconds under ASan, so the nested
compiler/linker scenario tests run serially with a 300-second hang backstop.
These were development checks on the explicit-EXIT path, not LC-I-15 closure.
At this checkpoint LC-77-05, final-input normal suite and isolation evidence
remained open; the next receipt supersedes the LC-77-05 status.

**LC-STEP-77C Regina fallthrough decision and proof, 2026-10-06.** Adrian
selected caller return for physical EOF; IBM's contrary whole-program
wording is an explicit compatibility exception for this case only. The
existing one-body terminal RETURN already implements it, so no new product
code was required. The then-current focused EXIT matrix passed 14/14 in
`cmake-build-debug/levelc_return_reference/exit-eof-focused.log`, covering
top-level fallthrough, a called final label that resumes its caller and drops
RESULT, and an external Level C provider's final-label fallthrough that
resumes its own wrapper/caller. The existing closed CALL/fallthrough and
function 44.1 regressions passed 6/6 in `exit-eof-existing.log` in optimized
and no-opt modes. Both guarded runs left zero residual processes. This
verifies LC-77-05; LC-77-01–04/06 and LC-I-15 remain open until final
qualification. The 14-test macOS ASan receipt above predates the EOF test
additions and is not the final-input sanitizer receipt.

**LC-STEP-77D pre-grouped QA finding and disposition, 2026-10-06.** The
normal Debug Level C run at `exit-final-levelc-suite.log` completed 673/676
with peak group RSS 5126.2 MiB and zero residual processes. Two failures
were one actual EXIT interaction: delayed CALL conditions queued for the
EXIT clause were skipped because the new termination flag was recorded
before the clause checkpoint. The lowerer now dispatches the pending
condition after EXIT's expression and before recording program exit; a
handler's own EXIT can then supersede the interrupted EXIT. The exact two
CALL transfer tests passed 2/2 in `exit-trap-fix-tests.log`. The third
failure, `levelc_call_host_entry`, used a prebuilt provider/entry image that
had not been regenerated after compiler changes. Building its declared
target recompiled both images, and the test passed in
`exit-call-host-retested.log`; no CALL or host product edit was needed.
The complete focused EXIT matrix passed 14/14 after the product fix in
`exit-postfix-focused.log` (peak group RSS 543.8 MiB; no residual
processes). Under Adrian's grouped-gate direction, this is a coherent
implementation checkpoint, not a full-suite or sanitizer pass on final
inputs. LC-77-06 and LC-I-15 closure remain open for the next grouped
qualification; prior partial/before-fix broad evidence is not promoted
to a passing final verdict.

**LC-I-16 PULL plan — vision and intended outcome, 2026-10-06.** Compile
Classic PULL as `PARSE UPPER PULL` through the existing shared parse-template
engine. One reached PULL consumes exactly one string from the active external
data queue; when it is empty it reads one line from default input. Uppercase
the full valid-Unicode source by codepoint before parsing, preserve the
Latin-1 ordinal bridge, and assign arbitrary legal template targets in the
visible Classic pool. A bare PULL still consumes and discards its source.
Use the already shipped `rxfnsb` named-queue/default-input implementation
as the initial queue source, shared with subsequent PUSH/QUEUE reviews, and
keep the broader host-selectable queue/input callback obligation under
LC-AC-06 visibly open. Do not edit linker or VM product code. Review PULL
as a whole instruction, then continue to PUSH and QUEUE; schedule their
shared broad regression/sanitizer gate under Adrian's thicker-increment
direction. The [IBM PULL reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=parse-pull-instruction)
and [parsing summary](https://www.ibm.com/docs/en/zos/2.5.0?topic=parsing-instructions-summary)
are the instruction reference.

1. **LC-78-01 — source forms and errors (verified 2026-10-06):** bare PULL and every
   optional single/comma template form retain authored source/AST anchors,
   contextual keyword behavior and specific invalid-tail/template errors.
   Verify IBM/Regina probes, parser recovery, source/canonical trees and
   optimized/no-opt diagnostics.
2. **LC-78-02 — source acquisition (verified 2026-10-06):** each reached instruction reads
   one active-queue entry before falling back to one default-input line,
   including empty text, an empty queue, EOF and repeated calls. A bare
   instruction discards exactly that entry. Preserve current named-queue
   selection, program/host lifetime and ordered side effects. Verify queue
   and piped-input probes without a blocking terminal wait.
3. **LC-78-03 — parsing and pool effects (verified 2026-10-06):** PULL uppercases the one
   source with the approved Unicode contract, then runs the same arbitrary
   word/dot/pattern/position/comma template executor as ARG/PARSE, assigning
   into shared and PROCEDURE-private/exposed pools in source order. Later
   comma templates receive null sources. Verify direct, nested, recursive,
   optimizer-parity and linked cases; retain dynamic-position errors.
4. **LC-78-04 — host and queue boundary (verified for current host 2026-10-06):** use the existing queue
   service for default behavior, retain the broader configurable host
   queue/input callback contract in LC-AC-06, and identify any genuinely
   new architecture decision before implementation. Verify current native
   host and Level B/G isolation without claiming the open host API complete.
5. **LC-78-05 — coherent qualification (verified 2026-10-06):** implement one acquisition
   route and one parse executor, keep architecture docs and source tags in
   sync, pass focused normal Debug tests for the whole PULL matrix, and
   commit its coherent implementation. Run the normal Level C, focused
   maintained sanitizer and Level B/G/RexxScript isolation matrix once on
   the grouped PULL/PUSH/QUEUE final inputs, then close LC-I-16 only when
   LC-78-01–05 are verified. Keep full Level C and Release 1 open.

1. **LC-STEP-78A (`LC-78-01–04`; complete 2026-10-06):** inventory source forms,
   errors, queue/input/EOF behavior and host paths against IBM and Regina;
   inspect parser AST, the ARG/PARSE executor, `rxfnsb` queue and default
   input, and source/linked/native entry behavior. Retain guarded reference
   and pre-change probes; resolve any real architecture gap before editing.
2. **LC-STEP-78B (`LC-78-01–04`; complete 2026-10-06; depends on 78A):** lower complete PULL
   through the shared source and template path, add source/error/runtime
   and host regressions, and verify queue effects with the existing service.
3. **LC-STEP-78C (`LC-78-05`; complete 2026-10-06; depends on 78B):** run focused normal checks,
   record the tested inputs and commit the whole-instruction increment;
   then review PUSH and QUEUE in order before the grouped broad gate.
4. **LC-STEP-78D (`LC-78-05`; complete 2026-10-06):** qualify the shared final
   code/test inputs once, reconcile PULL and the queue-family evidence,
   and close LC-I-16 only when every criterion above passes.

**LC-I-16 implementation receipt, 2026-10-06.** The parser's `PULL` AST and
`TEMPLATES` list already had the full ARG/PARSE template grammar. The lowerer
now validates that list through the same checked template shape as ARG, obtains
exactly one source from `RexxClassicConfig.pullText()`, and runs the existing
`parseplan` executor with uppercase on the first template. Later comma
templates parse null sources; a bare or empty-first-template PULL still
consumes its source. `pullText()` delegates to the shipped `rxfnsb.pull()`
queue, whose empty-queue route calls default `linein()`; it adds a configuration
seam without a linker or VM product change. No separate PULL parser engine or
queue storage was added. The shared configuration remains a future host
selection point under LC-AC-06.

The retained reference probe at
`cmake-build-debug/levelc_pull_reference/reference.log` records Regina queue
FIFO, bare discard, pattern, piped-input uppercase and EOF behavior. Regina
rejects a comma PULL template with 64.1 in that probe, whereas the IBM
instruction reference permits a template list and assigns null to templates
after the first source. The compiler follows the already planned IBM grammar
and the shared ARG/PARSE template implementation. The pre-change compiler
reported `unsupported main statement` on PULL in `prechange.log`.

The final-input focused Debug CTests passed the linked optimized/no-opt input
matrix 2/2 in `focused-expanded.log`, invalid source 1/1 and reached invalid
dynamic position 2/2 in `focused-all.log`, and the source/canonical tree
inspection 1/1 in `source-tree-test.log`. The input matrix covers untaken IF,
bare discard, words, literal pattern, leading empty/comma-null templates,
absolute/dynamic positions, compound target, Latin-1 uppercase plus
supplementary-text preservation, private/exposed procedure targets and EOF.
Shared ARG invocation/source-tree checks passed 2/2 in
`shared-focused.log`; the existing named queue functional baseline passed
1/1 in `queue-baseline.log`. The guarded runs left zero child processes.
The current `strupper`/Classic TRANSLATE route supplies the already documented
simple uppercase map; its broader Unicode mapping choice remains in the
shared character/configuration contract and is not silently described as
full Unicode case mapping. The Level C instruction has not yet been tested
against queue entries produced by Level C PUSH/QUEUE, and the grouped normal,
maintained sanitizer and Level B/G/RexxScript isolation gate is open under
78D. LC-I-16, LC-AC-04/06/08/59 and Release 1 remain open.

**LC-I-17 PUSH plan — vision and intended outcome, 2026-10-06.** A reached
Classic `PUSH [expression]` evaluates its optional expression once, converts
the resulting valid-Unicode scalar to text without losing embedded NUL, and
inserts that value at the front of the selected external data queue. Omission
inserts one null string. The next PULL sees the new head; local calls share
the active queue, while untaken branches cause no insertion. Use the same
`RexxClassicConfig`/`rxfnsb` service path as PULL and the following QUEUE
review, preserving the host-configuration obligation in LC-AC-06. Keep the
linker and VM product unchanged. The [IBM PUSH reference](https://www.ibm.com/docs/en/SSLTBW_2.3.0/pdf/ikja300_v2r3.pdf)
and [data-stack ordering example](https://www.ibm.com/docs/en/zos/2.5.0?topic=stack-exercise-using-data)
govern the Classic forms and order; the approved Unicode-first scalar model
governs text representation.

1. **LC-79-01 — source forms and errors (verified 2026-10-06):** bare and expression PUSH
   preserve source/AST ownership, clause-context keyword behavior and Classic
   diagnostics for malformed expression tails. Verify IBM/Regina probes,
   source/canonical trees, valid continuation and optimized/no-opt compiler
   errors.
2. **LC-79-02 — evaluation and text (verified 2026-10-06):** a reached expression evaluates
   exactly once before insertion, preserving side effects, empty text,
   Unicode, Latin-1 mapped ordinals and embedded NUL; bare PUSH inserts an
   empty string. Verify optimized/no-opt runtime output and exact-length queue
   round trips without an arbitrary expression or value-size guard.
3. **LC-79-03 — queue effects and lifecycle (verified for selected queue 2026-10-06):** PUSH inserts at the
   front of the selected queue, preserving existing FIFO-tail entries and
   order across nested/recursive Classic calls, branches and named queue
   selection. Verify with PULL and the shipped named-queue service; leave
   unimplemented stack-buffer/host environment facilities under their owning
   rows.
4. **LC-79-04 — coherent implementation and qualification (verified 2026-10-06):** use one
   queue configuration route for PULL/PUSH and later QUEUE, with no duplicate
   storage or VM/linker change; pass focused normal Debug source/error/runtime,
   optimized/no-opt and linked tests, then commit. Run one grouped normal
   Level C, maintained sanitizer and Level B/G/RexxScript isolation gate on
   final PULL/PUSH/QUEUE inputs before closing LC-I-17. Keep LC-AC-06 host
   configurability and full Level C/Release 1 visibly open.

1. **LC-STEP-79A (`LC-79-01–03`; complete 2026-10-06):** inventory the IBM/Regina forms,
   parser/validator AST, current `rxfnsb.push()` front insertion, PULL
   interaction and source/error behavior; retain guarded pre-change probes.
2. **LC-STEP-79B (`LC-79-01–03`; complete 2026-10-06; depends on 79A):** lower all PUSH forms
   through the shared queue configuration service, add whole-instruction
   regression cases and review any repeated compiler-to-config setup with
   PULL for simplification.
3. **LC-STEP-79C (`LC-79-04`; complete 2026-10-06; depends on 79B):** run focused normal tests,
   record exact evidence, update architecture and commit the coherent PUSH
   increment; then review QUEUE as the next whole instruction.
4. **LC-STEP-79D (`LC-79-04`; complete 2026-10-06):** qualify final shared queue
   inputs in the grouped normal/sanitizer/isolation checkpoint, reconcile
   PULL/PUSH evidence, and close LC-I-17 only when its criteria pass.

**LC-I-17 implementation receipt, 2026-10-06.** Regina's guarded reference
probe (`cmake-build-debug/levelc_pull_reference/push-reference.log`) confirms
bare null insertion, LIFO order, one function evaluation, untaken IF and 35.1
for a bad expression. The pre-change compiler stopped at `unsupported main
statement` in `push-prechange.log`. Parser review found that a surplus comma
or close parenthesis after a valid PUSH expression was silently dropped. The
grammar now emits source-anchored 35.1 for those forms, following the same
clause-boundary rule as EXIT/RETURN; tests cover each form plus `PUSH +`.

The lowerer accepts one optional expression and calls
`RexxClassicConfig.pushText()` with the evaluated value's exact `.string`
representation, or `""` when omitted. That method delegates to shipped
`rxfnsb.push()`. A shared compiler helper now dereferences the configuration
once for PULL/PUSH, avoiding parallel setup paths. The linked optimized and
no-opt matrix passed 2/2 and compile errors passed 3/3 in `push-focused.log`;
source/canonical inspection passed 1/1 in `queue-trees.log`. It covers
LIFO, bare empty, embedded NUL through the Latin-1 ordinal bridge, Unicode
text, a called expression exactly once, SELECT and loop arms, and nested
recursive procedure sharing. PULL's focused suite passed 6/6 after the
shared helper/test-script refactor in `pull-after-push.log`, and its tree check
passed again in `queue-trees.log`. The guarded builds/tests report zero
residual child processes.

The default selected queue is still the existing `rxfnsb` execution-local
queue; Level C cannot yet choose its named queue through an admitted Classic
instruction. Queue selection and configurable host ownership remain open
under LC-AC-06 and the upcoming queue-family review. The grouped normal
Level C, maintained sanitizer and Level B/G/RexxScript isolation gate is
pending on final PULL/PUSH/QUEUE inputs. LC-I-17, LC-79-03/04 and full Level
C/Release 1 remain open.

**LC-I-18 QUEUE plan — vision and intended outcome, 2026-10-06.** A reached
Classic `QUEUE [expression]` evaluates its optional expression once and
appends its exact valid-Unicode text to the selected external data queue.
Omission appends one null string. PULL retrieves older head entries before
that tail, while PUSH can subsequently insert ahead of both. Preserve shared
queue visibility across admitted Classic calls and the same configuration
seam as PULL/PUSH. Keep the broader configurable host queue/input callback
under LC-AC-06 and leave linker/VM product code unchanged. The
[IBM QUEUE reference](https://www.ibm.com/docs/en/SSLTBW_2.3.0/pdf/ikja300_v2r3.pdf)
and [data-stack ordering example](https://www.ibm.com/docs/en/zos/2.5.0?topic=stack-exercise-using-data)
define the instruction order and optional-expression rule; Level C uses the
approved Unicode-first scalar boundary.

1. **LC-80-01 — source forms and errors (verified 2026-10-06):** bare and expression QUEUE
   retain source AST/anchors, valid clause contexts and Classic errors for
   malformed expressions or surplus tails. Verify IBM/Regina probes,
   source/canonical trees, continuation and opt/no-opt diagnostics.
2. **LC-80-02 — value and effects (verified 2026-10-06):** a reached expression evaluates
   exactly once before appending; bare QUEUE appends empty text. Preserve
   Unicode, NUL, mapped Latin-1 ordinals and source/evaluation order without
   an arbitrary expression or queue-size limit. Verify optimized/no-opt and
   linked cases with PULL.
3. **LC-80-03 — ordering and lifecycle (verified for selected queue 2026-10-06):** FIFO tail order composes
   with existing PUSH front order, PULL consumption, nested/recursive calls,
   branches/loops and named selection through the shipped queue repository.
   Verify default execution and the named-queue service, with broader host
   configuration retained under LC-AC-06.
4. **LC-80-04 — grouped closure (verified 2026-10-06):** use the one compiler-to-config route
   for PULL/PUSH/QUEUE and retain source/error/runtime tests. After focused
   Debug tests and a coherent commit, run one normal Level C, maintained
   sanitizer and Level B/G/RexxScript isolation checkpoint on exact final
   inputs; close LC-I-16–18 only when their own criteria pass. Keep full Level
   C and Release 1 open.

1. **LC-STEP-80A (`LC-80-01–03`; complete 2026-10-06):** inventory IBM/Regina forms,
   parser AST and validation, default and named queue service behavior, and
   the pre-change unsupported or silent-accept cases.
2. **LC-STEP-80B (`LC-80-01–03`; complete 2026-10-06; depends on 80A):** lower all QUEUE forms
   through the existing shared configuration helper, repair parser errors
   where needed, and add whole-instruction interactions with PUSH/PULL.
3. **LC-STEP-80C (`LC-80-04`; complete 2026-10-06; depends on 80B):** run focused normal Debug,
   source/error and linked opt/no-opt tests, record exact evidence and commit
   the coherent QUEUE increment.
4. **LC-STEP-80D (`LC-80-04`; complete 2026-10-06; depends on 80C):** run the
   single grouped normal/sanitizer/isolation qualification, reconcile the
   PULL/PUSH/QUEUE criteria and close the three instruction rows only on
   verified evidence.

**LC-I-18 implementation receipt, 2026-10-06.** Regina's guarded reference
probe (`cmake-build-debug/levelc_pull_reference/queue-reference.log`) confirms
bare null, FIFO tail, mixed PUSH/QUEUE ordering, one function evaluation,
untaken IF and invalid-expression behavior. Pre-change Level C reported
`unsupported main statement` in `queue-prechange.log`. The parser had the
same silent surplus-comma/close-bracket hole as PUSH; both now retain an
anchored 35.1. The lowerer validates one optional expression and shares the
PUSH compiler-to-config writer, choosing `RexxClassicConfig.queueText()`;
that method delegates to `rxfnsb.queue()` with an exact-length `.string`.

The first focused linked run exposed a shared `RexxQueue.push()` defect after
prior PULLs had left physical array slots behind the active count. A mixed
QUEUE/PUSH/QUEUE sequence inserted a blank before the final tail in both
optimized and no-opt Level C. `RexxQueue.push()` now shifts only `_count`
active entries; `ts_rxqueue.crexx` retains the reproducer. This is a shared
Level B library repair required by the QUEUE review, not a VM/linker change.
The repaired QUEUE focused suite passed 6/6 in `queue-after-fix.log`, including
three source errors, linked optimized/no-opt runtime, and source/canonical
inspection. Its runtime matrix covers null, NUL/Latin-1 bridge, Unicode,
side effects, selected/untaken branches, loops, recursive calls and FIFO
interaction with PUSH. Rebuilt Level B queue functional tests passed 2/2 in
`queue-bg.log`; PULL/PUSH focused tests passed 12/12 on the repaired library
in `pull-push-after-queue.log`. Guarded runs left zero residual children.

At this implementation checkpoint, the active default queue and its
named-queue repository were shared, but Level C had no admitted host selector
for a named queue. The host configuration obligation remains open under
LC-AC-06. The grouped gate below supersedes this checkpoint's pending QA
status; no full Level C or Release 1 criterion closes from these instruction
receipts.

**LC-I-15–18 grouped closure receipt, 2026-10-06.** The unchanged product,
test and build inputs at `09d48617cb50845356e0e6bda6abedb16ef125f0`
passed the normal Debug core build and 694/694 Level C CTests in
`cmake-build-debug/levelc_pull_reference/group-build.log` and
`group-levelc.log`. The monitored suite peaked at 4785.5 MiB group RSS and
left zero child processes. The separate Level B/G/RexxScript isolation set
passed 11/11 in `group-isolation.log`, peaking at 392.2 MiB and leaving zero
children. The maintained macOS ASan runner built the affected product,
libraries, queue functional test and EXIT host harness in
`cmake-build-debugasan/asan-logs/20261006-122252-build` and
`20261006-122737-build`; its focused EXIT/PULL/PUSH/QUEUE and shared
`ts_rxqueue` CTests passed 34/34 in `20261006-122748-ctest`. Apple leak
detection is unavailable, so both ASan phases used leak detection off. The
runner exited and no test processes remained. The four implementation
commits are `15287b235` (EXIT), `8f478c800` (PULL), `0d9f275f9` (PUSH)
and `09d48617c` (QUEUE and active-count repair).

PULL/PUSH/QUEUE invoke the existing `rxfnsb` functions through one
`RexxClassicConfig` route, so they operate on the repository's currently
selected queue. The Level B `ts_rxqueue` optimized/no-opt regression verifies
named selection and queue storage; the Level C linked matrix verifies the
same service on its default selected queue, including interleaved
PULL/PUSH/QUEUE, recursion, Unicode and NUL. A temporary direct Level C
`CALL RXQUEUE` probe compiled to the existing 43.1 missing-routine path:
no Classic selector has been admitted. Providing host or Level C control
of named selection and default input remains under open LC-AC-06, not an
implemented claim for these closed instruction rows. The C-string host
`rxvml_run()` NUL obligation and full Level C/Release 1 criteria also remain
open. Documentation-only closure here does not change the tested inputs.

**LC-I-19 PARSE plan — vision and intended outcome, 2026-10-06.** A compiled
Classic PARSE in the agreed Level C scope must obtain exactly the selected source, preserve its original
text unless UPPER is requested, and apply one reviewed template executor to
all legal target, dot, literal-pattern, dynamic-pattern, positional and
comma-segment forms. Source expressions and variable reads occur before
target writes; queue and input forms consume exactly one item even with an
empty template. Every form must work inside the one-body local frame and
signed Level C programme boundary with correct source errors and pool
lifetime. Reuse the existing `parseplan`, activation arguments,
`RexxClassicConfig`, `rxfnsb` input/metadata and `RexxValue` bridge; retain
Unicode codepoint positions, the Latin-1 ordinal bridge and RexxValue binary
capability. Keep linker and VM product code unchanged. Host stream selection,
general Classic/non-Classic entry and C-string embedded-NUL input remain
open under LC-AC-06. The [IBM PARSE instruction](https://www.ibm.com/docs/en/zos/3.1.0?topic=instructions-parse)
and [IBM source summary](https://www.ibm.com/docs/en/zvm/7.3.0?topic=parsing-instructions-summary)
are reference points, with guarded local Regina probes retained under
`cmake-build-debug/levelc_parse_reference/`.

**Scope decision, Adrian, 2026-10-06:** the initial Level C PARSE surface is
ARG, PULL, SOURCE, LINEIN, VERSION, VALUE and VAR. `PARSE EXTERNAL` and
`PARSE NUMERIC` are excluded for now as host-specific extensions; they must
continue to receive the existing invalid-PARSE-type error rather than being
aliased to another source. IBM documents both as non-SAA subkeywords;
Regina admits EXTERNAL but rejects NUMERIC with 25.12, so this is an explicit
Level C scope boundary, not a claim that no non-mainframe interpreter has
EXTERNAL. If either is added later, EXTERNAL needs its distinct terminal
input-buffer contract and NUMERIC needs current DIGITS/FUZZ/FORM state.
Regina's LOWER and CASELESS extensions are outside the IBM UPPER syntax
agreed for Level C; they are also excluded from this review.

1. **LC-81-01 — forms and diagnostics (closed 2026-10-06):** accept every agreed source,
   optional UPPER, bare/empty and arbitrary comma templates, and legal
   IF/DO/SELECT/local contexts. Keep contextual keyword ownership, authored
   source/canonical trees, parser recovery and Classic error identities for
   missing source keywords, VAR names, VALUE WITH, malformed tails and
   runtime dynamic positions. Verify reference probes and opt/no-opt errors.
2. **LC-81-02 — one template route (closed 2026-10-06):** validate and lower all PARSE
   sources through the shared ARG/PULL `parseplan` template executor, with no
   word-only, source-specific or target-count fallback. Preserve evaluation
   order, completed-capture dynamic operands, source snapshots, dot and
   compound-target writes, comma-null behavior, Unicode codepoint positions
   and optimizer parity. Verify structural and linked tests.
3. **LC-81-03 — ARG and pool sources (closed 2026-10-06):** PARSE ARG reads the current
   activation's ordered present/omitted arguments without uppercasing by
   default; PARSE VAR snapshots scalar, stem or compound values before writes
   and keeps NOVALUE policy; PARSE VALUE evaluates its optional expression
   exactly once before WITH targets. Verify main, local, recursive, exposed
   and external signed Level C calls with empty and NUL-bearing text.
4. **LC-81-04 — input sources (closed 2026-10-06):** PARSE PULL uses the selected queue
   then default input; PARSE LINEIN bypasses queued values
   and consumes the default-input line. Bare forms still
   consume; EOF yields empty text. Verify queue/input ordering, piped-input
   nonblocking cases and current host behavior, retaining the broader host
   selection contract under LC-AC-06.
5. **LC-81-05 — SOURCE and VERSION (closed 2026-10-06):** PARSE SOURCE reports the
   executing programme's system, programme-entry mode and source identity
   through existing metadata and the program-root activation, unchanged by
   internal calls; PARSE VERSION reports the active runtime version string.
   Verify direct, nested, linked and signed external Level C invocation,
   source provenance and opt/no-opt parity. Broader host metadata services
   retain their LC-AC-06 owner.
6. **LC-81-06 — extension decision (closed 2026-10-06):** Adrian excludes
   PARSE EXTERNAL and PARSE NUMERIC from initial Level C; retain invalid-type
   diagnostics for both and keep general numeric context under LC-I-22.
   Verify compiler diagnostics for the excluded spellings.
7. **LC-81-07 — coherent implementation and qualification (closed 2026-10-06):** remove
   duplicate PARSE lowering routes, retain source tags/docs and permanent
   whole-instruction tests, run focused normal Debug through all affected
   consumers during development and commit the coherent instruction
   implementation. Qualify the coherent checkpoint across the final-input
   normal Level C suite, focused maintained sanitizer and Level B/G/RexxScript
   isolation, reusing unchanged passing evidence after obsolete-test removal.
   Full Level C and Release 1 criteria remain open.

1. **LC-STEP-81A (`LC-81-01–06`; complete):** complete IBM/Regina source,
   template and error probes; inspect grammar, checked shape, metadata,
   activation and input services; retain the extension scope decision and
   identify only genuine architecture gaps.
2. **LC-STEP-81B (`LC-81-01–03`; complete):** replace the VAR/VALUE-only
   one-segment guard with one checked PARSE source/template plan and shared
   `parseplan` lowering, retaining source order and diagnostics.
3. **LC-STEP-81C (`LC-81-03–06`; complete):** add admitted ARG, input,
   SOURCE and VERSION acquisition through
   existing compiler/configuration/runtime services, with no linker/VM edit.
4. **LC-STEP-81D (`LC-81-01–07`; complete):** add whole-form/error,
   opt/no-opt, linked/native and source-tree regressions, run focused normal
   checks, update architecture and commit the coherent PARSE implementation.
5. **LC-STEP-81E (`LC-81-07`; complete):** run the
   grouped final-input normal/sanitizer/isolation checkpoint once, reconcile
   every LC-81 criterion and close LC-I-19 only on verified evidence.

**LC-STEP-81A review checkpoint, 2026-10-06.** Guarded Regina probes in
`cmake-build-debug/levelc_parse_reference/parse_sources.log`,
`parse_input.log`, `parse_source_mode.log`, `parse_var_shapes.log` and
`parse_arg_frames.log` show that non-ARG comma segments receive null,
bare VALUE WITH uses null, PULL consumes the selected queue, LINEIN and
EXTERNAL bypass it for successive piped input lines, UPPER applies before
templates, VAR accepts stem/compound sources and snapshots before overwrite,
and internal calls preserve the programme's SOURCE mode. The compiler's
pre-change `prechange_*.log` probes accept only one nonempty VAR/VALUE
template, reject ARG/PULL/SOURCE/LINEIN/VERSION and empty/comma VALUE at
lowering, and reject EXTERNAL/NUMERIC at the grammar's 25.12 boundary.
`RexxClassicConfig` already owns the PULL queue route; `rxfnsb` has LINEIN,
SOURCEINFO and VERSION facilities. SOURCEINFO currently reports COMMAND,
so the programme-root activation must supply SUBROUTINE mode for admitted
signed external CALL and retain FUNCTION mode for a later external function
entry. No linker or VM product edit is needed for the
reviewed source route. Complete error/reference inventory before
LC-STEP-81A is marked complete.

**LC-STEP-81B/81C in-progress evidence, 2026-10-06.** On existing grammar
sources, the lowerer now validates one arbitrary template list and routes
ARG, PULL, LINEIN, SOURCE, VERSION, VALUE and VAR through the shared
`parseplan` executor. ARG preserves case unless UPPER is written; other
sources capture once and apply null to later comma segments. The config
adapter reads default input separately from the selected queue, and source
metadata uses the current programme root's entry mode without a VM or linker
product change. Permanent `levelc_parse_whole_{opt,noopt}` and
`levelc_parse_source_external_{opt,noopt}` pass on the linked toolchain;
the external test proves COMMAND/SUBROUTINE modes and a source filename
containing spaces, plus omitted/Unicode-capable external PARSE ARG with an
embedded NUL. `levelc_parse_source_tree` proves authored PARSE nodes
lower to `parseplan`; 38 existing PARSE/template/diagnostic regressions pass
in `cmake-build-debug/levelc_parse_reference/legacy-regressions.log`.
After the final generated-name cleanup, the normal Debug `rxc rxfnsc` build
passed in `current-build.log`; the PARSE whole-form, signed external SOURCE,
source-tree and PULL/PUSH/QUEUE affected checks passed 11/11 in
`final-code-focused.log`. The earlier 38-template/error receipt is unchanged
by that generated-name cleanup. The augmented external ARG fixture passes
opt/no-opt in `external-arg-final.log`. Main, private, exposed and recursive
PARSE ARG frame cases pass in `whole-frames-final.log` alongside all admitted
source forms, opt/no-opt and source-tree checks.
The first directory-path assertion in the external fixture was corrected
because SOURCEINFO provides the source filename rather than that parent
directory; the corrected filename-with-spaces test passes. The complete
error/reference inventory, excluded-extension diagnostics, grouped normal and
maintained sanitizer/isolation gates, and whole-instruction closure were then
completed in the checkpoint below. FUNCTION SOURCE mode has frame support but lacks a currently admitted
external function-expression entry; retain that integration with the open
external function-expression work rather than claiming it qualified here.

**LC-STEP-81A/81D scope and normal checkpoint, 2026-10-06.** The
`syntaxhighlight_levelc_parse_excluded_sources` test passes the retained
25.12/25.13 diagnostics for EXTERNAL and NUMERIC with and without UPPER in
`cmake-build-debug/levelc_parse_reference/excluded-permanent.log`. The full
normal Debug Level C run in `levelc-normal-checkpoint.log` completed 700
cases: 698 passed; two historic tests that expected now-supported PARSE ARG
programmes to be rejected failed because compilation correctly succeeded.
Their obsolete registrations and source fixtures were removed. After CMake
regeneration, the current Level C list has 698 tests, exactly the 698 that
passed on the same code/test inputs in that retained run. Repeating the whole
suite would add no new coverage; the removed negatives are replaced by the
whole PARSE main/local/recursive/exposed linked fixture. No product source
changed during that test reconciliation.

**LC-I-19 closure checkpoint, 2026-10-06.** The final PARSE fixture adds
main/local/recursive/exposed ARG, external signed ARG with omitted, Unicode
and embedded-NUL data, queue versus direct input with bare consumption/EOF,
stem/compound VAR snapshots, VALUE/UPPER/comma cases, SOURCE/VERSION and
NOVALUE on a missing VAR source. Its last changed inputs pass opt/no-opt in
`cmake-build-debug/levelc_parse_reference/novalue-final-normal.log` and the
source-tree check passes in `source-tree-final.log`. The unchanged 695 other
current Level C tests passed in `levelc-normal-checkpoint.log`; that run also
passed the two PARSE cases before their final NOVALUE input addition. The
maintained macOS ASan `rxc rxfnsc` build passed at
`cmake-build-debugasan/asan-logs/20261006-135248-build`; focused PARSE,
ARG/PULL and diagnostics passed 10/10 at `20261006-135645-ctest`, and the
last changed whole-form fixture passed opt/no-opt at `20261006-135843-ctest`.
Apple LSan is unavailable; no Linux full sanitizer or release-ready claim is
made. Level B/G/RexxScript isolation passed 11/11 in
`cmake-build-debug/levelc_parse_reference/isolation-final.log`. The
instruction closes on that bounded evidence and Adrian's explicit extension
scope decision; full Level C, broader host input selection, external function
expression entry and Release 1 qualification remain open under their owners.

**LC-I-20 ADDRESS plan — vision and intended outcome, 2026-10-06.** A compiled
Classic ADDRESS must select, swap and query the active command environment in
the current invocation; execute explicit transient commands through the
configured environment protocol; and apply its legal input, output and error
connections with Classic RC, condition and source behavior. The same visible
pool state must serve ADDRESS() and persist or restore at the correct internal
and external call boundaries. Preserve exact Unicode text, including embedded
NUL, and the existing host encoding contract. Build on the current
`RexxAddressState`, `RexxClassicConfig`, `RexxActivationArguments` and
`_rxsysb.addressrequest`/`addressresponse` environment registry. Do not alter
the linker, VM, bytecode or loader without a separate approved decision.
Implicit expression-only command clauses remain under LC-I-21; ADDRESS must
leave a coherent selected environment and connection state for them. Broader
host stream selection and C-string `rxvml_run()` embedded-NUL input remain
open under LC-AC-06. The full Level C and Release 1 criteria remain open.

1. **LC-82-01 — complete forms and errors (closed 2026-10-06):** reconcile every Classic
   ADDRESS source form (bare, named, VALUE, command-bearing, WITH and legal
   combinations) against the parser and IBM/Regina references. Preserve
   contextual keyword/variable ownership, authored source AST, exact
   evaluation order, valid IF/DO/SELECT/local placement, source-anchored
   diagnostics and recovery. Verify a form/error matrix, source/canonical
   trees, optimized/no-opt compilation and reference probes.
   The prior compliance note's `ADDRESS VALUE expr command` spelling was
   inaccurate: IBM and Regina treat all following expression tokens as the
   lasting environment name. `ADDRESS env command` is the transient form.
2. **LC-82-02 — invocation-local selection (closed 2026-10-06):** active and alternate
   environment changes, bare swap, dynamic VALUE and transient command target
   follow Classic timing. ADDRESS() reports the selected state; internal,
   recursive and external signed calls inherit and restore the required
   environment/connection state without changing the Level B global default.
   Verify reference and linked nested-call cases and pool-state tests.
3. **LC-82-03 — one configured command path (closed with approved host exception 2026-10-06):** evaluate the explicit
   command once and submit its Unicode text to the selected registered
   environment via one request/response adapter, including built-in, unknown
   and native host environments. Set RC, .RC and .RS with the documented
   completion mapping; deliver ERROR/FAILURE to enabled SIGNAL/CALL policies
   at the causing clause while preserving ordinary non-trapped completion.
   Verify callback and linked tests, source identity, Unicode and opt/no-opt.
   The 2026-10-06 approved embedded-NUL host-command exception is tracked
   below under `LC-HOST-ADDRESS-NUL`; it does not change the Unicode scalar.
4. **LC-82-04 — complete WITH connections (closed 2026-10-06):** INPUT, OUTPUT and ERROR
   NORMAL/STREAM/STEM resources, legal APPEND/REPLACE variants, repeated and
   reordered clauses, persistent/default connection state and transient
   overrides follow the reference. Resolve stream/stem names and data at the
   prescribed time; preserve text, line boundaries and empty results; report
   missing resource and I/O failures with the correct Classic condition/error.
   Verify reference, file/stem/default and host callback cases, ADDRESS()
   connection queries, and linked opt/no-opt output.
5. **LC-82-05 — coherent lowering and isolation (closed 2026-10-06):** one validated ADDRESS
   shape reaches one compiler/configuration dispatch path, with no independent
   special-case command implementation. No Level C-only node survives
   lowering. Preserve Level B/G ADDRESS and RexxScript behavior; inspect
   source/canonical AST and generated RXAS, run focused cross-consumer tests,
   the relevant normal correctness suite on final inputs and the maintained
   focused sanitizer check when the grouped checkpoint is due.
6. **LC-82-06 — instruction closure (closed 2026-10-06):** document the implementation,
   supported environments, connections, diagnostics, host limits and retained
   evidence; commit one coherent whole-instruction increment. Close LC-I-20
   only after LC-82-01–05 pass with the approved `LC-HOST-ADDRESS-NUL`
   exception. Keep LC-I-21 and every full programme/release criterion visibly
   open.

Adrian selected the ANSI `WITH ... STREAM name` timing on 2026-10-06: evaluate
the resource variable at the ADDRESS clause and retain that filename in the
connection. Regina's later resolution is a known reference difference. The
focused ADDRESS review has also found two existing bridge limits. Adrian
approved an additive shared ADDRESS library `input_binary` factory on
2026-10-06, preserving an INPUT STREAM byte snapshot without the newline added
by `input_string`; its focused and normal proof is recorded below. Adrian also
approved the narrow `rxvml` native callback bridge
repair on 2026-10-06: copy its existing condition and diagnostic fields into
the existing response object without changing the callback ABI or linker. The
focused `levelc_address_host_callback` passes with explicit FAILURE and ERROR
policies. The grouped checkpoint and instruction closure are recorded below.

**LC-HOST-ADDRESS-NUL — open host-interface obligation, approved ADDRESS
instruction exception 2026-10-06.** The review reproduced a host command containing embedded NUL:
the Level C scalar retained the complete text, but the shared process channel
raised `CHANNEL_ERROR` at `_address.crexx:1698` instead of a Classic condition.
The native callback request exposes a C string without a length, so it cannot
report the text after NUL. Adrian directed that this gap be documented and the
ADDRESS instruction closed on its remaining complete contract after final
qualification. The exception does not claim correct NUL command dispatch, does
not exclude NUL from Level C scalar values, and remains open under `LC-AC-06`
until a separately approved host-interface remedy. No native callback ABI or
VM change has been made. An
empty or whitespace-only command in a built-in process environment reached
the same channel; Regina completes it successfully without a process. The
Level C adapter now skips only those built-in blank spawns, still handles their
redirections, and keeps unknown-environment RC 30 behavior.

1. **LC-STEP-82A (`LC-82-01–04`; completed 2026-10-06):** inventory grammar, diagnostics,
   source AST, state/condition helpers and the existing environment protocol;
   run guarded IBM/Regina/local host probes; record any genuine decision gate
   before product edits.
2. **LC-STEP-82B (`LC-82-01/02/05`; completed on focused evidence; depends on 82A):** validate one
   ADDRESS source description and lower environment/connection state operations
   through the invocation-local Classic state.
3. **LC-STEP-82C (`LC-82-03–05`; completed 2026-10-06; depends on 82B):** implement one
   configured command adapter and complete WITH resource/response handling,
   reusing the current environment object and native callback protocol.
4. **LC-STEP-82D (`LC-82-01–05`; completed 2026-10-06; depends on 82C):** retain the whole
   valid/error, nested, host, Unicode, opt/no-opt, source-tree and linked matrix;
   repair uncovered causes, with focused checks during development.
5. **LC-STEP-82E (`LC-82-05/06`; completed 2026-10-06; depends on 82D):** run one relevant
   final-input normal correctness checkpoint and grouped focused sanitizer and
   isolation checks as scheduled, reconcile every LC-82 criterion, update docs
   and commit the coherent instruction review.

The closed whole-instruction reference inventory is:

| ADDRESS family | Reference finding | Permanent local coverage |
|---|---|---|
| Bare, named, VALUE and computed environment; `VALUE` plus `WITH` | Bare swaps saved settings; each lasting selection saves the previous setting, including reselection of the same name; VALUE consumes its full expression | `levelc_address_whole`, source tree |
| Explicit command and blank command | The explicit command is transient and its expression runs once; Regina completes blank built-in commands with RC 0 and retains the selected environment, but an unknown environment yields RC 30 | `levelc_address_whole`, exact-input fixture |
| INPUT/OUTPUT/ERROR with NORMAL, STEM and STREAM | STEM counts and APPEND/REPLACE apply at the clause; invalid counts raise SYNTAX 54.1; missing streams under NOTREADY trap report the resource and causing line | `levelc_address_whole`, `levelc_address_notready`, `levelc_address_input_exact`, native callback |
| STREAM variable timing and invocation scope | Adrian selected the ANSI snapshot at ADDRESS; Regina's later name resolution is an accepted difference. Local recursion inherits the selected environment and connection into each new frame, while its changes remain local | `levelc_address_whole`, external linked fixture |
| Native condition and status | Callback FAILURE and ERROR must reach their CALL policies with causing-clause SIGL; RC/.RC/.RS, nonzero process status and external signed invocation state remain coherent | Native callback, exact-input, `levelc_address_external` |
| Exact text/bytes | The shared line-input factory appends a newline; the approved binary factory preserves input bytes. Unicode output and a host-produced NUL byte reach STREAM unchanged | Protocol, exact-input and native callback fixtures |
| Invalid source forms | Missing VALUE operand, bare WITH, missing or literal STREAM resource, malformed STEM, repeated direction, NORMAL extra operand, INPUT APPEND and unknown WITH keyword diagnose at source | Nine `levelc_address_invalid_*` tests |

The ADDRESS implementation checkpoint built `rxc`, `rxas`, `rxlink`,
`rxvm`, `rxfnsc` and the native callback fixture successfully
(`/tmp/crexx-address-checkpoint-build.PHqUkS`). One grouped final-input
focused Debug command passed 24/24 invalid, linked opt/no-opt, source-tree,
native callback and shared ADDRESS tests
(`/tmp/crexx-address-checkpoint-focused.JAw8tk`). It includes native ERROR
delivery, nonzero process status, local recursion and once-only command
evaluation. Commit `5aa20b306` contains those product and test inputs. The
normal Debug Level C run exercised 717 tests on that product input: 713 passed;
three syntax-highlighting fixtures still used forms rejected by the
[ANSI ADDRESS grammar](https://www.rexxla.org/rexxlang/standards/j18pub.pdf)
and Regina (`ADDRESS WITH` without an environment, and postfix `REPLACE`),
while the CALL host-entry fixture had stale generated bytecode. The three
source fixtures were corrected to legal forms, and the CALL fixture rebuilt.
Isolated replay passed all four unchanged product tests
(`/tmp/crexx-address-four-after-fixture.nBZxXH`). The 713 unaffected results
remain valid; this is combined qualification evidence, not a single 717/717
run. The grouped final-input sanitizer build passed at
`cmake-build-debugasan/asan-logs/20261006-163614-build`, and focused ADDRESS,
host and shared-consumer CTest passed 24/24 at
`cmake-build-debugasan/asan-logs/20261006-163633-ctest`. Apple LSan is
unavailable; no Linux full sanitizer or release-ready claim is made.
`LC-82-01–06` and `LC-I-20` close with Adrian's explicit
`LC-HOST-ADDRESS-NUL` exception. `LC-AC-01/04/06/08/58/59` and full Level C
and Release 1 qualification remain open.

**LC-I-21 implicit-command plan — vision and intended outcome, 2026-10-06.**
An expression-only Classic clause must evaluate its command text once, use the
currently selected invocation-local ADDRESS environment and persistent INPUT,
OUTPUT and ERROR connections, then expose the same command status and condition
behavior as an explicit ADDRESS command. It must remain an authored command
node through source validation and reuse the ADDRESS request/response adapter
after lowering; the Level B/G compiler-exit command path and RexxScript sandbox
retain their own behavior. The approved `LC-HOST-ADDRESS-NUL` exception applies
to this same host transport and remains open under `LC-AC-06`. Full Level C and
Release 1 criteria remain open until separately qualified.

1. **LC-83-01 — forms, ambiguity and errors (closed 2026-10-06):** inventory string-literal,
   variable, compound, concatenated and function-result command expressions,
   their parser tree and warning policy, malformed expressions, assignment
   precedence and nested IF/DO/SELECT/local placement. Verify reference probes,
   source diagnostics and optimized/no-opt compilation.
2. **LC-83-02 — selected-environment execution (closed 2026-10-06):** capture the expression
   once before the request, use the active frame's environment and lasting
   connections without changing either, and preserve selection across local,
   recursive and signed external calls. Verify opt/no-opt, linked and callback
   cases against ADDRESS state queries and output resources.
3. **LC-83-03 — completion and conditions (closed 2026-10-06):** reuse the explicit ADDRESS
   response path for RC/.RC/.RS, ERROR/FAILURE and NOTREADY, with causing-clause
   source identity, normal completion and nonzero status. Verify signal and
   delayed-call handlers, built-in/unknown/native environments and Unicode
   command text; keep `LC-HOST-ADDRESS-NUL` visibly open.
4. **LC-83-04 — one lowering path and closure (closed 2026-10-06):** no independent implicit
   host dispatch or surviving Level C command node; inspect raw/canonical AST
   and RXAS, run focused normal and shared-consumer checks, the relevant normal
   correctness suite at the grouped checkpoint, and focused maintained
   sanitizer evidence. Document the result and commit the coherent instruction
   increment before advancing to NUMERIC.

1. **LC-STEP-83A (`LC-83-01–03`; completed 2026-10-06):** review Classic reference and parser
   forms, existing exit fallback and ADDRESS adapter; retain counterexamples.
2. **LC-STEP-83B (`LC-83-01–04`; completed 2026-10-06; depends on 83A):** validate the source
   command node and lower it through the one ADDRESS adapter and frame state.
3. **LC-STEP-83C (`LC-83-01–03`; completed on focused evidence 2026-10-06; depends on 83B):** add full valid/error,
   nested, host, source-tree and linked opt/no-opt evidence with focused checks.
4. **LC-STEP-83D (`LC-83-04`; completed 2026-10-06; depends on 83C):** run the required grouped
   normal/sanitizer and cross-consumer checks on final inputs, reconcile every
   criterion, update docs and commit the whole-instruction result.

The [ANSI command rule](https://www.rexxla.org/rexxlang/standards/j18pub.pdf)
evaluates the expression before copying ACTIVE environment and connections,
then uses the same `CommandIssue` operation as an explicit ADDRESS command.
Regina probes confirmed a lasting SYSTEM output stem for literal and variable
commands, RC 30 for an unknown selected environment, and blank concatenation
for `name (expression)`. The parser now accepts a leading adjacent
`name(args)` as a function term and uses its existing adjacency check for the
spaced form. Authored `IMPLICIT_CMD` and its non-string source warning survive
source validation; errors remain fatal. Lowering captures the command text
once before reading the frame's selected environment, copies the active
connections into `RexxClassicAddressCommand`, and shares explicit ADDRESS
factory, run, SYNTAX and condition emission. No linker, VM, callback ABI or
Level B/G exit path changed.

The final-input normal Debug product build passed at
`/tmp/crexx-implicit-final-product-build.LbTaFj`. Focused Debug CTest passed
27/27 at `/tmp/crexx-implicit-final-focused.SM0gtL`: linked opt/no-opt
literal, variable, compound, concatenated, Unicode, function-result, spaced
parenthesis, IF/SELECT/DO, local/recursive and external command cases;
source-tree and invalid syntax; exact INPUT STREAM/STEM, native callback,
RC/.RC/.RS, ERROR/FAILURE/NOTREADY with source identity; and Level B/G shared
ADDRESS/exit isolation. The `LC-HOST-ADDRESS-NUL` exception applies to the
shared transport and remains open. The grouped normal Debug Level C checkpoint
passed 724/724 on these final code and test inputs
(`/tmp/crexx-implicit-levelc-normal.AiNKOs`), with zero compiler, assembler,
linker, VM or CTest child processes afterward. The maintained macOS ASan
build passed at `cmake-build-debugasan/asan-logs/20261006-171254-build`, and
the matched focused ASan CTest passed 27/27 at
`cmake-build-debugasan/asan-logs/20261006-171704-ctest`. Apple LSan is
unavailable, and overnight Linux/full-platform sanitizer assurance remains
separate. No first-party sanitizer finding appeared. `LC-83-01–04` and
LC-I-21 close. `LC-AC-01/04/06/08/58/59`, the approved
`LC-HOST-ADDRESS-NUL` host obligation, and full Level C and Release 1
qualification remain open.

**LC-I-22 NUMERIC plan — vision and intended outcome, 2026-10-06.**
Compile each Classic NUMERIC clause as a runtime change to the current
invocation's numeric context. Expressions observe the new setting at the
authored point; nested/local calls inherit it and cannot leak their later
changes back to callers. The existing Level B/G and RexxScript numeric APIs
retain their contracts. Reuse the current numeric RXAS operations and shared
`RexxValue` arithmetic; no linker or VM change is authorized. This is one
whole-instruction checkpoint, not a sequence of case-sized deliveries.

1. **LC-84-01 — forms and diagnostics (closed):** `DIGITS`/`FUZZ` with an
   optional expression and `FORM SCIENTIFIC|ENGINEERING|VALUE expression`
   have the standard source shapes, including nested instruction positions.
   Invalid subkeywords, missing expressions and extra operands retain Classic
   error identity and source location. Verify raw/canonical AST, Regina and
   focused compile cases.
2. **LC-84-02 — evaluation and validation (closed):** evaluate a dynamic
   operand exactly once, before changing the setting. Bare DIGITS/FUZZ reset
   to 9/0. Reject invalid whole-number values, DIGITS not greater than FUZZ,
   FUZZ not less than DIGITS, the documented digit limit, and FORM values
   whose first translated character is neither S nor E; keep the previous
   context on failure. Verify boundary/error and side-effect probes.
3. **LC-84-03 — invocation lifetime (closed):** the main invocation begins at
   9/0/SCIENTIFIC; internal, recursive and signed external Classic calls
   inherit the caller's setting and restore the caller on return or trapped
   transfer. PROCEDURE pool changes do not alter numeric inheritance. Verify
   nested opt/no-opt and linked calls and numeric BIF queries.
4. **LC-84-04 — numeric effects and isolation (closed):** arithmetic,
   comparison, display and numeric BIFs use the active Classic context,
   including precision, FUZZ and engineering/scientific notation. Operands
   that discard significant nonzero digits raise the source-identified
   LOSTDIGITS condition when its inherited SIGNAL policy is enabled; zeros
   discarded at the precision boundary do not. Preserve Level B/G and
   RexxScript behavior, the Unicode scalar contract and `RexxValue` binary
   capability. Verify reference results, condition delivery, shared-consumer
   regressions and generated RXAS.
5. **LC-84-05 — coherent closure (closed):** one validated numeric source shape
   and one compiled context route replace unsupported placeholder handling;
   focused regressions, the relevant normal Debug Level C suite once on the
   final code/test inputs, and linked toolchain execution pass. Retain only
   concise evidence and keep the full programme criteria open.

1. **LC-STEP-84A (`LC-84-01–04`; complete):** inventory ANSI/Regina forms,
   current parser/validator, numeric RXAS, `RexxValue` and call-frame paths;
   settle a compiler/library-only context design before product edits.
2. **LC-STEP-84B (`LC-84-01–03`; complete):** validate and lower the
   complete instruction into one current-invocation context path with
   source-anchored errors and call-frame lifetime.
3. **LC-STEP-84C (`LC-84-03–04`; complete):** make shared value and BIF
   operations observe that context without changing non-Classic consumers;
   test the whole valid/error and frame matrix in focused runs.
4. **LC-STEP-84D (`LC-84-01–05`; complete):** inspect compiler-to-RXAS
   flow, run one grouped normal checkpoint, update architecture/reference
   docs, record concise evidence and commit the coherent instruction.

**LC-I-22 closure receipt.** The raw AST retains `LEVELC_NUMERIC`; the
lowered AST uses one activation setter route and emits existing
`setnumdgts`/`setnumfuz`/`setnumfrm` RXAS. Regina comparisons cover defaults,
arithmetic scale and notation, validation, nested/recursive calls, FORM VALUE,
LOSTDIGITS and numeric BIF behavior. The accepted signed external Level C
boundary inherits its caller's context; Regina starts a separately loaded
external program at defaults. The final Debug build passed. Focused CTest
passed 40/40 on the final inputs, including Level B/G, RexxScript, shared
`RexxValue` and BIF isolation; the grouped normal Debug Level C suite passed
735/735 (`ctest --test-dir cmake-build-debug --parallel 30
--output-on-failure -L levelc`). All test/toolchain child processes exited.
No VM or linker change was made. `LC-AC-01/04/06/08/58/59`,
`LC-HOST-ADDRESS-NUL`, remaining SIGNAL/TRACE/INTERPRET and full Level C and
Release 1 qualification remain open. No sanitizer gate was added for this
ordinary development increment.

**LC-STEP-75B descriptor-safe selection proposal — superseded 2026-10-06.**
The following proposed VM operation is retained only as history; Adrian
directed the compiler to use ordinary typed imports and prohibited linker or
runtime changes without further approval. Reuse the existing `metaloadmodule`, exposed-procedure enumeration
and `dcall` operations. At a reached CALL clause, the generated resolver
searches the selected provider module's *exposed* entries for the exact target
name. Before `dcall`, a proposed `metacheckproc` operation takes that
VM-owned procedure pointer and an exact `rxsig1` descriptor, verifies that
the pointer belongs to a loaded module and that its declared name, return
type and one activation-frame argument match, and returns a boolean. It does
not load a module, invoke the procedure or change a frame. A missing name
and a present name with the wrong signature remain distinct resolver
outcomes; neither may enter `dcall`. This reuses the runtime's descriptor
parser and signature comparator, already used by
`rxvml_call_procedure_descriptor`, while keeping CALL's argument/result
owner in `RexxActivationArguments`. The exact exposed name, package search
and source-discovery policy still depend on Adrian's provider decision.
The compiled B/G probe emits `.meta` return `.void` and argument
`frame=.rexxactivationarguments`; for its exposed name the corresponding
descriptor is `rxsig1|levelcextprobe.callentry|.void|frame=.rexxactivationarguments`.
The check must establish pointer membership in a loaded module before reading
its runtime fields, then compare the actual exposed name and metadata
signature by return type, argument count, type and value/reference mode. The
source-level argument identifier is not part of the call ABI. The existing
host matcher permits some type compatibility; that permissive mode is not the
proposed fixed-signature CALL ABI. `RexxValue.asString()` is the text boundary
for a returned frame value; a B/G provider may retain binary `RexxValue`
internally, but a visible Level C result must pass valid UTF-8 conversion.

1. `75B-S1` (`LC-75-02/04`): prove the operation rejects an absent, wrong-
   type, wrong-arity, foreign or stale pointer without invoking it, and
   accepts an exposed `.void(frame=.RexxActivationArguments)` procedure;
   verify both VMs and optimized/no-opt RXAS behavior.
2. `75B-S2` (`LC-75-01–04`; depends on S1 and provider policy): resolve only
   when CALL executes, enumerate exposed entries from the selected
   same-context provider, check the descriptor, then invoke via existing
   `dcall`; map absence/mismatch to source-anchored Classic/host diagnostics.
3. `75B-S3` (`LC-75-02–06`; depends on S2): use the same checked selection for
   Level C generated wrappers and admitted Level B/G entries, then qualify
   arguments, optional result, nested calls, host state, traps and packaged
   execution before any whole-CALL verdict.

Adding `metacheckproc` is an architectural VM contract change, so no product
edit for it precedes Adrian's decision under `AGENTS.md`. The alternative is
a new configuration-owned native host service that calls the existing RXVML
descriptor API; it avoids an opcode but needs a larger public host ABI and
ordinary `rxvm` registration path. The guarded raw-pointer prototype is not
an acceptable final signature check.
An initial exact-stem bytecode provider stage should use that same resolver
and frame contract so later source discovery does not create a second CALL
implementation. The proposed fixed Level B/G entry is a `.void` procedure
with one `.RexxActivationArguments` argument; it reads source-ordered values
and presence through the frame and calls `setReturnValue` only when returning
a Classic value. This proposed signature and the provider load/search policy
await Adrian's decision before product edits.

**LC-STEP-75B external implementation order (historical proposal, superseded).**
1. `75B-1` (`LC-75-02/04`): fix the one-argument frame ABI, exact target-name
   encoding and a Level C routine build mode whose exported entry and helper
   symbols are unique and whose provider image does not export ordinary
   `.main`. Require the same ABI from an explicitly exposed Level B/G entry.
2. `75B-2` (`LC-75-01–04`): make the single CALL resolver select local, exact
   BIF, then the external entry only when the clause executes. Reuse the
   activation's captured arguments and optional return state, and preserve
   the ordinary CALL versus expression versus delayed-handler epilogues.
3. `75B-3` (`LC-75-02/04`): load and call a packaged provider in the current
   VM context from explicit linked modules or configured exact-stem roots;
   distinguish absent target from a target that raises a condition. Establish
   ownership, reentrancy, source and host-state behavior with opt/no-opt,
   linked and hosted regressions. Runtime source discovery remains an open
   `LC-75-04` stage unless Adrian selects it for the first implementation.
4. `75B-4` (`LC-75-01–06`): reconcile quoted/unquoted names, unreachable
   missing CALL, recursion, external private pools/default settings,
   Unicode/NUL argument transport, optional results, nested host state and
   trap targets against the reference matrix. Keep CALL open until every
   applicable criterion passes or Adrian explicitly revises it.

These steps specify review order, not an approved new loader, CLI or VM API.
The exact API and package/source policy are the pending architectural gate.

**LC-STEP-75D missing-handler source repair plan (LC-75-01/05; active).**
The controlled trap producer and Regina reference show that a missing delayed
handler must report `16.1` at the clause that raised the condition. The shared
dispatcher currently signals at its own generated source. Keep one dispatcher
and the existing activation frame: on an unresolved selected handler, record
the diagnostic detail in that activation and return. Each source-anchored
clause checkpoint consumes the detail and emits `CLASSIC_SYNTAX` there; a
recursive pending-event dispatch must return to the outer checkpoint before
the check. This changes neither the VM ISA nor the approved frame model.
Acceptance: the full local/BIF policy lifecycle, including ON, OFF, re-ON,
nested child-policy isolation, repeated events, DELAY, SIGL and RESULT,
passes opt/no-opt under controlled injection; a missing selected handler
reports `16.1` at the raising clause in both modes; focused normal and
maintained sanitizer checks plus the relevant final-input normal Level C and
Level B/G/RexxScript checks pass. First extend the controlled fixtures, then
add the activation diagnostic handoff and checkpoint signal, then qualify and
record exact evidence before committing. This is a coherent CALL trap
increment, not LC-I-13 closure or proof of real ADDRESS/I/O/HALT producers.

**LC-STEP-75D delayed trap design review, 2026-10-05.** The existing
`FRAME_HANDLER_ON`/`sigbrv` route transfers control immediately and cannot
resume the interrupted clause after a CALL handler returns. Keep that route
for SIGNAL. A condition producer that belongs to an implemented instruction
will record a typed pending event in the current Classic activation when the
selected policy is CALL ON; a generated checkpoint at the completed source
clause delivers it through the same local/BIF/external CALL resolver, but
without the ordinary RESULT epilogue. The handler receives a fresh zero-argument
activation and a temporary DELAY status; its caller's policy and prior
CONDITION data remain intact. ERROR, FAILURE and NOTREADY raised while their
handler is delayed are suppressed; the separate HALT buffering rule stays
explicit. CALL/SIGNAL ON or OFF for one condition replaces the previous mode.
The policy, pending event and delivery state live in `RexxActivationArguments`
and retain Unicode descriptions and the raising source line. This uses the
approved one-body invocation frame and existing CALL resolver. It does not
change the VM ISA. The current Level C compiler has no ADDRESS, stream or
host-interrupt producer; those instruction/host owners must connect to this
delivery path and are not silently treated as completed CALL evidence. The
[IBM condition reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=reference-conditions-condition-traps)
and [IBM CONDITION reference](https://www.ibm.com/docs/en/zvm/7.2.0?topic=control-condition-function)
specify DELAY, replacement, subroutine restoration and clause-boundary
delivery. The guarded Regina probes above establish the concrete observable
matrix for this implementation. HALT host capture may require a separate VM
boundary review when that producer is implemented; no such change is approved
by this CALL plan.

| CALL condition | Producer owner still open | CALL-owned proof now available |
| --- | --- | --- |
| ERROR, FAILURE | ADDRESS and implicit command completion through `Config_Command` | Controlled typed event, policy, handler and clause checkpoint only |
| NOTREADY | Stream and I/O operations, including PULL/PARSE input | Controlled typed event and trap state only |
| HALT | VM/host interrupt capture and buffering | Activation buffering unit and controlled clause delivery; real interrupt source remains open |

CALL and SIGNAL consume these conditions according to their active policy;
the producer rows must wire and qualify actual event creation. This keeps
whole-instruction CALL and full Level C completion distinct.

**LC-STEP-75D bounded implementation receipt, 2026-10-05.** `CALL ON/OFF`
now selects per-activation ERROR/FAILURE/HALT/NOTREADY CALL policies in the
same table as SIGNAL. A queued event is delivered by one generated dispatcher
called after completed source statements and before a RETURN transfer, with a
fresh zero-argument Classic handler frame and shared local/BIF resolver.
The called frame sees `CONDITION('C')`, Unicode `D`, `I=CALL`, `S=DELAY`,
causing `SIGL` and `ARG()=0`; handler RETURN data does not change the
interrupted caller's `RESULT`. A nested ERROR/FAILURE/NOTREADY in its own
delayed handler is suppressed; one extra HALT is buffered on the interrupted
caller through nested frames for replay after the handler returns. The
dispatcher is one helper function, so clauses contain a single call rather
than a copy of every handler branch. No VM ISA or frame architecture changed.

The permanent `levelc_call_delayed_injection.py` test compiles ordinary Level C
source, inserts only a controlled queue call after two authored CALL ON clauses
in RXAS, then assembles and runs the product-generated handler path. It is
explicitly a producer surrogate, not evidence that ADDRESS, I/O or OS HALT
already raise Classic conditions. The optimized/no-opt test covers local and
ARG BIF handlers, Unicode description, DELAY, SIGL, argument count, caller
CONDITION restoration and both `RESULT` and `.RESULT` preservation; the
activation unit also checks nested policy isolation, suppression and one
buffered HALT. The final-input focused Debug receipt is
`cmake-build-debug/levelc-call-delayed-checkpoint-focused.log` (6/6);
the final-input ASan receipt with macOS leak detection off is
`cmake-build-debug/levelc-call-delayed-final-asan-ctest-guard.log` (4/4).
The first macOS sanitizer build stopped on unsupported LeakSanitizer mode;
the guarded rerun built the same source successfully with leak detection off.
The guarded normal Debug Level C suite passed **608/608** in
`cmake-build-debug/levelc-call-delayed-checkpoint-levelc.log` (peak
**5040.1 MiB** aggregate RSS); affected Level B/G and RexxScript isolation
passed **10/10** in
`cmake-build-debug/levelc-call-delayed-checkpoint-isolation.log`. Every
guarded final-input build/test process exited with zero residual children.

**LC-STEP-75D four-condition matrix, 2026-10-06.** Regina accepts ON/OFF
forms for ERROR, FAILURE, HALT and NOTREADY in
`cmake-build-debug/levelc-call-conditions-regina.log`. The permanent
`levelc_call_condition_matrix.rexx` fixture and controlled RXAS producer
exercise each condition at an authored NOP checkpoint. Both optimization modes
prove the selected condition name and description, `I=CALL`, `S=DELAY`,
causing `SIGL`, a zero-argument handler, OFF lifecycle and preservation of
`RESULT`/`.RESULT`; the handler returns a value that the caller ignores.
Focused Debug passed **2/2** in
`cmake-build-debug/levelc-call-condition-focused-v2.log`; the ten existing
controlled CALL scenarios also passed in
`cmake-build-debug/levelc-call-condition-neighbors.log`. Focused maintained
ASan passed **2/2** with macOS leak detection off in
`cmake-build-debug/levelc-call-conditions-asan-ctest-guard.log`. These tests
use synthetic pending events and do not prove ADDRESS, stream, host HALT or
external-handler production. The final-input normal Debug Level C suite passed
**621/621** in `cmake-build-debug/levelc-call-condition-final-levelc.log`
(peak **5756.5 MiB** aggregate RSS). Every guarded command left zero residual
children. Whole CALL and full Level C remain open.

`LC-75-05` and whole CALL remain **open**: the unimplemented ADDRESS, stream
and host-interrupt producers must connect and qualify their own source events;
the complete clause/lifecycle matrix and source identity for a missing delayed
handler still need proof; external handler targets depend on
the external Classic service decision. The controlled surrogate is not a
substitute for those tests or the whole CALL reference and negative matrix.

**LC-STEP-75D clause-boundary implementation receipt, 2026-10-05.** The IF
test, each evaluated WHEN test, and each evaluated DO header now capture their
logical result before using the same generated CALL dispatcher; a handler
cannot change the branch decision already made. Counted/controlled DO setup
checks before the first body, empty repetitive bodies check each iteration,
and LEAVE/ITERATE/RETURN/EXIT check before transfer. The controlled RXAS
injection fixture places pending events after authored CALL ON clauses and
observes handler output before IF, WHEN, DO and transfer continuations in both
optimization modes. The guarded final-input focused Debug receipt is
`cmake-build-debug/levelc-call-boundary-all-focused.log` (**6/6**); the
maintained ASan receipt with macOS leak detection off is
`cmake-build-debug/levelc-call-boundary-asan-focused-guard.log` (**6/6**).
The guarded normal Debug Level C suite passed **612/612** in
`cmake-build-debug/levelc-call-boundary-final-levelc.log` (peak **4837.0 MiB**
aggregate RSS); affected Level B/G and RexxScript isolation passed **10/10**
in `cmake-build-debug/levelc-call-boundary-isolation.log`. Every guarded
build/test process exited with zero residual children. Branch-value mutation,
the remaining clause/lifecycle matrix, real producers, external handler targets
and exact missing-handler source identity remain open before whole CALL closure.

**LC-STEP-75D trap lifecycle and source receipt, 2026-10-05.** A controlled
four-event test now proves repeated ON delivery, OFF suppression, re-ON,
child CALL OFF isolation, parent-policy restoration, DELAY/CALL/ARG0/SIGL,
and preservation of the caller's RESULT in both optimization modes. On an
unresolved delayed handler, the shared dispatcher records the `16.1` detail
in the current activation; the completed authored clause checkpoint consumes
it once and emits `CLASSIC_SYNTAX` with that clause's source anchor. A
controlled event before `NOP` now reports
`levelc_call_missing_delayed_handler.rexx:3:1: nop`, matching the reference
causing-clause rule. This repair uses the existing frame and no new VM
operation. The activation unit proves one-time diagnostic consumption.
Guarded focused normal Debug passed **12/12** in
`cmake-build-debug/levelc-call-origin-all-focused.log` after rebuilding the
linked activation test artifact (its initial stale optimized image failed);
maintained ASan with macOS leak detection off passed **12/12** in
`cmake-build-debug/levelc-call-origin-asan-focused-guard.log`. The final-input
normal Debug Level C suite passed **617/617** in
`cmake-build-debug/levelc-call-origin-final-levelc.log` (peak **5145.1 MiB**
aggregate RSS); Level B/G and RexxScript isolation passed **10/10** in
`cmake-build-debug/levelc-call-origin-isolation.log`. The guarded core
product/runtime build passed in
`cmake-build-debug/levelc-call-origin-final-product-build.log`. All guarded runs had
zero residual child processes. Real ADDRESS/I/O/HALT producers, external
targets, a buffered nested event's runtime source origin, remaining
condition/reference forms and whole CALL remain open.

**LC-STEP-75C local/BIF/result foundation receipt, 2026-10-05.** The initial
guarded linked reproducer in `cmake-build-debug/levelc-call-result-repro-product.log`
showed `result=RESULT|RESULT` after an internal `RETURN 'done'`.
`RexxVariablePool.applyCallResult` now owns the ordinary CALL epilogue:
an explicitly returned value sets both `RESULT` and `.RESULT`, including an
empty value; value-less completion drops both. The local one-body call reads
the existing frame's presence/value after the callee returns. The compiler
resolves unquoted local labels before the direct BIF table, decodes quoted
targets without uppercasing, and lets direct CALL and expression functions
use one BIF argument/context/error builder. BIF CALL always supplies a value;
CALL leaves `RC` untouched. The same epilogue can accept the planned external
provider's optional result, while CALL ON handler returns deliberately skip
it. No VM ISA or frame architecture change was made.

The permanent Regina-matched `levelc_call_resolution_result.rexx` matrix
checks local/BIF collision, quoted bypass, source-ordered omitted/present
arguments, zero-argument ARG BIF, empty/value-less/ordinary returns,
`RESULT`/`.RESULT`, `RC` and nested calls. Its authored/canonical tree test
keeps the quoted source and omission shape, and checks the result operation
and direct BIF entry after lowering. Guarded Regina output is
`cmake-build-debug/levelc-call-resolution-result-regina-final.log`; the
new focused optimized/no-opt, linked and AST tests passed **4/4** in
`cmake-build-debug/levelc-call-final-focused.log`, and 22 existing CALL,
ARG, BIF and function-result tests passed in
`cmake-build-debug/levelc-call-local-bif-focused.log`. The guarded core
product build passed in `cmake-build-debug/levelc-call-resolution-final-build.log`.
The normal Debug Level C suite passed **606/606** in
`cmake-build-debug/levelc-call-normal-debug.log`, peaked at **5024.3 MiB**
aggregate RSS, and left zero child processes. Level B/G and RexxScript
isolation passed **10/10** in `cmake-build-debug/levelc-call-isolation.log`.
The post-fix linked reproducer passed in
`cmake-build-debug/levelc-call-result-repro-product-after.log`.

This is an implementation increment within LC-I-13, not its closure.
LC-75-01–06 remain open until external Classic dispatch, real delayed CALL
condition production and exact clause delivery, full error/reference cases
and host lifecycle pass. The external service
proposal above is awaiting Adrian's decision; no external product edit has
begun. RETURN, EXIT, ADDRESS, SIGNAL, PARSE, full Level C and Release 1
retain their own open criteria.

**LC-STEP-73H historical profile bridge — superseded 2026-10-04.** The
following proposal and `LC-73H-*` checks are retained as design history and
probe evidence. They are not active implementation steps after Adrian's
Unicode-first decision under LC-STEP-88A. Vision: a host
selects BYTE or UTF8 once per VM context, with BYTE as the default; each new
Level C activation samples that choice into its existing `RexxClassicConfig`.
ARG, PARSE UPPER and shared Classic BIFs then use one configuration object
through local calls. The source `OPTIONS` unknown-word behavior, Level B
programs and RexxScript's independent evaluator configuration keep their
current contracts. The preferred route is a context-owned profile setting
through `rxvml`, a standalone `rxvm --classic-profile` selector, and one VM
read-only register operation that returns the current profile to the generated
Level C main prefix. The compiler uses its existing canonical `ASSEMBLER`
node, then calls `RexxClassicConfig.setProfile`; no new AST node is needed.
The reserved VM operation slot 411 is available but its opcode/effect/signal/
feature contract must be updated together. The existing `parseplan` executor
also needs the selected unit rule: a read-only probe shows current default
BYTE Level C yields `éa|` for position 3 in `éa`, whereas Regina yields
`é|a`. Force Level C templates to version-2 descriptors and use the next
reserved policy bit to request profile-sensitive units. One parseplan source
view then chooses byte offsets for BYTE and codepoint offsets for UTF8,
including literal lengths, dynamic patterns and exact output spans. Unflagged
Level B plans retain their codepoint behavior and the same executor remains
the sole template engine. A second read-only probe at position 2 yields
`C3A9|61` in current Level C versus Regina's `C3|A961`, confirming BYTE must
preserve fields that split a multibyte UTF8 sequence. A source
`OPTIONS UTF8` word would change the already closed OPTIONS language contract;
a compiler-only flag would not let an embedding host choose per context.
Adrian's approval is required under AGENTS.md before this VM/host/compiler
architecture change.

**2026-10-04 BYTE/UTF8 architecture review — proposal paused for revision.**
The host selector and `parseplan` unit bit described above are insufficient as
an implementation plan. The agreed BYTE contract allows arbitrary octets, but
the current Level C lowerer copies every supplied BIF argument through
`RexxValue.asString()`, converts SAY and PARSE sources/external operands through
`asString()`, and stores PARSE fields in `.string[]`. The VM `parseplan` also
uses codepoint offsets and `.string` result spans in normal UTF8 builds. These
paths cannot carry a BYTE field split inside a UTF8 sequence, or a value such
as `X2C('FF')`, without implicit UTF8 validation. A focused current-build
probe of `C2X(X2C('FF'))` raises `UNICODE_ERROR` at the compiler-generated
BIF argument copy, despite the documented exact-byte C2X/X2C contract.
Pool assignment retains the `RexxValue` object and direct BYTE BIF helpers can
operate on `.binary`, so the defect is at the compiler/runtime and VM operand
boundaries rather than the basic binary storage. Before approving or executing
LC-STEP-73H.1–3, revise the architecture to define one byte-preserving path
for expression/BIF arguments, PARSE input and results, SAY/output, and host
ingress/egress; audit `RexxValue` operators that call `asString()`. Preserve
the Level B `.string` invariant and the distinct RexxScript evaluator
configuration. The decision gate remains open, and no profile-selection code
has been changed.

- [ ] **LC-73H-01:** default BYTE output and existing Level B/RexxScript
  behavior remain unchanged; invalid profile selectors fail before execution.
  Verify exact-output and error tests in standalone and embedded runs.
- [ ] **LC-73H-02:** separate host contexts and consecutive activations can
  select BYTE/UTF8 without leakage; local routines and BIFs see the entry
  profile, and a changed host setting applies to the next activation. Verify
  host callback tests for both profiles and repeated runs.
- [ ] **LC-73H-03:** ARG source uppercasing, static/dynamic template positions,
  literal and variable patterns, and adjacent PARSE/BIF consumers use the
  selected character units. BYTE fields preserve exact octets even when a
  position cuts through UTF8 bytes; UTF8 fields use codepoints. Verify
  profile-paired optimized/noopt, linked, binary and shared `rxfnsc` checks.
- [ ] **LC-73H-04:** the single VM query operation has assembler, disassembler,
  interpreter, opcode-effect/signal and feature coverage; version-1 and
  unflagged version-2 PARSE descriptors retain their behavior, with unknown
  flags rejected. The C API, CLI and Level C architecture/reference docs
  agree. Verify focused contract tests, normal Debug build and the relevant
  Level C suite on the exact code inputs.

1. **LC-STEP-73H.1 (LC-73H-01/02/04; approval required):** add the per-context
   BYTE/UTF8 setting and read-only VM query, expose checked `rxvml` and CLI
   selectors, and validate the operation through both VM variants.
2. **LC-STEP-73H.2 (LC-73H-02/03/04; depends on 73H.1):** set the generated
   main configuration once before source clauses, preserving the existing
   local reference handoff. Mark all Level C parse plans profile-sensitive and
   make one VM template executor select byte or codepoint units without
   changing Level B plans or duplicating ARG/PARSE/BIF paths.
3. **LC-STEP-73H.3 (LC-73H-01/02/03/04; depends on 73H.2):** compare the
   selected-profile cases with the Classic reference where its configuration
   is comparable, run the focused and relevant normal checks, update docs,
   and commit separate reviewable implementation increments.

Further read-only LC-STEP-73A reference evidence: Regina's main `ARG` given
one command argument string `blue green` assigns `BLUE`/`GREEN`; an internal
call with three positions including an omitted middle position gives
uppercased word fields, permits a literal delimiter, retains the argument
source for a second ARG, and reports existence correctly through the ARG BIF
(`/tmp/crexx-arg-reference-matrix.xnNmdw`). A second fixture confirms dynamic
pattern variables, leading/trailing empty comma slots, a bare ARG, and
repeated instruction semantics (`/tmp/crexx-arg-reference-patterns.Vy4j5j`).
The current VM's canonical Level B main `arg args = .string[]` accepts `-a`
items as an array, providing a direct source for generated Level C main code;
local CALL omissions still require explicit presence flags. These probes
strengthen the proposed frame/parse design but do not approve or implement it.

**Earlier whole-instruction checkpoint: LC-I-02 DROP.** The October review
identified duplicated compiler/runtime DROP selection. The approved shared
pool ownership supports a single `dropSymbol` route. `LC-STEP-64A` — complete
(`LC-AC-58/62`, depends on 63F) removes the compiler's per-kind method
selection and the single-tail-component guard for DROP, retaining parser
validation and ordered per-item evaluation. `LC-STEP-64B` — complete (`LC-AC-59/62`,
depends on 64A) checks the complete parser/reference matrix, invalid source,
Regina substitution and exposure, opt/no-opt, AST, pool/RexxScript, normal
correctness and linked delivery, then closes DROP only if all pass. Each step
gets a separate reviewable commit and evidence receipt. The indirect-list
invalid-word rule remains the previously approved Regina behavior.

2026-10-04 LC-STEP-64A DROP simplification: direct scalar, stem and compound
items now call `RexxVariablePool.dropSymbol` in source order; parenthesized
items retain one `dropIndirectList` call after reading their reference. The
compiler no longer chooses `drop`/`dropStem`/`dropStemTail` or rejects a
compound name with multiple tail components. A whole-DROP fixture compares
five direct names, three substituted tail components, direct/indirect drops,
item order, local exposure and stem removal with Regina
(`/tmp/crexx-drop-regina.fgswsJ`). Parser invalid forms (`DROP`, `DROP ()`,
`DROP (1)`) retain `20.1` source diagnostics. Thirteen focused Release CTest
cases passed on the changed code/test inputs
(`/tmp/crexx-drop-focused2.iVSjzP`); the tree proof sees `dropSymbol` and
`dropIndirectList`. Full instruction qualification under 64B remains open,
as do assignment and wider profile/host rules.

2026-10-04 LC-STEP-64B whole-DROP closure: the parser admits nonempty lists
of direct symbols and parenthesized references; bare DROP and malformed
references produce `20.1` at the offending source. The compiler validates
each source node once, then emits one ordered pool call per authored item.
Direct items use `dropSymbol`; indirect items read their reference at that
position and use `dropIndirectList`, whose invalid-word skip follows Adrian's
Regina decision. No fixed list or compound-tail-component limit remains.
The new whole-DROP fixture matches Regina for five-item lists, three
substituted tail components, direct/indirect drops, item order, exposed local
pool mutation and stem removal (`/tmp/crexx-drop-regina.fgswsJ`). Existing
direct/indirect fixtures retain tail case, tombstone/default, invalid-word,
numeric tail, repeated target and nested IF/DO proof. Thirteen focused
Release tests, including opt/no-opt, invalid diagnostics and canonical trees,
passed (`/tmp/crexx-drop-focused2.iVSjzP`); shared pool tests passed 2/2
(`/tmp/crexx-drop-pool-tests.myRkaO`) and rebuilt RexxScript runtime/compat
tests passed 4/4 (`/tmp/crexx-drop-rexxscript-tests.ZPenYW`). Current
`rxc` -> `rxas` -> `rxlink`
-> `rxvm` output matched Regina byte for byte (191 bytes,
`/tmp/crexx-drop-linked.CNlG4h`), and the normal Release Level C suite passed
312/312 (`/tmp/crexx-drop-levelc-suite.vd90UJ`). `LC-AC-62` and `LC-I-02`
close here. This checkpoint preceded the approved Unicode-first scalar
route; LC-STEP-88D-2 later requalified DROP on that route. Host text services
remain a shared `LC-AC-04/06` obligation, not a DROP-specific exception or a whole-Level-C
completion claim.

### Closed review: LC-I-01 SAY

`SAY` was the first whole-instruction review. The parser has an expression child or
no child (`compiler/rxcpcgmr.y`), plus recovery for an invalid close bracket.
The lowerer uses one canonical `SAY` builder; the qualified no-child
change supplies an empty string to that builder. The emitter uses the normal
`SAY` opcode, and the VM routes output through its SAY exit callback. No new
AST node or instruction-specific runtime helper is indicated by this review.

Adrian approved three architecture decisions on 2026-10-03: cREXX string
lengths must carry through the SAY output route; BIF validation failure should
raise a signal through one shared check, preserving Classic `SYNTAX` identity;
and general variable reads belong to `RexxVariablePool`. The 2026-10-03
decision to retain the terminated SAY callback was explicitly superseded on
2026-10-04 by `LC-STEP-63E`; the former assignment pre-RHS tail capture was
corrected under `LC-STEP-65A` after the whole-instruction Regina probe.

The original `LC-STEP-63` sequence and its later disposition are:

1. **LC-STEP-63A (LC-AC-57/58; complete):** route validated Level C variable
   reads through pool `symbolValue`; remove the compiler-only stem/compound
   read limit. Prove substitution, case, exposure, defaults, source anchors,
   opt/no-opt, linked execution and the normal Level C suite; commit.
2. **LC-STEP-63B (LC-AC-57; complete):** carry an explicit
   byte length from SAY operands to default and configured output. Its
   original legacy-callback compatibility proof is historical after
   `LC-STEP-63E`; current output tests cover embedded NUL, host isolation,
   UTF8, error and linked behavior.
3. **LC-STEP-63C (LC-AC-06; shared work in progress, depends on 63A):** use one direct-BIF
   selection table and one shared result/error check. Deliver BIF failures as
   a Classic `SYNTAX` signal distinct from command `ERROR`; preserve argument
   presence, caller pool/configuration, RexxScript isolation and source
   positions. Prove successful and failing BIFs, opt/no-opt and linked
   execution; commit.
4. **LC-STEP-63T (LC-AC-59; later SIGNAL architecture gate, depends on 63C):**
   make Classic conditions and labels part of one routine-activation control
   flow, so a shared BIF `SYNTAX` signal can branch to `SIGNAL ON SYNTAX` with
   correct label, pool, `SIGL` and return lifetimes. Do not implement this
   architectural shift before Adrian approves the design below. It belongs
   to the later SIGNAL/CALL condition review, not SAY closure.
5. **LC-STEP-63D (superseded by 63F for LC-AC-57):** its SAY-specific
   parser, output, diagnostics and host audit closed in 63F. Remaining full
   expression, BIF, TRACE and condition work retains its original Level C
   acceptance criteria and instruction rows.

The original LC-STEP-63C configuration direction created one hidden
`RexxClassicConfig` owner per Level C program activation, passed through
generated local calls and attached to direct BIF contexts. Its focused
RANDOM/pool-isolation proof is recorded below. Remaining host-selected
configuration and complete BIF behavior stay open under `LC-AC-06/04`.

| SAY contract area | Current evidence | Closure state |
| --- | --- | --- |
| Childless and ordinary expressions in main, IF/DO and local procedures | `levelc_say_instruction.rexx`; Regina, opt/no-opt, linked output and source-anchored tree checks | SAY forms closed; unsupported expression services remain under `LC-AC-04/06` |
| Expression evaluation once and output order | Exposed counter called inside SAY expression; its inner SAY precedes the outer line | SAY evaluation and ordering closed; external/BIF service coverage is cross-cutting |
| Invalid source forms | Seven highlighter fixtures and direct `rxc` negatives cover stray punctuation, incomplete expressions and recovery with `37.1`, `37.2`, `35.1` or `36` | SAY parser/error route closed; wider expression source rules remain under `LC-AC-04/08` |
| Function and variable expression terms | The SAY lowerer invokes the same `levelc_expr_supported`/`levelc_lower_expr` path as other instructions, then one `asString` and canonical SAY node; supported BIF/local/pool read cases match Regina, including error source | No SAY-only expression limit remains; missing BIF services, external resolution, profiles and traps remain under `LC-AC-04/06` and CALL/SIGNAL |
| Unicode text output, configured host route and failure | Length-aware default and per-context output pass embedded-NUL, Unicode text, context-isolation, opt/no-opt, both VMs and linked checks; the terminated callback was removed in LC-STEP-63E | SAY output route closed; remaining host services and codec qualification stay under `LC-AC-04/06` |
| Source and condition propagation | Canonical SAY opcode carries source and trace clause metadata; direct BIF failure aborts before the next SAY with authored site, and console output errors retain VM signal identity | SAY propagation closed; TRACE hooks and caught Classic conditions remain under TRACE/SIGNAL |

Historical SAY-adjacent findings and their remaining cross-cutting owners:

- Reconcile every expression form admitted by the Classic grammar with the
  shared expression/BIF path under `LC-AC-04/06/08`. The direct table reaches
  59 of 70 recognised Classic BIF names plus LOWER/UPPER; eleven recognised
  names still need direct runtime services. External functions, configured state and
  reference error/trap behavior remain open. The SAY instruction adds no
  separate restriction to a supported expression.
- **Resolved compiler-only variable-read limit (LC-STEP-63A):** Regina writes
  `Q.x.y` and `Q.` for `a='x'; b='y'; say q.a.b; say q.`, whereas the old
  compiler rejected both SAY operands
  (`/tmp/crexx-levelc-say-variable.0er3lh/`). All validated Level C variable
  *reads* now use `RexxVariablePool.symbolValue`, including bare stems and
  multi-component compounds. The former assignment pre-RHS tail capture was
  corrected in `LC-STEP-65A`. The new pool-read fixture covers
  substitution, case, unset/dropped values, exposure and a CALL argument.
- **Resolved blank-result continuation for direct BIFs (LC-STEP-63C in
  progress):** the earlier `SAY SUBSTR('abc', 0)` probe printed a blank line
  and continued (`/tmp/crexx-levelc-say-bif-error.AUp7Pt/`). Direct BIFs now
  pass through one checked result boundary, which raises VM
  `CLASSIC_SYNTAX` with the recorded `40.14` identity before the next SAY.
  This is distinct from the VM's existing `ERROR` signal. The generic
  argument frame retains omitted slots and the visible caller pool; the
  compiler keeps the authored BIF source anchor. The VM panic path now reports
  that authored site in main or local procedure code even when a shared helper
  raises the signal. Host-selected configuration, full Classic trap lifecycle
  and RexxScript equivalence beyond the focused shared tests remain open;
  these are required work, not infeasible
  exceptions.
- Prove remaining host text services, TRACE hooks and trapped-condition
  lifecycle under `LC-AC-04/06`, TRACE and SIGNAL. The SAY byte route and
  direct error propagation have focused proof; they do not claim those shared
  capabilities complete.
- **Resolved output truncation (LC-STEP-63B):** the earlier BYTE probe
  `options levelc; say '410042'x` produced `41 00 42 0a` with Regina but
  `41 0a` through the old VM output route
  (`/tmp/crexx-levelc-say-nul.sdYMGN/`). `SAY` and `SAYX` now carry explicit
  operand lengths to the default console writer or a new length-aware RXVML
  context callback. The former legacy callback was removed in LC-STEP-63E;
  the current callback receives the full span. The exact-byte fixture covers
  UTF-8 bytes and childless
  SAY. Optimized, no-opt, linked and host-callback evidence is recorded below.
- The childless Regina, opt/no-opt, raw/canonical tree, normal Level C and
  linked results are retained; LC-STEP-63F records exact current-head proof.

These cross-cutting items are open implementation/proof work, not infeasible
exceptions. SAY is closed; `LC-STEP-62B` now audits previously touched
instructions before SIGNAL.

2026-10-03 STEP-63 checkpoint (SAY remains open): the single
`levelc_say_instruction.rexx` fixture covers expression and childless forms
in main, IF/DO and local procedures, plus an exposed counter mutated by a
function called in a SAY expression. Regina output matches Release optimized,
no-opt and linked RXBIN byte for byte. The focused CTests passed 3/3, and the
normal Release Level C suite passed 290/290 in 27 seconds
(`/tmp/crexx-levelc-say-suite.Wt01LA`). The linked proof retained compile,
assembly, link and VM outputs under
`/tmp/crexx-levelc-say-instruction.vkHmqr`. The tree CTest also checks
source anchors for the main childless, nested, expression and local procedure
SAY forms; the production boundary verifier checks parent/sibling ownership
and residual Level C nodes. It does not prove every association or trace
lifecycle. The embedded-NUL and BIF-error defects, complete function/BIF
reachability, configured host output and failure lifecycle remain open.

2026-10-03 LC-STEP-63A increment (SAY remains open): the compiler now sends
every validated Level C variable read to `RexxVariablePool.symbolValue` and
removes its duplicate single-tail/stem read path. The obsolete bare-stem
negative case became a positive runtime fixture. The new
`levelc_say_pool_reads.rexx` fixture matches Regina byte for byte through
optimized, no-opt and linked `rxc`/`rxas`/`rxlink`/`rxvm` execution; its tree
check retains source anchors. Release core product build passed
(`/tmp/crexx-levelc-pool-build.3RQw1j`); focused SAY checks passed 6/6,
shared pool runtime checks passed 2/2, and the normal Release Level C suite
passed 293/293 (`/tmp/crexx-levelc-pool-suite-final.f6lPO8`). The first
suite run had four expected test-oracle failures from the removed rejection
and former `stemSymbolValue` tree assertions; all were updated to the new
approved path and passed. Linked proof is under
`/tmp/crexx-levelc-say-pool-linked.TNUFOd/`. No full SAY closure is claimed;
output and BIF/condition work proceed under LC-STEP-63B/C.

2026-10-03 LC-STEP-63B increment (SAY remains open): `SAY`/`SAYX` now use a
VM byte-span output function; the default writer preserves embedded NUL and
the RXVML context API provides a length-aware callback without changing the
legacy signature. An RXVML procedure call now reports an unhandled output
signal to its host separately from an ordinary program return status. The
`levelc_say_bytes.rexx` output is byte-identical to Regina in optimized,
no-opt and linked execution, including `41 00 42 0a`, UTF-8 bytes and an
empty SAY (`/tmp/crexx-levelc-say-bytes-linked.d3nVge`). The host fixture
checks distinct context callbacks, complete byte spans, legacy normal text
and `NOTREADY`/host failure without callback delivery on embedded NUL. The
focused checks passed 6/6, including existing SAY and concurrent context
isolation; Release core/host build passed
(`/tmp/crexx-levelc-say-bytes-status-build.F6pnY5`), and the normal Release
Level C suite passed 296/296
(`/tmp/crexx-levelc-say-bytes-suite.5sNM2F`). Complete BIF and SAY lifecycle
work remains under LC-STEP-63C/D.

2026-10-03 LC-STEP-63C checkpoint (step and SAY remain open): a single
compiler table selects 59 direct Classic runtime entries, covering 57 of the
70 recognized BIF names plus LOWER/UPPER. The generic argument frame now
serves every selected BIF and retains omitted positions and the current
activation pool. One shared `rexxclassicbif_checked` boundary raises VM
`CLASSIC_SYNTAX` (code 28), separate from VM `ERROR` (code 3), when a direct
BIF records a Classic error. The missing-required and invalid-start SUBSTR
fixtures stop with `40.3` and `40.14` respectively; output after the failed
call does not run. A 59-entry inventory compiles, assembles and runs its
reachable path, and the running
BIF-family fixture covers nested calls, omitted slots, pool mutation, a local
procedure and local-name precedence against Regina in optimized and no-opt
runs. Both successful and failing linked images pass, with the successful
output byte-identical to Regina
(`/tmp/crexx-levelc-bif-qualified-linked.wo9Jn9`). The canonical tree check
retains the authored SAY/BIF source anchors. Focused compiler/BIF/RexxScript
checks passed 12/12 (`/tmp/crexx-levelc-bif-final-focused.eXd5pD`) plus
RexxScript runtime 2/2; the Release core and `rxfnsc` build passed
(`/tmp/crexx-levelc-bif-final-build.5X96R1`), and the normal Release Level C
suite passed 303/303 (`/tmp/crexx-levelc-bif-qualified-suite.lSbDiB`).
Host-selected character/numeric configuration, thirteen absent Classic BIF
services, external function lookup and Classic traps remain open. No
whole-instruction closure is claimed.

2026-10-03 LC-STEP-63C configuration increment (step and SAY remain open):
compiler lowering creates one `RexxClassicConfig` per program activation,
passes its reference through generated internal function and CALL signatures,
and attaches it to every direct BIF frame. The permanent
`levelc_bif_config_lifecycle.rexx` test checks seeded RANDOM sequence
continuation through a local function, reseeding through a local CALL, and
separate local variable-pool mutation. It passes optimized and no-opt, while
the linked image produces the same six true results
(`/tmp/crexx-levelc-config-linked-final.GCCytB`). Focused Level C, shared RANDOM,
and RexxScript checks passed 14/14
(`/tmp/crexx-levelc-config-final-focused.IXpWxw`); the Release core/runtime build
passed (`/tmp/crexx-levelc-config-final-build.gR4SDr`) and the normal Release
Level C suite passed 305/305 (`/tmp/crexx-levelc-config-suite.8YNlzF`).
The [IBM REXX/VM RANDOM reference](https://www.ibm.com/support/pages/zvm/library/710pdfs/71631400.pdf)
requires a repeatable sequence and program-global generator state across
internal calls. It does not specify the individual generated numbers; the
current Park-Miller sequence differs from Regina for the same seed. This is
an implementation difference in the algorithm, not a claimed exception to
the state/range contract. Full RANDOM reference proof and host-selected
configuration remain open.

The LC-STEP-63C diagnostic plan was to make the VM's existing panic
source reporter run for signals with a nonempty message and show the immediate
call site from retained source metadata. It will prove that a failed Classic
BIF identifies the authored SAY line in main and local procedure contexts,
with opt/no-opt and linked execution. This is a general VM diagnostic repair;
the signal identity and shared BIF check stay unchanged. Classic catch/trap
state remains a separate, open lifecycle obligation.

2026-10-03 LC-STEP-63C diagnostic increment (step and SAY remain open): the VM
now prints panic source metadata for signals with or without payload text and
the immediate caller's location when a callee raises an unhandled signal.
Classic `SUBSTR` error fixtures report the authored `SAY` line in main and
local procedures under optimized and no-opt builds; the linked local fixture
preserves that location (`/tmp/crexx-levelc-bif-location-linked.S1nrHQ`).
The shared BIF helper and VM signal identity did not change. Focused Level C
checks passed 5/5 (`/tmp/crexx-levelc-bif-location-focused.MmyTsC`); existing
VM panic and signal checks passed 8/8 in rxbvm/rxtvm
(`/tmp/crexx-levelc-bif-location-vm.czY3Oe`), and the normal Release Level C
suite passed 308/308 (`/tmp/crexx-levelc-bif-location-suite.sQMgiq`). Classic
trap delivery and condition state remain open.

2026-10-03 SAY invalid-source audit: seven retained expression/highlighter
fixtures and the representative compiler-negative test pass 8/8
(`/tmp/crexx-levelc-say-invalid-audit.bZ8jqF`). Running all seven through
Release `rxc` rejects each source with the expected `37.1`, `37.2`, `35.1`
or `36` identity (`/tmp/crexx-levelc-say-compile-audit.vHgmyI`). The
highlighter tests assert the fault token and following clause are preserved;
their Regina mapping is recorded in the architecture document's expression
review. This covers the known invalid SAY expression grammar families. It
does not complete broader expression semantics or source/profile proof.

2026-10-04 LC-STEP-63E callback consolidation: the VM, RXVML and RXPA now
expose one `(const char *, size_t)` SAY callback signature. The compiler exit
bridge, active-context test and mainframe console mock use it. The former
terminated-text registration API, its NUL-rejection branch and the
legacy-only host test branch were removed. Public RXPA macro and interpreter
documentation reflect the native ABI change; external hosts using the old
signature must rebuild. A permanent Level G byte-output fixture joins the
Level C fixture to check `41 00 42 0a` and UTF-8 output under optimized and
no-opt compilation. Eight focused callback, console, context, RXPA, Level G
and Level C tests passed (`/tmp/crexx-say-byte-parity-tests.CbdcMM`); both
Level G and Level C RXBINs produced the expected bytes through `rxtvm`.
The normal Release Level C suite passed 308/308 on the changed code/test
inputs (`/tmp/crexx-say-callback-levelc.6ijx8N`). One initial console test
run lacked the separately built harness; after building it, the test passed
(`/tmp/crexx-say-host-tests.ytj53z`). This completes `LC-AC-60`; SAY closure
and the prior-instruction baseline remain separate steps.

2026-10-04 LC-STEP-63F whole-SAY closure: the parser's two valid SAY forms
(`SAY expression` and childless `SAY`) and invalid-close-bracket recovery
map to one canonical SAY node. The lowerer uses the common expression path
and one `asString` conversion or an empty string; there is no SAY-specific
expression whitelist or secondary output implementation. The canonical
emitter retains source/trace metadata and the VM outputs the operand's full
length through the one default/custom route. The existing instruction
fixture proves once-only side effects, line order and childless output in
main, IF/DO and local procedures; source-anchored tree tests, invalid-source
tests, BIF failure/source tests, pool reads, opt/no-opt and callback tests
remain registered. On this code/test input, the Release Level C suite passed
308/308 (`/tmp/crexx-say-callback-levelc.6ijx8N`) and the eight focused
callback/Level C/Level G tests passed
(`/tmp/crexx-say-byte-parity-tests.CbdcMM`). Current `rxc` -> `rxas` ->
`rxlink` -> `rxvm` output for the whole-instruction fixture matched Regina
byte for byte (67 bytes, `/tmp/crexx-say-close.8AYujv`); `rxtvm` independently
produced the expected NUL/UTF-8 bytes for both Level C and G. The direct
output/host error path and authored BIF failure source are covered by the
retained focused tests. `LC-AC-57` and `LC-I-01` close here. The full
expression/BIF/host-profile contract, Classic trap delivery and TRACE hooks
remain open in `LC-AC-04/06/08` and their instruction rows; none is an
approved exclusion or a whole-Level-C completion claim.

### Approved architecture direction for LC-STEP-63T

**Vision and intended outcome.** A compiled Classic invocation has one VM
frame containing all of its targetable labels and source-order fallthrough.
CALL enters that body in a fresh activation, while direct/VALUE SIGNAL and
condition traps transfer within or unwind to the correct existing activation.
PROCEDURE changes the visible pool only when executed; ARG and optional RETURN
state belong to that activation. Existing Level B/G, RexxScript and toolchain
behavior remain qualified. This is a cross-cutting prerequisite for closing
ARG, PROCEDURE and CALL; it does not narrow the full SIGNAL instruction or
Level C completion contract.

The active lowerer treats each top-level label as a separate generated Level B
procedure. It requires `PROCEDURE` immediately after the label and a final
`RETURN`, and the main slice must `EXIT` before those routines. That bounded
shape supports current direct CALL and function tests, but a Classic `SIGNAL`
branch must transfer within the *current invocation* to a label and discard
crossed loop/control state. The VM `sigbr` handler can unwind to an installing
frame and jump to a label in that frame; the current generated procedures have
no such shared label space. A per-label call/trampoline workaround would have
to emulate continuation, fallthrough and return state around the VM signal
model. This is significant architectural work, not an infeasible feature.
The Regina reference probes retained at
`/tmp/crexx-levelc-signal-reference.RmaAab` show that `SIGNAL ON SYNTAX`
catches `SUBSTR('abc',0)` as `40.14` with `SIGL` set to the causing clause;
the branch suppresses later source, labels can fall through without
`PROCEDURE`, a called label without `PROCEDURE` shares the caller pool, and a
trap installed outside an exposed nested procedure sees its pool mutation.

**Direction approved by Adrian on 2026-10-05:** lower one Classic routine
invocation to one canonical VM frame with labeled basic blocks. Internal CALL
enters that compiled routine at the requested label in a new invocation;
`PROCEDURE` changes the invocation's visible pool when executed, not the
existence of a label. `SIGNAL` and `SIGNAL ON/OFF` use the VM branch/handler
path with a mapping from Classic `SYNTAX` to VM `CLASSIC_SYNTAX`. Condition
state belongs to the routine activation; the program-wide
`RexxClassicConfig` remains configuration, and the variable pool remains
variable storage. Adrian accepts new AST node types for within-procedure
labels and SIGNAL targets when the reviewed representation is simpler and
reusable; carry them through validation and the emitter, rather than adding
an ad hoc label trampoline.
Retain the current passing BIF, pool, SAY, optimizer and linked tests while
replacing the bounded label model. This approval settles the ownership and
control-flow direction, not the exact node layout or a completed feature.

Before any code edit, check the proposed route against these observable gates:

- `LC-63T-01`: direct and trapped `SUBSTR` errors keep `40.14`, authored
  source, and correct prior output; a trapped `SYNTAX` branch suppresses the
  unhandled panic and reaches the named label once.
- `LC-63T-02`: labels without `PROCEDURE`, normal fallthrough, `CALL`,
  `RETURN`, `SIGNAL` and nested local calls retain Classic pool and control
  lifetimes; `SIGL` and condition identity refer to the causing clause.
- `LC-63T-03`: main/local, optimized/no-opt, AST ownership/source anchors,
  linked execution, existing Release Level C correctness and RexxScript
  isolation pass. Full `SIGNAL`/`CALL` instruction obligations remain open
  until their separate reference matrices close.
- `LC-63T-04`: frame-local label, transfer and handler operations have
  explicit canonical AST shape, validation, flow/optimizer behavior and RXAS
  emission. Direct and trapped transfers discard crossed loop/reference
  lifetimes without corrupting a later call or return. `RexxValue` remains a
  value type; activation state owns argument and return presence. The source
  AST separately identifies `SIGNAL VALUE expression` so decoding a quoted
  expression cannot turn it into a static label target.

1. **LC-STEP-63T-1 (LC-63T-01/02/04; review complete 2026-10-05):** inventory all parsed
   SIGNAL forms and Classic label, fallthrough, CALL, PROCEDURE, RETURN and
   trap lifetimes; compare Regina, preserve direct versus VALUE in the source
   AST, and fix the exact canonical node contract before product edits.
2. **LC-STEP-63T-2 (LC-63T-02/04; in progress; depends on 63T-1):** add frame-local label,
   branch and handler nodes through AST validation, flow/optimizer and RXAS
   emission, with source anchors and structural tests. Reuse VM signal
   instructions and existing scoped cleanup machinery where their contracts
   match; no per-label trampoline or second token interpreter.
3. **LC-STEP-63T-3 (LC-63T-02/03; one-body implementation complete 2026-10-05; depends on 63T-2):** refactor local CALL,
   function entry, ARG, PROCEDURE and RETURN onto one compiled body per
   invocation with activation-owned pool and optional result state. Preserve
   once-only argument evaluation and recursive isolation; remove the old
   immediate-PROCEDURE/final-RETURN/per-label guards when the replacement runs.
   The implementation checkpoints are `63T-3A`, one generated callable body
   plus main wrapper and label-entry dispatch; `63T-3B`, source-position
   PROCEDURE pool transition and exposure with labels that share the parent
   pool; `63T-3C`, fresh CALL/function argument frames and optional RETURN
   state; `63T-3E`, activation entry eligibility and runtime PROCEDURE 17.1
   for main fallthrough or a second PROCEDURE in one invocation (while
   retaining the source placement check pending whole-PROCEDURE review);
   `63T-3E2`, correct eligibility to the documented first processed
   instruction rule, including nonfresh label fallthrough, and replace the
   permissive Regina-only success oracle; `63T-3G`, emit runtime 17.1 at the
   authored PROCEDURE clause and retain the activation's direct-call guard; and
   `63T-3D`, full main/local/nested/recursive, opt/no-opt, source, linked and
   normal-suite qualification before retiring the old partition; and
   `63T-3H`, runtime function-result presence and missing-result errors
   discovered by the frame matrix. Remove the old per-label static
   value-RETURN admission test so a function call can enter any local
   label, execute ARG and branch normally, then diagnose a reached bare
   RETURN or missing result with Classic identity and source. Keep
   subroutine RETURN and explicit empty-string results distinct, including
   nested/recursive calls. The existing optional-result activation state
   owns this distinction; complete CALL/RETURN reviews remain separate.
   Each checkpoint serves `LC-63T-02/03`; none alone closes ARG or SIGNAL.
4. **LC-STEP-63T-4 — complete (LC-63T-01/02/04/LC-AC-76; depends on 63T-3):** implement direct and
   VALUE SIGNAL, ON/OFF and named condition targets through the VM's frame
   branch/handler path; map Classic condition identities, SIGL and source,
   and discard crossed loop/reference state. Reviewable implementation
   checkpoints: `63T-4A` preserves VALUE intent explicitly in the source
   AST, validation and tree display; `63T-4B` lowers direct static branches
   onto FRAME_BRANCH with runtime missing-label 16.1, SIGL and crossed-scope
   cleanup; `63T-4C` evaluates VALUE once and dispatches through the same
   frame branch path with the approved Unicode-first scalar and missing-label
   behavior; `63T-4D` installs ON/OFF and named/default condition handlers,
   including RC/SIGL, one-shot delivery, nested policy and handler cleanup.
   The handler implementation review splits this into `63T-4D1`, a canonical
   event-binding node/emitter route; `63T-4D2`, source-order ON/OFF lowering
   with handler-entry trampolines, RC/SIGL and one-shot behavior; and
   `63T-4D3`, the full parsed condition-name and nested activation policy
   matrix. Within 4D3, first establish a distinct Classic non-SYNTAX event
   transport with a typed condition payload (`4D3-1`), then dispatch its
   seven-name activation policy and one-shot handler selection through the
   same frame path (`4D3-2`), then qualify available condition producers and
   activation-local `CONDITION()` state, and explicitly assign missing
   host/numeric producers to their instruction or host rows (`4D3-3`).
   The `4D3-3` reference matrix checks `CONDITION` C/D/E/I/S and omission,
   source-order policy changes, nested save/restore, and invalid options
   before a product edit. These were checkpoints of the then-open SIGNAL instruction.
   Each checkpoint retains focused evidence and is not SIGNAL closure.
5. **LC-STEP-63T-5 — complete (LC-63T-01–04/LC-AC-76; depends on 63T-4):** qualify the complete
   SIGNAL reference matrix, opt/no-opt, raw/canonical AST, linked toolchain,
   relevant normal correctness and Level B/G/RexxScript isolation. Update
   architecture/reference docs, commit coherent checkpoints and close the
   instruction only if its own criteria pass. Full CALL and ARG obligations
   retain their independent rows.

**2026-10-05 architecture review checkpoint.** Regina probes in
`/tmp/crexx-signal-design.vt0eqA` confirm a called label without
`PROCEDURE` shares the caller pool, direct `SIGNAL` branches inside that
invocation and returns to the caller, and `SIGNAL ON SYNTAX` catches
`SUBSTR('abc',0)` with `RC=40`, `SIGL=3`. Current lowerer partitions each
label into a separate generated procedure and requires immediate `PROCEDURE`
and final `RETURN`; the existing canonical `SIGNAL_BLOCK` is lexical while
Classic ON/OFF is activation-wide. The architecture companion records the
approved one-frame replacement and new canonical AST/emitter requirement.
`SIGNAL 'target'` and `SIGNAL VALUE 'target'` currently share a `STRING` child
type, with only raw-token quoting distinguishing them. A temporary parser
probe confirmed an explicit VALUE wrapper can preserve the source intent;
the probe was removed until its full lowerer/emitter route is implemented.
No product code changed in this review; LC-63T-01–04 remain open.

**2026-10-05 LC-STEP-63T-1 reference receipt.** The parser admits static
symbol/quoted targets, `VALUE expression`, `ON condition [NAME target]` and
`OFF condition`, with targeted 19.x/25.x malformed-source diagnostics. The
Regina corpus at `/tmp/crexx-signal-contract.oKKgov` confirms direct and
VALUE branches, source-order fallthrough, missing-label Error 16.1, loop
re-entry after transfer, optional RETURN presence, default/named SYNTAX
labels, nested trap unwind to the installing activation, inherited pool
mutation, causing SIGL, OFF behavior and automatic trap disable after one
delivery. The VM supplies per-frame copy-on-write signal tables and `sigbrv`
for branch delivery; current `SIGNAL_BLOCK` is lexical, so the approved
canonical frame nodes and activation state remain necessary. `LC-STEP-63T-2`
is next; no SIGNAL form is called executable or complete by this review.

**2026-10-05 LC-STEP-63T-2 node checkpoint.** Four childless canonical nodes
now describe a top-level frame label, an associated static branch, and
associated ON/unassociated OFF handler operations. The structural validator
requires branch and ON targets to be labels in the same procedure; the AST
display exposes their association. The flow overlay resolves forward/backward
branch edges after sequence construction, includes every label as a possible
fresh-call entry, and conservatively models asynchronous handler edges.
Inlining refuses to clone a callable carrying these frame nodes until its
association/activation semantics can be proved. RXAS emission writes a local
label, branch, `sigbr` or `sighalt`, maps Classic SYNTAX to
`CLASSIC_SYNTAX`, and uses crossed-scope cleanup for an explicit branch.
The `frame_control_ast` test checks structural validation, optimized/no-opt
flow analysis, source metadata and emitted RXAS. At this first checkpoint no
Level C instruction was lowered to these nodes. Handler delivery cleanup and one-shot policy,
VALUE dispatch, one-body invocation state, the full reference matrix and all
LC-63T-01–04 gates remain open for the following checkpoints.
The exact-input Debug `rxc` and unit target built; focused frame and Level
B/G signal checks passed 5/5, and the normal Debug Level C suite passed
463/463, including the new unit. Evidence:
`/tmp/crexx-63t2-qual-build.log`,
`/tmp/crexx-63t2-qual-focused.log`, and
`/tmp/crexx-63t2-qual-levelc.log`. The earlier sweep was superseded after
the crossed-scope cleanup review and is not counted as this checkpoint's
qualification.

**2026-10-05 LC-STEP-63T-3C state checkpoint.**
`RexxActivationArguments` now carries an optional return result with a
separate presence flag. A fresh frame starts absent; explicit Unicode
`RexxValue` results and bare-return clearing preserve captured ARG slots and
recursive isolation. This supplies state for the approved one-body route but
does not yet change Level C CALL/RETURN lowering or close `63T-3C`.
The exact-input Debug `testRexxActivationArguments` target built; its opt and
no-opt functional cases passed 2/2, the Classic ARG and RexxScript consumers
passed 6/6, and the normal Debug Level C suite passed 463/463. Evidence:
`/tmp/crexx-63t3-return-build.log`,
`/tmp/crexx-63t3-return-focused.log`,
`/tmp/crexx-63t3-return-consumers-build.log`,
`/tmp/crexx-63t3-return-consumers.log`, and
`/tmp/crexx-63t3-return-levelc.log`. All `LC-63T-01–04` gates remain open.

**2026-10-05 LC-STEP-63T-3A/3B/3C frame checkpoint.** The active lowerer
now emits one generated callable body, a main wrapper, and source-anchored
`FRAME_LABEL`/`FRAME_BRANCH` entry dispatch. Main captures host arguments;
each local CALL or function call captures its actuals once in a fresh
activation and selects a label in a separate VM frame. The former per-label
procedure bodies, mandatory immediate `PROCEDURE`, final `RETURN`, and general
main `EXIT` guards are removed. Labels without `PROCEDURE` share the caller
pool, and source-order fallthrough works from main and from a called label.
`PROCEDURE` creates a distinct private pool before rebinding the active pool;
EXPOSE aliases the saved caller pool. Explicit/bare RETURN update the
activation's separate result-presence state. Main/local validation and
lowering now share one dispatch path.

The first private-pool lowering wrote into a register linked to the caller,
allowing a self-exposure loop. A bounded VM reproducer was stopped before
qualification; the repaired source creates a separate private register and
then rebinds the active link. The affected execution returned `3` at about
38 MiB RSS under a process memory monitor. New optimized/no-opt regressions
cover main and called-label fallthrough, nested shared-pool calls, recursive
calls, and private pool isolation. Existing PROCEDURE EXPOSE, ARG/function,
source-tree and linked tests remain in the normal suite. The final exact-input
Debug Level C suite passed 473/473 with a process guard and no remaining
children; the ARG/RexxScript consumer set passed 6/6 after the independently
committed RexxScript file-test scheduling repair (`5682355ba`). Evidence:
`cmake-build-debug/levelc-63t3-pool-fix-runtime.log`,
`cmake-build-debug/levelc-63t3-new-cases.log`,
`cmake-build-debug/levelc-63t3-qual.log`, and
`cmake-build-debug/levelc-63t3-isolation-locked.log`.

`LC-STEP-63T-3` and `LC-63T-01–04` remain open. Main fallthrough into a
`PROCEDURE` label is still conservatively rejected until the runtime raises
17.1 according to fresh-call versus fallthrough entry state; the same Regina
reference gave 17.1 after printing the preceding main output. Function calls
still use a static value-RETURN slice check rather than a complete runtime
missing-result contract. CALL/RETURN/EXIT, SIGNAL, handler cleanup, condition
identity, linked/reference matrices and host interfaces need their remaining
instruction gates. A machine reboot removed older `/tmp` test logs; this
checkpoint's evidence is retained under the ignored Debug build tree.

**2026-10-05 LC-STEP-63T-3E PROCEDURE lifecycle checkpoint.** Each internal
CALL/function activation now records permission to execute one `PROCEDURE`;
main has no such permission. `PROCEDURE` validates before creating its private
pool. The old static main-`EXIT`/`PROCEDURE` fallthrough guard is gone.
Regina probes accepted ordinary instructions and label fallthrough before
`PROCEDURE`, and this checkpoint incorrectly used that permissive behavior as
the compatibility oracle. The [IBM PROCEDURE reference](https://www.ibm.com/docs/en/zos/3.1.0?topic=instructions-procedure)
requires it to be the first instruction processed after the internal call.
The late-success assertion in this checkpoint is superseded by `63T-3E2`;
the passing suite below does not qualify that incorrect expectation.
Main fallthrough and a second `PROCEDURE` were tested as runtime 17.1.

The exact-input Debug core/runtime build succeeded. Focused frame and
activation tests passed 8/8; RexxScript runtime isolation passed 2/2; the
normal Debug Level C suite passed 478/478. The guarded suite peaked at about
2008 MiB aggregate descendant RSS and left no child processes. Evidence:
`cmake-build-debug/levelc-63t3e-build.log`,
`cmake-build-debug/levelc-63t3e-focused.log`,
`cmake-build-debug/levelc-63t3e-rexxscript.log`,
`cmake-build-debug/levelc-63t3e-qual.log`, and
`cmake-build-debug/levelc-63t3e-reference-*.log`.

`LC-STEP-63T-3D`, the whole PROCEDURE review and all `LC-63T-01–04` gates
remain open. The source scanner rejects a same-label PROCEDURE after ordinary
statements as source 17.1; Regina's acceptance is a reference divergence,
not a reason to lift this check. Runtime 17.1 currently reports the
`RexxActivationArguments` library line and main wrapper source rather than
the authored PROCEDURE line (`levelc-63t3e-diagnostic.log`); source
provenance needs repair. Full CALL/RETURN/EXIT and SIGNAL lifecycles, linked
and host matrices retain their own gates.

**2026-10-05 LC-STEP-63T-3E2 first-instruction correction.** The
[IBM REXX PROCEDURE reference](https://www.ibm.com/docs/en/zos/3.1.0?topic=instructions-procedure)
requires PROCEDURE to be the first instruction processed after an internal
invocation. Regina's acceptance of later and `IF ... THEN PROCEDURE` forms
was a permissive interpreter behavior, not a reason to change that rule.
The uncommitted attempt to lift the source check was withdrawn; its new
nested test failed at compiler validation before this correction. The source
scanner's first-after-label check remains in place. The activation now loses
PROCEDURE eligibility on the first different executed instruction, including
SAY before source-order fallthrough to another label. An empty label does not
consume it. The former Regina-only late-success test now expects runtime
17.1 after the preceding output.

The guarded Debug build passed; focused opt/no-opt frame and activation tests
passed 12/12, RexxScript runtime passed 2/2, and the normal Debug Level C
suite passed 480/480. The suite peaked at about 1947 MiB aggregate descendant
RSS and left no child processes. Evidence:
`cmake-build-debug/levelc-63t3e2-build.log`,
`cmake-build-debug/levelc-63t3e2-focused.log`,
`cmake-build-debug/levelc-63t3e2-rexxscript.log`, and
`cmake-build-debug/levelc-63t3e2-qual.log`.
`LC-STEP-63T-3D` and all `LC-63T-01–04` gates remain open; authored-line
runtime 17.1, function/CALL/RETURN lifecycle, SIGNAL and the linked/host
matrix still need their respective qualification.

**2026-10-05 LC-STEP-63T-3G authored error source.** The compiler now emits
an eligibility check and `CLASSIC_SYNTAX` signal at the authored PROCEDURE
node before the pool transition. The activation method retains its guard for
direct callers. The runtime 17.1 tests require the authored source filename
and line for main fallthrough, late called-label fallthrough and a second
PROCEDURE, in optimized and no-opt runs. Private-pool positive cases remain
in the focused set. The Debug `rxc` build passed; focused tests passed 8/8;
the normal Debug Level C suite passed 480/480. The guarded suite peaked at
about 1886 MiB aggregate descendant RSS and left no child processes.
Evidence: `cmake-build-debug/levelc-63t3g-build.log`,
`cmake-build-debug/levelc-63t3g-focused.log`, and
`cmake-build-debug/levelc-63t3g-qual.log`. The unchanged E2 RexxScript
runtime evidence remains valid because this increment changed only compiler
lowering and tests.

`LC-STEP-63T-3D` and all `LC-63T-01–04` gates remain open. This checks one
authored error family; full PROCEDURE forms, CALL/RETURN/EXIT lifetimes,
SIGNAL, linked/reference and host matrices need their remaining reviews.

**2026-10-05 LC-STEP-63T-3D ARG/frame invocation matrix checkpoint.**
Regina and compiled probes agree for a no-PROCEDURE caller that rereads its
omitted ARG slot after falling through an empty label, a nested
`PROCEDURE EXPOSE` callee that changes the caller's pool, and recursive
ARG activations with shared and private pools. In the shared-pool case,
earlier frames observe the deepest call's final `depth`; private PROCEDURE
frames retain their own `depth`. Each activation retains its own argument
slots in both cases. Permanent optimized/no-opt tests cover all three
programs, and linked execution covers the nested caller and callee. A
separate compile test requires source-anchored 17.1 for `IF 1 THEN
PROCEDURE`, following the documented first-processed-instruction rule
despite Regina accepting that form. Focused results and the exact-input
normal Debug Level C suite passed 8/8 and 488/488 respectively. The guarded
suite peaked at about 1899 MiB aggregate descendant RSS and left no child
processes; the post-run process inventory was empty. Evidence:
`cmake-build-debug/levelc-63t3d-matrix-build.log`,
`cmake-build-debug/levelc-63t3d-matrix-focused.log`,
`cmake-build-debug/levelc-63t3d-matrix-qual.log`, and the three ignored
`cmake-build-debug/levelc-arg-*-reference.log` Regina probes.
At exact code/test commit `3713800a4`, the Release product build passed.
The first Release Level C sweep passed 485 tests and reported three Not Run
because their test executables were absent from the Release tree; it found
no product-test failure. Building those three targets and running the same
three tests passed 3/3 on unchanged inputs, completing the 488-test
partition without repeating the 485 passed cases. Guards reported no
remaining children. Evidence: `cmake-build-debug/levelc-63t3d-release-build.log`,
`cmake-build-debug/levelc-63t3d-release-qual.log`,
`cmake-build-debug/levelc-63t3d-release-missing-build.log`, and
`cmake-build-debug/levelc-63t3d-release-missing-tests.log`. The earlier
RexxScript runtime 2/2 evidence remains applicable: this checkpoint changed
only Level C test inputs, CTest registration and documentation, not shared
runtime code.
This is frame and ARG coverage, not LC-I-11 closure: remaining admitted
invocation modes, full error/reference review, and the shared SIGNAL and
CALL lifecycles remain open under LC-AC-71 and LC-63T-01–04.

**2026-10-05 LC-STEP-63T-3H review finding.** A local function whose label
contains only bare `RETURN` currently fails compilation as an unsupported
main statement, before its invocation can execute ARG. The static
`returns_value` bit examines only direct statements in a label segment;
it cannot represent a reached RETURN, fallthrough to another label, or
optional result per invocation. [IBM's RETURN instruction](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-return)
requires a result for a function, and its
[compiler runtime errors](https://www.ibm.com/docs/SSLTBW_3.2.0/pdf/h1981606.pdf)
distinguish an attempted bare function RETURN (45) from a function that
finishes without data (44).
Regina's bare-RETURN probe reported 44.1 instead; this difference is
recorded as reference behavior, while the worklist's existing 45.1
contract follows the IBM instruction/error definition. The minimal probe
and diagnostics are retained at `cmake-build-debug/levelc-function-bare-return-*.log`.
At this review point, the repair and qualification of 3H remained open.

**2026-10-05 LC-STEP-63T-3H function-result checkpoint.** The obsolete
per-label `returns_value` scan and admission guard are removed. A fresh
activation records whether a local entry is a function or subroutine;
either may run ARG and traverse labels before returning. A reached bare
function RETURN emits 45.1 at its authored instruction. A function frame
that finishes without setting a result emits 44.1 at the invoking expression.
An explicit empty-string RETURN is present, while a bare subroutine RETURN
remains legal. One shared compiler helper emits the anchored Classic signal
for these guards and the existing PROCEDURE 17.1 check. Optimized/no-opt
regressions cover both error paths and the positive empty/fallthrough/
recursive/subroutine matrix; linked execution covers the positive matrix.
The direct RexxActivationArguments test checks that the call kind is local
to its activation. The positive output matches Regina, while bare function
RETURN intentionally uses IBM 45.1 instead of Regina 44.1 as reviewed above.

On the exact code/test/build inputs for this checkpoint, the guarded Debug
build passed, focused function/activation/RexxScript tests passed 11/11,
and the normal Debug Level C suite passed 495/495. The Release build and
normal Release Level C suite passed 495/495. Peak aggregate descendant RSS
was about 2068 MiB in Debug and 1702 MiB in Release; every guard and the
post-run process inventory found zero residual compiler, assembler, VM,
build or test processes. Evidence:
`cmake-build-debug/levelc-63t3h-final-build.log`,
`cmake-build-debug/levelc-63t3h-final-focused.log`,
`cmake-build-debug/levelc-63t3h-debug-qual.log`,
`cmake-build-debug/levelc-63t3h-release-build.log`, and
`cmake-build-debug/levelc-63t3h-release-qual.log`.
LC-STEP-63T-3D/3H and the bounded one-body implementation step are complete;
LC-63T-01–04, SIGNAL, ARG and the whole CALL/PROCEDURE/RETURN/EXIT
instruction reviews remain open. The next shared dependency is
LC-STEP-63T-4 SIGNAL on the frame model.

**2026-10-05 LC-STEP-63T-4 pre-edit review.** Fresh guarded Regina
programs retained under `cmake-build-debug/levelc-signal-*-ref.rexx` and
`levelc-signal-*-reference.log` show direct SIGNAL setting SIGL to its
clause line, VALUE evaluating a variable target and setting SIGL, a reached
missing target raising 16.1 after preceding output, SYNTAX trapping a
`SUBSTR` error with `RC=40` and `SIGL` at the causing clause, and a direct
SIGNAL inside a called label followed by an explicit RETURN to the caller.
Each process exited. The detailed
[IBM z/VM reference](https://www.ibm.com/support/pages/zvm/library/710pdfs/71631400.pdf)
also gives a `SIGNAL VALUE` multiway internal CALL that returns explicitly
and says a SIGNAL inside a subroutine ends only that subroutine's active DO
loops. Shorter IBM user-guide wording that SIGNAL itself does not return to
the caller therefore does not overturn the approved same-invocation frame
model. LC-AC-76 and the `63T-4A–4D` checkpoints above preserve the full
instruction and its condition lifecycle as open; no SIGNAL product code had
changed at this pre-edit review.

**2026-10-05 LC-STEP-63T-4A source-AST checkpoint.** The parser now places
the authored `VALUE` expression in a `LEVELC_SIGNAL_VALUE` child of
`LEVELC_SIGNAL`; validation checks its shape, and raw-tree regression checks
direct, VALUE, ON and OFF source forms. The missing-expression source error
remains 19.4. This preserves intent for the later static branch and evaluated
dispatch paths without changing SIGNAL execution or admitting an incomplete
runtime form. The guarded core compiler build passed, focused tree and
syntax-highlighting tests passed 3/3, and the normal Debug Level C suite
passed 496/496 on the same code/test/build inputs. Peak aggregate descendant
RSS was about 2461 MiB, and the guard and process inventory found zero
residual compiler, assembler, VM, build or test processes. Evidence:
`cmake-build-debug/levelc-63t4a-build.log`,
`cmake-build-debug/levelc-63t4a-focused.log`,
`cmake-build-debug/levelc-63t4a-debug-qual.log`, and
`cmake-build-debug/compiler/tests/levelc_signal_source_tree.log`.
Checkpoint 4A is complete. Direct and VALUE transfers, ON/OFF handlers,
LC-AC-76, ARG and full Level C/Release 1 qualification remain open.

**2026-10-05 LC-STEP-63T-4B direct-branch checkpoint.** Direct symbol and
quoted SIGNAL targets now resolve to source-order `FRAME_LABEL` nodes in the
one-body invocation and emit `FRAME_BRANCH`, including crossed DO/selection
and reference cleanup. The authored SIGNAL clause writes its line number to
the visible `SIGL` symbol before transfer. A target absent from the local
label plan emits Classic 16.1 at runtime only if the statement is reached;
the former eager 16.1 source diagnostic is removed, while the existing
compile-time nested-label 16.2 remains. Quoted static targets now use the
normal string decoder before name resolution. Permanent opt/no-opt cases
cover direct and quoted transfers, a branch out of DO in a called activation,
unreached and reached missing labels, SIGL, source-anchored 16.1, and
canonical frame nodes. The IF and quoted-local outputs match fresh Regina
reference runs in `cmake-build-debug/levelc-63t4b-regina-*.log`.

The guarded Debug core build passed, focused tests passed 11/11, and the
normal Debug Level C suite passed 505/505 on the same code/test/build inputs.
Peak aggregate descendant RSS was about 1805 MiB; the guard and post-run
process inventory found zero residual compiler, assembler, VM, build or test
processes. Evidence: `cmake-build-debug/levelc-63t4b-build4.log`,
`cmake-build-debug/levelc-63t4b-focused4.log`, and
`cmake-build-debug/levelc-63t4b-debug-qual.log`. Checkpoint 4B is complete.
VALUE dispatch, ON/OFF handlers, LC-AC-76, ARG and full Level C/Release 1
qualification remain open.

**2026-10-05 LC-STEP-63T-4C evaluated-target checkpoint.** The explicit
`LEVELC_SIGNAL_VALUE` child now lowers its expression once into the shared
Classic `TRANSLATE` uppercase BIF and captures the resulting Unicode text.
Source-order comparisons against the invocation's labels use the same
associated `FRAME_BRANCH` nodes as direct SIGNAL; the fallback emits a
source-anchored runtime 16.1 with the evaluated target. SIGL records the
authored SIGNAL clause after evaluation and before dispatch. The generated
body imports TRANSLATE when a VALUE form needs it. Permanent opt/no-opt
tests cover one-time function side effects, mixed-case targeting, a later
label, and a missing target; canonical-tree evidence checks the frame branch,
uppercase BIF and fallback signal. Fresh Regina outputs agree for the
one-time and later-label cases, and its missing-label run reports 16.1.

The guarded Debug core build passed, focused tests passed 9/9, and the normal
Debug Level C suite passed 512/512 on the exact code/test/build inputs. Peak
aggregate descendant RSS was about 1955 MiB; every guard and the post-run
process inventory found zero residual compiler, assembler, VM, build or test
processes. Evidence: `cmake-build-debug/levelc-63t4c-build2.log`,
`cmake-build-debug/levelc-63t4c-focused2.log`,
`cmake-build-debug/levelc-63t4c-debug-qual.log`, and
`cmake-build-debug/levelc-63t4c-regina-*.log`. Checkpoint 4C is complete.
ON/OFF and named/default condition handlers, LC-AC-76, ARG and full Level
C/Release 1 qualification remain open.

**2026-10-05 LC-STEP-63T-4D pre-edit handler review.** Fresh guarded Regina
probes in `cmake-build-debug/levelc-signal-on-*-ref.rexx` and matching
`-reference.log` show default `SYNTAX` and named targets, `RC=40` and causing
`SIGL`, missing named label 16.1 only on delivery, OFF restoring the untrapped
40.14 error, and one-shot delivery (a second fault in the handler is not
trapped). Each reference process exited. The VM already has copy-on-write
frame handler tables and `sigbrv` event binding; the current childless
`FRAME_HANDLER_ON` emits only `sigbr`, so it cannot carry the causing line and
error identity into Classic pool updates. Extend that canonical node with an
explicit event binding, then enter a generated frame-local handler block that
disables its condition, updates pool state, and branches to the user label.
The raw event's line/message and the supported condition-name mapping require
focused proof before claiming 4D2/4D3. The VM signal table currently has no
separate names for Classic HALT, NOVALUE or LOSTDIGITS; their producer and
mapping review remains open rather than being silently excluded. No handler
product code changed in this review.

**2026-10-05 LC-STEP-63T-4D1 event-binding AST checkpoint.** A canonical
`FRAME_HANDLER_ON` may now own one `VAR_TARGET` binding for the delivered VM
signal; childless registrations retain their prior `sigbr` form. Structural
validation rejects other child shapes, and RXAS emission uses `sigbrv` with
the bound register and the existing Classic SYNTAX mapping. The frame-control
unit exercises both modes, structural validation and optimized/no-opt flow
analysis. This establishes compiler-to-emitter event transport but does not
yet lower a Level C ON/OFF source form or claim RC/SIGL and one-shot behavior.
The guarded Debug compiler/unit build passed, focused tests passed 3/3, and
the normal Debug Level C suite passed 512/512 on the exact code/test/build
inputs. Peak aggregate descendant RSS was about 2400 MiB; the guard and
post-run process inventory found zero residual build, test, compiler,
assembler or VM processes. Evidence:
`cmake-build-debug/levelc-63t4d1-build3.log`,
`cmake-build-debug/levelc-63t4d1-focused2.log`, and
`cmake-build-debug/levelc-63t4d1-debug-qual.log`. Checkpoint 4D1 is complete;
4D2/4D3, LC-AC-76, ARG and full Level C/Release 1 remain open.

**2026-10-05 LC-STEP-63T-4D2 same-frame handler increment (4D2 remains
open).** The Level C lowerer now accepts source-order `SIGNAL ON/OFF SYNTAX`,
including the default `SYNTAX` label and explicit `NAME` target. ON registers
the canonical event-binding frame handler; a generated frame-local entry
disables the condition after delivery, writes the event's causing line to
`SIGL` and its Classic major error code to `RC`, then branches to the selected
label. OFF restores untrapped behavior. Direct BIF calls capture their result
once, test the shared context and raise `CLASSIC_SYNTAX` at the authored call
site; the exported `rexxclassicbif_checked` remains available to other
callers. This corrects the former handler `SIGL=219` library-line error while
retaining unhandled 40.14 source and output order. Permanent optimized and
no-opt tests cover named/default catches; focused OFF and one-shot cases also
pass. The guarded core/library build and focused 15/15 checks passed with
zero residual child processes. Evidence:
`cmake-build-debug/levelc-63t4d2a-final-build.log` and
`cmake-build-debug/levelc-63t4d2a-focused.log`. The normal Debug Level C
suite passed 518/518 on the same code/test/build inputs; its guarded peak
aggregate RSS was about 2031 MiB, and all test/build/compiler/VM processes
exited. Evidence: `cmake-build-debug/levelc-63t4d2a-debug-qual.log`.

Two reference gaps remain explicit. A missing named target raises runtime
16.1 only when the condition arrives, but the panic source is the ON clause
(line 2) instead of Regina's faulting BIF clause (line 4); evidence:
`cmake-build-debug/levelc-63t4d2-matrix-focused.log` and
`cmake-build-debug/levelc-signal-on-missing-reference.log`. A parent ON policy
inherited by a nested CALL currently unwinds to the installing parent frame.
Regina handles it in the active called invocation: the exposed value and
`RC|SIGL` agree (`inner|40|10`), then RETURN resumes the caller and prints
`after`; the VM output currently omits `after`. Evidence:
`cmake-build-debug/levelc-63t4d2-nested-focused2.log` and
`cmake-build-debug/levelc-63t4d2-nested-regina2.log`. Resolve both source
provenance and nested handler ownership before 4D2/4D3 or SIGNAL closure.
Other condition names, complete reference matrix, LC-AC-76, ARG and full
Level C/Release 1 qualification remain open.

**Next 63T-4D2 frame-lifetime repair (LC-63T-01/02/04).** An activation will
own the selected ON-clause identity for each Classic condition. A local CALL
or function invocation copies that policy at entry, and the common body
prologue rebinds its selected handler to the new VM frame. Source-order ON,
OFF and one-shot delivery update both the activation policy and VM handler;
the variable pool remains variable storage. Verify parent-to-child delivery,
child override/OFF isolation, RETURN continuation, recursion, opt/no-opt and
event line/RC against Regina before closing nested policy. Separately repair
runtime missing named-label 16.1 to retain the event's causing source. These
are repairs within the approved one-body-per-invocation architecture.

**2026-10-05 LC-STEP-63T-4D2 nested SYNTAX policy checkpoint (4D2 remains
open).** `RexxActivationArguments` now owns a seven-condition policy table.
An internal CALL/function activation copies it independently, and the common
compiled body rebinds the selected ON-clause handler into its own VM frame
before label dispatch. ON, OFF and one-shot delivery update activation and VM
state together. The SYNTAX reference matrix now covers a parent policy caught
in an exposed child procedure, child ON override with parent restoration,
child OFF isolation and recursive calls with shared-pool argument mutation.
Regina and compiled opt/no-opt outputs agree, including `SIGL`, `RC`, handler
RETURN and caller continuation. The activation library unit verifies fresh,
copied and independent policy state. The guarded core/library and activation
test builds passed; combined focused checks passed 15/15 and the normal Debug
Level C suite passed 526/526 on the exact code/test/build inputs. Guarded
aggregate RSS peaked at about 2465 MiB in the broad run; each guard found
zero residual child processes. Evidence:
`cmake-build-debug/levelc-63t4d2b-build.log`,
`cmake-build-debug/levelc-63t4d2b-activation-build.log`,
`cmake-build-debug/levelc-63t4d2b-combined-focused.log`,
`cmake-build-debug/levelc-63t4d2b-debug-qual.log`, and the
`cmake-build-debug/levelc-63t4d2b-*-regina*.log` probes. Missing named-label
16.1 still reports the ON clause instead of the faulting clause; other
conditions, linked proof and complete SIGNAL qualification remain open.

**Next 63T-4D2 source-provenance repair (LC-63T-01/02/04).** A handler-entry
16.1 needs the delivered event's module/address, which ordinary `signal`
currently replaces with the trampoline's static source. Add a VM/RXAS signal
form with an explicit bound `.runtime_signal` origin and message operand,
validate that origin before use, and carry its source only for that raised
condition. The compiler will emit this form for a missing named handler label
after one-shot disable and visible RC/SIGL update. Prove the exact Regina
missing-label case with optimized/no-opt execution and authored panic source,
plus invalid-origin isolation and unaffected ordinary signal handling. Keep
the opcode semantics explicit in the shared ISA effect/signal descriptions.
This is an implementation of the approved frame-handler transport, not a
change to Classic SIGNAL syntax or scalar semantics.

**2026-10-05 LC-STEP-63T-4D2 missing-target source checkpoint (4D2 remains
open).** The canonical ISA now has `signalorigin "NAME",rMessage,rSignal`:
RXAS effect/signal metadata classifies its source operands and barrier, and
the VM accepts only a bound runtime event, records its module/address for that
new condition in the current run, and otherwise raises `INVALID_ARGUMENTS`.
Ordinary `signal` source and payload handling is unchanged. A generated
missing named-target handler disables one-shot policy, records RC/SIGL, then
uses this form to raise 16.1 at the delivered event's causing clause. The
Regina probe and compiled optimized/no-opt cases all locate the SUBSTR fault
at line 4. An explicit RXAS invalid-origin regression and existing `sigbrv`
transport checks pass. A linked image preserves 16.1, prior output and the
faulting source. The guarded core, origin fixture and linker builds passed;
focused checks passed 12/12 and the normal Debug Level C suite passed 528/528
on the exact code/test/build inputs. The broad guard peaked at about 1944 MiB
aggregate RSS and all guarded children exited. Evidence:
`cmake-build-debug/levelc-63t4d2c-build.log`,
`cmake-build-debug/levelc-63t4d2c-origin-build.log`,
`cmake-build-debug/levelc-63t4d2c-focused.log`,
`cmake-build-debug/levelc-63t4d2c-linked-build.log`,
`cmake-build-debug/levelc-63t4d2c-linked-run.log`, and
`cmake-build-debug/levelc-63t4d2c-debug-qual.log`. The separate threaded VM
build and linked-image run also retained the line-4 source:
`cmake-build-debug/levelc-63t4d2c-threaded-build.log` and
`cmake-build-debug/levelc-63t4d2c-threaded-run.log`. Other condition names,
complete handler cleanup, full SIGNAL matrix, LC-AC-76, ARG and full Level
C/Release 1 remain open.

**2026-10-05 LC-STEP-63T-4D3 reference and transport review (no product edit).**
The guarded Regina matrices in
`cmake-build-debug/levelc-63t4d3-regina-policy.log` and
`cmake-build-debug/levelc-63t4d3-regina-default.log` accept ON/OFF with
default or named labels for ERROR, FAILURE, HALT, NOTREADY, NOVALUE, SYNTAX
and LOSTDIGITS. A NOVALUE read traps at its causing line with the unset symbol
as `CONDITION('D')`; a numeric-digits-3 arithmetic operand traps LOSTDIGITS
at its causing line with the over-precise operand as the description. In
both reference cases an unset `RC` remains unset, so the SYNTAX `RC` rule
must not be copied to other conditions. The
[ANSI REXX standard](https://www.rexxla.org/rexxlang/standards/j18pub.pdf)
defines LOSTDIGITS for significant nonzero digits discarded from an
over-precise arithmetic operand; the
[IBM z/OS condition reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=reference-conditions-condition-traps)
describes the other six and the host command/error distinctions. Both
references inform the matrix rather than silently omitting the ANSI name.

The VM's `sig_atomic_t` mask admits codes 1–31, with 29 the only unused code;
30 is OTHER and 31 is BREAKPOINT. Reusing VM ERROR/FAILURE/NOTREADY directly
would trap unrelated Level B events, and widening the process/native signal
mask would affect the VM's signal-safe publication path. The 4D3 implementation
will use code 29 for a distinct Classic non-SYNTAX event carrying a typed
condition name and description in its payload. One frame handler will dispatch
to the activation's selected ON clause for that name; SYNTAX keeps its
separate code 28 path. Compiler-generated producers will raise that event at
the authored clause only when the Classic condition is enabled (or when HALT
requires delivery), while the host-command, I/O, interrupt and numeric
producer reviews remain explicit obligations. This stays within the approved
VM branch/handler and activation-policy architecture, preserves Level B
signal identities, and avoids conflating the seven Classic conditions.
`LC-STEP-63T-4D3`, SIGNAL, ARG and full Level C/Release 1 remain open.

**2026-10-05 LC-STEP-63T-4D3-1 typed event checkpoint.** VM signal 29 is now
`CLASSIC_CONDITION`, separate from Level B ERROR/FAILURE/NOTREADY and Classic
SYNTAX. Its `RexxClassicConditionEvent` payload carries one of the six
non-SYNTAX policy IDs and a Unicode description; the bound VM runtime event
continues to own causing source metadata. The VM, RXPA, compiler signal
validation and Level B signal-object name/code tables agree on code 29. A
linked runtime test raises a typed NOVALUE event and verifies its identity,
Unicode payload and source behavior in optimized and no-opt modes; the Level B
signal-object table and the signal-mask uniqueness test also pass. The initial
focused run exposed a harness mistake: the optimized test needs the opt-mode
argument to expect stripped source metadata. Correcting that test input made
the focused 4/4 pass without a product change.

The guarded core/library build completed, `test_signal_mask` passed 1/1, and
the normal Debug Level C suite passed 528/528 on these code/test/build inputs.
The broad guard peaked at about 2062 MiB aggregate RSS; every guard and the
post-run process inventory found zero residual compiler, assembler, VM, build
or test processes. Evidence:
`cmake-build-debug/levelc-63t4d3a-build.log`,
`cmake-build-debug/levelc-63t4d3a-build2.log`,
`cmake-build-debug/levelc-63t4d3a-focused3.log`,
`cmake-build-debug/levelc-63t4d3a-mask-build.log`,
`cmake-build-debug/levelc-63t4d3a-mask-test.log`, and
`cmake-build-debug/levelc-63t4d3a-debug-qual.log`.
The compiler has not yet dispatched that event to seven-name policy or added
condition producers. `LC-STEP-63T-4D3-2/3`, whole SIGNAL and ARG reviews,
and Level C/Release 1 qualification remain open.

**2026-10-05 LC-STEP-63T-4D3-2 and bounded 4D3-3 checkpoint.** The lowerer
accepts ON/OFF with default and named handlers for all seven parsed Classic
condition names. SYNTAX retains its distinct VM event; the other six use one
frame-local `CLASSIC_CONDITION` handler, a typed event ID, and the active
invocation's policy table to select the source-order ON clause. One-shot
delivery disables only that condition in the current activation. Internal
calls copy the parent's policy; child OFF, override and delivery do not alter
the parent's selection. A missing named target raises 16.1 at the causing
source clause. Neither non-SYNTAX delivery nor OFF changes `RC`; the bounded
NOVALUE producer guards authored variable reads and raises a typed event
containing the resolved symbol name only when NOVALUE is enabled. Compound
stem defaults and tail substitution use the existing shared pool operations.

Regina probes for default/OFF, one-shot re-enabling, nested inheritance,
child OFF/override, compound default and a missing target agree with the
compiled results. The source files include a leading `OPTIONS LEVELC` line,
so their compiled `SIGL` values are one line greater than the stripped Regina
probes. Optimized/no-opt CTests passed 14/14; the affected activation,
condition-event and Classic-BIF runtime tests passed 6/6. The guarded core
build and normal Debug Level C suite passed 542/542. The broad run peaked at
about 2707 MiB aggregate descendant RSS and left no child processes; the
post-run process inventory was empty. Evidence:
`cmake-build-debug/levelc-63t4d3b-override-build.log`,
`cmake-build-debug/levelc-63t4d3b-override-focused.log`,
`cmake-build-debug/levelc-63t4d3b-runtime-focused.log`,
`cmake-build-debug/levelc-63t4d3b-debug-qual.log`,
`cmake-build-debug/levelc-63t4d3b-regina-novalue.log`, and
`cmake-build-debug/levelc-novalue-nested-override-regina.log`.

This closes the policy-dispatch checkpoint and one available producer, not
`LC-STEP-63T-4D3-3` or SIGNAL. ERROR, FAILURE, HALT and NOTREADY need their
host, I/O and interrupt producer ownership and reference matrices;
LOSTDIGITS needs numeric-context integration. `CONDITION()` needs current
event name/description/state behavior for trap handlers. Full SIGNAL error,
source, linked, optimized/no-opt and Level B/G/RexxScript isolation under
`63T-5`, the ARG review and all Level C/Release 1 criteria remain open.

**2026-10-05 LC-STEP-63T-4D3-3 condition-state checkpoint.** A trapped
condition now records its name, description, extra field and trapping
instruction in the Classic activation. An internal call copies those fields
with its policy; a trap in the child changes only the child's current
condition, and the caller's state is restored on return. The direct
`CONDITION([option])` BIF reads C/D/E/I/S, defaults to I, normalizes valid
option initials and uses the shared argument checker for invalid option,
empty option and excess arguments. `S` reads the current policy, so one-shot
delivery reports OFF and a later ON reports ON. The bounded NOVALUE path
records the resolved symbol description and an empty ANSI extra field;
Regina returned `0` for `CONDITION('E')` in the retained probe, so that
implementation-specific difference is explicit. SYNTAX supplies the
major/minor error identity and an error-prefixed diagnostic description,
but the exact catalog-expanded `CONDITION('D')` text remains open.

Regina reference probes cover all five selectors, omission, nested
save/restore and 40.21/40.28/40.4 validation. The optimized/no-opt new cases
passed 12/12; adjacent SIGNAL and runtime cases passed 20/20. The guarded
core/library build passed, and the normal Debug Level C suite passed 554/554
on these code/test/build inputs. The broad run peaked at about 1979 MiB
aggregate descendant RSS and left no child processes; the post-run inventory
was empty. Evidence:
`cmake-build-debug/levelc-condition-reference.log`,
`cmake-build-debug/levelc-condition-syntax-reference.log`,
`cmake-build-debug/levelc-condition-nested-restore-reference.log`,
`cmake-build-debug/levelc-condition-empty-reference.log`,
`cmake-build-debug/levelc-condition-extra-reference.log`,
`cmake-build-debug/levelc-63t4d3c-build2.log`,
`cmake-build-debug/levelc-63t4d3c-rebuild.log`,
`cmake-build-debug/levelc-63t4d3c-rebuild2.log`,
`cmake-build-debug/levelc-63t4d3c-focused5.log`,
`cmake-build-debug/levelc-63t4d3c-adjacent-focused.log`, and
`cmake-build-debug/levelc-63t4d3c-debug-qual.log`.

`LC-STEP-63T-4D3-3` and SIGNAL remain open: complete SYNTAX description
rendering, ERROR/FAILURE/HALT/NOTREADY host and I/O producers, LOSTDIGITS
numeric producer, full condition extra-data behavior and the 63T-5 reference,
linked and isolation matrix still need proof. ARG and full Level C/Release 1
criteria remain open.

**LC-STEP-63T-5 whole-SIGNAL review in progress, 2026-10-05.** Treat the
following as one instruction contract before another SIGNAL closure claim:

| Contract area | Current implementation and decisive remaining check |
| --- | --- |
| Direct symbol/quoted and evaluated VALUE | One frame-local branch route, once-only VALUE evaluation and runtime 16.1 exist. Complete source, Unicode-label, nested-call, crossed-DO and re-entry reference cases in optimized/no-opt and linked output. A fresh Regina/compiled crossed-loop and subsequent SYNTAX trap probe agrees; its ignored evidence is `cmake-build-debug/levelc-signal-whole-probe-*.log`. |
| ON/OFF for seven parsed names | Activation policy, source-order override, one-shot delivery, nested inheritance and named/default labels exist. Audit malformed tails and reached missing targets together with the actual condition-producer matrix. |
| Condition state and descriptions | `CONDITION()` reads activation-local C/D/E/I/S and SYNTAX/NOVALUE state. Complete catalog-expanded SYNTAX description from the one standard diagnostic catalog and review extra data against the reference, without a second hand-maintained message table. |
| Producer ownership | SYNTAX and NOVALUE are live. ADDRESS and implicit commands now produce ERROR/FAILURE and stream/input NOTREADY; NUMERIC now produces LOSTDIGITS. HALT remains under host interrupt lifecycle (LC-AC-06). Exercise each live producer through SIGNAL and each of the seven handler identities through controlled typed events; keep missing real host producers with their owners. A parsed ON/OFF form alone is not delivery evidence. |
| Crossed control and adjacent instructions | Check loop/reference/handler cleanup on branch or trap and source-order label fallthrough. EXIT is closed; test its reached behavior after a local transfer without reopening that instruction. |
| Delivery and isolation | Verify raw/canonical AST, authored source and errors, full `rxc`→`rxas`→`rxlink`→`rxvm`, optimized/no-opt, normal correctness, and Level B/G/RexxScript isolation on the final code/test inputs. |

The current probe is a reference and implementation audit, not a new closed
instruction or a reason to move the strict whole-instruction queue. The
approved frame design remains the shared prerequisite for ARG, PROCEDURE and
CALL; LC-AC-76 and LC-I-23 remain open pending this whole review.

**LC-I-23 SIGNAL plan — vision and intended outcome, 2026-10-06.** Complete
Classic SIGNAL on the approved one-body, frame-local label and activation
model. A direct or evaluated transfer reaches the correct source label and
clears crossed control state. Condition handlers respond at their source
points, keep policy and condition state local to each invocation, and use the
same source-anchored diagnostic catalog and typed event route. Existing
ADDRESS, NUMERIC and variable-pool producers are exercised here; the absent
host HALT producer stays owned by `LC-AC-06`. The Level B/G and RexxScript
signal contracts remain intact. This is one whole-instruction checkpoint;
no linker or VM change is authorized by this plan.

1. **LC-85-01 — forms and errors (complete):** direct symbol/quoted target,
   explicit and special-character-implied VALUE expression, and ON/OFF for
   all seven condition names preserve raw/canonical AST shape, expression
   evaluation count, source location, malformed-tail identity and reached
   missing-target 16.1. Verify parser/AST inspection, Regina or normative
   source, and optimized/no-opt compile and runtime cases.
2. **LC-85-02 — frame transfer (complete):** direct and dynamic transfer select
   the first case-insensitive Unicode label, permit source-order fallthrough
   through duplicates, clear crossed DO/SELECT state, and support re-entry,
   local calls and RETURN/EXIT without stale references. Verify linked
   optimized/no-opt executions and generated RXAS frame branches.
3. **LC-85-03 — handlers and delivery (complete):** ON/OFF source order,
   named/default target, one-shot disable, child inheritance/override and
   caller restoration work for SYNTAX, NOVALUE, ERROR, FAILURE, HALT,
   NOTREADY and LOSTDIGITS. Exercise six identities with controlled typed
   events and SYNTAX through its native producer; use other live producers
   where available. Verify RC, SIGL and source
   identity, including a reached missing handler target. The unimplemented
   real host HALT producer remains `LC-AC-06`, not a SIGNAL exclusion.
4. **LC-85-04 — condition state (complete):** `CONDITION()` C/D/E/I/S and default
   reflect the current activation's last event and policy; SYNTAX uses the
   shared catalog, and available producer descriptions/extra fields follow
   their owned contracts. Verify reference wording, one-shot state and
   nested/recursive restoration with focused BIF tests.
5. **LC-85-05 — coherent closure (complete):** one frame-label/handler route
   covers the legal instruction rather than case-specific control paths.
   Inspect AST-to-RXAS, run focused linked and shared-consumer checks plus
   the normal Debug and Release Level C suites once on final code/test inputs,
   update architecture/reference docs and commit. Keep full Level C and
   Release 1 criteria open.

1. **LC-STEP-85A (`LC-85-01–04`; complete):** reconcile the retained SIGNAL
   reference matrix, current implementation and now-live producer paths;
   identify the smallest missing whole-contract mechanisms before edits.
2. **LC-STEP-85B (`LC-85-01–02`; complete):** repair source/branch/flow
   issues in the approved frame model and qualify transfer forms together.
3. **LC-STEP-85C (`LC-85-03–04`; complete):** complete the shared
   handler/policy/condition route, reference descriptions and live-producer
   matrix without changing each producer's separate owner contract.
4. **LC-STEP-85D (`LC-85-01–05`; complete):** run one grouped final
   correctness checkpoint, record concise evidence, synchronize docs and
   commit SIGNAL as a whole instruction.

**LC-I-23 closure, 2026-10-06.** The retained direct/quoted/VALUE, Unicode and
duplicate-label, crossed-control, nested-call, malformed-tail and missing-target
cases cover the transfer contract. This increment adds a linked opt/no-opt
matrix for six typed condition identities plus native SYNTAX, and extends the
native ADDRESS callback fixture through SIGNAL FAILURE and ERROR. Shared
activation and event tests cover policy, source, Unicode payload and caller
isolation; raw and lowered AST plus generated RXAS inspection confirm the
single frame-local route. Focused SIGNAL and adjacent regressions passed
106/106; Debug Level C passed 737/737, Release Level C passed 737/737, and
Level B/G signal and RexxScript isolation passed 10/10 in each group on final
code/test/build inputs.
All test/build child processes exited. There is no SIGNAL product-code change
in this final review. Real host HALT production and broader Level C and
Release 1 criteria remain open under their own owners.

**2026-10-05 SIGNAL closure-boundary correction.** Adrian clarified that
SIGNAL responds to conditions; ADDRESS, host/I/O and NUMERIC own the missing
ERROR/FAILURE, HALT/NOTREADY and LOSTDIGITS producers. Qualify SIGNAL's
ON/OFF, named/default dispatch, one-shot and nested policy for each parsed
condition identity with controlled typed events plus available live
SYNTAX/NOVALUE producers. Close SIGNAL when its own complete handler, direct
branch, error and lifecycle matrix passes. Keep each absent real producer
open under its owning row until separately implemented and tested; do not
label the SIGNAL instruction incomplete solely because those producers are
absent. Full Level C and Release 1 criteria remain open.

**SYNTAX description checkpoint, 2026-10-05.** `rxfnsc` generates a build-local
template module from the existing `messages/diagnostics.en_GB.msg` catalog.
The activation's trapped SYNTAX record now expands raw named inserts once,
without reinterpreting argument text that contains braces, quotes or
backslashes. Three Regina-derived `CONDITION('D')` wording cases (40.14,
40.12 with braces, and 40.12 with escapes) pass optimized and no-opt; the
shared BIF harness and existing SYNTAX state tests pass in the same focused
group, 11/11. The unchanged code/test/build inputs passed the normal Debug
Level C suite 564/564. Guarded evidence is
`cmake-build-debug/levelc-signal-catalog-final-build.log`,
`cmake-build-debug/levelc-signal-renderer-focused2.log`, and
`cmake-build-debug/levelc-signal-catalog-debug-qual.log` (peak 4973 MiB,
zero residual processes). Level G signal and RexxScript isolation checks
passed 10/10 in `cmake-build-debug/levelc-signal-catalog-isolation.log`.
The whole SIGNAL matrix remains pending; this
checkpoint does not close SIGNAL or ARG.

**Next whole-SIGNAL finding, 2026-10-05.** A reference probe of trapped
`SUBSTR('abc','')` requires `CONDITION('D')` to end with `found ""`.
The current shared BIF diagnostic builder omits an empty `value` insert and
therefore falls back to raw field names. A catalog/call-site audit also found
that 40.19/40.32/40.33 require two distinct `value` inserts, while the
current builder labels the second `optionslist`. Preserve named-insert
presence and order from the catalog for these shared BIF errors, including
empty values. Qualify exact reference wording for empty SUBSTR, two-value
RANDOM and DATE cases in optimized/no-opt, then run the normal correctness
suite. Invalid byte-data 23.1 still needs its separate hex-encoding producer;
this is not SIGNAL or BIF instruction closure.

The shared builder/renderer repair passes Regina wording for those three
discovered shapes in optimized/no-opt mode. A permanent crossed-control
SIGNAL case also branches out of nested DO groups, enters fresh loop control,
then traps SYNTAX and checks `SIGL` and one-shot state. Optimized, no-opt and
linked execution pass with the description group, 20/20 focused. Guarded
logs are `cmake-build-debug/levelc-signal-inserts-build.log`,
`cmake-build-debug/levelc-signal-inserts-rebuild.log`, and
`cmake-build-debug/levelc-signal-inserts-focused2.log`, all with zero
residual processes. The normal Debug Level C suite passed 573/573 on these
code/test/build inputs in
`cmake-build-debug/levelc-signal-inserts-debug-qual.log` (peak 4899 MiB,
zero residual processes). Level G signal and RexxScript isolation checks
passed 10/10 in `cmake-build-debug/levelc-signal-inserts-isolation.log`.
The whole SIGNAL matrix and ARG remain open.

**Unicode label normalization finding, 2026-10-05.** A guarded Level C probe
of `SIGNAL VALUE 'é'` to `é:` compiled but raised runtime 16.1: the VM's
Unicode `TRANSLATE` produced `É`, while the compiler's source-label
normalizer applied bytewise `toupper` and kept `é`. Use the VM's existing
simple codepoint uppercase map for compiler-side source labels and static
targets, then verify direct/quoted/VALUE SIGNAL, local CALL and duplicate
labels with Unicode names, optimized/no-opt and linked. Keep the Unicode-first
scalar contract; this is a normalization repair within the approved frame
design, not a new language-design choice.

The same Unicode-label matrix exposed a separate whole-SIGNAL/CALL label
rule: [IBM's clause reference](https://www.ibm.com/docs/en/cics-ts/6.x?topic=concepts-clauses-instructions)
allows duplicate labels, sends explicit transfers to the first occurrence,
and permits ordinary fallthrough through later occurrences. Regina agrees
for ASCII duplicates. The current one-body plan incorrectly rejects a
second name. Remove that rejection while retaining a frame label at every
source position; the existing first-match lookup should choose the target.
Qualify CALL and direct/VALUE SIGNAL first-target selection, ordinary
fallthrough, Unicode case variants, optimized/no-opt and linked output.

The label-normalization and first-duplicate frame repair passes 15/15
focused direct/VALUE/CALL/crossed-control checks in
`cmake-build-debug/levelc-signal-duplicate-focused.log`; the guarded build
is `cmake-build-debug/levelc-signal-duplicate-build.log`. Both left zero
residual processes. The final guarded build
`cmake-build-debug/levelc-signal-label-final-build.log` and normal Debug
Level C suite in `cmake-build-debug/levelc-signal-label-debug-qual.log`
passed on the exact code/test/build inputs, 579/579 (peak 4895 MiB, zero
residual processes). Level G signal and RexxScript isolation passed 10/10
in `cmake-build-debug/levelc-signal-label-isolation.log`. Duplicate labels
remain ordinary source-order frame blocks; explicit target lookup selects
the first. The SIGNAL handler/condition-state matrix remains open; real
producer-specific extra data remains with each producer's owning row.

**Remaining legal SIGNAL forms found in the whole review, 2026-10-05.**
Regina accepts `SIGNAL ON SYNTAX NAME 'caught'` and transfers to that quoted
trap name. The parser already emits a STRING target through its shared
`call_name_opt`, but Level C validation admits only a symbol and the lowerer
does not decode quoted text. Reuse the direct-SIGNAL source-literal decoder
and Unicode normalization for both targets; qualify quoted handler delivery,
missing-target source identity, optimized/no-opt and linked behavior.
[IBM's SIGNAL reference](https://www.ibm.com/docs/en/zvm/7.2.0?topic=instructions-signal)
also admits omission of `VALUE` when the evaluated expression starts with a
special character. Regina rejected `SIGNAL ('done')` with 19.4 in the local
probe. IBM's documented form governs this compatibility implementation. The
source adapter now inserts a zero-width VALUE token before an initial unary
operator or parenthesis, preserving source order and the same
`LEVELC_SIGNAL_VALUE` AST/lowering route as explicit VALUE. The quoted NAME
form uses the direct target's source-literal decoder and Unicode label lookup.

Optimized/no-opt quoted handler delivery, reached missing quoted targets,
parenthesized evaluated targets, unary `+`, `-` and logical NOT evaluated
targets, malformed missing operands, raw AST, and linked execution passed
20/20 focused checks in
`cmake-build-debug/levelc-signal-forms-focused-final.log`. The final guarded
product build passed in `cmake-build-debug/levelc-signal-implicit-final-build.log`.
On those exact code/test/build inputs, the normal Debug Level C suite passed
594/594 in `cmake-build-debug/levelc-signal-forms-debug-qual.log` (peak 4633
MiB, zero residual processes), and Level G signal/RexxScript isolation passed
10/10 in `cmake-build-debug/levelc-signal-forms-isolation.log`. The process
inventory after each run was empty. This source-form checkpoint does not close
LC-I-23 or LC-AC-76: the remaining handler, condition-state and transfer
matrix still needs its whole-instruction review. Missing real producers and
their extra data retain their separate owners above.

## Findings

- **LC-FIND-08 — resolved indirect DROP invalid words:** the
  [IBM DROP reference](https://www.ibm.com/docs/SSGMCP_5.5.0/reference/rexx/drop.html)
  requires valid variable names in a parenthesized subsidiary list. Local
  Regina probes ignored invalid words such as `1bad`, `/` and `(b)` while
  continuing to drop later valid names. Adrian selected the Regina behavior
  on 2026-10-03 for Level C; `LC-AC-56` has a permanent pool and compiled
  regression. Configured UTF8 symbol-classification proof remains open under
  `LC-AC-04`.
- **LC-FIND-07 — resolved shared stem lifecycle:** Regina showed that a new
  stem assignment replaces prior tails, a dropped tail hides even a stem
  default until reassigned, and substituted tail text keeps its case. The
  shared `RexxStem`/`RexxVariablePool` model now records these states and
  writes back a mutated value-class stem through local or exposed bindings.
  Compiled direct stem/compound `DROP` was completed under `LC-AC-53` and
  `LC-STEP-57`.
- **LC-FIND-06 — resolved, frozen PARSE backward cursor:** Regina returns
  `cd|ef|def` for `s='abcdef'; parse var s 3 a +2 b -1 c`, while the
  existing certified Level B exit and VM `parseplan` return `cd|ef|ef`.
  The pending field was stored correctly, but the VM clamped the cursor to
  its capture start after the negative movement. The repair retains the moved
  cursor without a literal anchor while preserving the separate literal-anchor
  rule. Focused Level B regression, the existing literal-anchor reference
  checks, Release and Debug PARSE suites (11/11 each), and the opt/no-opt,
  linked, round-trip and concrete-VM frozen-plan contract pass. Evidence:
  `/tmp/crexx-levelc-mixed-cases-UcSYCM/`,
  `/tmp/crexx-parse-backward-exit-tSscyN/`,
  `/tmp/crexx-parse-backward-normal-release2.ykorOK`,
  `/tmp/crexx-parse-backward-normal-debug.XNPDtE`,
  `/tmp/crexx-parse-backward-contract-release.HYVgQ4`, and
  `/tmp/crexx-parse-backward-contract-debug.kEM5Bb`.
- **LC-FIND-01 — resolved, empty source string literal:** the independent minimal
  `options levelc; say ''` source previously failed during lowering, although the parser
  accepts its STRING node. The parser uses a zero-length payload with an
  absent text pointer; `LC-STEP-39` now lowers that valid shape through the
  canonical empty string and shared `RexxValue`. The initial reproducer is
  `/tmp/crexx-levelc-empty-literal-log.LX2Ksv`; permanent checks and closure
  evidence are recorded under `LC-AC-36` below.
- **LC-FIND-02 — resolved, SAY-adjacent function call parsed as symbols:** the
  existing-syntax `options levelc; say 'x' length('a')` previously printed
  `x LENGTH a` through Level C instead of Regina's `x 1`. `LC-STEP-40`
  now builds the `FUNCTION` node for an adjacent opening parenthesis while
  keeping the blank-separated symbol-plus-group AST. Reproducer output:
  `/tmp/crexx-levelc-adjacent-call-reference.fh07ZP` and
  `/tmp/crexx-levelc-adjacent-call-output.07V6xB`.
- **LC-FIND-03 — resolved, ARG case normalization:** a local procedure with
  `ARG value` called as `echo('z')` returns `ZZ` in Regina when it appends
  `'Z'`, while the former Level C path returned `zZ`. `LC-STEP-41` now uses
  shared TRANSLATE before binding the callee pool target. Reproducer output:
  `/tmp/crexx-levelc-adjacent-call-regina-output.qX60V2` and
  `/tmp/crexx-levelc-adjacent-call-toolchain.17jBQE`.
- **LC-FIND-04 — resolved, abutted expression terms gained spaces:** the independent
  minimal `options levelc; item='a'; say '['item']'` prints `[ a ]`, while
  Regina prints `[a]`. This is the expression/parser adjacency path, separate
  from the three-target PARSE split. Its reproducer is
  `/tmp/crexx-levelc-parse-three-probe.OKSy14/concat.rexx` with paired
  `concat-regina.out` and `concat-crexx.out`. `LC-STEP-44` now selects
  `OP_CONCAT` for physically adjacent terms and `OP_SCONCAT` for spaced terms.
  The new fixture covers groups, calls, comment-only gaps and PARSE VALUE.
- **LC-FIND-05 — digit-start repair complete, leading-period symbols open:** the former
  Level C scanner emitted integer, dot and integer tokens for `1.5`; the
  expression parser could lose the dot. Before `LC-STEP-44`, blank concatenation
  made the existing `DO ... FOR 1.5` invalid-value test pass; unrestricted
  abuttal briefly turned it into `15`. The parser now refuses to infer
  abuttal across an omitted dot, preserving the invalid-value result.
  `LC-STEP-45/46` now scan decimal fraction and valid exponent forms into a
  single numeric source token and lower them through shared `RexxValue`, so
  `1.5` retains its value and still fails the contextual FOR count check.
  `LC-STEP-47` now scans digit-starting nonnumeric constant symbols as one
  source operand. Leading-period symbols and processor-state meanings remain
  open under `LC-AC-04/08`. Reproducer:
  `compiler/tests/rexx_src/levelc_slice35_dynamic_for_fraction.rexx` and
  `/tmp/crexx-levelc-abuttal-fraction-tree.pGPSc7`.

## AST structural crosswalk and closure order

This crosswalk is grounded in `compiler/rxcpcgmr.y`'s `program`,
`instruction`, `simple_instruction`, control-flow, and expression productions,
the `NodeType` catalogue, and the guards in `rxcp_levelc_lower.c`. **Slice**
means an accepted subset with executable evidence; **open** means the parser
may emit the shape but the lowerer rejects it, or runtime proof is incomplete.
Parser recovery `ERROR`/`WARNING` nodes remain source diagnostics and never
enter accepted lowering. A generic node name shared with Level B does not by
itself make its Classic shape executable.

| Parser-emitted family | Current AST/lowering disposition | Structural risk and next proof |
| --- | --- | --- |
| Program shell, `REXX_OPTIONS`, top-level `INSTRUCTIONS`, `LABEL` | Slice: one generated body with frame-label entry dispatch and a main wrapper; generated `REXX_OPTIONS` imports and canonical siblings replace the Classic instruction wrapper | Multiple file/label layouts, option placement, source anchors and generated symbol/scope ownership |
| `ASSIGN`, `SAY`, `NOP`, `EXIT`, `RETURN`, `LEVELC_DROP` | SAY, DROP, assignment, NOP, RETURN and EXIT instructions closed under their own criteria | Shared configuration and remaining instruction owners remain open under their own rows |
| `VAR_SYMBOL`/`VAR_TARGET`, strings, integers, expression operators, function calls | Slice: proven scalar/compound pool reads, including empty quoted strings, literal and operator methods, eager Classic `&`/`|`, bounded BIF/local calls including adjacent calls under blank concatenation | More expression shapes, remaining operator order, numeric context and missing-argument behavior remain open |
| `IF` with condition/THEN/ELSE; simple `DO` with `INSTRUCTIONS` | Whole IF and DO instructions closed; canonical branch/group builders cover accepted nested contexts | Shared condition/TRACE lifecycle and per-arm instruction owners remain open |
| `SELECT` with `INSTRUCTIONS` of `WHEN` and optional `OTHERWISE` | Whole SELECT instruction closed under LC-AC-68, including ordered/lazy arms and `34.2`/`7.3` errors | Shared condition/TRACE lifecycle remains open |
| Header-bearing `DO`, `REPEAT`, `FOR`, `WHILE`, `UNTIL`, `BY`, `TO`, `LEAVE`, `ITERATE` | Whole DO, LEAVE and ITERATE instructions closed under LC-AC-65/69/70 with one checked header, shared loop state, compound controls and arbitrary numeric counts | Shared NUMERIC, condition, TRACE and host proof remains open in their own rows |
| `LABEL`, `LEVELC_PROCEDURE`, `LEVELC_ARG`, `CALL`, `RETURN` | One generated callable body with frame labels; whole ARG, PROCEDURE, CALL and RETURN instructions closed, including aliases, signed CALL and activation result presence | Condition producers and full shared lifecycle remain open |
| `PARSE`, `PULL`, template/pattern/position nodes | Whole PARSE and PULL instructions closed: seven agreed PARSE sources and PULL use shared `parseplan` with arbitrary templates, ordered pool writes, source snapshots, errors and opt/no-opt linked proof | EXTERNAL/NUMERIC PARSE extensions are outside initial Level C; host input selection stays open under LC-AC-06 |
| `LEVELC_ADDRESS`, command expression, `LEVELC_PUSH`, `LEVELC_QUEUE` | PUSH and QUEUE instructions closed; ADDRESS and implicit command remain front-end foundations | Configured environment/stream protocol and RC/condition behavior under LC-I-20/21 |
| `LEVELC_NUMERIC`, `LEVELC_SIGNAL`, `LEVELC_TRACE`, `LEVELC_INTERPRET`, condition CALL forms | NUMERIC and SIGNAL whole instructions close under LC-I-22/23; TRACE and INTERPRET remain front-end or bounded foundations | Host HALT production, dynamic code, trace and shared compatibility criteria remain open under their rows |

The production post-lowering boundary verifier walks every accepted node,
rejects a missing root, sibling cycle, mismatched parent pointer, or surviving
`LEVELC_*` instruction node before normal canonical validation. Its direct
negative unit test proves each rejection path; accepted nested fixtures and
the `STAGE_LEVELC_LOWERED` debug probe cover generated output. This gate does
not yet prove generated scope/symbol correctness, source anchors, evaluation
order, or completeness of accepted structural families. Those remain open in
`LC-AC-08` and are tackled through `LC-STEP-12` and later structural increments.

## Coverage matrix

Inventory pass 2: the 70 recognized BIF names and 36 existing syntax catalogue
contracts have individual rows below. The
[reference-obligation appendix](levelc-reference-obligations.md) adds 75
finer non-BIF rows, including configuration, error and host behavior. Complete
reference-to-row reconciliation and feature-level evidence remain open under
`LC-AC-01`; the rows are a coverage map, not a conformance verdict.

States are **whole instruction** (instruction review closed), **slice** (bounded end-to-end execution), **front end** (parsed or
diagnosed, not generally executable), **runtime** (standalone Classic helper,
not general Level C compilation), and **open** (not yet evidenced). These are
implementation observations, not claims of full conformance. Every row
remains open for LC-AC-04 until qualified or explicitly excepted. The
Unicode-first scalar contract and default/STRICTC transfer timing are approved
language boundaries; they do not excuse unfinished instruction or host work.

| Area | Feature | Current state and evidence | Remaining proof |
| --- | --- | --- | --- |
| Source | comments, clauses, literals, symbols, contextual keywords, labels, continuations, source characters | Front end: `levelc_syntax_highlighting.md` | Reference edge cases, configured character/length limits, diagnostics |
| Expressions | precedence, arithmetic, comparisons, concatenation, prefix, eager logical `&`/`|` | Slice: `levelc_slice6_expressions`, `levelc_slice19_logical_eager`, empty quoted strings in `levelc_slice37_empty_string`, and adjacent function calls in `levelc_slice38_adjacent_call` | Full numeric context, remaining operator order, boundary/error and platform equivalence |
| Variables | scalar read/write, drop, compound names, bare stems, exposure, API pool | Whole assignment, DROP and PROCEDURE reviews cover source-ordered direct/indirect scalar/stem/exact compound exposure and private-pool alias lifecycle | External/API pool behavior and full shared-runtime contract remain open |
| Control | IF/THEN/ELSE | Whole IF instruction closed under LC-AC-67/LC-STEP-68C: comprehensive arm, nesting, error, opt/no-opt and linked evidence | Globally unsupported instructions in arms and shared trap/condition lifecycle retain their own open rows |
| Control | simple, counted and controlled DO/END, FOREVER, WHILE/UNTIL | Whole DO instruction closed under LC-AC-65/LC-STEP-70D, including compound controls, arbitrary numeric counts, error paths and linked execution | Shared NUMERIC, condition, TRACE and host lifecycle remain open in their own rows |
| Control | LEAVE/ITERATE | Both whole instructions closed under LC-AC-69/70, including named/unnamed transfers and default/STRICTC timing | Shared SIGNAL/TRACE and invocation lifecycle remain open |
| Control | SELECT/WHEN/OTHERWISE | Whole SELECT instruction closed under LC-AC-68/LC-STEP-69C | Shared condition, TRACE and host lifecycle remain open |
| Control | NOP | Whole instruction closed under LC-AC-64: childless opt/no-opt, nested SELECT/IF/DO/local, source anchors, invalid `21.1`, normal and linked proof | Shared labeled-clause, TRACE and host text lifecycle remains open under LC-AC-04/08 and later rows |
| Routines | labels, local/external CALL and functions, ARG, PROCEDURE EXPOSE, RETURN, EXIT | One generated body dispatches main/local labels with fresh frames; ARG, PROCEDURE, CALL, RETURN and EXIT whole-instruction reviews are closed. CALL includes linked and native host signed providers, result presence and delayed handler lifecycle. | Real condition producers and shared invocation lifecycle retain their own open reviews |
| PARSE | ARG, PULL, SOURCE, LINEIN, VERSION, VALUE, VAR; templates and UPPER | Whole PARSE instruction closed under LC-81-01–07: one `parseplan` route, all seven agreed sources, arbitrary targets/commas/patterns/positions, frame/source/input behavior and errors, with normal/ASan/isolation evidence | EXTERNAL/NUMERIC are explicitly excluded; host selection and external function-expression entry remain with their owners |
| Environment | ADDRESS, command clauses, WITH redirection | Front end: parser/validation | Configured command/stream service and RC/condition behavior |
| Conditions | CALL ON/OFF, SIGNAL, HALT, ERROR, FAILURE, NOTREADY, NOVALUE, LOSTDIGITS, SYNTAX | CALL ON/OFF closes under LC-I-13 and SIGNAL closes under LC-I-23; live SYNTAX/NOVALUE, ADDRESS ERROR/FAILURE/NOTREADY and NUMERIC LOSTDIGITS producers pass their instruction reviews | Real host HALT producer and full cross-instruction lifecycle remain open |
| Numeric | DIGITS, FORM, FUZZ, decimal arithmetic, rounding, logical conversion | Runtime: `RexxValue` foundation | Full context, limits, signal and optimized parity |
| Source/trace | TRACE, SOURCELINE, clause hooks, source preservation | TRACE whole instruction closed with the approved practical divergences; SOURCELINE and full source identity remain open | Source/line services and cross-instruction diagnostic lifecycle |
| Host | commands, external routines, queues, streams, time/random, traps, API variable pools, initialization/termination | Runtime foundation only | Configuration adapters and supported-platform contract |
| BIFs | each recognized Classic BIF | See individual rows below | Direct compiler calls, Classic argument/error/context equivalence |
| Shared runtime | Level C and RexxScript value, pool and overlapping BIF contracts | Both import `rxfnsc`; RexxScript uses a sandbox pool and `RexxValue` BIF frames | Cross-consumer behavior, errors, isolation and approved Unicode boundary under `LC-AC-06` |

### Syntax and instruction inventory

These 36 contract names come from the existing [raw language catalogue](component-catalogue/raw-language-syntax.md). Each row is distinct from full compatibility; several raw catalogue names group multiple Classic variants and still need finer reference reconciliation under `LC-AC-01`.

| Contract | Feature | Current evidence level | Remaining proof |
| --- | --- | --- | --- |
| `SYN-CLASSIC-OPTIONS` | Classic `OPTIONS` clauses and Level C selection | Whole instruction closed under LC-AC-66/LC-STEP-67D | Shared condition, TRACE and host proof remains open in its own criteria |
| `SYN-CLASSIC-CLAUSES` | Semicolon/EOL clause model | Parser and 24 closed instruction reviews | Full cross-instruction clause/condition lifecycle open |
| `SYN-CLASSIC-CONTEXTUAL-KEYWORDS` | Instruction words usable as symbols outside instruction context | Parser plus selected executable variable/expression contexts | Complete contextual reference matrix open |
| `SYN-CLASSIC-LABELS` | Labels and local routine names | One-body frame-label entry, source-order fallthrough, nested shared-pool and recursive CALL pass opt/no-opt; PROCEDURE 17.1 is source-anchored; CALL and SIGNAL label and handler matrices close under LC-I-13/23 | TRACE, INTERPRET and shared source lifecycle remain open |
| `SYN-CLASSIC-SYMBOLS` | Simple, compound, and constant symbols | Shared pool reads and closed assignment/DROP paths exercise scalar, stem and compound symbols | Constant-symbol and full condition/reference proof open |
| `SYN-CLASSIC-STEMS` | Classic stems and compound-variable tails | Whole assignment, DROP and PROCEDURE paths cover stem and exact compound aliases | Shared condition and host/API alias proof remains open |
| `SYN-CLASSIC-STRINGS` | Quoted, doubled-quote, hex, and binary strings | Hex/binary source literals use the fixed Latin-1 ordinal bridge in expressions, calls and PARSE patterns under LC-STEP-88C; four/eight-bit binary grouping, malformed later-group `15.2`, encoded CALL target, opt/no-opt, tree and linked evidence pass under LC-STEP-75A-B; SAY host text output passes under LC-STEP-88D-1 | Remaining quoted forms, error/reference equivalence and other host text inputs remain open |
| `SYN-CLASSIC-ASSIGNMENT` | Simple, stem and compound assignment with expression or empty RHS | Whole instruction closed under LC-AC-63/LC-STEP-88D-3; byte-literal scalar values, Unicode compound tails, NUL in substituted tails and RHS-order/local exposure pass | Shared host/external API remains open under LC-AC-04/06 |
| `SYN-CLASSIC-COMMAND` | Implicit command clause | Whole instruction closed under LC-I-21 with the shared ADDRESS adapter | `LC-HOST-ADDRESS-NUL` and full host service contract remain under LC-AC-06 |
| `SYN-CLASSIC-ADDRESS` | Classic ADDRESS forms | Whole instruction closed under LC-I-20 with selection, transient commands and WITH resources | `LC-HOST-ADDRESS-NUL` and full host service contract remain under LC-AC-06 |
| `SYN-CLASSIC-ARG` | Classic ARG instruction | Whole instruction closed under LC-AC-71/LC-STEP-73: main and routine frames, omitted/present values, arbitrary comma templates, patterns and positions, exposed targets, repeated reads, Unicode, shared PARSE execution, authored diagnostics, opt/no-opt and linked output; external Classic CALL reuses this frame under LC-I-13 | C-string host entry's embedded-NUL limit remains a host-interface obligation; INTERPRET retains its instruction owner and must reuse the argument frame |
| `SYN-CLASSIC-CALL` | CALL routine and CALL ON/OFF forms | Whole instruction closed under LC-75-01–06/LC-STEP-75E: legal direct and policy forms/errors, local/BIF/external signed resolution, source-ordered/omitted actuals, RESULT presence/drop, linked and native host Unicode/NUL execution, four-condition delayed delivery, policy replacement, nested isolation, buffered HALT, authored-clause diagnostics and optimized/no-opt parity. The accepted static boundary requires provider visibility at caller compilation and inclusion in the image; an omitted linked provider keeps core `FUNCTION_NOT_FOUND`. | Real ADDRESS, stream and host HALT condition producers and their source identity remain open with their owning instruction/host rows; full Level C qualification remains open |
| `SYN-CLASSIC-DO` | Simple, counted, conditional, and forever DO | Whole DO instruction closed under LC-AC-65/LC-STEP-70D, including compound controls and arbitrary numeric counts | Shared NUMERIC, condition, TRACE and host lifecycle remain in their own rows |
| `SYN-CLASSIC-DROP` | DROP instruction | Whole instruction closed under LC-AC-62/LC-STEP-88D-2, including arbitrary direct compounds, Regina-style invalid-word skip and configured Unicode text classification | Shared pool/external host behavior remains under LC-AC-04/06 |
| `SYN-CLASSIC-EXIT` | EXIT instruction | Whole instruction closed under LC-77-01–06 | Broader host result exchange remains under its own owner |
| `SYN-CLASSIC-IF` | Classic IF/THEN/ELSE | Whole IF instruction closed under LC-AC-67/LC-STEP-68C | Each arm's instruction and shared condition/TRACE lifecycle remain in their own rows |
| `SYN-CLASSIC-INTERPRET` | INTERPRET instruction | Parked, not implemented: parser recognizes the form; lowerer rejects it | `LC-87-01–05` and Release 1 disposition open under LC-GAP-01 |
| `SYN-CLASSIC-ITERATE` | ITERATE instruction | Whole instruction closed under LC-AC-70/LC-STEP-72C | Shared loop/condition/TRACE lifecycle remains in its own rows |
| `SYN-CLASSIC-LEAVE` | LEAVE instruction | Whole instruction closed under LC-AC-69/LC-STEP-71C | Shared loop/condition/TRACE lifecycle remains in its own rows |
| `SYN-CLASSIC-NOP` | NOP instruction | Whole instruction closed under LC-AC-64/LC-STEP-66B | Shared label/TRACE and configuration proof remains open under LC-AC-04/08 |
| `SYN-CLASSIC-NUMERIC` | NUMERIC DIGITS/FORM/FUZZ | Whole instruction closed under LC-I-22/LC-84-01–05 | Shared expression/BIF numeric semantics remain under LC-GAP-07 |
| `SYN-CLASSIC-PARSE` | PARSE variants and templates | Whole instruction closed under LC-81-01–07 for ARG/PULL/SOURCE/LINEIN/VERSION/VALUE/VAR, UPPER, shared `parseplan`, errors and final checkpoint | EXTERNAL/NUMERIC excluded by Adrian; host input selection remains under LC-AC-06 |
| `SYN-CLASSIC-PROCEDURE` | PROCEDURE and EXPOSE | Whole instruction closed under LC-74-01–05: private pool, direct/indirect scalar/stem/exact compound aliases, first-instruction 17.1, source/AST, opt/no-opt and linked proof; CALL and RETURN are closed under LC-I-13/14 | EXIT and full host/condition lifecycle retain their own rows |
| `SYN-CLASSIC-PULL` | PULL instruction/templates | Whole instruction closed under LC-78-01–05 | Host selection API remains under LC-AC-06 |
| `SYN-CLASSIC-PUSH` | PUSH instruction | Whole instruction closed under LC-79-01–04 | Host selection API remains under LC-AC-06 |
| `SYN-CLASSIC-QUEUE` | QUEUE instruction | Whole instruction closed under LC-80-01–04 | Host selection API remains under LC-AC-06 |
| `SYN-CLASSIC-RETURN` | RETURN instruction | Whole instruction closed under LC-76-01–05/LC-STEP-76C: bare/value, main/local/function paths, presence/drop, private/shared pools, once-only Unicode/NUL result, status bridge, anchored errors and direct/linked/native host optimizer parity | EXIT and general host text-result exchange retain separate owners |
| `SYN-CLASSIC-SAY` | SAY instruction | Whole-instruction closure under LC-AC-57/LC-STEP-88D-1; NUL, source hex ordinals, mapped high characters and non-Latin-1 text pass default, host, optimized/no-opt and linked output | Missing expression/BIF, TRACE and SIGNAL services remain shared work |
| `SYN-CLASSIC-SELECT` | SELECT/WHEN/OTHERWISE | Whole instruction closed under LC-AC-68/LC-STEP-69C | Shared condition/TRACE lifecycle remains in its own rows |
| `SYN-CLASSIC-SIGNAL` | SIGNAL target and ON/OFF conditions | Whole instruction closed under LC-AC-76/LC-STEP-85: direct/quoted/VALUE, seven condition identities, frame transfer, policy/condition state, source and error matrix pass linked opt/no-opt plus Debug/Release Level C | Real host HALT production remains under LC-AC-06; full Level C and Release 1 criteria remain open |
| `SYN-CLASSIC-TRACE` | TRACE options/value | Whole instruction closed under LC-I-24/LC-AC-77 with documented practical divergences | Correct scalar display retained; full source/host lifecycle remains under LC-GAP-05/06 |
| `SYN-CLASSIC-EXPRESSIONS` | Classic arithmetic, comparison, Boolean, and concatenation expressions | Bounded slice: documented operator family | Remaining Classic forms, errors and configuration proof open |
| `SYN-CLASSIC-BIF-CALL` | Recognised Classic BIF calls | Direct compiler table for 62 of 70 recognised names, plus LOWER/UPPER; shared SYNTAX result bridge; Unicode character-family matrix and implicit TRANSLATE ordinals under LC-STEP-88E-1 | Eight deferred stream entries and full Unicode/I/O/source/host/resource reference proof remain under LC-GAP-02; see the grouped audit and final product receipt |
| `SYN-CLASSIC-LOCAL-CALL` | Direct local function/procedure calls | Whole CALL/ARG/PROCEDURE/RETURN instruction reviews cover expression actuals, omitted positions, fresh frames, result presence/drop and private/shared pools | EXIT, condition producers and full shared configuration proof remain open |
| `SYN-CLASSIC-DSLSH` | Source tree, diagnostics, and syntax-highlighting projection | Parser-mode milestone | Execution and full diagnostic conformance remain separate |
| `SYN-CLASSIC-CANONICAL-LOWERING` | Transformation to canonical compiler AST | Structural verifier, frame-local label/branch/handler nodes and 24 closed whole-instruction reviews | INTERPRET and complete AST ownership/provenance proof remain open |

### BIF grouped contract audit, 2026-10-07

This crosswalk reconciles every catalogued name with the current direct path,
shared validator and maintained behavior fixtures. Inspecting each checklist
establishes required/optional arguments and option sets; the shared validator
owns omitted-position count, normalization and Classic error construction.
BIF-specific checks own value/range and state changes. Compiler fixtures verify
the authored SYNTAX bridge rather than accepting a raw VM conversion failure.
Each existing standalone fixture below includes value or error assertions; the
bitwise and numeric-state aliases use their named shared fixtures. This is an
admitted-path audit, not an exhaustive conformance assertion for undefined
Unicode behavior or unapproved stream infrastructure.

| Audit group | Names / maintained proof | Checked behavior and remaining boundary |
| --- | --- | --- |
| `BIF-AUDIT-TEXT` (28) | ABBREV, CENTER/CENTRE, CHANGESTR, COMPARE, COPIES, COUNTSTR, DELSTR/DELWORD, INSERT, LASTPOS, LEFT/LENGTH, OVERLAY, POS, REVERSE/RIGHT, SPACE/STRIP, SUBSTR/SUBWORD, TRANSLATE, VERIFY, WORD/WORDINDEX/WORDLENGTH/WORDPOS/WORDS; per-name units below, 104-value `levelc_bif_reference_audit`, WHOLE and integer-limit fixtures | Required/optional strings, exact WHOLE positions/counts, PAD, LTB/MN options, empty strings, omitted defaults, boundaries, word blanks, reference values, error identities and source. WHOLE is signed 64-bit; successful allocation for every in-range size is not proved. Existing codepoint transport is retained; Unicode-caused signals/logic errors are undefined pending assessment. |
| `BIF-AUDIT-ORDINAL` (12) | B2X, BITAND/BITOR/BITXOR, C2D/C2X, D2C/D2X, XRANGE, X2B/X2C/X2D; direct units, shared Bitwise fixture, LC-STEP-88B and Latin-1 compiled fixtures | HEX/BIN rules, omitted length/pad, signed/unsigned and arbitrary-precision radix values, bitwise padding, NUL/high ordinals, all 256 C2X/X2C round trips and XRANGE wrap. Existing fixed Latin-1 bridge and conversion signals remain; no codec/raw-byte I/O change or new Unicode compatibility claim. |
| `BIF-AUDIT-NUMERIC` (6) | ABS, FORMAT, MAX/MIN, SIGN, TRUNC; direct units, common legacy unit, `level[b/c/g]_bif_numeric_context` in four execution modes | ANSI caller-DIGITS initial +0, FORM, variadic required operands, ties, fixed truncation/scale, FORMAT fields/overflow/errors and caller restoration. B/G decimal families use the same approved rule with typed results/signals; independent float/int families remain unchanged. Reduced-DIGITS Regina behavior differs from the approved ANSI rule. |
| `BIF-AUDIT-POOL` (3) | DATATYPE, SYMBOL, VALUE; per-name units, shared datatype/pool tests and reference audit | AB(L)MNSUWX options, configured classes, constant versus variable symbol, exact symbol spelling, uninitialized/set/drop/compound and named external pool behavior, omitted new value versus empty value and errors. Shared ASCII symbol classification repaired; scanner/encoded source LC-GAP-06 and wider host variable-pool LC-GAP-04 stay open. |
| `BIF-AUDIT-STATE` (10) | ADDRESS, ARG, CONDITION, DIGITS/FORM/FUZZ, TRACE, DATE, TIME, RANDOM; per-name/shared Numeric units, existing instruction/host fixtures and LC-STEP-90D CONDITION receipt | Omitted/count/options, selected environment, omitted/empty argument state, numeric/trace restoration, frozen clause time and elapsed/reset state, date/time conversions, random ranges/seed/config isolation; CONDITION C/D/E/I/S, ON/OFF/DELAY and all seven admitted IDs. Live ERROR/FAILURE/NOTREADY, SYNTAX/NOVALUE/LOSTDIGITS and controlled typed CALL events have field proof. Real host HALT stays LC-GAP-04; no wider host-service or full source closure. |
| `BIF-AUDIT-QUEUE` (1) | QUEUED; LC-STEP-90C, direct unit and direct/linked opt/no-opt fixture | Same selected execution-local repository as PULL/PUSH/QUEUE; non-consuming counts, FIFO/LIFO, named selection, NUL, errors/source and context isolation. Wider Level C/C host selector stays LC-GAP-03. |
| `BIF-AUDIT-SOURCE` (1) | SOURCELINE; LC-STEP-90D, direct unit and direct/linked/provider opt/no-opt fixtures | Retained original comments/blanks, CRLF/CR/LF and final line, no runtime reread, count/index/omissions/errors/source, local unit sharing and separate binary provider isolation. Mapped inputs return unavailable/count zero; physical source NUL truncation and full mapping remain LC-GAP-06. |
| `BIF-AUDIT-MESSAGE` (1) | ERRORTEXT; LC-STEP-90D, direct unit and direct/linked opt/no-opt fixture | Shared standard English catalog; S/N fallback, major/minor/undefined codes, decimal subcode zeros, range 40.17, omissions/count/options and authored error source. No localization service/host ABI was added. |
| `BIF-AUDIT-STREAM` (8) | CHARIN, CHAROUT, CHARS, LINEIN, LINEOUT, LINES, QUALIFY, STREAM | No direct entry or standalone Classic implementation. All defaults/named streams, EOF, positioning, encoding/NUL, state/commands, resource cleanup and isolation criteria remain unrun/open. Adrian deferred new I/O/Unicode infrastructure pending architectural assessment; LC-STEP-90B proposal is not approved. |

The compiled reference audit records 104 expected values in one maintained
fixture/output pair, checked with Regina 3.9.7. ANSI numeric rounding is separately
checked against the approved IBM/Classic rule; it is not falsely labelled Regina
parity. Existing Unicode departures retain their earlier labelled receipts;
this session adds no Unicode/host transport rule. The final product receipt
below owns exact-input grouped execution and supported sanitizer results.

### Individual BIF inventory

The source list is `component-catalogue/raw-levelc-bifs.md` (recognition only).
Final recount on `d7b58e17d`: 70 catalogued names, 62 in the existing compiler direct
entry table, and eight deferred stream names. LOWER/UPPER are two additional
entries outside this catalog. No second dispatcher was added. Shared aliases,
numeric wrappers and bitwise methods retain their existing common bodies.

Each row below points to its implementation, maintained behavioral regression
and the grouped audit boundary above. A direct entry, a passing example or a
unit test alone does not close the complete reference contract. The admitted
ASCII/Latin-1 baseline has focused proof; deferred Unicode, I/O, source/host
and resource limits remain visible under their owners. The 70-name baseline is
not complete while the eight stream names and those obligations remain open.

| BIF | Current state | Evidence / next proof |
| --- | --- | --- |
| `ABBREV` | Direct + runtime; admitted baseline audited | [`RexxClassicBifAbbrev.crexx`](../../../lib/rxfnsc/RexxClassicBifAbbrev.crexx); [`testRexxClassicBifAbbrev`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifAbbrev.crexx); `BIF-AUDIT-TEXT` boundary |
| `ABS` | Direct + runtime; admitted baseline audited | [`RexxClassicBifAbs.crexx`](../../../lib/rxfnsc/RexxClassicBifAbs.crexx); [`testRexxClassicBifAbs`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifAbs.crexx); `BIF-AUDIT-NUMERIC` boundary |
| `ADDRESS` | Direct + runtime; admitted baseline audited | [`RexxClassicBifAddress.crexx`](../../../lib/rxfnsc/RexxClassicBifAddress.crexx); [`testRexxClassicBifAddress`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifAddress.crexx); `BIF-AUDIT-STATE` boundary |
| `ARG` | Direct + runtime; admitted baseline audited | [`RexxClassicBifArg.crexx`](../../../lib/rxfnsc/RexxClassicBifArg.crexx); [`testRexxClassicBifArg`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifArg.crexx); `BIF-AUDIT-STATE` boundary |
| `B2X` | Direct + runtime; admitted baseline audited | [`RexxClassicBifB2x.crexx`](../../../lib/rxfnsc/RexxClassicBifB2x.crexx); [`testRexxClassicBifB2x`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifB2x.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `BITAND` | Direct + runtime; admitted baseline audited | [`RexxClassicBifBitand.crexx`](../../../lib/rxfnsc/RexxClassicBifBitand.crexx); [`testRexxClassicBifBitwise`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifBitwise.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `BITOR` | Direct + runtime; admitted baseline audited | [`RexxClassicBifBitor.crexx`](../../../lib/rxfnsc/RexxClassicBifBitor.crexx); [`testRexxClassicBifBitwise`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifBitwise.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `BITXOR` | Direct + runtime; admitted baseline audited | [`RexxClassicBifBitxor.crexx`](../../../lib/rxfnsc/RexxClassicBifBitxor.crexx); [`testRexxClassicBifBitwise`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifBitwise.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `C2D` | Direct + runtime; admitted baseline audited | [`RexxClassicBifC2d.crexx`](../../../lib/rxfnsc/RexxClassicBifC2d.crexx); [`testRexxClassicBifC2d`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifC2d.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `C2X` | Direct + runtime; admitted baseline audited | [`RexxClassicBifC2x.crexx`](../../../lib/rxfnsc/RexxClassicBifC2x.crexx); [`testRexxClassicBifC2x`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifC2x.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `CENTER` | Direct + runtime; admitted baseline audited | [`RexxClassicBifCenter.crexx`](../../../lib/rxfnsc/RexxClassicBifCenter.crexx); [`testRexxClassicBifCenter`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifCenter.crexx); `BIF-AUDIT-TEXT` boundary |
| `CENTRE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifCenter.crexx`](../../../lib/rxfnsc/RexxClassicBifCenter.crexx); [`testRexxClassicBifCentre`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifCentre.crexx); `BIF-AUDIT-TEXT` boundary |
| `CHANGESTR` | Direct + runtime; admitted baseline audited | [`RexxClassicBifChangestr.crexx`](../../../lib/rxfnsc/RexxClassicBifChangestr.crexx); [`testRexxClassicBifChangestr`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifChangestr.crexx); `BIF-AUDIT-TEXT` boundary |
| `CHARIN` | Deferred; no direct entry | `BIF-AUDIT-STREAM`: LC-STEP-90B; implementation and complete stream contract await Adrian’s architecture/compatibility assessment |
| `CHAROUT` | Deferred; no direct entry | `BIF-AUDIT-STREAM`: LC-STEP-90B; implementation and complete stream contract await Adrian’s architecture/compatibility assessment |
| `CHARS` | Deferred; no direct entry | `BIF-AUDIT-STREAM`: LC-STEP-90B; implementation and complete stream contract await Adrian’s architecture/compatibility assessment |
| `COMPARE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifCompare.crexx`](../../../lib/rxfnsc/RexxClassicBifCompare.crexx); [`testRexxClassicBifCompare`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifCompare.crexx); `BIF-AUDIT-TEXT` boundary |
| `CONDITION` | Direct + runtime; admitted baseline audited | [`RexxClassicBifCondition.crexx`](../../../lib/rxfnsc/RexxClassicBifCondition.crexx); [`testRexxClassicBifCondition`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifCondition.crexx); `BIF-AUDIT-STATE` boundary |
| `COPIES` | Direct + runtime; admitted baseline audited | [`RexxClassicBifCopies.crexx`](../../../lib/rxfnsc/RexxClassicBifCopies.crexx); [`testRexxClassicBifCopies`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifCopies.crexx); `BIF-AUDIT-TEXT` boundary |
| `COUNTSTR` | Direct + runtime; admitted baseline audited | [`RexxClassicBifCountstr.crexx`](../../../lib/rxfnsc/RexxClassicBifCountstr.crexx); [`testRexxClassicBifCountstr`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifCountstr.crexx); `BIF-AUDIT-TEXT` boundary |
| `DATATYPE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifDatatype.crexx`](../../../lib/rxfnsc/RexxClassicBifDatatype.crexx); [`testRexxClassicBifDatatype`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifDatatype.crexx); `BIF-AUDIT-POOL` boundary |
| `DATE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifDate.crexx`](../../../lib/rxfnsc/RexxClassicBifDate.crexx); [`testRexxClassicBifDate`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifDate.crexx); `BIF-AUDIT-STATE` boundary |
| `DELSTR` | Direct + runtime; admitted baseline audited | [`RexxClassicBifDelstr.crexx`](../../../lib/rxfnsc/RexxClassicBifDelstr.crexx); [`testRexxClassicBifDelstr`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifDelstr.crexx); `BIF-AUDIT-TEXT` boundary |
| `DELWORD` | Direct + runtime; admitted baseline audited | [`RexxClassicBifDelword.crexx`](../../../lib/rxfnsc/RexxClassicBifDelword.crexx); [`testRexxClassicBifDelword`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifDelword.crexx); `BIF-AUDIT-TEXT` boundary |
| `DIGITS` | Direct + runtime; admitted baseline audited | [`RexxClassicBifNumeric.crexx`](../../../lib/rxfnsc/RexxClassicBifNumeric.crexx); [`testRexxClassicBifNumeric`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifNumeric.crexx); `BIF-AUDIT-STATE` boundary |
| `D2C` | Direct + runtime; admitted baseline audited | [`RexxClassicBifD2c.crexx`](../../../lib/rxfnsc/RexxClassicBifD2c.crexx); [`testRexxClassicBifD2c`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifD2c.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `D2X` | Direct + runtime; admitted baseline audited | [`RexxClassicBifD2x.crexx`](../../../lib/rxfnsc/RexxClassicBifD2x.crexx); [`testRexxClassicBifD2x`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifD2x.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `ERRORTEXT` | Direct + runtime; admitted baseline audited | LC-STEP-90C/D family receipt; [`testRexxClassicBifErrortext`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifErrortext.crexx); `BIF-AUDIT-MESSAGE` boundary |
| `FORM` | Direct + runtime; admitted baseline audited | [`RexxClassicBifNumeric.crexx`](../../../lib/rxfnsc/RexxClassicBifNumeric.crexx); [`testRexxClassicBifNumeric`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifNumeric.crexx); `BIF-AUDIT-STATE` boundary |
| `FORMAT` | Direct + runtime; admitted baseline audited | [`RexxClassicBifFormat.crexx`](../../../lib/rxfnsc/RexxClassicBifFormat.crexx); [`testRexxClassicBifFormat`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifFormat.crexx); `BIF-AUDIT-NUMERIC` boundary |
| `FUZZ` | Direct + runtime; admitted baseline audited | [`RexxClassicBifNumeric.crexx`](../../../lib/rxfnsc/RexxClassicBifNumeric.crexx); [`testRexxClassicBifNumeric`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifNumeric.crexx); `BIF-AUDIT-STATE` boundary |
| `INSERT` | Direct + runtime; admitted baseline audited | [`RexxClassicBifInsert.crexx`](../../../lib/rxfnsc/RexxClassicBifInsert.crexx); [`testRexxClassicBifInsert`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifInsert.crexx); `BIF-AUDIT-TEXT` boundary |
| `LASTPOS` | Direct + runtime; admitted baseline audited | [`RexxClassicBifLastpos.crexx`](../../../lib/rxfnsc/RexxClassicBifLastpos.crexx); [`testRexxClassicBifLastpos`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifLastpos.crexx); `BIF-AUDIT-TEXT` boundary |
| `LEFT` | Direct + runtime; admitted baseline audited | [`RexxClassicBifLeft.crexx`](../../../lib/rxfnsc/RexxClassicBifLeft.crexx); [`testRexxClassicBifLeft`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifLeft.crexx); `BIF-AUDIT-TEXT` boundary |
| `LENGTH` | Direct + runtime; admitted baseline audited | [`levelc_slice3_bif_length`](../../../compiler/tests/rexx_src/levelc_slice3_bif_length.rexx); [`testRexxClassicBifLength`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifLength.crexx); `BIF-AUDIT-TEXT` boundary |
| `LINEIN` | Deferred; no direct entry | `BIF-AUDIT-STREAM`: LC-STEP-90B; implementation and complete stream contract await Adrian’s architecture/compatibility assessment |
| `LINEOUT` | Deferred; no direct entry | `BIF-AUDIT-STREAM`: LC-STEP-90B; implementation and complete stream contract await Adrian’s architecture/compatibility assessment |
| `LINES` | Deferred; no direct entry | `BIF-AUDIT-STREAM`: LC-STEP-90B; implementation and complete stream contract await Adrian’s architecture/compatibility assessment |
| `MAX` | Direct + runtime; admitted baseline audited | [`RexxClassicBifMax.crexx`](../../../lib/rxfnsc/RexxClassicBifMax.crexx); [`testRexxClassicBifMax`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifMax.crexx); `BIF-AUDIT-NUMERIC` boundary |
| `MIN` | Direct + runtime; admitted baseline audited | [`RexxClassicBifMin.crexx`](../../../lib/rxfnsc/RexxClassicBifMin.crexx); [`testRexxClassicBifMin`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifMin.crexx); `BIF-AUDIT-NUMERIC` boundary |
| `OVERLAY` | Direct + runtime; admitted baseline audited | [`RexxClassicBifOverlay.crexx`](../../../lib/rxfnsc/RexxClassicBifOverlay.crexx); [`testRexxClassicBifOverlay`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifOverlay.crexx); `BIF-AUDIT-TEXT` boundary |
| `POS` | Direct + runtime; admitted baseline audited | [`RexxClassicBifPos.crexx`](../../../lib/rxfnsc/RexxClassicBifPos.crexx); [`testRexxClassicBifPos`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifPos.crexx); `BIF-AUDIT-TEXT` boundary |
| `QUALIFY` | Deferred; no direct entry | `BIF-AUDIT-STREAM`: LC-STEP-90B; implementation and complete stream contract await Adrian’s architecture/compatibility assessment |
| `QUEUED` | Direct + runtime; admitted baseline audited | LC-STEP-90C/D family receipt; [`testRexxClassicBifQueued`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifQueued.crexx); `BIF-AUDIT-QUEUE` boundary |
| `RANDOM` | Direct + runtime; admitted baseline audited | [`RexxClassicBifRandom.crexx`](../../../lib/rxfnsc/RexxClassicBifRandom.crexx); [`testRexxClassicBifRandom`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifRandom.crexx); `BIF-AUDIT-STATE` boundary |
| `REVERSE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifReverse.crexx`](../../../lib/rxfnsc/RexxClassicBifReverse.crexx); [`testRexxClassicBifReverse`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifReverse.crexx); `BIF-AUDIT-TEXT` boundary |
| `RIGHT` | Direct + runtime; admitted baseline audited | [`RexxClassicBifRight.crexx`](../../../lib/rxfnsc/RexxClassicBifRight.crexx); [`testRexxClassicBifRight`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifRight.crexx); `BIF-AUDIT-TEXT` boundary |
| `SIGN` | Direct + runtime; admitted baseline audited | [`RexxClassicBifSign.crexx`](../../../lib/rxfnsc/RexxClassicBifSign.crexx); [`testRexxClassicBifSign`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifSign.crexx); `BIF-AUDIT-NUMERIC` boundary |
| `SOURCELINE` | Direct + runtime; admitted baseline audited | LC-STEP-90C/D family receipt; [`testRexxClassicBifSourceline`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifSourceline.crexx); `BIF-AUDIT-SOURCE` boundary |
| `SPACE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifSpace.crexx`](../../../lib/rxfnsc/RexxClassicBifSpace.crexx); [`testRexxClassicBifSpace`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifSpace.crexx); `BIF-AUDIT-TEXT` boundary |
| `STREAM` | Deferred; no direct entry | `BIF-AUDIT-STREAM`: LC-STEP-90B; implementation and complete stream contract await Adrian’s architecture/compatibility assessment |
| `STRIP` | Direct + runtime; admitted baseline audited | [`RexxClassicBifStrip.crexx`](../../../lib/rxfnsc/RexxClassicBifStrip.crexx); [`testRexxClassicBifStrip`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifStrip.crexx); `BIF-AUDIT-TEXT` boundary |
| `SUBSTR` | Direct + runtime; admitted baseline audited | [`levelc_slice4_bif_substr`](../../../compiler/tests/rexx_src/levelc_slice4_bif_substr.rexx); [`testRexxClassicBifSubstr`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifSubstr.crexx); `BIF-AUDIT-TEXT` boundary |
| `SUBWORD` | Direct + runtime; admitted baseline audited | [`RexxClassicBifSubword.crexx`](../../../lib/rxfnsc/RexxClassicBifSubword.crexx); [`testRexxClassicBifSubword`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifSubword.crexx); `BIF-AUDIT-TEXT` boundary |
| `SYMBOL` | Direct + runtime; admitted baseline audited | [`RexxClassicBifSymbol.crexx`](../../../lib/rxfnsc/RexxClassicBifSymbol.crexx); [`testRexxClassicBifSymbol`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifSymbol.crexx); `BIF-AUDIT-POOL` boundary |
| `TIME` | Direct + runtime; admitted baseline audited | [`RexxClassicBifTime.crexx`](../../../lib/rxfnsc/RexxClassicBifTime.crexx); [`testRexxClassicBifTime`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifTime.crexx); `BIF-AUDIT-STATE` boundary |
| `TRACE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifTrace.crexx`](../../../lib/rxfnsc/RexxClassicBifTrace.crexx); [`testRexxClassicBifTrace`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifTrace.crexx); `BIF-AUDIT-STATE` boundary |
| `TRANSLATE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifTranslate.crexx`](../../../lib/rxfnsc/RexxClassicBifTranslate.crexx); [`testRexxClassicBifTranslate`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifTranslate.crexx); `BIF-AUDIT-TEXT` boundary |
| `TRUNC` | Direct + runtime; admitted baseline audited | [`RexxClassicBifTrunc.crexx`](../../../lib/rxfnsc/RexxClassicBifTrunc.crexx); [`testRexxClassicBifTrunc`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifTrunc.crexx); `BIF-AUDIT-NUMERIC` boundary |
| `VALUE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifValue.crexx`](../../../lib/rxfnsc/RexxClassicBifValue.crexx); [`testRexxClassicBifValue`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifValue.crexx); `BIF-AUDIT-POOL` boundary |
| `VERIFY` | Direct + runtime; admitted baseline audited | [`RexxClassicBifVerify.crexx`](../../../lib/rxfnsc/RexxClassicBifVerify.crexx); [`testRexxClassicBifVerify`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifVerify.crexx); `BIF-AUDIT-TEXT` boundary |
| `WORD` | Direct + runtime; admitted baseline audited | [`RexxClassicBifWord.crexx`](../../../lib/rxfnsc/RexxClassicBifWord.crexx); [`testRexxClassicBifWord`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifWord.crexx); `BIF-AUDIT-TEXT` boundary |
| `WORDINDEX` | Direct + runtime; admitted baseline audited | [`RexxClassicBifWordindex.crexx`](../../../lib/rxfnsc/RexxClassicBifWordindex.crexx); [`testRexxClassicBifWordindex`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifWordindex.crexx); `BIF-AUDIT-TEXT` boundary |
| `WORDLENGTH` | Direct + runtime; admitted baseline audited | [`RexxClassicBifWordlength.crexx`](../../../lib/rxfnsc/RexxClassicBifWordlength.crexx); [`testRexxClassicBifWordlength`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifWordlength.crexx); `BIF-AUDIT-TEXT` boundary |
| `WORDPOS` | Direct + runtime; admitted baseline audited | [`RexxClassicBifWordpos.crexx`](../../../lib/rxfnsc/RexxClassicBifWordpos.crexx); [`testRexxClassicBifWordpos`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifWordpos.crexx); `BIF-AUDIT-TEXT` boundary |
| `WORDS` | Direct + runtime; admitted baseline audited | [`RexxClassicBifWords.crexx`](../../../lib/rxfnsc/RexxClassicBifWords.crexx); [`testRexxClassicBifWords`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifWords.crexx); `BIF-AUDIT-TEXT` boundary |
| `XRANGE` | Direct + runtime; admitted baseline audited | [`RexxClassicBifXrange.crexx`](../../../lib/rxfnsc/RexxClassicBifXrange.crexx); [`testRexxClassicBifXrange`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifXrange.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `X2B` | Direct + runtime; admitted baseline audited | [`RexxClassicBifX2b.crexx`](../../../lib/rxfnsc/RexxClassicBifX2b.crexx); [`testRexxClassicBifX2b`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifX2b.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `X2C` | Direct + runtime; admitted baseline audited | [`RexxClassicBifX2c.crexx`](../../../lib/rxfnsc/RexxClassicBifX2c.crexx); [`testRexxClassicBifX2c`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifX2c.crexx); `BIF-AUDIT-ORDINAL` boundary |
| `X2D` | Direct + runtime; admitted baseline audited | [`RexxClassicBifX2d.crexx`](../../../lib/rxfnsc/RexxClassicBifX2d.crexx); [`testRexxClassicBifX2d`](../../../lib/rxfnsc/tests_functional/testRexxClassicBifX2d.crexx); `BIF-AUDIT-ORDINAL` boundary |

## Increment receipts

### LC-STEP-01 — initial inventory, 2026-10-03

- First-pass category matrix and all 70 recognized BIF names recorded in
  commit `941a8469c`. Fifty-three BIF names have standalone Classic modules;
  source recognition and module presence are deliberately not conformance
  claims. `LC-AC-01` remains open pending individual non-BIF rows and complete
  reference reconciliation.
- A second inventory pass records 36 syntax contracts and 74 detailed
  non-BIF reference obligations with explicit BYTE/UTF8/host/source scope.
  Current implementation states remain conservative; the final crosswalk,
  feature-level qualification and exception decisions remain open.

### LC-STEP-02 — bounded IF, 2026-10-03

- Regina reference run of `levelc_slice7_if_nested.rexx` produced
  `true`, `false`, `nested`, `after`, `once`, `1`, `1` in order. `IF 2`
  produced `34.1`; the compiled path now rejects it with `RXC-LC-34.1`.
- Release: 8/8 initial slice-7 focused tests, 94/94 Level C tests, 2/2
  `testRexxValue` opt/no-opt; the subsequently added generic logical-value
  negative test passed 1/1 on unchanged code. Retained logs:
  `/tmp/crexx-levelc-if-ctest.JsMsZT`,
  `/tmp/crexx-levelc-if-suite.24iyqv`,
  `/tmp/crexx-levelc-if-final-tests.k7ROaa`.
- Debug: 95/95 Level C tests and 2/2 `testRexxValue` opt/no-opt on the same
  source change. Retained log: `/tmp/crexx-levelc-if-final-tests.k7ROaa`.
- Release `rxlink` of the compiled nested fixture ran through `rxvm` and
  produced the same seven output lines. Retained log:
  `/tmp/crexx-levelc-if-link.Y5Cdut`.
- `RexxValue` now rejects values other than strict `0`/`1` for the shared
  logical helper. The `IF` path reports `34.1`; contextual `34.5/34.6`
  identities for logical-operator operands remain open in the full matrix.
  Regina accepts some padded/leading-zero spellings that the extracted Classic
  reference treats as invalid; those interpreter-specific cases are not used
  as conformance evidence for this slice.

### LC-STEP-03 — simple DO, 2026-10-03

- Regina and the compiled nested fixture both produced `before`, `nested`,
  `2`, `2`, `arm`, `3` in order. The fixture covers body order, nesting,
  `DO` in an `IF` arm, `IF` in a `DO` body, an empty group, and a local
  procedure group. The canonical target is a one-shot `DO 1` block.
- Release 8/8 focused slice-8 tests and 103/103 Level C suite; Debug 103/103
  Level C suite. The focused cases include opt/no-opt, tree shape, a counted
  `DO` rejection and an unsupported-body rejection. Retained logs:
  `/tmp/crexx-levelc-do-focused.OA1FOA`,
  `/tmp/crexx-levelc-do-release-suite.XnH4dK`,
  `/tmp/crexx-levelc-do-debug-suite.Pju44v`.
- Release `rxlink` of the compiled nested fixture ran through `rxvm` with
  the same six output lines. Retained log:
  `/tmp/crexx-levelc-do-link.gYjrFs`.
- Controlled/repetitive `DO`, `WHILE`/`UNTIL`, `LEAVE`/`ITERATE` and their
  state/condition behavior remain open. `LC-AC-01/04` and `R1-AC-01/02`
  remain open; this increment does not establish full Classic compatibility.

### LC-STEP-06 — childless NOP, 2026-10-03

- Regina and the optimized/no-opt compiled fixture each produced `before`,
  `middle`, `after`. The fixture places `NOP` in main, a local procedure,
  both selected `IF` arms and nested `DO` bodies; adjacent statements retain
  their order.
- Release toolchain build passed; focused slice-9 tests passed 2/2 and the
  normal Level C suite passed 105/105. Retained logs:
  `/tmp/crexx-levelc-nop-build.bO3BoM`,
  `/tmp/crexx-levelc-nop-tests.iKKtAb`,
  `/tmp/crexx-levelc-nop-suite.HEoZ0w`.
- `rxlink` of the optimized RXBIN ran through `rxvm` with the same three
  lines. Retained log: `/tmp/crexx-levelc-nop-link.73Ab9Z`.
- `NOP` source/TRACE lifecycle and full profile/platform qualification remain
  open under `LC-AC-04`; this is a bounded execution slice.

### LC-STEP-07 — shared architecture reconciliation, 2026-10-03

- The current Level C architecture design now identifies the guarded compiler
  path, common `rxfnsc` value/pool/BIF modules, and separate RexxScript
  evaluator and sandbox pool. This records Adrian's shared-foundation
  direction without treating the two languages as identical.
- The existing `RexxScriptEvaluator` uses `RexxVariablePool.setString()` for
  sandbox variables and `RexxValue` frames for shared BIF calls; Level C
  lowering uses the same library from compiled code. The required behavioral
  and isolation proof remains open under `LC-AC-06` and `LC-STEP-08`.

### LC-STEP-09 — scalar pool read and direct DROP, 2026-10-03

- The retained before-change reproducer showed Regina output `MISSING`,
  while compiled Level C printed a blank line for an unset scalar. A separate
  `DROP a` fixture printed `A` under Regina but was rejected by the compiler.
  Before-change log: `/tmp/crexx-levelc-pool-before.zIeQRx`.
- Scalar reads now use `RexxVariablePool.symbolValue()` rather than `value()`;
  guarded direct scalar lists lower to the pool's `drop()` method. The
  nine-line fixture matches Regina in main, nested `IF`/`DO`, and an exposed
  procedure pool, with optimized/no-opt parity. Indirect, stem and compound
  `DROP` remain fail-closed.
- Release core/runtime build passed, focused tests 6/6, and the selected Level
  C, pool and RexxScript correctness suite 115/115. The linked fixture matched
  Regina through `rxvm`. Retained logs:
  `/tmp/crexx-levelc-drop-build.2mXMyN`,
  `/tmp/crexx-levelc-drop-consumers-build.AVcSFk`,
  `/tmp/crexx-levelc-drop-focused.vvOvKu`,
  `/tmp/crexx-levelc-drop-suite.EB7iFh`,
  `/tmp/crexx-levelc-drop-linked-run.YcStKq`.
- Compound/stem reads and `DROP` variants, NOVALUE trap delivery, BYTE/UTF8
  profiles and complete cross-consumer semantics remain open under
  `LC-AC-01/04/06`.

### LC-STEP-10 — AST boundary and structural crosswalk, 2026-10-03

- The crosswalk above maps the parser's structural families to accepted slices
  and open lowering obligations. It keeps `SELECT`, controlled DO, PARSE,
  condition branches, and host/queue shapes visible for early AST closure.
- Accepted lowering now checks the active tree for missing root, wrong parent,
  sibling cycles and residual `LEVELC_*` nodes before canonical passes. The
  direct unit test rejects each malformed case and accepts the restored tree.
- Release `rxc` and unit-test build passed. The earlier nested/tree-shape
  focused set passed 8/8, the selected Level C suite passed 111/111, and a
  redirected `rxc -d2` probe found `STAGE_LEVELC_LOWERED` without AST errors.
  After the final unit-test wiring, `levelc_*` tests passed 61/61, including
  the direct negative unit. Retained logs:
  `/tmp/crexx-levelc-tree-build.cVVuMD`,
  `/tmp/crexx-levelc-tree-focused.ywIWWn`,
  `/tmp/crexx-levelc-tree-debug.8M7ww7`,
  `/tmp/crexx-levelc-tree-suite.awW5rx`,
  `/tmp/crexx-levelc-tree-build-confirm.q84koC`,
  `/tmp/crexx-levelc-tree-final-suite.aHpaIM`.
- `LC-AC-08` remains open: the verifier is a boundary invariant, while
  generated scope/symbols, evaluation order, source anchors and the remaining
  parser families still need implementation and evidence.

### LC-STEP-11 — bounded SELECT/WHEN/OTHERWISE, 2026-10-03

- Before-change, a simple SELECT printed `zero` in Regina and failed compiled
  Level C as an unsupported main statement. The raw parsed tree is `SELECT >
  INSTRUCTIONS > WHEN... [OTHERWISE]`; each WHEN has condition and body, while
  OTHERWISE has an optional inline body plus an `INSTRUCTIONS` list. Retained
  reproducer and AST logs:
  `/tmp/crexx-levelc-select-before.npooyz`,
  `/tmp/crexx-levelc-select-ast.HWxpA1`.
- The lowerer validates that shape, builds nested canonical `IF` branches in
  reverse order, and places each later condition inside the preceding ELSE
  block. `RexxValue.logicalWhenValue()` reports `RXC-LC-34.2`; the unmatched
  path reports `RXC-LC-7.3` with the SELECT source line. Regina reports base
  error `7` for this unmatched case; the compliance reference specifies `7.3`
  and is the current implementation target.
- The seven-line nested fixture matched Regina with optimized/no-opt parity.
  Release core and `rxfnsc` builds passed, five focused SELECT tests passed,
  and the selected Level C, `RexxValue` and RexxScript correctness suite passed
  127/127. A redirected debug probe showed source-anchored generated WHEN
  conditions and no AST validation error. `rxlink` plus `rxvm` reproduced the
  same seven lines. Retained logs:
  `/tmp/crexx-levelc-select-final-build.7BDc9R`,
  `/tmp/crexx-levelc-select-focused.X6HxHI`,
  `/tmp/crexx-levelc-select-suite.xQepez`,
  `/tmp/crexx-levelc-select-debug.jZHZB4`,
  `/tmp/crexx-levelc-select-link.PrvySs`.
- `LC-AC-08` and `LC-AC-04` remain open for the other structural and complete
  compatibility obligations.

### LC-STEP-13 — literal counted DO, 2026-10-03

- Before-change, Regina executed `DO 2` twice while compiled Level C rejected
  the parser's `DO > REPEAT > FOR > INTEGER` header. The reproducer and raw
  AST are retained at `/tmp/crexx-levelc-count-before.CVPYiy`.
- A neutral counted-DO builder now receives a fresh canonical count node.
  The Level C guard accepts only a non-negative decimal integer literal
  representable by the current `.int` builder, with no extra repetition or
  condition sibling. All other loop headers remain fail-closed; the prior
  unsupported fixture now covers a control-variable loop.
- The eleven-line zero/nested/IF/procedure fixture matched Regina in optimized
  and no-opt runs. Release build passed, focused tests 5/5, normal Level C and
  source-provenance suite 124/124, and linked RXBIN execution reproduced the
  eleven lines. Debug tree output showed source-anchored generated loops and
  no AST validation failure. Retained logs:
  `/tmp/crexx-levelc-count-build.913mV7`,
  `/tmp/crexx-levelc-count-focused.OBuaIY`,
  `/tmp/crexx-levelc-count-suite.zVJrGT`,
  `/tmp/crexx-levelc-count-debug.uso2v0`,
  `/tmp/crexx-levelc-count-link.6iwRyj`.
- Dynamic repetition counts, controlled loops, WHILE/UNTIL, LEAVE/ITERATE and
  their exact runtime errors remain open under `LC-AC-08/04`.
- A focused Debug build passed. The first Debug CTest attempt had one `Not
  Run` because `test_levelc_tree_boundary` had not been built in that tree;
  after building it, the focused Debug set passed 6/6. Logs:
  `/tmp/crexx-levelc-count-debug-build.wX76xJ`,
  `/tmp/crexx-levelc-count-debug-focused.9ijG98`,
  `/tmp/crexx-levelc-count-debug-unit-build.QGU5mL`,
  `/tmp/crexx-levelc-count-debug-focused-final.VkqpEj`.

### LC-STEP-14 — loop transfer across generated blocks, 2026-10-03

- The retained counterexample shows Regina leaving the source counted loop
  once, while an equivalent canonical tree with a bare `LEAVE` inside a
  generated `DO 1` branch only leaves that inner wrapper and continues the
  outer loop. This is the AST association risk Adrian prioritized. Reproducer:
  `/tmp/crexx-levelc-loop-association.Ecw4Bg`.
- Accepted literal counted loops now use a hidden canonical control symbol.
  Source childless `LEAVE` and `ITERATE` lower to named transfers to that
  symbol; existing canonical validation resolves the correct enclosing loop
  through generated wrappers. `ast_do()` and global association semantics are
  unchanged. Named Classic transfers remain outside this slice.
- The eight-line fixture matched Regina in optimized/no-opt and linked runs,
  covering IF, SELECT, nested simple/counted DO and a local procedure. A
  redirected tree probe showed source-anchored transfers and hidden typed
  control symbols without AST validation errors. Release build passed,
  focused transfer/count tests 6/6 plus a named-transfer diagnostic test,
  the selected normal Level C/source-provenance suite 128/128, and Debug
  focused tests 8/8. Retained logs:
  `/tmp/crexx-levelc-transfer-build.M5xxeb`,
  `/tmp/crexx-levelc-transfer-focused.518e63`,
  `/tmp/crexx-levelc-transfer-suite.BI7lhP`,
  `/tmp/crexx-levelc-transfer-debug-tree.3bjoKi`,
  `/tmp/crexx-levelc-transfer-link.RQWxpa`,
  `/tmp/crexx-levelc-transfer-debug-build.xUyrVm`,
  `/tmp/crexx-levelc-transfer-debug-focused.3t20z1`.
- `LC-AC-08/04` remain open for the other structural and full compatibility
  obligations; dynamic/controlled loops and named transfers are still open.

### LC-STEP-15 — DO FOREVER, 2026-10-03

- Before-change, Regina completed a guarded `DO FOREVER` example with output
  `3`, while Level C rejected its `DO > REPEAT("forever") > INSTRUCTIONS`
  shape. The raw-tree and reference receipts are retained at
  `/tmp/crexx-levelc-forever-before.S2g0km`.
- The guarded source shape now uses the existing hidden canonical loop symbol
  and named transfer mapping, with no FOR count. Unsupported extra header
  conditions remain fail-closed. The fixture has explicit termination guards
  so a wrong LEAVE target fails quickly instead of hanging a test runner.
- Regina and optimized/no-opt compiled execution matched all eight lines,
  including nested simple/SELECT/counted groups and a local procedure. Release
  build and focused tests 7/7 passed; the normal Level C/source-provenance
  suite passed 131/131. Debug focused tests passed 8/8. Linked execution
  matched Regina, and the redirected tree probe showed source-anchored
  transfers with no AST validation errors. Retained logs:
  `/tmp/crexx-levelc-forever-build.EXyQ2y`,
  `/tmp/crexx-levelc-forever-focused.EiyMdU`,
  `/tmp/crexx-levelc-forever-suite.t6MpZR`,
  `/tmp/crexx-levelc-forever-link.K1bR9s`,
  `/tmp/crexx-levelc-forever-tree.QvkEh6`,
  `/tmp/crexx-levelc-forever-debug-build.rEPkRy`,
  `/tmp/crexx-levelc-forever-debug-focused.pwsTd1`.
- `LC-AC-08/04` remain open for dynamic and controlled counts, WHILE/UNTIL,
  named transfers and other structural families.

### LC-STEP-16 — bounded DO WHILE, 2026-10-03

- The source `DO > WHILE > INSTRUCTIONS` shape lowers to a canonical
  controlled loop with WHILE under REPEAT. The condition stays on the loop
  check path and is evaluated before each iteration, including the first.
  The validator admits only supported expressions that emit no setup
  statements; the lowerer asserts that invariant again. Short-circuit,
  function and compound-tail condition forms remain fail-closed.
- Childless LEAVE/ITERATE reuse the source-loop hidden target from STEP-14.
  The shared `RexxValue.logicalWhileValue()` supplies exact `34.3` errors;
  no new AST node type was needed. Adrian permits a new type where the full
  validation-to-emitter path is simpler and more supportable.
- Regina and optimized/no-opt compiled output matched for zero-entry,
  condition reevaluation, nested wrappers, counted nesting and a local
  procedure. Release focused tests passed 5/5, selected normal Level C and
  source-provenance tests 136/136, shared RexxValue/RexxScript tests 8/8,
  and Debug structural/loop focused tests 13/13. Linked RXBIN output matched
  Regina. A redirected tree probe showed WHILE and logical calls anchored to
  source lines with no AST validation diagnostic. Retained logs:
  `/tmp/crexx-levelc-while-build.JsRHRZ`,
  `/tmp/crexx-levelc-while-focused.FPR0uT`,
  `/tmp/crexx-levelc-while-suite.pljBeV`,
  `/tmp/crexx-levelc-while-shared.1gClVb`,
  `/tmp/crexx-levelc-while-debug-build.NT8SKe`,
  `/tmp/crexx-levelc-while-debug-runtime.8bBCRi`,
  `/tmp/crexx-levelc-while-debug-focused.EBYGJh`,
  `/tmp/crexx-levelc-while-link-log.Yauzdi`, and
  `/tmp/crexx-levelc-while-tree.0MmH0y`.
- The final comment-only source tidy rebuilt Release and Debug `rxc`; the
  combined normal Level C/source-provenance/shared-runtime selection passed
  144/144, and Debug structural/loop focused tests passed 13/13. Final logs:
  `/tmp/crexx-levelc-while-release-final-build.JnHXOE`,
  `/tmp/crexx-levelc-while-debug-final-build.v4z9zZ`,
  `/tmp/crexx-levelc-while-release-final-tests.TYllNI`, and
  `/tmp/crexx-levelc-while-debug-final-tests.E0en89`.
- `LC-AC-08/04` remain open for UNTIL, setup-bearing conditions and the
  other structural and full compatibility families.

### LC-STEP-17 — bounded DO UNTIL, 2026-10-03

- Source `DO > UNTIL > INSTRUCTIONS` now lowers through the existing
  canonical end-check node under REPEAT. The condition executes after the
  first body and on ITERATE. The first Regina fixture mistakenly kept the
  condition false after changing its variable; it was corrected to become
  true after one iteration, and subsequent reference/VM runs used bounded
  output and time limits.
- The setup-free guard and hidden source-loop target are shared with WHILE;
  shared `RexxValue.logicalUntilValue()` reports `34.4`. Combined headers
  and setup-bearing expressions remain fail-closed. No new AST node type was
  needed for the existing end-check contract.
- Regina and optimized/no-opt compiler output matched for first-body
  execution, post-body checks, ITERATE through a generated DO, LEAVE through
  SELECT, nested counted loops and a local procedure. Release focused tests
  passed 6/6 and the selected normal Level C/source-provenance/shared-runtime
  suite passed 150/150. Debug structural/loop focused tests passed 19/19.
  Linked RXBIN output matched Regina; the redirected tree probe showed
  source-anchored UNTIL checks with no AST validation diagnostic. Logs:
  `/tmp/crexx-levelc-until-reference-output.2iuZg3`,
  `/tmp/crexx-levelc-until-release-build.ulMbQg`,
  `/tmp/crexx-levelc-until-direct-log.ySsHY7`,
  `/tmp/crexx-levelc-until-focused.F6YTOR`,
  `/tmp/crexx-levelc-until-suite.XL1FOh`,
  `/tmp/crexx-levelc-until-debug-build.vq3DBg`,
  `/tmp/crexx-levelc-until-debug-focused.NlTVt0`,
  `/tmp/crexx-levelc-until-link.3yt1BJ`, and
  `/tmp/crexx-levelc-until-tree.ZF35LG`.
- `LC-AC-08/04` remain open for setup-bearing conditions, combined and
  controlled loops, named transfers and the other structural families.

### LC-STEP-18 — literal count with WHILE/UNTIL, 2026-10-03

- The parser's `REPEAT` followed by WHILE or UNTIL now lowers when its FOR
  expression is a non-negative integer literal and its condition is setup
  free. The neutral controlled-loop builder places FOR and the condition in
  one canonical REPEAT; the source-loop hidden target continues to bind
  childless LEAVE/ITERATE across generated IF/DO/SELECT wrappers. Dynamic
  counts, FOREVER with a condition and setup-bearing conditions remain
  fail-closed. The prior negative combined-header fixture was removed when
  that exact form became executable, with new negative cases for the
  remaining unsupported headers.
- Regina and compiled opt/no-opt output matched for count-limited WHILE,
  UNTIL after-body checks, zero counts that skip nonlogical conditions,
  ITERATE, LEAVE through SELECT and a local procedure. Release focused tests
  passed 8/8 and the selected normal Level C/source-provenance/shared-runtime
  suite passed 157/157. Debug structural/loop focused tests passed 26/26.
  Linked RXBIN output matched Regina. The redirected tree probe showed FOR
  and WHILE/UNTIL under each accepted canonical REPEAT, with source lines and
  no AST validation diagnostic. Logs:
  `/tmp/crexx-levelc-combined-reference-output.dulkdk`,
  `/tmp/crexx-levelc-combined-release-build.sxX4Up`,
  `/tmp/crexx-levelc-combined-release-final-build.B7WfWR`,
  `/tmp/crexx-levelc-combined-direct-log.5H8bnC`,
  `/tmp/crexx-levelc-combined-release-final-focused.ihtUU8`,
  `/tmp/crexx-levelc-combined-suite.CAS8Ce`,
  `/tmp/crexx-levelc-combined-debug-build.ikZRTX`,
  `/tmp/crexx-levelc-combined-debug-focused.XHy7K8`,
  `/tmp/crexx-levelc-combined-link.Nym16W`, and
  `/tmp/crexx-levelc-combined-tree.ISdWvo`.
- `LC-AC-08/04` remain open for dynamic/controlled and setup-bearing loop
  headers, named transfers and the other structural families.

### LC-STEP-19 — eager Classic logical operands, 2026-10-03

- A local Regina side-effect reproducer and the
  [official ooRexx reference](https://www.oorexx.org/docs/pdf/rexxref.pdf)
  confirm that Classic `&` evaluates both operands; `|` follows the same
  source-order binary-operator rule. The prior branch materialisation skipped
  the right operand when the left determined the result. This was a separate
  pre-existing expression defect found while probing WHILE condition setup.
- The Level C lowerer now captures a copy of the left `RexxValue` before
  lowering right setup statements, then calls shared `logicalAnd()` or
  `logicalOr()` with the right value. The methods enforce contextual
  `34.5`/`34.6`. The old expression fixture's side-effect expectations and
  manual Level B target were corrected to the Classic reference; its first
  unchanged run failed in exactly those old expectations before correction.
  The block-expression WHILE work was held in a local stash and remains a
  distinct increment.
- Regina and optimized/no-opt compiled output matched for both operators,
  nested expressions, right-side calls on determining left values and a
  right-side mutation of the left variable. Release focused tests passed 9/9
  after the expected fixture correction; the selected normal
  Level C/source-provenance/shared-runtime suite passed 163/163. Debug focused
  tests passed 16/16. Linked RXBIN output matched Regina. The redirected
  tree probe showed source-anchored left snapshots and `logicalAnd`/
  `logicalOr` calls with no AST validation diagnostic. Logs:
  `/tmp/crexx-levelc-logical-eager-reference-output.72prLS`,
  `/tmp/crexx-levelc-logical-old-reference-output.QJx8aL`,
  `/tmp/crexx-levelc-logical-eager-release-build.WrFSVV`,
  `/tmp/crexx-levelc-logical-eager-focused.MFAkwl`,
  `/tmp/crexx-levelc-logical-eager-focused-final.7LpcTC`,
  `/tmp/crexx-levelc-logical-eager-suite.sdJEh8`,
  `/tmp/crexx-levelc-logical-eager-debug-build.BVglbq`,
  `/tmp/crexx-levelc-logical-eager-debug-focused.Zshigu`,
  `/tmp/crexx-levelc-logical-eager-link-log.VXRbv9`, and
  `/tmp/crexx-levelc-logical-eager-tree.9P77iG`.
- `LC-AC-08/04` remain open for the other expression and structural families.

### LC-STEP-20 — setup-bearing direct DO WHILE, 2026-10-03

- Direct `DO WHILE` accepts supported expressions with lowering setup. The
  neutral builder wraps setup statements and `LEAVE WITH` of the contextual
  logical value in the existing canonical `BLOCK_EXPR`; no new AST node or
  emitter path is needed. Setup-free conditions keep their direct expression
  shape. Direct UNTIL and count-plus-condition headers still reject setup
  because their distinct timing/limit paths need proof.
- A bounded Regina fixture and optimized/no-opt compiled runs matched for
  eager logical side effects, a per-entry SUBSTR BIF argument frame, changing
  compound tails, childless LEAVE/ITERATE through generated wrappers and a
  local procedure. Nonlogical conditions report `34.3`. The previous
  unsupported-WHILE fixture was removed when that form became executable.
  Release focused tests passed 4/4, the selected normal Level C/source-
  provenance/shared-runtime suite passed 166/166, and Debug structural/loop
  tests passed 28/28. Linked RXBIN output matched Regina. The redirected tree
  probe showed source-anchored BLOCK_EXPR, LEAVE_WITH and BIF context nodes
  with no AST validation error. Retained evidence:
  `/tmp/crexx-levelc-while-setup-reference-output.hlkmtT`,
  `/tmp/crexx-levelc-while-block-release-build.MoY4Cx`,
  `/tmp/crexx-levelc-while-block-direct-log.QGCOrX`,
  `/tmp/crexx-levelc-while-block-focused.zMyYeC`,
  `/tmp/crexx-levelc-while-block-release-suite.ui8xzl` (obsolete negative
  fixture, then removed),
  `/tmp/crexx-levelc-while-block-release-reconfigure.WsupOj`,
  `/tmp/crexx-levelc-while-block-release-final-suite.HdrpiK`,
  `/tmp/crexx-levelc-while-block-debug-build.qeDYFh`,
  `/tmp/crexx-levelc-while-block-debug-focused.bo880Q`,
  `/tmp/crexx-levelc-while-block-link.irp1c4`, and
  `/tmp/crexx-levelc-while-block-tree.s1TS6B`.
- `LC-AC-08/04` remain open for setup-bearing UNTIL/combined headers,
  dynamic/controlled loops, named transfers and other structural families.

### LC-STEP-21 — setup-bearing direct DO UNTIL, 2026-10-03

- Direct `DO UNTIL` now admits the same supported setup-bearing condition
  expressions as direct WHILE. The existing canonical `BLOCK_EXPR` places
  condition setup at UNTIL's post-body check, including the ITERATE path;
  setup-free conditions remain direct. Count-plus-condition headers still
  reject setup. No new AST node or emitter shape was needed.
- Bounded Regina and optimized/no-opt compiled output matched for an initially
  true condition checked only after the body, eager side effects after
  ITERATE, per-check SUBSTR BIF frames, changing compound tails, childless
  LEAVE/ITERATE through generated blocks and a local procedure. A nonlogical
  setup-bearing condition reports `34.4`. The obsolete unsupported-UNTIL
  fixture was removed. Release focused tests passed 4/4; the selected normal
  Level C/source-provenance/shared-runtime suite passed 169/169. Debug
  structural/loop tests passed 31/31. Linked RXBIN output matched Regina.
  The redirected tree probe showed source-anchored UNTIL > BLOCK_EXPR >
  LEAVE_WITH and a BIF context, with no AST validation error. Evidence:
  `/tmp/crexx-levelc-until-setup-reference-output.PUMS4G`,
  `/tmp/crexx-levelc-until-block-release-build.C8LELR`,
  `/tmp/crexx-levelc-until-block-direct-log.NsPSk4`,
  `/tmp/crexx-levelc-until-block-focused.tVTwVb`,
  `/tmp/crexx-levelc-until-block-release-suite.eZu6Hq`,
  `/tmp/crexx-levelc-until-block-debug-build.GKVpZm`,
  `/tmp/crexx-levelc-until-block-debug-focused.P0RHvW`,
  `/tmp/crexx-levelc-until-block-link.LrLH8w`, and
  `/tmp/crexx-levelc-until-block-tree.VWYALR`.
- `LC-AC-08/04` remain open for setup-bearing combined headers,
  dynamic/controlled loops, named transfers and other structural families.

### LC-STEP-22 — literal count with setup-bearing WHILE/UNTIL, 2026-10-03

- The literal-count combined headers now reuse the canonical condition
  BLOCK_EXPR builder. The obsolete setup-free expression guard and its
  negative fixture were removed; the literal-count, dynamic-count and
  FOREVER-with-condition guards remain. A zero count skips body and condition
  setup. WHILE does not check after reaching its count limit; UNTIL checks
  after the final body. Both follow Regina's eager side-effect counts.
- A bounded Regina fixture matched optimized/no-opt output for zero-count
  suppression, condition and count limits, BIF argument frames, ITERATE and
  LEAVE through generated blocks, local procedure scope, and `34.3`/`34.4`
  errors. The first fixture exposed independent `LC-FIND-01`; replacing its
  empty comparison operand with supported nonempty operands kept this
  increment scoped. Release focused tests passed 5/5; the selected normal
  Level C/source-provenance/shared-runtime suite passed 173/173. Debug
  structural/loop tests passed 35/35. Linked RXBIN output matched Regina.
  The redirected tree probe showed source-anchored FOR with WHILE/UNTIL
  BLOCK_EXPR and LEAVE_WITH nodes, with no AST validation error. Evidence:
  `/tmp/crexx-levelc-combined-setup-reference-output-v2.w5TaoW`,
  `/tmp/crexx-levelc-combined-block-release-build.73av2a`,
  `/tmp/crexx-levelc-combined-block-direct-log-v2.7S7eH9`,
  `/tmp/crexx-levelc-combined-block-focused.Ni1Hdp`,
  `/tmp/crexx-levelc-combined-block-release-suite.56iMw2`,
  `/tmp/crexx-levelc-combined-block-debug-build.kDTS4J`,
  `/tmp/crexx-levelc-combined-block-debug-focused.0HdBZ9`,
  `/tmp/crexx-levelc-combined-block-link.kpLZoK`, and
  `/tmp/crexx-levelc-combined-block-tree.MzWV5G`.
- `LC-AC-08/04` remain open for dynamic/controlled loop headers, named
  transfers, numeric errors and the other structural families.

### LC-STEP-23 — DO FOREVER WHILE/UNTIL, 2026-10-03

- The parsed childless FOREVER REPEAT can now carry WHILE or UNTIL. The
  existing neutral controlled-loop builder omits FOR, reuses the condition
  BLOCK_EXPR when setup is needed, and preserves the hidden source loop
  target. The obsolete unsupported FOREVER-condition fixture was removed.
- Bounded Regina and optimized/no-opt compiled output matched for zero-entry
  WHILE, post-body UNTIL, eager side effects, per-check BIF frames,
  LEAVE/ITERATE through generated blocks, a local procedure, and contextual
  `34.3`/`34.4` errors. Release focused tests passed 5/5; the selected normal
  Level C/source-provenance/shared-runtime suite passed 177/177. Debug
  structural/loop tests passed 42/42. Linked RXBIN output matched Regina.
  The redirected tree probe showed source-anchored REPEAT with WHILE/UNTIL
  BLOCK_EXPR and LEAVE_WITH nodes, and no FOR child on the source FOREVER
  loops. Evidence:
  `/tmp/crexx-levelc-forever-condition-reference-output.rCALZY`,
  `/tmp/crexx-levelc-forever-block-release-build.OHiWpe`,
  `/tmp/crexx-levelc-forever-block-direct-log.Tfs2mU`,
  `/tmp/crexx-levelc-forever-block-focused.vU9vI7`,
  `/tmp/crexx-levelc-forever-block-release-suite.VzhBl8`,
  `/tmp/crexx-levelc-forever-block-debug-build.Desvx2`,
  `/tmp/crexx-levelc-forever-block-debug-focused.4md9Cz`,
  `/tmp/crexx-levelc-forever-block-link.Apjgj4`, and
  `/tmp/crexx-levelc-forever-block-tree.9eUean`.
- `LC-AC-08/04` remain open for dynamic/controlled headers, named
  transfers, numeric errors and the other structural families. `LC-FIND-01`
  remains open independently.

### LC-STEP-24 — bounded dynamic direct DO count, 2026-10-03

- Direct `DO expression` now lowers supported count expressions through
  `RexxValue.repeatCountValue()` into canonical FOR. Setup-bearing expressions
  use the same neutral BLOCK_EXPR and LEAVE_WITH builder as conditions so
  their setup runs once at loop entry. The bounded conversion uses whole
  number validation and decimal materialization before integer conversion;
  values outside signed 32-bit range report contextual `26.2`. Dynamic count
  plus WHILE/UNTIL and controlled DO remain fail-closed. The former negative
  literal compile fixture is now a `26.2` runtime regression.
- A bounded Regina fixture matched optimized/no-opt output for scalar counts
  held despite body mutation, zero entry, `2.0`/`2E0`, one local-call side
  effect, BIF setup under FOR, childless LEAVE/ITERATE and procedure scope.
  The first direct run exposed an unintended `CONVERSION_ERROR` for `2.0`
  from `asInt()`; the decimal conversion repair passed before wider testing.
  Invalid text, fraction, negative and over-range values report `26.2`,
  including a large positive exponent. Regina reports Error 33 for a
  negative repetition expression; this slice follows the repository's
  standard `26.2` catalog pending full diagnostic reconciliation under
  `LC-AC-04`.
- Release focused tests passed 9/9 after the tree fixture gained a SUBSTR
  count that actually emits setup. The selected normal
  Level C/source-provenance/shared-runtime suite passed 183/183; the later
  added large-exponent test passed separately 1/1 without changing product
  code. Debug structural/loop and shared-value tests passed 59/59. Linked
  RXBIN output matched Regina. The redirected tree probe showed a
  source-anchored FOR > BLOCK_EXPR > LEAVE_WITH > repeatCountValue path for
  SUBSTR, with no AST validation error. Evidence:
  `/tmp/crexx-levelc-dynamic-count-reference-output-v2.ifBks8`,
  `/tmp/crexx-levelc-dynamic-count-release-build.i3tNX6`,
  `/tmp/crexx-levelc-dynamic-count-direct-log.7ys7ae` (initial failure),
  `/tmp/crexx-levelc-dynamic-count-release-rebuild.cfDcwu`,
  `/tmp/crexx-levelc-dynamic-count-direct-log-v3.fBdb6W`,
  `/tmp/crexx-levelc-dynamic-count-focused-final.lT4NK1`,
  `/tmp/crexx-levelc-dynamic-count-release-suite.4R6vgr`,
  `/tmp/crexx-levelc-dynamic-count-exponent-focused.RinOfO`,
  `/tmp/crexx-levelc-dynamic-count-debug-build.ZVUWtV`,
  `/tmp/crexx-levelc-dynamic-count-debug-focused.ayKd6Z`,
  `/tmp/crexx-levelc-dynamic-count-link.BbnPLj`, and
  `/tmp/crexx-levelc-dynamic-count-tree.JvWOu8`.
- Dynamic repetition counts now combine with supported WHILE/UNTIL conditions
  under one canonical REPEAT. Count conversion and any BIF/local-call setup
  run once under FOR; a zero count skips the condition and its setup. WHILE
  checks at entry, UNTIL after the body, including the count-limited final
  iteration. The bounded Regina fixture matched optimized/no-opt and linked
  RXBIN output. Release focused tests passed 6/6 and the selected normal
  Level C/source-provenance/shared-runtime suite passed 189/189. Debug
  structural/loop and shared-value tests passed 64/64. The redirected tree
  probe showed source-anchored FOR > repeatCountValue and independent
  WHILE/UNTIL > BLOCK_EXPR > LEAVE_WITH paths, with no AST validation error.
  Evidence: `/tmp/crexx-levelc-dynamic-combined-reference-output.UG88oS`,
  `/tmp/crexx-levelc-dynamic-combined-release-build.v9hJq7`,
  `/tmp/crexx-levelc-dynamic-combined-direct-log.jJs2jq`,
  `/tmp/crexx-levelc-dynamic-combined-focused.CrPv4i`,
  `/tmp/crexx-levelc-dynamic-combined-release-suite.eeyvBk`,
  `/tmp/crexx-levelc-dynamic-combined-debug-build.eBz0yG`,
  `/tmp/crexx-levelc-dynamic-combined-debug-focused.R4FHmc`,
  `/tmp/crexx-levelc-dynamic-combined-link.zqKoCK`, and
  `/tmp/crexx-levelc-dynamic-combined-tree.Nbveg1`.
- A scalar controlled `DO name = integer TO integer` now initializes and
  updates the visible pool variable. The canonical REPEAT contains a WHILE
  entry check over the current pool value and an UNTIL end-check BLOCK_EXPR
  that advances the pool value, including after ITERATE. A LEAVE skips the
  advance. Source control-variable mutation, zero-entry, nested loops,
  local procedure scope and final values matched Regina in optimized/no-opt
  and linked execution. Nonnumeric control values report `41.6`; BY, dynamic
  TO and controlled-plus-condition headers remain fail-closed. The redirected
  tree probe showed source-anchored WHILE > controlToContinue and UNTIL >
  BLOCK_EXPR > controlStepByOne/setValue > LEAVE_WITH, with no structural
  validation error. The selected Release Level C/source-provenance/shared
  runtime suite passed 195/195, and Debug structural/loop/shared-value tests
  passed 70/70. An initial focused test used stale generated CTest expected
  output after adding nested fixture lines; regenerating CMake restored a
  5/5 focused pass before the larger suites. Evidence:
  `/tmp/crexx-levelc-controlled-reference-output.hCs4cu`,
  `/tmp/crexx-levelc-controlled-release-build-final.50sCac`,
  `/tmp/crexx-levelc-controlled-focused-final.0OGSCw`,
  `/tmp/crexx-levelc-controlled-release-suite.AeafLD`,
  `/tmp/crexx-levelc-controlled-debug-build.4IIBOZ`,
  `/tmp/crexx-levelc-controlled-debug-focused.9vFoUy`,
  `/tmp/crexx-levelc-controlled-tree.Pr1gSI`, and
  `/tmp/crexx-levelc-controlled-link-log.grYt3z`.
- Signed literal BY now works in either TO/BY parser order. A negative step
  checks the lower bound, while zero and positive steps check the upper bound;
  the UNTIL end block advances the current pool value after ordinary and
  ITERATE paths. Bounded Regina output matched optimized/no-opt and linked
  execution for descending, ascending, guarded zero, opposite-direction
  zero-entry, body mutation, nested transfer and final values. The dynamic
  BY shape remains fail-closed. The redirected tree probe showed source
  anchors through WHILE > controlToContinueBy and UNTIL > BLOCK_EXPR >
  controlStepBy/setValue > LEAVE_WITH, with no structural validation error.
  Release selected correctness passed 198/198 and Debug structural/loop and
  shared-value tests passed 74/74. An initial shared-value build warning
  about an implicit branch-local binding was corrected before final build
  and suites. Evidence:
  `/tmp/crexx-levelc-by-reference-output.ECkvKQ`,
  `/tmp/crexx-levelc-by-release-build.WYm1fx`,
  `/tmp/crexx-levelc-by-release-rebuild.gGQzDh`,
  `/tmp/crexx-levelc-by-release-suite.yOq0q2`,
  `/tmp/crexx-levelc-by-debug-build.GCddqg`,
  `/tmp/crexx-levelc-by-debug-focused.2pMz1t`,
  `/tmp/crexx-levelc-by-tree.hgdhBa`, and
  `/tmp/crexx-levelc-by-link-log.HBlxiF`.
- A bounded literal FOR now joins a controlled TO header, with or without
  literal BY, in any parsed modifier order. The pool assignment precedes
  canonical REPEAT; FOR checks before WHILE, and the UNTIL end block advances
  the visible control variable after every executed body, including the final
  count-limited body and ITERATE. Regina output matched optimized/no-opt and
  linked execution for zero count, BY direction, mutation, LEAVE/ITERATE and
  local scope. Dynamic FOR and FOR without TO remain fail-closed. The
  redirected tree probe showed source-anchored FOR > WHILE > UNTIL with the
  BLOCK_EXPR pool step and no structural validation error. Release controlled
  focused checks passed 15/15 and the selected correctness suite 203/203;
  Debug structural/loop/shared-value checks passed 79/79. Evidence:
  `/tmp/crexx-levelc-for-reference-output.SfXAWx`,
  `/tmp/crexx-levelc-for-release-build.XvkBMA`,
  `/tmp/crexx-levelc-for-focused.X9e0I0`,
  `/tmp/crexx-levelc-for-release-suite.oK2LjO`,
  `/tmp/crexx-levelc-for-debug-build.Fd8bOH`,
  `/tmp/crexx-levelc-for-debug-focused.VbyWh3`,
  `/tmp/crexx-levelc-for-tree.5Kfxxl`, and
  `/tmp/crexx-levelc-for-link-log.GnUCkq`.
- A bounded literal controlled FOR now works without TO, with default or
  signed literal BY in either clause order. The canonical REPEAT has FOR and
  a source-anchored UNTIL/BLOCK_EXPR pool step, and the redirected tree probe
  found zero WHILE nodes and no structural validation error. Regina output
  matched optimized/no-opt and linked execution for zero count, ascending
  and descending steps, body mutation, ITERATE, LEAVE, nested grouping, local
  scope and final values. Unbounded controlled DO remains fail-closed. Release
  controlled focused checks passed 18/18 and the selected correctness suite
  206/206; Debug structural/loop/shared-value checks passed 82/82. Evidence:
  `/tmp/crexx-levelc-foronly-reference-output.TYlJ13`,
  `/tmp/crexx-levelc-foronly-release-build.Ngk8sl`,
  `/tmp/crexx-levelc-foronly-focused.bYKbZo`,
  `/tmp/crexx-levelc-foronly-tree.UrI1Uh`,
  `/tmp/crexx-levelc-foronly-link-log.xLD8XV`,
  `/tmp/crexx-levelc-foronly-release-suite.btNax4`,
  `/tmp/crexx-levelc-foronly-debug-build.JtE4St`, and
  `/tmp/crexx-levelc-foronly-debug-focused.XyAKqI`.
- Named `LEAVE` and `ITERATE` now bind case-insensitively to active supported
  source controlled loops, then use those loops' hidden canonical symbols
  through nested controlled loops, simple DO, IF and generated SELECT arms.
  Regina output matched opt/no-opt and linked execution, including outer
  targets, local procedure scope, visible final control values and a SELECT
  branch that first iterates and then leaves. Invalid names retained `28.3`
  and `28.4`. The redirected tree probe showed both SELECT-arm transfers
  resolving to the outer `__rxcp_levelc_loop_183` symbol with source anchors,
  and no structural validation error. Focused Release and Debug checks each
  passed 87/87; the selected Release correctness suite passed 211/211.
  Evidence: `/tmp/crexx-levelc-named-final-output.ry3ND0`,
  `/tmp/crexx-levelc-named-final-release-build.BGTyHT`,
  `/tmp/crexx-levelc-named-final-focused.mSJH86`,
  `/tmp/crexx-levelc-named-final-release-suite.GpPLgy`,
  `/tmp/crexx-levelc-named-final-debug-build.IAFCQu`,
  `/tmp/crexx-levelc-named-final-debug-focused.asSU2I`,
  `/tmp/crexx-levelc-named-final-tree-log.ciEinH`,
  `/tmp/crexx-levelc-named-final-rxlink.c9e0QC`, and
  `/tmp/crexx-levelc-named-final-rxvm.HZjxKG`.
- Controlled literal TO/FOR/BY loops now accept a supported WHILE condition.
  A canonical entry BLOCK_EXPR branches on the TO guard before evaluating
  WHILE and its setup; FOR remains the outer count guard. The source-anchored
  tree probe showed the two LEAVE_WITH paths and existing UNTIL pool step.
  Regina output matched opt/no-opt and linked execution for zero FOR, failed
  TO, TO plus FOR, signed BY, named ITERATE/LEAVE, local scope and final
  control values; invalid WHILE retained `34.3`. Controlled UNTIL remains
  fail-closed. Focused Release and Debug checks each passed 30/30; the
  selected Release suite passed 215/215. Evidence:
  `/tmp/crexx-levelc-controlled-while-final-output.ZFMbkl`,
  `/tmp/crexx-levelc-controlled-while-final-release-build.blK8E2`,
  `/tmp/crexx-levelc-controlled-while-final-debug-build.fhsCvL`,
  `/tmp/crexx-levelc-controlled-while-final-focused-release.P8Rr9m`,
  `/tmp/crexx-levelc-controlled-while-final-focused-debug.ezz8o0`,
  `/tmp/crexx-levelc-controlled-while-final-tree-log.RnyIDf`,
  `/tmp/crexx-levelc-controlled-while-final-rxlink.vXcWAn`,
  `/tmp/crexx-levelc-controlled-while-final-rxvm.hWDN67`, and
  `/tmp/crexx-levelc-controlled-while-final-suite.3s3YaD`.
- Controlled literal TO/FOR/BY loops now also accept a supported UNTIL
  condition. The canonical UNTIL end BLOCK_EXPR evaluates the condition and
  its setup first, returns true without advancing the pool variable, or
  advances the pool variable and returns false. The redirected tree probe
  showed both source-anchored LEAVE_WITH paths and no structural error.
  Regina output matched opt/no-opt and linked execution for failed TO and
  zero FOR guards, TO plus FOR, signed BY, named ITERATE and LEAVE, local
  scope, true UNTIL final values and false UNTIL after the final FOR body.
  Invalid UNTIL retained `34.4`. Focused Release and Debug checks each
  passed 33/33; the selected Release suite passed 218/218. Evidence:
  `/tmp/crexx-levelc-controlled-until-final-output.sl0khV`,
  `/tmp/crexx-levelc-controlled-until-final-release-build.6GfDgD`,
  `/tmp/crexx-levelc-controlled-until-final-debug-build.eZi4Ou`,
  `/tmp/crexx-levelc-controlled-until-final-focused-release.xvEh8a`,
  `/tmp/crexx-levelc-controlled-until-final-focused-debug.kqQ3Fc`,
  `/tmp/crexx-levelc-controlled-until-final-tree-log.xlrpul`,
  `/tmp/crexx-levelc-controlled-until-final-rxlink.8NqDE8`,
  `/tmp/crexx-levelc-controlled-until-final-rxvm.o7OKTS`, and
  `/tmp/crexx-levelc-controlled-until-final-suite.4HfEBG`.
- Scalar controlled DO now runs without TO or FOR, using its literal start,
  optional signed literal BY and the existing pool-backed end step. Guarded
  Regina output matched opt/no-opt and linked execution for default and
  descending steps, body mutation, named ITERATE/LEAVE, WHILE/UNTIL and
  local scope. The redirected tree probe showed the bare source DO lowered
  to one REPEAT with its hidden assignment and UNTIL pool step, with no FOR
  or synthetic WHILE on that loop. Release slice checks passed 3/3, Debug
  structural/loop/shared-value checks passed 35/35, and the selected Release
  correctness suite passed 220/220. Evidence:
  `/tmp/crexx-levelc-unbounded-control-reference-output.HRixQf`,
  `/tmp/crexx-levelc-unbounded-control-build.a4MQjg`,
  `/tmp/crexx-levelc-unbounded-control-focused.9MyCQ7`,
  `/tmp/crexx-levelc-unbounded-control-final-debug-build.NA5bWO`,
  `/tmp/crexx-levelc-unbounded-control-final-debug-focused.wHupzq`,
  `/tmp/crexx-levelc-unbounded-control-final-tree-log.N2zw6I`,
  `/tmp/crexx-levelc-unbounded-control-final-rxlink.Uz6AhR`,
  `/tmp/crexx-levelc-unbounded-control-final-rxvm.RtgQkL`, and
  `/tmp/crexx-levelc-unbounded-control-final-suite.BJQr1F`.
- Supported dynamic TO expressions are now evaluated and validated before
  assigning the new visible control value, then captured in a hidden
  canonical RexxValue local. The redirected tree probe showed the
  `controlToValue` capture before the pool assignment and a single hidden
  reference in each TO check. Regina output matched opt/no-opt and linked
  execution for endpoint mutation, a pre-existing control value observed by
  a TO function under FOR-zero, signed BY, WHILE/UNTIL and named outer
  ITERATE. A nonnumeric TO under FOR-zero reports `41.4` at setup. The shared
  `rxfnsc` image built in Release and Debug; focused Release checks passed
  4/4, the selected Release Level C/RexxValue/RexxScript suite 223/223, and
  Debug structural/loop/shared-value/RexxScript checks 44/44. Evidence:
  `/tmp/crexx-levelc-dynamic-to-reference-output.Oxyw65`,
  `/tmp/crexx-levelc-dynamic-to-build.VacXdq`,
  `/tmp/crexx-levelc-dynamic-to-focused.7wMczl`,
  `/tmp/crexx-levelc-dynamic-to-final-suite.j7fm7V`,
  `/tmp/crexx-levelc-dynamic-to-final-debug-build.brP7hY`,
  `/tmp/crexx-levelc-dynamic-to-final-debug-focused.zlljwp`,
  `/tmp/crexx-levelc-dynamic-to-final-tree-log.3nGjLn`,
  `/tmp/crexx-levelc-dynamic-to-final-rxlink.U0YRNN`, and
  `/tmp/crexx-levelc-dynamic-to-final-rxvm.aOFKn4`.
- Supported dynamic BY expressions now capture once before the new visible
  control assignment. Dynamic TO and BY follow the parser's written order;
  the redirected tree probe showed both orders, with the captured BY value
  used for TO direction and the UNTIL pool step. Regina output matched
  opt/no-opt and linked execution for source mutation, positive/negative
  and guarded zero steps, FOR-zero, no-TO, WHILE/UNTIL, named ITERATE,
  and local scope. Nonnumeric BY under FOR-zero reports `41.5` at setup.
  The shared `rxfnsc` image built in Release and Debug; focused Release
  checks passed 4/4, the selected Release Level C/RexxValue/RexxScript
  suite 226/226, and Debug structural/loop/shared-value/RexxScript checks
  47/47. Evidence: `/tmp/crexx-levelc-dynamic-by-final-output.OPzBAF`,
  `/tmp/crexx-levelc-dynamic-by-build.WijpWN`,
  `/tmp/crexx-levelc-dynamic-by-focused.GJluYO`,
  `/tmp/crexx-levelc-dynamic-by-final-suite.wA6Kiu`,
  `/tmp/crexx-levelc-dynamic-by-final-debug-build.7Dxae4`,
  `/tmp/crexx-levelc-dynamic-by-final-debug-focused.7dUaJ5`,
  `/tmp/crexx-levelc-dynamic-by-final-tree-log.NJpdo3`,
  `/tmp/crexx-levelc-dynamic-by-final-rxlink.TkVth6`, and
  `/tmp/crexx-levelc-dynamic-by-final-rxvm.IpplQZ`.
- The independent optimized call-window defect exposed by controlled FOR/BY
  now has an existing-syntax fixture. With the original affinity, its
  optimized path reported `41.6` after one body; no-opt and Regina produced
  both bodies and final control value 3. Disabling affinity for every call
  passed this fixture but made the existing `levelc_slice15_while` hang; the
  bounded repair uses a disjoint window only for calls under a canonical
  UNTIL end check. The emitted step call now marshals/restores the captured
  BY register. Release selected compiler checks passed 250/250, Debug
  focused checks passed 52/52, and linked RXBIN output matched Regina.
  Evidence: `/tmp/crexx-levelc-for-min.Yf5AZm/zero_then_literal-old.output`,
  `/tmp/crexx-levelc-call-window-preassign-focused.j5D997` (rejected broad
  candidate), `/tmp/crexx-levelc-call-window-until-focused.h6kqpx`,
  `/tmp/crexx-levelc-call-window-release-suite-final2.JPPBXU`,
  `/tmp/crexx-levelc-call-window-debug-build.d8G1D4`,
  `/tmp/crexx-levelc-call-window-final-debug-rebuild.5NAeJ4`,
  `/tmp/crexx-levelc-call-window-debug-focused-final2.RSuasd`,
  `/tmp/crexx-levelc-call-window-emission.81LA5Z`,
  `/tmp/crexx-levelc-call-window-regina-output.sLJ3gC`,
  `/tmp/crexx-levelc-call-window-final-rxlink.fh7JWi`, and
  `/tmp/crexx-levelc-call-window-final-rxvm.wmAT3T`.
- Controlled FOR now accepts a supported dynamic expression under the same
  written-order header capture used by TO and BY, before setting the visible
  control variable. The hidden signed-32-bit canonical count is consumed by
  REPEAT FOR and validated with contextual `26.3`. The source-anchored tree
  showed FOR/TO/BY captures in written order before pool assignment and one
  hidden FOR reference, with no structural validation error. Regina output
  matched optimized/no-opt and linked execution for zero and positive counts,
  mutation, whole decimal spelling, WHILE/UNTIL timing, named ITERATE, local
  scope and final pool values. Text, fractional, negative and over-range
  counts report `26.3`. Release focused checks passed 7/7 and the selected
  Level C/source-provenance/RexxValue/RexxScript suite 234/234; Debug
  structural/loop/shared-value/RexxScript checks passed 59/59. Evidence:
  `/tmp/crexx-levelc-dynamic-for-final-output.Hx3vbP`,
  `/tmp/crexx-levelc-dynamic-for-final-build.3XoHet`,
  `/tmp/crexx-levelc-dynamic-for-final-focused.XGC1lm`,
  `/tmp/crexx-levelc-dynamic-for-final-release-suite.OU1FgC`,
  `/tmp/crexx-levelc-dynamic-for-final-debug-build.qQtBk9`,
  `/tmp/crexx-levelc-dynamic-for-final-debug-focused.AzfcWY`,
  `/tmp/crexx-levelc-dynamic-for-tree-log.imSL3L`,
  `/tmp/crexx-levelc-dynamic-for-final-rxlink.5slfNf`, and
  `/tmp/crexx-levelc-dynamic-for-final-rxvm.YpKaJt`.
- Captured dynamic start now uses a hidden canonical `RexxValue` before the
  written-order TO/BY/FOR captures. The `STAGE_LEVELC_LOWERED` tree shows
  start, TO, BY and FOR in source order, then `controlStartValue()` at the
  visible pool set, with the start source anchor intact. Regina and linked
  RXBIN output match byte for byte for zero FOR, source mutation, signed
  decimal spelling, WHILE/UNTIL, named ITERATE, local scope and final pool
  values. Invalid start reports `41.6` after valid later clauses; invalid TO
  reports `41.4` first, and an invalid FOR count reports `26.3` first.
  Final Release Level C checks passed 229/229, focused Release 6/6, Debug
  controlled/structural/shared checks 57/57, and shared Release
  RexxValue/RexxScript checks 4/4. Evidence:
  `/tmp/crexx-levelc-dynamic-start-regina-output.nv9R5P`,
  `/tmp/crexx-levelc-dynamic-start-final-release-build.DpHht2`,
  `/tmp/crexx-levelc-dynamic-start-final-release-focused.VrFMdb`,
  `/tmp/crexx-levelc-dynamic-start-final-release-suite.cqh4Re`,
  `/tmp/crexx-levelc-dynamic-start-shared-release.c1vnYz`,
  `/tmp/crexx-levelc-dynamic-start-final-debug-build.cyqgNB`,
  `/tmp/crexx-levelc-dynamic-start-debug-focused.raexX4`,
  `/tmp/crexx-levelc-dynamic-start-tree.ODAevD`,
  `/tmp/crexx-levelc-dynamic-start-final-toolchain.hHePVE`,
  `/tmp/crexx-levelc-dynamic-start-final-linked-output.lEaINL`,
  `/tmp/crexx-levelc-dynamic-start-clause-order.UametO`, and
  `/tmp/crexx-levelc-dynamic-start-for-compile.z01VEf`.
- Empty single- and double-quoted source strings now lower to a canonical
  source-anchored empty `STRING` under the shared `RexxValue` factory. Regina
  and linked RXBIN output match byte for byte for SAY, assignment,
  concatenation, supported BIF/local calls and strict comparison. Empty TO
  reports contextual `41.4` instead of failing compilation. Final focused
  Release checks passed 4/4, the Release Level C suite 233/233 and Debug
  expression/AST/shared-value checks 20/20. The first tree test assertion
  named the parser comparison node rather than the canonical method; it was
  corrected, and no runtime regression was found. Evidence:
  `/tmp/crexx-levelc-empty-literal-baseline.AfmyLS`,
  `/tmp/crexx-levelc-empty-string-regina-output.rsxwSI`,
  `/tmp/crexx-levelc-empty-string-release-build.54d6FN`,
  `/tmp/crexx-levelc-empty-string-focused-release-final.d5hGwu`,
  `/tmp/crexx-levelc-empty-string-release-suite.1ZU6a4`,
  `/tmp/crexx-levelc-empty-string-debug-build.4gThvF`,
  `/tmp/crexx-levelc-empty-string-debug-focused.OVm4j4`,
  `/tmp/crexx-levelc-empty-string-final-tree.4BIMZA`,
  `/tmp/crexx-levelc-empty-string-link-log.djwxRM`, and
  `/tmp/crexx-levelc-empty-string-linked-output.3VVRE6`.
- A continuation-expression function name now becomes a source `FUNCTION`
  when its opening parenthesis touches the name; a blank-separated name and
  group remain `VAR_SYMBOL` plus `OP_SCONCAT`. The raw and lowered tree
  probes show the intended distinction and existing canonical BIF/local-call
  paths. Regina output matches opt/no-opt and linked RXBIN for literal and
  variable left operands, nested BIF arguments, empty string arguments, a local
  function and the spaced form. Final focused Release checks passed 3/3,
  Release Level C 236/236, and Debug parser/expression/shared checks 74/74.
  The distinct `ARG` case-normalization finding remains `LC-FIND-03`.
  Evidence: `/tmp/crexx-levelc-adjacent-call-regina-output.rmEEm9`,
  `/tmp/crexx-levelc-adjacent-call-release-build.ksfzMl`,
  `/tmp/crexx-levelc-adjacent-call-focused-release-final.yn84NQ`,
  `/tmp/crexx-levelc-adjacent-call-release-suite.cfqQkL`,
  `/tmp/crexx-levelc-adjacent-call-debug-build.6GCUaK`,
  `/tmp/crexx-levelc-adjacent-call-debug-focused.7jrIHu`,
  `/tmp/crexx-levelc-adjacent-call-tree.M4wN3G`,
  `/tmp/crexx-levelc-adjacent-call-link-log.OkFc9t`, and
  `/tmp/crexx-levelc-adjacent-call-linked-output.irO9L1`.
- Direct local `ARG` now applies shared Classic TRANSLATE to each captured
  argument before its source-anchored canonical pool set. The tree shows one
  BIF context/frame and call per target, in target order, with the import
  generated only when required. Regina and linked RXBIN output match for
  lowercase/mixed-case and spaced values, multiple arguments, exposure,
  unchanged caller variables and an empty argument. Final focused Release
  checks passed 8/8, Release Level C 239/239, shared TRANSLATE/RexxValue
  4/4, and Debug procedure/structural/shared checks 31/31. The current Level C
  call configuration defaults to BYTE; UTF8 delivery remains open under
  `LC-AC-04/06`. Evidence:
  `/tmp/crexx-levelc-arg-uppercase-reference.2nn27c`,
  `/tmp/crexx-levelc-arg-uppercase-baseline.qpCi0e`,
  `/tmp/crexx-levelc-arg-uppercase-regina-output.yaoWNR`,
  `/tmp/crexx-levelc-arg-uppercase-release-build.XJaXBG`,
  `/tmp/crexx-levelc-arg-uppercase-focused-release.GhBR4m`,
  `/tmp/crexx-levelc-arg-uppercase-release-suite.YLlJyL`,
  `/tmp/crexx-levelc-arg-uppercase-shared-release.OePRCZ`,
  `/tmp/crexx-levelc-arg-uppercase-debug-build.nR7gEZ`,
  `/tmp/crexx-levelc-arg-uppercase-debug-focused.DWm9zr`,
  `/tmp/crexx-levelc-arg-uppercase-tree.3Hzw3j`,
  `/tmp/crexx-levelc-arg-uppercase-link-log.pQWMl4`, and
  `/tmp/crexx-levelc-arg-uppercase-linked-output.c6E1qt`.
- The first `PARSE` AST slice validates the parser's outer and inner
  `TEMPLATES` nodes, then lowers one direct scalar target for `VAR` or `VALUE`
  sources, with optional UPPER through the shared Classic TRANSLATE frame.
  The raw tree has distinct source/template nodes; the canonical tree has
  source-anchored `symbolValue`/`setValue` and BIF calls with no `PARSE` node.
  Regina and linked RXBIN output match byte for byte for source preservation,
  a once-evaluated source expression, nested IF/DO, procedure scope, self
  assignment and uppercase. Multiple targets and literal patterns still fail
  closed. The boundary verifier now also rejects a residual PARSE or template
  node. Final Release Level C passed 245/245, eight focused Release tests and
  13 Debug structural/procedure tests passed. Evidence:
  `/tmp/crexx-levelc-parse-probe.kjXGc9/tree.log`,
  `/tmp/crexx-levelc-parse-fixture.wWlWfo/regina.out`,
  `/tmp/crexx-levelc-parse-fixture.wWlWfo/linked.out`,
  `/tmp/crexx-levelc-parse-final-build.a4BefS`,
  `/tmp/crexx-levelc-parse-release-suite.xumsvR`,
  `/tmp/crexx-levelc-parse-final-release-focused.0ti3pv`,
  `/tmp/crexx-levelc-parse-debug-build.bALj8Y`,
  `/tmp/crexx-levelc-parse-debug-focused.CyCSsi`,
  `/tmp/crexx-levelc-parse-tree.15G3Tc`, and
  `/tmp/crexx-levelc-parse-link.FDBVgU`, plus final verifier/build and linked
  proof in `/tmp/crexx-levelc-parse-verifier-build.z0XG9n`,
  `/tmp/crexx-levelc-parse-verifier-test-build.qZLfQH`,
  `/tmp/crexx-levelc-parse-final-suite.NMhbqO`,
  `/tmp/crexx-levelc-parse-final-debug-build.5Atjbl`,
  `/tmp/crexx-levelc-parse-final-debug-tests.GYe1lb`, and
  `/tmp/crexx-levelc-parse-final-toolchain.a8dmN3`.
- Three adjacent direct scalar `PARSE` targets now use
  `RexxValue.parseThreeWords()`, which wraps the existing `parsewords3` VM
  opcode and returns two words plus the original remaining tail. The Level C
  lowerer captures the source and result array before ordered pool writes;
  shared TRANSLATE handles optional UPPER. The optimized and no-opt fixtures
  match Regina byte for byte for leading/repeated/trailing blanks, short
  input, repeated targets, a source alias, one-time VALUE evaluation and local
  scope. The raw tree's nested templates disappear from the canonical tree;
  generated array/result nodes retain source anchors. Four-target templates
  and patterns still fail closed. Release Level C passed 249/249, focused
  Release 14/14, Debug 15/15, shared RexxValue opt/no-opt 2/2 and RexxScript
  runtime/compatibility 4/4. The linked RXBIN output matches Regina.
  Evidence: `/tmp/crexx-levelc-parse-three-reference.VE49lG`,
  `/tmp/crexx-levelc-parse-three-probe.OKSy14/regina-explicit.out`,
  `/tmp/crexx-levelc-parse-three-probe.OKSy14/crexx3.out`,
  `/tmp/crexx-levelc-parse-three-rxfnsc-build.oQsQ7O`,
  `/tmp/crexx-levelc-parse-three-compiler-build.JLasPA`,
  `/tmp/crexx-levelc-parse-three-shared-build.HJlCxG`,
  `/tmp/crexx-levelc-parse-three-shared-test.VDvktb`,
  `/tmp/crexx-levelc-parse-three-focused-release.VsyHp5`,
  `/tmp/crexx-levelc-parse-three-release-suite.s0dk44`,
  `/tmp/crexx-levelc-parse-three-debug-build.LCVPRF`,
  `/tmp/crexx-levelc-parse-three-debug-tests.KBNkZ2`,
  `/tmp/crexx-levelc-parse-three-rexxscript-build.FR90F5`,
  `/tmp/crexx-levelc-parse-three-rexxscript-tests.suoapn`,
  `/tmp/crexx-levelc-parse-three-tree.WZfAtg`,
  `/tmp/crexx-levelc-parse-three-fixture.46Rrly/regina.out`,
  `/tmp/crexx-levelc-parse-three-fixture.46Rrly/linked.out`, and
  `/tmp/crexx-levelc-parse-three-linked.xPkP8j`.
- Adjacent Classic terms now select the existing `OP_CONCAT` AST node; spaced
  terms retain `OP_SCONCAT`. The parser checks token gaps across group and
  call punctuation and comment-only gaps, while refusing to infer abuttal
  across an omitted dot. The raw tree shows both implicit nodes and the
  explicit `||` node; the lowered tree uses the canonical expression path.
  Regina and linked RXBIN output match byte for byte, including PARSE VALUE
  expressions. Release Level C passed 252/252, Debug focused 8/8, and the
  decimal FOR rejection remained green. Evidence:
  `/tmp/crexx-levelc-abuttal-release-suite.ClLyo4`,
  `/tmp/crexx-levelc-abuttal-debug-build.hrdpMk`,
  `/tmp/crexx-levelc-abuttal-debug-tests.AN4Zv6`,
  `/tmp/crexx-levelc-abuttal-tree.oD0XbH`, and
  `/tmp/crexx-levelc-abuttal-linked.gCu88S`.
- Decimal fraction constants now reach the raw tree as one source `DECIMAL`
  node for `1.5`, `1.`, and `.5`; the canonical tree constructs a shared
  `RexxValue` from the exact spelling. Regina output matches optimized,
  no-opt and linked execution for literals, arithmetic, PARSE VALUE and
  abutted expressions. Existing invalid fractional DO/FOR count diagnostics
  remain green; `1.0` and `1.` whole-number FOR probes also match Regina.
  Release Level C passed 255/255 and focused Debug 8/8. Evidence:
  `/tmp/crexx-levelc-decimal-tree.yu3Qoo`,
  `/tmp/crexx-levelc-decimal-final-suite.H6aH56`,
  `/tmp/crexx-levelc-decimal-final-debug-build.8hQLB7`,
  `/tmp/crexx-levelc-decimal-final-debug-tests.Sf2900`,
  `/tmp/crexx-levelc-decimal-final-linked.wudGXo`, and
  `/tmp/crexx-levelc-decimal-count.wVmu1e`.
- Exponent numeric constants with integer, trailing-dot and fractional
  mantissas now form one source `DECIMAL` node, with uppercase `E` in the
  lowered Classic value. The failing baseline raised a decimal conversion
  panic; the new opt/no-opt fixture matches Regina for source spelling,
  arithmetic, PARSE VALUE, abuttal and a whole-number FOR. A fractional
  exponent FOR still raises contextual `26.3`. Release Level C passed
  259/259, focused Debug 8/8, and linked output matches byte for byte.
  Evidence: `/tmp/crexx-levelc-exponent-baseline.UweCIf`,
  `/tmp/crexx-levelc-exponent-count-reference.Xcpebt`,
  `/tmp/crexx-levelc-exponent-tree.pBzsPS`,
  `/tmp/crexx-levelc-exponent-release-suite.2ogRVg`,
  `/tmp/crexx-levelc-exponent-debug-build.3r0pwZ`,
  `/tmp/crexx-levelc-exponent-debug-tests.nEbinY`, and
  `/tmp/crexx-levelc-exponent-linked.M8newV`.
- Digit-starting nonnumeric constant symbols now scan as one Level C-only
  raw token and one source `CONST_SYMBOL` node. The canonical tree creates
  uppercase shared `RexxValue` strings, independent of similarly named pool
  variables. Regina, optimized/no-opt and linked output match for letters,
  repeated periods, exponent-looking nonnumbers, PARSE VALUE and abuttal.
  The nonnumeric FOR count still reports `26.3`. The dedicated highlighter
  check classifies these as constants; Release Level C passed 264/264 and
  focused Debug passed 9/9. Evidence:
  `/tmp/crexx-levelc-constant-reference.FiKTwC`,
  `/tmp/crexx-levelc-constsym-tree.xkWjAO`,
  `/tmp/crexx-levelc-constsym-release-suite.P87UU4`,
  `/tmp/crexx-levelc-constsym-debug-build.MOE5TW`,
  `/tmp/crexx-levelc-constsym-debug-tests.6wtAsS`, and
  `/tmp/crexx-levelc-constsym-linked.jx6fO9`.
- Two direct scalar `PARSE` targets now use shared
  `RexxValue.parseWordAndRest()`: the first receives the first word and the
  second receives the source after exactly one separator. The method calls
  the existing `parsewords3` VM primitive for the first word, then retains
  the unparsed remainder from the captured source. The Level C lowerer
  captures both results before ordered pool writes and reuses shared
  TRANSLATE for optional UPPER. Regina, optimized/no-opt and linked output
  match for leading/repeated/trailing blanks, short input, repeated targets,
  source alias, one-time VALUE evaluation and local scope. Raw nested
  templates become canonical array/result and pool-write nodes. Existing
  IF/DO negative fixtures were advanced to four-target shapes at that step;
  `LC-AC-49` later advanced them to five. Release Level C passed 267/267, focused Debug 7/7, shared
  RexxValue opt/no-opt 2/2 and RexxScript integration 4/4. Evidence:
  `/tmp/crexx-levelc-parse-two-reference.65kbCR`,
  `/tmp/crexx-levelc-parse-two-reconfigure.N7F6ew`,
  `/tmp/crexx-levelc-parse-two-final-suite.iG3FvQ`,
  `/tmp/crexx-levelc-parse-two-debug-build.AM9MDF`,
  `/tmp/crexx-levelc-parse-two-debug-tests.hufXxc`,
  `/tmp/crexx-levelc-parse-two-rexxscript-tests.3Xjp1D`,
  `/tmp/crexx-levelc-parse-two-tree.lFVAkL`, and
  `/tmp/crexx-levelc-parse-two-linked.JAcmAY`.
- Three direct scalar `PARSE` targets followed by a final drop dot now use
  shared `RexxValue.parseThreeWordsDrop()` and the existing `parsewords3d`
  VM primitive. The lowerer validates the nested template, captures all
  results before ordered pool writes, and emits no dot target. Regina,
  optimized/no-opt, and linked output match for whitespace, short input,
  aliases, repeated targets, one-time VALUE evaluation, UPPER, and local
  scope. Middle dots were handled in `LC-AC-47`. The raw tree contains the final
  target dot; the canonical tree contains only the three result writes.
  Release Level C passed 271/271, focused Release 9/9, Debug 6/6, shared
  RexxValue opt/no-opt 2/2, and RexxScript integration 4/4. Evidence:
  `/tmp/crexx-levelc-parse-dot-release-suite.RYgUiH`,
  `/tmp/crexx-levelc-parse-dot-focused.edkWI2`,
  `/tmp/crexx-levelc-parse-dot-debug-build.n1RpSW`,
  `/tmp/crexx-levelc-parse-dot-debug-tests.TBCnql`,
  `/tmp/crexx-levelc-parse-dot-shared-tests.ilOeIV`,
  `/tmp/crexx-levelc-parse-dot-rexxscript-tests.SlcJ1n`,
  `/tmp/crexx-levelc-parse-dot-tree.HINvyR`, and
  `/tmp/crexx-levelc-parse-dot-linked.lXTUHm`.
- Two- and three-item PARSE templates now accept dot placeholders in any
  position when at least one scalar target remains. The lowerer uses the
  existing shared split methods, captures source/results once, skips dot
  pool writes, and preserves scalar write order. Regina, opt/no-opt and
  linked output match for leading/internal/final and repeated dots, source
  aliases, repeated targets, short input, UPPER, local scope and one-time
  VALUE evaluation. The raw tree retains target dots; the canonical tree
  uses only split calls and scalar pool writes. The previous middle-dot
  negative fixture was removed after its now-supported shape passed in the
  positive fixture. Four-item internal-dot and all-dot forms were carried to
  `LC-AC-49`.
  Release Level C passed 275/275, focused Release 14/14, and focused Debug
  14/14. Evidence: `/tmp/crexx-levelc-parse-placeholder-focused.UAmhb4`,
  `/tmp/crexx-levelc-parse-placeholder-final-release-suite.uHdOV3`,
  `/tmp/crexx-levelc-parse-placeholder-debug-tests.PNHONc`,
  `/tmp/crexx-levelc-parse-placeholder-tree.lWOHrY`, and
  `/tmp/crexx-levelc-parse-placeholder-linked.VmRJjp`.
- One-, two- and three-item all-dot direct PARSE templates now consume their
  source without writing the visible pool. A hidden canonical capture for
  the single-dot form preserves one-time VALUE expression effects in both
  optimizer modes; longer forms use the existing shared split-result capture.
  Regina, optimized/no-opt and linked output agree for unchanged VAR sources,
  repeated VALUE side effects, UPPER and local procedure scope. The raw tree
  retains each dot target; the canonical tree has hidden captures but no dot
  pool binding. The obsolete two-dot negative was replaced with a four-dot
  negative at that step; `LC-AC-49` later covered four-dot templates.
  Release Level C passed 278/278, focused Release 6/6,
  and focused Debug 7/7. Evidence:
  `/tmp/crexx-levelc-parse-all-dots-reference.rXJeiX`,
  `/tmp/crexx-levelc-parse-all-dots-focused.yvfJDh`,
  `/tmp/crexx-levelc-parse-all-dots-release-suite.CaCKT2`,
  `/tmp/crexx-levelc-parse-all-dots-debug-tests.yHx2pQ`,
  `/tmp/crexx-levelc-parse-all-dots-tree.Q551Qa`, and
  `/tmp/crexx-levelc-parse-all-dots-linked.OfbdAz`.
- Four-item direct scalar/dot PARSE templates now reuse one validated nested
  AST path. When the final item is a scalar, shared
  `RexxValue.parseFourWords()` composes `parsewords3` and
  `parseWordAndRest()` to retain the tail after the third word. With a final
  dot, `parseThreeWordsDrop()` supplies three words and the lowerer skips
  all dot positions. Source and results are captured before ordered scalar
  writes. Regina, opt/no-opt and linked output match for whitespace, short
  input, internal/final/all dots, aliases, repeated targets, one-time VALUE,
  UPPER and local scope. The raw tree retains four target items; the
  canonical tree uses existing split and pool-call nodes only. Three obsolete
  four-item negatives were removed; IF/DO negatives now use five targets.
  Release Level C passed 279/279 after that fixture update, focused Release
  12/12, Debug 9/9 plus two updated negatives, shared RexxValue opt/no-opt
  2/2, and RexxScript integration 4/4. Evidence:
  `/tmp/crexx-levelc-parse-four-reference.TUuzbM`,
  `/tmp/crexx-levelc-parse-four-focused.8Wk6eF`,
  `/tmp/crexx-levelc-parse-four-final-release-suite.gFH8A3`,
  `/tmp/crexx-levelc-parse-four-debug-tests.CnB0ru`,
  `/tmp/crexx-levelc-parse-four-updated-negative-release.eyBDrr`,
  `/tmp/crexx-levelc-parse-four-updated-negative-debug.Ehqwax`,
  `/tmp/crexx-levelc-parse-four-rexxscript-tests.QMwz3F`,
  `/tmp/crexx-levelc-parse-four-tree.hlZBCl`, and
  `/tmp/crexx-levelc-parse-four-linked.LtDJF8`.
- Direct scalar/dot `PARSE VAR` and `PARSE VALUE` templates now use one
  validated AST lowering path for any positive representable item count.
  `RexxValue.parseWordTemplate(count)` returns the first `count-1` words and
  the original remaining tail; the compiler captures that result vector
  before scalar pool writes and skips only dot writes. The previous
  count-specific dispatch and four-item guard are gone. Regina and optimized,
  no-opt and linked RXBIN output match for five and eight items, whitespace,
  short input, dots including all-dot, aliases, repeated targets, one-time
  VALUE evaluation, UPPER and local scope. Prior one- through four-item
  fixtures remain green, and the IF/DO unsupported checks now use pattern
  templates. Release Level C passed 281/281; focused Release and Debug each
  passed 31/31; RexxScript integration passed 4/4. Raw/canonical tree
  inspection and the tree-shape tests show one `parseWordTemplate` lowering
  call per source PARSE and no surviving Level C template node. Evidence:
  `/tmp/crexx-levelc-parse-generic-release-build.YRynPa`,
  `/tmp/crexx-levelc-parse-generic-focused-release.m3fwAi`,
  `/tmp/crexx-levelc-parse-generic-release-suite.gPK7ld`,
  `/tmp/crexx-levelc-parse-generic-debug-build.YlI2Sa`,
  `/tmp/crexx-levelc-parse-generic-focused-debug.xN3a72`,
  `/tmp/crexx-levelc-parse-generic-rexxscript-build.dfVPXV`,
  `/tmp/crexx-levelc-parse-generic-rexxscript-tests.ERFAIo`,
  `/tmp/crexx-levelc-parse-generic-tree.zHv2HE`, and
  `/tmp/crexx-levelc-parse-generic-link.TfKI0n/`.
- `LC-AC-08/04` remain open for other controlled
  endpoints, wider count values, named
  transfers to wider unsupported loop shapes, and other structural families.

### LC-STEP-54 — static mixed PARSE templates, 2026-10-03

- `PARSE VAR` and `PARSE VALUE` now accept one validated static template with
  any representable ordered mix of scalar/dot targets, literal delimiters and
  absolute/relative positions. The compiler serializes the parsed AST to the
  existing frozen version-1 `parseplan` descriptor and emits one canonical
  `ASSEMBLER` call. The VM helper owns delimiter search, cursor movement and
  field capture; the lowerer snapshots the source and writes non-dot results
  through the Classic variable pool in authored order. Direct word-only
  templates retain the generic shared `RexxValue` path. No target-count or
  item-order dispatch and no new AST/emitter node were needed.
- Regina and optimized/no-opt output agree for leading, trailing, adjacent,
  repeated and missing patterns; absolute, relative, backward, zero and
  out-of-range positions; numeric spelling overlays; hex, binary, quoted and
  UTF-8 delimiters; all-dot templates; aliases, six targets, one-time VALUE
  effects, UPPER and local scope. Raw tree inspection retains ordered source
  `PARSE` children; the canonical tree has `parseplan` and pool writes with
  no surviving Level C template nodes. The static fixture is a permanent
  opt/no-opt and tree-shape regression. Dynamic pattern/position operands,
  comma templates and other source forms remain open.
- Release core build passed, focused Release 6/6 and Debug 6/6 passed, and
  the normal Release Level C suite passed 284/284. The final fixture compiled,
  assembled, linked and executed with byte-identical Regina output. The
  then-proposed opt-in UTF8 configuration was still open under `LC-AC-04/06`;
  this historical increment proved UTF-8 delimiter bytes in the then-current
  BYTE configuration. The Unicode-first route later superseded those profiles.
  Evidence: `/tmp/crexx-levelc-static-close-release-build.ptGl28`,
  `/tmp/crexx-levelc-static-close-debug-build.UiSZmq`,
  `/tmp/crexx-levelc-static-close-release-focused.Tj5rLe`,
  `/tmp/crexx-levelc-static-close-debug-focused.osxsnh`,
  `/tmp/crexx-levelc-static-close-suite.b1oLs5`,
  `/tmp/crexx-levelc-static.4nBnDm/close-tree.log`, and
  `/tmp/crexx-levelc-static.4nBnDm/close-linked.out`.

### LC-STEP-56 — shared Classic stem lifecycle, 2026-10-03

- Regina reference probes showed that a stem default assignment replaces old
  explicit and dropped tails; `DROP stem.tail` hides the default for that tail
  until reassignment; and a variable-derived tail retains its substituted
  case. The prior shared model discarded dropped-tail state and uppercased
  derived text. This is the distinct shared-runtime cause behind the direct
  compound `DROP` work; `LC-AC-53` remains open for compiler lowering.
- `RexxStem` now tracks dropped-tail tombstones, clears old tail state on
  default assignment and clears a tombstone on tail assignment.
  `RexxVariablePool` preserves derived-tail case and offers non-mutating
  `stemSymbolValue` plus `dropStemTail`. A focused failure found that mutated
  stems retrieved from pool slots need copyback; the helper now writes them
  back to local or exposed bindings. The permanent pool test covers default
  reset, case-distinct tails, dropped reads, reassignment, stem drop and
  exposed mutation.
- Release `rxfnsc` and the standalone RexxScript runner built. Focused pool
  opt/no-opt and linked tests passed 2/2; RexxScript integration passed 4/4;
  the normal Release Level C suite passed 284/284. Evidence:
  `/tmp/crexx-levelc-drop.qf1gA9/semantics.out`,
  `/tmp/crexx-levelc-stem-runtime-focused.PEEXmi` (initial failure),
  `/tmp/crexx-levelc-stem-runtime-rebuild.oq0Frs` (scope diagnostic),
  `/tmp/crexx-levelc-stem-runtime-rebuild.EpZx8L`,
  `/tmp/crexx-levelc-stem-runtime-final-focused.mrPABk`,
  `/tmp/crexx-levelc-stem-rexxscript-build.uJaVLM`,
  `/tmp/crexx-levelc-stem-rexxscript-tests.DT87if`, and
  `/tmp/crexx-levelc-stem-runtime-levelc-suite.1Kg8ty`.

### LC-STEP-57 — direct stem and compound DROP, 2026-10-03

- The direct `DROP` guard now accepts scalar, stem and supported single-tail
  compound names in one ordered list. Each compound tail is materialized once
  immediately before its corresponding pool call. Lowering emits canonical
  `drop`, `dropStem` and `dropStemTail` calls; `RexxVariablePool` and
  `RexxStem` retain the default, dropped-tail and exposed-alias rules. Direct
  compound reads use the non-mutating `stemSymbolValue` method. Parenthesized
  indirect references remain guarded.
- The Regina fixture covers case-distinct derived tails, dropped reads,
  reassignment, list order with an earlier scalar drop affecting a later tail,
  full stem drop, numeric tails, exposed procedure scope and nested `IF`/`DO`.
  The linked image produced byte-identical output. Raw/canonical tree probes
  show source `LEVELC_DROP` nodes replaced by ordered helper calls with hidden
  tail captures. Obsolete direct stem/compound negative fixtures were replaced
  by optimized, no-opt and tree-shape regressions. An older tree-shape test was
  updated from `stemValue` to the new non-mutating read method.
- Release Level C passed 285/285 and focused Debug passed 5/5 after rebuilding
  the Debug `rxfnsc` metadata. RexxScript 4/4 and focused pool opt/no-opt 2/2
  evidence from `LC-STEP-56` remains valid because its runtime inputs did not
  change in this compiler increment. The initial Release sweep found only the
  outdated tree-shape expectation; the initial Debug run found an outdated
  local `rxfnsc` build. Evidence: `/tmp/crexx-levelc-drop-final-build.log`,
  `/tmp/crexx-levelc-drop-final-suite.log`,
  `/tmp/crexx-levelc-drop-debug-build.log`,
  `/tmp/crexx-levelc-drop-debug-runtime-build.log`,
  `/tmp/crexx-levelc-drop-debug-final-focused.log`,
  `/tmp/crexx-levelc-drop.qf1gA9/tree.log`,
  `/tmp/crexx-levelc-drop.qf1gA9/final-reference.out`, and
  `/tmp/crexx-levelc-drop.qf1gA9/linked-output.out`.

### LC-STEP-58 — shared one-symbol DROP, 2026-10-03

- `RexxVariablePool.dropSymbol` resolves one validated Classic symbol against
  the visible pool and dispatches to the existing scalar, stem or single-tail
  drop operation. The pool regression proves case-preserved substituted tails,
  unaffected case-distinct tails, full stem/default removal and exposed scalar
  and stem aliases. The helper accepts one symbol, leaving the separate
  subsidiary-list parser and invalid-word policy to `LC-STEP-59`.
- Release and Debug pool opt/no-opt tests passed 2/2 in each build; the Release
  RexxScript runtime/compatibility tests passed 4/4, and the normal Release
  Level C suite passed 285/285. The optimized pool test uses the linked image,
  covering `rxc`, `rxas`, `rxlink` and `rxvm`. Evidence:
  `/tmp/crexx-levelc-dropsymbol-release-build.log`,
  `/tmp/crexx-levelc-dropsymbol-debug-build.log`,
  `/tmp/crexx-levelc-dropsymbol-debug-focused.log`,
  `/tmp/crexx-levelc-dropsymbol-release-focused.log`, and
  `/tmp/crexx-levelc-dropsymbol-levelc-suite.log`.

### LC-STEP-59 — indirect DROP subsidiary lists, 2026-10-03

- `RexxVariablePool.dropIndirectList` consumes one captured list value and
  iterates every word in source order. The shared Classic symbol classifier
  skips invalid words per Adrian's Regina decision; each valid word goes
  through `dropSymbol`, so earlier drops affect later compound substitutions.
  The pool module's build dependencies now stage `RexxClassicConfig` and
  `RexxClassicDatatype`. The initial focused build exposed the missing module
  staging and was repaired before qualification.
- Level C's guarded `VAR_REFERENCE` shape now lowers to a single canonical
  `dropIndirectList` call with the referenced variable evaluated at its place
  in the authored direct/indirect list. The Regina fixture proves scalar,
  invalid-word, compound, list order, whole-stem, dropped list-variable,
  compound list-reference, exposed procedure and nested `IF`/`DO` cases.
  Raw tree inspection shows `LEVELC_DROP` and `VAR_REFERENCE`; neither remains
  in the lowered tree. The obsolete indirect negative fixture was replaced
  with optimized, no-opt and tree-shape regressions. There is no fixed list
  length in the helper.
- Focused Release and Debug compiler tests passed 6/6 each; pool opt/no-opt
  passed 2/2 in each build; RexxScript passed 4/4; and the normal Release
  Level C suite passed 287/287. The fixture compiled, assembled, linked and
  executed with byte-identical Regina output. Indirect-list classification
  used the then-default BYTE profile at this checkpoint; the approved Unicode
  route later superseded it and LC-STEP-88D-2 requalified DROP. Wider host and
  platform proof remains open under `LC-AC-04/06`. Evidence:
  `/tmp/crexx-levelc-indirect-regina.out`,
  `/tmp/crexx-levelc-indirect-release-pool-rebuild.log`,
  `/tmp/crexx-levelc-indirect-debug-pool-rebuild.log`,
  `/tmp/crexx-levelc-indirect-release-pool-focused.log`,
  `/tmp/crexx-levelc-indirect-debug-pool-focused.log`,
  `/tmp/crexx-levelc-indirect-release-core-build.log`,
  `/tmp/crexx-levelc-indirect-debug-core-build.log`,
  `/tmp/crexx-levelc-indirect-release-focused.log`,
  `/tmp/crexx-levelc-indirect-debug-focused.log`,
  `/tmp/crexx-levelc-indirect-suite.log`,
  `/tmp/crexx-levelc-indirect-rexxscript.log`, and
  `/tmp/crexx-levelc-indirect-linked.m1vxRl/`.
