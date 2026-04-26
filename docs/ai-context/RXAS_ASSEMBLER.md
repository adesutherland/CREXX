# cREXX Assembler Architecture (`rxas`)

The `rxas` assembler is responsible for translating human-readable Intermediate Representation (IR) assembly (`.rxas`) into packed executable bytecode (`.rxbin`) consumed by the `rxvm` interpreter.

## 1. Assembler Pipeline

The assembler processes source files through a pipelined, pseudo-two-pass architecture:

1. **Lexical Analysis (`re2c`)**:
   - Source defined in `assembler/rxasscan.re`.
   - Tokenizes input into REXX Assembly primitives: registers (`RREG`, `GREG`, `AREG`), literal types (`STRING`, `INT`, `FLOAT`, `DECIMAL`, `HEX`), symbols (`ID`, `LABEL`, `FUNC`), and assembler directives (e.g., `.locals`, `.globals`, `.expose`, `.meta`).
   - The lexer also recognizes the interface metadata keywords used at the end of
     `.meta` records: `.interface`, `.implements`, and `.member`.

2. **Parsing (`Lemon`)**:
   - Grammar defined in `assembler/rxasgrmr.y`.
   - Enforces the structural integrity of the `.rxas` file (headers, function definitions, variable declarations, and instruction sequences).
   - The parser actions invoke Builder API functions directly (e.g., `rxasgen*`, `rxaslabl`, `rxasproc`), translating syntax rules into buffered internal data structures.
   - Instruction names are derived from `binutils/include/rxops.h`, so new VM
     opcodes become assembler mnemonics through that shared table. Recent core
     runtime additions include the object/interface instructions and the
     `sock*` TCP socket instructions.

3. **In-Memory Buffering & Constant Pooling**:
   - Handled in `assembler/rxasassm.c`.
   - Instructions and operand slots are appended sequentially into a dynamic array of `bin_code` elements.
   - Complex and pooled literal types (Strings, Floats, Decimals, Procedure Headers, Metadata) are deduplicated via AVL Trees and injected into a variable-length **Constant Pool**.

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
Strings, pooled float literals, procedure mappings, debug metadata, and exported symbols are packed into `const_pool`. This is a sequential buffer of dynamically sized records. Every record starts with a `chameleon_constant` header dictating its type and byte size.
Types include: `STRING_CONST`, `FLOAT_CONST`, `PROC_CONST`, `EXPOSE_REG_CONST`, `EXPOSE_PROC_CONST`, `META_FUNC`, `META_REG`, etc.

The serialized `expose_head` chain includes both `EXPOSE_REG_CONST` and
`EXPOSE_PROC_CONST` records. Runtime linking and other module-local walkers now
rely on that chain instead of scanning the whole constant pool.

The interface/callable-contract work extends that same metadata path rather
than introducing a second binary header mechanism. In addition to `META_CLASS`
and `META_ATTR`, the assembler now serializes:
- `META_INTERFACE` for one interface header
- `META_IMPLEMENTS` for one concrete-class-to-interface link
- `META_MEMBER` for one interface method or factory declaration

Metadata-only modules are valid. For example, an interface contract file may
compile to `.rxas` containing `.meta` records and no function bodies; `rxas`
must still emit a `.rxbin` so import and runtime factory resolution can load
the contract metadata.

For interface methods, the member-kind string now distinguishes:
- `method` for an abstract interface method
- `method final` for a Level B final/default interface method body

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

The current instruction surface is intentionally raw TCP:

- `socknew rSock`
- `sockclose rRc,rSock`
- `sockconnect rSock,rHost,rPort`
- `sockbind rSock,rHost,rPort`
- `socklisten rRc,rSock,rBacklog`
- `sockaccept rClient,rServer`
- `sockshutdown rRc,rSock,rHow`
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

`sockconnect` and `sockbind` keep the socket handle in their first operand and
record the result in the handle's status slot; use `sockstatus` or
`sockerror` immediately after those operations. The higher-level
`rxsocket.rexx` wrapper turns these into function return codes for ordinary
Level B code.
