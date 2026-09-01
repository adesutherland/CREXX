# PERF2-02 stable-site semantic quickening worklist

Status: complete; favorable Release verdict accepted and broad QA passed

Started: 2026-07-23

Purpose: resumable control plane for the Q0-Q7 design comparison and isolated
reference/value PoC panel selected by accepted PERF2-01 evidence. The worklist
did not itself authorize a production quickener, public RXAS/RXBIN/ABI change,
or language-semantic change; the later Stage 5/6 decisions record Adrian's
explicit production approval and verdict acceptance.

## Decision gate and mandatory stop

PERF2-02 exits only when the retained evidence identifies the fastest correct
owner for the exact Bounce reference-storage and Richards value-copy sites,
including machine ceiling, both VMs, guards, startup/RSS/state bytes and
lifecycle. The final result must recommend exactly one disposition named in the
handover prompt and propose at most one smallest production slice.

**Stop point:** present the measured panel and recommendation to Adrian, then
stop for architecture selection. Do not install the winning PoC in production,
begin broad closeout, stage, commit or push.

## Scope guard

- [x] Preserve canonical RXBIN, public RXAS, public ABI and language semantics.
- [x] Preserve every pre-existing main-worktree change; do not reset, stash,
      overwrite, stage, commit or push.
- [x] Keep diagnostic schema-5 source separate from profiling-off product and
      PoC timing trees.
- [x] Exclude selector/BIF/general-dispatch/fusion/representation/pooling/JIT
      campaigns from this activity.
- [x] Treat static/direct placement and PERF2-06/07 owner reassignment as valid
      outcomes rather than presuming quickening wins.
- [x] Retain the dated charter, closed initial roadmap and accepted PERF2-01
      bundle unchanged.

## Numbered execution plan

1. Freeze repository, evidence, host, power, toolchain and build state.
2. Complete mandatory evidence, VM, handler/helper and focused-test reading.
3. Trace Bounce and Richards source through optimized RXAS/RXBIN and both VMs.
4. Complete the semantic-invariant/site tables and Q0-Q7 plus companion ADR.
5. Create detached clean scratch worktrees and one hash-bound patch per PoC.
6. Build machine-level/direct controls before persistent-site variants.
7. Run focused correctness/lifecycle fixtures, then bounded unprofiled Release
   target, guard, startup, RSS, image/state and teardown measurements.
8. Consolidate evidence, update roadmap/worklist, recommend one owner and stop.

## Stage 0 - exact state and prototype isolation

### Repository and accepted baseline at start

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- HEAD: `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`
- Upstream: `origin/develop` at the same commit; ahead/behind `+0/-0`
- Accepted product source: clean detached worktree
  `/private/tmp/crexx-perf2-01-product.dbLYqo` at the exact commit above.
- Accepted product binaries: `rxvm`
  `2bb27f8a31298e20a67b6a986040580f83e5bcd7261b1795333e3d36762ca871`,
  `rxbvm`
  `a738271972e52ea8a0cf0981eb86d58179c6f715824595eb8ca576737e360e15`,
  and `library.rxbin`
  `a9b660f6a67fd57fa35ae180a6b3c0f2764d44241fc0272bd6c6b843ce5d8e10`.
- Accepted bundle manifest SHA-256:
  `fde9aa923fc451ea16edc0c4f09bf532faa2e024aa22fbfc72e35a965f78eacc`.
- Independent bundle verification: `1948/1948` checksum rows passed on
  2026-07-23; verifier exit status `0`.
- Accepted limitations: no Apple hardware event counters; noisy Base64 and
  NetRexx Richards labels retained; selector attempts are zero; profile/RXSEQ/
  native/heap observations are diagnostic, not product timing.

### Main-worktree dirty scope before the first PERF2-02 edit

The main worktree contains intentional PERF2-00/01 work. Product/diagnostic
source is therefore **not identical**: product source is clean HEAD, while the
main tree carries the accepted schema-5 diagnostic extension. The diagnostic
tracked patch over `docs/...profiling`, `docs/...RXVM_INTERPRETER.md` and
`interpreter/` hashes to
`69032b4e694776f2fac105696aa182d93a013dfbc931166f204dfe695642cba5`.
The complete tracked pre-PERF2-02 diff hashes to
`72b1401b69aeea538f7ee7035f9e35f97ad889e5858a30034775c3ea8d063ee3`.

Pre-existing modified tracked paths:

```text
docs/ai-context/RXVM_INTERPRETER.md
docs/books/crexx_programming_guide/profiling.md
interpreter/CMakeLists.txt
interpreter/rxvminstrument.h
interpreter/rxvminstrument_profile.h
interpreter/rxvmintp.c
interpreter/rxvmmain.c
interpreter/rxvmprofile.c
interpreter/rxvmprofile.h
interpreter/rxvmvars.h
interpreter/tests/check_vm_call_census.cmake
interpreter/tests/check_vm_profile_csv.cmake
interpreter/tests/check_vm_profiling_docs.cmake
interpreter/tests/test_rxvmprofile.c
performance/README.md
performance/ROADMAP.md
performance/evidence/benchmark-median-summary.md
performance/tools/run_evidence_bundle.crexx
```

Pre-existing untracked roots/files:

```text
performance/PERF2-01-HANDOVER-PROMPT.md
performance/PERF2-01-WORKLIST.md
performance/PERF2-02-HANDOVER-PROMPT.md
performance/ROADMAP-INITIAL-SWEEP-2026-07-23.md
performance/TEAM-PERFORMANCE-UPDATE-2026-07-23.md
performance/evidence/2026-07-23-perf2-01-current-baseline/
performance/tools/build_perf2_gap_ledger.crexx
performance/tools/summarize_perf2_artifacts.crexx
performance/tools/summarize_perf2_profiles.crexx
tests/performance/rexxcps_family_controls.crexx
```

PERF2-02 adds only this worklist, its architecture record, its evidence bundle
and the bounded live-roadmap updates to the main worktree. Prototype product C
source remains outside it.

### Host, power and tools at start

- Host: `Mac.lan`, model `Mac17,3`, Apple M5, arm64, 10 logical CPUs,
  24 GiB RAM; Darwin `25.5.0`, macOS `26.5.2` build `25F84`.
- Power: AC attached, battery 80%, AC low-power mode `0`; no recorded thermal,
  performance or CPU-power warning.
- Load at capture: `2.72 3.32 2.32`; uptime 9 days 14:37.
- Storage: approximately 640 GiB free on the data volume and `/private/tmp`.
- Toolchain: CMake `4.3.2`, Ninja `1.13.2`, Apple clang `21.0.0
  (clang-2100.1.1.101)`.
- Existing main build trees: `cmake-build-release` = Release/profiling off,
  `cmake-build-debug` = Debug/profiling off, `cmake-build-profile` =
  Release/profiling on; all use Ninja and `BUILD_TESTING=ON`.

Formal/bounded timing blocks must recapture AC, low-power, thermal and load
before and after, run serially, and avoid overlapping builds or campaigns.

### Scratch strategy

Every source PoC starts from a detached clean worktree at the accepted commit.
Each gets a separate source directory, build directory, patch, patch SHA-256,
binary hashes and CMake cache under a temporary PERF2-02 scratch root. No PoC
imports the schema-5 patch unless a written coverage gap requires it; any such
diagnostic tree is separate and never supplies product timing.

| Prototype | Clean source/build | Patch identity | Status |
| --- | --- | --- | --- |
| Q0 same-session drift control | exact accepted product worktree/binaries | no patch | complete |
| Q3a broad direct semantic helper/control | detached clean HEAD | `b4782927...` | complete; ceiling only |
| Q3b exact same-guard direct control | detached clean HEAD | `e673e61e...` | complete; measured winner |
| Q4 eager process-private form | detached clean HEAD | `5f9d81ce...` | complete; rejected against Q3b |
| Q5 lazy first-hit form | represented by Q7 COPY client | included in Q7 patch | pruned; no learned useful fact |
| Q6 threshold/tiered form | design comparison only | no patch | pruned with semantic reason |
| Q7 reusable private substrate | detached clean HEAD | `87ca836e...` | complete; rejected |
| diagnostic counters/profiles | separate build-private runs only | never timed as product | complete; Q3b/Q4 per-site hits/fallbacks and Q7 transitions/first-hit sample retained |

- [x] Exact branch, HEAD/upstream and dirty scope captured.
- [x] Accepted bundle independently checksum-verified.
- [x] Product/diagnostic source relationship and hashes recorded.
- [x] Host, power, thermal, load, toolchain and build configurations recorded.
- [x] Scratch isolation and per-prototype provenance contract defined.

Blockers: none at Stage 0. Any public format/ABI/language/irreversible
architecture proposal becomes `decision required` and stops.

## Stage 1 - semantic unit and stability proof

The pre-implementation matrix and design-selection table live in
[`PERF2-02-ARCHITECTURE.md`](PERF2-02-ARCHITECTURE.md). They must be completed
from code/test evidence before the first C PoC.

- [x] Exact Bounce source/procedure/module/instruction site table complete.
- [x] `MKREF`, `LINKREF`, `SETREF`, `DEREF`, `LINKATTR*`, `ENDLIFE`, owner
      lookup/cell identity/invalidation/recycled-frame paths traced.
- [x] Local/argument/global/attribute/caller-owned/nested storage proven.
- [x] Exact Richards source/procedure/module/instruction site table complete.
- [x] `COPY_REG_REG` entries separated by scalar/reference/string/decimal/
      binary/native/object/recursive/cleanup/alias shape.
- [x] Static, load-time and execution-only facts separated for both families.
- [x] Smallest guard, mutation/invalidation events and generic fallback named.
- [x] Schema-5 coverage gap disposition written before any telemetry edit.
- [x] Focused test inventory and missing fixtures recorded.

## Stage 2 - Q0-Q7 and companion decision record

- [x] Q0 canonical fallback characterized.
- [x] Q1 compiler-owned/static ceiling characterized and bounded PoC measured.
- [x] Q2 existing authored-RXAS control pruned in favor of direct symbol remap.
- [x] Q3a and exact same-guard Q3b direct controls implemented and measured.
- [x] Q4 eager specialization implemented and measured against Q3b.
- [x] Q5 lazy first-hit form represented by Q7 COPY and pruned with reason.
- [x] Q6 threshold/tiered specialization pruned: no durable learned fact exists.
- [x] Q7 reusable substrate serves two families or one family plus a convincing
      semantic fixture, and is compared with identical one-off fast semantics.
- [x] Companion strategies retain stable IDs, proof obligations and separable
      incremental measurements.
- [x] State bytes/site/module, state machine, failure/concurrency/publication,
      invalidation and dual-VM costs complete for every viable option.

At least Q0 plus two plausible forms must be prototyped for the leading family.

## Stage 3 - focused correctness and bounded product evidence

- [x] Machine-level inline/direct primitive ceiling retained.
- [x] Reference/value alias, storage-class, recursive/native and lifetime tests
      pass for implemented surfaces.
- [x] Prepare-only, repeated run, RXVML/re-entry, late-load, TRACE/source,
      signal/unwind, profiling/RXSEQ identity and both dispatch modes pass in
      proportion to each prototype.
- [x] Allocation/preparation failure remains canonical and failure-atomic for
      the recommended zero-state direct surface; Q7 gaps are explicit rejection reasons.
- [x] Canonical Bounce measured in `rxvm` and `rxbvm`.
- [x] Canonical Richards measured in both VMs for Q1 and Q7 owner controls.
- [x] Storage, Towers, Sieve, Permute and Base64 guard cells retained.
- [x] First-hit disposition, steady-state, startup/load-first-result, RSS,
      state/image bytes and teardown retained: Q3b/Q4 have no learned first-hit
      transition; Q7 first-specialize/disable events have a separate perturbed
      diagnostic sample, while balanced product timing remains authoritative.
- [x] Separate build-private diagnostics retain per-site executions, exact hits,
      guard misses/fallbacks and Q7 state transitions. Q3b/Q4 invalidation is
      not applicable because they retain no learned state; Q7's missing general
      invalidation/reference-dequick mechanism remains an explicit rejection gap.
- [x] Raw samples, outputs, commands, host state and exact hashes retained in
      `performance/evidence/2026-07-23-perf2-02-quickening-poc/`.

## Stage 4 - decision package

- [x] Architecture record contains final site facts, Q0-Q7 dispositions,
      companion outcomes and accepted/rejected reasoning.
- [x] Prototype patch/build provenance and compact evidence are checksum-closed.
- [x] Recommendation chooses exactly one permitted disposition.
- [x] Smallest proposed production slice is stated only because evidence supports it.
- [x] Live roadmap records the bounded result, including negative/reassigned
      ownership.
- [x] PERF2-02 is marked `decision required` only after its stated
      exit is met.
- [x] Adrian received the mandatory decision package; no production work,
      staging, commit or push followed before explicit approval.

## Final disposition

The one recommendation is **direct value/reference helper work belongs first in
PERF2-07/PERF2-06**. Exact Q3b is 5.066075x/4.651793x faster than Q0 on Bounce,
7.584% faster than eager Q4 in `rxvm`, and tied with Q4 in `rxbvm`. Q7 adds
state/lifecycle cost without a target benefit. Richards Q1 separately reassigns
its removable receiver-copy work to the compiler/inliner.

Adrian approved the exact zero-state Q3b A-LOCAL/A-ATTR guard inside canonical
`MKREF_REG_REG` on 2026-07-24, with unchanged fallback and public interfaces.
Adrian then accepted the favorable mandatory first Release verdict and
authorized broad closeout on the same date.

Evidence closure: `checksums.sha256` verifies 136/136 non-manifest bundle files
and hashes to
`f2953489e4d03ee7be8211f69182adbd406fe9fa2f0ed7e08f962cc1e5f97239`.

## Stage 5 - approved production slice and first Release verdict

- [x] Adrian approved the exact Q3b slice on 2026-07-24.
- [x] Roadmap/worklist moved from `decision required` to provisional
      implementation before the production edit.
- [x] Exact canonical-handler edit and focused dual-VM guard fixture complete.
- [x] Minimum focused correctness passes in both VM modes (10/10 focused Debug
      CTest plus the new Release fixture in both VMs).
- [x] Implementation frozen and ordinary profiling-off Release product built.
- [x] Smallest decisive Q0/production Bounce comparison retained. Paired
      medians are -80.596%/-78.464% elapsed (`rxvm`/`rxbvm`), all 12/12 and
      22/22 pairs favorable with wholly favorable mean 95% intervals. The
      required ten-sample `rxbvm` absolute-noise append is retained unchanged.
- [x] First Release verdict reported to Adrian; stop for direction before
      broad validation or closeout.

## Stage 6 - accepted verdict and broad closeout

- [x] Adrian accepted the favorable first Release verdict and authorized every
      remaining QA step plus a local commit on 2026-07-24.
- [x] Full Debug build passed after repairing the pre-existing schema-5 test
      instrumentation backend; full Debug CTest passed 1907/1907.
- [x] Full ordinary profiling-off Release build and CTest passed 1907/1907;
      the production `rxvm`/`rxbvm` hashes remained identical to the measured
      first-verdict binaries.
- [x] Full macOS AddressSanitizer build and CTest passed 1907/1907 with no ASan
      finding. LeakSanitizer is unsupported by the Apple runtime and was
      explicitly disabled rather than claimed.
- [x] Isolated install produced 133 files; its installed toolchain compiled and
      ran `examples/hello.crexx` natively.
- [x] Both installed VMs passed the accepted pre-change optimized Bounce RXBIN
      and library, preserving the public bytecode compatibility gate.
- [x] No CPack/package target is configured; the isolated installed-tree test
      is the repository's applicable packaging gate.
- [x] All six maintained Level B performance-tool self-tests pass against the
      final ordinary Release toolchain.
- [x] PERF2-01 and PERF2-02 retained bundle checksums independently verify.
- [x] The staged maintained-source/document scope passes `git diff --check`;
      hash-bound raw captures and prototype patches retain their original
      whitespace and all evidence bundles are checksum-closed.

## Resumption rule

Reread root/performance instructions and the live roadmap, verify the accepted
bundle and current Git state, then resume from the first unchecked box. Recheck
host/power state before every measurement block. Never overwrite a passing raw
block or blur diagnostic and profiling-off product evidence.
