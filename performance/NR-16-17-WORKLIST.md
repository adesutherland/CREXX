# NR-16/NR-17 stable-state resolution worklist

Status: complete; NR-16/NR-17 verdicts, noisy Base64 trade-off and final Tier A
absolute checkpoint accepted and retained
Started: 2026-07-23
Branch: `develop`
Starting HEAD: `bdf64e2a3e9e24953642cf4aa21cb503c72de4f6`
Starting upstream: `origin/develop` at
`240b29f456e995928206f04285a7c319612ff022`
Starting worktree: clean; two local commits ahead of `origin/develop`

## Objective and boundaries

Find the fastest evidence-supported end-to-end architecture for stable-state
TRACE, ADDRESS-environment selection, and linked call/provider resolution.
Resolve stable facts once and keep the repeated success path at an already-off
test, ID/pointer equality, or direct bound-target load where semantics permit.
NR-16 and NR-17 share that design ceiling but retain separate ownership,
invalidation and fallback rules.

The dated programme report is a read-only historical charter. This worklist
and `performance/ROADMAP.md` are the live control plane. No push is authorized.
Do not commit unless Adrian explicitly requests it.

Adrian approved the selected B+C NR-16 architecture and the independent NR-17
runtime operand-image architecture on 2026-07-23. Implement and measure NR-16
first. Do not begin the NR-17 production change until Adrian accepts the
mandatory first NR-16 Release verdict. No public ABI, canonical ISA or
serialized RXBIN change is selected.

## Verified starting state

- [x] `develop` is at exact local HEAD
      `bdf64e2a3e9e24953642cf4aa21cb503c72de4f6`.
- [x] `origin/develop` is at exact commit
      `240b29f456e995928206f04285a7c319612ff022`.
- [x] The branch is two local commits ahead: accepted NR-15 production
      `55e8bfe286e5656d9d2e35adfc5b1ea15d4d0d32` plus the isolated-fixture fix
      `bdf64e2a3e9e24953642cf4aa21cb503c72de4f6`.
- [x] The worktree was clean before this worklist, roadmap entry and focused
      fixture were added; `git diff --check` passed at the baseline and after
      the control-plane edits.
- [x] Root and performance instructions plus the complete live roadmap were
      read before this activity started.
- [x] The historical NR-16/NR-17 exit criteria and the required architecture,
      linker, RXBIN 007, VM, emitter and TRACE technical guidance were read.

## First-report findings and approved decisions

- No production source has been edited. The only live-tree changes are this
  worklist, the roadmap status, and the focused opt/noopt x `rxvm`/`rxbvm`
  observational fixture.
- The final optimized focused medians use 20,000 operations and eleven serial samples.
  After subtracting the matched loop control, `rxvm`/`rxbvm` cost about
  0.974/0.988 us for inactive `TRACE OFF`, 1.183/1.234 us for
  `TRACE VALUE _trace_current_mode()`, and 2.894/3.250 us per instruction in a
  real `TRACE NORMAL; TRACE OFF` pair. Retained source/TRACE metadata was
  1.189 us per inactive self-set versus 1.169 us stripped in a separate
  `rxvm` sample; it changes image size (215,362 versus 175,818 bytes) but does
  not expose a metadata scan on the dormant path.
- Current RexxCPS 2.2d executes the TRACE and ADDRESS clauses 14 times per
  nominal 1,000-clause iteration. A canonical 100 x 100 run therefore has
  140,000 of each operation. The directly measured TRACE cost attributes about
  165.6 ms (`rxvm`) / 172.7 ms (`rxbvm`) to inactive TRACE alone, at
  1.183/1.234 us each. Current one-run canonical controls were 6,284,640 CPS
  over 1.6 s (`rxvm`) and 5,830,156 CPS over 1.7 s (`rxbvm`).
- The ADDRESS compiler exit currently mislowers `ADDRESS VALUE expression` as
  environment `VALUE` plus a command expression. The generated RexxCPS RXAS is
  `_address('value',addressenv().environment_name(),...)`, so the supposed
  same-selection clause constructs a request, invokes the unknown environment,
  and runs command TRACE hooks. This is an existing-language correctness bug,
  not a legitimate stable-state cost.
- On optimized `rxvm`/`rxbvm`, current static same-environment selection costs
  0.966/1.129 us; the mislowered RexxCPS form costs 6.708/7.652 us; explicitly
  spelling its intended current semantics as
  `_set_address_environment(addressenv().environment_name())` costs
  2.307/2.664 us; using the already cached current normalized name costs
  1.084/1.258 us. Alternating selection costs 1.029/1.191 us per change.
- A diagnostic source-preserving NOP panel, explicitly not canonical evidence,
  measured median `rxvm` RexxCPS at 6,221,595 CPS status quo,
  7,337,452 without the inactive TRACE work, 18,154,076 without the mislowered
  ADDRESS work, and 28,353,994 without both. This is a ceiling comparison, not
  authorization to change observable evaluation semantics.
- Adrian approved exact private TRACE self-query/set and exact public ADDRESS
  current-environment identity-query/set forms as observational identities
  eligible for source-bearing no-op lowering. All non-exact forms retain their
  full observable evaluation and fallback behavior.
- Adrian approved the independent NR-17 process-local operand-image shape, but
  it remains held behind acceptance of the first NR-16 Release verdict.

## First NR-16 Release verdict (2026-07-23)

Implementation was frozen after focused correctness. The direct compiler,
TRACE/ADDRESS protocol and four-way stable-state cells passed 12/12, followed
by 15/15 host callback, Rexx/native provider, redirect and active TRACE cells
(16/16 including their linked-artifact build fixture).
The ordinary Release product was `Release` with `CREXX_VM_PROFILING=OFF`.
Formal samples ran serially on AC power with low-power mode off and no thermal
or performance warning: one warmup per cell followed by 12 balanced,
interleaved before/after rounds. Raw provisional evidence is outside the
worktree at `/private/tmp/nr16-release-verdict.prNkcx/evidence/`.

- Canonical RexxCPS median rose 6,211,909.5 -> 28,255,187 CPS on `rxvm`
  (+354.855%) and 5,630,531 -> 26,575,136.5 on `rxbvm` (+371.983%); all
  12/12 pairs were favorable for each VM.
- Control-adjusted exact inactive `TRACE VALUE _trace_current_mode()` fell
  from 1,200.108/1,255.590 ns to 0.410/0.368 ns per operation
  (`rxvm`/`rxbvm`). A canonical 100 x 100 equivalent therefore falls from
  168.015/175.783 ms to 0.057/0.051 ms of exact inactive TRACE work. The
  post-change calibrated canonical run uses count 300, so its actual 420,000
  identity operations account for about 0.172/0.154 ms.
- Repeated static `TRACE OFF` fell from 984.315/1,020.160 ns to
  29.633/31.023 ns per operation. Real NORMAL-to-OFF transitions also improved
  42.573%/46.824% in raw elapsed while retaining the full activation path.
- The exact current ADDRESS query/set fell from 6,760.243/7,620.330 ns to
  2.065/1.720 ns. Static same-name selection improved 68.778%/69.693%, and the
  public current-name helper improved 60.103%/61.540% in raw elapsed.
- Alternating real environment changes regressed reproducibly: +3.916%
  (`rxvm`) and +2.916% (`rxbvm`) raw elapsed, with 0/12 favorable pairs for
  each VM. Control-adjusted cost per selection moved from 1,055.354 to
  1,096.960 ns and from 1,205.773 to 1,241.105 ns respectively. This keeps
  rework/revert live despite the decisive canonical win.
- The stripped canonical linked image fell 193,207 -> 112,468 bytes because
  the mislowered command path is no longer retained. The stripped stable-state
  image moved 176,152 -> 176,435 bytes; the library archive moved
  857,352 -> 857,945 bytes. RSS was not broadened into this first gate.
- An active-TRACE retained-metadata control prints one generated
  `exit_fragment` `cnop` site in both optimized and `-n` products, rather than
  the two original identity-clause coordinates. Adrian therefore selected a
  revised policy during the verdict: optimized builds may delete the exact
  identities; `-n` keeps `CNOP` and is the recommended Rexx debugging form.
  That rework is not implemented at this mandatory stop.

Adrian accepted the verdict and authorized the bounded rework: optimized
products delete only the two exact identities, `-n` retains `CNOP`, and the
alternating-ADDRESS regression must be removed before the repeat Release gate.

## Approved NR-16 rework focused gate (2026-07-23)

Implementation is frozen for the repeat Release verdict. The two certified
exits now return a private, source-comment-tagged identity CNOP. Optimized `rxc`
deletes only that exact internal replacement before AST grafting; `-n` parses
it as an ordinary CNOP. Handwritten CNOP remains in both forms. The generated
four-way stable-state RXAS contains two CNOPs in each `-n` image and none in
either optimized image. With active TRACE, both VMs report the CNOP in `-n`
and skip directly to the following BPOFF in optimized code.

ADDRESS now keeps only the current environment index as authoritative state.
A real environment change writes that one index; current-name and object
queries derive directly from the indexed normalized-name/object arrays.
Replacement and alias rows remain stable in place.

The focused gate passed 23/23 selected cells after correcting one test-only
whitespace expectation: direct ADDRESS/TRACE exits, protocol/alias/replacement,
Rexx and native host/provider controls, redirects, active command tracing, and
the opt/noopt x `rxvm`/`rxbvm` stable-state matrix. No production failure was
observed. `git diff --check` passes. No broad CTest, sanitizer, install/package,
commit or push has run.

## Repeat NR-16 Release verdict (2026-07-23)

The ordinary Release product remained `Release` with
`CREXX_VM_PROFILING=OFF`. The host was on AC power with low-power mode off and
no thermal or performance warning. One warmup per cell preceded 12 serial,
balanced and interleaved baseline/candidate rounds. All 468 valid warmup and
recorded program runs passed. Raw disposable evidence is outside the worktree
at `/private/tmp/nr16-release-rework.t5KN4d/evidence/`.

- Canonical RexxCPS rose 6,258,455 -> 28,449,018.5 CPS on `rxvm`
  (+354.569%) and 5,638,005 -> 26,534,491 CPS on `rxbvm` (+370.636%);
  all 12/12 pairs were favorable for both VMs.
- Optimized exact `TRACE VALUE _trace_current_mode()` now emits no instruction.
  Its 100,000-operation loop is within control/timer noise: control-adjusted
  medians are -0.130/-0.170 ns per source operation (`rxvm`/`rxbvm`). Thus the
  canonical TRACE identity contributes no dynamic instruction and effectively
  zero timed work; `-n` retains the source-bearing CNOP.
- Repeated static `TRACE OFF` remains at 29.440/30.995 ns per operation.
  Real transition cost improves 42.413%/46.951% in raw elapsed.
- Exact current ADDRESS query/set emits no optimized instruction and is within
  control noise at 0.115/0.200 ns. Static same-name selection improves
  68.319%/69.295%, and the public current-name helper improves
  62.193%/63.470% in raw elapsed.
- The required alternating real-environment guard still fails reproducibly.
  Median elapsed regresses 4.597% on `rxvm` and 3.042% on `rxbvm`; paired
  medians regress 4.077% and 3.159%, with only 0/12 and 1/12 favorable pairs.
  Control-adjusted cost per selection is 1,036.818 -> 1,084.510 ns and
  1,201.132 -> 1,237.707 ns respectively.
- Stripped linked canonical size is 112,644 bytes versus the 193,207-byte
  baseline. The stable-state linked images are 176,435 bytes versus 176,152.

This verdict trips the explicit governance guard. Implementation remains
provisional and frozen; no broad closeout, cleanup, commit, push or NR-17
production edit is authorized. The narrowest live rework is to restore the
normalized current name as the authoritative setter state and make the current
object index a lazy validated cache. A real change would then perform the
baseline one name write, while a later object query validates the cached row
against that name and repairs it only when stale. Revert remains the other live
option.

Adrian approved that lazy validated-cache rework on 2026-07-23. If and only if
its focused paired Release guard clears, Adrian also approved NR-17 option 2:
process-local direct-call operand binding in both `rxvm` and `rxbvm`, followed
by NR-17's own focused correctness and first paired Release verdict. No ISA,
serialized RXBIN or public ABI change is authorized.

The lazy-cache implementation is frozen after 23/23 selected focused cells
passed in Debug. The current normalized name is authoritative; known real
changes perform one registry scan and one name write, while `addressenv()`
validates and repairs the object index lazily. The focused protocol explicitly
covers alternating away/back, stale-cache repair and current-object
replacement. The ordinary profiling-off paired Release guard is next.

The paired Release guard passed on 2026-07-23. One warmup per cell preceded 12
serial balanced/interleaved rounds on AC power with no thermal or performance
warning; all 112 warmup/recorded program runs passed. Canonical RexxCPS rose
353.238%/370.544% (`rxvm`/`rxbvm`) with 12/12 favorable pairs. Same static
ADDRESS selection improved 68.549%/69.936%. The previously failing alternating
real-environment cell now improves 46.687%/48.315%, with 12/12 favorable pairs
for both VMs. Disposable evidence is at
`/private/tmp/nr16-lazy-verdict.58Dgln/evidence/`. This clears the condition in
Adrian's approved sequence, so NR-17 option 2 may begin; broad NR-16 closeout
remains deferred.

## First NR-17 Release verdict (2026-07-23)

The selected option uses each module's existing process-local execution image
in both VMs. At preparation time, only operands whose RXAS format is `P` are
resolved from their canonical procedure offsets to stable caller-local
`proc_runtime *` values. `PROC_OP` is consequently one pointer load. The
canonical RXBIN stays immutable and unchanged; dynamic calls retain their
register target; unresolved imports retain their stable stub, whose fields are
patched in place by later linking. A post-start load refreshes execution images
after `rxvm_link`, so newly loaded modules and providers remain visible. No
ISA, serialized RXBIN or public ABI contract changes.

The minimum Debug gate passed 15/15 selected tests: fixed direct calls,
unresolved-to-late-loaded imports, native calls, signal unwind in both VMs,
loadmodule, and the four NR-16/17 opt/noopt runtime cells. A separate optimized
late-load run under `rxbvm` also passed. The ordinary Release binaries are
profiling-off. The exact post-NR-16 baseline binaries were preserved before
the VM edit, and both sides ran the same retained 20-million-direct-call image
and the same freshly linked canonical RexxCPS image. One warmup per cell
preceded 12 serial balanced/interleaved pairs on AC power with no thermal or
performance warning.

- Direct-call elapsed improves by a paired median 1.161% on `rxvm`; the mean
  95% interval is -2.090% to -0.684%, with 12/12 favorable pairs. The first
  12 `rxbvm` pairs improve by a median 2.567%, interval -3.401% to -0.286%
  and 11/12 favorable. A shared slow episode tripped the absolute-span noise
  rule, so ten further pairs were retained rather than deleting an outlier.
  Across all 22, `rxbvm` improves by a median 2.417%, interval -2.779% to
  -0.851%, with 20/22 favorable.
- Canonical RexxCPS native rate improves by paired medians of 2.723% (`rxvm`)
  and 0.409% (`rxbvm`). Their mean 95% intervals are +2.083% to +3.959% and
  +0.065% to +1.104%, with 12/12 and 9/12 favorable pairs. Process elapsed
  independently improves by paired medians of 2.553% and 0.557%; both
  intervals are favorable.
- A deliberately tiny load-to-first-result-plus-teardown control was batched
  as 100 fresh processes per observation. At the 36-pair governance cap its
  approximately 2.3 ms/process result remains noisy/inconclusive. Median
  process times differ by only -0.0045 ms (`rxvm`) and -0.0037 ms (`rxbvm`),
  so neither approaches the lifecycle guard requiring both 5% and 1 ms.
- The `rxvm` allocation count is unchanged because it already owned an
  execution image. The linked RexxCPS image contains 29,502 instruction cells,
  so `rxbvm` now owns a 236,016-byte execution image for that workload.
  Measured median peak RSS changes from 8,945,664 to 8,896,512 bytes on
  `rxvm` and 8,806,400 to 8,912,896 bytes on `rxbvm`; the latter is +106,496
  bytes and is far below the greater-than-5%-and-1-MiB RSS guard. Both VM
  executable file sizes are byte-count unchanged.

This is a favorable NR-17 first verdict with no correctness, throughput,
lifecycle, RSS or artifact guard hit. The lifecycle diagnostic remains
honestly noise-bound. Production and harness work are frozen at the mandatory
stop: no broad CTest, sanitizer, install/package, closeout cleanup, commit or
push has run. Provisional raw evidence is outside the worktree at
`/private/tmp/nr17-first-release-verdict/`; the exact comparator binaries are
at `/private/tmp/nr17-direct-call-baseline.xR29kj/`.

Adrian accepted this verdict on 2026-07-23 and explicitly authorized the full
benchmark sweep on the quieter host followed by the shortest coordinated
NR-16/NR-17 closeout. Retain the exact post-NR-16 comparator binaries and run
all Release samples serially; do not substitute historical or rebuilt
comparators.

## Full paired portfolio guard (2026-07-23)

The ordinary profiling-off Release portfolio used the exact retained
post-NR-16/pre-NR-17 VM binaries and the same freshly rebuilt optimized split
workload/library images on both sides, retaining source/TRACE metadata. The
initial serial matrix covered all 11 Tier A steady-state workloads with one
warmup and 12 paired rounds: 528/528 recorded samples passed. Append-only
governance sweeps added 384 and 144 passing recorded samples for cells tripping
the absolute-noise or paired-interval rules. Across all three blocks 1,056
recorded samples passed; no observation was removed.

- The five-workload common geomean is clear adverse but within guard on
  `rxvm`: paired median -0.839% at 24 pairs, with mean 95% interval -1.718% to
  -0.195%. The `rxbvm` common geomean is clear favorable: +4.948%, interval
  +3.218% to +5.812%, at 12 pairs. Neither crosses the -1% common guard.
- Canonical RexxCPS remains clear favorable at +1.846% (`rxvm`, 12/12
  favorable) and +1.829% (`rxbvm`, 24/24 favorable) by native-rate paired
  median. List improves +8.206%/+6.565% and JSON +2.267%/+4.030%.
- Clear adverse but sub-guard individual results are `rxbvm` Sieve -2.743%,
  `rxvm` Bounce -2.641%, and `rxbvm` Mandelbrot -0.720%. Permute `rxvm` and
  both Towers engines remain honestly noisy/inconclusive at the 36-pair cap.
- `rxvm` Base64 is the sole guard hit. At the 36-pair cap its throughput paired
  median is -3.464%, with only 11/36 favorable pairs; the mean 95% interval is
  still very wide at -5.609% to +1.138%. Its three chronological 12-pair block
  medians are +0.512%, -2.777%, and -4.037%, so the observation is materially
  time-varying rather than a stable selected subset. The corresponding
  `rxbvm` Base64 result is clear favorable at +29.432% over 24 pairs.
- Retained NR-05 profiling shows the optimized Base64 control executed only
  500 `CALL_REG_FUNC_REG` instructions and attributed 0.029078% of self time
  to them. The current formal argument scales the same program, so the NR-17
  direct-call mechanism has no demonstrated multi-percent Base64 footprint.
  Together with the opposite `rxbvm` result and chronological drift, this is
  evidence against a causal direct-call regression, but governance does not
  permit the guard to be silently waived.
- The closeout summary helper hit its own repeated-allocation/call limit on the
  seventeenth summary row. An exact compiled-image discriminator failed at the
  same point on both the retained baseline and candidate VMs, excluding NR-17
  as its cause. Evidence summaries are therefore retained in two bounded
  partitions plus a separate common-geomean file.

Raw samples, manifests, bounded summary helper and capped summaries are under
`performance/evidence/2026-07-23-nr-16-17-closeout/`. The implementation and
timing evidence are frozen. The RSS portfolio, broad Debug CTest and remaining
closeout work have not run. Governance now requires Adrian to choose rework,
revert, or explicit acceptance of the noisy Base64 trade-off before closeout
continues.

Adrian explicitly accepted the noisy Base64 trade-off on 2026-07-23. The host
then moved to battery power, so no further formal performance or RSS sampling
is permitted. The accepted first-verdict RSS/artifact evidence and completed
full timing portfolio bound the production decision; closeout resumes with
ordinary correctness, retained-evidence finalization and diff review only.

After the ordinary Debug build and broad 1905/1905 CTest pass, Adrian requested
a final current-product baseline across the complete benchmark portfolio. The
host returned to stable AC power. This final campaign is an absolute baseline,
not another selection verdict: all 11 Tier A steady-state workloads on both VMs
use the formal two-warmup/ten-recorded contract, followed by separate lifecycle
and three-sample RSS dimensions.

## Exit criteria

### NR-16 TRACE

- [x] Dormant TRACE-off execution and repeated `TRACE OFF` perform no material
      Level B construction, mode normalization, metadata search, breakpoint
      scan, allocation, handler dispatch or output preparation.
- [x] Measure total canonical RexxCPS time attributable to TRACE operations and
      average cost per effectively inactive operation.
- [x] Reach approximately one cheap state test, or mechanically safe complete
      compiler elimination, on the inactive/same-state path.
- [x] Preserve OFF-to-active-to-OFF transitions, supported modes and sinks,
      hooks, breakpoint/debug behavior, source/result/command tracing,
      namespace policy, signals and exceptional unwind.

### NR-16 ADDRESS

- [x] Measure repeated same-environment selection separately from alternating
      selection and command dispatch.
- [x] Audit normalization, environment/object lookup, aliases, cached objects,
      native callbacks, and `_set_address_environment` /
      `_ensure_address_environment` independently.
- [x] Reach an ID/pointer equality ceiling on a proven already-current
      selection without weakening environment lookup or replacement behavior.
- [x] Preserve Rexx/native providers, host callbacks, redirects, EXPOSE and
      write-back, nested RXVML calls, aliases, replacements, signals and error
      behavior.

### NR-17 calls and providers

- [x] Classify ordinary direct, imported, dynamic, method, factory, native and
      unresolved calls in linked images, loose modules and post-start loading.
- [x] Record exactly which NR-17 requirements are already satisfied by accepted
      NR-04A one-time binding, bound rows, direct factory path and generation
      invalidation; make no duplicate production change for closed scope.
- [x] Prove whether any remaining linked resolved call crosses a stub,
      registry, name/signature lookup, module/procedure scan or redundant
      selector on repeated execution.
- [x] Resolved success paths bypass redundant stubs/lookups while signature
      validation, native/plugin providers, unresolved fallback and visibility
      after late loading remain correct.

## Mechanically separate current-path audits

### A. TRACE path ledger

| Question | Optimized | `-n` | `rxvm` | `rxbvm` | Retained/stripped | Evidence / disposition |
| --- | --- | --- | --- | --- | --- | --- |
| Ordinary per-instruction cost while inactive | control 0.014 us/op | control 0.016 us/op | generic interrupt poll only | generic interrupt poll only | both | Inactive execution with no TRACE clause has no TRACE handler/metadata path; ordinary dispatch retains the general signal/interrupt poll. |
| Repeated `TRACE OFF` while already off | 0.974/0.988 us/op | 0.996/1.040 us/op | 0.974 opt / 0.996 `-n` | 0.988 opt / 1.040 `-n` | stripped focused; retained control | Three emitted `BPOFF` paths plus `_trace_set`, `_trace_current_mode`, reset helper and activation logic; first call constructs `.tracecontroller`. |
| Real OFF -> active -> OFF transition | 2.894/3.250 us/instruction | 2.949/3.064 us/instruction | 2.894 opt / 2.949 `-n` | 3.250 opt / 3.064 `-n` | retained behavior required | Genuine transition deliberately retains normalization, controller state and breakpoint activation fallback. |
| Compiler/RXAS/Level B/runtime/breakpoint work | same in opt/`-n` | same in opt/`-n` | computed-goto `BPOFF` clears mask | switch `BPOFF` clears mask | retained 1.189 us; stripped 1.169 us | Dormant cost is Level B call/normalization/reset work, not a metadata or breakpoint scan. `BPOFF` itself is the cheap ceiling. |

### B. ADDRESS path ledger

| Question | Optimized | `-n` | `rxvm` | `rxbvm` | Rexx/native | Evidence / disposition |
| --- | --- | --- | --- | --- | --- | --- |
| Repeated same-environment selection | static 0.966/1.129 us; intended public query 2.307/2.664 us | same shape as opt | measured separately | measured separately | protocol fixtures cover both | `_set_address_environment` normalizes, ensures/scans, then scans again; `_address_environment` also ensures/scans twice before public method dispatch. |
| Alternating environments | 1.029/1.191 us per change | same shape as opt | measured | measured | provider objects cached | Expected real change still pays normalization and scans but does not reconstruct cached objects. |
| Command dispatch without environment change | isolated fixture retained | isolated fixture retained | covered | covered | Rexx/native focused controls passed | Request/redirect/provider/TRACE-hook work belongs to command dispatch, not selection. The current RexxCPS form accidentally takes this path. |
| Normalization/cache/alias/replacement work | linear name arrays and cached objects | same | same Level B module | same Level B module | aliases/replacement preserved | Current name is a normalized string, not an ID/object slot. Registration replaces the object in-place; aliases may name the same object, so an ID/pointer design needs explicit replacement rules. |

### C. Call/provider path ledger

| Call kind | Linked image | Loose module | After late load | Native/plugin | Remaining repeated work |
| --- | --- | --- | --- | --- | --- |
| Ordinary local direct | operand is constant-pool procedure offset | same loader path | runtime object address remains stable | n/a | Every `CALL*` executes `PROC_OP`, a binary search of the module's sorted offset-to-`proc_runtime*` table. |
| Direct imported bytecode | caller-local stub is cold-patched once | same | unresolved stub is patched when provider arrives | n/a | No name/registry lookup or trampoline after link, but the same per-call binary search locates the patched stub. |
| Dynamic `dcall` | already consumes `proc_runtime*` from a register | same | producer/selector controls invalidation | supported | No redundant direct-call lookup in `DCALL`; retain dynamic fallback. |
| Method selector | NR-04A bound callable table plus generation-guarded 2-way site cache | graph materialized on load | cache invalidates on generation | Rexx providers retained | Satisfied by NR-04A; selected target is a direct `proc_runtime*` and `DCALL` consumes it. |
| Factory selector | bound provider rows, generation cache and direct single-provider/no-match target | graph materialized on load | bindings rebuild and generation advances | plugin/provider registry retained | Satisfied by NR-04A, including general scored fallback. |
| Native direct/imported | caller stub patched to native function pointer | same | unresolved native stub can be patched later | supported | Per-call `PROC_OP` binary search remains, then `rxvm_callfunc` receives the native pointer directly. |
| Unresolved fallback | `start == SIZE_MAX` signals function not found | same | later link patches the stable stub | supported | Must not cache a failure; caching the stable stub itself is safe because its fields are updated in place. |

## Retained evidence audit

- [x] Reuse the accepted NR-04A graph/binding measurements and late-load tests;
      do not repeat valid binding or approximately-control-cost graph cells.
- [x] Reuse NR-05 call-census path/kind/arity/site/outcome rows before adding
      instrumentation. Document any current-product mismatch that makes a
      bounded refresh necessary.
- [x] Audit current post-NR-15 canonical RexxCPS source/image hashes before
      reusing older elapsed or instruction evidence.
- [x] Keep profiling self-time as prioritization evidence; confirm any selected
      change with ordinary profiling-off Release wall time.

Accepted NR-04A already satisfies the NR-17 method/factory/provider scope:

1. the sealed RXBIN 007 semantic graph is materialized once;
2. portable callable IDs bind once to `proc_runtime*` per semantic generation;
3. dispatch and provider rows are process-local bound rows;
4. method/factory sites use generation-guarded caches;
5. single-provider/no-match factories take a direct bound target; and
6. late loading rebuilds bindings and advances the semantic generation.

No production duplication is selected for those requirements. The only open
NR-17 steady-state scope is ordinary `CALL*` operand resolution. A disposable
C ceiling PoC measured a 64-entry optimized binary lookup at 2.458 ns/op versus
0.229 ns for a prebound pointer and 0.244 ns for a dense ordinal; with 256
entries it was 3.828/0.241/0.246 ns. At `-O0`, 64 entries were
9.869/1.084/1.169 ns. Today's schema-4 RexxCPS smoke refresh observed 661,480
direct bytecode calls, 28,021 dynamic bytecode calls, 81 fresh and 689,421
reused frames at count 10, plus 100 distinct executed direct-call sites. The
canonical direct-call footprint is therefore about 6.6 million calls, making
the 64-entry pointer ceiling worth only about 15 ms before end-to-end proof.

## Candidate panel and machine-level ceilings

The exact inline controls are: one already-off bit/state test, one current-ID or
pointer equality test, and one direct bound-target load. A stable success path
that allocates, normalizes strings, traverses a graph, scans metadata, performs
name lookup or rebinds a portable reference fails the design ceiling unless
evidence proves that work unavoidable.

For each applicable candidate record dynamic instructions/calls/lookups/
allocations, steady elapsed, startup/link/load/teardown, image and retained
memory, eager/lazy/purpose-built preparation, invalidation/failure behavior,
both VMs, optimized/`-n`, retained/stripped metadata, and late/native/plugin
behavior.

| ID | Candidate | TRACE | ADDRESS | NR-17 | Required evidence | Disposition |
| --- | --- | --- | --- | --- | --- | --- |
| A | Status quo / evidence-only closure | fails at 1.0-1.3 us inactive | fails; also exposes `ADDRESS VALUE` mislowering | selected closure for all accepted NR-04A method/factory/provider scope | exact current-path audit and retained evidence mapping | partial winner only for NR-04A scope |
| B | Compiler/RXAS site-specific elimination or lowering | source-bearing NOP is the measured zero-work ceiling for the exact private identity form | first restore `VALUE` parsing; exact public identity elimination has provider-method observability risk | no blanket direct-call elimination | optimized/`-n`, source/TRACE, calls/references and authored RXAS negatives | fastest NR-16 ceiling; requires semantics approval |
| C | Runtime idempotent fast path | Level B ready/active guard can avoid construction/normalization but still pays a frame/call | early normalized-name equality removes two scans; cached current slot/object can narrow query | stable caller-local import stub can be cached directly with no late-load invalidation | inline ceiling, ownership, invalidation and late-load proof | recommended fallback/general path after B |
| D | Link/load-time prebinding or narrower operands/tables | no advantage over B/C | no stable linker ownership for mutable environment state | patch `rxvm`'s existing execution operand image; `rxbvm` needs a new shadow image or sidecar | link/load cost, image/memory and plugin/late-load proof | selected NR-17 winner; direct calls and canonical RexxCPS improve in both VMs |
| E | Bounded combination | exact identity lowering plus guarded full transition | parser correction plus cached normalized state/full replacement fallback | NR-04A closure plus optional direct-call operand prebinding | must beat the simpler candidate end to end | selected coordinated architecture with separate NR-16/NR-17 ownership |

## Recommendation and requested decisions

1. Correct `ADDRESS VALUE expression` parsing first. This restores the already
   documented/intended language form and is a correctness repair, not a new
   syntax decision.
2. For NR-16, select B+C: emit a source-bearing no-op for only the mechanically
   exact private TRACE identity form, add a no-construction/already-off guarded
   fallback for static `TRACE OFF`, retain the full path for every real mode
   change, and cache ADDRESS's current normalized slot/object with an early
   same-name test. The open language decision is whether the exact public
   `addressenv().environment_name()` identity form is contractually pure enough
   to eliminate evaluation; if not, it must retain provider method dispatch and
   cannot reach the NOP ceiling.
3. Close NR-17's accepted NR-04A provider/selector scope without code. For the
   remaining direct calls, the fastest general implementation is a process-local
   operand image that binds function operands to the stable caller-local
   `proc_runtime*`. `rxvm` already owns such an image; `rxbvm` would gain a
   canonical-bytecode shadow image so both VMs share the direct pointer load.
   Late load remains safe because unresolved stubs are patched in place. Adrian
   must decide whether the roughly 2.2 ns/call ceiling gain justifies the extra
   `rxbvm` instruction-image memory and preparation scan.

If approved, keep two verdicts inside this coordinated activity: implement the
NR-16 parser/state work first, run only focused correctness, then freeze and
report the canonical RexxCPS plus stable-state first Release verdict. Only after
that verdict is accepted should the independent NR-17 operand-image change be
implemented and measured against its direct-call workload and canonical
call-heavy control. This keeps ownership and invalidation separate as required.

## Focused fixtures and workloads

- [x] Dormant TRACE-off loop with no executable TRACE transition.
- [x] Repeated `TRACE OFF`, plus real OFF-to-active-to-OFF mode changes.
- [x] Repeated same ADDRESS selection and alternating environments.
- [x] Rexx and native ADDRESS callbacks, including replacement/alias control.
      The focused current-product gate passed 24/24 across the new matrix,
      native/CMS/LLM/provider callbacks, protocol/CREXX redirects and TRACE
      normal/command behavior.
- [x] Direct local/imported linked calls and every remaining provider/selector
      case identified by the audit.
- [x] Late-loaded provider and unresolved-before/resolved-after control reused
      from accepted NR-04A evidence; source audit confirms stable stub patching.
- [x] Canonical RexxCPS with direct total and per-inactive-TRACE attribution.
- [x] At least one decisive end-to-end workload with a demonstrated
      mechanism footprint.
- [x] Performance samples serial; canonical workloads unchanged; diagnostics
      separately named; `rxvm` and `rxbvm` reported independently.

## Numbered execution plan and stop gates

1. [x] Verify exact Git state and read required governance and technical
   sources.
2. [x] Create this worklist and mark NR-16/NR-17 in progress in the live
   roadmap before production work.
3. [x] Audit retained NR-04A/NR-05 evidence and current TRACE, ADDRESS and
   call/provider paths; close satisfied NR-17 scope honestly.
4. [x] Build minimal correctness fixtures and current-path measurements, using
   retained call-census/profile facilities before new instrumentation.
5. [x] Run disposable or default-off candidate comparisons against the exact
   inline ceilings and fill the complete design panel.
6. [x] Present the required first report: baseline, audits, NR-04A closure,
   footprint, candidate panel, recommendation, architecture decision and next
   gate. This worklist is the retained report; the chat handoff accompanies it.
7. [x] The selected NR-16 identity semantics and independent NR-17 operand
   image architecture were approved by Adrian on 2026-07-23.
8. [x] Present the numbered production-edit plan, implement the
   smallest general selected form, and run minimum focused correctness only.
9. [x] Freeze immediately after each approved production slice survives
   focused correctness, build ordinary profiling-off Release, run the smallest
   decisive paired end-to-end comparison against valid retained evidence, and
   stop for Adrian. Adrian accepted both first verdicts.
10. [x] The accepted full timing sweep completed and its capped `rxvm` Base64
    guard was explicitly accepted as a noisy, non-causal trade-off. No further
    formal performance/RSS run will start while the host is on battery.
11. [x] Complete the ordinary Debug build and broad CTest. The first pass found
    only three stale `_address.crexx` source-coordinate goldens; after their
    exact +19-line updates, focused 3/3 and broad 1905/1905 passed.
12. [x] Retain the final all-benchmark absolute timing, lifecycle and RSS
    baseline requested by Adrian, then finalize evidence/control-plane status
    and review the complete diff. The final campaign retained 500/500 passing
    rows: 44 timing warmups, 310 recorded timing samples, 66 RSS samples and 80
    lifecycle phase rows. The historical same-laptop comparison is recorded in
    the evidence README.
13. [x] No commit or push was performed. Commit remains available only on
    Adrian's explicit request; push remains unauthorized.
