# cREXX performance governance

Status: approved standing policy

Approved: 2026-07-20

This document is the normative authority for performance measurement,
aggregation, regression decisions and publication. The dated programme report
defines the original `NR-*` charter; `ROADMAP.md` records live status;
`evidence/benchmark-median-summary.md` indexes results. Neither is a substitute
for this policy.

## Principles

1. Correctness, equivalence and provenance precede timing.
2. Canonical product results remain separate from diagnostics and controls.
3. Throughput, lifecycle, memory and artifact size are independent dimensions.
4. An aggregate never hides a material per-workload regression.
5. Same-session paired evidence is required where environmental drift can
   affect a before/after decision.
6. Raw observations remain available, but repository evidence stays compact
   and purpose-led.
7. New or changed performance tooling is cREXX Level B, not Python.

## Portfolio policy

### Tier A coverage portfolio

The approved Tier A portfolio contains eleven steady-state workloads and one
separately reported lifecycle lane:

| Workload | Role | Common aggregate |
| --- | --- | --- |
| RexxCPS | Classic mixed semantics and benchmark-native clause rate | no; cREXX and NetRexx are disclosed adaptations |
| Sieve | integer arrays, indexing and nested iteration | yes |
| Permute | recursion, calls, returns and array mutation | yes |
| Mandelbrot | floating point, branches and bit operations | no; ooRexx fails the common checksum and NetRexx adds timed arithmetic helpers |
| Towers | objects, allocation and recursion | no; the ooRexx procedural cell removes the intended object/allocation work |
| Bounce | object/reference dispatch and short-lived allocation | yes |
| Storage | sustained allocation and tree construction | no; the cREXX node wrapper changes allocation work |
| List | linked-list construction and reference mutation | no; the cREXX weak-reference arena remains materially adapted |
| Richards | larger call graph and scheduler state transitions | yes; all three runtimes use the same disclosed state-machine representation |
| JSON | parser/text/collection capability | no; the native APIs construct different representations |
| RFC 4648 Base64 | binary/byte processing and checksum observation | yes |
| compile/load/first-result | lifecycle | reported separately; never a throughput aggregate input |

The initial common aggregate is therefore exactly **Sieve, Permute, Bounce,
Richards and Base64** across CREXX, ooRexx and NetRexx.

Regina remains RexxCPS-only. Java and native C remain labelled controls rather
than mandatory portfolio members. A valid result excluded from the common
aggregate stays visible with its comparability label and reason.

### Tier B and reserve

Queens, DeltaBlue, Havlak, filesystem-I/O work and focused Classic-semantics
probes remain Tier B/reserve. Run a reserve workload when a concrete coverage
question requires it; do not silently add it to Tier A or a published
aggregate.

A workload enters Tier A or changes aggregate membership only after explicit
approval, NR-02-equivalent provenance/correctness/equivalence qualification and
a formal baseline. Version aggregate membership. Never compare geometric means
whose membership differs.

## Result modes and labels

### Canonical result

The canonical CREXX score uses the ordinary profiling-off CMake `Release`
product, optimized workload image, separately loaded current `library.rxbin`,
retained source/TRACE metadata and the documented workload argument. Report
`rxvm` and `rxbvm` separately.

Canonical ooRexx, Regina and NetRexx results use the qualified source/runtime
mode recorded by the portfolio plan. A translated ooRexx image, altered JVM
mode or otherwise different execution form is a separate result.

Canonical NetRexx common-portfolio cells use `options nobinary decimal` and
keep timed arithmetic, loop/state values and numeric arrays in NetRexx `Rexx`
decimal semantics. Generated Java and the default HotSpot JIT are the normal
NetRexx implementation/runtime substrate and are fair parts of that result;
record the JDK/JVM and retain the generated Java/classes. A source compiled
with `options binary`, or one whose timed numeric state is expressed through
primitive Java numeric types, is a labelled binary/JVM control and is excluded
from the common Rexx aggregate. Host-Java storage required by a workload, such
as Base64 `byte[]`, is permitted only when disclosed and when its timed numeric
work remains in `Rexx` decimal semantics.

### Diagnostic and control modes

Opaque-input/result-observation, no-TRACE, no-opt, profiling,
stripped-metadata, translated-image, binary-typed NetRexx, adapted and
research-control runs retain exact labels. They may explain or bound a
canonical result but never silently replace it or enter the common aggregate.

Use these interpretation labels:

- `release claim`: every formal gate below is met for a named commit/platform;
- `observation`: retained factual result without every release-claim gate or
  without a matched comparator;
- `diagnostic`: noncanonical evidence used to explain behaviour;
- `inference`: causal interpretation supported by observations but not directly
  measured as the product outcome; and
- `control` or `upper bound`: intentionally different Java/native-C/research
  mechanism, excluded from canonical scores and aggregates.

## Measurement dimensions

Publish separate tables for:

1. correctness and comparability;
2. canonical workload throughput or benchmark-native rate;
3. compile, translate, assemble and load-to-first-result lifecycle;
4. peak resident set size (RSS) and, when relevant, CREXX profiling allocation,
   value and frame counters; and
5. source, generated-source, RXAS, RXBIN, linked-image, class and package size.

Do not combine these dimensions into a single score. A deployment, flexibility
or observability benefit belongs in interpretation alongside its measured cost;
it does not waive a throughput regression.

## Sampling and environment

### Minimum samples

| Purpose | Warmups | Recorded observations | Claim boundary |
| --- | ---: | ---: | --- |
| qualification pilot | at least 1 | at least 3 | never a release claim |
| formal absolute baseline | 2 | 10 per cell | formal same-host observation |
| formal before/after decision | at least 1 per cell | 12 paired rounds | regression/performance decision |
| peak RSS | 0 | 3 per canonical cell | separate memory result |
| artifact size | n/a | 1 hash-bound observation | deterministic dimension |

All samples run serially. Rotate runtime order by workload/round in a formal
cross-runtime baseline. For before/after decisions, balance and interleave the
cells in the same session.

For common elapsed-time workloads, use one common argument across CREXX,
ooRexx and NetRexx that makes the fastest pilot at least 1.0 second. If this
would make another runtime exceed 30 seconds per sample, do not silently use
unequal work. Keep the result process-inclusive/diagnostic until a separately
normalized kernel contract is approved.

An initial normalized-kernel exception was approved on 2026-07-20 after a
version-1 calibration appeared to put the fastest cells at 0.050-0.272 seconds
while the slowest were already 2.330-15.569 seconds. That premise came from
binary-typed NetRexx ports and is withdrawn for the NR-10 common aggregate. The
corrected decimal-NetRexx qualification supports one equal argument per
workload with fastest medians at 1.062-1.182 seconds and slowest medians at
1.614-16.061 seconds, so NR-10 uses the ordinary equal-work contract.

If a future workload genuinely cannot meet the equal-work boundary and Adrian
approves a normalized-kernel exception, each runtime/workload cell must:

- calibrate a disclosed integer work count into a 3-10 second target window;
- retain two calibration points with identical semantics and admit the cell
  only when their normalized `work / process elapsed` rates agree within 5%;
- include process startup in elapsed time and retain the exact work count in
  every raw sample;
- use the unchanged formal two-warmup/ten-recorded schedule after calibration;
  and
- label the publication `normalized throughput`, never `equal-work elapsed`.

If a cell fails the two-point check, exclude it from the aggregate without
imputation and retain it as a diagnostic until the contract is reviewed.

### Machine conditions

Run formal campaigns on AC power with low-power mode off. Record before and
after:

- host, OS, CPU and logical CPU count;
- battery/AC and low-power state;
- thermal-pressure state and system load;
- compiler, CMake, generator and build options;
- runtime/JDK/JVM versions and non-default flags; and
- source, tool, executable and workload-artifact hashes.

Do not run builds, tests, package updates, other benchmark campaigns or known
heavy background work concurrently.

## Summary statistics and uncertainty

For every absolute cell report recorded sample count, minimum, first quartile,
median, third quartile, maximum and median absolute deviation (MAD). The median
is the canonical point estimate; raw observations remain authoritative.

For paired decisions report:

- paired median percentage change;
- paired first/third quartile and favorable count;
- mean paired percentage change; and
- a two-sided 95% Student-t interval around the mean paired percentage.

This interval convention matches the accepted NR-09 decision evidence. Label
the interval as applying to the mean even when the paired median is the headline
point estimate.

If relative MAD exceeds 3% or the recorded min/max span exceeds 10%, append ten
serial samples under unchanged conditions. If a paired interval crosses zero or
a regression guard, append another 12 balanced pairs, up to 36 total. A series
that remains ambiguous is `noisy/inconclusive`; do not select a favourable
subset.

Remove no sample without an independently demonstrated fault. Record the exact
sample, fault and disposition. Normally invalidate and repeat the whole affected
block rather than delete one observation.

## Ratios and geometric means

Orient every throughput ratio so larger is better:

- elapsed-time workload: `reference median elapsed / CREXX median elapsed`;
- normalized-throughput or native-rate workload:
  `CREXX median rate / reference median rate`.

Give each included workload equal weight. For `N` named ratios `r_i`, calculate:

```text
G = exp(sum(ln(r_i)) / N)
```

Publish four independent common aggregates:

- `rxvm / ooRexx`;
- `rxvm / NetRexx`;
- `rxbvm / ooRexx`; and
- `rxbvm / NetRexx`.

Do not average CREXX VMs or references together. Do not impute a missing,
failing, `not comparable` or materially adapted cell. Name every exclusion,
publish `N` and list the exact membership beside every aggregate.

## Regression budget

The default budget for future production performance changes is:

| Dimension | Guard |
| --- | --- |
| correctness | zero failures |
| common Tier A throughput | no more than 1% regression in any published common geometric mean |
| comparable Tier A workload | no more than 3% regression in any individual workload |
| lifecycle | escalate when regression is greater than both 5% and 1 ms in a named phase |
| peak RSS | escalate when regression is greater than both 5% and 1 MiB |
| artifact size | escalate when regression is greater than both 5% and 4 KiB |

A guard hit requires a same-session paired/interleaved rerun, causal review and
an explicit Adrian decision to rework, revert or accept the trade-off. An
aggregate gain never waives a per-workload, lifecycle, memory or artifact guard.

Do not call a difference between unmatched sessions a regression. Run the
retained accepted product/image in the new session as a drift control. Published
historical medians remain observations, not substitutes for that control.

## Release-claim gate

A performance result may enter release material only when it identifies:

- clean source commit or tag candidate and platform scope;
- canonical profiling-off Release product and exact VM mode;
- complete named portfolio and aggregate membership/version;
- correctness/equivalence status and every exclusion;
- formal sample minimums, raw samples and uncertainty;
- source/build/runtime/machine/power provenance and exact commands;
- source/generated/internal-form/executable hashes;
- lifecycle, RSS and artifact-size results;
- outstanding regression-guard decisions; and
- interpretation boundary: claim, observation, diagnostic, inference or upper
  bound.

Cross-platform or release-wide wording requires evidence for every platform it
names. A same-host NR-10 result is a claim only for that named host/platform.

## Compact evidence retention

Retain one final bundle per formal campaign with:

- README/interpretation, machine-readable provenance, exact schedule/commands
  and recursive checksums;
- consolidated warmup/recorded samples and correctness/output tables;
- scorecard source tables and rendered Markdown;
- source/runtime/tool/build/artifact hashes and sizes; and
- only the generated/internal forms necessary to audit a forensic claim.

Keep calibration, failed setup attempts, superseded trial schedules, ordinary
build products and reproducible duplicate binaries outside the repository.
Reference stable earlier forensics rather than copying them. Preserve
decision-relevant negative/noisy samples and invalid-sample records.

Review every retained file for an audit or reproduction purpose. Do not prune
existing historical bundles merely to apply this policy retrospectively.

## Document ownership

- `performance/AGENTS.md`: mandatory concise standing instructions.
- `performance/README.md`: operational workflow and document/evidence index.
- `performance/portfolio/cross-runtime-plan.md`: scope, comparability and
  baseline-entry rules.
- this document: normative measurement, aggregation, regression and claim
  policy.
- `performance/templates/performance-scorecard.md`: publication structure.
- `performance/evidence/benchmark-median-summary.md`: results index only.
- `performance/ROADMAP.md`: live status and dated decisions.
