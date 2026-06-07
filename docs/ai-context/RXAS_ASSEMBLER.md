# cREXX Assembler Architecture (`rxas`)

The `rxas` assembler is responsible for translating human-readable Intermediate Representation (IR) assembly (`.rxas`) into packed executable bytecode (`.rxbin`) consumed by the `rxvm` interpreter.

## 1. Assembler Pipeline

The assembler processes source files through a pipelined, pseudo-two-pass architecture:

1. **Lexical Analysis (`re2c`)**:
   - Source defined in `assembler/rxasscan.re`.
   - Tokenizes input into REXX Assembly primitives: registers (`RREG`, `GREG`, `AREG`), literal types (`STRING`, `INT`, `FLOAT`, `DECIMAL`, `HEX`), symbols (`ID`, `LABEL`, `FUNC`), and assembler directives (e.g., `.locals`, `.globals`, `.expose`, `.meta`).
   - `HEX` is the RXAS binary-literal token. It accepts byte-paired
     `0x...`/`0X...` text, including empty `0x`, and is stored as
     `OP_BINARY` rather than as an integer literal.
   - The lexer also recognizes the interface metadata keywords used at the end of
     `.meta` records: `.interface`, `.implements`, and `.member`.

2. **Parsing (`Lemon`)**:
   - Grammar defined in `assembler/rxasgrmr.y`.
   - Enforces the structural integrity of the `.rxas` file (headers, function definitions, variable declarations, and instruction sequences).
   - The parser actions invoke Builder API functions directly (e.g., `rxasgen*`, `rxaslabl`, `rxasproc`), translating syntax rules into buffered internal data structures.
   - Instruction names are derived from `binutils/include/rxops.h`, with a
     public-source filter for bytecode/runtime-only entries. `RESERVED_*`,
     `INULL`, `INTERRUPT`, and `IUNKNOWN` remain opcode-table entries but are
     deliberately rejected as RXAS mnemonics. Recent core runtime additions
     include the object/interface instructions and the `sock*` TCP socket
     instructions.

3. **In-Memory Buffering & Constant Pooling**:
   - Handled in `assembler/rxasassm.c`.
   - Instructions and operand slots are appended sequentially into a dynamic array of `bin_code` elements.
   - Complex and pooled literal types (Strings, Binary literals, Floats, Decimals, Procedure Headers, Metadata) are deduplicated via AVL Trees and injected into a variable-length **Constant Pool**.

4. **Backpatching & Optimization (Second Pass)**:
   - Forward references (branches to undefined labels, calls to undefined procedures) are logged as a linked list of references.
   - At the end of the parse, `backptch()` traverses all trees, resolves symbol addresses, updates the binary stream, and applies peephole jump optimizations.

## 2. Core Internal Data Structures

The state of the assembler is held within the `Assembler_Context` struct, specifically within the `context->binary` object, which mirrors the layout of the final `.rxbin` file.

### Instruction Stream
Instructions are flattened into an array of unions called `bin_code` (defined in `binutils/include/rxdefs.h`). An instruction is represented by an opcode element, immediately followed by elements representing its operands.
```c
// Example buffer size handling in gen_instr()
context->binary.binary[context->binary.inst_size].instruction.opcode = opcode;
context->binary.binary[context->binary.inst_size++].instruction.no_ops = operands;

// Operand elements
context->binary.binary[context->binary.inst_size++].index = get_reg_number(...);
context->binary.binary[context->binary.inst_size++].iconst = token->integer;
```

### Constant Pool
Strings, binary literals, pooled float literals, procedure mappings, debug metadata, and exported symbols are packed into `const_pool`. This is a sequential buffer of dynamically sized records. Every record starts with a `chameleon_constant` header dictating its type and byte size.
Types include: `STRING_CONST`, `BINARY_CONST`, `FLOAT_CONST`, `PROC_CONST`, `EXPOSE_REG_CONST`, `EXPOSE_PROC_CONST`, `META_FUNC`, `META_INLINE`, `META_REG`, etc.

The serialized `expose_head` chain includes both `EXPOSE_REG_CONST` and
`EXPOSE_PROC_CONST` records. Runtime linking and other module-local walkers now
rely on that chain instead of scanning the whole constant pool.

The interface/callable-contract work extends that same metadata path rather
than introducing a second binary header mechanism. In addition to `META_CLASS`
and `META_ATTR`, the assembler now serializes:
- `META_INTERFACE` for one interface header
- `META_IMPLEMENTS` for one concrete-class-to-interface link
- `META_MEMBER` for one interface method or factory declaration

Cross-file compiler inlining also uses the metadata path. Callable signatures
remain in `META_FUNC`; inline-body templates are carried separately in
`META_INLINE`, emitted in RXAS as:

```rxas
.meta "fully.qualified.callable"=".inline" "I4;..."
```

The `I4` payload is the compiler-owned inline transport described in
`compiler/docs/inlining_design.md`. `rxas` stores it as `META_INLINE`, and
`rxdas` must emit it back to the same logical `.meta ... ".inline" "I4;..."`
spelling so source, RXAS, and binary import paths do not drift. Linked final
images normally strip `META_INLINE`; library artifacts preserve it for
downstream `rxc` optimisation.

Source/debug metadata now has two separate identities:

- `.srcstep` `step-id` is a module-local id for one concrete source anchor:
  pooled file name, whole source line, active range, and provenance flags. It is
  not an instruction address or a source line number.
- `.srcstep` and `.traceevent` `clause-id` is a module-local grouping key for
  all anchors and semantic events that belong to one logical authored clause.
  Simple clauses often use the same value for both ids; split clauses, generated
  helper code, loop pieces, or inlined fragments may use several step ids for one
  clause id.

`0` means there is no source/clause anchor. In a linked image, consumers should
treat identity as module plus id because ids are only local to their original
module.

The RXAS spellings are:

```rxas
.srcstep <step-id> <clause-id> <flags> "<file>" <line> <start-col> <end-col> "<whole-source-line>"
.traceevent "<kind>" <mode-mask> "<value-source>" "<value-type>" "<register-type>" <value-ref> <source-step-id> <clause-id> <flags> "<symbol>" "<resolved-name>"
```

TRACE event records deliberately store compact codes, not presentation strings.
Current event kinds include `A` assignment, `V` variable read, `C` resolved
compound name, `L` literal, `O` binary operation, `P` prefix operation, `F`
function result, and `M` message. The TRACE exit handler maps those to classic
prefixes such as `>=>`, `>V>`, `>C>`, `>L>`, `>O>`, `>P>`, `>F>`, and `+++`.
The mode mask controls visibility in modes such as Results and Intermediates.
`value-source` is `R` for a register, `K` for a constant-pool value, or `N`
when no value is attached. Register-backed events must name an actual available
register at that address; handlers must not infer values from source text or
scope metadata.

Metadata-only modules are valid. For example, an interface contract file may
compile to `.rxas` containing `.meta` records and no function bodies; `rxas`
must still emit a `.rxbin` so import and runtime factory resolution can load
the contract metadata.

For interface methods, the member-kind string now distinguishes:
- `method` for an abstract interface method
- `method final` for a Level B final/default interface method body

### Binary Literals
RXAS binary literals are written as byte-paired hex with a `0x` or `0X`
prefix. `0x00ff` stores two bytes (`00 ff`) in a `BINARY_CONST`, and `0x`
stores an empty binary constant. `rxdas` emits the canonical lowercase
`0x...` spelling.

`load rDst,0x...` uses `LOAD_REG_BINARY` and loads the VM register's binary
slot, not its string slot. Binary-buffer VM instructions exposed at RXAS are:

- `blen rOut,rBin`
- `getbyte rOut,rBin,rIndex`
- `setbyte rBin,rIndex,rByte`
- `bconcat rDst,rLeft,rRight`
- `bappend rDst,rRight`
- `setbinpos rBin,rOffset`
- `getbinpos rOut,rBin`
- `bslice rDst,rSrc,rLen`
- `bupdate rDst,rOffset,rSrc`
- `stobin rReg`
- `bintos rReg`

Indexes and lengths are bytes and zero-based. `bslice` reads from the source
binary cursor and truncates at end-of-buffer. `setbyte` and `bupdate` are
strict and raise `OUT_OF_RANGE` for invalid indexes, bytes outside `0..255`, or
overlay writes past the destination length. `stobin` copies the register's
current string bytes into its binary slot. `bintos` validates the register's
current binary bytes as UTF-8 and copies them into its string slot; invalid
bytes raise `UNICODE_ERROR` in UTF builds.

Level B exposes these byte-buffer instructions through the `rxfnsb` binary
helpers (`binlength`, `binbyte`, `binsetbyte`, `binsubstr`, `binconcat`,
`binoverlay`, `bininsert`, `bindelstr`, `binpos`, `bincompare`, `bin2x`,
`x2bin`). The source-level `||` operator also lowers to `bconcat` when either
operand is `.binary`; blank concat remains a text-only operation.

### Attribute Array Helpers

RXAS exposes VM attribute-array operations for array/object backing storage:

- `getattrs rOut,rArray`
- `setattrs rArray,rCount` / `setattrs rArray,count`
- `minattrs rArray,rCount` / `minattrs rArray,count`
- `linkattr` / `linkattr1`, `linktoattr` / `linktoattr1`, and
  `unlinkattr` / `unlinkattr1`
- `insattrs rArray,index,count` and `delattrs rArray,index,count`
- `insattrs1 rArray,index,count` and `delattrs1 rArray,index,count`

The forms without a `1` suffix use zero-based indexes. The `*1` forms use
one-based indexes and are the usual fit for Level B array BIFs. Bulk insert
accepts an index in `0..num_attributes` (`1..num_attributes+1` for `*1`) and
inserts before that position. Bulk delete is strict: the requested range must
fit inside the current attributes. Passing a count of zero is a no-op, but the
index still must be in range. Invalid ranges raise `OUT_OF_RANGE`.

### Reference Helpers

RXAS exposes the VM-first reference surface before any Rexx source syntax is
finalized:

- `mkref rRef,rSource` creates a reference value to the storage currently named
  by `rSource`, after existing local links have resolved to their target
  storage.
- `deref rDest,rRef` copies the current referenced value into `rDest`. This is
  a snapshot copy, including nested attributes, not a live alias.
- `linkref rLocal,rRef` links a local register to the referenced storage until
  the existing `unlink rLocal` restores its base storage.
- `setref rRef,rSource` copies `rSource` into the referenced storage.
- `refvalid rOut,rRef` stores `1` when the reference is still valid and `0`
  otherwise.
- `unref rRef` clears a reference value and releases its reference-cell retain.

Invalid reference use raises `REFERENCE_INVALID`; `refvalid` is the non-raising
probe. A reference becomes invalid when its target storage lifetime ends, such
as deleted/shrunk attribute storage or frame-owned local storage after return.
Plain overwrite of still-live storage keeps the reference valid. These
instructions are optimiser barriers until reference alias effects are modelled
more deeply. Assigning a scalar value into the register that holds a reference
value clears that register's reference payload; it does not invalidate the
referenced target storage.

In UTF builds, RXAS string constants are text, not byte containers. Hand-written
string operands are unescaped and validated before entering the constant pool;
invalid UTF-8 is an assembly error with guidance to use `0x...` binary
literals. Use `load rDst,0x...`, `freadb`, `fwriteb`, `sockrecvb`, or
`socksendb` for arbitrary bytes.

### Symbol Tracking (AVL Trees)
To deduplicate constants and resolve identifiers in `O(log N)` time, the assembler leverages a custom AVL tree implementation (`avl_tree.h`). Active trees include:
- `string_constants_tree`
- `decimal_constants_tree`
- `binary_constants_tree`
- `label_constants_tree`
- `proc_constants_tree`
- `extern_constants_tree`

## 3. Two-Pass Resolution (Backpatching)

Assembly requires a two-pass approach because a jump or call can reference a label or procedure defined further down in the file. `rxasassm.c` handles this elegantly by building a `struct backpatching` header for every symbol encountered:

```c
struct backpatching {
    int defined;          // 1 if definition encountered, 0 if only referenced
    size_t index;         // Final resolved target index in the binary or const_pool
    struct backpatching_references *refs; // Linked list of unresolved usages
};

struct backpatching_references {
    size_t index;         // The instruction operand index in the bin_code array
    Assembler_Token *token; 
    struct backpatching_references *link;
};
```

When the `EOF` is reached, the `backptch(Assembler_Context *context)` function orchestrates the final resolution:

1. **`optimise_labels()`**: Performs peephole optimization. If a label simply targets an unconditional branch (`br`), the target of the label is recursively collapsed to point directly at the final destination, saving execution cycles.
2. **`backpatch_procedures()`**: Walks the `proc_constants_tree`. Raises errors for any procedures referenced but never defined. Resolves valid procedures.
3. **`backpatch_labels()`**: Walks the `label_constants_tree`. Updates the placeholder `bin_code` indices mapped in the `refs` linked list with the actual instruction offsets.

## 4. Binary Emission (`.rxbin`)

Once parsing and backpatching conclude without errors, the assembler flushes the `Assembler_Context` state into a packed `.rxbin` file.

The structural output consists of:
1. **Magic Header & Version**: Validates the file format.
2. **Global Counters and Section Sizes**: e.g., Number of global registers (`context->binary.globals`), expanded section sizes, stored section sizes, and section flags.
3. **The Constant Pool**: The assembled `context->binary.const_pool`, optionally blob-compressed on disk if that reduces size.
4. **The Bytecode Stream**: The resolved `context->binary.binary` instruction slots, optionally packed on disk as a logical opcode/operand token stream.

As of format version `002` and later, float operands are no longer stored inline as raw
`double` payloads in operand slots. Instead, the operand slot carries an index
to a deduplicated `FLOAT_CONST` record in the constant pool.

`rxdas` emits float operands and `FLOAT_CONST` pool entries with enough
significant digits to distinguish binary64 values that would otherwise share a
six-decimal rendering, while keeping integer-looking values parseable as RXAS
float tokens (for example, `-0.0` rather than `-0`).

As of the current `003` layout, `rxas` still builds the normal in-memory
`bin_code[]` and raw constant pool first. The section compaction step happens
when `write_module()` serializes the module:

- instruction slots are packed as logical opcode/operand tokens
- signed integer operands are ZigZag encoded in that packed stream
- the constant pool is optionally compressed with the lightweight in-tree LZSS codec
- the file header stores both the stored byte counts and the expanded target sizes

## 5. Current Interface-Dispatch Additions

The current interface runtime slice relies on three assembler-visible opcodes
and the metadata records above:

- `setobjtype rX,"fully.qualified.class"` stamps a newly created object value
  with its concrete runtime class identity
- `srcmethod rProc,rObj,"member"` resolves a concrete procedure from an object
  value plus a member name
- `srcfproc rProc,"fully.qualified.interface",rArgs` resolves the default `*`
  factory provider for an interface
- `srcfproc rProc,"fully.qualified.interface..factory_name",rArgs` resolves a
  named factory provider for an interface
- `typeof rOut,rObj` returns the canonical source type name of an object value
- `istype rOut,rObj,"type"` checks an object value against an interface,
  class, or `.object`
- `asserttype rObj,"type"` raises `CONVERSION_ERROR` if the object value does
  not match the requested interface, class, or `.object`

`srcfproc` now supports both the default `*` surface and named factory
selectors. Provider selection is a VM concern: the assembler simply emits the
opcode and the interface/class metadata needed for runtime lookup.

For `call ... , rArgs`, `dcall`, and `srcfproc ... , rArgs`, the trailing
register operand names the argument-count register. The actual argument values
are taken from the contiguous registers immediately after that count register.
That hidden contiguous argument block is semantically part of the instruction
use set, so optimiser passes must treat those opcodes as barriers unless they
fully model the implicit register consumption.

The peephole optimiser also consumes instruction metadata from
`binutils/include/rxops.h`. `FLOW_JUMP`, `FLOW_COND`, and `FLOW_TERM` block
`NO_HAZARD` rule skips automatically. `FLG_OPT_BARRIER` is for `FLOW_NEXT`
instructions that still must not be skipped, such as calls, signal handler
configuration, explicit signal/check instructions, and opaque argument-block
operations. `FLG_IMPLICIT_REG_USE` is for linear instructions whose register
effects are not represented as normal operands. For example, `inc0`, `dec0`,
`inc1`, and friends are still linear execution, but the optimiser treats them
as using the corresponding fixed local register when checking whether an
intervening instruction is relevant to a rule.

Signal support adds action-aware and dynamic-name forms:

- `sigcalla proc(),"NAME"` installs a handler that returns an internal action
  marker interpreted by the VM as `skip`, `retry`, or `fail`
- `sigpush "NAME"` saves the current handler entry for `NAME` on the current
  frame's handler stack
- `sigpop "NAME"` restores the most recent saved handler entry for `NAME`
- `sigbrv label,rSignal,"NAME"` installs a branch handler that wraps the raw
  interrupt object as a Level B `.signal` value in `rSignal` before branching
- `signal rName` raises a signal whose name is read from a string register
- `signal rName,rPayload` raises a dynamic-name signal with a payload object

Unknown dynamic names raise `INVALID_SIGNAL_CODE`. Literal `signal "NAME"`
forms are still assembled directly against the static signal table.

Optimiser rule operands use lowercase `r` for a captured register. Uppercase
`R`, `G`, and `A` match literal local/global/argument register numbers. The
assembler uses this to express rules such as `inc r0 -> inc0` without adding
mnemonic-specific C code to the optimiser engine.

`typeof`, `istype`, and `asserttype` are object-contract operations. Compiler
generated code uses them for object casts/tests/introspection; scalar
`typeof`/`is` cases are folded earlier by `rxc`.

Interface default methods use that same path. The assembler does not introduce
new opcodes for them; it simply carries `META_MEMBER` kind `method final` and
emits the interface method body as an ordinary procedure. The VM method
registry then decides whether `srcmethod` should bind `class.member` directly
or fall back to the interface's emitted default-body procedure.

At the current Level B stage, the runtime selection rule is:

- each provider row may carry an optional resolved class-side `§match` or
  `§match.member` procedure
- `srcfproc` causes the VM to call that effective `match` on every candidate
  with the same argument list that will later be passed to the selected
  factory
- if no explicit `match` exists, the candidate behaves as if it had an
  implicit `match` returning `1`
- candidates with score `<= 0` are rejected
- the highest positive score wins
- tied highest scores break alphabetically by fully qualified concrete class
  name

## 6. Core Socket Instructions

Core TCP sockets are exposed as normal RXAS mnemonics generated from
`binutils/include/rxops.h`. They use VM-managed integer handles rather than OS
file descriptors or pointers, so programs cannot accidentally retain a native
socket after the VM context is freed. All socket instructions are
`FLG_OPT_BARRIER` because they perform external I/O or mutate context-owned
handle state.

The current instruction surface is intentionally raw TCP with optional client
TLS connect support:

- `socknew rSock`
- `sockclose rRc,rSock`
- `sockconnect rSock,rHost,rPort`
- `sockconnecttls rSock,rHost,rPort`
- `sockbind rSock,rHost,rPort`
- `socklisten rRc,rSock,rBacklog`
- `sockaccept rClient,rServer`
- `sockshutdown rRc,rSock,rHow`
- `sockstarttls rRc,rSock,rHost`
- `socksend rBytes,rSock,rText`
- `socksendb rBytes,rSock,rBin`
- `sockrecv rText,rSock,rMax`
- `sockrecvb rBin,rSock,rMax`
- `sockpending rBytes,rSock`
- `socktimeout rRc,rSock,rMillis`
- `sockblocking rRc,rSock,rFlag`
- `socknodelay rRc,rSock,rFlag`
- `sockkeepalive rRc,rSock,rFlag`
- `sockpeer rText,rSock`
- `socklocal rText,rSock`
- `sockstatus rStatus,rSock`
- `sockerror rText,rSock`

`sockconnect`, `sockconnecttls`, and `sockbind` keep the socket handle in their
first operand and record the result in the handle's status slot; use
`sockstatus` or `sockerror` immediately after those operations. The higher-level
`rxsocket.rexx` wrapper turns these into function return codes for ordinary
Level B code.

`sockconnecttls` is the portable client TLS connect primitive. It connects to
`rHost:rPort`, starts TLS before any application bytes are exchanged, and uses
`rHost` for SNI and certificate name verification. `sockstarttls` remains the
lower-level true STARTTLS primitive for protocols that exchange clear-text bytes
before TLS; backends that cannot upgrade an existing connection in place return
a negative unsupported status instead of reconnecting behind the caller.

Both TLS instructions are present in all builds. When no TLS backend is
compiled in, they record a negative socket status; `sockstarttls` also returns
that code in `rRc`. Backends are selected at CMake configure time. Fresh builds
default to `CREXX_ENABLE_TLS=NETWORK` on Apple platforms,
`CREXX_ENABLE_TLS=OPENSSL` on non-Windows Unix-like platforms, and
`CREXX_ENABLE_TLS=SCHANNEL` on Windows. The Network backend uses
Network.framework, Security.framework, CoreFoundation.framework, and the system
trust store for `sockconnecttls`. The OpenSSL backend supports both direct TLS
connect and true STARTTLS. The SChannel backend uses Windows SChannel/SSPI and
the Windows trust store, and supports both direct TLS connect and true STARTTLS.
