# PERF3-11 K04e in-place compare/branch verdict

Status: **accepted and complete**

## Scope and provenance

- Source-control baseline:
  `56a19aae69aaed6232798a60ea99ec39d4659ccf` on
  `codex/perf3-rxas-flow-infrastructure`.
- Candidate scope: the generic K04 proof, exact opcode component/signal
  metadata, fixed/range call-use ownership, permanent fixtures and closeout
  documentation. There is no VM handler, ISA, RXBIN, ABI or language semantic
  change.
- Exact generated RexxCPS RXAS SHA-256:
  `b575305ab154f60f378f8cdbd3a44811e66368daa894d8b4add934066726707f`.
- Control: retained D0.6 ordinary Release `rxas`; candidate: the same input
  assembled by K04e ordinary profiling-off Release `rxas`.
- Runtime product: `CREXX_VM_PROFILING=OFF`; counts use the already-qualified
  schema-5 profiling build diagnostically against the exact two images.
- Host: Apple M5, Darwin 25.5.0 arm64, macOS 26.5.2, 10 logical CPUs,
  Apple clang 21.0.0, CMake 4.3.2 and Ninja 1.13.2.
- Timing ran on AC power with low-power mode off, no thermal/performance
  warning and no competing CREXX build, test or benchmark process.

## Correctness and semantic contract

K04e admits an in-place integer comparison only when component SSA supplies
the exact pre-write integer `ValueId` for the same physical result/source
register. Existing local-base, alias, hidden-cleanup, result-use, TRACE and
reachable call-window guards still apply. Fixed and zero-argument calls expose
their complete caller interface through explicit operands; only range calls
add an implicit base/count local window.

The runtime-source audit additionally records:

- `STRLEN`: source-string read, integer success write, failure-atomic
  `UNICODE_ERROR` before writes and preservation of other destination
  components;
- all three `ISUB` forms: integer success write clearing reference/native
  payloads and failure-atomic `OVERFLOW_UNDERFLOW` before writes; and
- the three-form loose `REQ` through `RLTE` family: non-signalling string
  reads with an integer result that clears reference/native payloads.

Focused Debug and ordinary profiling-off Release checks pass **4/4**. The
first broad Debug run passed 2,020/2,021 and exposed one stale M06 golden: it
expected the former invented all-local window at a zero-argument call. The
corrected permanent fixture proves the stronger intended result, passes its
focused rerun, and the final closeout panel passes **5/5** in both Debug and
Release. The unqualified broad rerun passes **2,021/2,021** in 201.24 seconds.

## Exact image and dispatch result

| Metric | D0.6 control | K04e | Change |
| --- | ---: | ---: | ---: |
| VM instructions | 1,222 | 1,221 | -1 |
| TRACE events | 1,249 | 1,248 | -1 |
| RXBIN bytes | 68,609 | 68,601 | -8 |
| target-site dispatches | 1,120,000 | 560,000 | -560,000 |

The disassembly changes only the intended hot shape:

```text
ilt r72,17,r72
.traceevent O r72
brf target,r72

    ->

ble target,r72,17
```

The schema-5 branch-site census records exactly 560,000 executions under both
VMs. Total profile counts differ by -560,028 (`rxvm`) and -559,971
(`rxbvm`) because the final timer-text/control tail varies by 28-29
low-frequency instructions; the identified site itself is exact. Both cells
return result 0 and the RexxCPS PASS marker.

## Profiling-off Release verdict

One warmup per image/VM preceded 12 balanced pairs. The min/max and confidence
rules extended the unchanged serial campaign to 24 and then the maximum 36
pairs. No sample was removed. All **144/144** canonical recorded executions
contain the canonical-default provenance marker, benchmark rate and PASS
result, with zero stderr.

| VM | Control median CPS | K04e median CPS | Paired median | Mean 95% interval | Favourable |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 45,946,845.5 | 45,992,519.5 | +0.336% | -0.819% to +1.925% | 20/36 |
| `rxbvm` | 44,891,141.5 | 44,958,635.0 | +0.757% | -0.755% to +2.617% | 22/36 |

Both intervals still cross zero at the governed limit, so runtime is
**noisy/inconclusive**, not a proven speedup. Both point estimates are
positive and neither interval reaches the -3% workload regression guard.
Adrian accepted the functionally equivalent exact dispatch reduction and this
neutral/inconclusive runtime verdict on 2026-08-04.

## Evidence map and command shapes

- `static/image.csv`: exact assembler/image identities, bytes, instructions
  and TRACE events.
- `dynamic/hot-site.csv`: exact site and whole-profile dispatch counts.
- `timing/raw.csv`: every recorded canonical CPS result in execution order.
- `timing/summary.csv` and `timing/paired-summary.csv`: governed absolute
  and paired statistics.
- `checksums.sha256`: recursive closure excluding the checksum file itself.

Material command shapes:

```text
cmake --build cmake-build-{debug,release} --target rxas rxvm rxbvm test_rxop_metadata
ctest --test-dir cmake-build-release -R <focused K04e panel>
rxas -o <image.rxbin> <exact-rexxcps.rxas>
rxdas <image.rxbin>
<profile-vm> --profile=counts --profile-output <csv> <image> library.rxbin -a --smoke-count 200
<ordinary-vm> <image> library.rxbin -a
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

Temporary full logs and products remain under
`/private/tmp/crexx-k04e-verdict.g6gG1F` and
`/private/tmp/crexx-k04e-closeout-*.log`; reproducible build output is not
duplicated here.

## Decision boundary

K04e closes parity without restoring the unsafe tactical raw-register rule.
The result does not claim a statistically proven runtime gain, change the
accepted D0.6 peephole boundary, or authorize a new optimization consumer.
The next separately governed action is the requested clean-product Mac
scorecard refresh, followed by a RexxCPS clause-opportunity reassessment.
