# PERF3-13 M0 RXVM allocator and ownership audit

Date: 2026-08-05

Status: **Gate A audit complete; selected design approved for Gate B on
2026-08-05; no Gate A runtime implementation**

## Result first

Select a **hybrid worker heap** for the first unchanged-`value` implementation:

- one worker-owned heap per RXVM execution context;
- a synchronized central depot that exchanges whole slabs only on refill,
  excess return and teardown;
- 64 KiB aligned standard slabs, one allocation class per slab;
- typed silos for 240-byte `value` objects, power-of-two-count `value` arrays
  and 80-byte reference cells;
- power-of-two capacity classes for variable string, binary and ordinary byte
  storage, with decimal storage retained on the native-plugin family;
- separately tracked oversized extents; and
- explicit foreign allocator families for OS, library, public-ABI and dynamic
  plugin storage that RXVM is not entitled to free through its heap.

Do not use a central allocator thread on the normal path. Do not use a universal
power-of-two object allocator as the product design. Do not change the current
240-byte `value`, frame semantics, RXAS/RXBIN, HTTP or channel surface in the
allocator implementation.

The current procedure-affine frame recycler is retained. The evidence says
fresh frames are cold, whereas values, attribute storage and side buffers
provide the material allocator traffic.

## Provenance and measurement boundary

- Gate A worktree:
  `/Users/adrian/CLionProjects/CREXX-rxvm-worker-memory-gatea`.
- Branch: `codex/rxvm-worker-memory-gatea`.
- Accepted base: `4813e98d1dca1ac77d5899dd6c5787e4b83f4772`.
- The main `/Users/adrian/CLionProjects/CREXX` copies of the PERF3-13 planning
  edits were removed and those two paths were verified clean before this audit
  continued.
- No timed program, benchmark or performance build was run for Gate A. The
  shared host therefore did not need to be reserved.

The retained counts-only M0 package remains in the preserved worktree at:

`/Users/adrian/CLionProjects/CREXX-rxvm-worker-memory/performance/evidence/2026-08-04-perf3-13-m0-string-distribution`

Its committed source base is `965b461d813f6042063ee786d8d00cea870da096`.
The committed production `interpreter/` difference from that base to the Gate A
base is empty; the only `interpreter/` change is
`interpreter/tests/test_rxvmstem.c`. The retained package used uncommitted,
profiling-only probes and is not profiling-off timing evidence, but its
allocation and distribution counts observe the same production RXVM source
now under audit.

A header-only layout check using the local compiler, without running RXVM,
confirmed the Gate A base sizes:

| Type | Size |
| --- | ---: |
| `value` | 240 bytes, alignment 8 |
| `rxvm_reference_cell` | 80 bytes |
| `rxvm_reference_context` | 544 bytes |
| `stack_frame` fixed header | 168 bytes |
| `proc_runtime` | 56 bytes |
| `module` | 216 bytes |
| `rxvm_context` | 736 bytes |
| `interrupt_entry` | 40 bytes |
| `interrupt_saved_entry` | 56 bytes |

The accepted PERF3-12B scorecard deliberately contains no lifecycle/RSS
campaign, and its sample rows have no `max_rss_bytes`. Gate A therefore does
not invent whole-VM live, retained, fragmentation or RSS values. Those are
mandatory allocator telemetry and first-verdict outputs under Gate B.

## Direct allocator surface

The source census covers production RXVM, its built-in services, the embedding
library and bundled VM plugins. Counts are lexical allocation/growth and
`free()` expressions: they are not dynamic event counts, and platform branches
can make multiple source expressions mutually exclusive.

| Domain | Allocate/grow expressions | `free()` expressions | Callback `free` among them |
| --- | ---: | ---: | ---: |
| Core values, frames, modules and references | 89 | 179 | 2 |
| Core spawn, ADDRESS-command, exit and RXPA services | 53 | 149 | 0 |
| Socket/TLS platform layer | 9 | 44 | 0 |
| `rxvml` embedding API and VM CLI | 14 | 22 | 0 |
| Plugin framework | 8 | 22 | 1 |
| Bundled `mc_decimal` and decNumber | 36 | 48 | 0 |
| Optional bundled `db_decimal` | 4 | 2 | 0 |
| **Gate B product boundary** | **213** | **466** | **3** |
| Profiling-only support | 14 | 30 | 0 |
| SAA/tool frontends outside the first conversion | 25 | 119 | 0 |

The first Gate B completion claim is therefore not “the six existing profile
categories use slabs”. It is: every allocator-eligible expression in the
213-site product boundary routes through the worker/depot service, every direct
free is either routed or present in the exception ledger, and a mechanical
source scan enforces that boundary. Profiling support and SAA/tool frontends can
follow without blocking the first RXVM product verdict, but must not be
silently described as converted.

### Ownership families

| Storage family | Present lifetime/owner | Gate B disposition |
| --- | --- | --- |
| Standalone values, globals and API values | VM context or caller-managed API lifetime | Typed single-value silo; owner recovered from slab for context-free `rxvml_value_free()` |
| Attribute values | Stable blocks retained by the parent value across growth | Typed arrays keyed by 1/2/4/8/... values; keep addresses stable |
| Attribute pointer arrays | Parent value capacity, replaced on growth | Worker byte classes; allocate-copy-return for growth |
| String and binary buffers | Sticky reusable capacity on a value | Worker power-of-two byte classes; preserve reset/reuse and explicit trim |
| Decimal buffers | Native decimal-plugin allocation family attached to a value | Keep libc/plugin-owned in Gate B; a cross-DSO allocator-service ABI is required before migration |
| Composite stack frames | Procedure-affine free list until context teardown | Retain recycler; allocate backing block from worker large/general path |
| Interrupt tables and saved handlers | Frame/root-run lifetime | Worker general/typed storage; preserve failure and unwind rules |
| Reference cells | Reference-context active set or bounded free list | Typed 80-byte silo attached to the owning worker/reference context |
| Module/runtime/graph/interface metadata | Context lifetime, some replaced on late load | Worker general storage; no cross-worker mutation |
| Parse snapshots and command scratch | Instruction/request lifetime | Worker byte class or later task scratch; never central-thread RPC |
| Socket/spawn/ADDRESS state | Context/request lifetime plus foreign OS handles | RXVM-owned parts use worker storage; foreign subobjects keep native release API |
| Bundled decimal/plugin state | Plugin, frame or value lifetime | Keep plugin-private and decimal payload storage foreign in Gate B; migrate only through an approved versioned allocator-service ABI |
| Profiler tables and traces | Profiling-run lifetime | Separate follow-on conversion; not part of profiling-off verdict |

The current API exposes `value` directly and `rxvml_value_free()` has no context
argument. A worker pointer must therefore not be added to every `value` merely
to make free possible. Standard-slab address recovery and an oversized header
provide ownership without restoring eight bytes to every value.

## Retained traffic evidence

The table below is the representative `rxvm` side of the retained M0 panel.
The deterministic workloads have matching allocation rows under `rxbvm`; the
small self-calibrating RexxCPS difference did not affect the design decision.
Bytes are cumulative requests, not peak resident memory.

| Workload | Fresh/reused frames | Value slots | Attribute value blocks/bytes | Attribute pointer requests/bytes | String requests/bytes | Binary requests/bytes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Base64 | 2 / 2,499 | 128 | 1 / 1,920 | 3 / 192 | 14 / 16,832 | 2,507 / 2,568,256 |
| JSON | 18 / 769,983 | 1,596 | 11 / 21,120 | 32 / 2,176 | 6 / 4,224 | 10,008 / 323,232 |
| RexxCPS | 20 / 425,022 | 1,650,662 | 16 / 42,240 | 43 / 4,160 | 420,022 / 13,441,920 | 2 / 4,096 |
| Richards | 9 / 2,383,132 | 1,293 | 111 / 253,440 | 333 / 24,000 | 3 / 256 | 0 / 0 |
| Towers | 14 / 1,639,487 | 96,569,328 | 12,071,103 / 23,176,517,760 | 36,213,309 / 2,317,651,776 | 2 / 128 | 0 / 0 |

Three conclusions follow directly:

1. Frame activation is hot but fresh frame allocation is not. Replacing the
   frame recycler or separating its one composite block would confound the
   allocator programme with a previously rejected frame-layout family.
2. Towers is dominated by eight-value attribute blocks: 1,920 bytes at the
   current layout. A generic 2,048-byte class would add 128 bytes per request,
   or 1,545,101,184 cumulative requested bytes in this one cell.
3. RexxCPS makes 420,022 string-buffer requests totalling 13,441,920 bytes
   while the retained M0 peak was only 10 external buffers/736 bytes. That is
   churn suitable for a local byte silo, not evidence for permanently retaining
   every exceptional capacity.

The retained external-string peaks were 12 buffers/16,640 bytes for Base64,
5/4,160 for JSON, 10/736 for RexxCPS, 1/128 for Richards and 1/64 for Towers.
Whole-allocator current/peak/live/committed/reserve/oversized values remain an
explicit evidence gap, not a zero.

## Why the hybrid beats a universal power-of-two allocator

| Object | Exact size | Universal class | Internal increase |
| --- | ---: | ---: | ---: |
| One `value` | 240 | 256 | 16 bytes / 6.67% |
| Eight-value attribute block | 1,920 | 2,048 | 128 bytes / 6.67% |
| Reference cell | 80 | 128 | 48 bytes / 60% |
| `proc_runtime` | 56 | 64 | 8 bytes / 14.29% |
| `module` | 216 | 256 | 40 bytes / 18.52% |
| Saved interrupt entry | 56 | 64 | 8 bytes / 14.29% |

Pure powers of two are attractive for byte buffers because the value already
records capacity and same-class growth can be a no-op. They are not a good
universal object policy. The hybrid keeps the same constant-time slab/class
lookup while avoiding the worst measured fixed-object waste.

General-purpose allocators reinforce the boundary rather than supplying a
design to copy wholesale:

- [mimalloc](https://microsoft.github.io/mimalloc/index.html) uses one size
  class per page, normally 64 KiB, and separates local from concurrent frees;
- [rpmalloc](https://github.com/mjansson/rpmalloc) uses thread-owned heaps,
  aligned spans and an atomic per-page path for cross-thread frees;
- [jemalloc](https://jemalloc.net/jemalloc.3.html) documents both the benefit of
  thread caches and the retained-memory/fragmentation cost of excessive arenas
  and coarse classes.

RXVM therefore starts smaller: one worker, no remote frees, a bounded depot and
only the classes its current representation can justify. Remote-free queues are
designed into ownership metadata but remain closed until the worker gate.

## Selected substrate contract

### Standard slabs and local classes

- Standard slab size: 64 KiB, aligned to 64 KiB on every supported platform.
- One slab contains one class and one owner-generation.
- A header at the aligned base records magic/version, depot, owner worker and
  generation, class ID, slot size/count, local bump position, free-list state,
  live count and telemetry counters.
- Ordinary allocation/free on the owner worker uses no lock and no atomic.
- Standard pointer ownership is recovered by masking to the aligned slab base
  and validating the header.
- The initial generic byte classes are 16, 32, 64, 128, ... 16 KiB. The current
  32-byte minimum string-capacity policy remains unchanged during M1; lowering
  it belongs to the later value-shape panel.
- Typed classes provide exactly one `value`, arrays of 2/4/8/... values and one
  80-byte reference cell. Add another typed class only when source/evidence
  shows it materially avoids waste or lifecycle complexity.
- An allocation above 16 KiB, or a type that cannot usefully share a standard
  slab, uses a tracked oversized extent with an owner/size/alignment prefix.
  The initial threshold is telemetry-adjustable without changing ownership
  semantics.

A 64-byte provisional slab header would leave 65,472 payload bytes. That fits
272 exact 240-byte values versus 255 generic 256-byte slots, and 818 exact
80-byte reference cells versus 511 generic 128-byte slots. These are design
capacity calculations, not measured product results.

### Depot

- The depot obtains/releases only standard slabs and oversized extents from the
  platform allocation boundary.
- It is a synchronized data structure, not a required thread.
- Worker refill and excess-empty-slab return may take its lock; ordinary local
  allocation/free may not.
- Reserve is bounded per class and globally. Excess empty slabs are released;
  an explicit trim operation can release reserve at a safe point.
- A later background scavenger may call the same trim operation, but no
  allocator correctness or progress rule depends on it.

### Allocation, growth and failure

- Prefer explicit worker/context parameters at constructors and lifecycle
  boundaries.
- For buffers attached to managed values, recover the allocator from the value
  slab or existing buffer slab. Stack-local `value` objects used by RXVM
  services may use the currently entered worker as a bounded fallback.
- The current worker binding is thread-local, scoped to an RXVM/API entry and
  restored on exit. It is not a mutable process-global allocator.
- `realloc` becomes: retain in place when the class still fits; otherwise
  allocate from the owning worker, copy the used bytes and return the old slot.
- Preserve failure atomicity, especially `try_set_num_attributes()`. Existing
  signal, error-return and panic policies remain distinct; the allocator does
  not silently turn one into another.
- Reset keeps ordinary reusable capacity. Destroy returns it. Oversized buffers
  have an explicit trim/release rule so a one-off input is not sticky forever.

### Future remote free and worker teardown

M1 has exactly one worker and performs no cross-thread operation. The slab
header nevertheless reserves enough identity to distinguish owner worker plus
generation. Later, a non-owner free must either:

1. enqueue the slot on a bounded remote-free list owned by the slab/worker; or
2. transfer an otherwise empty whole slab through the depot.

It must never put the slot directly on the receiving worker's local list.
Worker teardown drains remote frees, destroys owned VM values, returns all
slabs, releases oversized extents and detects any still-live allocation. A
worker ID without a generation is insufficient because IDs can be reused.

## Exact foreign and exception ledger

These are allocator families, not excuses for unclassified direct calls.

| Family | Required release | Disposition |
| --- | --- | --- |
| `getaddrinfo()` results | `freeaddrinfo()` | Always foreign/OS-owned |
| OpenSSL `SSL`/`SSL_CTX` and related objects | Matching OpenSSL release API | Always foreign-library-owned |
| Apple Network/Security/CoreFoundation objects | Framework cancellation/release API | Always foreign-library-owned |
| Windows SChannel, certificate, process, DLL and socket handles | Matching Win32 release API | Always foreign/OS-owned |
| `dlopen()`/`LoadLibrary()` handles | `dlclose()`/`FreeLibrary()` | Always loader-owned |
| RXBIN, signature, AVL and RXPA objects returned by dependency APIs | `free_module()`, `rx_sig_free()`, `free_tree()` or owning API | Foreign until those libraries accept an allocator service; outside RXVM-only edits |
| `exepath()` returned buffer | Current platform allocator contract | Foreign until the platform API supplies allocator/release pairing |
| Dynamically loaded plugin instances and payload resources | Plugin `free`/native finalizer | Foreign; never guessed from pointer shape |
| Bundled decimal payloads, temporaries and plugin instances | Current plugin ABI uses libc and can be loaded across a DSO/DLL boundary | Foreign for Gate B; direct RXVM allocator calls would duplicate TLS/allocator state or require a new shared/runtime ABI |
| `rxvml_discover_classes()` returned array | Currently caller uses libc-compatible allocation and no paired RXVML release exists | Keep libc-owned until an approved paired-release API/ABI change |

Gate B implementation proved that the proposed direct bundled-plugin conversion
was not ABI-neutral. A dynamic plugin cannot call a statically embedded RXVM
allocator without unresolved/exported-symbol rules on some hosts, duplicated
allocator/TLS state on others, or a shared/versioned host-services interface.
The direct-call candidate failed the macOS dynamic-plugin link proof and was
removed. Decimal payloads, plugin-private objects and plugin temporaries are
therefore a deliberate foreign family for the unchanged-ABI Gate B verdict.
Gate D owns any versioned allocator-service transition; this is not treated as
an accidental convenience exception.

At Gate B completion, the only ordinary system allocation calls in the RXVM
conversion boundary should be in the depot/oversized platform provider or next
to a ledgered foreign API contract. Callback fields named `free` must remain
visibly distinct from libc `free()`.

## VM concurrency implications discovered during M0

`run()` already keeps its active frame, current module, current locals and
dispatch pointers as C-stack locals. That is necessary but not sufficient for
multiple workers.

The following mutable process-global state prevents safely running arbitrary
contexts concurrently today:

- `interrupts` in `rxvmintp.c`;
- `current_rxpa_context`, `static_linked_functions` and
  `static_linked_metadata` in `rxvmload.c`;
- fallback reference IDs in `rxvmref.c`;
- plugin factories and `current_loading_handle` in the plugin framework;
- `rxvml_active_context` in `rxvml.c`;
- the ADDRESS command directory stack in `rxcrexxcmd.c`; and
- static mutable diagnostic buffers in bundled plugins.

In addition, `rxvm_context` owns mutable globals, procedure frame free lists,
references, graph/interface caches and the socket registry. M1 must not claim
thread safety merely because allocations are worker-owned.

The later worker model should therefore start as an isolate-like model: each
worker has its own RXVM context, C/VM stack, registers, globals, references,
service registries and heap. Immutable RXBIN/module images may be factored into
a shared program image later. Python's multiple-interpreter guidance warns that
extension static state breaks isolation; [V8 isolates](https://v8.dev/docs/embed)
similarly make the VM instance and heap the unit of isolation.

Values do not cross that boundary as raw register pointers. Later transfer uses
copy, move, immutable/ref-counted external buffer or serialization semantics.
[Erlang's off-heap binaries](https://www.erlang.org/doc/apps/erts/garbagecollection.html)
and [Ruby Ractor move/copy rules](https://docs.ruby-lang.org/en/3.4/ractor_md.html)
show why large buffers and mutable object graphs need different policies.

For the eventual channel layer, the mainframe analogy is useful: a request is
submitted with explicit buffers/status, the channel progresses independently,
and completion is posted back to the owning execution context. IBM describes
channel I/O as asynchronous to the main processor, and
[libuv](https://docs.libuv.org/en/v1.x/design.html) likewise confines each
event loop to one thread while worker-pool completion returns to that loop.
Neither pattern permits an arbitrary I/O thread to call into a live RXVM stack.

## Gate B implementation slices if approved

Gate B is one unchanged-layout allocator programme, delivered in bounded
correctness-preserving slices:

1. Add the depot, one worker heap, platform slab provider, debug ownership
   validation and allocator-only unit/stress tests. Convert no value layout.
2. Convert standalone values, attribute values/pointers, VM-owned string/binary
   buffers and reference cells. Preserve stable addresses and sticky reuse;
   keep native decimal storage on its plugin allocator family.
3. Convert frames, signals, globals, modules, runtime/interface/graph metadata
   and late-load paths while retaining the current frame recycler.
4. Convert spawn, ADDRESS-command, socket/TLS-owned storage and `rxvml`
   internal storage; preserve every foreign subobject's release family.
5. Prove the dynamic-plugin allocator boundary, retain the justified foreign
   family unless an ABI-neutral route exists, and close the mechanical
   direct-call/exception audit.
6. Freeze implementation after the minimum focused correctness checks. Ask
   Adrian to clear and reserve the host, then run the mandatory ordinary
   profiling-off Release verdict against the unchanged-layout libc control.
7. Stop for Adrian before broad closeout or any value-layout experiment.

No timed cell in step 6 may start on the shared host without Adrian's explicit
reservation confirmation.

## Gate A disposition

- Retained counts are sufficient to select the hybrid substrate.
- Missing whole-allocator live/RSS/fragmentation evidence is intentionally
  supplied by allocator telemetry and the Gate B verdict, not by another M0
  performance run.
- The universal power-of-two object allocator is rejected as the product
  candidate but retained as an analytical/control policy.
- The hybrid worker heap is selected for Gate B implementation review.
- Gate B was separately approved on 2026-08-05. Value-layout work, VM workers
  and channel semantics remain closed until their later gates are separately
  approved.
