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
   - Instruction operands are parsed as a recursive comma-separated list.
     `rxasquev()` and `rxasgenv()` carry the resulting variable-length token
     vector; there is no three-operand grammar or queue limit.
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

The canonical operand format in `rxops.h` is a NUL-terminated signature: one
kind code (`R`, `I`, `S`, `L`, and so on) per operand. Consumers iterate that
signature instead of switching over a closed `FMT_*` enum. The in-memory
`no_ops` field remains a 32-bit `int`, preserving the eight-byte `bin_code`
cell; the practical operand-count bound is therefore the bytecode stream's
existing `INT_MAX`, not a small instruction-format ceiling.

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
.meta "fully.qualified.callable"=".inline" "I6;c,1,..."
```

The `I6` payload is the compiler-owned inline transport described in
`compiler/docs/inlining_design.md`. `rxas` stores it as `META_INLINE`, and
`rxdas` must emit it back to the same logical `.meta ... ".inline" "I6;..."`
spelling so source, RXAS, and binary import paths do not drift. Linked final
images normally strip `META_INLINE`; library artifacts preserve it for
downstream `rxc` optimisation. The leading `c` record is a versioned callable
proof summary; `rxc` reconstructs its facts from the transported body and
requires exact agreement before enabling imported inlining.

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

### Jump Tables

Procedure-local `.jtable` declarations and same-line `.jcase` label decorations
are resolved after label optimization. The assembler emits the resulting table
as a host-layout-independent `BINARY_CONST`; `jumps`, `jumpr`, `jumpn`,
`jumpb`, `jumpbs`, and `jumpi` operands are patched to that pool entry.
`jumps` uses exact bytes, `jumpr` canonicalizes trailing-space-padded Rexx text,
and `jumpn` canonicalizes numeric strings through the shared parser in
`binutils/include/rxnumparse.h`. Release 1 packed algorithms are
`linear`, `openhash`, and `acph`. `auto` uses linear for one case. Larger tables
use open hashing for average key lengths up to two bytes; tables of at least 256
cases also use it for averages up to four bytes. Remaining tables use ACPH.
Shared format constants and hash functions live in
`binutils/include/rxjtable.h` and must remain common to the assembler and VM
readers.

The complete packed layout, canonicalization, corruption, disassembly, and
ownership contract is in [RXBIN_JUMP_TABLES.md](RXBIN_JUMP_TABLES.md).

### Binary Literals

RXAS binary literals are written as byte-paired hex with a `0x` or `0X`
prefix. `0x00ff` stores two bytes (`00 ff`) in a `BINARY_CONST`, and `0x`
stores an empty binary constant. `rxdas` emits the canonical lowercase
`0x...` spelling.

`load rDst,0x...` uses `LOAD_REG_BINARY` and loads the VM register's binary
slot, not its string slot. Binary-memory VM instructions exposed at RXAS are:

- `blen rOut,rBin`
- `blen rOut,0x...`
- `bresize rBin,rLength`
- `bclear rBin`
- `bfill rBin,rByte`
- `getbyte rOut,rBin,rIndex`
- `setbyte rBin,rIndex,rByte`
- `bcopy rDst,rSrc`
- `bcopy rDst,rSrc,rOffset`
- `bcopy rDst,0x...,rOffset`
- `bgetu8 rOut,rBin,rOffset`
- `bgeti8 rOut,rBin,rOffset`
- `bgetu16 rOut,rBin,rOffset`
- `bgeti16 rOut,rBin,rOffset`
- `bgetu32 rOut,rBin,rOffset`
- `bgeti32 rOut,rBin,rOffset`
- `bgeti64 rOut,rBin,rOffset`
- `bgetf32 rOut,rBin,rOffset`
- `bgetf64 rOut,rBin,rOffset`
- `bgetu8 rOut,0x...,rOffset`
- `bgeti8 rOut,0x...,rOffset`
- `bgetu16 rOut,0x...,rOffset`
- `bgeti16 rOut,0x...,rOffset`
- `bgetu32 rOut,0x...,rOffset`
- `bgeti32 rOut,0x...,rOffset`
- `bgeti64 rOut,0x...,rOffset`
- `bgetf32 rOut,0x...,rOffset`
- `bgetf64 rOut,0x...,rOffset`
- `bsetu8 rBin,rOffset,rValue`
- `bseti8 rBin,rOffset,rValue`
- `bsetu16 rBin,rOffset,rValue`
- `bseti16 rBin,rOffset,rValue`
- `bsetu32 rBin,rOffset,rValue`
- `bseti32 rBin,rOffset,rValue`
- `bseti64 rBin,rOffset,rValue`
- `bsetf32 rBin,rOffset,rFloat`
- `bsetf64 rBin,rOffset,rFloat`
- `bcheckrange rBin,rOffset,rLen`
- `bgets rString,rBin,rOffset`
- `bgets rString,0x...,rOffset`
- `bsets rBin,rOffset,rString`
- `bsets rBin,rOffset,"literal"`
- `sget rString,"literal",rOffset`
- `bmove rDst,rSrc,rLen`
- `bmemmove rBin,rDstOffset,rLen`
- `bcmpb rCmp,rBin,rNeedle`
- `bcmpb rCmp,0x...,rNeedle`
- `bcmpb rCmp,rBin,0x...`
- `bcmpb rCmp,0x...,0x...`
- `bcmps rCmp,rBin,rString`
- `bcmps rCmp,rBin,"literal"`
- `bcmps rCmp,0x...,rString`
- `bcmps rCmp,0x...,"literal"`
- `bconcat rDst,rLeft,rRight`
- `bappend rDst,rRight`
- `setbinpos rBin,rOffset`
- `getbinpos rOut,rBin`
- `bslice rDst,rSrc,rLen`
- `bupdate rDst,rOffset,rSrc`
- `stobin rReg`
- `bintos rReg`

Indexes, offsets, and lengths are bytes and zero-based. The typed `bget*` and
`bset*` instructions use canonical little-endian storage order; they do not use
host-native struct layout, alignment, or padding. `bgetu8` is the strict
counterpart to current `getbyte`: `getbyte` still returns `-1` for
out-of-range reads, while typed reads raise `OUT_OF_RANGE`. Typed writes raise
`OUT_OF_RANGE` when the field does not fit or when an integer value is outside
the target storage type's range. `bgeti64`/`bseti64` are the Release 1 `.int`
storage form. `bgetf32`/`bsetf32` store IEEE binary32 values and widen/narrow
through the VM float register; `bgetf64`/`bsetf64` store IEEE binary64 values.

`bresize` sets the logical byte length and zero-fills newly exposed bytes. The
VM may keep a larger private physical capacity, grown in blocks and reused
across later logical resizes; `blen` reports only the logical length. Negative
lengths raise `OUT_OF_RANGE`, and allocation failure raises `FAILURE`.
`bclear` sets the logical byte length and cursor to zero. `bfill` fills the
current logical byte range and raises `OUT_OF_RANGE` for bytes outside
`0..255`.

`bcopy rDst,rSrc,rOffset` copies exactly the current logical length of `rDst`
from a binary register or constant source at a byte offset. `bcheckrange` is a
strict side-effect-free range check for `rOffset..rOffset + rLen`; negative
offsets, negative lengths, and ranges past the logical binary length raise
`OUT_OF_RANGE`. `bgets`/`bsets` read and write zero-terminated UTF-8 fields in
binary memory, including the stored NUL on writes but excluding it from the
Rexx string value on reads. `sget` extracts a codepoint-counted slice from a
`STRING_CONST`, starting at a byte offset that must be a UTF-8 boundary.
`bcmpb` and `bcmps` compare without materializing a temporary copy; the compare
register supplies the input source offset and is overwritten with `-1`, `0`, or
`1`.

`setbinpos`, `getbinpos`, and `bslice` are legacy cursor instructions retained
for compatibility and covered by `rxasbinarylegacytests`. New Release 1 source
lowering uses target-sized `bcopy`, typed reads/writes, and zero-copy compares
instead of cursor-based extraction. `setbyte` and `bupdate` are strict and
raise `OUT_OF_RANGE` for invalid indexes, bytes outside `0..255`, or overlay
writes past the destination length. `stobin` copies the register's current
string bytes into its binary slot. `bintos` validates the register's current
binary bytes as UTF-8 and copies them into its string slot; invalid bytes raise
`UNICODE_ERROR` in UTF builds.

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

Once parsing and backpatching conclude without errors, the assembler flushes the `Assembler_Context` state into an RXBIN 007 container.

The structural output consists of:

1. **Magic Header & Version**: Validates the file format.

2. **Module Directory and Section Sizes**: the module identity, global-register count (`context->binary.globals`), six fixed section ranges, and exact stored sizes.

3. **Portable Constants and Metadata**: the native in-memory pool is converted to fixed-width, little-endian, ID-linked records.

4. **Bytecode and Semantic Graph**: the resolved instruction slots are variable-integer encoded, constant operands become record IDs, graph-bearing operands become dense graph IDs, and graph facts/indexes occupy their own sections.

As of format version `002` and later, float operands are no longer stored inline as raw
`double` payloads in operand slots. Instead, the operand slot carries an index
to a deduplicated `FLOAT_CONST` record in the constant pool.

`rxdas` emits float operands and `FLOAT_CONST` pool entries with enough
significant digits to distinguish binary64 values that would otherwise share a
six-decimal rendering, while keeping integer-looking values parseable as RXAS
float tokens (for example, `-0.0` rather than `-0`).

RXAS still builds the normal in-memory `bin_code[]` and raw constant pool first.
`write_module()` then emits only 007: signed integers use ZigZag variable
integers, portable records replace native pool layouts, and the base sections
are uncompressed. There is no legacy-output mode; 006 is rejected and repository
artifacts are rebuilt. The 007 container and semantic graph are specified in
[RXBIN_007_SEMANTIC_GRAPH.md](RXBIN_007_SEMANTIC_GRAPH.md).

The fixed direct-bytecode call forms `call1` through `call4` name each caller
argument register explicitly. RXAS sets `RXBIN007_FEATURE_FIXED_CALLS` whenever
one is present; `call` with a count register remains the general fallback for
higher arity, imported/native and dynamically selected targets. The existing
two-operand `call result,procedure()` remains the zero-argument direct form.

RXAS feeds the common `rxbin` graph builder from class, interface,
implements, member, callable, factory, and signature facts. After ordinary
backpatching it resolves local procedure references, finalizes the indexed
module graph, and emits the same sectioned 007 container used for linked images,
with a one-entry module directory. Unknown external type/member references are
complete opaque nodes, not dangling strings.

Parameter and return types remain canonical text on type nodes, including
preseeded built-in nodes such as `.float`; signatures refer to those nodes with
module-local dense IDs. Type-, member-, and factory-bearing serialized
instruction operands also use module-local graph IDs. RXAS/RXDAS text syntax
remains human-readable and descriptor based.

## 5. Current Interface-Dispatch Additions

The current interface runtime slice relies on three assembler-visible opcodes
and the metadata records above:

- `setobjtype rX,"fully.qualified.class"` stamps a newly created object value
  with its concrete runtime class identity and clears any uninitialized-object
  marker
- `setobjuninit rX,"fully.qualified.class"` writes a typed object placeholder
  and marks it uninitialized; the compiler uses this for bare object class
  defaults such as `.SomeClass`
- `assertinitialized rX` raises `OBJECT_NOT_INITIALIZED` if `rX` is a typed
  uninitialized object value
- `isinitialized rOut,rX` stores `0` for a typed uninitialized object and `1`
  otherwise
- `srcmethodsel rProc,rObj,"rxsig1|member|return_type|args"` resolves a
  concrete procedure from an object value plus a method descriptor
- `srcfprocsel rProc,"rxsig1|fully.qualified.interface|return_type|args",rArgs`
  resolves the default `*` factory provider for an interface
- `srcfprocsel rProc,"rxsig1|fully.qualified.interface..factory_name|return_type|args",rArgs`
  resolves a named factory provider for an interface
- `typeof rOut,rObj` returns the canonical source type name of an object value
- `istype rOut,rObj,"type"` checks an object value against an interface,
  class, or `.object`
- `asserttype rObj,"type"` raises `CONVERSION_ERROR` if the object value does
  not match the requested interface, class, or `.object`

`istype` and `asserttype` check only object/class/interface compatibility.
Initialization is a separate lifecycle check. `assertinitialized` is the
raising check; `isinitialized` is the non-raising probe.

`srcfprocsel` supports both the default `*` surface and named factory
selectors. Provider selection is a VM concern: the assembler simply emits the
opcode and the interface/class metadata needed for runtime lookup. The selector
string is a callable descriptor (`rxsig1|name|return_type|args`), which lets the
VM and linker reject providers whose metadata has the right name but the wrong
signature.

For `call ... , rArgs`, `dcall`, and `srcfprocsel ... , rArgs`, the trailing
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

The opcode metadata inventory is split deliberately without duplicating facts.
`rxops.h` remains authoritative for the dense opcode list, operand format,
control flow, opcode flags and source-mnemonic status.
`binutils/include/rxopeffects.h` is the one-to-one semantic sidecar: it has one
ordered entry for every opcode slot, including reserved and internal slots.
`binutils/include/rxopsignals.h` is a second one-to-one sidecar for signal
capability, failure phase, signal-name source, failure-visible writes,
dependencies, continuations and observability properties. The former coarse
`RXOP_SEM_MAY_THROW` flag has been retired; generic effects no longer duplicate
or approximate signal behavior. Handler-policy instructions additionally name
their normal-path action (`ignore`, halt, branch/call/return, breakpoint,
push/pop), exact static or literal signal-name source and source operand. An
unknown signal set is distinct from an unknown policy write: a known call may
have conservative signal behaviour without changing its caller's handler
table.
`binutils/rxopmeta.c` combines both sources through the stable
`rxop_effects()` and `rxop_signal_contract()` C APIs; their count functions
expose the mechanically checked inventory sizes.

`RxOpEffects` uses the following guarantees:

- `state` is `CLASSIFIED`, `CONSERVATIVE`, `RESERVED`, or `INTERNAL`.
  Conservative, reserved, internal and unknown/out-of-range queries are
  optimizer barriers and never claim a kill.
- `reads` and `writes` are conservative possible-access sets over explicit
  operand positions. Existing instructions retain the compact three-bit masks;
  wider instructions use one-bit-per-operand signature strings. Consumers use
  `rxop_effect_reads_operand()` and `rxop_effect_writes_operand()` rather than
  interpreting either representation directly. A consumer must account for
  two positions naming the same physical register.
- `kills` is a proof set: every named operand is definitely overwritten
  without reading its previous value. It is always a subset of `writes` and is
  deliberately narrower than possible writes.
- `implicit` covers fixed local-register read/modify/write, integer-coded local
  copy/target registers, argument-register indexes and the runtime-sized local
  range after operand 3 used by `call`, `dcall`, and `srcfprocsel`.
- `branch_targets` identifies explicit label operands through the same generic
  query representation. Packed jump-table
  instructions instead carry `RXOP_SEM_INDIRECT_BRANCH`.
- `semantics` covers direct and dynamic call boundaries, returns, alias
  creation/release, reference creation/read/write/release, storage-lifetime
  end, indirect writes, indirect branches and opaque side effects. Signal
  behavior comes only from `RxOpSignalContract`.
- `flow` and `optimizer_barrier` are returned in the same object but are
  derived from the canonical `rxops.h` flow/flags. A non-classified state also
  forces the returned barrier bit.

The current inventories each have 650 entries: 591 source opcodes (585
classified and six explicitly conservative process/redirect operations), 56
reserved slots and three internal handlers. Coverage is not inferred from an instruction name
or format. The audit used the VM handlers between `START_OF_INSTRUCTIONS` and
`END_OF_INSTRUCTIONS`, the assembler/compiler behavior described here, and
focused semantic tests. The tests mechanically validate table alignment,
operand-mask legality, flow/branch/call/implicit consistency, source coverage,
reserved/internal treatment and fail-closed unknown queries; they do not claim
to parse arbitrary C handler bodies formally.

NR-27 adds a transient whole-procedure machine-flow layer after the unchanged
20-item local peephole and before ordinary RXBIN emission. Stable peephole
output is moved, with its token ownership, into a growable procedure stream.
The assembler then builds resolved label successors/predecessors, reachability,
typed-view liveness and transformation-specific must-availability/may-reach
facts over the original queue records. Surviving records still pass through the
same assembler emission functions; RXAS syntax, canonical RXBIN and the public
ABI do not change.

PERF3-11 now also constructs an immutable `RxasFlowProcedure` sidecar after
the existing rewrites reach their fixed point. `assembler/rxas_flow_graph.c`
owns stable record, instruction, basic-block and typed-edge descriptors for one
procedure epoch. It does not mutate tokens or queued records and no rewrite
consumer uses it yet. The original queue index remains the record identity;
every opcode record maps to a stable instruction ID, block ID and exact
pre-emission RXBIN address. Source-step, TRACE, label and metadata records keep
their own IDs and map to the block/address at which they are observed.

Code blocks start at procedure entry, labels, branch and handler targets, and
after calls, signalling instructions and other control terminators. Seven
stable synthetic blocks represent entry, handler dispatch, asynchronous
handler entry, normal return, unwind, terminal exit and unresolved control.
Edges distinguish normal fallthrough, branch, signal skip, signal retry,
handler, unwind, terminal and explicit fail-closed unknown flow. Signal edges
come from `RxOpSignalContract.continuations`; handler-policy instructions with
label operands feed the handler and asynchronous roots rather than inventing a
normal branch from the registration instruction.

Construction first snapshots record/opcode metadata, builds an open-addressed
label index, forms blocks, then appends edges. It is expected linear in queued
records plus emitted edges; it does not rescan all existing edges when adding a
new one. While the legacy rewrite graph remains, orchestration hands its final
resolved `OpInfo` pointers to the sidecar before freeing the old graph; this
avoids a second opcode-table parse and avoids overlapping the two graph memory
images. Read APIs require the owning epoch and return no descriptor for a
stale epoch. `rxas -d` prints deterministic `PERF3 flow-*` records without
pointer values, so graph identities and typed edge counts can be compared
mechanically. Allocation or construction failure simply disables this
consumer-free sidecar and leaves the unchanged emission path available.

Stage 3 layers a demand-driven structural-analysis manager over each immutable
procedure epoch. `assembler/rxas_flow_analysis.c` retains successor edge IDs
and unique predecessor block sets in sparse CSR form, then computes reachable
reverse postorder from a virtual root joining the procedure-entry, registered-
handler and asynchronous-handler roots. Unreachable blocks remain represented
by the graph but are excluded from dominance, SCC and loop proofs.

The cached structural result contains Cooper-Harvey-Kennedy immediate
dominators, dominance-tree intervals, sparse dominance frontiers, iterative
Kosaraju SCCs, dominance-classified backedges and a loop hierarchy. Natural
loops with the same header share one region; irreducible SCCs remain explicit
conservative regions. Loops formed solely by signal-retry continuations carry
`RXAS_FLOW_LOOP_SIGNAL_RETRY_ONLY`, so a later language-loop transformation
cannot mistake retry control for source iteration. Source order remains a
diagnostic mapping and is never treated as dominance evidence.

Every structural query requires the graph epoch. The first successful result
is cached until graph destruction; a failed deliberately small work budget may
be retried with a larger budget. Allocation failure, invalid control or budget
exhaustion leaves the analysis unavailable and cannot enable a rewrite.
Deterministic `PERF3 flow-analysis*` and `PERF3 flow-loop` diagnostics expose
work, retained-byte, reachability, predecessor, dominator-iteration, frontier,
SCC, backedge and loop counters. Post-dominance is deliberately deferred until
a consumer needs a must-execute query. Ordinary assembly does not solve this
unused analysis: `rxas -d`, tests and future optimizer consumers request it
explicitly. Stage 3 therefore changes neither queued records nor RXBIN output.

Stage 4 adds `assembler/rxas_flow_signal.c`, a second demand-driven epoch
cache layered on the structural result. Handler policy is modeled as a sparse
write-once chain with entry, write, phi and clobber definitions. Procedure
entry is an explicit inherited-unknown policy parameter. Static/literal
handler installation produces an exact named action; unresolved opcodes
clobber the whole policy. Parallel normal and signal-skip edges remain distinct
phi inputs even when Stage 3 correctly reports one unique predecessor block.
Cyclic phi queries defer the backedge identity and merge external definitions,
which preserves an unchanged loop policy without using source order as proof.

Each signal edge selects policy from the instruction's recorded failure phase:
before-write edges see the incoming policy, after-write edges see the normal
transfer, and partial/unknown policy writes fail closed. Return and unwind
edges expose the procedure-entry policy parameter because handler-table
changes are local to the callee frame. The VM initially shares the caller's
handler table and copies it on a child-frame write. This frame-local policy
rule must not be confused with call arguments: call-window argument slots are
pointers to caller-owned value storage, so callee writes and reference effects
can remain caller-visible after return.

`sigpush` leaves the active handler unchanged, but the VM may silently fail to
allocate its saved stack entry. A later `sigpop` therefore yields explicit
stack-unknown policy unless a future proof establishes a successful matching
push; the analysis never assumes restoration merely from lexical pairing.

Numeric context, plugin, locale, external state, reference-visible state,
TRACE and call boundaries use separate sparse effect identities. Calls advance
the call/reference/external/plugin/locale identities while preserving handler
policy. TRACE records advance only TRACE identity. Before/after/partial signal
edges select the corresponding effect versions; an unproved partial effect
gets an explicit unknown definition rather than one global barrier.
`rxas_flow_policy_at_instruction()`, `rxas_flow_policy_on_edge()`,
`rxas_flow_effect_at_instruction()` and `rxas_flow_effect_on_edge()` are the
initial narrow query surface. A deliberately small budget, stale epoch,
allocation failure or malformed graph returns no usable result. Ordinary
assembly does not request the cache; tests, `rxas -d` and future consumers do.
Stage 4 is output-neutral and preserves exact canonical RXBIN images.

Stage 5 adds `assembler/rxas_flow_ssa.c`, a third demand-driven epoch cache
over the same immutable procedure.  It separates a register name from the
storage currently named by that register.  Local, argument and global
registers begin with distinct base `StorageId` definitions; `link`, `linkarg`,
attribute/reference links, `swap`, their fused forms and `unlink` then create,
move or restore symbolic storage identities.  In particular, `linkarg` names
caller-owned argument storage rather than inventing callee-local value state.
Unknown aliases and unsupported indirect mutation remain explicit unknowns.
Calls preserve mappings that are structurally unchanged but advance the Stage
4 reference/effect identities needed to prevent a false unchanged-value proof.
For a fused opcode that both remaps registers and writes values, the mapping is
retained but component values fail closed until canonical metadata describes
the order of its intra-instruction sub-events; the analysis does not guess
whether the write named the pre- or post-remap storage.

Each storage component is represented by a write-once `ValueId`.  The tracked
set is integer, float, string, decimal, binary, attributes, reference and
host-owned native payload.  Values
distinguish procedure-entry, direct write, constant, copy, derived, known
absent, phi and unknown definitions; a null/absent value is not the same as an
unavailable proof.  Copy propagation therefore carries presence as well as
the source identity, including a `dcopy` of a null decimal.  Join values and
storage mappings are materialized lazily and cyclic phis canonicalize an
unchanged loop identity without using source order.  An unknown storage input
has a separate value identity and cannot collide with a valid zero-based
`ValueId` or simplify a join into a false proof.

Normal instruction states consume the canonical component read/write and
derivation metadata.  Signal skip, retry, handler and unwind edges instead
select the Stage 1 failure-visible writes and the Stage 4 edge state.  Derived
`itos`, `ftos` and `dtos` string values name their integer, float or decimal
source `ValueId` and the exact numeric/plugin/effect identities on which the
operation depends.  Derivation metadata also names the source operand, so a
two-register conversion such as `itof rTarget,rSource` does not accidentally
use the destination's old value as its proof source.  This is
storage/component infrastructure only: no Stage 5 consumer changes queued
records or emitted RXBIN.

Persistent states retain only mapping and component definitions.  Point
queries walk and cache the required storage/value chain; they do not allocate
a `nodes x registers x components` table.  `rxas -d` deliberately materializes
only actual derivation sites and prints deterministic `PERF3 flow-ssa*`
counters, so diagnostics exercise the first proof candidates without forcing
every possible point/component query.  Work-budget exhaustion, allocation
failure, a stale epoch or invalid control returns no usable analysis.  Ordinary
consumer-free assembly does not request this cache.

Stage 6 adds `assembler/rxas_flow_proof.c`, a fourth per-epoch cache and the
only authority for migrated proof consumers.  It exposes bounded queries for
dominated successful repetition, equivalent scalar-constant writes,
speculatability, loop must-execute and loop component invariance.  Proof keys
name symbolic `StorageId`s; result records
name the generator/candidate `ValueId`s, effect identities and a stable failure
reason.  Value and effect phis are compared through conservative reduction and
equivalent leaf sets rather than numeric identity or source order.  Stale
epochs, invalid control, unsupported signal dependencies, allocation failure
and work-budget exhaustion return an unavailable or rejected proof and cannot
enable a rewrite.

The first migrated authority was repeated one-register `itos`.  A deletion
requires the generator's successful continuation to dominate the candidate,
the same storage and equivalent integer source/string result, and equivalent
numeric-context and reference-visible effect dependencies.  The old private
per-generator availability solver has been removed.  The new service is
allowed to prove a larger safe domain; the old solver's decisions are a
retained minimum-capability baseline, not a parity oracle.

Calls model caller-owned argument mutation without becoming a universal local
barrier.  `call1` through `call4` create unknown component definitions for
their explicit actual arguments.  A range call records its local call-window
base and uses a statically constant count when available; an unknown count
conservatively covers every later local.  Storage outside the argument window
may remain provably unchanged.  Normal and failure continuations use the same
argument exposure rule, so signal edges cannot recover a value the callee may
have changed.

The NR-27 register universe distinguishes local (`r`), argument (`a`) and
global (`g`) register classes and tracks integer, float, string, decimal,
binary, attribute and reference views. Explicit and implicit accesses come
from `rxop_effects()`. Register metadata and register-backed TRACE records are
observations. Alias/reference/lifetime/indirect/opaque operations taint involved
registers and invalidate the relevant facts. Legacy consumers still treat
calls and other modeled barriers conservatively; migrated component-SSA
consumers may retain facts only for storage proved outside the call argument
window and unaffected by reference-visible effects.
Registered branch SIGNAL handlers are conservative asynchronous observation
targets for every executable instruction in the procedure. An unresolved
label or unknown opcode makes control flow incomplete and disables all NR-27
rewrites for that procedure. A packed jump-table branch is complete only when
its procedure-local declared table, every `.jcase` label and the mandatory miss
fallthrough are all present in the same retained procedure stream. The flow
graph then includes every case edge plus the miss edge. Undeclared, malformed,
cross-procedure, unsupported or off-graph-miss tables remain fail-closed.

Component information is a canonical opcode-metadata service rather than an
NR-27-local mnemonic switch. `rxop_component_reads()` and
`rxop_component_writes()` distinguish integer, float, string, decimal, binary,
attribute, reference and native-payload components for each explicit operand.
`rxop_component_clears()` records components made provably absent by a write.
For example, integer and float constant loads clear both reference and native
payloads; `bcopy` carries binary and native payload together.
`rxop_value_derivation()` identifies proved representation relationships;
`rxop_derivation_context_reads()` and `rxop_context_writes()` make numeric
context part of those facts. `rxop_signal_contract()` distinguishes proven
non-signal, known signal and fail-closed unknown behavior and records
pre-write, post-write, partial-write or unknown failure phases. Unproved
opcodes and signal locations remain conservative. The contract inventory marks
the VM `concat`/`sconcat` family non-signalling, stem writes failure-atomic
before writes, and decimal comparisons plugin-signalling after their integer
result write. Alias-topology effects are independently versioned from mutation
observable through an existing reference. An unknown exceptional signal set
does not invent a normal numeric-context write for an otherwise classified
opcode. A register-backed TRACE event observes only the component named by its
value type; the event remains present and does not automatically observe every
representation inside the value.

The graph-owned storage-identity service follows direct `link`, `swap` and
`unlink` mapping rather than equating raw register numbers. The first
component consumer uses this identity to remove a later one-register `itos`
only when an earlier `itos` for the same storage proves both the unchanged
integer source and unchanged string result under the same numeric context on
every incoming normal path. Relevant component writes, context changes,
ambiguous/tainted references, caller-owned argument mutation, opaque or
indirect effects and unproved signal phases reject the proof. An unrelated
no-argument call does not invent a local-value write. TRACE records are
retained; ordered same-boundary delivery lets the producer event survive even
when the redundant executable conversion is removed. The proof service is
generic: after `itos`, M01 deleted the old repeated-`itof` authority and made a
metadata-driven one-register `xtoy` consumer its successor. All 20 such
conversions now name their exact source component, result component,
derivation and context dependencies in canonical metadata; the consumer has
no conversion-mnemonic allow-list. The proof admits only a dominating
successful repetition with equivalent component values and effects.

The first larger-domain case is `itod`. Both bundled decimal plugins implement
integer-to-decimal conversion for every integer, with allocation failure
handled by the VM panic convention. `itod` and the same-function `btod`
therefore clear stale backend diagnostics and are total non-signalling
operations; their values still depend on numeric context and plugin identity.
Signalling conversion families remain rejected where the first successful
continuation does not dominate the repetition. `btoi` and `itob` remain
rejected pending a distinct proof that repeating their same-component
normalization does not change the value. Later loop hoisting remains a
separate consumer.

M02 migrates repeated integer and exact-bit float constant writes.  The legacy
raw-register availability solver is deleted.  A candidate is removable only
when its before/after `StorageId` is unchanged, every reachable scalar
`ValueId` leaf equals the candidate constant, and reference/native-payload
components are already absent on every path.  This last condition preserves
the VM assignment side effects that release a reference and finalize
host-owned binary state.  Equal-value phis, direct link/swap mappings and
ordered TRACE paths can therefore prove even though raw register or source
order differs; different phis, signed-zero bit changes and hidden cleanup fail
closed.  A cheap syntactic demand filter only decides whether to ask the proof
service and never authorizes deletion.  Migrated consumers share one immutable
proof session per fixed-point epoch, and retained-byte metrics are refreshed
after lazy query materialization.

M03 migrates repeated `NULL`.  The legacy raw-register all-view availability
solver and its blanket TRACE guards are deleted.  A candidate must be an exact
classified non-signalling `NULL`, its pre-instruction target must resolve to a
known `StorageId`, and every reaching leaf of all eight tracked components
must already be `ABSENT`.  This proves that deletion skips no scalar/payload
destruction, reference release or native-payload finalization.  Distinct
write-once absent leaves may agree through a phi, direct aliases follow
storage identity, and explicit TRACE records remain ordered observations.
An intervening component write or unknown storage/value leaf fails closed.
The proof deliberately uses pre-instruction storage: VM `NULL` clears the
value reached by a register but does not change the register's link topology.

M04 migrates exact same-storage copies. The canonical
`rxop_same_storage_copy_is_noop()` contract covers `copy`, `icopy`,
`fcopy`, `scopy`, `dcopy`, `acopy` and `bcopy`: when both operands
denote the same physical value storage, the VM path performs no write,
allocation, signal or cursor/effect update. This conditional contract is more
exact than the opcode's general signal contract; different-storage `bcopy`
remains conservative. Identical physical register operands prove the old floor
directly. Distinct registers require equal pre-instruction `StorageId`, or
storage phis with the same unique write-once leaf. LINK-established and
agreeing-phi aliases can therefore prove, while divergent, different or unknown
storage fails closed. Explicit TRACE records remain; an event after a
semantically empty copy does not make that copy observable.

The admitted first transformation panel is deliberately narrower than the
fact engine:

- `icopy`, `fcopy` and strict-comparison uses of `scopy` may be redirected only
  when the typed source view is unchanged on every path and every may-reaching
  destination observation can be redirected atomically; decimal copy remains
  excluded from that established consumer pending decimal-lifetime proof,
  although `dcopy` itself is now a proven total non-signalling operation;
- all seven exact same-storage copy families use the M04 conditional metadata
  and StorageId proof, while nonidentity full-value copies remain excluded
  pending ownership, payload, attribute, flag and cleanup proof;
- repeated integer/bitwise-equal float constants use the M02 storage/value and
  hidden-cleanup proof; repeated `null` uses the M03 known-storage and
  all-component-absence proof;
- repeated metadata-admitted one-register XTOY derivations use the generic
  dominated-success repetition proof;
- a repeated one-register `itos` is removed only under the storage-identity,
  integer/string-component and numeric-context proof described above;
- unreachable instructions, TRACE records and source-step records are removed
  only when the entire procedure CFG is complete; and
- dead-result deletion remains disabled because a nominal numeric destination
  write may release hidden reference or native-payload state even when its
  numeric result is dead.

Every admitted rewrite removes at least one executable instruction and inserts
none, so fixed-point iteration terminates by a strictly decreasing instruction
count. `rxas -d` prints per-candidate accept/reject reasons and a per-procedure
summary. `-n` bypasses both the local and whole-procedure optimizers.

NR-18 adds one reverse direction for surviving `icopy`/`fcopy` records. For an
immediately adjacent, classified, nonthrowing, single-kill producer, the pass
may retarget operand 1 from a disposable local temporary to the copy's final
local and delete the copy. The written typed view must be exact; the final view
must be dead at the producer boundary; the temporary view must be dead after
the copy; the producer must not read the final; and TRACE/source-step, implicit,
alias/reference/lifetime, opaque, ownership and asynchronous-handler
observations reject the candidate. These conditions make the original and
retargeted states equal at every observable edge while reducing the executable
count by one.

Copy rewrites proved from one graph may be applied in the same iteration only
when their complete physical source/destination register pairs are disjoint.
Their substitutions then commute and cannot invalidate one another's liveness
or availability facts. Procedures newly admitted through exact jump-table
edges always receive reachability cleanup. Global typed-value analysis is
additionally bounded to one million `queue-item x liveness-word` cells for
those procedures; above the bound, `rxas -d` reports `scope=reachability-only`.
This preserves the linear high-yield cleanup without turning large generated
dispatch procedures into quadratic assembly work. Procedures without resolved
indirect tables retain the established NR-27 behavior without that bound.

The older compare/branch rule still reads explicit read/kill and implicit
register facts through `rxop_effects()` inside the bounded local queue. NR-27
does not change that rule's selection boundary. `test_rxop_metadata` is
generated from `op_table` and both complete sidecars so instructions cannot be
added without mechanically aligned effect and signal entries. The signal
sidecar records decimal literal load as plugin-signalling and decimal copy as
total/non-signalling, while one- and two-register integer-to-float conversion
and float comparison are also non-signalling in the current VM handlers.

Attribute/register-view cleanup is also a keyhole concern. A full `copy`
already copies the VM value status word, so `copy rA,rB` followed immediately
by `acopy rA,rB` is reduced to the full copy for hand-written or legacy RXAS.
Compiler-generated RXAS should not emit that pair for typed payload access. A
future explicit flag-copy operation would use `acopy`, but there is no current
compiler use case. Typed payload copies are `icopy`, `fcopy`, `scopy`, `dcopy`,
and `bcopy`; `bcopy` copies only the binary payload and byte cursor, not
public/compiler/library status flags. Duplicate `link`/typed-copy/`unlink`
reads, and equivalent `linkattr1` reads for the same owner and one-based
attribute slot, may reuse the first detached copy when no barrier or
mapped-register use intervenes.

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

Legacy rules retain three inline operand pairs for source compatibility. New
superinstruction rules select a variable-length operand-pattern side table;
matching and output generation iterate the whole pattern, and captured values
are stored in a dynamically grown map rather than ten fixed slots. The wide
`cnop` regression exercises nine captured input and output operands through
this path.

Opcode arity is likewise defined by the complete format string rather than a
three-operand ceiling. RXAS parsing, RXBIN writing and reading, disassembly,
link relocation, VM decoding, metadata inspection, compiler emission, and the
`rxc` `ASSEMBLE` path all iterate that declared operand sequence. The NR-09
large-instruction batch uses forms up to eight operands; adding another arity
does not require a new fixed-width transport structure.

The NR-09 fused instructions are catalogued in
`docs/reference/rxas/instructions/12-large-instructions.md`. They preserve the
ordered state changes and signals of their component sequence, including an
intermediate write where that write is part of the public instruction contract.
Compiler-owned rewrites may instead choose a result-only form when the removed
temporary has no source-level observation.

The NR-14 frozen-PARSE instructions are catalogued on the same reference page.
`parsewords3`, `parsepos2`, and `parsewords3d` encode complete exact plans in
their opcode identities; `parseplan` carries a compiler-generated versioned
descriptor in an ordinary string-constant operand. RXBIN 007 writers set
`RXBIN007_FEATURE_FROZEN_PARSE` when any of the four opcodes is present. Readers
reject an image that uses one without that feature bit, and older readers reject
the unknown feature bit, so a provisional image cannot be silently decoded with
different semantics. The descriptor adds no RXBIN section or relocation form.

The NR-15 native-stem family is catalogued under arrays, attributes,
references, and objects. `steminit`, `stemget`, `stemset`, `stemreset`,
`stemget2`, `stemset2`, `stemsize`, `stemkeyat`, and `stemvalueat` use ordinary
register operands and set `RXBIN007_FEATURE_NATIVE_STEM`. The feature declares
the instruction contract only: the receiver's private hash-table bytes remain
ordinary process-local VM value state and are never serialized as an RXBIN
section. Two-segment forms stream `left || "." || right` during lookup and
materialize a canonical key only for a proved new insertion.

`typeof`, `istype`, and `asserttype` are object-contract operations. Compiler
generated code uses them for object casts/tests/introspection; scalar
`typeof`/`is` cases are folded earlier by `rxc`.

Interface default methods use that same path. The assembler does not introduce
new opcodes for them; it simply carries `META_MEMBER` kind `method final` and
emits the interface method body as an ordinary procedure. The VM method
registry then decides whether `srcmethodsel` should bind `class.member` directly
or fall back to the interface's emitted default-body procedure.

At the current Level B stage, the runtime selection rule is:

- each provider row may carry an optional resolved class-side `§match` or
  `§match.member` procedure
- `srcfprocsel` causes the VM to call that effective `match` on every candidate
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
