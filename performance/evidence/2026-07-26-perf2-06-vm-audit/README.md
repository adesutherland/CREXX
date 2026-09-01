# PERF2-06 VM audit - first mandatory stop

Status: **selection stop; no production implementation installed**

Baseline: `e7090198e45002a6a73b654f6d98b9eb91d2e5cb`

## Decision

PERF2-06 should select a **call/frame change**, not an execution-stream rewrite
or lifecycle cache, as its first production direction. The exact first slice
should remove eager per-call interrupt-table inheritance with one centralized
cold mutation/failure path. The measured COW patch proves the mechanism but is
rejected as production code because its repeated inline failure path grows
`run()` and clearly regresses `rxbvm` guard cells.

Adrian's segmented two-stack proposal is a first-class VM-C option. Its precise
form is a non-moving chunked value arena plus a separate compact control stack.
It is potentially the broader simplification and performance win, but only if
it also avoids most `value **locals` relinking. A two-stack allocator that
retains the full pointer map merely reproduces the current recycler's main hot
work in a different layout. PERF2-06 owns selection/control-frame architecture;
PERF2-07 owns the value, clear/copy and alias-representation part. It should be
the second bounded architecture PoC after—or explicitly instead of—the narrow
interrupt slice, according to Adrian's selection.

## Exact current-HEAD attribution

### What remains VM-owned

1. **Frame/control inheritance is material.** Optimized Permute executes
   432,950 bytecode calls and List 572,500. Only 7/44 frames are fresh; 432,944/
   572,457 are recycled. Allocation is therefore not the steady-state problem.
   Reused entry still relinks 9,957,712/13,312,071 local pointers, resets `a0`,
   copies numeric/plugin state and copies a 1,280-byte interrupt table on every
   child call. That table traffic is 554,176,000 bytes for Permute and
   732,800,000 bytes for List.
2. **The table copy is visible in product binaries.** A two-second List sample
   attributes 115/1,545 `rxvm` and 114/1,543 `rxbvm` top-of-stack samples
   (7.4% in each) to `memmove`. The narrow COW PoC removes `memmove` from the
   collapsed entries at the five-sample reporting threshold.
3. **Value/reference helpers remain larger than pure dispatch on List, but are
   mainly PERF2-07-owned.** Current samples show `copy_value` at 259/1,545 and
   279/1,543 samples, plus trimming/reference-lifetime helpers. Those operations
   must not be relabelled as a dispatcher problem.
4. **Inactive interrupt polling is exact but not selectable for removal.** The
   canonical profiles execute one `interrupts && !is_interrupt` poll per
   dynamic instruction and zero scans when inactive: 12,906,826 polls on
   Permute and 28,477,526 on List. Removing it changes delivery semantics; the
   prior `unlikely` layout experiment remains negative.
5. **Private-image preparation is bounded and small.** Exact List+library load
   owns 144 modules and 177,123 eight-byte cells, or 1,416,984 private bytes in
   each VM. Preparation is 331 us/306 us median, below 4% of measured
   load+link+prepare. It does not outrank frame work.
6. **Both VMs have the same ownership model.** Each executes an owned
   `execution_image`. `rxvm` binds handler pointers; `rxbvm` uses copied
   canonical/private opcodes. Direct function operands are privately rebound.
   Canonical `segment.binary` is immutable and supplies RXBIN, reflection,
   source/profile/TRACE and debug identity.
7. **Dispatch, handler and helper separation is diagnostic, not a product-time
   claim.** Timing profiles put the common transition path near 12 ns per
   same-frame instruction and call/return transitions near 10 ns, but clock
   instrumentation perturbs those values. Product sampling leaves about
   63%-64% at `run()` on List and the rest in named value/reference/context
   helpers. Apple `sample` provides no branch-miss or I-cache counters, so no
   stream-layout claim is made from unavailable hardware events.

Current deterministic mechanism counts for both VMs are retained in
`results/current-mechanism-summary.csv`; diagnostic phase timing is in
`results/current-timing-profile-summary.csv`.

### Lifecycle boundary

For exact optimized List plus library, the profiling-off load-only harness
reports these 31-run medians:

| Phase | `rxvm` | `rxbvm` |
| --- | ---: | ---: |
| load program | 88 us | 88 us |
| load library | 3.674 ms | 3.685 ms |
| link | 5.810 ms | 5.814 ms |
| prepare private image | 331 us | 306 us |
| destroy without execution | 27 us | 29 us |

The separate tiny CLI boundary is 2.7595 ms/2.7035 ms from load through first
result. It includes process and first-frame/plugin effects and is not used to
claim an isolated first-frame cost. Compile/assemble are recorded separately.

## Bounded COW PoC

The PoC patch is checksum-identified in `PROVENANCE.md`. It preserves canonical
RXBIN and both public VM modes, shares a parent's table on child entry, allocates
a private copy at the first child mutation, owns a root table and releases
owned tables during full cleanup.

### Correctness and machine-work gate

- Current and COW focused dual-VM suites pass 65/65.
- Optimized Permute, List and Base64 retain exact deterministic dynamic
  instruction, call, frame-work, value-operation and branch counts.
- Fresh frame blocks shrink by 1,264 bytes each. Recorded frame-block allocation
  falls by 8,848 bytes on Permute, 55,616 on List and 2,528 on Base64. The PoC's
  separately allocated root table is not included in those frame-block counters;
  RSS is reported separately.
- The 1,280-byte inherited copy disappears on child entry. No workload in this
  panel mutates its inherited table during the measured hot loop.

### Profiling-off Release timing

Thirty-six position-balanced pairs, canonical work counts, same exact RXBIN
and library; negative percentages favor COW:

| Workload | VM | Current median | COW median | Paired median | Mean 95% interval | Favorable | Result |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- |
| Permute 50 | `rxvm` | 87.993 ms | 83.807 ms | -4.549% | [-4.878%, -3.465%] | 35/36 | clear favorable |
| Permute 50 | `rxbvm` | 94.330 ms | 90.629 ms | -3.970% | [-6.516%, -2.770%] | 34/36 | clear favorable |
| List 100 | `rxvm` | 85.248 ms | 78.825 ms | -7.427% | [-8.010%, -6.097%] | 35/36 | clear favorable |
| List 100 | `rxbvm` | 92.743 ms | 92.423 ms | -0.639% | [-2.973%, +1.515%] | 23/36 | inconclusive |
| Base64 500 | `rxvm` | 347.480 ms | 328.423 ms | -6.762% | [-5.420%, +3.026%] | 21/36 | inconclusive |
| Base64 500 | `rxbvm` | 347.881 ms | 379.887 ms | +10.009% | [+3.507%, +12.964%] | 12/36 | clear adverse |
| Sieve 50 | `rxvm` | 24.282 ms | 24.271 ms | -0.089% | [-0.442%, +0.367%] | 19/36 | neutral/inconclusive |
| Sieve 50 | `rxbvm` | 25.940 ms | 26.889 ms | +4.003% | [+3.171%, +6.126%] | 0/36 | clear adverse |

The raw samples and the Level B paired summarizer are retained. Five-run peak
RSS medians are descriptively 32-98 KiB lower on Permute/List, mixed within
16-64 KiB on the two guards, and too page-granular to select the change.
Native-sample footprint is 12.3/12.2 MiB versus 12.4/12.3 MiB current.

### Text and rejection reason

| Binary | Current file | COW file | Current `__text` | COW `__text` | Current `run()` | COW `run()` |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 998,776 | 998,840 | 810,700 | 821,520 | 543,340 | 554,040 |
| `rxbvm` | 982,440 | 999,016 | 804,704 | 811,092 | 536,668 | 542,936 |

The repeated allocation-failure `SET_SIGNAL`/`DISPATCH` branch at each signal
mutation site expands hot `run()` code by 10,700/6,268 bytes. That explains a
credible code-layout mechanism for the cross-VM guard regressions. The exact
PoC is rejected; the ownership conclusion is retained.

## Ranked panels

`CANDIDATE-PANEL.md` contains the complete VM-B through VM-F panel, including
machine ceilings, earliest owners, invariants, failure behavior, lifecycle,
late load, both modes, identity and cross-platform blockers. In short:

1. VM-C1 narrow interrupt-policy inheritance: strongest measured mechanism;
   redesign the cold path before any production edit.
2. VM-C2 segmented non-moving values plus compact control frames: strongest
   broader architecture option; prove alias-overlay economics before timing.
3. VM-C3 changed-only numeric/plugin synchronization: real but not yet exactly
   counted; defer behind C1.
4. VM-B compact/hot-cold stream: memory ceiling exists, steady-state owner not
   proved; defer to the native counter matrix.
5. VM-D interrupt/cold outlining: semantic and layout risk exceeds current
   evidence; retain poll.
6. VM-F lifecycle caching: current preparation is too small to prioritize.

## Recommended independently measurable ladder

1. **VM-C1b, selected production slice:** replace the inline inherited table
   with a shared policy/table plus one centralized cold mutation/OOM path. A
   sparse override is acceptable if taken-handler lookup remains cold. Require
   exact 1,280-byte-copy removal, no new per-dispatch work, and `run()` growth
   no larger than 1 KiB in either VM.
2. **Mandatory first Release verdict:** freeze after focused signal, recursion,
   reference, breakpoint/TRACE and late-load correctness. Compare against the
   exact current product above on optimized List work 100 in both VMs as the
   primary decisive cell, with Permute work 50 confirming call recursion and
   Sieve work 50 guarding unrelated layout. Start at 12 balanced pairs and
   extend only under governance to 36. Accept only if both target VM modes are
   clearly favorable or one is clearly favorable and the other statistically
   neutral, no guard hits the 3% adverse threshold, and text/RSS remain within
   their declared bounds. Report and stop again before broad QA.
3. **VM-C2 architecture PoC:** compare current pointer maps, segmented values
   retaining maps, and segmented values with a proved alias overlay. Use exact
   relink/overlay/clear/cursor counters and recursion/reference/signal/plugin
   semantics before timing. If selected instead of VM-C1b, this becomes the
   next isolated PoC—not a direct production rewrite.
4. **VM-C3:** add an exact effective-context-change counter, then try lazy
   decimal sync only if the count/ceiling remains material after VM-C1.
5. **VM-B/VM-E:** run compact-stream controls and hardware counters on Linux
   ARM64, Linux x86-64 GCC/Clang and workflow-supported Windows x86-64. Final
   private/default dispatch selection remains PERF2-11 Gate E.
6. **VM-F:** reconsider lifecycle only if later larger-module evidence makes
   preparation material.

## Documentation reconciliation

`docs/ai-context/RXVM_INTERPRETER.md` now states the exact current dual-owned
execution-image model. The historical dispatch investigation retains its dated
measurements but adds a 2026-07-26 reconciliation so its old “canonical
`rxbvm`” wording is not read as current behavior.

## Approval stop

No production source has been changed, no broad closeout has run, and nothing
has been committed or pushed. Adrian must now choose one of:

- VM-C1b as the precise first production slice;
- VM-C2 segmented value/control stacks as the next bounded architecture PoC;
- another panel candidate; or
- no production change.
