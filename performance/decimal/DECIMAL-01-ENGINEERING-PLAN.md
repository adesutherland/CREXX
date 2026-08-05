# DECIMAL-01 decimal backend performance engineering plan

Status: **approved; focused repair accepted; Gate 1 harness qualified and awaiting host reservation; no candidate selected**

Date: 2026-08-05

## 1. Decision sought

Determine the fastest decimal-provider architecture that preserves cREXX's
documented numeric semantics across supported macOS, Linux and Windows builds.
The result may be:

1. a better-tuned Mike Cowlishaw `decNumber` provider;
2. a different arbitrary-precision decimal library;
3. a fixed 34-digit decimal fast provider with a correctness-preserving
   arbitrary-precision fallback;
4. a new native-64-bit implementation of the existing algorithm;
5. a platform-specific acceleration behind one portable semantic contract; or
6. retention of the current provider because no alternative survives both the
   correctness and whole-product performance gates.

This plan deliberately treats correctness, provider-core speed, CREXX adapter
cost, whole-VM speed, portability and maintenance as separate verdicts.

## 2. Independence and authority boundary

`DECIMAL-01` is separate from:

- [`NUMERIC-01`](../NUMERIC-01-WORKLIST.md), which completed the typed Level B
  numeric BIF surface and RexxCPS numeric-context/type work;
- PERF3's compiler, RXAS, representation and runtime queue; and
- the historical performance-programme charter.

The investigation may create isolated workloads, scratch candidate adapters
and retained evidence. It does not authorize a production plugin edit, a
change to `value`, a plugin ABI change, a new language option, a public RXAS or
RXBIN change, or a different default provider. Those remain explicit Adrian
decisions.

The source layout for this activity is:

| Location | Ownership |
| --- | --- |
| `performance/decimal/` | plan, worklist, candidate ledger and evidence index |
| `tests/performance/decimal/` | approved Level B provider workloads and focused implementation comparisons |
| `performance/tools/` | maintained Level B orchestration only |
| external scratch root such as `${TMPDIR}/crexx-decimal-lab/` | hash-pinned third-party source, disposable native library harnesses and isolated builds |
| `interpreter/rxvmplugin/rxvmplugins/` | production provider code, untouched until candidate selection and approval |

No third-party source is vendored during the evidence panel. A retained bundle
records source URL, version/tag, archive or commit hash, licence, local patches,
compiler and build command.

## 3. Verified current-product baseline

The review baseline is:

- repository `/Users/adrian/CLionProjects/CREXX`;
- clean `develop` at `4813e98d1`, equal to `origin/develop` when this plan was
  created;
- host `Darwin 25.5.0 arm64`, Apple M5, 10 logical CPUs;
- Apple Clang 21.0.0; and
- Apple ARM64 `long double` is 8 bytes with 53 mantissa bits and
  `LDBL_DIG=15`, so it is the same precision class as binary64 on this host.

There are two current providers behind the same `decplugin` function table:

| Provider | Current role | Representation | Principal finding |
| --- | --- | --- | --- |
| `mc_decimal` | statically registered normal/default provider | Cowlishaw `decNumber`, arbitrary precision | Correctness baseline candidate; CREXX uses `DECDPUN=8`, `DECUSE64=1`, `DECNUMDIGITS=64`, and `DECBUFFER=64` |
| `db_decimal` | separately loadable alternative/control | platform `long double` plus explicit decimal rounding | Not a general correctness candidate: precision and behaviour vary by compiler/OS, and Apple ARM64 silently caps an 18-digit cREXX context to 15 digits |

The normal library is therefore not simply an untouched 32-bit build. Its
coefficient is stored in base-10^8 `uint32_t` units and it uses 64-bit
intermediates where Cowlishaw's compatibility design permits them. It also has
CREXX-added signed/unsigned 64-bit conversions. It is still a 32-bit-limb
algorithm: it does not use base-10^18 `uint64_t` limbs with native 128-bit
products on modern 64-bit hosts.

The current tuning deserves measurement rather than assumption:

- every ordinary `decNumber` value reserves the compile-time 64-digit shape
  even when the effective context is 9 or 18 digits;
- `DECDPUN=8` favours arithmetic over conversion, while the upstream guidance
  identifies 3, 4, 8 and sometimes 9 as workload-dependent choices;
- `DECBUFFER=64` avoids some internal allocations but increases stack working
  sets; and
- literal forms in several VM instructions build and free a temporary decimal
  value, so provider speed and adapter allocation cost can differ materially.

## 4. Current semantic and architecture findings

### 4.1 Mike Cowlishaw numeric context to preserve

The provider and its VM adapter participate in five documented procedure
settings:

| Setting | Required behaviour |
| --- | --- |
| `NUMERIC DIGITS` | working significant-digit precision; cREXX default 18, Classic default 9 |
| `NUMERIC FUZZ` | reduce significant digits used for numeric comparison without reducing calculation precision |
| `NUMERIC FORM` | scientific versus engineering textual form |
| `NUMERIC CASE` | lower/upper exponent marker and special-value text |
| `NUMERIC STANDARD` | `COMMON`: half-even and clamped decimal context; `CLASSIC`: half-up and unclamped context plus Classic quotient rules owned jointly with the VM/compiler |

`OPTIONS NUMERIC_COMMON`/`NUMERIC_CLASSIC` also changes parsing and must not be
confused with the runtime provider context.

### 4.2 Gaps to disposition before performance claims

The source review found these correctness risks in the current boundary. They
are Gate 0 inputs, not permission to fix them in this activity:

1. Both providers receive `NUMERIC FUZZ`, but neither provider's context-sync
   or decimal comparison path consumes it. The VM stores and reports the
   setting. A direct 2026-08-05 RXAS diagnostic confirmed that `FUZZ 1` did not
   change decimal equality under either VM or provider.
2. `db_decimal` clamps requested precision to `LDBL_DIG`. Its rounding helper
   uses binary `log10l`, `powl` and `roundl` after arithmetic and does not select
   half-even for `STANDARD COMMON`. Direct proof also found that it ignored
   engineering/lower formatting and rounded a Common halfway case half-up. It
   therefore cannot be treated as a decimal-correct oracle merely because a
   displayed result matches.
3. The source documentation requires the Classic integer
   magnitude/precision constraint for decimal `%` and `//`, while the current
   VM comments still describe its enforcement as required work. The VM builds
   integer division from provider divide plus truncate and remainder from
   divide/truncate/multiply/subtract.
4. Provider diagnostics are mutable context state. Signal number, message,
   clearing, trap order and partial-result behaviour must match, not just the
   final coefficient.
5. Decimal value bytes have no provider tag. The most recently registered
   decimal provider is the process default, and values are meaningful only to
   that provider. Two raw representations cannot safely be mixed by switching
   providers mid-process.
6. The registry holds one factory-created provider instance with mutable
   diagnostic and numeric-context state. Candidate context ownership,
   re-entrancy and thread safety must be explicit; baseline timing remains
   serial and does not imply concurrent-provider support.
7. An unconfirmed field report says the RXAS numeric-context instructions may
   not be applying all settings correctly. The compiler normally replaces the
   five immediate setters with `NUMSCI` or `NUMENG` for a fully constant,
   fuzz-zero context. Current focused coverage passes, but it does not yet
   exhaustively prove every individual setter/getter form, every observable
   setting effect, or exact combined-versus-expanded equivalence.

Gate 0 must classify each item as implemented elsewhere, confirmed defect,
document mismatch, or intentional limitation. Timings retain the current
product unchanged; corrected semantics, if approved separately, get a new
baseline and are never compared to old work as though equivalent.

### 4.3 Existing isolation seam

The plugin seam is already suitable for fair per-process comparisons:

- `mc_decimal` is registered statically;
- `rxvm -p <dynamic-provider>` loads another provider, whose registration at
  the head of the factory list makes it the default for that process; and
- the same RXBIN workload can therefore be executed in separate processes
  under each provider with no compiler or workload change.

This is the primary comparison mode. A hybrid fixed/arbitrary implementation
would need to be one composite plugin with tagged private values, or it would
need a separately approved value/plugin contract. Loading two untagged raw
providers and switching between them is excluded.

## 5. Candidate panel

All entries remain candidates or controls until measured on CREXX workloads.
Published upstream benchmark figures provide orientation only.

| ID | Candidate | Precision/model | Initial disposition |
| --- | --- | --- | --- |
| D0 | Current `mc_decimal` | arbitrary `decNumber` | Exact product baseline and initial oracle input; retain unless displaced |
| D1a | Current `db_decimal`, unrestricted | platform binary `long double` with decimal rounding | Diagnostic performance ceiling only; correctness failures expected and retained |
| D1b | Current `db_decimal`, qualified Classic 9 | same provider, restricted to `STANDARD CLASSIC`, `DIGITS 9`, `FUZZ 0` and individually qualified workloads | ooRexx-comparable speed control when exact output, branch and signal behaviour matches; separately labelled and never treated as the general provider |
| D2 | Retuned `decNumber` | arbitrary, same code family | First low-integration-cost panel: `DECDPUN`, `DECNUMDIGITS`, `DECBUFFER`, allocation and compiler-codegen grid |
| D3 | Cowlishaw `decQuad` | fixed IEEE decimal128, 34 digits | Strong fixed-precision comparator; already vendored but not built into the current provider; can cover cREXX 18 and Classic 9 but not arbitrary `DIGITS` |
| D4 | `libmpdec` 4.0.1 | correctly rounded arbitrary-precision decimal C library | Primary external arbitrary-precision candidate; current project documents macOS, Linux, Windows, GCC, Clang and Visual Studio testing |
| D5 | Intel Decimal Floating-Point Math Library 2.0 Update 4 | fixed decimal32/64/128, principally BID | Fixed-precision and platform comparator; current Intel package claims Linux, Windows and macOS support; licence and redistribution are explicit gates |
| D6 | Boost.Decimal 1.91 | portable C++14 fixed decimal32/64/128 plus fast variants | Modern cross-platform fixed comparator; C++ bridge, code size, maturity and rounding-mode mapping must be measured |
| D7 | GCC `_Decimal128` plus runtime support | fixed 34-digit compiler type, DPD or BID software/hardware by target | GCC/platform-only ceiling, not a portable default; GCC documents incomplete library support and target-dependent availability |
| D8 | Native 64-bit-limb `decNumber`-derived PoC | arbitrary base-10^18-style limbs using 64x64-to-128 arithmetic | High-effort candidate opened only if profiles show limb arithmetic dominates and D2/D4 do not meet the target |
| D9 | Hybrid fixed-34 plus arbitrary fallback | composite provider | Architecture candidate only after D3 and an arbitrary provider independently pass; requires tagged representation, fallback and transition-cost proof |

### 5.1 Candidates not in the first implementation batch

- `decDouble` is useful at Classic 9 digits but its 16-digit precision cannot
  represent cREXX's default 18-digit context, so it is a bounded control rather
  than a default provider.
- GMP/MPFR provide strong arbitrary binary or integer arithmetic, not a direct
  implementation of General Decimal Arithmetic. They may provide diagnostic
  ceilings or exact-integer building blocks but require a new decimal
  coefficient/exponent, rounding, signal and text layer.
- Microsoft/OLE `DECIMAL`, fixed-scale integer libraries and a raw
  `__int128` coefficient do not cover arbitrary `DIGITS`, the required exponent
  range or General Decimal Arithmetic. They may be fixed-domain controls only.
- Boost.Multiprecision `cpp_dec_float` has fixed compile-time precision and its
  documented conversion behaviour is truncating. It is not a first-line
  semantic replacement for a runtime-context decimal provider.

## 6. Correctness programme

Correctness is a hard prerequisite. A faster candidate with one unexplained
numeric, textual or signal mismatch is rejected for production selection.

### 6.1 Oracle hierarchy

Use independent evidence in this order:

1. the General Decimal Arithmetic specification and its official `decTest`
   vectors;
2. ANSI Classic rules and the cREXX language reference for the deliberately
   different `COMMON`/`CLASSIC` contract;
3. existing focused cREXX RXAS, plugin and Level B tests;
4. a frozen current-`mc_decimal` transcript for compatibility detection, not
   as authority when it conflicts with 1 or 2; and
5. differential and metamorphic tests across candidates.

The official vectors must be mapped through the exact `decplugin` surface and
CREXX signal model. Unsupported vector operations are reported, never counted
as passes.

### 6.2 Context matrix

At minimum test:

- digits `5`, `9`, `18`, `34`, `50`, `64`, `100` and `1000` where the provider
  claims support;
- fuzz `0`, `1`, `digits-1` and invalid `fuzz >= digits`;
- scientific and engineering form;
- lower and upper numeric case;
- Common half-even/clamped and Classic half-up/unclamped standards; and
- inherited and procedure-local contexts, nested calls, recursion, signals and
  return to the caller's context.

### 6.3 Operation/contract matrix

Cover every provider function and every VM composition:

- string, signed 64-bit integer and binary64 input/output;
- add, subtract, multiply, divide, power and negate;
- register/register and literal forms;
- compare, compare-string and all `FUZZ` boundaries;
- zero, signed zero, infinities, quiet/signalling NaNs and payload handling to
  the extent cREXX promises it;
- truncate, round, integer division and remainder;
- coefficient/exponent extraction and simple/scientific/engineering text;
- exact, rounded, inexact, overflow, underflow, subnormal, conversion error,
  invalid operation and division-by-zero status;
- destination aliasing and in-place operations;
- allocation failure and partial-result visibility; and
- plugin load, context sync, value copy/clear, frame reuse and final teardown.

The RXAS numeric-context surface is part of this matrix, not merely compiler
setup plumbing. Cover both literal and register forms of `SETNUMDGTS`,
`SETNUMFUZ`, `SETNUMFRM`, `SETNUMCAS` and `SETNUMSTD`; all five `GETNUM*`
forms; and the combined `NUMSCI`/`NUMENG` instructions.

`DPOW` is a separate gate. Libraries differ in guarantees for non-integral
power, so it must pass authoritative vectors and cREXX signal/text behaviour
rather than inherit the verdict of add/multiply/divide.

### 6.4 Known-gap handling

Gate 0 produces a small ledger with four dispositions per finding:

- `conformant-current`;
- `confirmed-current-defect`;
- `document-defect`; or
- `deliberate-documented-limitation`.

A confirmed defect is repaired only under separate approval. Every candidate
is then measured against the same accepted semantic baseline. `db_decimal`
may continue as an explicitly non-conforming performance control, but its
numbers cannot enter a candidate score.

### 6.5 RXAS numeric-context instruction proof

The unconfirmed field report is resolved with fresh current-product evidence,
not historical green status alone. The proof must cover:

1. assembly, disassembly and opcode metadata for every individual and combined
   form;
2. literal and register setter inputs followed by independent `GETNUM*`
   readback;
3. observable precision, comparison/fuzz, scientific/engineering form,
   lower/upper case, Common/Classic rounding and signal effects;
4. exact state and decimal-result equivalence between `NUMSCI`/`NUMENG` and the
   corresponding five-setter sequence;
5. invalid-operand behaviour, signal identity and proof that rejected combined
   operands do not partially mutate or synchronize the context;
6. compiler emission in optimized and no-opt modes, including inherited,
   nonzero-fuzz and digits-below-five fallback cases;
7. nested procedure entry/return and caller-context restoration; and
8. `rxvm` and `rxbvm` under the default, explicit `mc_decimal` and explicit
   `db_decimal` providers, with provider-specific expected differences kept
   visible.

The 2026-08-05 opening check rebuilt the focused binaries and passed 9/9
existing tests, including `nr09_numeric_context_contract`, all six RXAS decimal
provider/VM combinations and both provider suites. The expanded direct RXAS
fixture then passed basic individual/combined state and parity coverage across
the same matrix. A separately named observable diagnostic confirmed that
`FUZZ` is not consumed by comparisons and that `db_decimal` ignores the tested
form/case and Common half-even effects. Evidence is retained in
[`2026-08-05-decimal-01-rxas-numctx-opening`](../evidence/2026-08-05-decimal-01-rxas-numctx-opening/).

Adrian then authorized repair before further work. The bounded repair keeps
comparison precision provider-owned, applies effective `DIGITS - FUZZ` without
changing stored arithmetic precision, makes `db_decimal` rounding explicitly
Common half-even or Classic half-up, and reuses the framework REXX formatter
for form/case. Focused Debug and ordinary profiling-off Release each pass 9/9,
and the compact observable diagnostic passes all six VM/provider cells in both
builds. This provisional correctness verdict is retained in
[`2026-08-05-decimal-01-numctx-repair-verdict`](../evidence/2026-08-05-decimal-01-numctx-repair-verdict/).
No performance timing or broader Gate 0 closeout was performed.

## 7. Performance experiment design

### 7.1 Four measurement layers

| Layer | Question answered | Measurement boundary |
| --- | --- | --- |
| L0 provider core | Is the arithmetic library itself faster? | Direct upstream/native operation loop in external scratch; no VM and no `decplugin` allocation |
| L1 CREXX adapter | What does representation, allocation, conversion and the function table cost? | Isolated candidate plugin harness with CREXX `value` lifecycle |
| L2 VM opcode | What is the cost seen by `rxvm` and `rxbvm`? | Fixed-work RXAS/Level B microcells under an explicitly selected provider |
| L3 application | Does it improve real cREXX programs? | Correctness-gated decimal-heavy workloads plus lifecycle/RSS/artifact scorecard |

L0 native harnesses are disposable diagnostic inputs outside the repository.
They do not become the maintained performance control plane. Maintained
orchestration and result validation are Level B cREXX.

### 7.2 Microcell shapes

For each operation, include data-dependent shapes rather than one friendly
constant:

- one-limb/small integers;
- aligned and widely separated exponents;
- carry/borrow chains;
- cancellation to zero and signed-zero edges;
- exact and repeating division;
- multiplication at 9, 18, 34, 64, 100 and 1000 digits;
- short and long numeric text, with and without exponent;
- equal, early-different and late-different comparison;
- integral and non-integral power with small and larger exponents; and
- ordinary finite success paths kept separate from signal paths.

Inputs are pre-created for arithmetic-only cells. Separate cells include parse,
format, allocation and teardown. A candidate may not claim an arithmetic win
from omitting work that the baseline cell performs.

### 7.3 Representative Level B workloads

The first portfolio should include:

1. a fixed-work add/subtract/multiply/divide mix;
2. a conversion-heavy decimal-text ledger;
3. an independently authored, publishable CDB-1 billing workload informed by
   Telco's public application shape without copying restricted source or data;
4. decimal Mandelbrot with exact output/checksum and explicitly fixed context;
5. compound-interest/amortisation work that exercises rounding boundaries;
6. the existing RexxCPS `decimal-string` family control; and
7. canonical RexxCPS as a separately reported whole-product guard, not as the
   sole decimal benchmark.

Canonical workloads remain unchanged. Provider diagnostics use separately
named sources and cannot silently replace the formal portfolio.

There is no reviewed standards-body arithmetic performance suite. Official
`decTest` material remains correctness evidence. Cowlishaw's Telco workload is
an application reference whose own documentation says its narrow operation
mix is not suitable for benchmarking decimal implementations generally. CREXX
therefore publishes its exact CDB-1 operation mix, inputs, work counts,
checksums, licences and raw observations and reports any billing cell as one
portfolio member rather than a standards benchmark.

### 7.4 Cost counters

Retain per cell where measurable:

- elapsed/throughput raw samples;
- operation count and operand digit/exponent distribution;
- decimal allocations, reallocations, frees and bytes;
- stack/temporary workspace and peak RSS;
- decimal value payload/capacity bytes and copy bytes;
- string conversion bytes;
- context-sync count and elapsed cost;
- plugin load/unload and load-to-first-result time;
- static and dynamic plugin/file size; and
- hardware cycles/instructions/cache/branch samples as diagnostic attribution
  on supported hosts.

Counters must not contaminate formal timing. Confirm findings with ordinary
profiling-off Release builds.

## 8. decNumber optimization panel

The current provider gets a fair tuning investigation before replacement.

### D2-A — compile-time tuning grid

In isolated builds compare:

- `DECDPUN=3`, `4`, `8` and `9`;
- `DECNUMDIGITS=18`, `34` and `64`, plus exact-sized dynamic storage where a
  safe adapter PoC exists;
- `DECBUFFER` sizes near `18`, `34`, `64` and `128`, rounded to the relevant
  `DECDPUN` and four-digit tuning boundaries; and
- current release flags under Apple Clang, Linux GCC/Clang and Windows MSVC.

Do not combine winners from unrelated cells without measuring the composed
build. Conversion-heavy and arithmetic-heavy winners may differ.

### D2-B — adapter and allocation work

Measure before changing:

- per-value 64-digit capacity at 9/18-digit contexts;
- literal temporary allocate/parse/free sequences;
- compare-string temporary allocation above `DECNUMDIGITS`;
- double conversion temporary strings;
- repeated result-capacity checks; and
- context sync on frame entry/return.

Compare exact-sized allocation, small-buffer storage, capacity reuse and a
narrow provider-owned pool only as isolated alternatives. Ownership, OOM,
frame reuse, references, value copies, plugin unload and Windows CRT boundaries
are correctness gates.

### D8 — native 64-bit limb route

Open only after D2 attribution. The PoC must define:

- coefficient radix and normalization;
- 64x64-to-128 multiply/add/divide primitives using `__uint128_t` on
  GCC/Clang and an explicit MSVC implementation/fallback;
- carry, quotient estimation and conversion algorithms;
- endianness and serialized/non-serialized boundaries;
- a 32-bit or portable fallback; and
- differential proof against official vectors and the accepted oracle.

This is a new arithmetic implementation, not a macro substitution. It must not
be selected because 64-bit sounds native; the profile must show that its
targeted work is material.

## 9. Fixed-precision and hybrid panel

`decQuad`, Intel decimal128, Boost `decimal128_t` and GCC `_Decimal128` all
provide 34 decimal digits, enough for cREXX's default 18 and Classic 9. They do
not satisfy arbitrary `NUMERIC DIGITS` alone.

First compare them as complete fixed-provider controls at supported contexts.
Only then compare a hybrid architecture:

1. fixed representation for contexts and results provably within its precision
   and exponent contract;
2. arbitrary representation otherwise;
3. a tag owned by one composite plugin;
4. exact transitions with no double rounding;
5. stable copy/clear/reference behaviour; and
6. measured tag branch, promotion, memory and code-size costs.

Two hybrid policies must be compared if both are viable:

- procedure-context selection, where all values in a frame use one form; and
- per-value selection, where values carry their form and operations may
  promote.

Procedure selection is simpler but must handle decimal values passed between
different-context procedures without format confusion or loss. Per-value
selection is more flexible but adds a hot tag/dispatch and a larger ownership
surface. Either is an architecture decision requiring Adrian's approval.

## 10. Platform and toolchain matrix

Use per-host ratios to the exact same-host current provider. Do not rank
absolute timings from different hosts as though they were one experiment.

| Sequence | Platform/toolchains | Purpose |
| --- | --- | --- |
| P1 | Apple ARM64, Apple Clang | primary design/PoC host; proves the current `db_decimal` binary64 limitation |
| P2 | Linux x86-64, GCC and Clang | main 64-bit codegen, `__uint128_t`, libmpdec, Intel/GCC controls and attribution |
| P3 | supported Linux ARM64, GCC and Clang where supported | non-Apple ARM64 portability and codegen |
| P4 | Windows x86-64, MSVC; clang-cl/MinGW controls only where buildable | supported Windows contract, CRT/allocation boundary and absence of GNU-only assumptions |
| P5 | optional POWER/s390x or another real decimal-hardware host | platform ceiling only when available; never inferred from x86/ARM |

Cross-compilation is build evidence, not runtime correctness or performance
evidence.

## 11. Sampling and scorecard rules

- Discovery microcells may use one warmup and at least seven recorded serial
  samples after calibration.
- Candidate decisions use the governance requirement: at least one warmup and
  12 paired, balanced/interleaved recorded rounds per cell.
- Formal absolute cells use two warmups and ten recorded serial samples.
- Run on AC with low-power mode off and capture pre/post host, power, thermal
  and load state.
- Before every DECIMAL-01 timing session, ask Adrian to clear and reserve this
  shared host. Do not begin until he confirms exclusivity; then audit for
  competing benchmark, compiler, build and test processes. If competing work
  appears, pause before the next cell and invalidate any affected samples.
  Correctness builds/tests are not performance evidence and their elapsed time
  is never reused as a benchmark result.
- Remove no outlier without an independently demonstrated fault.
- Report `rxvm` and `rxbvm`, static and dynamic provider, arithmetic and
  lifecycle, time, RSS and artifacts separately.
- Use a zero-decimal or integer-only guard to detect plugin load/context/code
  layout movement unrelated to decimal work.
- Apply the standing 3% per-workload guard and stop on a guard hit. A decimal
  aggregate may be added only after its membership and weighting are approved;
  it does not enter the common-five aggregate.

No external upstream benchmark result is a CREXX result. Every selected claim
comes from retained raw CREXX evidence on the named host.

## 12. Execution phases and hard gates

### Gate 0 — semantic contract and current-gap ledger

Deliver:

- exact `decplugin` and VM-composition operation inventory;
- the five-setting context matrix;
- the complete individual and combined RXAS numeric-context instruction proof;
- signal and formatting contract;
- disposition of `FUZZ`, Common/Classic rounding/clamp, Classic quotient and
  `db_decimal` precision findings; and
- official-vector mapping and oracle hierarchy.

**Stop disposition:** Adrian accepted the focused repair verdict on 2026-08-05
and directed the programme to its second stage, Gate 1. Baseline preparation
may begin; no timing cell may run until the remaining correctness/setup
boundary is recorded and Adrian has explicitly cleared and reserved the host.

### Gate 1 — current-provider cost baseline

Build the independent Level B workload set and capture D0, D1a and D1b through
L1-L3 on Apple ARM64. Attribute arithmetic, conversion, allocation, context
and lifecycle costs. D1a mismatches remain visible and excluded from
selection; D1b contains only individually correctness-qualified Classic-9
workloads and remains a separately disclosed speed-control row.

The prepared workload set is specified as the independently authored,
publishable [`CREXX Decimal Benchmark (CDB-1)`](CREXX-DECIMAL-BENCHMARK.md).
Three images are retained: compiler-on/RXAS-on as the primary product verdict,
the identical compiler output with RXAS optimization off as the assembler
isolation control, and compiler-off/RXAS-off as a broad diagnostic. Maintained
qualification disassembles every final RXBIN and rejects an image if required
runtime decimal operations have been optimized away.

**Stop:** approve the first candidate batch using observed cost, not a library
reputation.

### Gate 2 — low-cost candidate panel

Recommended first batch:

1. D2 `decNumber` tuning grid;
2. D4 `libmpdec` direct and adapter PoC; and
3. D3 `decQuad` fixed 34-digit comparator.

This panel answers whether the current family can be tuned, whether the
strongest portable arbitrary candidate wins, and whether fixed precision has
enough ceiling to justify hybrid design. Retain negative results.

**Stop:** Adrian selects no change, one arbitrary provider for integration, or
a separately approved fixed/hybrid design panel.

### Gate 3 — extended/platform candidates

Open D5 Intel, D6 Boost, D7 GCC and D8 native-64-bit only where Gate 2 leaves a
material question. Licence, toolchain or semantic failures reject a route
without requiring full timing.

Before opening any extended candidate source work, produce a public-evidence
dossier. For each candidate record the exact version/date, author or sponsor,
precision and semantic mode, workload/operation mix, hardware, compiler flags,
raw-data/source availability and whether the evidence is upstream-authored,
independent peer-reviewed, independently reproduced or historical only.
Include correctness/test-suite evidence, licences, supported platforms,
maintenance activity and material public issue reports. Public results may
prioritize or reject a PoC, but cannot become a CREXX performance result.

Initial orientation already identifies four useful but non-equivalent sources:
Cowlishaw's historical operation tables; mpdecimal's cross-library Mandelbrot
and testing material; the 2009 peer-reviewed cross-library decimal benchmark
suite; and Boost.Decimal's current reproducible operation/`charconv` harness.
Their ages, sponsor interests, different APIs and different machines must
remain explicit rather than averaging their reported numbers.

### Gate 4 — isolated complete-plugin comparison

Implement selected candidates as separately named, dynamically loadable
plugins in an isolated branch/worktree. Compare identical RXBIN and Level B
inputs, static/dynamic load, both VMs, optimized/no-opt correctness, startup,
steady state, memory, teardown and unrelated guards.

**Stop:** present the complete candidate matrix. No production/default change
until Adrian selects one architecture.

### Gate 5 — minimum production edit and first Release verdict

After explicit selection, reimplement the candidate in production form. Run
only minimum focused correctness, freeze implementation, build ordinary
profiling-off Release, run the smallest decisive paired provider and
application cells, report the verdict and stop under `performance/AGENTS.md`.

Neutral, negative, noisy or incorrect results keep revert/no-change live.

### Gate 6 — accepted closeout and supported platforms

Only after Adrian accepts Gate 5:

- remove disposable PoCs;
- run focused plus full Debug validation and proportional sanitizer checks;
- validate Apple ARM64, Linux x86-64, Linux ARM64 and Windows in order;
- retain one checksum-closed evidence bundle;
- update provider and numeric documentation; and
- decide default/static/dynamic packaging and any deprecation of
  `db_decimal`.

Commit and publication remain separately authorized actions.

## 13. Selection rubric

### Mandatory pass/fail

- 100% of the accepted correctness corpus and cREXX signal/text contract;
- arbitrary `DIGITS` support or an exact, tested fallback;
- both numeric standards and all five numeric context settings;
- supported Apple, Linux and Windows build/runtime path;
- safe allocation, value copy, frame reuse, plugin unload and OOM behaviour;
- acceptable licence and redistribution terms; and
- no language, public ABI, RXAS/RXBIN or serialized-value change without a
  separate approved decision.

### Ranked dimensions after correctness

1. whole-product decimal workload throughput at 9 and 18 digits;
2. throughput/scaling at 34, 64, 100 and 1000 digits;
3. conversion-heavy and arithmetic-heavy balance;
4. allocation/copy bytes, peak RSS and value size;
5. load-to-first-result and context/lifecycle overhead;
6. plugin and installed artifact size;
7. performance consistency across both VMs and supported platforms; and
8. maintenance surface, upstream vitality, patch burden and diagnostic quality.

A fixed fast lane is selected only when its integrated gain clearly repays the
tag/promotion/code-size cost. A bespoke 64-bit fork is selected only when it
beats maintained external alternatives materially enough to justify owning an
arithmetic library.

## 14. Initial engineering recommendation

1. Keep `mc_decimal` as the production correctness baseline for now.
2. Split `db_decimal` reporting into D1a unrestricted diagnostic ceiling and
   D1b correctness-qualified Classic-9 speed control. D1b is directly relevant
   to the ooRexx comparison but never stands in for the default 18-digit cREXX
   product, which this Apple ARM64 provider cannot represent.
3. Run the D2 tuning grid before rewriting `decNumber`; the current 8/64/64
   settings are a hypothesis, not a measured optimum for CREXX's 9/18-digit
   workload mix.
4. Put `libmpdec` in the first external batch. It directly implements the same
   General Decimal Arithmetic family, supports arbitrary precision and has the
   strongest stated cross-platform/toolchain coverage among the reviewed C
   candidates.
5. Put already-vendored `decQuad` beside it as the first fixed 34-digit ceiling.
   Its result will tell us whether a hybrid is economically interesting before
   any value-tag architecture is designed.
6. Defer the native 64-bit-limb fork until attribution shows that 32-bit-limb
   arithmetic, rather than allocation/conversion/VM adapter work, is the
   dominant remaining cost.

## 15. Deliverables

The completed evidence/design activity must leave:

- this plan updated with dated gate outcomes;
- a resumable `DECIMAL-01-WORKLIST.md` before experimental source work;
- a current semantic/gap ledger;
- a complete RXAS individual/combined numeric-context result matrix;
- a candidate/version/licence/build manifest;
- a source-attributed public-evidence dossier used to prioritize the extended
  panel;
- correctness corpus manifest and result matrix;
- raw provider/core, adapter, opcode and application samples;
- per-platform time/RSS/artifact/lifecycle scorecards;
- accepted and rejected candidate records with reasons;
- a recommendation covering arbitrary precision, fixed fast paths,
  `db_decimal`, default/static/dynamic packaging and fallback; and
- a paste-ready implementation handoff that stops at the mandatory first
  Release verdict.

## 16. Primary external references

Reviewed on 2026-08-05. These establish capability and candidate eligibility,
not CREXX performance:

- Mike Cowlishaw, [General Decimal Arithmetic](https://speleotrove.com/decimal/)
  and [`decNumber` overview](https://speleotrove.com/decimal/dnintro.html);
- Cowlishaw, [`decNumber` tuning options](https://speleotrove.com/decimal/dnopts.html)
  and [historical provider performance appendix](https://speleotrove.com/decimal/dnperf.html);
- mpdecimal, [`libmpdec` 4.0.1 documentation](https://www.bytereef.org/mpdecimal/doc/libmpdec/index.html),
  [context/correct-rounding contract](https://www.bytereef.org/mpdecimal/doc/libmpdec/context.html),
  [platform matrix](https://www.bytereef.org/mpdecimal/), and
  [decimal benchmark descriptions](https://www.bytereef.org/mpdecimal/benchmarks.html)
  plus its [public testing account](https://www.bytereef.org/mpdecimal/testing.html);
- Intel, [Decimal Floating-Point Math Library 2.0 Update 4](https://www.intel.com/content/www/us/en/developer/articles/tool/intel-decimal-floating-point-math-library.html);
- GCC, [`_Decimal32`/`64`/`128` support and limitations](https://gcc.gnu.org/onlinedocs/gcc/Decimal-Float.html)
  and [software DPD/BID runtime routines](https://gcc.gnu.org/onlinedocs/gccint/Decimal-float-library-routines.html); and
- Boost 1.91, [Boost.Decimal](https://www.boost.org/doc/libs/latest/libs/decimal/doc/html/basics.html)
  and its [published benchmark methodology/results](https://www.boost.org/doc/libs/latest/libs/decimal/doc/html/benchmarks.html); and
- Anderson et al., [Performance Analysis of Decimal Floating-Point Libraries and Its Impact on Decimal Hardware and Software Solutions](https://iccd.et.tudelft.nl/2009/proceedings/465Anderson.pdf),
  a historical peer-reviewed cross-library workload and methodology source.

## 17. Approval record

On 2026-08-05 Adrian approved the seven-step gated DECIMAL-01 plan and opened
Gate 0 correctness work only. He required the unconfirmed RXAS numeric-context
instruction report to be validated as part of Gate 0 and required public
evidence to inform the extended-candidate panel. All later selection and
production stops remain binding.

The development host is shared with two other performance agents. No
DECIMAL-01 timing may start without asking Adrian to clear and reserve the
machine and receiving confirmation. A pause request stops work before another
performance cell begins; the exact checkpoint remains in the worklist.
