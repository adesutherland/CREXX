# PERF3-05-R2 profile-selected instruction-handler worklist

Status: **in progress — approved comparative programme through the 30% inline checkpoint**

Approved: 2026-08-09

Purpose: express every RXAS/RXBIN interpreter handler once behind an internal
definition boundary that can emit the handler directly inside the dispatch
owner or call a normal out-of-line handler function.  Use exact current-product
profiles and ordinary Release evidence to distinguish dynamically hot,
call-sensitive instructions from merely expensive instructions and measure the
effect of the selected owner size on both concrete VM engines.

## Authority and stop boundary

Adrian approved full autonomous processing on 2026-08-09 for both concrete
engines, `rxtvm` and `rxbvm`, through the formal 30% inline benchmark.  This
authority includes the isolated branch/worktree, local commits, handler macro
and call-ABI implementation, batch migration, regression repair within that
architecture, profiling and comparative evidence.

This activity must not push, merge, install, change the public RXAS/RXBIN ISA,
change a public/plugin ABI, select a production/default policy, or silently
continue into cross-platform closeout.  Stop early for an unresolved
correctness failure, inability to restore the all-inline equivalence control,
or a required public/architectural change.  Negative all-outlined and
profile-selected controls are retained evidence rather than reasons to hide or
discard the requested comparison.

The 30% denominator is the 588 non-reserved public opcodes at the approved
starting point.  At most 176 are inline in the final bounded panel.  All 62
reserved public slots and both private execution-image handlers are migrated
and measured but excluded from that percentage.

## Exact starting point

- Branch: `codex/perf3-05-r2-handler-panel`.
- Worktree: `/Users/adrian/CLionProjects/CREXX-perf3-05-r2-handler-panel`.
- Source: clean `origin/develop` at
  `6a65b9c685b3776da211bcd209af14fcf23be445`.
- The one intervening commit after the planning review changes only the
  `rxqueue` library/test stdin fixture; it does not touch the interpreter.
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs, 128 KiB L1 instruction
  cache; Apple clang 21.0.0, CMake 4.3.2 and Ninja 1.13.2.
- Ordinary Release is profiling-off `-O3 -DNDEBUG`.
- Current opcode ledger: 650 public slots, 62 reserved and 588 non-reserved;
  two additional private execution-image handlers live inside the interpreter.
- `rxvm` is a compiler-selected product alias.  Dispatch-engine evidence names
  `rxtvm` and `rxbvm`; the alias is not an independent timing cell.
- A stale `#line` directive currently maps the interpreter owner to a deleted
  temporary worktree.  Preserve it in the untouched baseline, then remove it
  before source-address attribution and prove that ordinary Release code is
  unaffected.

## Semantic invariants

- Keep canonical RXBIN and the process-private execution image unchanged.
- Preserve early next-target resolution, PC/index conversion and the stable
  noinline/noclone label owner required by direct-threaded dispatch.
- Begin, retire and terminal instrumentation exactly once per executed
  instruction in both profiling and instrumentation-test builds.
- Preserve register/reference ownership, frame activation, call/return windows,
  signal and interrupt delivery, TRACE/source identity, dynamic/native/plugin
  calls, late-loaded modules, private handlers and terminal cleanup.
- A called handler cannot jump to a label owned by `rxvm_run_owned_core()`.
  Owner-only continuations remain explicit and are completed by the owner.
- Outlined handler wrappers are force-noinline.  Helpers invoked inside either
  form retain their existing compiler inline policy.

## Design-selection record

### A — canonical state object for all handlers

Move the execution locals into one `rxvm_exec_state` object and use the same
field accesses in inline and called forms.  This makes the called ABI simple,
but changes register allocation and the all-inline owner before the experiment
has established a baseline.  Retain as a fallback only if direct-local
expansion cannot be made correct.

### B — single macro body plus owner-local/called access environments

Define each body once in coherent internal files.  Direct emission expands the
body against the existing owner locals.  Called emission expands the same body
inside a force-noinline function against a pointer facade over those locals and
returns a small owner-continuation result where normal dispatch is insufficient.
The owner performs retire/poll/dispatch and owner-only cleanup.  This is the
selected canary design because it preserves the closest possible all-inline
control while making the call boundary explicit.

### C — duplicate direct and callable implementations

Keep the current inline body and hand-write a second function form.  This can
preserve code shape initially but creates two semantic sources for every
instruction and cannot meet the maintenance objective.  Rejected.

Design B remains provisional until the canary set covers sequential, branch,
signalling, frame/call, return/terminal and private-handler paths in both VM
modes with profiling on and off.

## Measurement model

Classify instructions with independent evidence rather than total profile time
alone:

1. dynamic count share normalized within each workload;
2. breadth across the governed portfolio;
3. instrumented average/body time, treated cautiously for very cheap handlers;
4. native samples and handler/native text size;
5. measured call sensitivity from the all-inline/all-outlined controls; and
6. branch/I-cache evidence exposed by the host, with hot address footprint
   reported as a proxy when an architectural counter is unavailable.

High-count cheap handlers are expected to be more call-sensitive than
low-count expensive helpers.  Raw aggregate count must not let one long-running
workload select the entire panel.

## Work stages

### Stage A — current baseline

- [ ] Freeze clean ordinary Release and profiling builds for both VM engines.
- [ ] Record source/build/host/power provenance, hashes, `__text`, owner extent,
      dispatch sites, object/archive size, build time/RSS, lifecycle and RSS.
- [ ] Prove exact outputs and retain formal absolute timing for the governed
      representative set.
- [ ] Capture counts across Tier A plus RexxCPS and bounded native attribution.

### Stage B — framework canary

- [ ] Add the internal handler definition, policy and call-ABI surfaces.
- [ ] Cover sequential, branch, signal, call/frame, terminal and private forms.
- [ ] Build and run focused dual-VM Debug/Release/profiling checks.
- [ ] Compare the canary all-inline form to the untouched code shape.

### Stage C — complete handler migration

- [ ] Migrate reserved/load/copy/simple numeric handlers.
- [ ] Migrate comparison/branch/float/decimal/conversion handlers.
- [ ] Migrate string/Unicode/PARSE/binary/stem handlers.
- [ ] Migrate attribute/reference/lifetime/private handlers.
- [ ] Migrate call/frame/return/signal/metadata/plugin handlers.
- [ ] Migrate filesystem/spawn/environment/network/terminal handlers.
- [ ] Verify exactly one semantic definition for every public/private handler.

Each batch receives focused correctness and build checks before a local commit;
major boundaries receive a broad Debug CTest sweep.

### Stage D — all-inline control

- [ ] Build every handler directly inside the owner.
- [ ] Pass full Debug correctness and exact output/count checks.
- [ ] Compare preprocessed shape, symbols, owner/text/artifact size, dispatch
      form, build cost/RSS and paired ordinary Release timing to Stage A.
- [ ] Repair or stop on an unexplained semantic/all-inline equivalence failure.

### Stage E — all-outlined control

- [ ] Force every handler wrapper out of line and verify symbols/disassembly.
- [ ] Pass dual-VM correctness and instrumentation balance.
- [ ] Measure call/return cost, owner/text size, build/RSS, lifecycle, product
      RSS, native footprint and representative timing.

### Stage F — profile-selected panels

- [ ] Produce a replayable ranked handler ledger from the frozen profiles.
- [ ] Compare cumulative 5%, 10%, 20% and at-most-30% inline policies.
- [ ] Start with one common panel; add a VM-specific 30% control only when the
      `rxtvm`/`rxbvm` evidence demonstrates a material ranking divergence.
- [ ] Run the formal balanced 30% comparison against current, all-inline and
      all-outlined controls on both concrete engines.

### Stage G — evidence and stop

- [ ] Retain one compact checksum-closed evidence bundle with raw samples.
- [ ] Update this worklist, `ROADMAP.md` and `RXVM_INTERPRETER.md`.
- [ ] Review and locally commit the bounded result without pushing.
- [ ] Report and stop at the 30% checkpoint before production selection or
      cross-platform/default-VM work.

