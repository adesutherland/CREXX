# cREXX Virtual Machine (Interpreter) Architecture

The `rxvm` interpreter is the runtime component of the `crexx` toolchain. It loads, links, and executes the compiled `.rxbin` bytecode. Its design emphasizes performance through direct threaded code (computed gotos), aggressive stack frame recycling, and an optimized value struct to handle REXX dynamic typing. As of `rxbin` format `003`, serialized module data and runtime-only execution state are explicitly separated, and serialized instruction/constant sections may be compacted on disk before being expanded during load.

## 1. VM Lifecycle

The execution of a program within `rxvm` is handled in discrete phases (as defined in `inc/rxvm.h`):
1. **Creation**: `rxvm_create()` allocates the root `rxvm_context`.
2. **Loading**: `rxvm_load()` ingests a `.rxbin` binary file, reads the section flags and stored sizes, expands any packed instruction/constant sections back into normal buffers, and then loads the result into an internal `module` struct. In the current `003` layout the reader accepts three record kinds: `MODULE_LOCAL`, `POOL_SHARED`, and `MODULE_SHARED`. Shared-backed modules borrow the current shared pool in memory rather than copying it.
3. **Linking**: `rxvm_link()` traverses newly loaded modules to resolve exports and external imports into a unified memory map. The call is now dirty-checked, so repeated bridge/runtime entry points become fast no-ops when no module state changed.
4. **Preparation**: `rxvm_prepare()` optionally populates per-module dispatch side tables for maximum speed without mutating serialized bytecode.
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
    size_t number_locals;            /* Number of local registers */
    size_t nominal_number_locals;    /* Procedure-declared local count */
    size_t number_args;              /* Argument count for the frame */
    unsigned char is_interrupt;      /* Signal currently being handled, or zero */
    interrupt_entry interrupt_table[RXSIGNAL_MAX]; /* Signal / Exception handlers */
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

### Signal / Interrupt Handling
The VM signal model is implemented directly in the interpreter loop. Each
`stack_frame` owns an `interrupt_table[RXSIGNAL_MAX]` and an `is_interrupt`
marker. `frame_f()` copies the caller's table into a newly entered child frame,
so handlers installed by a caller are visible to procedures it calls later, but
changes made in a child frame do not mutate the caller's table. Returning from a
procedure restores the caller's signal state by returning to the caller frame.
Each frame also owns an `interrupt_stack` used by block-scoped handlers. The
`sigpush` and `sigpop` instructions save and restore individual handler entries
on that stack. Frame cleanup clears any remaining pushed entries, which gives
block-scoped handling a safety net for frame exit and frame recycling.

Signal codes are defined in `interpreter/rxsignal.h`. The handler responses are
`IGNORE`, `HALT`, `SILENT_HALT`, `RETURN`, `BRANCH`, `BRANCH_VALUE`, `CALL`,
and `CALL_BRANCH`, exposed in RXAS as `sigignore`, `sighalt`, `sigshalt`,
`sigret`, `sigbr`, `sigbrv`, `sigcall`, and `sigcallbr`. `KILL` is always
halt-only. `BREAKPOINT` is the debugger/trace signal rather than an ordinary
error condition.

`sigcalla` installs an action-aware call handler. It receives the same raw
five-attribute interrupt object as `sigcall`, but the handler's return string
is interpreted as a VM action marker:

- `__rxsignal_skip`: resume after the signal point
- `__rxsignal_retry`: resume at the recorded interrupted address
- `__rxsignal_fail` or any missing/unknown action: fall through to the default
  panic path, including the closest preceding source location

The Level B `SIGNAL` compiler exit hides those internal marker strings behind
`.signalaction.skip()`, `.signalaction.retry()`, and `.signalaction.fail()`.

Pending signals are held in the global `interrupts` bitset. `DISPATCH` checks
that bitset after each instruction when the current frame is not already inside
an interrupt handler. The VM scans signal codes in numeric order, clears ignored
signals during the scan, and clears the selected non-breakpoint signal before
handling it. `BREAKPOINT` remains pending until `bpoff`, which lets the
debugger/trace path keep stepping.

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
REXX-level stepping should usually use exact-address `.src` metadata so it
stops on authored clause boundaries.

### `value` (Dynamic Typing Representation)
Classic REXX is a dynamically typed language where "everything is a string" conceptually, but performance dictates native type usage when possible. The `value` struct (from `interpreter/rxvalue.h`) is a polymorphic container storing a REXX variable's state. 

To limit memory fragmentation, strings shorter than `SMALLEST_STRING_BUFFER_LENGTH` (32 bytes) are stored directly inside the struct via `small_string_buffer` rather than forcing a heap allocation.

```c
struct value {
    value_type status;               /* Partitioned status/cache flags */
    rxinteger int_value;             /* 64-bit/32-bit native integer */
    double float_value;              /* Native floating point */
    
    void *decimal_value;             /* Pointer for arbitrary precision math */
    size_t decimal_value_length;
    
    char *string_value;              /* String payload pointer */
    size_t string_length;
    
    char *binary_value;
    size_t binary_length;

    const rxvm_native_payload_ops *native_payload_ops;
    unsigned int native_payload_flags;

    const char *object_type_name;    /* Runtime concrete class name */
    size_t object_type_name_length;
    
    value **attributes;              /* For associative arrays/objects */
    size_t num_attributes;

    /* Inline memory buffer to save mallocs on small strings */
    char small_string_buffer[SMALLEST_STRING_BUFFER_LENGTH]; 
};
```
Variables (`locals` arrays) consist of arrays of `value*` pointers managed strictly by the VM frames. There is no automated background Garbage Collector (GC). Instead, frame-bound variables are deterministically cleared (`clear_value`) and memory released when a `stack_frame` dies and exits scope.

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
VM-private band except through VM-owned content setters. `GETTP` and
`GETANDTP` return readable flags, while unmasked `BRTPT` only tests public
flag bands so private cache bits do not change legacy branch behavior.
Status flag instructions still take normal `rxinteger` operands in RXAS/RXBIN;
the VM applies only the low 32 bits when reading a flag mask.

In normal UTF builds, `RXFLAG_VM_UTF8_VALID` and
`RXFLAG_VM_UTF8_COUNT_VALID` mean the string byte span is known well-formed
UTF-8 and `string_chars` is a validated codepoint count. Constant loads mark
the cache as trusted, bounded string setters and text reads validate before
marking it, copy/move/string-copy preserve it, and codepoint-safe concat/slice/
truncate/append paths propagate it. Invalid or unknown byte-oriented text clears
the VM-private UTF cache bits while preserving the bytes for explicit binary or
compatibility handling.

The two `object_type_name` fields are the current Level B hook for interface
dispatch. Class factories stamp object values with `setobjtype`, and later VM
lookups use that concrete class identity when resolving interface member calls.
In UTF builds, `string_length` is the byte length while `string_chars` is the
codepoint count. Any instruction that synthesizes or truncates a string must
keep both in sync and reset `string_pos` / `string_char_pos` to the start of
the new value.

The `xtos` family of scalar-to-string conversions is allowed to mutate the
destination value to materialize its string representation. This is acceptable
for linked values, including object attributes, as representation
materialization rather than user-visible assignment. The current VM does not
maintain a validity flag for a cached string representation, so repeated
conversions still perform the conversion work and the compiler does not rely on
string-form reuse.

### Copy, Move, and Native Payloads

Rexx objects are not user-visible native pointers. A Rexx object value is still
a VM `value`: it may have attributes, a concrete `object_type_name`, scalar
storage, and an optional binary payload. Class/interface dispatch is based on
the stamped object type plus metadata, not on exposing a C pointer to Rexx code.

The VM has two distinct value-transfer paths:

- `copy_value(dest, source)` preserves the source and duplicates the current
  value state into `dest`. Strings, decimals, ordinary binaries, and attributes
  are copied into destination-owned storage.
- `move_value(dest, source)` transfers ownership of malloced buffers and
  attribute arrays into `dest`, then reinitializes `source`. This is used for
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
ordinary byte sequence. The register also carries `binary_pos`, a byte cursor
used by `SETBINPOS`, `GETBINPOS`, and cursor-based `BSLICE`.

RXAS-level binary opcodes operate only on the binary slot. `LOAD_REG_BINARY`
loads a `BINARY_CONST` from `0x...` RXAS syntax. `GETBYTE` reads zero-based
binary offsets and returns `-1` for out-of-range reads. `SETBYTE` and
`BUPDATE` are strict and raise `OUT_OF_RANGE` for invalid byte indexes or
fixed-size overlay writes past the destination length. `BCONCAT`, `BAPPEND`,
and `BSLICE` build ordinary byte buffers without UTF-8 validation and clear
VM-private UTF cache flags on the destination.

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
- The VM must own every `binary_value` buffer. Copy hooks should never install
  externally allocated memory directly into `binary_value`; they should call
  `SETNATIVEPAYLOAD()` / `rxvml_set_native_payload()` with the bytes to store.
  The helper mallocs the destination buffer, copies those bytes, and records
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

The public `rxvml_call_procedure()` and `rxvml_call_method()` entry points run
Rexx procedures by installing a temporary external-call trampoline
(`ext_proc`, `ext_argc`, `ext_args`, and `ext_ret`) and then entering `run()`.
These calls are allowed from native callbacks, including ADDRESS environment
callbacks, while another `run()` invocation is already active.

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

As of `RXVML_ABI_VERSION` 7, native `rxvml_address_request` also carries
`stdin_endpoint`, `stdout_endpoint`, and `stderr_endpoint` VM values. Native
providers should use `rxvml_address_emit_output(ctx, request, text)` and
`rxvml_address_emit_error(ctx, request, text)` rather than reaching into those
opaque redirect values. These helpers write to `ADDRESS ... output/error`
redirects and finalize them so Rexx array/string captures are readable when
the callback returns; without a redirect they write to the normal VM
stdout/stderr path.

ADDRESS command text may contain host-variable anchors whose meaning belongs to
the selected environment handler. The compiler auto-exposes visible Rexx scalar
variables named by `:name` inside string-literal command text, and by `${name}`
inside command text that reaches the ADDRESS exit. The command string itself is
not interpolated by the VM. Rexx providers read the exposed values from
`addressrequest.get_binding_value(name)`; native providers can use
`rxvml_address_binding_get(request, name, out, out_len)`. Handlers that write a
host variable return a normal `var` updated binding, so the existing ADDRESS
write-back path handles both explicit `EXPOSE` variables and auto-exposed
anchors.

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

### Threaded vs Bytecode Dispatch
The VM uses conditional compilation (`#ifdef NTHREADED`) to flip between two execution models:
1. **Direct Threading (Default/Fast Mode)**: During the "Preparation" phase, `rxvm_prepare()` fills a per-module `prepared_dispatch` array with C `void*` pointers targeting the exact `&&label` implementing each opcode. The instruction dispatch reduces to an incredibly fast computed goto: `goto *next_inst;`.
2. **Standard Bytecode Mode (`NTHREADED`)**: Operates via a massive standard C `switch(opcode)` statement wrapped in a while loop.

### Dispatch Macros
Instructions are executed via macro-driven blocks. For example, moving to the next instruction looks like:

```c
#define CALC_DISPATCH(n) { next_pc = pc + (n) + 1; next_inst = current_module->prepared_dispatch[next_pc - current_module->segment.binary]; }
#define DISPATCH         { pc = next_pc; if (interrupts && !current_frame->is_interrupt) goto INTERRUPT; goto *next_inst; }
```
`DISPATCH` actively checks a global `interrupts` bit-flag to immediately branch into signal exception handling if an error occurred natively. Internal signal-raising macros stamp `interrupted_pc` with the faulting instruction before dispatch advances `pc`; breakpoint and asynchronous interrupts leave it unset so their handlers continue to receive the next instruction/resume address. The default fallback panic report uses the stamped address when present to print the module/address and, when `META_FILE` / `META_SRC` metadata is present, the closest preceding REXX source line. Linked images built with source stripping may only have the module/address.

### Instruction Flow Example
Inside the `run()` loop, implementations are declared using `START_INSTRUCTION`. The assembler passes operands inline sequentially in the binary array.
```c
        START_INSTRUCTION(IADD_REG_REG_REG) CALC_DISPATCH(3)
            DEBUG("TRACE - IADD R%lu,R%lu,R%lu\n", REG_IDX(1),
                  REG_IDX(2), REG_IDX(3));
            REG_RETURN_INT(op2RI + op3RI)
            DISPATCH
```
In this example:
- `CALC_DISPATCH(3)` specifies that this instruction consumes 3 operand blocks.
- `op2RI` grabs the integer struct value mapped to Operand 2.
- `REG_RETURN_INT` maps the result back into the memory of Operand 1.
- `DISPATCH` safely jumps the Program Counter (`pc`) to the next instruction.

### Pooled float operands

As of `rxbin` format `002` and later, float literals are loaded from the constant pool
instead of being stored inline in operand slots. The bytecode still keeps the
same instruction formats (`FMT_F`, `FMT_R_F`, `FMT_R_R_F`, etc.), but the
operand slot now contains an index into a `FLOAT_CONST` record in
`const_pool`, and the interpreter resolves that record when a float operand is
read.

As of the current `003` layout, the loader is also responsible for undoing the
two stage-2 section codecs before execution begins:

- the instruction section may arrive as a packed logical token stream with
  operand counts reconstructed from the opcode table
- the constant pool may arrive as a compressed LZSS blob

Neither codec is visible inside the execution loop. By the time `run()` starts,
the module again looks like a normal `bin_code[]` plus raw constant-pool
buffer.

## 4. Current Interface Dispatch in the VM

The current Level B interface runtime slice adds three VM-facing pieces on top
of the older object model:

- `SETOBJTYPE_REG_STRING` stores a concrete class name on an object value
- `SRCMETHOD_REG_REG_STRING` resolves the effective method procedure from
  `object_type_name + member_name`
- `SRCFPROC_REG_STRING_REG` resolves an interface factory provider for either
  `interface_name` or `interface_name..factory_name`
- `TYPEOF_REG_REG` returns the canonical source type name of an object value
- `ISTYPE_REG_REG_STRING` tests an object value against an interface, class,
  or `.object`
- `ASSERTTYPE_REG_STRING` raises `CONVERSION_ERROR` on a failed object cast

`srcmethod` and `srcfproc` both return a `proc_runtime *` in a normal
register, and the existing `dcall` path performs the actual invocation.

### Current `srcmethod` semantics

The current implementation is now:

- it rebuilds an interface-method registry only when newly loaded modules
  invalidate that cache
- registry rows are keyed by fully qualified concrete class name plus member
  name
- for each `class implements interface` link, the VM resolves the effective
  procedure for each interface member during link
- if a concrete `class.member` procedure exists, that wins
- otherwise, if the interface member kind is `method final`, the VM binds the
  interface's emitted default-body procedure instead
- if no registry row exists, `srcmethod` still falls back to a direct
  `class.member` lookup before raising `FUNCTION_NOT_FOUND`

### Current `srcfproc` semantics

The current implementation is now:

- it handles both the default `*` interface factory surface and named factory
  selectors
- it rebuilds a factory-provider registry only when newly loaded modules
  invalidate that cache
- registry rows are keyed by interface FQN plus factory member name
- for each candidate class, it resolves the concrete `§factory` or
  `§factory.member` procedure through the existing metadata/procedure tables
- each candidate row may also carry an optional resolved `§match` or
  `§match.member` procedure
- `srcfproc` calls the effective `match` on every candidate with the same
  argument list that will later be passed to the selected factory, even when
  only one candidate exists
- if a candidate has no explicit `match`, the VM behaves as if it had an
  implicit `match` returning `1`
- candidates scoring `<= 0` are rejected
- the highest positive score wins
- ties are broken alphabetically by fully qualified concrete class name
- if no provider exists, the VM raises `FUNCTION_NOT_FOUND`

Runtime module loading matters here as well. `METALOADMODULE` marks the
VM link state dirty and immediately calls `rxvm_link()` after a successful
load, so later `srcfproc`, `srcmethod`, and direct imported calls can see
the new provider without an automatic filesystem sweep.
