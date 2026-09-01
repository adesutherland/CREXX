# DECIMAL-01 decimal correctness and performance worklist

Status: **Stage 3 complete; D2 tuned decNumber, D3 decQuad and D4 libmpdec
rejected; retain current mc_decimal; no production change selected**

Opened: 2026-08-05

Control plan:
[`DECIMAL-01-ENGINEERING-PLAN.md`](DECIMAL-01-ENGINEERING-PLAN.md)

## 1. Authority and stop boundary

Adrian approved the complete seven-step gated engineering plan on 2026-08-05
and authorized Gate 0 correctness work. This slice may:

- audit current compiler, RXAS, VM and decimal-provider behaviour;
- add focused correctness fixtures and evidence records;
- map official decimal vectors and public evidence; and
- update this worklist, the engineering plan and the live roadmap.

It does not select or implement a candidate decimal provider, change the
default provider, change the public RXAS/RXBIN or plugin ABI, alter language
semantics, or start a performance campaign. Adrian subsequently authorized the
bounded repair of the three confirmed provider-consumption defects before any
further work. That repair does not open broader production or candidate
authority. Adrian accepted its focused verdict on 2026-08-05 and directed the
programme to the second stage, Gate 1 current-provider baseline preparation.
Timing authority remained conditional on a newly confirmed exclusive-host
reservation. Adrian confirmed the host clear and approved execution through
the bounded alternative-library materiality decision on 2026-08-18. This does
not authorize production provider selection or integration.

## 2. Shared-host reservation rule

This Mac is shared with two other performance agents.

- [x] Record that focused correctness builds/tests may run but their elapsed
  time is not performance evidence.
- [x] Before this timing session, ask Adrian to clear and reserve the host.
- [x] Receive Adrian's 2026-08-18 explicit confirmation before starting any
  performance cell.
- [x] After confirmation, audit for competing benchmark, compiler, build and
  test processes and capture power/thermal/load state.
- [x] Run the retained formal samples serially. If competing work appears or Adrian asks for
  a pause, stop before the next cell and record the exact checkpoint.
- [x] Treat samples exposed to competing work as invalid rather than attempting
  to correct them statistically.

DECIMAL-01 Gate 1 timing and the bounded Gate 2 candidate screen are
authorized. Every timing block still requires a fresh environment audit and
must stop if the reservation or host state changes.

## 3. Verified opening baseline

- [x] Repository: `/Users/adrian/CLionProjects/CREXX`.
- [x] Source baseline: `develop` at `4813e98d1`, equal to `origin/develop`
  before the DECIMAL-01 documentation changes.
- [x] Host: Darwin ARM64, Apple M5, 10 logical CPUs, Apple Clang 21.0.0.
- [x] Current provider inventory: static/default `mc_decimal`; separately
  loadable `db_decimal`.
- [x] Current `mc_decimal` configuration: `DECDPUN=8`, `DECUSE64=1`,
  `DECNUMDIGITS=64`, `DECBUFFER=64`, using base-10^8 `uint32_t` limbs and
  64-bit intermediates.
- [x] Current Apple `db_decimal` substrate: 8-byte, 53-bit-mantissa
  `long double`, `LDBL_DIG=15`.

## 4. Gate 0A — exact semantic and operation inventory

- [ ] Enumerate every `decplugin` entry, input/output ownership rule, diagnostic
  mutation and allocation path.
- [ ] Map every decimal RXAS opcode and VM-composed operation to provider calls.
- [ ] Record register/register, register/literal, aliasing and missing-value
  forms.
- [ ] Map decimal conversions, formatting and comparisons to the five-field
  numeric context.
- [ ] Map compiler lowering for decimal `%`, `//`, comparisons and power.
- [ ] Record frame entry/return, inherited context, plugin loading and teardown
  ownership.

## 5. Gate 0B — RXAS numeric-context instruction report

Adrian reported that the RXAS digits and related instructions may not be
working and noted that normal compiler output uses a combined instruction.
This remains an open current-product question until the complete matrix below
passes or produces a defect record.

### 5.1 Current source finding

- [x] Identify the individual surface: literal/register `SETNUMDGTS`,
  `SETNUMFUZ`, `SETNUMFRM`, `SETNUMCAS`, `SETNUMSTD`, plus five `GETNUM*`
  instructions.
- [x] Identify the combined surface: `NUMSCI digits,case,standard` and
  `NUMENG digits,case,standard`; both set fuzz zero and synchronize the decimal
  provider once.
- [x] Confirm compiler selection: combined setup is emitted only for a complete
  compile-time context with fuzz zero, valid form/case/standard and digits at
  least five. Inherited fields, nonzero fuzz and digits 1-4 retain individual
  setters.
- [x] Confirm existing direct RXAS decimal coverage tests only the digits
  setter/getter pair; other individual fields are not covered there.
- [x] Confirm the compiler-owned `nr09_numeric_context_contract` checks
  optimized/no-opt combined emission, setting readback and a division result
  under both VMs and default/explicit providers, but does not prove the full
  individual/combined equivalence matrix.

### 5.2 Fresh opening test

The current binaries were rebuilt and the focused current suite was run:

```text
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(nr09_numeric_context_contract|rxasdecimaltests|rxasdecimaltests-rxbvm|rxasdecimaltests-mc|rxasdecimaltests-mc-rxbvm|rxasdecimaltests-db|rxasdecimaltests-db-rxbvm|mc_decimal_full_tests|db_decimal_tests)$'
```

- [x] Result: 9/9 passed on 2026-08-05.
- [x] Interpretation: useful current baseline, not disposition of the report.
- [x] Do not use the CTest elapsed time as performance evidence.

### 5.3 Required instruction proof

- [x] Extend the hand-authored RXAS decimal fixture for all ten individual
  setter forms and all five getters.
- [x] Add direct `NUMSCI` and `NUMENG` state/readback plus precision-effect
  coverage.
- [x] Compare combined and five-setter state plus exponent-formatting results
  from identical starting contexts.
- [ ] Complete the authoritative observable-effect matrix for digits, fuzz
  comparison, form, case and standard, not only stored integer readback. The
  opening cases cover all five settings: digits and `mc_decimal` form/case/
  standard pass, while `FUZZ` and the tested `db_decimal` form/case/Common
  cases are classified failures; broader boundaries remain open.
- [x] Prove invalid combined digits, case and standard operands signal before
  partial context mutation.
- [ ] Prove combined setup synchronizes the selected decimal provider exactly
  once where a diagnostic counter can be added without contaminating product
  semantics; otherwise use observable post-sync arithmetic plus source proof.
- [ ] Cover nested procedure context inheritance and restoration at Level B;
  direct RXAS child/caller isolation passes in the opening diagnostic.
- [x] Run optimized and no-opt compiler paths.
- [x] Run `rxvm` and `rxbvm` with default, explicit `mc_decimal` and explicit
  `db_decimal`.
- [x] Keep expected provider divergences separate from instruction failures.
- [x] Reproduce failures in a small separately named RXAS diagnostic, classify
  them, and stop before any fix.

The expanded fixture adds tests 59-66 to
`interpreter/tests/tests_decimal.rxas`. The rebuilt 9-test focused selection
passes again under both VMs and all three provider-selection modes. This
dispositions basic setter/getter storage, combined state installation,
precision synchronization, combined/expanded formatting parity and combined
pre-mutation validation as working on the current Mac build. It does not yet
complete the authoritative semantic matrix, but the separate observable
diagnostic dispositions its opening cases: it confirms the `FUZZ` defect for
all providers and the tested `db_decimal` form/case/Common-standard failures.

## 6. Gate 0C — current semantic-gap ledger

Each row must close as `conformant-current`, `confirmed-current-defect`,
`document-defect` or `deliberate-documented-limitation`.

| Finding | Current opening status | Required proof |
| --- | --- | --- |
| RXAS individual and combined numeric context | basic state path conformant; observable report partly confirmed in provider consumption | finish individual invalid forms and authoritative semantics |
| `NUMERIC FUZZ` consumption in decimal comparisons | **focused repair green** under both VMs and providers, including register/register and register/literal equal/distinct boundaries | broader vector boundary coverage remains |
| Common versus Classic provider rounding/clamp | **focused repair green** for `db_decimal` positive/negative and odd/even ties; `mc_decimal` remains conformant in the same panel | exponent/status-vector matrix remains |
| Form/case under `db_decimal` | **focused repair green**: shared formatter produces scientific/upper and engineering/lower results | broader thresholds and special values remain |
| Classic `%`/`//` magnitude constraint | VM/compiler comments indicate unfinished enforcement | language contract plus focused generated/RXAS execution |
| diagnostic/signal state | mutable provider context | exact status, clearing and partial-result matrix |
| untagged provider values | representations cannot safely mix | lifecycle/plugin-isolation proof and explicit limitation |
| provider instance concurrency | shared mutable factory instance | ownership audit; concurrency is not inferred from serial tests |
| Apple `db_decimal` default precision | 15 available versus cREXX default 18 | confirm as deliberate diagnostic limitation or defect |

Adrian authorized repair of the confirmed defects on 2026-08-05 before any
further candidate or performance work. Gate 0 remains open until focused
Debug and ordinary profiling-off Release correctness proof passes.

### 6.1 Approved bounded repair design

- [x] Keep `FUZZ` consumption inside each provider's existing compare and
  compare-string entry points. A VM-side tolerance would duplicate decimal
  representation and rounding rules outside their owner. Each comparison
  rounds temporary operands to effective `DIGITS - FUZZ`; stored arithmetic
  values and the arithmetic precision remain unchanged.
- [x] For `mc_decimal`, use temporary `decNumber` values under a copied
  comparison context. Do not mutate either operand or the frame's arithmetic
  precision.
- [x] For `db_decimal`, make context rounding explicitly select Common
  half-even or Classic half-up. Do not depend on the process floating-point
  rounding mode. The binary provider remains a diagnostic/qualified control,
  not a claim of arbitrary-decimal equivalence.
- [x] Route `db_decimal` text through its existing normalized
  coefficient/exponent extractor and the framework-owned REXX formatter.
  Ad-hoc `%LE`/`%LG` branching is rejected because it would duplicate form,
  case and simple/exponential threshold rules.
- [x] Preserve the decimal plugin ABI, RXAS instruction encodings, stored
  numeric-context representation and current signal ownership.
- [x] Prove the focused repair under both VMs with default, explicit
  `mc_decimal` and explicit `db_decimal`, then repeat the focused suite in an
  ordinary profiling-off Release build. Do not run a performance cell without
  a new Adrian-confirmed host reservation.

Focused Debug and ordinary profiling-off Release each pass 9/9. The separate
observable program passes 6/6 in each build across `rxvm`/`rxbvm` and default,
explicit `mc_decimal` and explicit `db_decimal`. Evidence:
[`2026-08-05-decimal-01-numctx-repair-verdict`](../evidence/2026-08-05-decimal-01-numctx-repair-verdict/).
Adrian accepted this focused correctness verdict on 2026-08-05 and directed
the programme to Gate 1 baseline preparation. No broad CTest, sanitizer,
candidate work or performance timing has started.

## 7. Gate 0D — correctness corpus and oracle

- [ ] Inventory applicable General Decimal Arithmetic `decTest` vectors by
  provider operation.
- [ ] Map unsupported operations explicitly rather than counting them as
  passes.
- [ ] Add cREXX Common/Classic, text, signal and procedure-context cases not
  represented by the upstream vectors.
- [ ] Freeze current `mc_decimal` transcripts for compatibility detection only.
- [ ] Define differential and metamorphic cases for every candidate.
- [ ] Include digits 5, 9, 18, 34, 50, 64, 100 and 1000 where supported.
- [ ] Include all five context settings, boundary/special values and every
  provider operation.

## 8. Gate 0E — deliver and stop

- [x] Retain the opening RXAS numeric-context evidence bundle at
  [`2026-08-05-decimal-01-rxas-numctx-opening`](../evidence/2026-08-05-decimal-01-rxas-numctx-opening/).
- [ ] Retain the final consolidated Gate 0 evidence bundle with provenance,
  commands and raw correctness output.
- [ ] Complete the current-gap disposition ledger.
- [x] Update the engineering plan and live roadmap only after the proof is
  green or the exact failures are recorded.
- [x] Present confirmed defects and the focused repair verdict to Adrian.
- [x] Stop for Adrian's repair-verdict acceptance before broad closeout or any
  Gate 1 timing.
- [x] Record Adrian's 2026-08-05 acceptance and direction to open Gate 1
  baseline preparation.

## 9. Next gates

### Gate 1 current-provider baseline

Preparation is open for D0 `mc_decimal`, D1a unrestricted `db_decimal`
diagnostic ceiling, and D1b individually qualified Classic-9 `db_decimal`
speed control. Gate 1 measurement requires a newly confirmed exclusive host
reservation. Harness construction, workload qualification and retained-command
design are not performance evidence and may proceed before reservation.

- [x] Freeze the D0s/D0d/D1a/D1b cell boundary in
  [`GATE1-CELL-MATRIX.md`](GATE1-CELL-MATRIX.md).
- [x] Implement fixed-work Common-18 and Classic-9 Level B workloads with a
  runtime opaque seed and deterministic checksums.
- [x] Implement the L1 direct decimal-plugin ABI payload without internal
  timing or statistics.
- [x] Qualify both VMs, opt/no-opt images, default/explicit providers, the L1
  payload, RexxCPS decimal-string controls and optimizer-integrity checks:
  81/81 selected Release CTests pass; elapsed values are discarded.
- [x] Create a three-way optimizer boundary: compiler-on/RXAS-on product,
  identical compiler-on/RXAS-off isolation control, and compiler-off/RXAS-off
  broad diagnostic.
- [x] Disassemble all six final context/mode RXBIN images and require them to
  retain runtime parse/format, arithmetic and decimal comparisons rather than
  constant-folding or assembler-optimizing away the opaque operands.
- [x] Admit all six Classic-9 `db_decimal` modes after exact D0/D1b checksum
  agreement; retain Common-18 `db_decimal` mismatches as D1a diagnostics.
- [x] Define the independently authored, publishable
  [`CREXX Decimal Benchmark (CDB-1)`](CREXX-DECIMAL-BENCHMARK.md); its Gate 1
  kernels form the arithmetic core.
- [ ] Add the original billing application extension before the first public
  CDB-1 result; public Telco material may inform its shape but restricted source
  or data is not copied.
- [ ] Run the canonical RexxCPS provider-row guard after the host is reserved.
- [x] Calibrate and capture the retained Gate 1 L1 adapter and L2/L3 product
  blocks after Adrian explicitly confirmed exclusive host availability.

The current-provider capture is retained at
[`2026-08-18-decimal-01-gate1-current-provider`](../evidence/2026-08-18-decimal-01-gate1-current-provider/).
The prepared optimizer-control and canonical RexxCPS guard manifests were not
run in that bounded capture and remain explicitly open rather than inferred
from checksum identity.

### Gate 2 first candidate panel

Queued candidates are tuned `decNumber`, `libmpdec` and fixed-34 `decQuad`.
Adrian authorized this bounded panel through its materiality decision on
2026-08-18. Candidate sources/builds remain isolated outside the repository;
no production provider selection or integration is authorized. The screening
and final materiality thresholds are fixed in
[`PERFORMANCE-CLOSEOUT-PLAN.md`](../PERFORMANCE-CLOSEOUT-PLAN.md): a candidate
must show credible L1 headroom before L2/L3 work, and only a
correctness-qualified result of at least 10% across the predeclared
representative L3 boundary can be called material.

#### D4 libmpdec 4.0.1 disposition

- [x] Pin the official 4.0.1 archive and SHA-256
  `96d33abb4bb0070c7be0fed4246cd38416188325f820468214471938545b1ac8`;
  record the BSD-2-Clause licence and supported toolchain/platform boundary.
- [x] Build the upstream static library externally and pass its complete local
  `check_local` suite. The downloadable extended test corpus was not run
  because the upstream helper required unavailable `wget`.
- [x] Implement an opt-in, non-default adapter with pointer-free VM-owned
  payloads, temporary `mpd_t` views and no plugin-ABI or production-provider
  change.
- [x] Pass the existing full decimal-provider contract, exact numeric
  arithmetic/conversion/comparison/context checks and a dedicated raw-copy,
  destroy-source and in-place-arithmetic lifecycle proof.
- [x] Capture the formal L1 screen: one warmup plus 12 balanced/interleaved
  recorded pairs, 260 passing samples and no removed observation.
- [x] Double-check the adverse arithmetic result with a lean-capacity adapter
  and a direct decNumber/libmpdec core comparator.
- [x] Reject D4 at L1. The formal adapter loses 41.66% and 26.28% arithmetic
  throughput in Common-18 and Classic-9, and the direct-core diagnostic still
  loses 21.90% and 5.30% after matching libmpdec's operation capacity. Only
  comparison clears the 15% improvement threshold;
  the required two representative modes do not. Do not run L2/L3.

Evidence:
[`2026-08-18-decimal-01-libmpdec-screen`](../evidence/2026-08-18-decimal-01-libmpdec-screen/).
The unexecuted D2 tuned-decNumber and D3 fixed-34 decQuad routes remain recorded
as possible future experiments; they are not part of the narrowed D4 verdict.

#### D2 tuned decNumber progress

- [x] Record Adrian's 2026-08-18 direction to continue the remaining Stage 3
  candidates after reviewing D4.
- [x] Define the complete isolated 48-build grid: `DECDPUN` 3/4/8/9,
  `DECNUMDIGITS` 18/34/64 and `DECBUFFER` 20/36/64/128.
- [x] Keep every grid point as a composed build and retain the current 8/64/64
  provider as the same-session control; do not synthesize an unmeasured winner.
- [x] Build all 48 grid plugins and pass all 48 full provider-contract checks
  in the isolated profiling-off Release tree. Confirm the ordinary current
  provider remains byte-identical to the prior Release candidate-control
  build.
- [x] Run the low-cost adapter calibration after Adrian's fresh clear-host
  confirmation: one warmup plus eight balanced rounds, 882/882 guarded samples
  passing. Reject the first attempt in full because XProtect overlapped its
  final 11 seconds; retain only the subsequent clear-host capture as evidence.
- [x] Apply the progression rule. The best balanced complete build
  (`DECDPUN=8`, `DECNUMDIGITS=18`, `DECBUFFER=64`) is 0.23%/1.67% slower than
  current at Common-18/Classic-9. The largest one-context gain is only 2.70%
  and carries a 1.74% loss in the other context. No build has credible 10%
  headroom or meets the two-mode 15% rule; stop before formal L1 and L2/L3.

#### D3 fixed-34 decQuad progress

- [x] Add an opt-in, non-default comparator using the already-vendored
  `decQuad` and `decContext` sources; retain one pointer-free 16-byte decimal128
  value directly in VM-owned sidecar storage.
- [x] Preserve Common-18 and Classic-9 arithmetic, conversion, comparison and
  context-sync checksums against `mc_decimal`; keep the copy/clear byte count
  representation-specific (16 bytes for decQuad versus 44 for current
  decNumber).
- [x] Pass the dedicated raw-copy, destroy-source and in-place-arithmetic
  lifecycle proof. A diagnostic run of the broader provider contract also
  passed every assertion before it requested unsupported 50- and 200-digit
  contexts; the remaining failures are retained as the explicit fixed-34
  boundary, not counted as passes.
- [x] Record that decQuad has no power primitive. The comparison adapter
  composes only bounded integral-exponent power from decQuad multiply/divide;
  this is **not a decQuad capability** and is not part of the D3 timing kernel.
- [x] Add low-cost adapter and direct-core calibration manifests. Do not run
  them until a fresh clear-host confirmation.
- [x] Run the checksum-gated D3 adapter and direct-core calibration after
  Adrian's clear-host confirmation: 180/180 adapter samples and 36/36 core
  samples pass their guards with no observation removed.
- [x] Reject D3 before a formal screen. Adapter arithmetic is 44.40%/64.76%
  slower at Common-18/Classic-9, and direct-core arithmetic is 49.28%/66.73%
  slower. Its 16-byte representation improves only isolated copy/clear and
  context-sync cells; conversion and comparison remain adverse. Per Adrian's
  stated arithmetic stop rule, do not open formal L1 or L2/L3.

D3 remains a rejected fixed-precision comparator, not an arbitrary-precision
replacement. Contexts above 34 digits and non-integral power are unsupported
by this candidate and are labelled as such. The retained D2/D3 evidence is
[`2026-08-18-decimal-01-stage3-calibration`](../evidence/2026-08-18-decimal-01-stage3-calibration/).

### Gate 3 public evidence and extended candidates

Before any extended candidate is opened, retain a source-attributed dossier
covering exact versions, dates, semantics, workloads, hardware, compiler
flags, source/raw-data availability, correctness suites, licences, platform
support, maintenance and material issue reports.

The preliminary orientation is retained in
[`PUBLIC-EVIDENCE-ORIENTATION.md`](PUBLIC-EVIDENCE-ORIENTATION.md). Initial
sources are:

- Cowlishaw's historical `decNumber`/`decQuad` operation tables;
- mpdecimal's cross-library Mandelbrot results and testing account;
- Anderson et al.'s 2009 peer-reviewed decimal-library benchmark suite; and
- Boost.Decimal's current reproducible operation and conversion harness.

These sources inform experiment priority only. Their results are from
different eras, implementations, APIs and hosts and are not CREXX evidence.

### Gates 4-6

Complete-plugin selection, production integration/first Release verdict and
cross-platform closeout remain closed and separately stopped as specified in
the engineering plan.

## 10. Resume checkpoint

Current checkpoint: Stage 3 is complete. D4 libmpdec was rejected at formal L1;
D2 tuned decNumber and D3 fixed-34 decQuad were rejected by valid clear-host
calibration because neither showed credible progression headroom. Retain the
current 8/64/64 `mc_decimal`; no provider, ABI or production change is selected.
DECIMAL-01 has no further authorized action. The overall performance closeout
now treats Stage 2 as complete: the unchanged concurrency replay is waived and
RexxCPS/optimizer-integrity qualification has moved into the authorized Stage
4 benchmark-portfolio and source-quality preparation.
