# PERF3-05-R2 profile-selected instruction-handler worklist

Status: **complete at the approved Apple 30% inline checkpoint — no production policy selected**

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

Design B passed the canary and complete-migration gates for sequential, branch,
signalling, frame/call, return/terminal and private-handler paths in both VM
modes with profiling on and off. It is retained as the internal measurement
framework; that does not select a non-default placement policy.

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

## Checkpoint result

The implementation expresses 651 handlers exactly once: 649 opcode/sentinel
handlers plus two private execution-image handlers. `INTERRUPT` remains the
owner's internal dispatch target and is not an RXAS-executable handler. The
definitions are grouped into five internal files and can produce an inline
body or a force-noinline function call without changing the instruction source.

The frozen 22-profile common ranking observed 184 instruction forms. With each
workload normalized before aggregation, cumulative dynamic count share is:

| Inline handlers | Public share | Cumulative dynamic share |
| ---: | ---: | ---: |
| 29 | 4.93% | 75.07% |
| 59 | 10.03% | 91.35% |
| 118 | 20.07% | 99.767% |
| 176 | 29.93% | 99.9999969% |

The leading ten are `BRF_ID_REG`, `UNLINK_REG`,
`LINKATTR1_REG_REG_INT`, `BR_ID`, `IEQ_REG_REG_INT`, `LOAD_REG_INT`,
`BRT_ID_REG`, `ICOPY_REG_REG`, `IADD_REG_REG_INT`, and
`IGT_REG_REG_REG`. This is the dynamic-heat answer; it does not by itself
predict the compiler's fastest owner layout.

### Static shape

| Shape | Engine | `run()` extent | `__text` | Product file | Outlined symbols |
| --- | --- | ---: | ---: | ---: | ---: |
| untouched | `rxtvm` | 535,556 | 831,532 | 1,020,584 | 0 |
| untouched | `rxbvm` | 530,528 | 827,180 | 1,020,760 | 0 |
| all-inline | `rxtvm` | 532,512 | 828,488 | 1,020,632 | 0 |
| all-inline | `rxbvm` | 531,868 | 828,520 | 1,020,808 | 0 |
| all-outline | `rxtvm` | 31,824 | 913,248 | 1,135,064 | 651 |
| all-outline | `rxbvm` | 32,268 | 902,588 | 1,135,032 | 651 |
| profile-30 | `rxtvm` | 200,160 | 888,228 | 1,109,656 | 475 |
| profile-30 | `rxbvm` | 200,584 | 881,236 | 1,093,096 | 475 |

All-outline brings the complete owner under the host's 128 KiB L1I size;
profile-30 does not. Total text grows because the callable bodies remain in the
binary. Size alone is not an acceptance test.

### Formal Release comparison

The balanced matrix ran seven workloads, three shapes and both engines with two
warmups and twelve retained rounds. All 588 executions passed; all 504 recorded
samples remain in the evidence bundle. Values below are performance change
against all-inline, so negative is worse. RexxCPS uses its reported rate and
the others use elapsed time.

| Workload | `rxtvm` profile-30 | `rxbvm` profile-30 | `rxtvm` all-outline | `rxbvm` all-outline |
| --- | ---: | ---: | ---: | ---: |
| Sieve | -14.07% | -19.02% | -50.59% | -43.11% |
| Permute | -20.24% | -19.59% | -62.65% | -58.31% |
| Bounce | -16.56% | -29.20% | -64.18% | -55.37% |
| Richards | -2.24% | -4.99% | -18.49% | -14.30% |
| Base64 | -3.12% | -1.67% | -24.77% | -16.75% |
| Towers | -1.73% | -1.38% | -9.81% | -9.11% |
| RexxCPS | -5.49% | -4.41% | -23.62% | -22.56% |
| geometric mean | **-9.35%** | **-12.08%** | **-40.02%** | **-34.24%** |

This is not primarily call overhead. The exact profiles for Sieve, Permute,
Bounce, Richards, Base64 and Towers execute zero profile-30 outlined handlers.
RexxCPS executes eight outlined instructions out of 23,569,107 on `rxtvm` and
eight out of 22,947,535 on `rxbvm`. The adverse movement therefore comes almost
entirely from compiler optimization/code layout/register allocation/branch
placement caused by the changed owner population. That directly confirms the
original instability concern: a frequency-perfect panel can still be a slower
code shape.

The 29.93% panel is a completed negative checkpoint, not a product candidate.
Intermediate 5/10/20% timing products were not pursued after this stronger
result: the 30% panel already contains every materially executed instruction
in six workloads, so smaller frequency-only panels cannot distinguish call
cost from the now-proved layout effect. A follow-on must first introduce a
layout-controlled selection method or compiler/native attribution; it is
outside this approval.

## Work stages

### Stage A — current baseline

- [x] Freeze clean ordinary Release and profiling builds for both VM engines.
- [x] Record source/build/host/power provenance, `__text`, owner extent,
      artifact size and build time/RSS. Product lifecycle/RSS is deferred after
      the timing result rejects both reduced-owner controls.
- [x] Prove exact outputs and retain formal absolute timing for the governed
      representative set.
- [x] Capture counts across Tier A plus RexxCPS. Native attribution is deferred
      because the count-complete 30% panel itself proved a layout effect.

### Stage B — framework canary

- [x] Add the internal handler definition, policy and call-ABI surfaces.
- [x] Cover sequential, branch, signal, call/frame, terminal and private forms.
- [x] Build and run focused dual-VM Debug/Release/profiling checks.
- [x] Compare the canary all-inline form to the untouched code shape.

### Stage C — complete handler migration

- [x] Migrate reserved/load/copy/simple numeric handlers.
- [x] Migrate comparison/branch/float/decimal/conversion handlers.
- [x] Migrate string/Unicode/PARSE/binary/stem handlers.
- [x] Migrate attribute/reference/lifetime/private handlers.
- [x] Migrate call/frame/return/signal/metadata/plugin handlers.
- [x] Migrate filesystem/spawn/environment/network/terminal handlers.
- [x] Verify exactly one semantic definition for every public/private handler.

Migration batches received focused build and dual-engine workload checks. The
complete all-inline, all-outline and profile-30 boundaries received broad
Release CTest sweeps; all-outline also received the broad Debug sweep before
the bounded local commit.

### Stage D — all-inline control

- [x] Build every handler directly inside the owner.
- [x] Pass full Release correctness and exact workload output checks; the stronger
      outlined call ABI receives the broad Debug suite.
- [x] Compare preprocessed shape, symbols, owner/text/artifact size, dispatch
      form, build cost/RSS and paired ordinary Release timing to Stage A.
- [x] Repair or stop on an unexplained semantic/all-inline equivalence failure.

### Stage E — all-outlined control

- [x] Force every handler wrapper out of line and verify symbols/disassembly.
- [x] Pass dual-VM correctness and instrumentation balance.
- [x] Measure the call/shape ceiling, owner/text/artifact size, build/RSS and
      representative timing. Product RSS/native attribution is deferred after
      the decisive adverse result.

### Stage F — profile-selected panels

- [x] Produce a replayable ranked handler ledger from the frozen profiles.
- [x] Compute cumulative 5%, 10%, 20% and at-most-30% profile coverage; the
      stronger 30% result stops timing of smaller frequency-only panels.
- [x] Start with one common panel; no VM-specific panel is selected at this
      checkpoint because both engines prove the same layout confound.
- [x] Run the formal balanced 30% comparison against current, all-inline and
      all-outlined controls on both concrete engines.

### Stage G — evidence and stop

- [x] Retain one compact checksum-closed evidence bundle with raw samples.
- [x] Update this worklist, `ROADMAP.md` and `RXVM_INTERPRETER.md`.
- [x] Review and locally commit the bounded result without pushing.
- [x] Report and stop at the 30% checkpoint before production selection or
      cross-platform/default-VM work.
