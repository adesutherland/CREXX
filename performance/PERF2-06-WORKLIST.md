# PERF2-06 VM execution-engine worklist

Status: VM-C1b explicitly accepted with recorded `rxbvm` code-layout debt;
Debug/Release closeout green and VM-C2 clean-base PoC authorized

Started: 2026-07-26

Purpose: resumable control plane for the current-HEAD execution-image,
dispatch/fetch, residual call/frame, interrupt/cold-path, lifecycle and
cross-platform audit. This activity may produce isolated prototypes and a
measured production ladder, but it does not authorize a production slice,
public RXAS/RXBIN/ABI change, language change, commit or push.

## Decision gate and mandatory stop

PERF2-06 must present the complete current-HEAD attribution and bounded PoC
panel before any production slice is installed. The package must identify the
lowest-cost owner for remaining VM work across steady-state time, startup,
private image/RSS, text size, lifecycle, compatibility, observability and
maintenance.

**Stop point:** present the measured VM-B through VM-F panel, recommended
independently measurable ladder and precise first production slice to Adrian,
then stop for architecture/production selection. Do not install the ladder,
run broad closeout, stage, commit or push.

## Scope and ownership guard

- [x] Preserve canonical immutable RXBIN, public RXAS, public ABI and language
      semantics.
- [x] Keep profiling-off Release timing as product authority; profiles,
      RXSEQ, native stacks and counters remain diagnostics.
- [x] Keep compiler/inliner, RXAS, link/load, private preparation and runtime
      facts assigned to their earliest safe owner.
- [x] Keep PERF2-07 whole-value/reference-payload ownership separate unless a
      fresh VM-engine mechanism is independently identified.
- [x] Preserve per-instruction interrupt semantics, TRACE/source/profile/RXSEQ
      identity, signals/unwind, late load, plugins and both VM modes.
- [x] Preserve canonical benchmark inputs and work counts. Name optimized,
      no-opt, stripped-metadata and other controls separately.
- [x] Keep maintained analysis/orchestration in cREXX Level B. Native host
      sampling may supply diagnostic input; Python is not a maintained path.
- [x] Preserve the dated programme charter and closed historical evidence.

## Numbered execution plan

1. Freeze repository, evidence, host, power, toolchain and build state.
2. Read and trace the current preparation, dispatch, call/frame, interrupt,
   profiling, late-load and teardown implementation plus focused tests.
3. Build independently identified profiling-off and diagnostic current-HEAD
   products in isolated source/build directories.
4. Refresh the least VM-owned attribution needed for both VMs, keeping
   steady-state, startup/preparation and teardown separate.
5. Complete the VM-B through VM-F placement/candidate panel with exact
   invariants, machine ceilings, lifecycle costs and cross-platform blockers.
6. Prototype only the strongest candidates needed to distinguish ownership
   and architecture; gate timing on correctness and reduced machine work.
7. Reconcile stale execution-image documentation, consolidate the evidence,
   recommend one production ladder and stop for Adrian's decision.

## Stage 0 - exact starting state and evidence integrity

### Repository at start

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- HEAD: `e7090198e45002a6a73b654f6d98b9eb91d2e5cb`
- Subject: `perf: relink exact references privately`
- Upstream: `origin/develop` at
  `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`
- Relation: `+6/-0`
- Initial worktree: clean
- Publication: nothing from the six local commits has been pushed by this
  activity.

### Host, power and toolchain at start

- Capture: `2026-07-26T14:59:01Z`
  (`2026-07-26T15:59:01+0100`).
- Host: Adrian's MacBook Air, `Mac17,3`, Apple M5, arm64, 10 logical CPUs,
  24 GiB RAM.
- OS: macOS 26.5.2 build 25F84; Darwin 25.5.0.
- Power: AC attached, battery 80%, low-power mode `0`; no recorded thermal,
  performance or CPU-power warning.
- Load: `2.09 2.96 6.46`; uptime 12 days 9:07.
- Free storage: approximately 629 GiB on the data volume and `/private/tmp`.
- Toolchain: CMake 4.3.2, Ninja 1.13.2, Apple clang 21.0.0
  (`clang-2100.1.1.101`).
- Existing ordinary Release: Ninja, `Release`, `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`, `BUILD_TESTING=ON`, NETWORK TLS.
- Existing diagnostic Release: same compiler/options with
  `CREXX_VM_PROFILING=ON`.
- Existing Debug: Ninja, `Debug`, `CREXX_VM_PROFILING=OFF`,
  `BUILD_TESTING=ON`, NETWORK TLS.

Formal or bounded timing blocks must recapture AC, low-power, thermal and load
before and after, run serially, and avoid overlapping builds/test campaigns.

### Accepted evidence independently replayed

| Bundle | Verified rows | Manifest SHA-256 |
| --- | ---: | --- |
| PERF2-01 current baseline | 1,948/1,948 | `fde9aa923fc451ea16edc0c4f09bf532faa2e024aa22fbfc72e35a965f78eacc` |
| PERF2-02 quickening PoC | 136/136 | `f2953489e4d03ee7be8211f69182adbd406fe9fa2f0ed7e08f962cc1e5f97239` |
| PERF2-02 first Release verdict | 23/23 | `405ab71b8a0ac2ccf60f94588de46d27882da89c912f177885ac6e4a16510201` |
| PERF2-05 semantic-assist panel | 2,073/2,073 | `9a564829ecd9c4a51ec752aef0c116b5e97e6bc5082574801b62200590e99426` |
| PERF2-05 R1a first Release verdict | 35/35 | `916c6f729812a7f8d51b69282e98ec9505f5f35d50a713e2ee089fa5f3a2ae95` |

Replay logs are temporary diagnostics under
`/private/tmp/perf2-06-checksums.G2LF2s/`; the checksum-closed repository
bundles remain authoritative.

### Product isolation contract

- Current-HEAD baseline source: clean detached worktree at exact HEAD; no
  worklist/documentation edits and no reused mutable main build directory.
- Diagnostic source: a separate clean detached worktree at the same exact HEAD
  with profiling enabled; never a product-timing binary.
- Each PoC: its own detached source and build directory, exact tracked patch
  hash, source commit, CMake cache, product hashes and input hashes.
- Baseline, candidate and diagnostic products never share a mutable build
  directory.
- Scratch build products and superseded trials remain outside the repository;
  only compact decision-relevant evidence may be retained.

- [x] Exact branch, HEAD/upstream and clean starting state verified.
- [x] Host, power, toolchain and existing configuration state recorded.
- [x] All five accepted evidence manifests used by this activity replayed.
- [x] Independent product/diagnostic/PoC identity contract defined.
- [x] Design/selection stop recorded before production edits or PoCs.

## Stage 1 - current source and documentation model

- [x] Root and performance instructions, live roadmap/governance and dated
      charter boundary read.
- [x] Current execution-image, preparation, dispatch/fetch and active-frame
      code traced in both modes.
- [x] Current frame allocation/reuse, argument binding, result placement,
      numeric-context and interrupt inheritance code traced.
- [x] Current interrupt scan/delivery, TRACE/profile identity, late-load,
      lifecycle and teardown paths traced.
- [x] PERF2-01, PERF2-02 and PERF2-05 attribution/selection evidence read and
      checksum-verified.
- [x] Documentation discrepancy confirmed: both VMs own a process-local
      `bin_code[]` execution image. `rxvm` binds handler pointers in instruction
      cells; `rxbvm` dispatches copied canonical/private opcodes from that owned
      image. Canonical `segment.binary` remains the reflection/source image.
- [x] Reconcile `RXVM_INTERPRETER.md` and the dated dispatch investigation's
      now-stale implemented-result wording without rewriting historical
      measurements.

## Stage 2 - current-HEAD VM attribution

- [x] Freeze isolated source/build/input hashes and validate focused dual-VM
      correctness.
- [x] Record execution-image cells/bytes touched and preparation cost for the
      exact current image set.
- [x] Refresh dispatch/transition versus handler/helper attribution for both
      VMs.
- [x] Refresh residual dynamic call population and frame-entry subphases after
      PERF2-03 through PERF2-05.
- [x] Separate interrupt polling, taken scans, TRACE/debug and numeric-context
      inheritance/synchronization.
- [x] Record whole-value transfer/copy/clear/conversion counts and bytes while
      preserving PERF2-07 ownership.
- [x] Establish verification/copy, binding, private preparation, plugin,
      first-frame, load-first-result and teardown boundaries.
- [x] Capture Apple ARM64 native stacks/text/branch or cache data where the host
      toolchain permits; record unavailable counters without invention.
- [x] Retain startup-inclusive and steady-state results separately.

## Stage 3 - ranked placement and candidate panel

For every viable candidate record exact selecting workloads/cost, earliest safe
owner, invariants, fallback/failure, preparation/first-hit/steady-state/RSS/
image/text/teardown cost, late-load invalidation, both VM modes, identity,
mathematical correctness, machine-work ceiling and disposition.

- [x] VM-B current wide-cell image versus compact switch/operand overlay,
      hot/cold representation, accepted zero-state private handlers and only
      proved semantic combinations.
- [x] VM-C shared/COW interrupt state, changed-only numeric activation,
      proved-leaf/light frames, compiler-coordinated result/arguments and
      shape-specific reset/pooling where current call evidence justifies them.
- [x] Add VM-C2 as a first-class segmented, non-moving value arena plus compact
      control-stack option. Keep register alias/overlay lookup as its decisive
      machine-work question and place value representation jointly with
      PERF2-07.
- [x] VM-D interrupt-poll component/layout and cold-path options without
      weakening delivery or TRACE/debug identity.
- [x] VM-E Apple ARM64 result plus reproducible Linux ARM64, Linux x86-64 GCC/
      Clang and Windows x86-64 matrix with explicit unavailable blockers.
- [x] VM-F CAP-04 load-only boundary and preparation/lifecycle ranking.
- [x] Retain or overturn each named negative result only with new evidence.

## Stage 4 - bounded PoCs

- [x] Select only candidates whose current mechanism footprint can distinguish
      ownership or execution architecture.
- [x] Record isolated source/product/patch hashes and identical exact inputs.
- [x] Pass focused correctness plus fewer instructions, fetches, branches,
      copies or other exact measured operations before formal timing.
- [x] Measure both VMs, preparation/startup, steady-state, RSS/private image,
      text size, teardown and unrelated guard cells.
- [x] Retain neutral/rejected candidates and exact rejection reasons.

## Stage 5 - mandatory selection package

- [x] Exact current-HEAD attribution for `rxvm` and `rxbvm`.
- [x] Ranked VM-owned candidate and ownership table.
- [x] Complete execution-stream, call/frame, interrupt/cold-path, lifecycle and
      cross-platform panels.
- [x] Bounded PoC results, including neutral/rejected forms.
- [x] Documentation discrepancies and reconciliations.
- [x] Independently measurable production ladder.
- [x] Precise first production slice, retained baseline and decisive Release
      cell.
- [x] One disposition: private stream/layout, call/frame, narrower helper,
      documentation/cross-platform only, or no production change.
- [x] Report to Adrian and stop; no production implementation, broad closeout,
      stage, commit or push.

## First mandatory stop result

The decision package is retained at
`evidence/2026-07-26-perf2-06-vm-audit/`. Current-head evidence selects the
residual call/frame path:

- Permute/List have 432,950/572,500 bytecode calls but only 7/44 fresh frames;
  current recycling already eliminates hot allocation.
- Every child call still copies 1,280 interrupt-table bytes, totaling
  554,176,000/732,800,000 bytes. Native List samples attribute 7.4% in both
  VMs to `memmove`.
- The bounded COW control removes that copy and passes 65/65 focused tests, but
  grows `run()` by 10,700/6,268 bytes and is clearly adverse on Base64 and
  Sieve `rxbvm`. The exact patch is rejected.
- VM-C1b, a shared policy/table with one centralized cold mutation/OOM path,
  is the recommended precise first production slice if Adrian selects it.
- VM-C2, Adrian's non-moving segmented value arena plus compact control stack,
  is the recommended broader architecture PoC option. It advances only if it
  can remove most local pointer relinks without imposing an alias-overlay tax
  on every operand.
- Private stream and lifecycle changes are deferred: exact current preparation
  is only 306-331 us for 1,416,984 owned bytes and Apple hardware counters are
  unavailable.

At this first-stop point selection was open. The approval recorded below
supersedes only that production-edit block; broad closeout, VM-C2, commit and
push remain blocked.

## Approved production selection - VM-C1b

Adrian approved the recommended sequence on 2026-07-26: implement VM-C1b as
the precise first production slice, then stop at its mandatory profiling-off
Release verdict. VM-C2 remains the next bounded architecture option; approval
of VM-C1b does not authorize that follow-on PoC or broad closeout.

Comparative selection:

1. **Selected:** shared/COW interrupt-policy inheritance with one centralized
   cold mutation/OOM path. Child entry becomes pointer/ownership assignment;
   the 1,280-byte copy moves to the first actual frame-local mutation. The
   selected shape must avoid repeated failure/dispatch blocks in `run()` and
   remain compatible with VM-C2's later compact control stack.
2. **Rejected production shape:** the retained bounded COW patch. It proves the
   mechanism but repeats mutation failure handling at each opcode, grows
   `run()` by 10,700/6,268 bytes and clearly regresses `rxbvm` guards.
3. **Deferred alternative:** sparse handler overlays. They may avoid a full
   first-mutation copy but add lookup/ownership structure and are not needed to
   prove the selected inheritance mechanism.
4. **Deferred architecture:** VM-C2 segmented non-moving values plus compact
   control frames. Its register-alias lookup question requires a separate
   comparative PoC after this verdict.

Machine gate: eliminate exactly one 1,280-byte interrupt-table copy per child
bytecode call, reduce allocated frame-block size by 1,264 bytes, add no
per-dispatch work, preserve deterministic instruction/call/branch/value counts,
and keep `run()` growth within 1 KiB in each VM.

Correctness gate before timing: focused dual-VM recursion, signal-code and
sentinel bounds, handler push/pop, breakpoint, signal call/unwind, references,
instrumentation identity and late-load coverage only. Once these pass, freeze
the implementation and build the isolated profiling-off Release candidate.

First verdict: exact retained current product and images remain the baseline.
List optimized work 100 in both VMs is the primary cell; Permute optimized work
50 confirms recursive calls and Sieve optimized work 50 guards unrelated
layout. Start with 12 paired balanced rounds and extend only under governance.
Report immediately and stop without broad CTest, sanitizer, install/package,
documentation polish, VM-C2 work, commit or push.

## VM-C1b mandatory first Release verdict

The exact verdict package is retained at
`evidence/2026-07-26-perf2-06-vm-c1b-first-release-verdict/`. Its checksum
ledger is regenerated after the accepted closeout and diagnostic additions.

- [x] Focused dual-VM source signal, runtime signal/instrumentation, reference
      lifetime, late-load and LOADMODULE checks pass.  The final narrowed
      signal regression passes 52/52 after the last layout refinement.
- [x] The child 1,280-byte copy is removed and `stack_frame` falls from 1,432
      to 168 bytes, exactly the required 1,264-byte reduction.
- [x] `run()` clears the 1 KiB growth ceiling: `rxvm` shrinks 5,660 bytes and
      `rxbvm` shrinks 796 bytes versus the exact retained baseline.
- [x] The initial 12 balanced pairs pass all 144 recorded executions without
      exclusion.  List `rxvm` and both Permute cells are clear favorable; List
      `rxbvm` and Sieve `rxvm` are inconclusive.
- [x] Sieve `rxbvm` is clear adverse: paired mean `+5.368694%`, 95% interval
      `[+4.720473%, +6.016915%]`, 0/12 favorable. No append was warranted.
- [x] The adverse cell was diagnosed before acceptance. Sieve executes no
      bytecode calls or handler-table mutation, and a work-500 control scales
      the regression, excluding COW execution and one-time root allocation as
      the mechanism. Apple Clang changes global code generation within the
      approximately 536 KiB flattened `rxbvm` `run()` body; a narrow alignment
      pad control does not recover the result.
- [x] Adrian explicitly accepted the measured trade-off on 2026-07-26 because
      the call-heavy gains and faster `rxvm` Sieve justify retaining VM-C1b.
      The accepted known debt is `PERF2-06-D01`, documented in
      `CODE-LAYOUT-DEBT.md` in the verdict package.
- [x] Full Debug build and CTest pass 1,924/1,924.
- [x] Full ordinary profiling-off Release build and CTest pass 1,924/1,924.
- [x] The implementation commit is recorded by exact SHA in a follow-up ledger
      commit after the implementation commit exists: `TO_BE_RECORDED`.

**Accepted closeout:** VM-C1b is production-selected with `PERF2-06-D01` as
explicit debt, rather than reclassifying the adverse Sieve result as neutral.
ASan, install/package and a cross-platform campaign were not required by this
shortest accepted closeout. Adrian authorized VM-C2 as the next separate PoC;
it must start from the clean committed base and remain independently
discardable.

## Resumption rule

Reread root/performance instructions and the live roadmap, verify Git state and
the accepted evidence actually used, then resume from the first unchecked box.
Recheck host/power state before every measurement block. Preserve isolated
product identity and never overwrite a passing raw block or blur diagnostic and
profiling-off evidence.
