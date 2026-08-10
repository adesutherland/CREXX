# cREXX Virtual Machine (Interpreter) Architecture

The `rxvm` interpreter is the runtime component of the `crexx` toolchain. It loads, links, and executes the compiled `.rxbin` bytecode. Its design supports portable switch dispatch and, on GNU/Clang-family compilers, direct threaded code (computed gotos), aggressive stack frame recycling, and an optimized value struct to handle REXX dynamic typing. The current `.rxbin` format is `007`, a coordinated compatibility break with no 006 reader. Its linker-sealed, text-backed/numeric-ID semantic graph, rule-neutral query surface, numeric type/member/factory operands, and follow-on cache/overlay design are specified in [RXBIN_007_SEMANTIC_GRAPH.md](RXBIN_007_SEMANTIC_GRAPH.md).

## 1. VM Lifecycle

The execution of a program within `rxvm` is handled in discrete phases (as defined in `inc/rxvm.h`):
1. **Creation**: `rxvm_create()` allocates the root `rxvm_context`.
2. **Loading**: `rxvm_load()` ingests one or more 007 containers, validates the fixed header and six sections, materializes portable constants/metadata and the semantic graph, expands variable-integer instructions into the normal runtime image, and loads each module directory entry into an internal `module` struct. A linked container shares one materialized pool and graph across its modules; concatenated archive containers remain independently owned.
3. **Linking**: `rxvm_link()` traverses newly loaded modules to resolve exports and external imports into a unified memory map. The call is now dirty-checked, so repeated bridge/runtime entry points become fast no-ops when no module state changed.
4. **Preparation**: `rxvm_prepare()` builds an owned per-module
   `execution_image` for both VM modes. Operand cells are copied and direct
   function operands are rebound to process-local `proc_runtime *` values.
   Computed-goto `rxtvm` stores process-local handler pointers in instruction
   cells; switch-dispatch `rxbvm` keeps copied opcodes there and may install
   process-private opcode forms. Canonical `segment.binary` remains immutable
   and is still the serialization, reflection, source/profile and debug
   identity.
5. **Execution**: `rxvm_run()` / `rxvm_call()` invoke a target procedure (typically `main`) and launch the main interpreter loop.

Runtime code can explicitly late-load another `.rxbin` or `.rxplugin` through
the debugger-style `METALOADMODULE` instruction. The public rxfnsb wrapper is
`loadmodule(path) -> .int`; it returns the last loaded module number, or a
non-positive value on failure. A successful `METALOADMODULE` immediately calls
`rxvm_link()` so existing unresolved imports can bind to procedures/classes in
the newly loaded file. There is intentionally no automatic directory sweep:
callers are responsible for loading the provider artifact they want.

## 2. Core Internal Structs

### `rxvm_context`
The root state of the VM environment. It houses the loaded modules, global configuration, and debugging state.
```c
typedef struct rxvm_context {
    char *location;
    size_t num_modules;
    module **modules;
    struct avl_tree_node *exposed_proc_tree;
    struct avl_tree_node *exposed_reg_tree;
    char link_dirty;
    char interface_method_registry_dirty;
    char interface_factory_registry_dirty;
    struct rxvm_socket_registry *socket_registry;
    char debug_mode;
    // ...
} rxvm_context;
```

`link_dirty` is raised when new modules are loaded. The separate
`interface_method_registry_dirty` and `interface_factory_registry_dirty` flags
track when the interface method and factory caches need rebuilding. This keeps
repeated `rxvm_link()` calls cheap while still supporting late module loading.
Because linked images may share one constant pool across multiple modules,
module-local runtime walkers now follow `proc_head`, `expose_head`, and
`meta_head` chains instead of sweeping the entire pool.

Under RXBIN 007 the loader validates and retains the materialized sealed-image
semantic graph. During linking it builds one process-local graph binding with a
dense callable-ID-to-`proc_runtime *` array and bound factory/provider rows.
Numeric type/member/factory operands and graph indexes avoid rediscovering
those relationships by scanning canonical metadata. Modules also own compact
side tables for dynamic method/factory instruction sites; their entries are
guarded by the context semantic generation. Generic, source/debug, RXVML, and
cross-image/native compatibility paths continue to traverse the canonical
module-local `meta_head` chain where needed. The T6 append-only overlay remains
future work; current late linking rebuilds the complete bindings and advances
the generation coherently.

`socket_registry` is the context-owned table for core TCP sockets. Rexx and
RXAS code see small positive integer handles, not native descriptors. The
registry closes every live socket during `rxfremod()`, which keeps sockets out
of `value` payload ownership and avoids stale OS resources after a VM context
is destroyed.

### `proc_runtime`
Serialized `PROC_CONST` entries now remain metadata-only. During module load,
`rxvm` builds a parallel `proc_runtime` table that carries execution-only
state such as resolved code ownership, prepared entry addresses, and frame
recycling lists.

```c
typedef struct proc_runtime {
    proc_constant *definition;      /* Serialized procedure metadata */
    int locals;                     /* Resolved local-register count */
    bin_space *binarySpace;         /* Owning code segment, or NULL for native */
    stack_frame **frame_free_list;  /* Shared frame recycler head */
    stack_frame *frame_free_list_head;
    size_t start;                   /* Resolved code address / native entry */
    char *name;                     /* Cached pointer to definition->name */
} proc_runtime;
```

### `stack_frame`
To minimize heap allocation overhead, the VM uses a custom call stack model. `stack_frame` structs maintain scope, local variables, and return state. When a function returns, the `stack_frame` is not immediately freed; it is placed onto a `frame_free_list` associated with the procedure, allowing the VM to rapidly reuse stack blocks for repeated calls.

```c
struct stack_frame {
    stack_frame *prev_free;          /* Pointer to next free recycled frame */
    stack_frame *parent;             /* Caller stack frame */
    proc_runtime *procedure;         /* Executing runtime procedure state */
    bin_code *return_pc;             /* Program Counter to return to */
    value *return_reg;               /* Target register for return values */
    unsigned char has_reference_lifetimes; /* Frame-owned storage has references */
    size_t number_locals;            /* Number of local registers */
    size_t nominal_number_locals;    /* Procedure-declared local count */
    size_t number_args;              /* Argument count for the frame */
    unsigned char is_interrupt;      /* Signal currently being handled, or zero */
    uint32_t caller_arg_base;         /* First caller call-window argument */
    interrupt_entry *interrupt_table; /* Shared or frame-owned signal policy */
    unsigned char interrupt_table_owned; /* Frame owns and must free the table */
    interrupt_saved_entry *interrupt_stack; /* Block-scoped signal handler saves */
    numeric_context num_context;     /* Numeric context for the procedure */
    struct decplugin *decimal;       /* Decimal plugin context */
    char decimal_loaded_here;        /* Whether this frame loaded decimal support */
    struct uniplugin *unicode;       /* Unicode plugin context */
    char unicode_loaded_here;        /* Whether this frame loaded Unicode support */
    value **baselocals;              /* Array of initial / fixed pointers */
    value **locals;                  /* Active pointer map to variable values */
};
```

Frame recycling is a performance feature, not a semantic shortcut. When a
frame is reused, the VM relinks local register pointers back to their base
storage, relinks globals, and resets the argument-count register. Incoming
arguments do not own value storage, but their entry pointers are recorded in
both `baselocals` and the active `locals` map. This makes `UNLINK` well-defined
for argument registers and preserves the frame-entry mapping needed when a
nested call uses `a1...` as its call window. Ordinary
return places the frame on the procedure recycler; full value teardown happens
when recycled frames are drained. Because references are rare, frames carry a
small flag that is set on the frame that owns referenced storage. A helper may
execute `MKREF` against caller-owned receiver storage, so the VM finds and marks
the owner frame rather than assuming the current frame owns the target. Only
flagged frames run the reference-lifetime cleanup on return, invalidating
frame-owned local and `a0` storage plus nested attribute storage without freeing
reusable buffers. Lexical scopes that own local storage emit `endlife` for each
storage-owning local before block metadata is closed and, when eligible, before
the compiler returns those registers to the reuse pool. Register reuse is more
conservative than lexical cleanup: arguments, `.ref` arguments,
receiver/factory pseudo-locals, exposed symbols, reference-targeted storage,
and compiler-generated inline scaffolding are not packed into the scoped reuse
pool. Generated trace-helper locals are not a normal-program hot path; they may
still receive ordinary block `endlife` / metadata closeout inside the generated
TRACE helper, but they are excluded from scoped reuse allocation. This is the
same lifetime invalidation operation as frame cleanup, but scoped to the
storage whose block lifetime has ended.
`clear_frame()` performs full storage cleanup, remaining signal-handler stack
cleanup, releases any frame-owned private interrupt table, and performs any VM
plugin instance cleanup when a frame is finally destroyed. Signal-stack
cleanup never allocates while unwinding or destroying a frame.
The `SAFE_RECYCLED_STACKFRAMES` build-time debug guard can additionally zero
locals on reuse.

Counted argument-bearing bytecode calls record the first register in the
caller's contiguous call window. Normal return still executes the
compiler-emitted reverse swaps and pays no restoration scan. If a branch-style
signal handler discards the callee before those instructions run, the unwind
path uses that one index plus `number_args` to restore the caller's active
pointer permutation. It finds each displaced call-slot base pointer in the
caller's active map and performs the inverse pointer swap, preserving mutations
and pre-call links instead of resetting values. The base pointer may be
frame-owned local storage or an incoming argument's recorded entry pointer;
neither case copies the value.

`CALL1` through `CALL4` are direct-bytecode alternatives for the common fixed
arities. They capture their explicitly named caller value pointers before
frame activation and bind them as the callee's ordinary `a1...aN` entries. No
caller pointer permutation exists, so their child frame deliberately has no
`caller_arg_base` restoration work; the callee cannot distinguish the call
form. Argument status is still established by the caller before entry. These
forms require `RXBIN007_FEATURE_FIXED_CALLS` and do not target native
procedures; compiler-generated imported/native, dynamic and higher-arity calls
retain the counted path.

Native calls have no child frame; their cold branch path recovers the counted
window from the interrupted `CALL`, `DCALL`, `SWAPCALL`, `SETTPSWAPCALL`, or
`SETTPCALL` instruction. All five forms carry the argument-count register at
operand position three, so the fixed runtime image supplies the window base
without a generic operand scan. Any future call-bearing fused opcode must be
added to this cold decoder as part of its signal/unwind contract.

### Signal / Interrupt Handling
The VM signal model is implemented directly in the interpreter loop. A root
frame owns an initialized `interrupt_table[RXSIGNAL_MAX]`; a child frame starts
by sharing its parent's table pointer. This makes the common procedure-entry
path pointer inheritance rather than a 1,280-byte policy copy. Before the first
frame-local mutation, the child makes a private copy and marks it owned, so
handlers installed by a caller are visible to later callees while child changes
still cannot mutate the caller's policy. The LIFO call-frame lifetime keeps a
shared parent table live until its children return. Returning restores the
caller's signal state by returning to the caller frame, and a recycled child
releases any private table before it is reused.

Each frame also owns an `interrupt_stack` used by block-scoped handlers. The
`sigpush` and `sigpop` instructions save and restore individual handler entries
on that stack. Any instruction that must change the frame's handler policy
first uses the common copy-on-write helper; allocation failure follows the
normal signal/OOM path. Frame cleanup clears any remaining pushed entries and
discards their saved table values without forcing a private copy, which gives
block-scoped handling a safety net for frame exit and frame recycling without
allocating during cleanup.

Signal codes are defined in `interpreter/rxsignal.h`. The handler responses are
`IGNORE`, `HALT`, `SILENT_HALT`, `RETURN`, `BRANCH`, `BRANCH_VALUE`, `CALL`,
and `CALL_BRANCH`, exposed in RXAS as `sigignore`, `sighalt`, `sigshalt`,
`sigret`, `sigbr`, `sigbrv`, `sigcall`, and `sigcallbr`. `KILL` is always
halt-only. `BREAKPOINT` is the debugger/trace signal rather than an ordinary
error condition.

`REFERENCE_INVALID` is the dedicated signal for a reference value whose target
storage has been destroyed or invalidated. It defaults to halt, participates in
normal signal handling, and can be probed without raising through the RXAS
`refvalid` instruction. Raising operations include `deref`, `linkref`, and
`setref`; using a non-reference value with those operations is treated as an
invalid reference. `endlife rLocal` is the RXAS storage-lifetime primitive used
by compiler-generated scope cleanup. It invalidates references to `rLocal` and
nested attribute storage, and releases any reference payload held by that
register, but it does not clear ordinary register contents.

`OBJECT_NOT_INITIALIZED` is the dedicated signal for a typed object value that
has not completed factory initialization. It defaults to halt and participates
in normal signal handling. The raising check is `assertinitialized`; the
non-raising probe is `isinitialized`. Type compatibility remains separate:
`istype` and `asserttype` can succeed for a typed uninitialized object.

`sigcalla` installs an action-aware call handler. It receives the same raw
five-attribute interrupt object as `sigcall`, but the handler's return string
is interpreted as a VM action marker:

- `__rxsignal_skip`: resume after the signal point
- `__rxsignal_fail` or any missing/unknown action: fall through to the default
  panic path, including the closest preceding source location

The Level B `SIGNAL` compiler exit hides those internal marker strings behind
`.signalaction.skip()` and `.signalaction.fail()`. Instruction-level retry was
retired on 2026-08-03: it is not a standard REXX trap continuation, had no
production users, and cannot safely repeat instructions with partial writes or
external side effects. A legacy `__rxsignal_retry` return is unknown and
therefore follows the fail path. Source code that needs retry uses an explicit
loop or wrapper procedure.

Pending signals are held in one worker-VM-owned execution-local
`pending_interrupts` word. The active context publishes that word's address to
native raisers only while the execution is live. `DISPATCH` checks the direct
local word after each instruction when the current frame is not already inside
an interrupt handler. The VM scans signal codes in
numeric order, clears ignored signals during the scan, and clears the selected
non-breakpoint signal before handling it. `BREAKPOINT` remains pending until
`bpoff`, which lets the debugger/trace path keep stepping.

For `CALL` and `CALL_BRANCH` handlers the VM passes a raw object-like argument
with five attributes:

1. signal code
2. module number
3. address in module
4. signal name
5. payload/message object

This raw shape is used by debugger/runtime code. Level B maps it through small
raw interop classes with explicit `with register.N` attributes, rather than
requiring library code to hand-write `linkattr1` for each slot.

Level B's `rxfnsb.runtime_signal_raw` maps the five raw VM slots, and
`rxfnsb.runtime_signal` wraps that mapped raw object behind the public `.signal`
interface. Its normal factory keeps the public `.signal(name, message)` shape,
and generated handler wrappers attach the raw VM object through the internal
`set_raw` method before invoking user code typed as `.signal`. Branch-value
handlers installed with `sigbrv` perform the same wrapping in the VM before
branching to the handler label, so compiler-generated block handlers can bind an
`as name` local directly as a user-facing `.signal` value.

Address semantics matter. VM-raised fault signals stamp the faulting
instruction address before dispatch advances. `BREAKPOINT` and native or
asynchronous interrupts use the next-instruction/resume address. Panic/error
reporting should resolve closest preceding source metadata for a fault address;
REXX-level stepping should usually use exact-address `.meta_source_step`
metadata so it stops on authored clause boundaries or debugger-selected active
ranges.

`metaloaddata` exposes structured metadata records to Level B handlers. The
record string is the value itself, and numbered attributes carry the payload:

- `.meta_source_step`: `[step_id, clause_id, flags, file, line,
  active_start_column, active_end_column, whole_source_line]`
- `.meta_trace_event`: `[kind, mode_mask, value_source, value_type,
  register_type, value_ref, source_step_id, clause_id, flags, symbol,
  resolved_name]`

The trace-event code fields are compact numeric character codes in the VM
payload. TRACE handlers map them to presentation prefixes later and may read
frame-local registers only when `value_source` names a register and `value_ref`
is non-negative.

Several ordered trace events may share one executable address after compiler
or assembler optimization. When that address is reached, the trace controller
collects every visible result event at that exact boundary into an ordered
pending batch. The generated TRACE exit drains the complete batch before
execution continues, preserving event count, metadata order and the component
value named by each record. Delivery does not scan an address range and does
not infer that skipped, branched-over or signal-bypassed instructions ran.
Consequently an executable `cnop` or conversion is not required merely to keep
two metadata events at distinct addresses.

This guarantee applies to the events retained in the optimised image; it does
not require the optimiser to preserve an event for an operation or value that
it removes or moves. Optimised TRACE may therefore differ from the no-opt
event stream, while every retained event must remain safe and ordered. Use
compiler and assembler no-opt mode for source-correspondent tracing.

Unstripped images may expose trace-event metadata. `metaloaddata` treats
optional trace-event strings such as `symbol` and `resolved_name` defensively:
absent or invalid references are reported as empty strings rather than causing
metadata inspection to fail. Linked images built with `STRIP SOURCE` do not
carry `META_SOURCE_STEP` or `META_TRACE_EVENT`; they are not source-level TRACE
or RXDB debug artifacts.

### `value` (Dynamic Typing Representation)
Classic REXX is dynamically typed, but the VM keeps frequently used native
representations direct. The selected Release 1 register layout is
`RXVM_VALUE_LAYOUT_NAME == "L32SDH"` with
`RXVM_VALUE_ABI_VERSION == 2`. It is 176 bytes in the normal 64-bit UTF-8
product and 168 bytes in a 64-bit `NUTF8` build. Compile-time assertions in
`interpreter/rxvalue.h` enforce those sizes.

The selected shape has no inline string. Its direct fields are:

- the partitioned 32-bit status word, signed 64-bit integer and binary64 float;
- a direct decimal payload pointer;
- a direct string pointer and raw `size_t` allocation capacity, followed by
  32-bit logical byte length, character count and private UTF-8 cache positions;
- a direct binary pointer with raw `size_t` actual length and capacity;
- direct native-payload operations and flags;
- direct reference identity and reference payload cells;
- the direct immutable object-type descriptor; and
- direct live/unlinked attribute maps, backing-block list, native-width
  capacity/count metadata and active attribute count.

Only string logical metrics are narrowed to 32 bits. Untrusted or growing
string ingress is checked once before mutation; invariant-preserving hot stores
are direct. Allocation capacities, allocation arithmetic, binary actual length,
decimal sizes and attribute counts remain `size_t`. They are not encoded,
shifted or decoded on access.

String, decimal, binary and attribute storage are side allocations owned by the
same worker allocator as the containing register. String storage begins in the
32-byte capacity class and grows through power-of-two classes. Decimal storage
keeps the engine's payload address direct in `value`; a fixed 16-byte
`rxvm_decimal_metadata` header containing raw `size_t` length and capacity is
immediately before that payload. Header and payload are one allocation, so
decimal growth does not require a separate descriptor allocation or a second
hot pointer indirection. Binary and attribute storage retain their direct
metadata because moving those families behind descriptors did not earn their
lifecycle and code-size cost.

Logical reset is deliberately sticky. `value_zero()` clears observable state
but retains ordinary string, decimal, binary and attribute capacity for the
same register's later reuse. Physical destruction (`clear_value()` /
`destroy_value_storage()`) runs native finalizers, destroys nested values and
returns every owned side allocation. There is no automatic pressure or
per-reset reclamation check in this baseline. `rxvm_memory_trim()` is an
explicit quiescent operation that releases only the central depot's empty
reserve; a later reclamation design requires its own evidence and approval.

The register structure is an internal, rebuild-together VM ABI, not an RXBIN
format or transport envelope. The compiler, RXVM/RXVML, RXPA, bundled decimal
plugins and any native consumer including `rxvalue.h` must be rebuilt together
when `RXVM_VALUE_ABI_VERSION` changes. A `value` pointer must never be serialized
or sent to another process or host.

### Worker Slab and Sidecar Allocation

One `rxvm_memory_context` owns a synchronized central depot; each
`rxvm_memory_worker` owns its local slabs, extents and statistics. The selected
S0 geometry is fixed at 64 KiB aligned slabs with a 64-byte header:

- byte classes are powers of two from 16 bytes through 16 KiB;
- typed value-array classes cover 1, 2, 4, 8, 16, 32 and 64 values without
  rounding each value to a generic byte class;
- reference cells have their own typed class; and
- larger byte requests and value arrays use exactly tracked, 16-byte-aligned
  oversized extents.

Within a slab, allocation first pops a worker-local returned-slot list. If that
list is empty it advances the slab's bump area: the contiguous sequence of
slots that has never yet been handed out. This is just pointer arithmetic plus
one counter, not a search or system allocation. The central lock is used only
when a worker acquires or returns a whole slab. One empty slab per worker/class
is kept local; additional empty slabs return to the depot, which retains at
most two per class and 32 overall before returning excess to the system.

`rxvm_memory_enter()` binds the active worker in thread-local storage so value
helpers can use the correct arena without threading an allocator parameter
through every opcode. Ownership is recovered from the aligned slab or extent
header. A release under a different active worker is rejected and counted as a
wrong-owner free; allocator families are never guessed. Each registered arena
also records the OS thread that created it. Entering or destroying that arena
from a different thread is rejected before its thread-local binding or local
slab lists can be changed.

A worker arena is single-thread-owned execution state, not a lockable heap
handle. It must never be entered concurrently by two OS threads. Every thread
that allocates VM-managed storage needs its own registered worker arena. An I/O
helper may instead use a deliberately independent non-VM allocation domain and
transfer its result after synchronization. The central depot is the
synchronization boundary for whole slabs; adding a lock around ordinary
allocation would hide an ownership error and defeat the local fast path.
Managed-only metadata services such as `rxvm_memory_capacity()` must likewise
receive a pointer from this allocator family. Injected or foreign allocators
retain an explicit requested-capacity contract instead of being probed for
slab metadata.

The EF-0 spawn path uses that independent-domain option. A redirect reader or
writer receives a private libc-owned single-shot completion, not an RXVM
worker, register or `value *`. String and line-array input is flattened into an
immutable byte snapshot before its writer starts. stdout and stderr readers
grow separate byte buffers, publish exactly one terminal state, and may finish
in either order without sharing mutable state. Joining the helper is the
publication/acquire boundary. Only then does the receiver thread append bytes
or construct lines in its own destination register under its currently entered
worker. Broken pipe, child/launch/thread failure, partial output and abandoned
request paths use the same join-before-destruction contract. POSIX uses
close-on-exec completion descriptors and Windows uses private non-inheritable
handle duplicates; neither platform hands a parent worker to an I/O thread.

The current Gate E E1 execution product still creates one logical worker, but
its ownership shell is now explicit. `rxvm_runtime` owns the memory context and
central whole-slab depot. The embedded `rxvm_worker` in `rxvm_context` owns the
allocator arena, thread identity and lifecycle. The compatibility CLI and
RXVML paths each create one runtime/worker pair, so no public execution or pool
API has changed.

The worker lifecycle is idle, running, draining and stopped. Only the owning
thread may start/end execution or begin teardown. Nested RXVML calls on that
same thread are depth-counted and retain the running state until the outermost
call returns; a foreign thread is rejected without changing the lifecycle.
Teardown is permitted only from idle, moves through draining, destroys the
worker arena, unregisters it from the runtime and then destroys the now-empty
runtime domain. Debug teardown continues to abort on any live allocation.

Gate E2 appends a worker-owned active-state record to `rxvm_context`, preserving
the offsets of earlier context members. This is not a guarantee that the C
compiler will retain an identical flattened-core stack/register layout; the
direct execution slot is deliberately local to `run()` and the accepted E2
verdict records the resulting code-layout effect. A checked thread-local
locator identifies the owning context during execution and native load
callbacks, but the mutable RXVML/RXPA binding, RXPA copy-out pool, SAY route and
CREXX command state live in the context. Its active interrupt field points to
the sole execution-local pending word while the VM is running and is null at
teardown. Nested same-worker execution transfers pending bits to its direct
slot and restores them to the suspended slot on return. The standalone product
designates its main VM context as the sole OS-addressable interrupt target.
POSIX signal and Windows console callbacks map the event and set a bit in that
context's own pending word; they do not create a second queue or make every
worker poll process-global state. Other contexts retain only their local word.
Any later propagation to multiple workers is explicit Gate F communication
performed outside the raw OS callback.

This shell is a prerequisite for, not proof of, multi-threaded VM execution.
Later Gate E slices give every worker its own stack/register set, frame
recycler, module globals, native/plugin instances, reference/socket registries
and procedure-affine state before creating concurrent worker threads.
`move_value()` is currently an owner-local operation:
moving its pointers into another worker would leave them owned by the source
arena and later fail the ownership check. Cross-worker communication must
therefore copy into receiver-owned storage, use an explicitly transferable
immutable buffer, or transfer ownership of a whole suitably isolated block at
a defined safe point. Process and cross-host channels must use a versioned
serialized envelope. None may expose a raw `value` or silently mutate a
register owned by another worker. The preferred future Rexx-visible envelope
is register-centric: one logical typed scalar or binary register image may
contain ordered child-register images. That `ChannelValue` is a transport
description, never the internal `value`; the receiver materializes it into its
own register tree. Large binary content may later select immutable chunks or a
bounded stream capability beneath the same logical surface.

Gate E3b-P1 makes the existing RXPA surface safe for multiple live VM contexts
without changing its initializer or procedure signature. Static constructor
registrations are retained as an owned, synchronized process catalogue and are
replayed into a distinct native module for every VM; the first VM no longer
consumes the list. A dynamic load returns an explicit DSO reference. Each VM
retains its references until its procedure frame caches, module globals,
reference values, native payloads, modules and provider instances have been
destroyed, so no reachable function or payload-operation pointer outlives its
code.

Every native `proc_runtime` carries an internal capability word in the 64-bit
alignment slot after `locals` and a load-selected invoker. A procedure from a
plugin with a valid version-1 `PROCESS_REENTRANT` manifest binds permanently to
the direct adapter. An unmarked procedure also binds direct while exactly one
legacy-capable VM is live. Registering a second legacy-capable VM quiesces
direct legacy execution, rebinds all live legacy invoker slots to the recursive
locked adapter and makes that mode sticky for the process lifetime. A
reentrant-only VM is not registered with this legacy coordinator and cannot
cause the transition.

`run()` announces and leaves the VM execution boundary to the cold coordinator;
load and teardown register or remove owned invoker slots. Ordinary native calls
load the already-selected invoker and contain no capability branch, catalogue
lock or coordinator lock. Plugin initialization remains serialized because
legacy dynamic plugins copy the helper table into the DSO-static
`_rxpa_context`; repeated `dlopen`/`LoadLibrary` calls do not imply private DSO
statics.

Native-payload `copy` and `finalize` operations remain serialized in P1 even
when the originating procedure plugin is process-reentrant. Session factories,
per-context plugin userdata and per-call capability flags are a later gate.
The public author contract and opt-in macro are documented in
`docs/ai-context/CREXX_LIBS.md`.

Variables (`locals` arrays) consist of arrays of `value*` pointers managed
strictly by the VM frames. There is no automated background Garbage Collector
(GC). Frame-bound variables are either recycled for later calls or
deterministically cleared (`clear_value`) when a `stack_frame` is finally
destroyed. Reference identities for frame-owned storage are invalidated on
ordinary frame exit for frames that own referenced storage, even if the
reference was created by a callee helper. Recycled stack storage therefore
cannot keep an escaped weak reference valid.

Attribute arrays use two parallel pointer arrays:

- `attributes`: the live logical slots
- `unlinked_attributes`: the VM-owned backing values used when a slot is
  unlinked or recycled

`set_num_attributes()` owns allocation and capacity growth. Bulk attribute
insert/delete instructions (`INSATTRS`/`INSATTRS1` and
`DELATTRS`/`DELATTRS1`) manipulate the pointer arrays directly by rotating
both arrays together, then clearing only the VM-owned backing values for
inserted or removed slots. This preserves the `UNLINKATTR` invariant and avoids
clearing an externally linked register when a logical attribute slot is deleted.

`status.all_type_flags` is a partitioned 32-bit status word stored as
`uint32_t`. The masks live in `binutils/include/rxflags.h` so
compiler-emitted RXAS and VM code agree:

- `0x000000FF`: VM-private/read-only outside the VM
- `0x0000FF00`: compiler call ABI flags
- `0x00FF0000`: stable library/runtime ABI flags
- `0x7F000000`: user/experimental flags
- `0x80000000`: reserved

The first VM-private allocations are reserved for UTF-8 validity,
codepoint-count validity, and known Unicode normalization forms. `SETTP`,
`SETORTP`, and `LOADSETTP` mask external writes; they cannot set or clear the
VM-private band except through VM-owned content setters. `SETTP` replaces only
the non-zero public flag bands present in the requested value, with
`SETTP reg,0` retaining the explicit "clear all public flags" behavior. This
keeps compiler call-ABI flag writes from clearing library/runtime cache flags
on one-register values. `GETTP` and `GETANDTP` return readable flags, while
unmasked `BRTPT` only tests public flag bands so private cache bits do not
change legacy branch behavior.
Status flag instructions still take normal `rxinteger` operands in RXAS/RXBIN;
the VM applies only the low 32 bits when reading a flag mask.

Level B register flag views expose masked status-word partitions for
system-programmer classes. VM-private, compiler call-ABI, and all-readable views
are read-only at source level. Library and user views are writable; a public
write view covers library and user flags only, not compiler flags. Source-level
flag-view writes replace only the selected masked band and must preserve all
other status bits. The compiler emits `settpmask target,value,mask` for these
writes; the VM applies `(old & ~mask) | (value & mask)` after restricting the
mask to source-writable library/user bands.

In normal UTF builds, `RXFLAG_VM_UTF8_VALID` and
`RXFLAG_VM_UTF8_COUNT_VALID` mean the string byte span is known well-formed
UTF-8 and `string_chars` is a validated codepoint count. Constant loads mark
the cache as trusted, bounded string setters and text reads validate before
marking it, copy/move/string-copy preserve it, and codepoint-safe concat/slice/
truncate/append paths propagate it. Level B text ingress rejects invalid UTF-8:
public RXVML setters return failure, `freadline`/`freadcdpt` and `bintos` raise
`UNICODE_ERROR`, socket text receive reports an invalid text status, and RXPA
native calls recursively validate returned values plus updated argument and
signal trees after the callback returns. Raw VM helpers can still clear the
cache for internal materialization, but arbitrary bytes belong on the `.binary`
path. `CREXX_RXPA_DISABLE_UTF8_CHECKS=1` disables the RXPA post-call check for
developer migration/debugging; normal builds should leave it enabled.

UTF instruction loops must use the matching metric: byte-span scans use
`string_length`, while character-index iteration and cache seeks use
`string_chars`. In particular, `POSCHAR` iterates over `string_chars` in a UTF
build; using byte length as its character bound can seek beyond the last
codepoint and turn a stale decoded value into a false match. `NUTF8` keeps the
single byte-count model.

The `object_type` descriptor pointer is the Level B hook for object identity,
type tests and interface dispatch. Graph-backed descriptors are materialized by
`rxbin` when a checked graph is built or loaded; they contain canonical
name/length plus graph/ID identity and reach precomputed assignability and
dispatch views. VM-only synthetic types use immutable static descriptors.
Class factories stamp object values with `setobjtype`, and later VM lookups use
that concrete descriptor when resolving interface member calls. Copy, move and
zero operations therefore transfer or clear one pointer rather than duplicating
name, length, graph and ID fields on every value.
Bare object class defaults are represented as ordinary object type metadata plus
the VM-private `RXFLAG_VM_OBJECT_UNINITIALIZED` flag. `setobjuninit` creates
that state, while `setobjtype` clears it when a factory has produced an
initialized object. The VM-private flag partition keeps this lifecycle marker
out of public register flag writes such as `settp`.
In UTF builds, `string_length` is the byte length while `string_chars` is the
codepoint count. Any instruction that synthesizes or truncates a string must
keep both in sync and reset the VM-private UTF lookup cache to the start of the
new value. The cache is acceleration state only: RXAS cannot observe it, value
copies do not preserve it, and optimizer metadata must not model it as a
logical read or write.

In-place string-byte writers use the private completion helpers rather than
assigning `string_length` alone. `finish_string_write()` resets the lookup cache,
validates/recounts an arbitrary UTF-8 byte span and replaces only the
VM-private UTF validity bits. `finish_ascii_string_write()` records the exact
byte/codepoint count for known ASCII output. Both preserve compiler/public
type flags, other scalar/decimal/binary/object representations, buffer
capacity and reference identity. This contract applies even when conversion
materializes a string through a linked value: the referenced storage remains
the owner, but stale codepoint validity from the previous byte span must never
survive. The PERF2-07 V3 regression covers `DCOPY; DTOS; STRLEN`, Unicode and
typed-null destinations, a live reference alias and sibling numeric writers
on both VMs and both optimization modes.

`DCOPY` treats decimal absence as a total copy of decimal absence: it clears
the destination's logical decimal length, preserves reusable backing storage
and unrelated value components, and does not signal. `DTOS` is likewise total:
decimal absence formats as `nan`, plugin diagnostics are cleared at the
conversion boundary, and the completed ASCII write refreshes string length/count
and validity metadata. Checked `INC`/`DEC` forms and both `SETNUMFUZ` forms
validate into temporary state and signal before mutating the observed integer
or numeric context.

The `xtos` family of scalar-to-string conversions is allowed to mutate the
destination value to materialize its string representation. This is acceptable
for linked values, including object attributes, as representation
materialization rather than user-visible assignment. The current VM does not
maintain a validity flag for a cached string representation, so repeated
conversions still perform the conversion work and the compiler does not rely on
string-form reuse.

### Loose String Comparison Numeric Prefilter

Loose string comparison first attempts binary64 conversion of both operands;
when either conversion fails, it performs the existing blank-padded byte
comparison. `rxvm_loose_string2float()` avoids calling the copied `strtod()`
path only for an empty span or a leading byte that cannot begin a numeric
subject. Digits, signs, whitespace, common radix bytes, `inf`/`nan` initials,
high bytes and the active locale radix initial all fall through to the exact
existing `string2float()` converter. The helper therefore changes conversion
ownership and cost, not comparison grammar or results.

Keep the helper private and out of line so `rxvm_loose_compare_text()` remains
inlineable. The no-inline spelling must cover GCC/Clang and MSVC. Do not extend
this optimization into a public opcode, serialized RXBIN form, JSON-specific
path or value cache without a separate design and invalidation/lifecycle proof.
The selected implementation and governed contract/performance evidence are in
`performance/PERF3-03-WORKLIST.md`.

### Decimal Plugin Runtime

`.decimal` values are stored in the co-allocated worker-owned header/payload
sidecar described above. The payload bytes contain the selected plugin's
decimal representation; the plugin must obtain or grow that storage through
the host `reserve_decimal` service and must not call `free(decimal_value)`.
The VM loads the default decimal plugin before executing the first
frame, attaches it to the frame numeric context, and syncs that plugin whenever
a child frame inherits a copied numeric context. A frame that loads its own
decimal plugin marks `decimal_loaded_here`, so cleanup can free it when that
frame exits.

The decimal plugin interface is the boundary between VM instructions and the
concrete decimal engine. The current `decplugin` API covers numeric-context
sync, required string sizing, conversion to and from string, integer, and
double values, coefficient/exponent extraction, arithmetic operations
(`add`, `sub`, `mul`, `div`, `pow`, `neg`), comparisons, zero testing,
truncate, and round. Decimal instructions in `rxvm` should go through that API
rather than depending on one decimal backend's internal representation.
Both decimal backends map a failed text parse to `CONVERSION_ERROR`; the decNumber
backend maps conversion-syntax and invalid-operation status bits explicitly,
while the long-double backend maps its invalid parsed value. Decimal conversion
instructions then raise that through the ordinary catchable VM signal path.
The decimal-plugin interface and `value` layout are one tightly coupled
internal contract: bundled/static/dynamic plugins and their host must be
rebuilt together for ABI version 2. This does not change RXBIN decimal
semantics or create a stable third-party wire ABI.

### Copy, Move, and Native Payloads

Rexx objects are not user-visible native pointers. A Rexx object value is still
a VM `value`: it may have attributes, a concrete `object_type` descriptor,
scalar storage, and an optional binary payload. Class/interface dispatch is
based on the stamped object type plus the semantic graph, not on exposing a C
pointer to Rexx code.

The VM has two distinct value-transfer paths:

- `copy_value(dest, source)` preserves the source and duplicates the current
  value state into `dest`. Strings, decimals, ordinary binaries, and attributes
  are copied into destination-owned storage.
- `move_value(dest, source)` transfers ownership of allocated buffers and
  attribute arrays into `dest`, then reinitializes `source`. Here "ownership"
  means owner-local value ownership; it does not retag worker slabs or authorize
  a cross-worker move. This is used for
  returning true local values from a Rexx procedure and is the efficient path
  for unique ownership.
- Rexx `RET_REG` moves only when returning a real local register. Returning an
  argument, global, or linked attribute is copied so the caller-visible source
  remains valid.
- Rexx procedure calls initially link caller argument registers into callee
  locals. The compiler/emitter is responsible for adding defensive copies for
  by-value writable formals when required by the language contract.

Native-backed objects should use the binary slot as their physical payload
carrier, but only with an explicit `rxvm_native_payload_ops` descriptor when
the payload owns native resources. The descriptor is normally a static provider
object shared by many values; the value stores just a pointer to it plus flags,
so there is no per-instance ops allocation.

Ordinary `.binary` payloads should be built through the shared helpers in
`rxvmvars.h`: `reserve_binary_buffer()`, `prep_binary_buffer()`, `set_binary()`,
`append_binary()`, `append_binary_value()`, `concat_binary()`, and
`slice_binary()`. These helpers keep `binary_length` and
`binary_buffer_length` consistent, reuse existing capacity where possible, and
clear native payload finalizers before replacing a native-backed object with an
ordinary byte sequence. Binary values carry no logical cursor.

RXAS-level binary opcodes operate only on the binary slot. `LOAD_REG_BINARY`
loads a `BINARY_CONST` from `0x...` RXAS syntax. `BCOPY_REG_REG` copies only
the binary payload; it deliberately does not copy
public/compiler/library status flags. `GETBYTE` reads zero-based binary
offsets and returns `-1` for out-of-range reads. `SETBYTE` and `BUPDATE` are
strict and raise `OUT_OF_RANGE` for invalid byte indexes or fixed-size overlay
writes past the destination length. `BCONCAT` and `BAPPEND` build ordinary byte
buffers. `BSLICE_REG_REG_REG_REG` takes explicit destination, source, start,
and length registers and clips the requested byte range to the source length.
These operations do not perform UTF-8 validation and clear VM-private UTF
cache flags on the destination. `BCHECKRANGE` validates a zero-based byte offset and
length without mutating either register; negative values and ranges that do not
fit inside the current logical binary length raise `OUT_OF_RANGE`.

Typed binary memory opcodes are strict fixed-width views over the same binary
slot. `BGETU8`, `BGETI8`, `BGETU16`, `BGETI16`, `BGETU32`, `BGETI32`,
`BGETI64`, `BGETF32`, and `BGETF64` read from zero-based byte offsets, with
register and binary-constant source forms where the source is read-only.
`BSETU8`, `BSETI8`, `BSETU16`, `BSETI16`, `BSETU32`, `BSETI32`, `BSETI64`,
`BSETF32`, and `BSETF64` write to zero-based byte offsets. These opcodes use
canonical little-endian storage order and do not use host-native struct layout,
alignment, or padding. Invalid ranges, negative offsets, and integer values
outside the target storage type raise `OUT_OF_RANGE`. `BRESIZE` preserves
existing bytes, zero-fills growth, sets only the logical byte length, and may
reuse/grow the private physical binary capacity in blocks. It raises
`OUT_OF_RANGE` for negative lengths. `BCLEAR` sets the logical binary length
to zero. `BFILL`
fills the current logical byte range and requires a byte value in `0..255`.

The Release 1 binary-memory VM surface also includes target-sized copy from a
byte offset (`BCOPY`), zero-terminated UTF-8 text fields (`BGETS`/`BSETS`),
string-constant extraction (`SGET`), different-register and same-register
memory moves (`BMOVE`/`BMEMMOVE`), and zero-copy compares
(`BCMPB`/`BCMPS`). `BCMPB` and `BCMPS` use the compare register as an input
source offset and overwrite it with `-1`, `0`, or `1`.

`FREADB` reads bytes with `fread(ptr, 1, n, file)`, so `binary_length` is the
actual byte count read, not a C item count.

When `native_payload_ops` is set:

- `clear_value()` calls the payload finalizer, if any, before freeing the
  `binary_value` buffer.
- `move_value()` transfers both the binary buffer and the ops pointer to the
  destination and clears the source, so finalization still happens exactly once.
- `copy_value()` calls the provider copy hook if one is supplied. If no copy
  hook is supplied, the VM falls back to the same byte copy used for ordinary
  `.binary` values and copies the ops pointer as-is. A copy hook is responsible
  for installing the destination payload, normally by calling
  `SETNATIVEPAYLOAD()` or the corresponding internal helper.
- The VM must own every `binary_value` buffer through its allocator family.
  Copy hooks should never install
  externally allocated memory directly into `binary_value`; they should call
  `SETNATIVEPAYLOAD()` / `rxvml_set_native_payload()` with the bytes to store.
  The helper allocates receiver-owned destination storage, copies those bytes,
  and records
  the shared ops pointer.
- Scalar/string setters clear an attached native payload before replacing the
  visible value, so stale native resources are not accidentally copied after a
  register is reused as an integer, float, or string.

That fallback is intentional but dangerous for unique native ownership. A
provider that supplies a finalizer but no copy hook must ensure bit-copied
payloads are safe to finalize more than once, for example by storing a shared
registry handle or a refcounted object. Providers that store raw pointers,
file descriptors, sockets, thread handles, or other unique resources must
provide a copy hook that retains/clones/duplicates the resource, or avoid
value-owned native payloads and use a context-owned registry handle instead.
Finalizers release the nested native resource referenced by the payload; they
must not free the `binary_value` buffer itself, because the VM frees that
buffer after the finalizer returns.

### Core Socket Registry

Core sockets deliberately follow the context-owned registry pattern described
above. `interpreter/rxvmsock.c` implements a small TCP wrapper over POSIX
sockets on Unix-like platforms and Winsock2 on Windows. The default TLS backend
is platform-selected: Network.framework on Apple platforms, OpenSSL on
non-Windows Unix-like platforms, and SChannel on Windows. Windows builds link
the platform `ws2_32`, `secur32`, and `crypt32` libraries for socket and TLS
support.

The VM supports IPv4/IPv6 name resolution through `getaddrinfo()` and creates
TCP streams on demand during `sockconnect` or `sockbind`. Timeouts, blocking
mode, `TCP_NODELAY`, and `SO_KEEPALIVE` are properties of the VM socket entry
and are applied to the native descriptor when one is open. Accepted sockets
inherit timeout and blocking mode from the listening socket.

Every socket entry carries a last-status slot and last-error string:

- `0` means success
- `1` means EOF/peer closed
- `2` means timeout
- `3` means the operation would block
- negative values are VM or OS errors such as invalid handle, DNS failure,
  bad argument, or socket-not-open

`sockstatus` reads the numeric slot and `sockerror` reads a short diagnostic
string. Operations that return data or byte counts still update the status
slot, so callers can distinguish an empty receive from timeout/EOF by checking
status afterwards.

Client TLS is layered under the same registry entries. `sockconnecttls` is the
portable client path: it connects to the host and port, starts TLS before any
application bytes are exchanged, and uses the host operand for SNI and
certificate name verification. After a successful TLS connect, `socksend`,
`socksendb`, `sockrecv`, `sockrecvb`, and `sockpending` use the active TLS
session. The instruction is available in all builds and records a negative
status when no TLS backend is compiled in.

`sockstarttls` remains a lower-level true STARTTLS instruction for protocols
that must exchange clear-text bytes before TLS. Backends that cannot upgrade an
existing connection in place return a negative unsupported status rather than
silently reconnecting.

TLS backends are selected with `CREXX_ENABLE_TLS`. Fresh CMake configurations
default to `NETWORK` on Apple platforms, `OPENSSL` on non-Windows Unix-like
platforms, and `SCHANNEL` on Windows. `OPENSSL` uses OpenSSL with default
verification paths and hostname checks, and supports both direct TLS connect
and true STARTTLS. `SCHANNEL` uses Windows SChannel/SSPI and the Windows trust
store, performs hostname verification, and supports both direct TLS connect and
true STARTTLS over an existing Winsock stream. `NETWORK` is macOS-only and uses
Network.framework, Security.framework, and CoreFoundation.framework so
certificate validation goes through the operating system trust store and VM
binaries have no OpenSSL runtime dependency. The Network.framework backend
supports `sockconnecttls` and reports true `sockstarttls` as unsupported because
the public framework API does not upgrade an existing BSD socket in place.

### Nested rxvml Calls

The public `rxvml_call_procedure_descriptor()`,
`rxvml_call_factory_descriptor()`, and `rxvml_call_method_descriptor()` entry
points run Rexx callables by installing a temporary external-call trampoline
(`ext_proc`, `ext_argc`, `ext_args`, and `ext_ret`) and then entering `run()`.
The descriptor form is mandatory as of `RXVML_ABI_VERSION` 8. Descriptors use
`rxsig1|name|return_type|args`; for example,
`rxsig1|pkg.proc|.int|name=.string`. The runtime resolves the name and checks
the return/argument signature against metadata before invoking. These calls are
allowed from native callbacks, including ADDRESS environment callbacks, while
another `run()` invocation is already active.

Nested `rxvml` calls must save and restore both the external-call trampoline
and the active `rxvml_context`. Without that preservation, a callback that calls
back into a Rexx method can overwrite the outer callback invocation state and
cause the original `run()` to return through the wrong procedure or with the
wrong argument/return vector.

The ADDRESS sandbox/stem helpers use direct VM-layout mutation for the standard
`.standardaddresssandbox` and `.standardaddressstem` classes, with nested method
dispatch reserved as the fallback for non-standard interface implementations.
This keeps repeated native callback writes stable while still allowing future
custom Rexx objects to implement the same ADDRESS interfaces.

### ADDRESS Environment Objects and Functions

`ADDRESS` environments are normal Rexx objects implementing
`_rxsysb.addressenvironment`. The runtime registry caches those objects by
normalised environment name. Rexx callers should use `addressenv(name)` to get
the cached object for direct use. The older `_address_environment(name)` helper
remains as the internal/runtime entry point.

Two optional sibling interfaces carry the prototype environment-context
extension:

- `_rxsysb.addressenvironment` includes the public `environment_name()` and
  `environment_id()` identity methods for system, path, Rexx, and native
  environments.
- `_rxsysb.addressinstance` is retained as the internal bind hook for providers
  that let the runtime attach a host-supplied instance id.
- `_rxsysb.addressfunctionenvironment` lets a provider handle generic
  environment-scoped function calls through an `addressfunctionrequest` /
  `addressfunctionresponse` pair.

Native `rxvml` hosts register command and/or function callbacks with
`rxvml_address_register_callback_environment(ctx, name, id, command_cb,
function_cb, userdata)`. The pre-release command-only
`rxvml_address_register_callback_environment(ctx, name, command_cb, userdata)`
signature was retired; pass `NULL` for `id` or `function_cb` when those features
are not needed. The native provider object stores the callback handle and
instance id, so both `ADDRESS env "command"` and explicit
`(addressenv(env) as .addressfunctionenvironment).invoke(...)` reach
the same host environment instance.

Native ADDRESS text uses the same Level B UTF-8 boundary as RXVML. CREXXSAA
validates variable setter values before buffering write-back records, and the
lower RXVML ADDRESS helpers validate command text, output/error text, sandbox
updates, and stem updates before copying them into VM strings. Invalid bytes
should be passed through `.binary` or native payload APIs rather than through
text callbacks.

Windows does not change that internal contract. cREXX command lines,
environment names/values and logical paths remain UTF-8; the platform adapter
converts them to UTF-16 for `CreateProcessW` and other Win32 `W` APIs, then
converts returned UTF-16 text to UTF-8. It must not change a process-global
console code page or C locale to simulate worker-local UTF-8. Child standard
streams remain byte streams: a provider must apply an explicit encoding
converter when a child does not speak UTF-8, and the later channel envelope may
carry that encoding metadata without changing its binary payload.

As of `RXVML_ABI_VERSION` 7, native `rxvml_address_request` also carries
`stdin_endpoint`, `stdout_endpoint`, and `stderr_endpoint` VM values. Native
providers should use `rxvml_address_emit_output(ctx, request, text)` and
`rxvml_address_emit_error(ctx, request, text)` rather than reaching into those
redirect values. These helpers write to `ADDRESS ... output/error` redirects and
finalize them so Rexx array/string captures are readable when the callback
returns; without a redirect they write to the normal VM stdout/stderr path.

ADDRESS redirect endpoint values are native payloads with the internal type name
`rxsysb.redirect_endpoint`. The payload stores a refcounted native endpoint cell,
so ordinary VM value copies retain the cell instead of byte-copying raw OS handle
values. Its private completion contains the endpoint handle, independently
allocated bytes, diagnostics and one terminal state; it contains no worker or
VM value pointer. Input endpoints snapshot their source before thread creation.
Output endpoints are consumed only after join, when the receiver worker copies
the bytes into the Rexx string or array. The last finalizer closes native
handles, joins any owned redirect worker thread and discards an unconsumed
completion. `SPAWN` resolves these native endpoint payloads before dispatching
to `rxspawn.c`; it does not accept plain raw `REDIRECT` binary buffers. Existing
bytecode remains compatible because redirect values are created by runtime
instructions, not stored as durable `REDIRECT` struct bytes in `.rxbin` files.
New runtime code should not inspect `binary_value` directly for redirect
endpoints; use the redirect helper path or the public RXVML emit helpers.

ADDRESS command text may contain host-variable anchors whose meaning belongs to
the selected environment handler. The compiler auto-exposes visible Rexx scalar
variables named by `:name` inside string-literal command text, and by `${name}`
inside command text that reaches the ADDRESS exit. The scanner also exposes
stems for `:name[]`, `:name.`, `${name[]}`, and `${name.}`. The command string
itself is not interpolated by the VM. Rexx providers read exposed scalar values
from `addressrequest.get_binding_value(name)` and stem values from
`get_binding_stem_value`; native providers can use
`rxvml_address_binding_get(request, name, out, out_len)` for scalar bindings.
Handlers that write a host variable return a normal updated binding, so the
existing ADDRESS write-back path handles both explicit `EXPOSE` variables and
auto-exposed anchors.

The built-in command environments split into four spawn modes:

- `CREXX` is the default command environment. `_address.crexx` routes it to
  `SHELLSPAWN_MODE_CREXX`, implemented by `interpreter/rxcrexxcmd.c`. This is a
  cREXX-defined command set with stable behavior across supported operating
  systems, not a shell. It owns worker-local persistent
  `cd`/`pushd`/`popd` and environment overrides,
  file/text/process/time/network commands, `batch`, and `run` for direct
  executable dispatch. Without an output or error redirect, emitted text is
  flushed to the normal VM stdout or stderr stream immediately rather than
  being held until task completion. CREXX expands host-variable scalar anchors
  to one command argument and stem anchors to zero or more command arguments
  after its own command parsing; `run :argv[]` therefore launches the child
  through an argv vector rather than by flattening the array to a command
  string. Relative file commands use the worker's logical directory. Child
  launch receives immutable merged working-directory/environment snapshots;
  the parent process directory and environment are never temporarily changed.
- `SYSTEM`, `COMMAND`, and `CMD` route the command string through the platform
  command processor so shell built-ins and command syntax work consistently.
  On POSIX, the VM invokes standard `sh -c`; it finds `sh` from `_CS_PATH`
  (the C interface for the standard utility path that `getconf PATH` exposes),
  then falls back to `/bin/sh` and finally ordinary `PATH`. Do not use the
  user's `SHELL` environment variable here; that names an interactive login
  preference, not the standard command processor. On Windows, the VM invokes
  `%COMSPEC% /D /S /C`, falling back to `cmd.exe` if `COMSPEC` is unset.
- `PATH` is the direct executable environment. On POSIX it parses the command
  into an argv vector, resolves the executable through the process `PATH`, and
  calls it directly; on Windows it uses direct `CreateProcessW` command-line
  dispatch. This route intentionally does not provide shell built-ins, pipes,
  redirects, or shell expansion.
- `SHELL` is configured shell dispatch. The VM reads `CREXX_ADDRESS_SHELL` for
  the shell executable and `CREXX_ADDRESS_SHELL_ARGS` for the arguments placed
  before the command text. If unset, it falls back to the same command-processor
  defaults as `SYSTEM`.

`CREXX` multiple-command handling is intentionally explicit: command text is a
single CREXX command and shell operators such as `;`, `&&`, `||`, and pipes are
usage errors. Use repeated `ADDRESS` statements or `ADDRESS CREXX "batch"` with
input lines. `batch` skips blank lines and `--` comments and stops at the first
non-zero return code.

`demos/native/sqlite/` shows the database-oriented form of the native provider
model. The provider routes by the ADDRESS environment name carried in the
request (`SQLITE` initially), looks up a driver table, and then treats SQL
named parameters such as `:name` as handler-specific uses of ADDRESS
host-variable bindings. This is the intended shape for later database drivers.

`demos/llm/llm_address_environment.crexx` shows the same idea for Rexx-hosted
providers. One Rexx environment class claims a family of model-shaped
environment names (`LLM_GPT_4_1`, `CLAUDE_SONNET_4_5`, `GEMINI_2_5_FLASH`,
`GEMMA4_LATEST`) and routes internally to the `rxfnsg` LLM drivers. This keeps
model dispatch cheap: `_address.rexx` caches the constructed environment object
by normalised name after the first lookup, and the provider performs a small
registry lookup over driver-contributed exact aliases and prefixes rather than
probing every driver on every command.

For Rexx callers, `addresscall(env, name, ...) -> .string` is the simple
string-returning convenience surface over `_address_function(...)`. `_address_call`
remains as the internal/runtime spelling for existing code. Use
`_address_call_response(env, name, ...) -> .addressfunctionresponse` when the
caller needs the function rc, condition, or diagnostics. This helper layer is
provider-neutral: Rexx and native ADDRESS environments see the same
`addressfunctionrequest` protocol underneath.

## 3. The Execution Loop

The core execution engine lives in `run()` within `interpreter/rxvmintp.c`. 

### Product and concrete dispatch executables

The build has one stable product entry point and up to two concrete engines:

- `rxbvm` is the portable `switch(opcode)` engine and is always built;
- `rxtvm` is the direct-threaded computed-goto engine and is built only by
  GNU/Clang-family compilers; and
- `rxvm` is the compiler-selected product entry point. It is a relative
  symlink on Unix-like systems and a copied executable on Windows.

Clang and AppleClang select `rxbvm`; GCC selects `rxtvm`; MSVC builds only the
switch engine and copies `rxbvm.exe` to `rxvm.exe`. The broad correctness and
smoke suites execute `rxvm`. A small explicit dispatch-contract test runs each
concrete engine that exists, so the non-default engine does not become an
unbuildable blind spot.

`rxvme` and the `rxvml` embedding library follow the same compiler-selected
engine. `rxbvme` and `rxbvml` remain explicit switch-dispatch forms.

### Threaded vs switch dispatch
The VM uses conditional compilation (`#ifdef NTHREADED`) to flip between two execution models:
1. **Direct Threading (`rxtvm`)**: During the preparation phase,
   `rxvm_prepare()` copies each module's canonical instruction slots into an
   owned runtime image. Operand cells remain unchanged and each instruction
   cell stores the C `void*` for its `&&label`. Target selection loads
   `next_pc->handler`, and dispatch uses `goto *next_inst;`. The runtime image
   is process-local, is never serialized or exposed through reflection, and is
   refreshed safely after a late link.
2. **Switch Dispatch (`rxbvm`, `NTHREADED`)**: Executes the owned
   `execution_image` through a C `switch(opcode)` statement. Public instruction
   cells initially contain copied canonical opcodes; preparation may replace a
   selected cell with a process-private opcode while leaving
   `segment.binary` unchanged. Operands, calls, branches and active-frame
   transitions use the same owned-image contract as `rxtvm`.

Neither source form is assumed to be universally faster. Generated performance
depends on compiler transformations, architecture, branch prediction, code
layout, and the cost of locating the next handler. The current Release 1
investigation is tracked in
`docs/planning/beta-3/notes/vm-dispatch-performance-investigation.md`.

### Active-frame and dispatch contracts

Every frame change passes through `VM_ACTIVATE_FRAME` (or its nullable
counterpart). That boundary refreshes the active frame, binary space, module,
execution base, canonical base, constant pool, and locals as one coherent
state change. Debug builds assert that all cached pointers agree with the
active frame. Operand and branch macros consume only that coherent state.

Instructions select their next target through intent macros. Sequential flow
uses `VM_ADVANCE`, canonical indices use `VM_SELECT_INDEX`, and already
resolved execution pointers use `VM_SELECT_POINTER`. Computed-goto and switch
differences stay inside these macros, and handler resolution still happens
before the current handler body completes. For example:

```c
#define VM_ADVANCE(n) do { next_pc = pc + (size_t)(n) + 1; VM_RESOLVE_SELECTED(); } while (0)
#define DISPATCH do { pc = next_pc; if (pending_interrupts && !current_frame->is_interrupt) goto INTERRUPT; VM_DISPATCH_TARGET(); } while (0)
```
`DISPATCH` actively checks the current VM's sole direct pending-interrupt word to
immediately branch into signal exception handling if an error occurred
natively. Internal signal-raising macros stamp `interrupted_pc` with the
faulting instruction before dispatch advances `pc`; breakpoint and
asynchronous interrupts leave it unset so their handlers continue to receive
the next instruction/resume address. The default fallback panic report uses
the stamped address when present to print the module/address and, when
`META_SOURCE_STEP` metadata is present, the closest preceding REXX source line.
Linked images built with source stripping have only the module/address for this
fallback context.

VM signal codes 1 through 31 map to the non-sign `sig_atomic_t` mask bits 0
through 30. `RXSIGNAL_MAX` is a sentinel and has no mask bit. All producers and
consumers use the validated `rxsignal_mask()` helper, and the interrupt scan
stops before the sentinel bit; this keeps invalid codes from shifting by a
negative count or into the signed high bit.

`INTERRUPT` is the internal dispatch target used by this macro, not a source
instruction. `INULL` and `IUNKNOWN` are runtime sentinel handlers that raise
`UNKNOWN_INSTRUCTION`; `rxas` intentionally rejects all three names as source
mnemonics. Opcode slot 514 is now `ENDLIFE_REG`; the later opcode numbers are
preserved.

### Handler definitions and placement policy

Every public opcode handler, both runtime sentinels and both private
execution-image handlers have one semantic definition behind
`RXVM_HANDLER(...)` or `RXVM_PRIVATE_HANDLER(...)`. The definitions are split
by concern across `rxvmhandlers_core.inc`, `rxvmhandlers_control.inc`,
`rxvmhandlers_numeric.inc`, `rxvmhandlers_string.inc`, and
`rxvmhandlers_system.inc`; `run()` no longer owns a second copy of any handler
body. A definition keeps the instruction-entry/debug code and its
implementation together.

The definition files are expanded twice. At file scope they produce
force-noinline `rxvm_handler_NAME(rxvm_handler_state *)` functions. Inside the
dispatch owner, handlers selected inline by `rxvmhandlerpolicy.h` expand the
same implementation body directly. The internal lowering of outlined handlers
is compiler-specific because Clang and GCC generated materially different hot
dispatch from the same source:

- Clang emits only the selected inline switch cases or computed-goto labels.
  All outlined public identities share one cold owner entry and function table.
  That edge snapshots scalar values into `rxvm_handler_state`, invokes the
  selected handler through a cold no-inline trampoline, and commits the result.
- Real GCC retains the R2 per-identity label/case and direct outlined call with
  the pointer facade. GCC optimizes that shape better than the shared-cold form;
  using the Clang repair under GCC reverses the measured benefit.

Both forms return the same small `rxvm_handler_result`: normal dispatch,
interrupt dispatch, interrupted-instruction resume, interrupt-table OOM, or
terminal cleanup. The Clang snapshot is deliberately value-based because the
earlier pointer-rich facade made owner locals addressable whenever any outlined
handler was reachable, changing Clang register allocation and stack shape even
when no outlined handler ran. The all-inline panel preserves the exact R2
pointer-facade source shape so compiler heuristics and generated owner layout do
not move merely because dead facade scaffolding was deleted; the optimizer
eliminates that facade and no shared function table or trampoline is emitted.

In the Clang shared-cold form, threaded dispatch obtains an outlined public
opcode from the immutable canonical image because its execution-image
instruction cell contains a handler address; switch dispatch reads the numeric
execution-image opcode. Label ownership, interrupt polling and final dispatch
remain in `run()` in every lowering.

The CMake cache setting `CREXX_VM_HANDLER_PANEL` selects the internal build
shape:

- `profile-20` (the provisional product default) expands the frozen 20%
  prefix while honoring the never-inline ledger;
- `all-inline` expands every handler inside `run()` and remains the literal
  source-shape/performance control;
- `all-outline` calls every handler, providing the minimum-owner/maximum-call
  control;
- `profile-5`, `profile-10`, `profile-15`, `profile-20` and `profile-30`
  expand successive prefixes of the frozen Apple public-handler heat ranking;
  and
- `max-eligible` expands every normal eligible handler while keeping the
  reviewed host-bound, reserved and sentinel classes callable.

Every handler has one central tier rather than one definition per panel. Both
private fused handlers enter at the 5% tier. The current 56-handler
`NEVER` class covers sockets, console I/O, clocks/environment access,
spawn/redirection, file I/O and dynamic module loading. It is a reviewed code-
placement attribute: literal `all-inline` deliberately ignores it to remain
the exact equivalence control, while every profile and `max-eligible` honors
it. A later profile can justify an explicit tier change, but a percentage
threshold cannot silently override it.

The frozen R2 percentage denominator of 588 non-reserved public opcode slots
included the owner-internal `INTERRUPT` target, which has no handler definition.
The implementation actually controls 589 non-reserved public-plus-private
definitions (587 public handlers plus two private). The candidate totals are
31, 61, 90, 120 and 175 for the nominal 5%, 10%, 15%, 20% and 30% panels; three
top-176 host operations remain callable. `max-eligible` is 531/589 (90.15%),
or 531/587 (90.46%) after excluding the two sentinels as well. `INTERRUPT` has
an explicit owner-only, always-inline tier.

The panel setting changes no RXAS/RXBIN encoding or public/plugin ABI. Adrian
selected common `profile-20` as the provisional product default after the R5
Apple comparison and explicitly accepted the measured 10.072% GCC `rxtvm`
Bounce regression. On this Apple host, Clang's absolute profile-20 throughput
was also directionally faster than GCC's in every governed workload/engine
cell, although the compiler runs were not a paired compiler-selection trial.

The exact 20% membership is not a permanent tuning claim. Release-finalisation
work must rebuild the panel from a wider current benchmark portfolio, include
private/fused dispatch and newly added instructions, audit the never-inline
ledger, and retain panel-membership diffs over time. Intel Linux is the next
platform validation. `all-inline` remains available as the invariant control,
and explicit `-DCREXX_VM_HANDLER_PANEL=...` selections remain supported for
diagnosis and cross-platform comparison. Existing CMake build directories keep
their cached panel; use a fresh build tree or set the option explicitly when
validating the new default.

The complete sequence of accepted and rejected source shapes, the Clang/GCC
code-generation differences, and the current rules for preserving maximum
observed C optimisation are recorded in the
[VM and C Compiler Optimisation Report](../planning/release-1/vm-c-compiler-optimisation-report-2026-08-09.md).

### Process-private fused execution handlers

Preparation performs two structural, process-private fusions in each module's
owned execution image. This is immutable load-time quickening, not adaptive
runtime rewriting: canonical RXBIN and `segment.binary` remain unchanged, and
the prepared site is not subsequently rewritten or dequickened.

- `PRIVATE_R1_RELINK` recognizes `unlink destination; linkref destination,
  source_reference` when source and destination differ. It performs the fast
  relink when the runtime reference target is valid, otherwise resumes the
  canonical `linkref` path. Debug mode or a pending breakpoint also uses the
  canonical observable path.
- `PRIVATE_R2_COPYATTR1` recognizes `linkattr1 temporary, object, immediate;
  copy destination, temporary; unlink temporary` with the required distinct
  registers. It uses the fused reference-descriptor path only when the runtime
  payload permits it; range errors, generic values, debug mode and pending
  breakpoints retain the canonical instruction-by-instruction behaviour.

`rxtvm` writes the corresponding private label address into its execution
image; `rxbvm` writes a numeric opcode above the public opcode range. Both map
instrumentation back to the first canonical public opcode so public semantic
profiles remain comparable. Placement/heat profiling must additionally count
the private dispatch identity: attributing it only to `UNLINK` or `LINKATTR1`
can incorrectly classify a frequently executed private handler as cold.

The shared inventory and gap report between RXAS static fusion and these VM
load-time fusions is a separate roadmap item, `PERF3-05-R4`. Making a private
fusion a normal serialized instruction, or adding adaptive runtime quickening,
requires its own RXAS/RXBIN compatibility and architecture decision.

### Instruction Flow Example
The assembler passes operands inline sequentially in the binary array. A
handler definition is written once and the placement policy supplies its
dispatch label/case:
```c
RXVM_HANDLER(IADD_REG_REG_REG,
    VM_ADVANCE(3)
    DEBUG("TRACE - IADD R%lu,R%lu,R%lu\n", REG_IDX(1),
          REG_IDX(2), REG_IDX(3));
    REG_RETURN_INT(op2RI + op3RI)
    DISPATCH
)
```
In this example:
- `VM_ADVANCE(3)` specifies that this instruction consumes 3 operand blocks.
- `op2RI` grabs the integer struct value mapped to Operand 2.
- `REG_RETURN_INT` maps the result back into the memory of Operand 1.
- `DISPATCH` safely jumps the Program Counter (`pc`) to the next instruction.

`VM_ADVANCE(n)` and `REG_OP(n)` accept any operand position represented by the
opcode signature; handlers are not restricted to the traditional three named
operand aliases. `CNOP_REG_REG_REG_REG_REG_REG_REG_REG_REG` is the focused
nine-operand execution and tooling regression.

### Opcode effects inventory

The VM handlers are also the semantic evidence for the machine-readable opcode
effects inventory consumed by RXAS and future data-flow work. The canonical
structural inventory remains `binutils/include/rxops.h`; the complete ordered
semantic sidecar is `binutils/include/rxopeffects.h`; and
`rxop_effects()` in `binutils/rxopmeta.c` exposes their consolidated C API.
Effects cover explicit and implicit register access, proven overwrites,
branch-target operands, calls/returns, aliases, references and storage
lifetimes, possible exceptional transfer, indirect effects and conservative
barriers.

The inventory is an audited contract, not a parser for arbitrary handler C.
Handler-sensitive classifications are checked against the implementations in
this instruction region and representative semantic tests. Every opcode slot
must have an entry. Reserved/internal slots, explicitly conservative entries,
and unknown/out-of-range API queries fail closed: they remain barriers and
offer no kill proof. NR-04 does not change VM execution or serialized RXBIN and
does not enable a new optimizer transformation.

### Private native stem representation

NR-15's `stem*` handlers share `interpreter/rxvmstem.h` in both dispatch modes.
The D2-hybrid representation uses the receiver's ordinary binary buffer for a
versioned little-endian header, 256 bucket heads, and 16-byte
hash/next/generation entries. Ordinary VM attributes own the keys, values, and
default string, so normal deep copy, move, reference lifetime, and destruction
continue to own all string storage. Bucket and next indexes are one-based; zero
is the chain sentinel. Entries stay in insertion order.

The layout is private process-local state, not public ABI and not serialized
into RXBIN. `RXBIN007_FEATURE_NATIVE_STEM` gates the nine instruction forms,
while each runtime receiver is validated for magic, version, exact size,
capacity/count consistency, attribute coverage, and bounded chain traversal.
Allocation-reporting paths commit logical insertion, update, reset, or output
replacement only after their required storage succeeds; reserved private
capacity may remain reusable after a failed insertion. Generation overflow,
capacity overflow, invalid extraction indexes, and corrupt metadata are
translated to VM signals by the shared handler macro.

`stemget2` and `stemset2` hash and compare two string registers with one `.`
separator without constructing a hit/miss key. `stemset2` constructs the
canonical joined key once, and only after absence is proved. Compiler direct
lowering is intentionally limited to proved simple-storage concrete
`rxfnsb.stem` receivers; complex or reference-sensitive receiver shapes keep
the normal method path, whose library body invokes the same native operations.

`interpreter/rxvminstrument.h` is the compile-time instrumentation contract for
both VM modes. A backend can observe VM begin/end, instruction begin plus
retire or terminal, frame activation, call/return transitions, and interrupt
selection/entry/resume/terminal paths using canonical module/instruction
coordinates. The contract also exposes native-call boundaries and module-set
changes needed by callable profiling. With no backend selected, every hook
preprocesses to a no-op and does not evaluate its arguments, branch, access
state, or call a function.

### Timing/count profiling

`CREXX_VM_PROFILING` selects the timing/count instrumentation backend for a
dedicated VM build. The option is off by default, so ordinary builds retain the
fully preprocessed no-op hook contract. Configure a Release profiling build
with:

```sh
cmake -S . -B cmake-build-profile \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=ON
cmake --build cmake-build-profile --config Release \
  --target rxvm rxbvm rxtvm rxvme rxbvme rxseq
```

Omit `rxtvm` when the configured compiler does not support GNU-style
labels-as-values (notably MSVC).

The command-line surface is compiled into compiler-selected `rxvm`, concrete
`rxbvm`, optional concrete `rxtvm`, and the embedded-standard-library variants
`rxvme` and `rxbvme`. Timing profiling remains off at
runtime until `--profile`, `--profile=timing`, or `--profile-output` is passed.
`--profile=counts` selects the same diagnostic census with clock reads disabled
and all timing fields zero, so repeated counts-only reports are deterministic.
By default the report is written as a human-readable table to standard error.
Use `--profile-output file` or `--profile-output=file` to write it to a file.
An output filename ending in `.csv`, case-insensitively, selects CSV; every
other filename selects the table format. The output option also enables
profiling.
Profiling options are parsed before the first RXBIN filename; ordinary program
arguments continue to follow `-a`.

```sh
rxvm --profile program.rxbin
rxvm --profile-output profile.txt program.rxbin
rxbvm --profile-output profile.csv program.rxbin
rxvm --profile=counts --profile-output counts.csv program.rxbin
```

The instruction table measures monotonic wall time from instruction entry to
retire or terminal. The transition table measures retire to the next
instruction entry and distinguishes same-frame sequential/branch transitions,
call frame entry, return frame exit, interrupt entry/resume, external entry,
and termination.

Each instruction row also reports the effective handler placement as
`inline`, `outline`, or `mixed`. Profiling records placement at the actual
handler-entry boundary while retaining canonical public-opcode timing/count
attribution, so a private fused handler cannot silently inherit an incorrect
label from its serialized opcode. `mixed` means the same canonical opcode was
observed through both placements. CSV schema 5 writes this in the existing
`value` column; ordinary profiling-off builds still preprocess the added hook
argument away without evaluating it.

The same report contains procedure/method and call-mechanics tables. Callable
names, return types, and argument signatures come from `META_FUNC`; a
module/procedure fallback is used for older binaries without that record. The
procedure table classifies runtime rows as procedure, method, factory, or
native and reports calls, normally completed calls, calls discarded by an
exceptional unwind, elapsed call time, inclusive body time, and self time.
Rows are sorted by elapsed time (native total for native rows). Inclusive body
time includes nested bytecode calls, so rows overlap; self time does not.
Each bytecode row also reports `native_child`, the observed native-call time
removed from self time. It is already inside inclusive body time and must not
be added to elapsed time.
Inlined calls have no runtime frame and therefore remain attributed to their
containing procedure rather than appearing as separate rows.

For bytecode calls, the normal-return boundaries are:

```text
caller call-instruction entry
    -> callee first-instruction entry       entry overhead
    -> callee return-instruction entry      inclusive body
    -> caller next-instruction entry        exit overhead
```

Elapsed time covers the whole outer span. These are observed VM call-mechanics
spans, not an estimate of the speed-up from inlining. External entry and
terminal return are measured against their nearest available VM boundary.
Dynamic calls are attributed to the concrete runtime procedure.

The same profiling-only backend also records the NR-05 dynamic call census at
authoritative VM boundaries. It distinguishes direct/dynamic bytecode and
native calls, external/root entry, signal-handler entry, failed/unresolved
attempts, exact actual arity, callable metadata kind, and fresh/reused/no-child
frame disposition. `RET_REG` records its actual true-local move or
non-local copy branch; void, ignored, immediate, terminal, and unwind domains
remain distinct. `SRCMETHODSEL` and `SRCFPROCSEL` record
attempt/success/failure separately from the later `DCALL`.

Call-window attribution uses a dynamic backward slice of the executed
straight-line trace, not source names or adjacency. The NR-04
`rxop_effects()` kill/flow facts stop the slice at proven
definitions. Reached `SWAP_REG_REG` operations are setup swaps and
reached whole-value `COPY_REG_REG` definitions are defensive
argument copies. For normal return, the profiler reconstructs the pointer
permutation made by those setup swaps and buffers subsequent swaps until their
combined effect recovers the pre-call mapping; only that completed restoration
sequence receives credit. A setup whose mapping is deliberately carried into
another call or to frame completion has no completed normal restoration, but
does not by itself imply lost profiling data. All remaining executed
swaps/copies stay unclassified. The cold signal paths report discarded frames, restored
bytecode/native call windows, actual inverse pointer swaps, and restoration
failures without changing the existing restoration algorithm.

Native plugin calls are listed with call count and total time around
`rxvm_callfunc`. Body/self and entry/exit breakdowns are intentionally omitted
because the VM cannot observe the native implementation's internal phases.
Native time remains part of its calling instruction's instruction timing, but
is removed from the bytecode caller's self time to avoid double attribution in
the procedure view. The instruction, transition, and procedure sections are
overlapping views and must not be summed together.

Every retired hot-loop instruction also increments the interrupt-poll count.
Taken interrupt scans and the mechanics from selection to the first handler
instruction, resume, or terminal outcome are reported as sub-phases. These
sub-phases overlap the complete interrupt transition time and must not be added
to it. The no-pending poll itself remains inside ordinary transition time; the
profiler intentionally does not add another pair of timer reads around that
check.

CSV output retains the original columns in their original order and identifies
the format as schema version 5. Schema 5 preserves the same 24-column header:

```text
section,name,value,id,count,total_ns,average_ns,min_ns,max_ns,percent,selected,entries,resumes,terminals,module,kind,completed,unwound,return_type,args,bytes,max_bytes,high_water,status
```

Procedure rows use `value` to distinguish `elapsed`, `inclusive_body`, `self`,
`native_child`, `entry_overhead`, `exit_overhead`, and `native_total` metrics.
There are multiple metric rows per bytecode callable rather than one
denormalized row.

Schema 5 retains all schema-4 rows and adds explicit per-domain `status` rows,
`value_operation` rows for whole-value/typed transfer and teardown helpers,
`frame_entry` rows split by fresh/reused frame source, and canonical `branch`
site rows. For a branch row, `selected`, `entries`, `resumes`, and `terminals`
carry taken, fall-through, same-module backward-target, and cross-module-target
counts. For a frame-entry row, `id` carries phase units. Schema-5-aware readers
must continue to accept schema 4 and treat these new domains as unavailable
rather than fabricating zeroes.

Schema 4 retained the schema-3 `allocation` rows and added
`census` aggregates, exact `call` rows,
`return` placement, `dynamic` selection,
`mechanics` attribution, and `unwind` restoration rows.
For `call` rows only, the existing `bytes`,
`max_bytes`, and `high_water` columns carry setup swaps,
normal restoration swaps, and defensive argument copies. Summary and row
status expose overflow or degraded tracking; zero-count categories are
retained.

The allocation counters are private profiling instrumentation, active only
between profile begin/end, and count successful requests rather than live
heap:

- `frame_blocks` counts fresh combined frame/pointer/local-value blocks;
  `frame_activations` counts fresh plus recycled activations, records maximum
  simultaneous active frames in `high_water`, and `frame_reuses` identifies
  recycler hits;
- `standalone_values` counts successful `value_f()` allocations;
  `attribute_value_blocks` and `attribute_pointer_storage` cover VM-managed
  object-attribute capacity;
- `string_buffers` and `binary_buffers` cover VM value-buffer allocations and
  capacity changes, with successful reallocations charged at their requested
  new capacity;
- `value_slots` totals counted standalone, frame-local/a0, and attribute
  `value` structs. Its byte fields overlap their containing allocation rows by
  design.

The allocation scope excludes loader state, profiler/RXSEQ bookkeeping,
plugin-private/native-payload ownership, reference-lifetime payloads, OS/TLS,
and temporary native conversion storage. Allocation rows carry `complete`,
`overflowed`, or `degraded`; summary rows retain counter overflow plus
procedure/allocation tracking availability. A non-zero active-frame balance
degrades the frame-activation row.

Timing values are raw instrumented wall times. The report includes the minimum
positive adjacent clock-read interval and zero-delta calibration count, but
does not subtract them from short instructions. Compare instruction shares
within equivalent profiled runs rather than treating the values as the
uninstrumented VM's absolute cost. Instruction and transition counters use
fixed per-run arrays; the common hot path performs no allocation, locking,
callbacks, sorting, or output formatting. Allocation accounting itself is
fixed-state TLS-local addition at the existing allocation/frame boundaries.
The procedure activation array is
allocated once per run and grows only if call depth exceeds its current
capacity. Bytecode procedure timing reuses the existing instruction
timestamps; only native calls add a dedicated timer pair, and exceptional
stack unwinds add a timestamp outside the ordinary path.

Do not use an inactive profiling build as the uninstrumented performance
baseline: the enabled backend still contains inexpensive runtime guards. Only
a build configured without `CREXX_VM_PROFILING` has the compile-time no-op
shape.

### Dynamic instruction-sequence profiles

The same `CREXX_VM_PROFILING` build can extract executed windows of two,
three, or four instructions. This is a separate run mode from timing profiling
and must be given an `.rxseq` output file:

```sh
cmake --build cmake-build-profile --target rxvm rxbvm rxtvm rxseq

rxvm --sequence-count=2 --sequence-output run.rxseq program.rxbin
rxbvm --sequence-count 4 --sequence-output run.rxseq program.rxbin
rxtvm --sequence-count 4 --sequence-output run.rxseq program.rxbin
```

`--profile` and `--sequence-count` are intentionally mutually exclusive.
Ordinary builds configured without `CREXX_VM_PROFILING` contain neither
runtime surface.

The VM records dynamic execution counts against `(module, canonical starting
instruction slot, window length)`. At run start it allocates one 64-bit counter
per expanded instruction slot in every loaded module; the on-disk result is
sparse. A window continues only across actual sequential fall-through
transitions in the same module and frame. Taken branches, bytecode-frame calls
and returns, interrupt entry/resume, external frame entry, and termination
break it. A native call that returns normally remains within its CALL
instruction and can participate in a sequential window. A branch may be the
last instruction in a window, but a window never crosses the branch when it is
taken. Loop iterations increase the recorded site count.

The extractor writes a versioned binary instruction-sequence execution profile.
It starts with the eight-byte `RXSEQBIN` magic. Its fixed 48-byte header uses
little-endian integers and records the format version, header size, sequence
length, VM result, flags, module count, and sparse site count. Each module
record contains a variable-length ID, fixed little-endian 64-bit
expanded-content hash, variable-length instruction size, and a length-prefixed
UTF-8 name. Each site stores `(module ID, start slot, count)` as canonical
unsigned LEB128 values, so common records take only three to five bytes. The
format contains no process-sized integers, native structure padding, or
host-endian fields.

The VM deliberately does not decode or normalise operands in the interpreter
hot loop. It writes only non-zero aggregated sites, so repeated loop executions
increase a 64-bit count rather than increasing the file length.

Run the offline second stage with the same RXBIN module set:

```sh
rxseq run.rxseq program.rxbin library.rxbin
rxseq run.rxseq program.rxbin library.rxbin --output candidates.csv
```

Module argument order does not matter, but every profiled module must be
present with the exact content used by the run, and no additional module may
be supplied. `rxseq` fails on a missing module, content-hash mismatch, or
instruction-size mismatch. The profile records all loaded modules, even those
with no non-zero site. Consequently, a capture made with `rxvme` or `rxbvme`
also requires the exact RXBIN image corresponding to its embedded standard
library. Prefer `rxvm`/`rxbvm` with explicit inputs or a single linked image
when a self-contained sequence-analysis workflow is needed.

For each site, `rxseq` decodes the emitted RXBIN instructions and
alpha-renames operands by first occurrence across the whole window. Registers
use `r1`, `r2`, and so on; every other encoded operand (literal, pool
constant, label, or procedure reference) uses `c1`, `c2`, and so on.
Reuse is retained:

```text
IADD_REG_REG_REG(R17,R5,R9) | COPY_REG_REG(R5,R22)
    -> IADD_REG_REG_REG(r1,r2,r3) | COPY_REG_REG(r2,r4)
```

Sites with the same normalised pattern are clustered, and their dynamic counts
are summed. The report includes execution count, static site count, module
count, one concrete mapping/example, and `candidate` status. Operand
normalisation and mapping text are dynamically sized, so candidates are no
longer screened out merely for having more than three distinct symbols. This is only candidate
extraction: control-flow, liveness, aliasing, exceptions, interrupt behaviour,
and other transformation safety must be reviewed separately before defining a
combined opcode or optimiser rule. The table heading also reports the window
length, cluster/site counts, and counter-overflow status. An output name ending
in `.csv`, case-insensitively, selects CSV; otherwise `rxseq` writes the
human-readable report format. Candidate CSV uses:

```text
rank,count,sites,modules,symbols,status,pattern,mapping,example_module,example_start
```

### Frozen PARSE execution

NR-14 adds four canonical RXBIN 007 instructions behind
`RXBIN007_FEATURE_FROZEN_PARSE`. `parsewords3`, `parsepos2`, and
`parsewords3d` implement exact common plans with direct register results; their
handlers preserve source/output aliasing by snapshotting source bytes before
the first write. `parsewords3` is also a chain primitive for eligible longer
implicit-word templates.

`parseplan` executes a compact descriptor from a string constant into a reusable
result vector. Version 1 stores frozen item kinds, store/drop flags, literal
bytes plus character lengths, fixed-width numeric movement, and the declared
item/result counts. Version 2 additionally stores indexed dynamic delimiter and
position references. A reference selects either an earlier completed capture
or an external operand that compiler-generated code placed temporarily after
the public result slots. The handler consumes those values without textual
plan decoding and shrinks the vector to its public result count on success.

The handler bounds-checks the header, reference indexes, and every item; rejects
trailing or structurally inconsistent data with `INVALID_ARGUMENTS`; and raises
`CONVERSION_ERROR` for an invalid dynamic numeric position. It uses Unicode
code-point positions in UTF builds and has no load-time cache or private
prepared representation.

Compiler eligibility remains fail-closed: exact common forms use the direct
instructions and every remaining supported exit form uses `parseplan`, including
logging/TRACE and explicit `INTO`. Ordered source-level assignments remain
outside the VM primitive, preserving repeated targets, source aliases, trimming,
and TRACE source metadata. There is no runtime textual PARSE executor.

### Pooled float operands

As of `rxbin` format `002` and later, float literals are loaded from the constant pool
instead of being stored inline in operand slots. The bytecode still keeps the
same instruction formats (`FMT_F`, `FMT_R_F`, `FMT_R_R_F`, etc.), but the
operand slot now contains an index into a `FLOAT_CONST` record in
`const_pool`, and the interpreter resolves that record when a float operand is
read.

In RXBIN 007 the loader decodes the canonical variable-integer instruction
section and portable constant/metadata records before execution. The base
format deliberately does not compress sections. By the time `run()` starts,
the module again presents a normal `bin_code[]` plus native constant-pool
buffer, while graph-bearing operands remain dense graph IDs.

## 4. Current Interface Dispatch in the VM

The current Level B interface runtime slice adds these VM-facing pieces on top
of the older object model:

- `SETOBJTYPE_REG_STRING` uses its serialized type ID to store one immutable
  `RxGraphTypeRef *` on an object value
- `SRCMETHODSEL_REG_REG_STRING` uses its member ID, the receiver descriptor's
  dense dispatch row, the graph's bound callable array and a two-way
  generation-guarded instruction-site cache
- `SRCFPROCSEL_REG_STRING_REG` uses its factory-bucket ID and a cached bound
  provider range; a single-provider/no-match bucket has a direct-target path
- `TYPEOF_REG_REG` returns the canonical source type name of an object value
- `ISTYPE_REG_REG_STRING` tests an object value against an interface, class,
  or `.object`
- `ASSERTTYPE_REG_STRING` raises `CONVERSION_ERROR` on a failed object cast

`srcmethodsel` and `srcfprocsel` both return a `proc_runtime *` in a normal
register, and the existing `dcall` path performs the actual invocation. Human
RXAS/RXDAS uses callable descriptors (`rxsig1|name|return_type|args`), but a
linked 007 instruction carries a numeric graph member/factory ID. Signatures
are validated while the graph and bindings are built, not reparsed on the
sealed success path.

### Current `srcmethodsel` semantics

The current graph fast path is:

- every graph callable is resolved to `proc_runtime *` once during a semantic-
  generation binding rebuild
- the receiver descriptor provides a dense member-to-callable row, and the
  callable ID indexes the bound target array
- a two-way site cache is keyed by receiver descriptor and semantic generation
- if a concrete `class.member` procedure exists, that wins
- otherwise, if the interface member kind is `method final`, the VM binds the
  interface's emitted default-body procedure instead
- cross-graph/native compatibility may still use the legacy sorted registry,
  but the sealed numeric path does not

### Current `srcfprocsel` semantics

The current graph fast path is:

- it handles both the default `*` interface factory surface and named factory
  selectors
- each factory ID indexes a prebound provider range; every row contains the
  concrete class name, factory target, optional match target and match-required
  flag
- the instruction-site cache retains that binding across calls and is cleared
  by semantic-generation mismatch
- a one-provider bucket with no explicit `match` returns its direct target
  without running the general scoring loop
- `srcfprocsel` calls the effective `match` on every candidate with the same
  argument list that will later be passed to the selected factory, even when
  only one candidate exists
- if a candidate has no explicit `match`, the VM behaves as if it had an
  implicit `match` returning `1`
- candidates scoring `<= 0` are rejected
- the highest positive score wins
- ties are broken alphabetically by fully qualified concrete class name
- if no provider exists, the VM raises `FUNCTION_NOT_FOUND`

Graph construction matches source-short and canonical class-type spellings in
factory signatures. This prevents a valid source descriptor from creating a
duplicate providerless factory bucket during RXAS/RXLINK remapping. The
standalone graph harness audits the numeric IDs embedded in executable
instructions in addition to measuring a known-valid bucket.

Runtime module loading matters here as well. `METALOADMODULE` marks the
VM link state dirty and immediately calls `rxvm_link()` after a successful
load, so later `srcfprocsel`, `srcmethodsel`, and direct imported calls can
see the new provider without an automatic filesystem sweep.

The `crexx` driver keeps bare `-l` names as packaged libraries below
`CREXX_HOME/bin`, but any `-l` value containing `/` or `\\` is an exact path.
Late-loading applications should pass the intended `.rxbin` filename to
`loadmodule()` explicitly; neither the driver regression nor the VM late-load
path searches user-controlled directories for a provider.
