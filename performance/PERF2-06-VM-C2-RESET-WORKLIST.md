# PERF2-06 VM-C2 reset countermeasure PoC worklist

Status: complete; R1 and R2 rejected at the mandatory PoC stop

Started: 2026-07-27

Purpose: reject the C2-B write-side dirty ledger and test whether fixed-core
reset specialization removes current frame-reuse relink work without adding
bookkeeping to mapping writes or ordinary operand reads. This is an isolated
architecture PoC only. It does not authorize production integration, public
RXAS/RXBIN/ABI or language change, broad QA, implementation commit, or push.
Adrian later authorized a results-only closeout commit after rejecting R1/R2.

## Exact clean base

- Branch: `codex/perf2-06-vm-c2-reset-poc`.
- Base commit: `ab9eef54576388121cdb02e285bd99981e68d8b3`.
- Base subject: `evidence: retain PERF2-06 sieve outputs`.
- Accepted VM-C1b implementation commit:
  `a39608426e2c1bb84d5fc0c4f767f4c9492339a9`.
- `origin/develop`: `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`.
- Base relation to `origin/develop`: `+9/-0`.
- Base and new isolated worktree were clean before this worklist.
- The rejected C2-A/C2-B implementation and evidence remain untouched in the
  earlier discardable worktree.

## Selected question and falsifiable hypothesis

Current recycled frames reset the fixed mapping prefix for procedure locals and
module globals, then restore and reinitialize `a0` separately. Incoming
`a1...aN` mappings are a variable tail and are rebound by every call; they do
not need a general reuse reset. The current
implementation already makes one allocation for a fresh frame and amortizes it
through procedure-owned recycling, so this PoC isolates mapping reset rather
than allocation.

Hypothesis: one fixed-core clear operation, optionally skipped by a conservative
preparation-time `may_rebind_core` flag, can reduce frame-entry instructions and
text without any mapping-write/read tax. If Release time is neutral/adverse or
the static flag cannot conservatively cover canonical, fused, signal and
late-load paths, reject the countermeasure.

## Numbered plan

1. Freeze the exact accepted frame as R0 and retain the first VM-C2 result as
   the authority rejecting write-side dirty tracking.
2. Implement R1: reset the contiguous fixed mapping core in one clear/copy
   operation; leave the variable argument tail to call binding.
3. Implement R2: add a procedure-static conservative `may_rebind_core` flag
   computed during module/execution preparation. Skip R1 only when no canonical
   instruction can mutate a fixed-core binding. Unknown cases use R1.
4. Count reused activations, core reset calls/slots/bytes, flag-skipped
   activations, flagged procedures and preparation work. Add no per-write or
   per-read countermeasure code.
5. Pass focused dual-VM recursion, fixed/count-call arguments, globals,
   `LOAD`/`SWAP`/`LINK*`/`UNLINK*`, references, signal unwind, TRACE/debug and
   late-load correctness before timing.
6. Compare exact profiling-off Release R0/R1/R2 products on optimized Permute
   50 and List 100 as call-heavy cells, with Sieve 50 and Base64 500 guarding
   unrelated code layout. Record `run()`/text size.
7. Report one result and stop before production integration or a new
   quickening implementation.

## Design comparison

### R0 - accepted dense loops

Retain the current separate local/global loops plus `a0` pointer/value reset.
This is the compatibility and code-layout baseline. It performs no work on
mapping writes or ordinary operand reads.

### R1 - fixed-core clear reset

Treat `[locals, globals]` as one contiguous pointer-mapping prefix and restore
it from the equally contiguous base prefix in one operation. Restore `a0` from
its base pointer separately, then zero and set its value exactly as today. The
variable `a1...aN` tail is excluded:
counted and fixed calls already overwrite every supplied argument binding.

R1 changes no allocation, value ownership, alias semantics, reference
lifetime, signal unwind, public format, or operand lookup. It tests only
whether concentrated reset code is cheaper/smaller than the current loops.

### R2 - static reset-needed flag

R2 retains R1 as the conservative fallback. A procedure-owned immutable flag
states whether any instruction in the executable procedure can change a
fixed-core mapping. Alias-free procedures skip the pointer reset; `a0` still
receives the current argument count.

The flag is prepared once from canonical/runtime instruction facts. It is not
set on mapping writes. At minimum the classification must cover `LOAD` binding
forms, all `SWAP`/call-window forms, `LINK`, `LINKARG`, `LINKATTR*`, `LINKREF`,
`METALINKPREG`, `UNLINK*`, and private fused equivalents. Any unknown,
unclassified or range-ambiguous instruction selects R1.

### Deferred - exact static reset list

A sorted per-procedure destination list can reset only `K` possible slots with
zero write-side tracking. It is not part of the first simple comparison. It
advances only if R2 shows useful static separation but the coarse flag leaves
material dense reset work.

### Deferred - quickened link clearing

Private preparation can eliminate exact compiler-generated
`LINK/LINKATTR; one nonescaping use; UNLINK` mappings when debug, signal,
reference, alias and resume-state semantics are preserved. It cannot store
activation pointers or mapping state in the shared execution image. Existing
private R1/R2 forms remain conservative inputs to R2; no new private handler is
added in this reset PoC.

## Fixed calls and separated arguments

`CALL1` through `CALL4` already capture named caller pointers without a
contiguous caller call-window permutation. In a later two-stack design they can
populate argument pointers embedded directly in the compact control record;
only higher-arity/dynamic/native counted calls need a variable argument vector.
This PoC records that boundary but keeps current callee operand access so the
reset mechanism is measured independently.

## Post-verdict consideration - value payload affinity

The current procedure-owned recycler also gives a useful value-level locality:
if one register grows a large string, binary, object or attribute payload, the
same procedure/register slot can retain reusable internal capacity on its next
activation. A future generic value stack must not silently discard that
benefit.

After the R0/R1/R2 verdict, assess whether payload storage can be recycled or
transferred independently of the compact control/mapping record. Compare at
least procedure/register affinity, shape-and-capacity pools and high-water
frame retention. The review must preserve reference identity/invalidation,
object attributes, native payload destructors, string/binary ownership,
argument aliasing and thread-local ownership. It is analysis only and is not a
fourth reset variant.

## Machine gates

- Zero new work on mapping writes and ordinary operand reads.
- R1 performs exactly one non-empty fixed-core mapping reset per reused
  activation; zero-length cores avoid a `memcpy(0)` call.
- R2 skips the fixed-core reset only for conservatively proved procedures;
  `a0` and supplied arguments remain exact.
- Unknown opcode/effect state selects the R1 fallback.
- Deterministic instruction, call, branch and value-operation counts match R0
  on the exact diagnostic inputs.
- Preparation work, reset slots/bytes, skipped activations and text size are
  reported before timing interpretation.

## Final PoC result - reject R1 and R2

The retained evidence is under
[`evidence/2026-07-27-perf2-06-vm-c2-reset-poc/`](evidence/2026-07-27-perf2-06-vm-c2-reset-poc/).
Both implementations are mechanically correct, but neither survives the
profiling-off Release verdict.

- R1 performs one fixed-core copy on every reused activation. At the formal
  work counts it resets 9,957,712 Permute slots, 13,312,071 List slots and
  19,461 Base64 slots: 79,661,696, 106,496,568 and 155,688 bytes respectively.
  Sieve has no reused activation and zero reset bytes.
- R2 proves 161-164 loaded procedures clean and fails closed for 36 unknown
  procedures. It skips all 499 Base64 reuse resets, but no Permute reset and
  only 3,099 of 572,457 List activations: 0.541% of activations and 0.070% of
  eligible slots. It still copies 79,661,696 Permute bytes and 106,422,192
  List bytes.
- R0, R1 and R2 have exact instruction, call, branch and value-operation rows
  for optimized Permute 50, List 100 and Sieve 50 in both VMs.
- R1 and R2 each pass 78 frame/reference/argument, 57
  signal/TRACE/instrumentation and 8 re-entry/late-load tests: 143/143 per
  variant in diagnostics-on, profiling-off Debug products.
- Thirty-four retained balanced pairs reach the programme cap. R1 is clear
  adverse on List in both VMs, Base64 `rxvm` and Sieve `rxbvm`. R2 is clear
  adverse on Permute `rxbvm`, List in both VMs and Sieve `rxbvm`. Base64
  remains layout/startup-sensitive and noisy, with large opposite-direction
  swings between VMs. The absolute-cell gate also flags Sieve R0 `rxvm`
  (24.241% span) and R1 `rxbvm` (10.009% span).
- The decisive control is Sieve: it has zero reused activations, yet R1
  `rxbvm` is `+1.801135%` mean elapsed and R2 `rxbvm` is `+2.284914%`.
  R1 has no classifier, so its regression isolates generated native-code
  layout/register-allocation debt. R2 scans 55,104 instructions at startup,
  so its regression can combine classifier cost and layout; neither can be
  reset work. The absolute-cell gate flags the R1 `rxbvm` Sieve span, but the
  34-pair mean interval remains wholly adverse. This strengthens the existing
  Intel x86-64 GCC/Clang cross-platform requirement in `PERF2-06-D01`.
- R1 shrinks `run()` and Mach-O `__text` by about 3.3 KiB in each VM. R2 keeps
  a roughly 3.2 KiB `run()` reduction, but its preparation classifier leaves
  only a 1.2 KiB total `__text` reduction and grows each executable by about
  16.5 KiB. Smaller text did not produce a stable portfolio win.

R1 and R2 are rejected. Do not install either implementation and do not add an
exact reset list or quickened clearing form from this result. The isolated
branch remains dirty and discardable so the accepted base can be recovered
without a revert.

## Post-verdict payload-affinity conclusion

The current procedure-owned frame recycler does preserve useful register
affinity: a large string, binary, decimal or object capacity retained by a
logical register can be reused by the same procedure/register on a later call
within the same `run()`. A strict universal LIFO value stack would lose that
property whenever another procedure reused the same physical cells.

The preferred future two-stack shape is therefore:

1. a genuinely linear, segmented compact control stack;
2. a separate non-moving value-slab arena whose free lists are procedure
   affine;
3. one slab per activation containing only procedure locals plus `a0`;
4. a control record or compact sidecar pointing to its slab and owning the
   mutable local/global/argument mapping plus `LOAD`/`SWAP`/`LINK*`/`UNLINK*`
   restoration state, with `CALL1...CALL4` argument pointers embedded directly
   and a separate vector only for variable/high arity calls; and
5. existing return-time signal cleanup and the
   `has_reference_lifetimes`-gated locals/`a0` scan, followed by returning the
   unchanged slab to the same procedure, with full value clearing and native
   finalization once at overwrite or run teardown.

This recycles the register innards without an unconditional per-return value
scan, per-register transfer or hot mapping ledger. It does not eliminate the
flag-gated reference scan or the mutable pointer mapping. It must remain
context/thread local. Reference identity, native payload ownership and live
object-link state are semantic state, not recyclable capacity. A returned
value whose buffer is moved to the caller also cannot retain the same buffer
in the callee slab without copying or shared/COW ownership.

If procedure-affine slabs cause unacceptable high-water retention, test
capacity capsules only as a later rung, starting with ordinary strings and
non-native binaries. Objects, decimals, references and native payloads should
not be part of the first capsule PoC. The control/mapping split remains a
PERF2-06 architecture question; payload-capacity capsules and representation
policy route to PERF2-07.

## Correctness gates

- [x] Local/global/a0 mapping after repeated and recursive reuse.
- [x] `CALL1...CALL4`, counted/dynamic/native calls and zero/high arity.
- [x] Writable/by-reference/optional/repeated arguments and results.
- [x] `LOAD`, all swap forms, `LINK*`, `UNLINK*`, attributes and invalid refs.
- [x] Normal return plus signal call/branch/unwind mapping restoration.
- [x] TRACE/debug/breakpoint and private R1/R2 fallback identity.
- [x] External entry, repeated API re-entry and late-loaded modules.
- [x] Both `rxvm` and `rxbvm`.

## Mandatory PoC stop

R0/R1/R2 machine work, focused correctness, profiling-off Release time and code
size are retained and reported. The stop is active: no production selection or
installation, broad QA, implementation commit, push, exact reset list or new
quickened link form follows from this rejected PoC. Only the result, evidence
and live-status closeout are retained.
