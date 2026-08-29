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

Native-callable provenance uses `META_PROVIDER`, paired by callable symbol with
the canonical `META_FUNC` signature:

```rxas
.meta "namespace.callable"=".provider" "stable-provider-id"
.meta "namespace.optional"=".provider.optional" "stable-provider-id"
```

`.provider.required` is accepted as an explicit alias of `.provider`. Provider
IDs are platform-independent and restricted to ASCII letters/digits followed by
letters, digits, `.`, `_`, or `-`. The assembler stores only symbol, provider
ID and required/optional flags in this record; it does not duplicate the
callable signature. RXBIN 007 writers set the native-provider feature bit.

Module initialization uses a dedicated `META_INITIALIZER` record:

```rxas
.meta "example.boot"=".initializer" ".void" boot() ""
```

The five-field shape deliberately resembles callable metadata so generated
RXAS and `rxdas` round trips retain the namespace-qualified diagnostic symbol
and the local procedure reference. The assembler requires `.void`, an empty
argument signature, and a local bytecode procedure. It stores the symbol and
procedure as typed constant-pool references rather than treating the label as
an exported callable. Multiple records are valid and their metadata-chain order
is execution order. RXBIN 007 writers set the initializer feature bit.

The serialized `expose_head` chain includes both `EXPOSE_REG_CONST` and
`EXPOSE_PROC_CONST` records. Runtime linking and other module-local walkers now
rely on that chain instead of scanning the whole constant pool.

The interface/callable-contract and initializer work extend that same metadata
path rather than introducing a second binary header mechanism. In addition to
`META_CLASS` and `META_ATTR`, the assembler now serializes:

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
`0x...` spelling. Operand rendering is dynamically sized: a binary constant
may be much larger than the disassembler's ordinary stack line buffer and must
still round-trip in full. For an oversized binary used directly by one or more
instructions, ordinary `rxdas` output emits the value once as a private
`§rxdas.const.<pool-offset>` `.const` alias and uses that alias at each operand;
valid jump-table binaries retain their procedure-local `.jtable` form. The
explicit `-p` constant-pool dump still prints each complete value.
`encode_binary_to_hex()` is a bounded
`snprintf`-style formatter; do not reintroduce pointer or remaining-length
arithmetic that advances beyond the supplied buffer.

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
- `pgeti rOut,rBin,rIndex`
- `pseti rBin,rIndex,rValue`
- `pgetf rOut,rBin,rIndex`
- `psetf rBin,rIndex,rFloat`
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
- `bslice rDst,rSrc,rStart,rLen`
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

`pgeti`/`pseti` and `pgetf`/`psetf` are the distinct Release 1 host-native
packed-item surface. Their index is a zero-based item number rather than a byte
offset. Integer items have the exact host `rxinteger` representation and float
items have the exact host VM `double` representation. They deliberately do not
provide endian or width conversion, and the same binary bytes may be viewed as
either item type on different accesses. A negative index, index multiplication
overflow, or an item that does not fit wholly inside the binary's logical byte
length raises `OUT_OF_RANGE` before a write. RXAS accepts register operands for
these operations; the compiler materializes a readable binary constant into an
aligned runtime binary register before emitting `pgeti` or `pgetf`.

`bresize` sets the logical byte length and zero-fills newly exposed bytes. The
VM may keep a larger private physical capacity, grown in blocks and reused
across later logical resizes; `blen` reports only the logical length. Negative
lengths raise `OUT_OF_RANGE`, and allocation failure raises `FAILURE`.
`bclear` sets the logical byte length to zero. `bfill` fills the
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

`bslice rDst,rSrc,rStart,rLen` performs explicit zero-based extraction without
mutating the source. The former binary cursor instructions are retired and
their numeric opcode slots remain reserved; old RXBIN images using them are
not compatible with this instruction set. Any build directory or auxiliary
worktree first built before this cursorless change must therefore be cleaned
and rebuilt before use; mixing retained generated RXBIN with current tools is
unsupported. Release 1 source lowering also uses
target-sized `bcopy`, typed reads/writes, and zero-copy compares where a
materialized slice is unnecessary. `setbyte` and `bupdate` are strict and
raise `OUT_OF_RANGE` for invalid indexes, bytes outside `0..255`, or overlay
writes past the destination length. `stobin` copies the register's current
string bytes into its binary slot. `bintos` validates the register's current
binary bytes as UTF-8 and copies them into its string slot; invalid bytes raise
`UNICODE_ERROR` in UTF builds.

The explicit string form is
`substring rDst,rSrc,rStart,rLen`, where positions and lengths count Unicode
characters. A negative start clips to zero and a non-positive length produces
the empty string. `bslice` likewise clips a negative start to zero, but a
negative binary length raises `OUT_OF_RANGE`. Both operations are safe when
destination and source name the same storage. A required allocation either
completes before the destination changes or raises `FAILURE`; the opcode signal
metadata records these failure-before-write contracts.

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

The K04e handler audit records `STRLEN` as a string read with
failure-atomic `UNICODE_ERROR` before its integer write, all three `ISUB`
forms as failure-atomic `OVERFLOW_UNDERFLOW` before their integer write, and
the three-form loose `REQ` through `RLTE` family as non-signalling string
reads with an integer result. Integer subtract and loose comparison use the VM
integer setter and therefore clear reference/native payload components;
`STRLEN` preserves other destination components. These entries describe the
existing handlers and do not change VM or RXBIN semantics.

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

RXAS optimisation routing is explicit in `assembler/rxas_flow_pass.c`. Every
current rule family has one stable owner—local mechanical scan, immutable CFG,
component SSA/use proof or diagnostic-only—and a capability mask over local
scan, CFG, dominance, signal, storage, value, use and loop facts. One linear
opcode/metadata census conservatively identifies potential consumers before
whole-procedure analysis. A zero candidate count may avoid a consumer, but the
census is never a correctness proof and is deliberately allowed to overselect.
K05 branch threading and M00 reachability request CFG only; exact K06 copy/
status subsumption remains local; M01-M06 and K01-K04 retain their component
proof ownership. M01-M04 request the component base, while M05, M06 and
K01-K04 additionally request the sparse use index. The established `-d` flow
dump has its own explicit diagnostic route. No current production consumer
requests loop analysis.

D0.5 adds an exact linear write-once/single-use typed-copy route. One local-
register census records explicit occurrences plus metadata, TRACE, implicit
register and call-window observations. `ICOPY`, `FCOPY` or `SCOPY` may bypass
SSA only when its destination occurs exactly as that copy destination and one
immediately adjacent, non-signalling component read; the read is redirected
and the copy is deleted in the same sparse transaction. Reused registers,
mapping operations, calls, TRACE/meta observations, signalling consumers and
all non-adjacent uses remain on the SSA route.

NR-27 adds a transient whole-procedure machine-flow layer after the bounded
100-item local peephole and before ordinary RXBIN emission. Stable peephole
output is moved, with its token ownership, into a growable procedure stream.
One immutable `RxasFlowProcedure` is then built for each rewrite epoch;
surviving records still pass through the same assembler emission functions, so
RXAS syntax, canonical RXBIN and the public ABI do not change.

The peephole is a permanent preprocessing owner, not legacy code scheduled for
removal. Exact local rewrites belong there when opcode metadata proves complete
equivalence without procedure-wide liveness, alias/storage, signal-path,
hidden-cleanup or TRACE-observation reasoning. Adjacent non-signalling algebra
and mechanical encoding selection are the normal cases. CFG/SSA remains the
owner when any of those wider facts are required. Increasing the bound must be
measured against the same input for pre-graph census, graph/SSA size, emitted
image, assembler time and peak memory; the bound is not a substitute for a
procedure analysis.

`assembler/rxas_flow_graph.c` is the sole owner of stable record, instruction,
basic-block, typed-edge and reachability descriptors for that epoch. M00 uses
its entry/same-frame/asynchronous-root reachability to remove dead opcode,
TRACE and source-step records. Existing semantic consumers share its lazy
proof manager, and K05 collects a complete compatible branch-thread batch from
the same CFG only after those consumers decline the epoch. Any mutation ends
the epoch and rebuilds the graph, except that compatible semantic edits are
one validated transaction whose proof-facing records remain pinned to the
epoch originals. The original queue index remains the record
identity;
every opcode record maps to a stable instruction ID, block ID and exact
pre-emission RXBIN address. Source-step, TRACE, label and metadata records keep
their own IDs and map to the block/address at which they are observed.

Code blocks start at procedure entry, labels, branch and handler targets, and
after calls, signalling instructions and other control terminators. Seven
stable synthetic blocks represent entry, handler dispatch, asynchronous
handler entry, normal return, unwind, terminal exit and unresolved control.
Edges distinguish normal fallthrough, branch, signal skip, handler, unwind,
terminal and explicit fail-closed unknown flow. Signal edges
come from `RxOpSignalContract.continuations`; handler-policy instructions with
label operands feed the handler and asynchronous roots rather than inventing a
normal branch from the registration instruction.

Construction snapshots record/opcode metadata, builds an open-addressed label
index, forms blocks, appends edges and computes compact rooted reachability. It
is expected linear in queued records plus emitted edges. There is no parallel
legacy edge graph, dense record-by-register use/kill allocation or dense M07
storage environment. M07 is a debug-only executable oracle over sparse SSA
storage nodes and queries. Read APIs require the owning epoch and return no
descriptor for a stale epoch. `rxas -d` prints deterministic `PERF3 flow-*`
and `PERF3 sparse-storage-oracle` records without pointer values, so graph,
edge and retained-analysis identities can be compared mechanically.

Stage 3 layers a demand-driven structural-analysis manager over each immutable
procedure epoch. `assembler/rxas_flow_analysis.c` retains successor edge IDs
and unique predecessor block sets in sparse CSR form, then computes reachable
reverse postorder from a virtual root joining the procedure-entry, registered-
handler and asynchronous-handler roots. Unreachable blocks remain represented
by the graph but are excluded from dominance, SCC and loop proofs.

The cached base structural result contains Cooper-Harvey-Kennedy immediate
dominators, dominance-tree intervals and sparse dominance frontiers. SCC,
backedge and loop-forest construction is a separate monotonic capability on
the same cached object. When explicitly requested it adds iterative Kosaraju
SCCs, dominance-classified backedges and a loop hierarchy. Natural loops with
the same header share one region; irreducible SCCs remain explicit conservative
regions. Instruction-level signal retry was retired on 2026-08-03, so
backedges and loop regions represent source/control flow rather than synthetic
retry cycles. Source order remains a diagnostic mapping and is never treated
as dominance evidence.

Every structural query requires the graph epoch. The first successful result
is cached until graph destruction; a failed deliberately small work budget may
be retried with a larger budget. Allocation failure, invalid control or budget
exhaustion leaves the analysis unavailable and cannot enable a rewrite.
Deterministic `PERF3 flow-analysis*` and `PERF3 flow-loop` diagnostics expose
work, retained-byte, reachability, predecessor, dominator-iteration, frontier,
SCC, backedge and loop counters; dormant loop facts print as unavailable rather
than being inferred. Post-dominance is deliberately deferred until a consumer
needs a must-execute query. Ordinary assembly does not solve unused loop
analysis: tests and future optimizer consumers request it explicitly. Stage 3
therefore changes neither queued records nor RXBIN output.

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
set is integer, float, string, decimal, binary, attributes, reference,
host-owned native payload and the distinct attribute-count component.  The
count is not a value-status flag: `setattrs`, `minattrs`, `getattrs` and
attribute links describe it explicitly in canonical opcode metadata.  Values
distinguish procedure-entry, direct write, constant, copy, derived, known
absent, phi and unknown definitions; a null/absent value is not the same as an
unavailable proof.  Copy propagation therefore carries presence as well as
the source identity, including a `dcopy` of a null decimal.  Join values and
storage mappings are materialized lazily and cyclic phis canonicalize an
unchanged loop identity without using source order.  An unknown storage input
has a separate value identity and cannot collide with a valid zero-based
`ValueId` or simplify a join into a false proof.

Normal instruction states consume the canonical component read/write and
derivation metadata.  Signal skip, handler and unwind edges instead
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

The proof cache is a capability-lazy service rather than an eager bundle.
Every M01-M06 and K01-K04 call site presents its stable routing ID; the service
acquires the route's CFG, dominance, signal, storage, value and optional use
facts monotonically for the immutable epoch. Public query entry points reject
when their required capability was not declared, so a query cannot silently
expand analysis. A failed stronger capability does not invalidate already
acquired lower-capability facts: that route rejects, while later base
consumers remain safe and available. Loop queries likewise require the
separate loop capability. The compatibility constructor still requests the
complete service for tests and external callers that explicitly need it.

Storage resolution creates a provisional phi only when recursive lookup of a
join requires a cycle placeholder or when predecessor storages genuinely
differ. Equal acyclic predecessors resolve directly to their common
`StorageId`; diagnostics report the elided phi, retained input and cache
counts. Whole-procedure semantic consumers are admitted only when the
indirect-value work bound and the 262,144 block-by-register join bound both
pass. An over-bound procedure continues through local mechanical rewrites,
M00 and K05, but SSA consumers and diagnostic SSA/use materialization fail
closed. Future recovery of valuable advanced cases must use an explicitly
bounded candidate-sliced or region proof rather than raising this limit.

Compatible semantic rewrites are committed through the sparse transactional
manager in `assembler/rxas_flow_batch.c`. M04, K02/K03, M05, M06, K04, M02,
M03 and M01 retain their established priority, but disjoint typed plans may be
collected from one immutable epoch before one rebuild. Only records actually
edited are snapshotted. Proof-facing `RxasFlowRecord` pointers are pinned to
those originals while later consumers inspect the provisional queue; complete
validation either commits the registered records together or restores all of
them. A delete-only consumer additionally requires its target to remain
unchanged from the epoch. K01 remains a separately budgeted storage-
permutation epoch, and K05 remains a later CFG-only batch because topology-
changing rewrites require a different compatibility proof from semantic SSA
rewrites. Deterministic `PERF3 semantic-batch` diagnostics report plans,
changed/deleted records, opcode replacements, operand rewrites and private
locals provisioned. A transaction may retarget the register operand of an
otherwise unchanged TRACE record when an immutable proof plan owns that exact
event. Procedure `.locals` and the assembler's current-local count advance only
after every record in the transaction validates and commits; a rejected or
zero-candidate plan cannot leak a local or leave a partial TRACE rewrite.

An immediate one-based `linkattr1` can additionally name an interned attribute
path.  Its identity contains the owner `StorageId`, exact attribute-count
`ValueId`, reference-effect generation and slot.  The path is reusable through
owner register aliases, but a dynamic/non-positive slot, unknown or merged
count, different owner, or changed reference generation remains fail-closed.

The M05 migration adds `assembler/rxas_flow_use.c`, a fifth demand-driven
per-epoch cache over component SSA.  It indexes direct `ValueId` uses,
storage observations and reverse phi dependencies once per procedure;
edge-aware liveness then propagates only through those sparse dependencies.
Explicit operands, read/write operands, metadata, register-backed TRACE,
implicit reads, call windows and opaque observations remain distinct use
kinds. Component writes are indexed separately from observations. Writes
through a storage phi are expanded once to every
possible leaf storage; an unresolved target becomes an opaque-write barrier,
not a false read of an unrelated `ValueId`.  A range call is retained as one
bounded call-window observation rather than expanded across every
local/component pair.

The first use-index consumer replaces the dense per-candidate typed-copy
solver.  Exact `icopy`, `fcopy` and strict `scopy` deletion requires a local,
unaliased destination, the copy's write-once result `ValueId`, an equivalent
source value at every redirected use, no metadata/TRACE/read-write or
live call-window observation, and at least one exact component-only operand
rewrite.  The proof service returns an immutable all-or-nothing rewrite plan;
the optimizer validates the complete plan before changing any operand or
deleting the copy.  Budget exhaustion, stale epochs and incomplete use facts
reject the whole plan.  The old availability/may-reach solver and its repeated
dense register scans have been removed.

X01 is the first component-placement consumer built on the same sparse use
index. It recognizes an exact adjacent typed copy followed by a one-register
metadata-derived XTOY operation, but asks the proof service—not adjacency—to
authorize the rewrite. Its immutable plan proves distinct local unaliased
storage, the copied input and derived result `ValueId` relationships, dead
displaced components, redirectable result uses, and compatible call, signal,
context, reference, metadata and TRACE observations. The consumer validates
the complete plan, retargets the derivation to the original source, redirects
all proved result uses, removes only matching derived-value TRACE events, and
deletes the copy as one transaction. The first production case is
`dcopy temporary,source` followed by `dtos temporary`; the mechanism is
component- and derivation-metadata driven rather than mnemonic-specific.
Reference objects are a separate observation from register-to-storage aliases:
an earlier `mkref` may keep either storage visible even when the register alias
census is one. X01 therefore rejects a placement when such a reference-creation
path can reach the candidate; reference lifetime/release is not guessed.

H01 is the first loop-capable value-reuse consumer. It recognizes repeated
joined string keys only after exact CONCAT component metadata identifies an
equivalent literal/constant left component and right-component `ValueId`.
The proof requires a dominating lazy seed and candidate within one common
reducible natural loop, private destination storage at both construction
sites, exact signal/effect compatibility and a complete redirectable use set.
Source order and register-number equality are not proofs. A changed component,
reference or alias exposure, relevant call-window observation, unowned TRACE
event, irreducible/no-common loop, stale epoch or exhausted capability rejects
the candidate.

Accepted candidates sharing one seed are planned against the same immutable
epoch and receive one new private local. The seed CONCAT, its exact TRACE event
and direct proved seed uses are retargeted to that local; later equivalent
CONCAT and owned TRACE records are deleted and their stem-key uses are
redirected. This permits the compiler's former temporary register to be reused
or overwritten later without changing the cached generation. Placement remains
lazy: the current production case deliberately rejects preheader hoisting
because the seed is conditional, the right component is loop-variant and the
TRACE event is ordered. The mechanism is metadata/value/use driven rather than
specific to RexxCPS, a stem name or a register number.

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

TRACE observation is profile-sensitive. In an optimised assembly, a rewrite
may omit, combine or relocate an event when it removes or moves the operation
or value represented by that event. It must delete a removed value's matching
event atomically, retain unrelated events, and never leave metadata reading a
deleted, stale or mismatched component. Multiple retained events may share one
reached address and remain ordered, so `CNOP` is not needed merely to separate
them. `rxas -n`, together with compiler no-opt mode, provides the source-
correspondent trace path; ordered batching is not a requirement to preserve
every pre-optimisation event.

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
allocation, signal or externally observable effect. This conditional contract is more
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
immediately adjacent, metadata-classified, non-signalling, context-neutral,
single-component producer, the proof service may return an immutable plan that
retargets operand 1 from a disposable local temporary to the copy's final local
and deletes the copy. The exact producer result must have only the typed copy
as a sparse direct or dependent use. Both storages must be local, known and
unaliased; the producer must not read the final storage through another
register mapping; and TRACE/register metadata, calls, opaque uses, asynchronous
handlers and source-address observations reject the candidate.

Scalar producers such as `load` and integer comparisons release reference and
native-payload state, whereas `icopy`/`fcopy` write only their typed component.
Producer forwarding therefore also proves that every component cleared by the
producer is already absent in both the discarded temporary and the final
destination. Fresh local entry storage supplies a known empty state;
arguments, globals, aliases and unknown or previously populated storage fail
closed. This hidden-cleanup condition is part of opcode metadata and prevents
typed liveness alone from changing lifetime behaviour.

Copy rewrites proved from one graph may be applied in the same iteration only
when their complete physical source/destination register pairs are disjoint.
Their substitutions then commute and cannot invalidate one another's SSA/use
proof. Procedures newly admitted through exact jump-table
edges always receive reachability cleanup. Global typed-value analysis is
additionally bounded to one million `queue-item x liveness-word` cells for
those procedures; above the bound, `rxas -d` reports `scope=reachability-only`.
This preserves the linear high-yield cleanup without turning large generated
dispatch procedures into quadratic assembly work. Procedures without resolved
indirect tables retain the established NR-27 behavior without that bound.

Integer compare/branch fusion is also a whole-procedure proof-service consumer.
Canonical opcode metadata maps each `IEQ`/`INE`/`IGT`/`IGTE`/`ILT`/`ILTE` plus
`BRT`/`BRF` form to `BEQ`/`BNE`/`BGT`/`BGE`/`BLT`/`BLE`, including normalized
immediate/register operand order. The immutable plan requires the compare and
branch to be consecutive executable instructions in one CFG block, all three
opcodes to be total and context-neutral, and the local result storage to be
unaliased. A source may share that storage only when it is the same physical
result register and component SSA supplies the exact pre-write integer
`ValueId`; the fused branch then reads that incoming value rather than the
materialized Boolean. Every reference/native payload cleared by materializing
the Boolean must already be absent. The result integer `ValueId` may have only
the branch read and, under the selected TRACE
policy, any number of precisely matching result events between compare and
branch. The immutable plan records and revalidates those events, deletes them
atomically with the compare, and leaves unrelated same-boundary events present.
Live non-trace uses, metadata and caller windows reject fusion; unrelated TRACE
events remain. Call-window handling now resolves the exact local bounds when
the count has a constant integer `ValueId`, otherwise conservatively extends to
the highest local. Classified fixed-arity and zero-argument calls expose their
complete caller interface through explicit operands; only range-call forms
add an implicit base/count local window. The proof follows the compare result
through copy/derived values
and phis and rejects only when that identity is visible in the resolved window
or the query fails closed. Counted CALLs had deliberately unknown signal
contracts in the K04c baseline. Their former retry edge turned an otherwise
constant count into an unknown phi and widened the window. K04d retires that
non-standard, unused continuation and adds propagated-call partial-state
metadata for the remaining skip/handler/unwind failure paths. The old bounded
recursive read-before-kill solver and its keyhole rules have been removed.

`test_rxop_metadata` is generated from `op_table` and both complete sidecars so
instructions cannot be added without mechanically aligned effect and signal
entries. The signal sidecar records the integer fused branches as
non-signalling, decimal literal load as plugin-signalling and decimal copy as
total/non-signalling, while one- and two-register integer-to-float conversion
and float comparison are also non-signalling in the current VM handlers. The
legacy `FGT`/`BRT` to `FGTBR` shortcut is not selected until the fused float
opcode receives an explicit signal contract and equivalent component proof.

Attribute/register-view cleanup also retains one small mechanical keyhole. A
full `copy`
already copies the VM value status word, so `copy rA,rB` followed immediately
by `acopy rA,rB` is reduced to the full copy for hand-written or legacy RXAS.
This is the closed K06 classification, not a proof-service consumer. Opcode
metadata records full `COPY` as reading/writing every component and `ACOPY` as
reading/writing only `RXOP_COMPONENT_ATTRIBUTES`; both are non-signalling. The
VM handlers confirm `copy_value()` assigns `status.all_type_flags` before
`ACOPY` would repeat that exact assignment. The declarative rule captures the
same source/destination pair and permits no intervening executable instruction.
`test_rxop_metadata` locks the subset/non-signalling invariant, while the
focused optimizer fixture locks different-source, different-target and
executable-gap negatives. Moving this exact local algebra into CFG/SSA would
add analysis cost without adding a correctness fact.
Compiler-generated RXAS should not emit that pair for typed payload access. A
future explicit flag-copy operation would use `acopy`, but there is no current
compiler use case. Typed payload copies are `icopy`, `fcopy`, `scopy`, `dcopy`,
and `bcopy`; `bcopy` copies only the binary payload, not
public/compiler/library status flags.

Duplicate `link`/typed-copy/`unlink` reads and equivalent one-based
`linkattr1` reads are no longer keyhole authorities. One immutable proof plan
validates both exact triples, proves the full typed-copy component mask
equivalent, and rewrites the later link to copy the first
detached value before deleting its original copy/unlink. An attribute plan
also proves the candidate slot is in range so deletion cannot suppress an
`OUT_OF_RANGE` signal. Calls through aliased arguments, divergent source phis,
changed source/detached components, changed count/reference generation,
different owners or slots, component writes and unproved signal paths reject.
Register metadata and TRACE reads of the unchanged first detached value may
remain ordered while the redundant second executable read is removed. The
twelve former syntax-expanded rules have been deleted.

Cancelling `swap` pairs are likewise owned by the proof service rather than
the legacy bounded `NO_HAZARD` keyholes. A pair-key demand filter only asks
the question. The immutable proof verifies the exact physical operands,
dominance, restored pre-first/post-second `StorageId` bindings and every
intervening raw-register use from the shared use index; relevant component
reads/writes, register metadata, TRACE, call windows and opaque state
reject. Different registers that already name the same storage are an exact
identity case, while a disjoint intervening permutation or unrelated external
effect need not block deletion. The consumer has no source-queue distance
limit and revalidates the complete plan before deleting either instruction.

Potential signals remain explicit CFG edges. A source-linear normal/skip
spine may cross a known static signal when its handler policy is still the
inherited entry policy: resumption reaches the restoring swap, while branch,
return, unwind and terminal actions leave the current frame's temporary
mapping. A handler installed, merged or clobbered in the procedure rejects.
Uses reachable from the asynchronous-handler root also reject even when their
records lie outside the lexical interval. Ordinary control splits and unknown
signal-name sets remain closed. This is narrower than treating signals as
invisible and more capable than requiring one basic block.

NR-09 may encode two adjacent or metadata-separated swaps as one ordered
four-operand `swapn`. Repeating the same unordered physical pair composes to
identity algebraically for every input mapping and is deleted by the same K01
proof. Component SSA independently composes all overlapping `swapn` pairs in
operand order into final input-state sources; it does not interpret them as a
parallel assignment. Larger arbitrary permutation simplification remains a
separate future rewrite consumer.

Signal support adds action-aware and dynamic-name forms:

- `sigcalla proc(),"NAME"` installs a handler that returns an internal action
  marker interpreted by the VM as `skip` or `fail`; the unused non-standard
  `retry` action has been retired
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

## 7. Core Channel Instructions

Core concurrency and future transport providers use one transport-neutral RXAS
family. Level B classes must use these instructions;
there is no parallel RXPA task-start/task-wait ABI and providers do not add
provider-specific opcodes.

The current public instructions are:

| Opcode | RXAS form | Outputs | Inputs |
| ---: | --- | --- | --- |
| `650` | `chanopen rStatus,rChannel,rProviderType,rRequiredCapabilities,rConfiguration` | status, channel | provider type `.int`, capability mask `.int`, canonical RXCV `.binary` |
| `651` | `chanstart rStatus,rTicket,rChannel,rEnvelope,rWaitMicroseconds` | status, ticket | channel `.int`, RXCV envelope `.binary`, relative wait `.int` |
| `652` | `chanwait rStatus,rCompletion,rChannel,rWaitMicroseconds` | status, completion | channel `.int`, relative wait `.int` |
| `653` | `chancancel rStatus,rChannel,rTicket,rReason` | status | channel/ticket `.int`, RXCV reason `.binary` |
| `654` | `chanclose rStatus,rChannel,rMode` | status | channel `.int`, mode `.int` |

The two-output forms require distinct output registers. Outputs may alias input
registers because the VM snapshots inputs before writing failure defaults or
results. A failed operation returns a status and leaves the companion output at
`0` or empty binary; the instructions have `RXSC_NONE` signal contracts.

All five instructions are initially `FLG_OPT_BARRIER` with classified opaque
effects. Optimizers must not move, duplicate, fold or remove them. TRACE,
debugger and profiler identity remains the canonical opcode. The handlers are
outlined/cold under the current `profile-20` policy.

Any image containing one of these opcodes requires
`RXBIN007_FEATURE_CHANNELS` (`1 << 3`). RXAS emits it, RXLINK retains the
validated union, and RXBIN readers reject either a channel opcode without the
bit or an unsupported feature bit. RXAS/RXDAS round trips preserve the five
mnemonics and operand order.

`rProviderType` is one mutually exclusive implementation code;
`rRequiredCapabilities` is a bit mask. Type `0` is invalid; type `1` is the
core local-thread provider, type `2` is the isolated-process provider, type
`3` remains the reserved open-host provider, type `4` is the reusable byte
endpoint and type `5` is the structured child-process provider. Extension
codes begin at `65536`. Type `1` advertises bounded admission (`0x0001`), cancellation
(`0x0002`), deadlines (`0x0004`) and completion-order observation (`0x0008`).
Unknown required bits fail instead of being ignored. A private validated
registration seam and fake-provider conformance fixture exist; no public
provider-plugin ABI is implied.

Configuration, envelopes, reasons and completions are canonical versioned RXCV
documents. The current implementation includes the complete value tree, typed
task register images, provider-owned deadlines/scopes, Level B classes and
Level G lowering over sealed task procedure, transferable receiver and
`.taskwork` factory targets.
Assembler `.task1`, `.task2` and `.task3` metadata carry an 80-byte binding;
RXAS resolves its image, callable, signature and adapter identity and RXBIN
validation rejects malformed or stale bindings. Raw
channel/ticket integers are execution-local capabilities, not transferable
values. Concurrent HTTP/TLS is a Level G library consumer, not an opcode or
provider type. Exact status codes, RXCV layouts and lifecycle rules are in
[`CREXX_CONCURRENCY.md`](CREXX_CONCURRENCY.md) and the per-instruction
[RXAS reference](../reference/rxas/instructions/09-io-sockets-processes-and-time.md).
