# NR-10/NR-11 resumable baseline and governance worklist

Status: complete

Started: 2026-07-20

This is the temporary control plane for the formal cross-runtime baseline and
the durable performance-governance system. The dated 2026-07-15 programme
report remains a historical charter and is not edited by this activity.

## Starting state

- Branch: `develop`
- Starting commit: `1596d7c8cbbfd360b66d50423ef4ea80320fd885`
- Starting worktree: clean
- Branch relation: three local commits ahead of `origin/develop`
- Delivery boundary: local documentation/evidence changes only; do not push
- NR-09: complete and out of scope unless this audit exposes a concrete factual
  error

## State vocabulary

- `pending`: required work has not started;
- `in progress`: evidence or documentation is being assembled;
- `pass`: the named factual gate is satisfied and points to retained evidence;
- `reused`: existing retained evidence satisfies the gate without a rerun;
- `decision required`: Adrian must approve the proposed policy or scope;
- `blocked`: an external runtime or indispensable input is unavailable;
- `n/a`: the gate does not apply to the named result.

## Hard boundaries

- Do not add compiler, assembler, linker or VM production code.
- Do not create a new reusable benchmark harness unless an indispensable gap is
  demonstrated and Adrian approves it.
- Any new or changed performance orchestration, provenance, statistics,
  aggregation, checksum or scorecard tool must be written in cREXX Level B,
  after reading `docs/ai-context/CREXX_LEVELB_AUTHORING.md`. Do not implement
  NR-10/NR-11 tooling in Python. Historical evidence-local Python scripts stay
  historical and are not the model for this activity.
- Keep the repository evidence footprint deliberate. Retain one final NR-10
  bundle with consolidated raw tables and only decision-relevant generated or
  internal forms. Keep calibration, failed setup attempts, rebuild products and
  superseded scratch campaigns outside the repository. Reuse existing NR-02
  forensics by reference instead of copying them. Do not delete existing
  historical evidence as part of NR-10/NR-11.
- Reuse `performance/tools/run_cross_runtime.crexx`,
  `performance/tools/run_lifecycle.crexx`, the approved Tier A sources and the
  NR-02 equivalence/forensic evidence.
- Do not run a formal baseline until Adrian approves the portfolio,
  aggregation, sampling, uncertainty and regression-budget policy below.
- Keep canonical product results separate from opaque-input, no-TRACE,
  profiling, stripped-metadata and research-control diagnostics.
- Never infer a regression from unmatched sessions without a same-session
  drift control.

## Retained-evidence audit

The audited NR-02 roots are:

- `evidence/2026-07-15-nr-02-cross-runtime/`;
- `evidence/2026-07-15-nr-02-portfolio-expansion/`; and
- the live result index `evidence/benchmark-median-summary.md`.

Together the two NR-02 bundles contain 498 files and 172 sample rows. The
cross-runtime captures contain 38 warmups and 113 recorded rows with 151
stdout/stderr pairs; 111 recorded rows pass and the two retained failures are
the expected ooRexx Mandelbrot negative checks. The lifecycle ledger contains
21 passing phase rows. All 41 files in the expansion bundle's generated-form
checksum manifest verify.

| NR-10 charter requirement | Existing evidence | Audit disposition / remaining work |
| --- | --- | --- |
| Same-host runtime versions | Apple M5/macOS host, CREXX build, ooRexx 5.1.0 r12973, Regina 3.9.7, NetRexx 5.10-GA and JDK 26.0.1 are recorded | `reused` for qualification; recapture versions, binary hashes and host/power state in the current formal session |
| Raw results and correctness | Per-cell CSV plus raw stdout/stderr is retained; lifecycle rows retain correctness | `reused` for qualification only; most cells have one warmup plus three recorded runs and are explicitly non-formal |
| Source provenance and licences | Per-workload lineage, licences, substitutions and correctness contracts are recorded | `reused`; add exact current source hashes to the formal bundle |
| CREXX internal forms | NR-02 records optimized retained-metadata mode; later evidence retains current RXAS/RXBIN and exact hashes | `partial`; retain current formal source, RXAS, RXBIN, linked-library and VM hashes for both `rxvm` and `rxbvm` reporting |
| Optimizer-elimination proof | RexxCPS canonical/opaque A/B perturbations, observed state, opt/no-opt checks and partial/current CREXX profile evidence are retained; every other workload has runtime arguments and an observed correctness result | `reused` as qualification; canonical formal scores remain separate from these diagnostics |
| ooRexx forensics and operation counts | Executable translated images exist for every applicable workload; RexxCPS has canonical/A/B images and a bounded count-one trace with 990 source and 4,012 intermediate kernel records | `reused`; do not time translated images as though they were source-mode results |
| NetRexx generated form | Generated Java, Java 8 class files and `javap -c -p` inspection are retained across the portfolio; result-observation paths are documented | `reused`; hash the exact generated forms used by the formal run |
| Regina scope | Canonical/A/B RexxCPS 2.2 plus trace evidence is retained | `reused`; Regina remains RexxCPS-only and outside the common aggregate |
| Java/native-C controls | NetRexx generated Java is retained, but no independent Java portfolio control or the charter's two labelled native-C RexxCPS ceilings was selected | `decision required`; keep controls outside Tier A and add only the smallest evidence-only native-C ceiling pair if Adrian confirms the charter requires it for NR-10 closure |
| Reproducible commands | Per-cell manifests retain exact argv/cwd/expectation and the two existing Level B tools are versioned | `partial`; the formal root must retain the full schedule, build commands, environment and recursive checksums |
| Machine/power conditions | Host/CPU/OS are recorded; the later O3 rerun discloses battery/AC drift | `missing` for a formal campaign; require AC power, low-power state and pre/post thermal/load capture |
| Sampling and uncertainty | Serial raw samples and medians exist; no valid sample was dropped merely for looking extreme | `missing` for policy; NR-02 sample counts are too small for a formal baseline and no programme-wide uncertainty/rerun rule exists |
| Startup/lifecycle separation | Compile/translate, CREXX assemble and load-to-first-result are separate rows | `reused` structurally; repeat formally and add the `rxbvm` load-to-first-result result separately |
| Memory | NR-03+ profiling retains CREXX allocation/value/frame diagnostics | `missing` for the cross-runtime formal baseline; retain peak RSS separately from throughput and from CREXX profiling counters |
| Artifact size | Generated forms exist and the expansion generated-form hashes verify | `partial`; publish source/generated-source/class/RXAS/RXBIN/linked-image sizes as a separate dimension |
| Current product state | NR-09 retains a final same-session 12-round RexxCPS result for both VMs | `partial`; it is not a cross-runtime NR-10 run, and the NR-02 cREXX sources/build predate material benchmark and product changes through current HEAD |

### Audit conclusion

NR-02 satisfies the portfolio qualification, equivalence, optimizer-resistance
and most runtime-forensics prerequisites. It does **not** satisfy NR-10's
formal-baseline exit criterion because it declares itself non-formal, uses
small pilot sample counts, lacks formal power/uncertainty/RSS/complete artifact
provenance, predates the current cREXX product, and has no native-C ceiling
pair. A bounded current same-host campaign is therefore required if NR-10 is to
be marked complete.

## Policy proposal — decision required

Adrian approved this policy as drafted on 2026-07-20. The accepted rules are
now normative in `PERFORMANCE-GOVERNANCE.md`; this section remains the decision
record.

### 1. Portfolio and comparability

Recommended:

- Keep the approved Tier A portfolio at 12 coverage items: 11 steady-state
  workloads plus the separately reported lifecycle lane.
- Define the initial common CREXX/ooRexx/NetRexx aggregate as exactly five
  steady-state workloads: Sieve, Permute, Bounce, Richards and Base64.
- Treat Richards as eligible because the same disclosed state-machine
  representation and correctness contract is used across all three runtimes;
  disclosure does not create unequal timed work between cells.
- Report RexxCPS separately rather than in the common aggregate because the
  cREXX 2.2d and NetRexx 2.2n sources are disclosed adaptations of canonical
  Classic 2.2.
- Exclude Mandelbrot, Towers, Storage, List and JSON from the initial common
  aggregate under their current `not comparable`, materially adapted or open-
  review dispositions. Keep their valid results visible as diagnostics.
- Keep Regina RexxCPS-only. Keep Java and native C as labelled controls, never
  as mandatory portfolio members or aggregate inputs.
- Keep Queens, DeltaBlue, Havlak, filesystem I/O and focused Classic probes in
  Tier B/reserve. A Tier B workload enters Tier A or changes an aggregate only
  after explicit approval, NR-02-equivalent qualification and a formal
  baseline; historical aggregates retain their original membership/version.

Decision: `pass` — approved 2026-07-20

#### Equal-work calibration result — 2026-07-20

After the focused Release build and 31/31 benchmark-labelled correctness tests
passed, a disposable one-sample calibration exercised the five approved common
workloads with one equal argument per workload. NetRexx completed the cells in
0.050-0.272 seconds while the slowest runtime was already at 2.330-15.569
seconds. Scaling NetRexx to the approved 1.0-second minimum would push multiple
slowest cells beyond the 30-second cap. The disposable manifest and outputs
remain outside the repository as required.

This triggers the approved equal-work rule: the cells must remain process-
inclusive diagnostics unless Adrian separately approves a normalized kernel
contract. No formal samples have been retained and the five-workload aggregate
has not been published. The smallest proposed contract is:

- calibrate a disclosed per-runtime work count to a 3-10 second target window;
- retain at least two calibration points per cell and require consistent work
  semantics plus normalized rates within 5% before admitting the cell;
- report `work / process elapsed` as normalized throughput, including process
  startup in every sample;
- use `CREXX normalized rate / reference normalized rate`, still higher-is-
  better, with the same five-workload membership and four separate geometric
  means; and
- retain the exact work count per cell and label this as a normalized-throughput
  aggregate, not an equal-work elapsed aggregate.

Decision: `pass` — approved 2026-07-20

#### NetRexx numeric-mode correction — 2026-07-20

The first capture then exposed a concrete qualification error: the five common
NetRexx sources used `options binary`, so their timed arithmetic/state compiled
to primitive Java operations rather than NetRexx decimal `Rexx` semantics.
Those results and the 0.006220/0.006149 CREXX/NetRexx aggregates are withdrawn
as Rexx comparisons and retained only as binary/JVM controls. The default
HotSpot JIT remains canonical and fair.

The common sources and lifecycle probe were corrected to `options nobinary
decimal`, with timed numeric state in `Rexx` values; Base64 retains only its
disclosed Java `byte[]` storage. A fresh 1+3 equal-work qualification found
fastest medians of 1.062-1.182 seconds and slowest medians of 1.614-16.061
seconds. The premise for the unequal-work exception therefore no longer holds:
the canonical manifest v3 uses one common argument per workload and the
ordinary equal-work contract. The uncommitted unequal-work decimal capture was
deleted as superseded scratch rather than added to the repository.

### 2. Metrics, ratios and geometric means

Recommended:

- Publish separate scorecards for throughput, lifecycle/startup, peak RSS,
  artifact size and correctness. Never combine these dimensions into one
  scalar score.
- Use the median recorded result for each canonical cell. Report sample count,
  minimum, first quartile, median, third quartile, maximum and median absolute
  deviation (MAD).
- Orient every throughput ratio so larger is better:
  `reference elapsed / CREXX elapsed`, or `CREXX rate / reference rate` for a
  native rate such as RexxCPS.
- Give every included workload equal weight. For named ratios `r_i`, compute
  `G = exp(sum(ln(r_i)) / N)`.
- Publish four common aggregates independently:
  `rxvm/ooRexx`, `rxvm/NetRexx`, `rxbvm/ooRexx` and `rxbvm/NetRexx`. Do not
  average the two CREXX VMs or the two references together.
- Do not impute a missing, failing, `not comparable` or materially adapted
  cell. Exclude it, name it, publish `N` and the exact membership, and do not
  compare aggregate values whose membership differs.
- Keep `rxvm` and `rxbvm` as distinct product rows in every throughput,
  lifecycle, memory and artifact table where the VM affects the result.

Decision: `pass` — approved 2026-07-20

### 3. Sampling, environment and uncertainty

Recommended numerical policy:

- Qualification pilot: at least one warmup plus three recorded samples;
  never a release claim.
- Formal absolute baseline: two warmups plus ten recorded serial samples per
  cell. Rotate runtime order by workload/round so one runtime is not always
  first or last.
- Formal before/after regression decision: at least one warmup per cell and 12
  recorded paired rounds in a balanced/interleaved schedule. Report the paired
  median percentage, paired IQR, favorable count, and the two-sided 95%
  Student-t interval around the mean paired percentage, matching the retained
  NR-09 convention.
- Peak RSS: three serial recorded executions per canonical cell, outside the
  throughput samples. Artifact size is deterministic and needs one hash-bound
  observation.
- For common elapsed-time workloads, choose one common argument across the
  three runtimes that makes the fastest pilot at least 1.0 second. If that
  makes any runtime exceed 30 seconds per sample, do not silently use unequal
  work; keep the cell process-inclusive/diagnostic unless a separately
  normalized kernel contract is approved.
- Run on AC power with low-power mode off; record pre/post battery, thermal,
  load, OS/CPU and tool versions. Run no builds, tests, package updates or other
  benchmark sessions concurrently.
- If relative MAD exceeds 3% or the recorded min/max span exceeds 10%, append
  ten more serial samples under unchanged conditions. For paired decisions
  whose 95% interval crosses zero or a regression guard, append another 12
  balanced pairs, up to 36 total. If the series remains ambiguous, publish it
  as noisy/inconclusive rather than selecting a favorable subset.
- Remove no sample without an independently demonstrated fault. Record the
  exact sample, fault and disposition; normally invalidate and repeat the
  whole affected block rather than delete one observation.

Decision: `pass` — approved 2026-07-20

### 4. Regression budget and escalation

Recommended numerical policy for future production performance changes:

- Correctness budget: zero failures.
- Common Tier A throughput budget: no more than a 1% regression in any
  published common geometric mean.
- Per-workload guard: no comparable Tier A workload may regress by more than
  3%, even when the aggregate improves.
- Lifecycle guard: escalate a regression greater than both 5% and 1 ms in any
  named phase.
- Peak-RSS guard: escalate a regression greater than both 5% and 1 MiB.
- Artifact-size guard: escalate a regression greater than both 5% and 4 KiB.
- A guard hit requires a same-session paired/interleaved rerun, causal review
  and an explicit Adrian decision to rework, revert or accept the trade-off.
  An aggregate gain never waives a per-workload, lifecycle, memory or artifact
  guard.
- Do not compare an unmatched new session with an older published median and
  call the difference a regression. Run the retained accepted product/image in
  the new session as a drift control, as the corrected NR-09 campaign did.

Decision: `pass` — approved 2026-07-20

### 5. Claims and interpretation labels

Recommended:

- `release claim`: clean identified source/tag candidate, canonical
  profiling-off Release product, complete named portfolio/membership, formal
  sample minimums, correctness, hashes, raw data, uncertainty and no unresolved
  regression guard; the host/platform scope is explicit.
- `observation`: a factual result from a complete retained cell that does not
  meet every release-claim gate or has no matched comparator.
- `diagnostic`: opaque-input, no-TRACE, profiling, no-opt, stripped-metadata,
  adapted or otherwise noncanonical evidence used to explain behavior.
- `inference`: a causal interpretation supported by observations but not
  directly measured as the claimed product outcome.
- `control` or `upper bound`: Java/native-C/research mechanisms that deliberately
  differ from the shipped product; never a canonical score.

Decision: `pass` — approved 2026-07-20

### 6. Repository evidence retention

Working rule confirmed by Adrian: preserve enough evidence to reproduce and
audit a decision without accumulating every transient file in source control.

Recommended retained set for the single final NR-10 bundle:

- one README/interpretation record, one machine-readable provenance manifest,
  the exact run schedule/commands and recursive checksum file;
- consolidated warmup/recorded raw-sample and correctness/output tables rather
  than one stdout and one stderr file per successful sample;
- scorecard source tables plus rendered Markdown;
- source commit and source hashes, runtime/tool hashes and versions, build
  options, artifact hashes and sizes;
- only the textual/generated/internal forms needed for a forensic claim, such
  as current cREXX RXAS or a newly required generated form; and
- decision-relevant negative/noisy samples and independently justified invalid
  sample records.

Keep outside the repository in a task-specific temporary directory:

- calibration runs, failed setup attempts and superseded trial schedules;
- ordinary build-tree products and reproducible duplicate binaries;
- duplicate copies of NR-02 Java/class/`javap`, ooRexx translation and trace
  evidence already referenced by stable repository paths; and
- convenience logs whose facts are already present in the retained manifest or
  consolidated raw tables.

Before finalizing, review every new evidence file for a specific audit or
reproduction purpose. File count is not itself a quality metric.

## Smallest proposed formal NR-10 campaign — do not run yet

1. Rebuild the ordinary profiling-off Release product from the approved clean
   commit and pass the focused benchmark correctness gate.
2. Capture one compact checksum-closed provenance root: Git state,
   build/cache/options,
   compiler/tools, runtime/JVM versions and flags, host/power/thermal/load,
   exact commands, source/generated/internal-form/binary hashes and sizes.
3. Run the 11 Tier A steady-state workloads on CREXX `rxvm`, CREXX `rxbvm`,
   ooRexx and NetRexx wherever the NR-02 disposition permits a valid result.
   Preserve the known `not comparable`/diagnostic labels. Run Regina only for
   canonical RexxCPS.
4. Use the formal sample/schedule rules above. Publish the five-workload common
   aggregate and keep RexxCPS plus the other valid Tier A diagnostics outside
   it.
5. Repeat the lifecycle phases with ten recorded observations; report CREXX
   compile, assemble and load-first-result, ooRexx translate/load-first-result,
   NetRexx compile/load-first-result, and a separate `rxbvm`
   load-first-result row.
6. Capture peak RSS separately with three executions per canonical valid cell,
   and retain the source/generated-source/class/RXAS/RXBIN/linked-image size
   matrix.
7. Reuse NR-02's canonical/opaque, ooRexx trace and NetRexx generated-form
   forensics by reference. Refresh only hashes/current cREXX proof needed to
   bind them to the current baseline; do not duplicate old generated artifacts.
8. If approved as required for charter closure, add two evidence-only RexxCPS
   native-C ceiling controls: mechanical scalar control flow and a more
   faithful dynamic-value form, each with an explicit non-eliminable result
   sink. Label both controls/upper bounds and exclude them from aggregates.
9. Produce the standard scorecard, verify recursive checksums and reproducible
   commands, and link the bundle from the result index and roadmap.

If the existing Level B runners cannot capture or report a required formal
field, propose the smallest bounded Level B extension or reporting tool before
editing it. Do not fill the gap with Python.

Campaign state: `pass` — corrected equal-work decimal-NetRexx evidence is
retained compactly at `evidence/2026-07-20-nr-10-formal-baseline/`; the initial
binary matrix is explicitly excluded from the common aggregate.

## Durable documentation ownership

| Document | Approved content to own | State |
| --- | --- | --- |
| `performance/AGENTS.md` | mandatory sampling, drift-control, outlier, canonical-mode, evidence-retention and regression-escalation instructions | pass |
| `performance/README.md` | operational workflow and links to governance, template, portfolio and evidence index | pass |
| `performance/portfolio/cross-runtime-plan.md` | Tier A/B scope, exact common subset, comparability/baseline-entry rules and control boundaries | pass |
| `performance/PERFORMANCE-GOVERNANCE.md` | normative formulas, metrics, sample minimums, uncertainty, regression budget and claim taxonomy | pass |
| `performance/templates/performance-scorecard.md` | standard publication structure and required tables/provenance | pass |
| `performance/evidence/benchmark-median-summary.md` | dated result index and explicit exclusions only; not policy authority | pass |
| `performance/ROADMAP.md` | live status and dated decisions | pass |

## Completion gates

### NR-10

- [x] Audit the retained NR-02 evidence against every charter requirement.
- [x] Obtain approval for portfolio membership, controls and formal sampling.
- [x] Retain the current same-host formal baseline with separate `rxvm` and
  `rxbvm` results.
- [x] Retain versions, raw results, exact commands, source/internal-form
  provenance, correctness, optimizer-resistance references, ooRexx/NetRexx
  forensics, approved controls, memory, artifact and lifecycle evidence.
- [x] Verify recursive checksums and reproducibility instructions.
- [x] Review the final evidence inventory and remove only task-created scratch
  or reproducible duplicates that have no audit purpose; do not prune existing
  historical bundles in this activity.
- [x] Publish the factual scorecard and result-index entry.
- [x] Mark NR-10 complete only when the historical charter's exit criterion is
  met.

### NR-11

- [x] Obtain approval for numerical thresholds, aggregation, missing-cell,
  outlier, regression-budget and claim policies.
- [x] Read `docs/ai-context/CREXX_LEVELB_AUTHORING.md` before any approved
  `.crexx` tooling edit and keep all new/changed tooling in Level B.
- [x] Put mandatory future-agent rules in `performance/AGENTS.md`.
- [x] Put operational navigation in `performance/README.md`.
- [x] Put portfolio/comparability rules in `portfolio/cross-runtime-plan.md`.
- [x] Create the focused governance authority and standard scorecard template.
- [x] Prove the template with the NR-10 publication.
- [x] Validate internal links and policy consistency.
- [x] Run `git diff --check` and review the final diff.
- [x] Update this worklist and `performance/ROADMAP.md` with the accepted dated
  decisions and factual completion state.

## Resumption point

NR-10 and NR-11 are closed with the corrected equal-work decimal-NetRexx
baseline. Future production performance edits use `PERFORMANCE-GOVERNANCE.md`,
the scorecard template and this formal baseline as an unmatched historical
observation; regression verdicts still require a new same-session accepted-
product drift control. Do not reinterpret the absolute baseline as a matched
regression result or promote the retained binary/JVM control into a Rexx score.
