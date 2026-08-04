# PERF3-06 qualified-deficit closure and Mac scorecard worklist

Status: **complete — formal accepted-product Apple scorecard retained**

Started: 2026-08-04

Purpose: capture one compact same-session Apple ARM64 throughput scorecard for
the accepted PERF3 product before another production candidate changes the
baseline.  The scorecard keeps the common five, RexxCPS and Towers in their
approved lanes and makes every guard, exclusion and interpretation boundary
explicit.

## Authority and stop boundaries

- Adrian authorized PERF3-06 on 2026-08-04 after accepting the completed
  PERF3-11 D0.1-D0.5 infrastructure and consumer migration.
- This is an evidence/accounting activity.  It does not authorize a compiler,
  assembler, VM, library, language, ISA, ABI or serialized-format change.
- The ordinary profiling-off Release product at the frozen commit is the sole
  cREXX timing authority.  No build, test, profiling or other benchmark process
  may overlap a formal timing block.
- Stop on a correctness failure, AC/low-power/thermal invalidation, unresolved
  competing load or matrix-driver failure.  Preserve an invalid block and
  label it; never prune a correctness-passing sample.
- Do not turn the unmatched PERF3-01 session into a before/after regression
  claim.  Current same-session runtime ratios are formal absolute observations;
  earlier scorecards and individual candidate verdicts are context only.
- No Linux, Windows, architecture/default-VM selection, PERF3-12 compiler
  rereview, production candidate, push or publication claim belongs here.

## Exact starting state

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `codex/perf3-rxas-flow-infrastructure`
- Frozen accepted product: `5fbe36049e26ee73ea0cf1720a7fc416f33d0fe2`
  (`perf: bound and compact RXAS semantic analysis`)
- Upstream at start: `origin/develop` at
  `21fdcf529d0e51ea264bf0c92ccfbdc06dea8200`; local branch is 29 commits
  ahead and zero behind.
- Starting worktree: clean.
- Host: MacBook Air `Mac17,3`, Apple M5, 10 logical CPUs, 24 GiB RAM.
- Start power check: AC attached, battery 80%, low-power mode off.
- Comparators: ooRexx 5.1.0 r12973, Regina 3.9.7, NetRexx 5.10-GA and
  Temurin/OpenJDK 26.0.1+8.  Their executable/JAR hashes match the retained
  PERF2/PERF3-01 comparator identities.

The accepted product tranche since the PERF3-01 current-product scorecard is:

- PERF3-02 C1abc full-copy/ownership lowering for Richards and Towers;
- PERF3-03 C4 v3 private locale-aware loose-comparison prefilter;
- PERF3-10 ordered TRACE batching and trace-safe redundant conversion proof;
- PERF3-11 reusable RXAS flow/proof infrastructure, migrated proof consumers,
  retry retirement and the D0.1-D0.5 scalable-analysis closeout; and
- PERF3-05's retained ordinary L0 VM product, with its tested LTO/PGO/layout
  alternatives rejected.

## Frozen comparison contract

### Formal timing lanes

1. Common `N=5`: Sieve, Permute, Bounce, Richards and Base64 under `rxvm`,
   `rxbvm`, ooRexx and decimal-semantics NetRexx.
2. Qualified separate lane: Towers under both cREXX VMs and object-equivalent
   ooRexx; binary/JVM NetRexx remains a labelled control.
3. First-class separate lane: canonical-default cREXX RexxCPS 2.2d under both
   VMs, canonical Classic ooRexx 2.2, Regina 3.9.7 and disclosed NetRexx 2.2n.
4. Approved no-ratio dispositions remain unchanged for Mandelbrot, Storage,
   List and JSON.

The common equal-work arguments remain Sieve 5,500; Permute 5,000; Bounce
4,200; Richards 20; and Base64 2,500.  Towers remains 100 repetitions.
RexxCPS uses its canonical no-argument `100 x 100` contract.

Every absolute cell receives two warmups and ten recorded serial observations,
with runtime order rotated by workload and round.  A cell whose relative MAD
exceeds 3% or whose min/max span exceeds 10% receives the single governed
ten-sample append under unchanged conditions.  No second append is allowed.

The common score publishes four independent geometric means with exact
membership: `rxvm/ooRexx`, `rxbvm/ooRexx`, `rxvm/NetRexx` and
`rxbvm/NetRexx`.  Higher is better.  `rxvm` and `rxbvm` are never averaged.

### Evidence dimensions

PERF3-06 refreshes canonical throughput and deterministic product/image/source
identity.  It references, rather than reruns, the separately retained D0.5
assembler elapsed/RSS result and earlier lifecycle/RSS campaigns.  No lifecycle
or peak-RSS value is blended into throughput, and no stale value is presented
as a current measurement.

Expected retained root:

`performance/evidence/2026-08-04-perf3-06-mac-scorecard/`

Expected versioned manifest:

`performance/manifests/perf3-06-current-mac-v1.txt`

## Numbered execution plan

1. Replay the PERF3-01 and relevant accepted-slice checksum/hash authorities;
   verify current benchmark/comparator sources and the matrix driver.
2. Create a clean detached source and isolated ordinary Release build at the
   frozen commit, with `CREXX_VM_PROFILING=OFF`; hash all product and selected
   workload artifacts.
3. Run the minimum selected-workload correctness gate in both VM modes and
   exact formal argument/canonical RexxCPS smoke checks.
4. Freeze the immutable PERF3-06 manifest with only the isolated current
   product paths changed from the approved PERF3-01 comparison contract.
5. Capture pre-run host, power, thermal, load and process state; run the full
   serial `2 + 10` matrix and retain all outputs and raw samples.
6. Apply the one governed timing append only to cells selected mechanically by
   the formal noise rule; merge without removing any initial sample.
7. Publish current workload ratios, four `N=5` geometric means, target gaps,
   separate Towers/RexxCPS lanes, guard outcomes, exclusions and deterministic
   artifact/instruction-image changes.
8. Capture post-run state, recursively checksum the compact evidence bundle,
   update the roadmap/worklist, validate the diff and commit locally without
   pushing.

## Resumable stage ledger

### Stage A — control plane and exact freeze

- [x] Root and performance instructions reread.
- [x] Live roadmap and retained PERF2/PERF3-01 scorecard contracts reviewed.
- [x] Frozen commit, branch/upstream relation and clean worktree verified.
- [x] AC, low-power, host load and comparator availability checked.
- [x] PERF3-06 worklist created and roadmap status changed to `in progress`.

### Stage B — retained authority and exact product

- [x] Replay the exact checksum/hash authorities used by this refresh.
- [x] Verify current source and comparator identities against PERF3-01.
- [x] Create the clean detached source and profiling-off Release build.
- [x] Pass the focused dual-VM correctness and exact-argument gate.
- [x] Hash the current tools, products, sources and selected RXAS/RXBIN images.

The PERF3-01, PERF3-02 C1abc, PERF3-03 C4 and PERF3-10 recursive authorities
replay `101/101`, `12/12`, `19/19` and `37/37` rows respectively.  The matrix
driver retains PERF3-01 SHA-256 `efb194ec8194...`, all comparator products keep
their accepted hashes and `git diff 3f43a0014..5fbe36049` reports no selected
benchmark or comparator source change.

The clean detached source is
`/private/tmp/crexx-perf3-06.yHwy0b/src`; its isolated build is `Release`,
Apple-clang `-O3 -DNDEBUG` and `CREXX_VM_PROFILING=OFF`.  Focused optimized
workload and matrix-driver CTest passes `10/10`.  The exact formal arguments
pass `14/14` across both VM modes, including two canonical-default RexxCPS
provenance markers.

The cREXX product/image, current source, comparator/generated product,
manifest/tool and predecessor-authority inventory contains 79 SHA-256-bound
rows in the PERF3-06 evidence root.  The frozen manifest differs from the
approved PERF3-01 contract only in its header, cREXX product/image paths and
the clarified unchanged exclusion wording.

### Stage C — formal same-session timing

- [x] Freeze the versioned matrix manifest.
- [x] Capture a valid pre-run environment and process state.
- [x] Complete all initial `2 + 10` serial rotated cells with correct output.
- [x] Derive and run at most one governed append for selected noisy cells.
- [x] Merge summaries and publish exact common and separate-lane ratios.
- [x] Capture a valid post-run environment and process state.

The initial matrix passes `348/348` executions: 58 warmups plus 290 recorded
samples.  The governed rule selects only `bounce-oorexx`, `base64-rxvm` and
`base64-rxbvm`; their append passes `30/30`.  All three remain noise-labelled
at `n=20`, no sample is removed and no second append is taken.  The final
common means are `2.453066x/2.285744x` versus ooRexx and
`0.912280x/0.850054x` versus decimal NetRexx for `rxvm`/`rxbvm`.  Pre/post
state remains AC, low-power off, without a recorded thermal/performance/power
warning or overlapping build/test/VM process.

### Stage D — scorecard closeout

- [x] Record target progress, guard status, exclusions and interpretation
      boundaries without unmatched-session regression claims.
- [x] Record deterministic source/RXAS/RXBIN/product identities and changes.
- [x] Recursively checksum and verify the compact evidence root.
- [x] Update the roadmap and this worklist to the exact outcome.
- [x] Run focused evidence audits, `git diff --check` and full diff review.
- [x] Commit locally without pushing and describe the next roadmap item.

The final compact evidence root contains 44 recursively bound files.  All
`44/44` checksum rows verify; `checksums.sha256` itself has SHA-256
`8e70971cf46d327749db010e37acfa09383310975f241c5a60cea931cda9d734`.

## Resumption rule

Reread root/performance instructions and the live roadmap, verify the frozen
commit and dirty scope, then resume at the first unchecked item.  Before any
timing block recheck AC, low-power, thermal/load and active build/test/VM
process state.  If the source, product or environment differs, record the delta
before continuing.
