# PERF3-03 bounded conversion review worklist

Status: **complete on Apple — C4 v3 retained; Windows validation queued**

Approved: 2026-08-01

Purpose: determine whether the current bounded string-to-integer and
string-to-binary64 conversion paths have a material, general and
contract-preserving optimization, without conflating them with the much larger
decimal conversion/formatting lane or preselecting a public RXAS operation.

## Authority and stop boundaries

Adrian approved the bounded PERF3-03 evidence/design gate after accepting the
PERF3-05 retain-L0 disposition. This activity may audit current behavior,
retain external contract/cost controls and build disposable isolated PoCs. It
must stop for selection before a production compiler, VM, RXAS/RXBIN, ABI,
signal or language-contract edit.

Adrian selected the narrow C4 v3 production candidate and accepted its
favourable first ordinary Release verdict on 2026-08-01. That acceptance
authorized proportional closeout, but not commit or push.

Any selected production implementation receives the mandatory first ordinary
profiling-off Release verdict immediately after minimum focused correctness.
The previously proposed five-operand binary-span operation remains an
unselected architecture option, not inherited authority.

## Exact starting point

- Branch/HEAD: local `develop` at
  `4a3940395980dc40ea45917d71d99caa080e89bb`, three commits ahead of
  `origin/develop`.
- Accepted uncommitted product scope: P1A A1 demand-driven RXAS storage
  attachment. PERF3-05 proved fresh and unchanged-source ordinary Release
  products byte-identical; those binaries remain current baseline authority.
- PERF3-05 is complete with no production VM edit. PERF3-05-B1 is an
  independent build/API item and is outside this activity.
- Five protected untracked lifecycle RXBIN files remain outside scope and must
  not be overwritten, deleted or staged.
- Host/toolchain: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs; Apple clang
  21.0.0, CMake 4.3.2 and Ninja 1.13.2.

## Current implementation and contract

### Integer

`string2integer()` allocates `length + 1` bytes on every call, copies the span,
NUL-terminates it and calls `rxinteger_parse()`/`strtoimax(..., 10)`. Current
observable behavior includes libc leading whitespace, optional sign, decimal
digits, signed-range checking, trailing whitespace and complete-input
rejection after that whitespace.

### Binary64

`rx_string_to_double()` copies the span into a 128-byte stack buffer or a heap
buffer for inputs of 128 bytes or more, NUL-terminates it and calls `strtod()`.
It accepts leading/trailing libc whitespace and libc sign/special-value forms,
inherits the active C locale, rejects all `ERANGE` outcomes, and reports only
success/failure to `STOF`, which maps failure to `CONVERSION_ERROR`.

The review must explicitly freeze signed zero, normal/subnormal boundaries,
halfway rounding, maximum finite values, overflow/underflow, long inputs,
complete consumption, whitespace, signs, `inf`/`nan`, locale behavior and
failure output mutation. JSON grammar and its non-throwing method status remain
separate caller contracts.

## Entry evidence and materiality boundary

- PERF3-01 reports 3,009,511 explicit conversion instructions for canonical
  RexxCPS, but 3,008,006 are `ITOS`/`STOD`/`DTOS`; only 300 are `STOF` and two
  are `STOI`. Those opcode counts do not include implicit numeric conversion in
  loose string comparison.
- `rxvm_loose_compare_text()` calls `string2float()` for both operands on every
  loose comparison. Retained Intel Linux native evidence places libc `strtod`
  at 7.26%/9.19% for Clang `rxvm`/`rxbvm` RexxCPS and 0.78-1.63% for Base64.
  This is the material general runtime owner that the explicit opcode count
  hides; current diagnostic instrumentation must recover its input/outcome
  shape before candidate selection.
- Sieve, Base64, Richards and Towers each execute one explicit `STOI` in their
  current exact profile and no `STOF`. The integer helper must therefore prove
  a compiler/assembler or realistic typed-conversion owner independently; its
  strong isolated allocation ceiling alone is insufficient.
- The current general hot use case is repeated typed conversion such as
  `rxjson.node_float()`/`node_f32_array()` and other future bounded-string
  callers. CRI-13 remains an evidence source, not permission for JSON-specific
  runtime handling.
- Advancing a production candidate requires a measured end-to-end use case
  that is both common in realistic code and general beyond one JSON method.
  A fast isolated parser with no material caller is only a retained ceiling.

## Compared designs

### C0 — current copied libc paths

Exact `string2integer()` and `rx_string_to_double()` behavior, allocations,
copied bytes, code size and VM-level `STOI`/`STOF` cost.

### C1 — bounded integer parser

A direct `[begin,end)` decimal parser with checked signed accumulation, no
allocation and no copy. It must byte-for-byte preserve current success/failure,
range and whitespace/sign behavior. This is the narrowest plausible production
form and establishes the integer machine ceiling.

### C2 — copied C-locale binary64 control

Retain the temporary NUL buffer but use an explicit C-locale conversion. This
separates locale determinism from allocation/copy removal and correctly-rounded
engine choice. It is not a no-copy candidate.

### C3 — bounded locale-independent binary64 parser

Compare an established correctly rounded `[begin,end)` implementation with the
current wrapper and C2. Full-slice consumption and existing compatibility forms
must be explicit. An implementation that cannot prove rounding and retained
edge cases is rejected even if fast.

### C4 — loose-comparison ownership

Compare a cheap fail-closed numeric classifier, direct bounded conversion and
an optional value-owned parse-result cache against the current two-`strtod`
path. A classifier may only reject definitely nonnumeric spans; uncertain and
numeric forms fall back to the compatible converter. Any cache must enumerate
mutation invalidation, eager/lazy ownership, memory and lifecycle cost.

### C5 — caller/API ownership

Compare reuse behind existing `STOI`/`STOF`, a private bounded core callable by
trusted internal users, and the previously proposed non-throwing binary-span
operation. A new public operation or serialized encoding needs a separate
architecture selection. Private execution-image fusion remains a shape-specific
fallback, not the default candidate.

## Selection gates

- Correctness corpus and exact current-result oracle precede timing.
- Measure helper-only time separately from VM dispatch and end-to-end caller
  time. Record allocation count/bytes and copied bytes.
- Preserve both `rxvm` and `rxbvm`, optimized/no-opt semantics and signal
  behavior for any integrated PoC.
- Establish a direct bounded-parser machine ceiling before proposing a runtime
  instruction or call boundary.
- Do not treat 3.0 million explicit decimal conversions as direct evidence for
  `STOI`/`STOF`; keep the separately observed loose-comparison `strtod` cost.
- A production candidate needs a realistic material caller, not only a
  synthetic high-iteration microbenchmark.
- Keep integer and binary64 rungs independently selectable; one must not carry
  the semantic or implementation risk of the other.
- Preserve every rejected comparator, seed and exact reason.

## Work stages

### Stage A — contract and caller census

- [x] Freeze exact source, headers, compiler/libc/locale state and current
      baseline hashes.
- [x] Inventory every production caller and retained dynamic `STOI`/`STOF`
      count; separate decimal conversion operations.
- [x] Build the current behavior corpus across grammar, range, rounding,
      special values, long input and locale cases.
- [x] Record allocations/copies and output/signal behavior on success/failure.

### Stage B — isolated ceilings

- [x] Build C0/C1 integer controls and prove exact result equivalence.
- [x] Build C0/C2/C3 binary64 controls; reject unavailable or semantically
      unproved engines explicitly.
- [x] Compare helper-only time, code size, allocation/copy and exact edge
      results without editing production source.
- [x] Decide which, if any, form merits one integrated disposable PoC.

### Stage C — realistic use-case proof only if justified

- [x] Prove a current material caller and exact generated/runtime path.
- [x] Compare current and hand-equivalent ceiling end to end in both VMs.
- [x] Enumerate existing-opcode/private-core/public-span ownership and select
      no architecture silently.

### Stage D — selection stop

- [x] Retain one compact checksum-closed evidence bundle.
- [x] Present recommended retain/reject/defer dispositions for C0-C5 with exact
      reasons.
- [x] Adrian selects, rejects or requests rework of the C4 v3 production
      candidate.
- [x] Update the roadmap and report to Adrian.
- [x] Stop before a production edit, broad closeout, PERF3-04 or push.

### Stage E — selected production first Release verdict

- [x] Implement only the private locale-aware loose-comparison rejector with a
      portable no-inline spelling.
- [x] Pass the minimum focused Debug build and correctness gate: 6/6.
- [x] Freeze implementation and build the ordinary profiling-off Release
      product.
- [x] Run the smallest material same-session verdict with every governed noise
      and interval append retained: 212/212 executions pass.
- [x] Stop before broad closeout, sanitizer, platform expansion, commit or
      push.
- [x] Adrian accepts the favourable first Release verdict or requests bounded
      rework/revert.

### Stage F — accepted production closeout

- [x] Complete the full Debug build and pass 1,972/1,972 CTests with the
      repository's `--parallel 30` setting.
- [x] Build both ASan VM variants and pass the focused logic/conversion gate
      6/6 with leak detection off.
- [x] Retain the failed leak-on attempt: this macOS ASan runtime reports
      `detect_leaks is not supported on this platform`; do not claim LSan.
- [x] Complete the ordinary Release product, install 136 files into an isolated
      prefix and pass installed `rxvm`/`rxbvm` smoke 2/2.
- [x] Document the private prefilter and exact-converter fallback in the VM
      architecture reference.
- [x] Retain checksum-closed closeout evidence.
- [x] Record Windows/MSVC as a real-build pre-publication follow-up because no
      local Windows cross-toolchain is available.
- [x] Stop without commit or push.

## Evidence result and recommendation

The material current owner is implicit binary64 parsing in loose comparison,
not explicit `STOI`/`STOF`. Exact instrumentation records 6,315,583 loose
comparisons in canonical RexxCPS and 3,425,000 in Base64 2500. The corresponding
failed operands are 2,131,166 and 6,325,000; 2,129,732 and 6,050,000 respectively
are first-byte rejectable. Permute, Bounce, Richards and Towers execute zero
loose comparisons at current PERF3 work counts in both VMs. Because PERF3-05
proved that compiled layout can still move zero-exposure workloads, Permute,
Bounce and Richards were retained as timed common-layout guards; Towers remains
a diagnostic correctness control.

The C1 integer control removes one allocation/copy and is 5.17x faster in the
isolated median, but no material current runtime owner justifies its embedded-
NUL compatibility decision. C2 is timing-neutral and changes locale policy.
The tested libc++ C3 `from_chars` control is slower and incompatible with
current hex, NaN payload, `ERANGE`/subnormal and embedded-NUL behavior. The
value-cache form is architecture-deferred because every VM-private flag bit is
assigned; it needs a value-layout/ABI change or side table plus invalidation and
lifecycle proof.

C4 prefilter v1 is rejected because its expansion prevented the loose
comparator from being inlined and confirmed a Base64 `rxvm` guard regression.
V2 restored caller inlining but is rejected unchanged because hard-coded
dot/comma handling cannot prove arbitrary active-locale radix compatibility.

C4 v3 is the recommendation for Adrian's production-selection decision. It
uses an out-of-line, locale-aware impossible-leading-byte rejector and otherwise
calls the exact current converter. The installed-locale oracle passes 531 cases
in all 288 locales (152,928 comparisons, zero mismatches), and integrated
logic/conversion plus Sieve/Base64/RexxCPS/JSON checks pass 12/12 across both
VMs. Final paired medians are:

| Workload | `rxvm` | `rxbvm` | Disposition |
| --- | ---: | ---: | --- |
| Sieve zero-call control | +0.409%, `n=12` | +1.523%, `n=12` | favourable layout control |
| Base64 | +4.470%, `n=34` | +9.025%, `n=22` | `rxvm` noisy/inconclusive; `rxbvm` decisive favourable |
| RexxCPS | +2.736%, `n=12` | -0.254%, `n=34` | `rxvm` decisive favourable; `rxbvm` noisy/inconclusive |
| Permute zero-call control | +1.066%, `n=24` | -0.446%, `n=12` | `rxvm` favourable; `rxbvm` small adverse inside guard |
| Bounce zero-call control | +1.064%, `n=36` | +0.758%, `n=36` | both favourable at cap |
| Richards zero-call control | +0.490%, `n=24` | +0.147%, `n=36` | `rxvm` favourable; `rxbvm` neutral/inconclusive at cap |

No v3 cell demonstrates the 3% workload regression guard. The common-layout
panel retained all 348 executions: 156 initial, 120 first-append and 72 final-
append executions, all correct. The initial driver status was repaired by a
summary-only replay after changing only the mistaken aggregate metadata; no
sample was rerun, replaced or removed. Adrian selected private loose-comparison
ownership only: no public opcode, span API, RXAS/RXBIN change, JSON
specialization or cache.

Evidence: [`2026-08-01-perf3-03-conversion-review`](evidence/2026-08-01-perf3-03-conversion-review/).

## Selected production first Release verdict

The selected edit adds one private `rxvm_loose_string2float()` helper. It
rejects only empty or impossible-leading-byte spans and otherwise calls the
exact current converter. The helper uses a dedicated portable no-inline macro;
the existing Windows behavior of unrelated label-owner helpers is unchanged.

Minimum Debug validation passes 6/6. The fresh ordinary `-O3 -DNDEBUG`,
profiling-off Release binaries add 96 bytes for `rxvm` and 128 bytes for
`rxbvm`, and reproduce the selected PoC's file sizes and helper addresses.
The governed material verdict is:

| Workload | `rxvm` | `rxbvm` | Disposition |
| --- | ---: | ---: | --- |
| Base64 2500 | +4.859%, `n=34` | +5.780%, `n=22` | both decisive favourable |
| canonical RexxCPS | +2.517%, `n=12` | -0.609%, `n=34` | `rxvm` decisive favourable; `rxbvm` noisy/neutral at cap |

All 212 executions pass, no sample is removed, and no cell reaches the -3%
workload regression guard. Adrian accepted this verdict. Proportional Apple
closeout subsequently passes 1,972/1,972 full Debug tests, 6/6 focused ASan
tests, a complete Release build/install and installed VM smoke 2/2. LSan is
not supported by this macOS runtime and no local Windows toolchain is present.
Evidence:
[`2026-08-01-perf3-03-c4-first-release-verdict`](evidence/2026-08-01-perf3-03-c4-first-release-verdict/).
Closeout:
[`2026-08-01-perf3-03-c4-closeout`](evidence/2026-08-01-perf3-03-c4-closeout/).

## Resumption rule

The Apple production slice is complete. Preserve contract seeds, rejected
converter controls and every raw timing block. Before publication, run the
queued Windows/MSVC build plus focused logic/conversion and material workload
checks without reopening candidate design. Commit remains separately
user-authorized and Adrian has now given that authorization; push remains
prohibited until explicitly requested. If source, compiler, libc, locale or
current image hashes change, revalidate the affected result rather than
silently reusing it.
