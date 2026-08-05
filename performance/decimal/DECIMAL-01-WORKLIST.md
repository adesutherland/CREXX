# DECIMAL-01 decimal correctness and performance worklist

Status: **focused repair accepted; Gate 1 baseline preparation open; no timing reservation or candidate selected**

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
Timing authority remains conditional on a newly confirmed exclusive-host
reservation.

## 2. Shared-host reservation rule

This Mac is shared with two other performance agents.

- [x] Record that focused correctness builds/tests may run but their elapsed
  time is not performance evidence.
- [ ] Before every timing session, ask Adrian to clear and reserve the host.
- [ ] Wait for explicit confirmation before starting any performance cell.
- [ ] After confirmation, audit for competing benchmark, compiler, build and
  test processes and capture power/thermal/load state.
- [ ] Run formal samples serially. If competing work appears or Adrian asks for
  a pause, stop before the next cell and record the exact checkpoint.
- [ ] Treat samples exposed to competing work as invalid rather than attempting
  to correct them statistically.

No DECIMAL-01 timing is currently authorized or running.

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

### Gate 2 first candidate panel

Queued candidates are tuned `decNumber`, `libmpdec` and fixed-34 `decQuad`.
No candidate source or build has been opened.

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

Current checkpoint: Adrian accepted the focused repair and opened Gate 1
baseline preparation. Complete the remaining operation/semantic inventory,
broaden authoritative vectors around the repaired findings, cover the remaining
invalid individual-setter forms and Classic quotient rule, and construct the
independent D0/D1 workload harness. These are correctness/setup actions, not
performance runs. Before the first timing cell, ask Adrian to clear and reserve
the host and wait for explicit confirmation.
