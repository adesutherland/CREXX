# PERF2-06 candidate and placement panel

## Ranked disposition

| Rank | Candidate | Evidence-selected cost | Exact ceiling | Earliest safe owner | Disposition |
| ---: | --- | --- | --- | --- | --- |
| 0 | Current dual private execution images and recycled combined frames | Reference model; correctness and identity already established | none | VM | retain as fallback/baseline |
| 1 | VM-C1: shared immutable interrupt policy plus frame-local COW or sparse override | Every child bytecode call copies 32 40-byte entries. Permute/List copy 554,176,000/732,800,000 bytes; native List samples place `memmove` at 115/1,545 and 114/1,543 top-of-stack samples. | Remove exactly one 1,280-byte table copy per child bytecode call and 1,264 inline bytes per allocated frame block; no dispatch tax is permitted. | VM frame/signal state | advance only as a redesigned cold-helper slice; reject the measured inline-failure PoC shape |
| 2 | VM-C2: segmented stable value stack plus compact control stack | Current recycling already avoids almost every hot allocation, but reused entry still performs 9,957,712/13,312,071 local-pointer relinks on Permute/List. | At most 9,957,877/13,312,911 local relink stores, plus cursor/allocation bookkeeping; it cannot claim the rest of frame-entry work unless its design removes it. | PERF2-06 for frame/control architecture; PERF2-07 for value storage, clearing, copy and alias representation | advance to bounded architecture PoC if Adrian selects it; do not install directly |
| 3 | VM-C3: sync numeric context only when effective context changes | `syncNumericContext` remains visible in 20/1,545 and 14/1,543 current List samples; inherited-context phase averages about 15 ns per reused call in diagnostic profiles and also contains interrupt inheritance. | One plugin sync per child activation with an active unchanged decimal context; exact call count still needs a dedicated counter. | VM/plugin boundary | defer behind VM-C1 attribution; combine only if independently measured |
| 4 | VM-B1: compact switch stream or hot/cold operand overlay | Current List+library prepares 177,123 eight-byte cells, 1,416,984 bytes in each VM. Preparation median is only 331 us/306 us (`rxvm`/`rxbvm`). | A hypothetical four-byte image can save no more than 708,492 bytes before required pointer-width operands and overlays; no steady-state ceiling is proved. | loader/private preparation and VM | defer pending native counter matrix; no current Apple evidence outranks VM-C |
| 5 | VM-D1: cold interrupt/TRACE outlining | Inactive profiles execute one short-circuit poll per instruction and zero scans. Prior removal changes semantics; prior global `unlikely` layout was severely adverse. | Only the inactive-poll machine sequence; delivery cannot be removed. Historical upper bound was about 1% to 5%, not a valid candidate result. | VM dispatch | defer; retain poll and identity unchanged |
| 6 | VM-F1: lifecycle/preparation cache or reusable image facility | Private prepare is under 4% of measured load+link+prepare and roughly 0.3 ms for a 1.417 MB image. | At most the 306-331 us prepare phase within one context; cross-context reuse adds invalidation and process-pointer ownership. | loader/linker/VM lifecycle | reject as current priority; keep idempotent per-context preparation |

## VM-B - execution stream and fetch layout

The status quo is two equal-sized owned execution images. `rxvm` replaces
instruction cells with handler pointers; `rxbvm` dispatches copied canonical
or process-private opcodes. Both rebind direct function operands privately.
Canonical `segment.binary` remains immutable and authoritative for RXBIN,
reflection and observability.

| Form | Startup/image | Steady state | Identity and late load | Status |
| --- | --- | --- | --- | --- |
| current eight-byte-cell image | 177,123 cells/1,416,984 bytes; prepare 331/306 us | accepted Q3b zero-state/private forms; no state check at a specialized site | exact canonical coordinate conversion; newly loaded modules link and prepare before execution | baseline |
| compact `rxbvm` opcode stream | theoretical maximum saving 708,492 bytes at four bytes/cell, less pointer-width operand storage | may reduce fetch footprint but adds decoding/overlay accesses; no current counter proof | needs canonical-index mapping and private-opcode namespace | deferred PoC |
| hot opcode plus cold operand/metadata overlay | extra build and invalidation work | can keep rare metadata out of fetch, but most handlers still need operands | must retain source/profile/TRACE identity and late-load rebuild | deferred PoC |
| eager/stateful quickening | more preparation and/or first-hit state | did not beat accepted zero-state Q3b | canonical identity possible but state lifecycle unnecessary | negative evidence retained |
| broad RXSEQ superinstructions | larger private and text representations | raw sequence frequency did not prove semantic reuse | high mapping/debug surface | rejected |

Apple `sample` has no hardware branch-miss or I-cache counters. The current
product `run()` sizes are 543,340/536,668 bytes and Mach-O `__text` sizes are
810,700/804,704 bytes (`rxvm`/`rxbvm`). A compact representation does not
advance without Linux counter evidence showing that stream fetch, rather than
handler/helper/frame work, selects it.

## VM-C - calls and frames

### Current activation model

Fresh allocation is already rare on call-heavy cells:

| Workload | Dynamic instructions | Bytecode calls | Fresh frames | Reused frames | Frame-entry work units |
| --- | ---: | ---: | ---: | ---: | ---: |
| Permute optimized | 12,906,826 | 432,950 | 7 | 432,944 | 11,256,730 |
| List optimized | 28,477,526 | 572,500 | 44 | 572,457 | 15,030,414 |
| Base64 optimized | 46,724,369 | 500 | 2 | 499 | 21,045 |

The current combined allocation contains the frame, two pointer maps and
frame-owned local/`a0` values. A returned frame moves to its procedure's free
list. Reuse relinks local/global pointers, resets `a0`, copies the 1,280-byte
interrupt table, copies numeric context, rebinds plugin pointers and syncs an
active decimal plugin. All recycled frames are drained at context teardown;
reuse does not cross `rxvm_call()` contexts.

### VM-C1 COW control

The bounded PoC replaces the inline table with an inherited pointer, allocates
a root table, and makes a private copy on the first frame-local signal-table
mutation. Focused recursion, signals, push/pop, breakpoint, reference,
instrumentation and late-load guards pass 65/65 in both product modes. Exact
deterministic execution counts are unchanged.

The operation gate passes: the hot inherited `memmove` disappears from the
List native samples, and recorded fresh frame-block allocation falls by 1,264
bytes per fresh frame. The implementation gate fails: repeating allocation
failure/dispatch handling at signal opcodes grows `run()` by 10,700 bytes in
`rxvm` and 6,268 bytes in `rxbvm`. Formal timing is favorable for Permute in
both modes and List `rxvm`, inconclusive for List `rxbvm`, clearly adverse for
Base64 and Sieve `rxbvm`. This exact patch must not ship.

A selectable successor must centralize the cold mutation/failure path and keep
the fast inheritance path to pointer/mark assignment. A sparse override is
also valid because handler lookup occurs only after an interrupt is pending;
it must still cover all 31 real codes, the `RXSIGNAL_MAX` sentinel boundary,
push/pop restoration, unwind and caller isolation.

### VM-C2 segmented two-stack architecture

This is the exact version of Adrian's proposed option:

- a value arena grows by fixed or size-classed chunks; an existing chunk is
  never reallocated or moved while any contained value is live;
- a separate compact control stack records procedure/return state, value and
  control marks, argument/result placement, reference-lifetime state, numeric
  and plugin ownership, and the inherited signal-policy pointer/overlay;
- frame entry normally advances cursors; return or signal unwind clears live
  value payloads and reference identities in semantic order, rolls cursors back
  to saved marks, and retains empty chunks for later calls;
- a procedure whose value span does not fit the current chunk starts in a new
  suitably sized chunk, so every value in the span has a stable address;
- no public RXAS, RXBIN, ABI, source/profile/TRACE identity or language change
  is implied.

The hard issue is not chunk growth; it is register alias resolution. Existing
`value **locals` supports globals, arguments, `LINK`/`UNLINK`, references,
call-window swaps and handler unwind by changing pointer-map entries. Merely
moving that map into a control stack keeps nearly all relink work and therefore
has little steady-state ceiling beyond already-rare allocation. Replacing it
with `value_base + register` plus a sparse alias overlay removes most relink
stores, but can tax every operand unless the private execution representation
can select direct versus aliased lookup without losing correctness or code
layout.

The bounded PoC gate for VM-C2 must therefore compare three forms on the same
images: current pointer map; segmented storage retaining the pointer map
(allocation/memory control); and segmented storage with a proved alias overlay
(relink control). It must count cursor operations, relink stores, overlay
lookups/hits, clears and bytes before timing. Required cases are recursion,
arguments/results, globals, linked locals, escaped references and invalidation,
signal unwind and push/pop, native/plugin calls, decimal modes, TRACE and late
load. This candidate spans PERF2-06 architecture selection and PERF2-07 value
representation; it is not merely “PERF2-07 later.”

## VM-D - interrupt, TRACE and cold paths

Optimized canonical profiles report `interrupt_polls == dynamic_instructions`
and zero scans when no signal is pending. The poll is already short-circuited
as `interrupts && !current_frame->is_interrupt`. Release `DEBUG`/TRACE logging
compiles out; authored source metadata does not create an extra runtime poll.
Taken handling scans codes in numeric order and preserves the interrupted or
resume coordinate.

The poll stays. No removal, blanket `unlikely`, or delivery reordering is a
candidate. A later cold outline is allowed only if both VMs retain canonical
instruction identity, source/profile/RXSEQ coordinates, breakpoint persistence,
signal ordering, handler action semantics and resume behavior, and only if
generated-code/counter evidence beats the known layout regressions.

## VM-E - cross-platform completion

| Target | Current exact PERF2-06 evidence | Required reproducible completion | Blocker |
| --- | --- | --- | --- |
| Apple ARM64, Apple clang | complete for current attribution and COW PoC | repeat selected slice with Release timing, text and native samples | none after selection |
| Linux ARM64, supported GCC | unavailable at this commit | same manifests; `perf stat` cycles, instructions, branches/misses, cache and supported L1I/iTLB events | no host in this activity |
| Linux x86-64, GCC | unavailable | same plus `perf record/report` and `run()` disassembly | no host in this activity |
| Linux x86-64, Clang | unavailable | compiler-matched build and same counter matrix | no host in this activity |
| Windows x86-64, workflow-supported MSYS2 GCC | unavailable | profiling-off Release paired matrix, RSS/text and focused semantic suite | no Windows host in this activity; hardware counters tool-dependent |

Historical Linux and earlier-commit results remain orientation only. They do
not select a current default. Final stream/default architecture belongs to
PERF2-11 Gate E.

## VM-F - lifecycle and preparation

The load-only harness uses current profiling-off VM libraries and exact List
plus library inputs (144 modules, 177,123 cells):

| Phase median | `rxvm` | `rxbvm` |
| --- | ---: | ---: |
| load program | 88 us | 88 us |
| load library | 3.674 ms | 3.685 ms |
| link | 5.810 ms | 5.814 ms |
| private prepare | 331 us | 306 us |
| destroy without execution | 27 us | 29 us |

A separate tiny CLI probe measures startup-inclusive load-to-first-result at
2.7595 ms/2.7035 ms. Compile and assemble are separately reported and are not
VM lifecycle. Plugin/first-frame work is included in the CLI boundary but is
not independently isolated enough to select a change. Private preparation is
small beside load/link and is not a current priority. Late load must continue
to dirty link state, rebuild affected bindings and prepare newly executable
modules before entry; stale private operands or private opcodes must fail
preparation rather than fall back silently.

## Compatibility and failure contract common to advancing candidates

- Canonical RXBIN stays immutable and contains no process pointer/state.
- Allocation/growth overflow and OOM fail before publishing a partial frame,
  value segment, policy overlay or execution image.
- Call result placement, writable inputs, pointer aliases and references retain
  exact existing behavior in both VMs.
- Signal unwind performs value/reference/plugin cleanup in semantic order and
  restores caller call-window state before rolling back arena marks.
- TRACE, source, profile and RXSEQ use canonical module/instruction identity.
- Late-loaded modules either prepare and bind coherently or remain
  unexecutable; no mixed-generation state is allowed.
- Current combined frames/private images remain the fallback until a selected
  slice passes its mandatory first ordinary Release verdict and later QA.
