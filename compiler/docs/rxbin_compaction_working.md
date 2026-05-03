# RXBIN Compaction Working Notes

Status: working, stages 1 and 2 plus runtime-state split implemented
Last updated: 2026-04-22

## 1. Purpose

This document is the working record for reducing `rxbin` size by compacting
literal storage, compressing serialized module data, and later adding a
build-time linker.

It is intended to capture:

- current implementation facts
- staged direction
- compatibility constraints
- open approval points
- the immediate next step once approved

## 2. Scope of this programme

This work currently covers three stages:

1. move floating-point literals out of inline instruction slots and into the
   constant pool
2. compress serialized `rxbin` content:
   - a code-stream packing scheme for the current 64-bit instruction storage
   - constant-pool compression with Heatshrink or an equivalent lightweight
     compressor
3. add a build-time linker for modules

Stage 2 is now specified and implemented in the current `003` format.

In addition, this programme now includes one supporting structural refactor:

- separate serialized module data from runtime-only VM state before the
  compression and linker stages

## 3. Current implementation review

### 3.1 `rxbin` layout today

Current `rxbin` files are written by `rxas` through `binutils/include/rxbin.h`
and loaded by `rxvm`.

Important current facts:

- the file already has a real module header, not a headerless stream
- the current on-disk format version is `003`
- each module stores:
  - header
  - module name
  - module description
  - instruction stream
  - constant pool
- the instruction stream is serialized as an array of `bin_code` entries
- each `bin_code` entry currently has room for:
  - opcode metadata
  - an integer literal
  - a floating literal
  - a char literal
  - an index into the constant pool or instruction stream

Operationally, the instruction stream is slot-based rather than byte-packed.
On the current 64-bit targets this effectively means one 8-byte storage slot
per opcode or operand entry.

Important clarification:

- the main module header is not the primary place where runtime-only state
  leaks into the format
- the more significant leakage is in the serialized instruction and
  constant-pool record shapes

### 3.2 Literal handling today

Current assembler behaviour is split:

- string literals are deduplicated into the constant pool
- decimal literals are deduplicated into the constant pool
- binary literals are deduplicated into the constant pool
- procedure and metadata records live in the constant pool
- integer literals are emitted inline in instruction slots
- floating literals are emitted inline in instruction slots as `double`

Important consequence:

- repeated float literals are not deduplicated today
- every inline float operand costs a full instruction slot in the code stream
- step 1 cannot be just a storage tweak; it changes how float-operand
  instructions are encoded and decoded

### 3.3 Linking today

There is already runtime linking in `rxvm`.

Important current facts:

- `rxvm_load()` / `rxldmod()` can load multiple modules
- `rxvm_link()` resolves exposed procedures and exposed registers across loaded
  modules
- runtime linking must remain available for late-loaded modules and native
  plugins
- the file loader already supports more than one serialized module in a single
  file by reading module records until EOF

Important consequence:

- stage 3 is not "add linking from nothing"
- stage 3 is better framed as "add build-time linking and packaging", while
  keeping runtime linking for dynamic cases

### 3.4 Serialized/runtime state leakage today

The main structural issue is that some serialized records are also used as live
runtime objects.

Important historical facts before the refactor:

- `module_file` had runtime-only wrapper flags such as `fromfile` and
  `native`, though those flags were not themselves written into the file
  payload
- `bin_code` reserved space for `impl_address`, which was runtime-only
  threaded dispatch state
- `proc_constant` carried runtime-only fields:
  - `binarySpace`
  - `frame_free_list`
  - `frame_free_list_head`
- runtime linking mutated imported procedure records by copying runtime fields
  from the resolved exporter
- `rxvm_prepare()` rewrote opcode slots in place into dispatch pointers

Important consequence:

- serialized records currently double as mutable runtime state
- this is awkward for a build-time linker, because the runtime mutates the same
  structures the file format exposes
- this also makes it harder to share or reason about loaded module images
  cleanly across contexts or future parallel execution models

Important limit:

- separating serialized and runtime state would help linker design and future
  concurrency work
- it would not, by itself, make `rxvm` thread-safe, because global registers,
  exposed-symbol trees, interface registries, and module lifecycle state remain
  mutable runtime state in the VM context

## 4. Programme goals

The goals are:

- reduce serialized `rxbin` size without changing Level B language semantics
- preserve cross-platform portability of `rxbin`
- use explicit format-version gates for incompatible changes
- avoid pushing decompression/decoding complexity into the hot execution path
  when that work can be done once at load time

The preferred bias is:

- compact on disk
- simple, regular in-memory structures after loading

## 5. Stage 1: pool-backed floating literals

### 5.1 Problem statement

Floating literals are currently emitted inline in the instruction stream. That
is expensive for repeated literals and blocks later code-stream packing.

### 5.2 Preferred direction

Preferred direction for stage 1:

- add a dedicated constant-pool entry for binary 64-bit floating literals
- deduplicate those entries in the assembler
- in the final unpublished `003` layout, keep the existing float instructions
  but store float operands as
  constant-pool indexes instead of inline `double` payloads in the code stream
- materialize the `double` from the pool during execution
- keep the programme on `003` rather than carrying a transient published `002`

The preferred pool representation is:

- store the exact 64-bit payload, not a text form
- deduplicate by stored bit pattern, not by decimal spelling

That keeps the constant compact and avoids floating-point comparison problems in
the assembler.

### 5.3 Encoding decision

The stage-1 direction is now fixed:

- do not add sibling opcodes just for pool-backed floats
- keep the existing float instructions
- under the final `003` format, interpret float operands as indexes of
  `FLOAT_CONST` records in the constant pool

This keeps the instruction set stable while changing only the serialized
representation of float operands.

### 5.4 Compatibility notes

Stage 1 should be treated as a hard format change.

Expected consequences:

- assembler changes in literal pooling and opcode emission
- VM loader/interpreter changes in operand decoding
- disassembler changes if it prints float operands directly from inline slots
- format-version handling in `rxbin` header checks

Compatibility decision:

- no backward compatibility is required for this programme
- `001` is not supported by the current tools
- earlier unpublished `003` layouts are also not supported
- it is acceptable for the updated tools to reject old layouts rather than
  support dual decoding paths

### 5.5 Acceptance target

Stage 1 is complete when:

- repeated float literals are emitted once into the constant pool
- produced binaries execute with the same observable numeric behaviour as
  before
- the tools consistently emit and expect the final `003` layout
- focused tests cover repeated literals, negative zero, and mixed float/decimal
  instruction cases

## 6. Supporting refactor: separate serialized and runtime state

### 6.1 Recommendation

This refactor should be part of the programme.

Recommended sequencing:

1. complete stage 1 first
2. perform the serialized/runtime split next
3. then proceed to stage 2 compression work
4. then proceed to stage 3 linker work

This should not be folded into stage 1, because that would expand a targeted
float-format change into a much broader runtime object-model refactor.

### 6.2 Why it belongs in the programme

This refactor directly supports the later stages:

- it gives the linker a cleaner object model to work with
- it removes VM execution details from the serialized record shapes
- it makes section compression less coupled to runtime mutation
- it creates a better boundary for future multi-context or parallel execution
  work

### 6.3 Preferred direction

Preferred direction:

- keep on-disk structures purely serializable
- build runtime side tables/objects during load
- keep threaded dispatch data in runtime-only storage rather than in the
  serialized instruction image
- keep per-procedure frame recycling state in runtime-only storage rather than
  in `PROC_CONST` payloads
- make runtime linking bind runtime procedure objects, rather than mutating
  serialized procedure records with borrowed runtime pointers

Concretely, the first split should focus on:

- procedure runtime state
- prepared/threaded instruction state
- owner-module/runtime backpointers

### 6.4 Limits of this refactor

This refactor is worthwhile, but it is not a full multithreading solution.

Even after this split, shared mutable VM state would still exist in:

- module globals
- exposed symbol registries
- interface dispatch registries
- module lifecycle state in `rxvm_context`

### 6.5 Implementation status

This refactor is now implemented in file format `003`.

The implemented shape is:

- `bin_code` no longer reserves `impl_address`
- `proc_constant` is serialized metadata only
- `rxvm` builds per-module `proc_runtime` tables during load
- exposed-procedure lookup, runtime linking, reflection, and external call
  entry points now use `proc_runtime *`
- `rxvm_prepare()` populates per-module `prepared_dispatch` side tables instead
  of mutating serialized instruction slots
- frame recycling state now lives only in runtime procedure objects

## 7. Stage 2: serialized compression

### 7.1 Problem statement

After stage 1, the instruction stream and constant pool are still serialized in
an almost direct form. That is simple, but it leaves size on the table.

This stage should assume that the supporting refactor in section 6 has either
been completed or has been explicitly deferred with a clear reason.

### 7.2 Header shape in `003`

Stage 2 keeps the programme on `003`.

Compatibility note:

- no compatibility is required with the earlier unpublished `003` baseline
- the new `003` header layout is now the only supported `003` layout

The header now carries both logical and stored sizes:

- `instruction_size`: expanded number of `bin_code` slots after load
- `instruction_stored_size`: number of bytes stored for the instruction section
- `constant_size`: expanded constant-pool size after load
- `constant_stored_size`: number of bytes stored for the constant section
- `section_flags`: bit flags describing whether each section is raw or packed

Current flag allocation:

- bit `0`: instruction section packed with the stage-2 code-stream codec
- bit `1`: constant section packed with the stage-2 blob codec

Important consequence:

- the loader always knows the final target size before it starts expanding a
  packed section

### 7.3 Code-stream format

The instruction section is no longer serialized as raw `bin_code` memory when
packing wins. It is serialized as a logical token stream:

- opcode
- operand 1
- operand 2
- operand 3

`no_ops` is not stored in the packed form. It is reconstructed from the opcode
table during load.

Packing rules:

- all tokens are encoded as unsigned integers
- signed `I` operands are ZigZag-transformed before encoding
- all other operand classes use their current unsigned/index value directly

The byte-level integer codec is:

- `0xxxxxxx`: one value in the range `0..127`
- `10aaabbb`: two consecutive values in the range `0..7`
- `110xxxxx yyyyyyyy`: one 13-bit value
- `1110xxxx yyyyyyyy zzzzzzzz`: one 20-bit value
- `11110xxx ...`: one 27-bit value in 4 bytes total
- `111110xx ...`: one 34-bit value in 5 bytes total
- `1111110x ...`: one 41-bit value in 6 bytes total
- `11111110` plus 6 payload bytes: one 48-bit value in 7 bytes total
- `11111111` plus 8 payload bytes: one 64-bit value in 9 bytes total

Payload bytes are stored most-significant first inside each multi-byte form.
Writers emit the shortest canonical form.

Operational rules:

- packing is done across the full logical token stream, not per raw slot
- the dual-tiny form may pair any two consecutive token values `0..7`,
  including opcodes or ZigZag-transformed signed operands
- the loader expands the packed stream back into ordinary `bin_code[]`
  before the VM links or prepares the module

### 7.4 Constant-pool format

The constant pool stays structurally unchanged in memory.

When compression wins, the stored constant section uses a lightweight
Heatshrink-style LZSS blob codec:

- one control byte describes the next 8 tokens
- control bit `0`: literal byte
- control bit `1`: back-reference token
- back-reference token size: 2 bytes
- window size: 4 KiB
- encoded distance range: `1..4096`
- encoded match length range: `3..18`

This keeps the implementation self-contained while preserving the intended
"Heatshrink or equivalent lightweight compressor" design point.

### 7.5 Loader/runtime behaviour

Stage 2 remains a load-time-only compaction design:

- packed instruction streams are expanded once during module load
- packed constant pools are expanded once during module load
- the interpreter loop, linker, disassembler, and metadata walkers continue to
  work on normal expanded in-memory data

Section writers may still emit raw sections when packing does not reduce size.

## 8. Stage 3: build-time linker

### 8.1 Problem statement

Today module linking happens at runtime. That is necessary for dynamic loading,
but it does not reduce file size for known module sets built together.

This stage should also assume that the supporting refactor in section 6 has
been completed first.

### 8.2 Tool placement and repository structure

Recommended repository placement:

- add the linker as a new top-level sibling tool directory, alongside
  `assembler`, `disassembler`, and `interpreter`
- keep `binutils` as the low-level shared format/helper layer
- do not move `assembler` or `disassembler` under `binutils`

Reason:

- the repository already models user-facing tools as top-level siblings
- `binutils` currently holds shared format tables and helpers, not standalone
  tools
- the linker will be a product/tool in its own right, even if it reuses
  `rxbin` helpers from `binutils`

Recommended working name in this note:

- use `rxlink` as the placeholder tool name
- the final executable name can still be approved separately

### 8.3 Stage-3 product model

The linker should be treated as a standalone build-time tool with two inputs:

- command-line arguments for the common/simple case
- a control file for repeatable builds and manual member selection overrides

The intended job is:

- accept one or more `.rxbin` inputs
- treat each serialized module record as a candidate member
- choose which members to include
- emit one linked/package output

The mainframe-style control-file requirement is sensible and should be part of
stage 3 from the start, not an afterthought.

### 8.4 Command-line and control-file model

Recommended control model:

- CLI for simple direct use:
  - output path
  - explicit root inputs
  - optional control file path
  - optional map/report output
- control file for:
  - forced include
  - forced omit
  - explicit roots
  - reusable input lists
  - output/map defaults

Recommended control-file style:

- simple line-oriented control statements
- case-insensitive keywords
- comments allowed
- no heavy parser needed

Recommended minimum directives:

- `OUTPUT <file>`
- `INPUT <file>`
- `ROOT <member>`
- `INCLUDE <member>`
- `OMIT <member>`
- `MAP <file>`

Recommended member naming rule:

- bare module/member name is allowed when unique
- otherwise the qualified form should be `input-path::module-name`

Recommended precedence:

- `OMIT` removes a candidate from automatic resolution
- `INCLUDE` force-adds a member even if not otherwise referenced
- omitting a forced root should be an error
- omitting a needed provider should surface as an unresolved-symbol error

### 8.5 Automatic member selection

The existing `rxvm_link()` logic is the right starting point for symbol
resolution, but automatic member selection must go slightly beyond it.

Recommended dependency rules:

- hard dependency edges come from imported `EXPOSE_PROC_CONST` entries
- interface/factory metadata should also create dependency edges:
  - `META_IMPLEMENTS` in an included module should pull in the matching
    interface-defining module when present
- exposed registers should not, by themselves, force module inclusion

Reason:

- runtime linking already resolves procedure imports by exposed name
- runtime interface/factory registries are built by scanning metadata across
  all loaded modules, so contract modules matter for correct behaviour
- exposed registers are soft shared-state bindings and do not imply that some
  other module must exist

Recommended ambiguity rule:

- duplicate providers should be a linker error by default
- `INCLUDE` / `OMIT` should be the way to disambiguate intentionally

This is stricter than the current runtime linker, which mostly warns and keeps
the first exporter inserted into the search tree. The build-time linker should
prefer deterministic and reviewable output over silent first-wins behaviour.

### 8.6 Unified `003` record-stream container

Stage 3 now uses one unified top-level `rxbin` shape in `003`, but it remains
streamable and concatenable by raw byte copy.

The file is a sequence of self-delimiting records read until EOF.

Record types:

- `MODULE_LOCAL`
  - one code module plus its private constant pool
  - this is what `rxas` emits
- `POOL_SHARED`
  - one shared constant-pool blob with no code section
  - this is emitted by `rxlink`
- `MODULE_SHARED`
  - one code module/header with no local pool payload
  - this is emitted by `rxlink`

Reader model:

- `MODULE_LOCAL` loads normally and ignores any current shared pool
- `POOL_SHARED` installs or replaces the current shared pool
- `MODULE_SHARED` attaches to the current shared pool without copying it

Consequences:

- ordinary single-module output is still just one serialized record
- plain archive creation remains byte concatenation of existing files
- linked images are written as one `POOL_SHARED` record followed by one or
  more `MODULE_SHARED` records
- concatenating multiple linked images is valid because each new
  `POOL_SHARED` record resets the active shared pool

### 8.7 Shared-pool rewrite strategy

One shared constant pool with separate code modules is feasible and is now the
implemented first pass.

What stays module-local:

- instruction/code sections
- procedure start addresses
- metadata instruction addresses
- per-module globals count and chain heads

What becomes shared within a linked image:

- the constant-pool address space
- literal constants
- procedure/expose/meta record storage

What the linker rewrites:

- module header offsets such as `proc_head`, `expose_head`, and `meta_head`
- instruction operands that carry constant-pool indexes
- internal constant-pool references such as:
  - `proc_constant.exposed`
  - `expose_proc_constant.procedure`
  - metadata string/type/arg/member references
  - metadata `prev` / `next` links

First-pass consolidation policy:

- copy all module-structural records into the shared pool
- deduplicate only leaf literal constants across members:
  - `STRING_CONST`
  - `BINARY_CONST`
  - `DECIMAL_CONST`
  - `FLOAT_CONST`
- do not try to deduplicate `PROC_CONST`, `EXPOSE_*`, or `META_*` records in
  the first pass

### 8.8 Why code modules stay separate

Advantages:

- no need to flatten all code into one giant instruction stream
- no cross-module code-address relocation for `proc.start` or `meta.address`
- the runtime execution model still treats procedures as belonging to a
  specific code segment/module
- debugging and disassembly remain module-oriented
- the linker work stays focused on member selection and constant-pool rebasing

Main cost:

- all module-local walkers must now follow `proc_head`, `expose_head`, and
  `meta_head` rather than sweeping the whole constant pool
- all shared-pool references must be rebased consistently

Overall recommendation:

- keep code modules separate in stage 3
- do not flatten instruction addressing in the first linker pass

### 8.9 What was reused from `rxvm`

The linker reuses the following ideas from `rxvm`:

- catalog exports by exposed procedure name
- resolve imported procedures against that catalog
- reuse the existing `read_module()` / `read_module_mem()` entry points
- reuse the current notion that one input file may contain multiple module
  members

Important differences from `rxvm`:

- the linker decides inclusion/exclusion of members before runtime
- the linker rewrites serialized offsets into a shared-pool address space
- duplicate providers are deterministic linker errors rather than runtime
  warnings
- unresolved imports are preserved as imported stubs so later runtime loading
  can still satisfy them if desired

### 8.10 Current implementation status

Stage 3 is now implemented in `003`.

Implemented pieces:

- standalone top-level tool: `rxlink`
- CLI options:
  - output path
  - control file path
  - explicit roots
  - optional map file
  - optional location/debug flags
- control-file directives:
  - `INPUT`
  - `ROOT`
  - `INCLUDE`
  - `OMIT`
  - `OUTPUT`
  - `MAP`
- selector forms:
  - bare member name
  - qualified `input-path::member`
- automatic member selection:
  - explicit roots/includes first
  - otherwise `main()`-containing modules, then first-input fallback
  - import closure over exposed procedures
  - interface/implements closure over metadata
- linked-image writer:
  - one `POOL_SHARED` record
  - one `MODULE_SHARED` record per selected module
  - one deduplicated shared pool across those modules

Supporting runtime/tool changes that were required:

- `read_module()` and `read_module_mem()` now understand all three record
  types
- shared-pool modules borrow the active pool without copying it
- `rxvm`, compiler import scanning, and `rxdas` now use module-local
  `proc_head` / `expose_head` / `meta_head` traversal where needed

### 8.11 Non-goals for the first linker pass

The first linker does not try to replace runtime linking entirely.

Runtime linking is still needed for:

- `METALOADMODULE` and similar late-load cases
- native plugins
- mixed static/dynamic deployments

The first linker also does not yet try to:

- flatten all code into one instruction image
- strip every unused metadata record
- canonicalize or deduplicate all structural metadata records
- remove every imported-procedure stub from the serialized image
- preserve shared-pool packaging when disassembling and reassembling through a
  single `.rxas` file; that repackaging step remains the linker's job

### 8.12 Later linker wins

Later linker passes could additionally:

- rewrite imported-procedure stubs to direct provider procedure references and
  then drop redundant imported records
- strip unused exports or metadata
- deduplicate selected structural metadata records
- precompute more relocation/import information
- optionally define a later fully flattened single-code-image format if that
  ever becomes worthwhile

Those are useful extensions, but they are not required for the first stage-3
milestone.

## 9. Cross-cutting constraints

The whole programme should preserve these properties:

- `rxbin` remains platform-neutral on supported targets
- the hot interpreter loop continues to see normal decoded data structures
- debugging, disassembly, and diagnostics keep working on new files
- versioning is explicit rather than heuristic

## 10. Approval points

The main approval points from this note are:

1. the whole unpublished programme should stay on the final `003` layout with
   no backward compatibility for `001` or earlier unpublished `003` variants
2. stage 1 should keep the existing float instructions and make their float
   operands pool-backed in `003`
3. the serialized/runtime state split should be added to the programme as the
   next structural step after stage 1 and before compression/linking
4. stage 2 uses a two-part load-time decompression design inside `003`:
   - packed logical token stream for the instruction section
   - lightweight LZSS blob compression for the constant pool
5. stage 3 should add a standalone top-level linker tool rather than placing
   the tool itself under `binutils`
6. stage 3 should support both CLI arguments and a mainframe-style control file
   with `INCLUDE` / `OMIT` overrides
7. stage 3 should use a unified record-stream `rxbin` container with
   `MODULE_LOCAL`, `POOL_SHARED`, and `MODULE_SHARED` records
8. stage 3 linked output should use one shared constant pool with separate
   per-module code sections
9. stage 3 automatic member selection should follow both imported procedures
   and interface/factory metadata dependencies
10. stage 3 should treat duplicate providers as linker errors unless the user
    disambiguates them explicitly

## 11. Potential follow-up after stage 3

With stages 1, 2, and 3 now implemented, the next worthwhile follow-up items
would be:

- decide whether to rewrite imported stubs directly to provider procedures
- decide whether unresolved imports should remain runtime-linkable or become
  hard linker errors
- decide whether a later optional code-flattening mode is worth the extra
  relocation complexity
