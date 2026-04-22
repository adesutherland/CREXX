# RXBIN Compaction Working Notes

Status: working, stage 1 and runtime-state split implemented
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

This note does not yet commit the exact stage-2 code-stream algorithm. That
still needs to be written down precisely before implementation.

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
- in `002`, keep the existing float instructions but store float operands as
  constant-pool indexes instead of inline `double` payloads in the code stream
- materialize the `double` from the pool during execution
- bump the file-format version from `001` to `002`

The preferred pool representation is:

- store the exact 64-bit payload, not a text form
- deduplicate by stored bit pattern, not by decimal spelling

That keeps the constant compact and avoids floating-point comparison problems in
the assembler.

### 5.3 Encoding decision

The stage-1 direction is now fixed:

- do not add sibling opcodes just for pool-backed floats
- keep the existing float instructions
- under file format `002`, interpret float operands as indexes of
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
- `001` and `002` are cleanly separated by the version gate
- it is acceptable for the updated tools to reject `001` rather than support
  dual decoding paths

### 5.5 Acceptance target

Stage 1 is complete when:

- repeated float literals are emitted once into the constant pool
- produced binaries execute with the same observable numeric behaviour as
  before
- the tools consistently emit and expect `002`
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

### 7.2 Code-stream compression direction

The code stream already uses fixed-width slots. The intended stage-2 direction
is:

- define a specific packing algorithm for the slot stream
- optimize for common small integers, frequent indexes, and repeated opcode
  patterns
- decompress into the existing in-memory `bin_code` form during module load

The exact algorithm is still `TBD`. This note only records the required shape:

- compact on-disk representation
- deterministic load-time expansion back to normal `bin_code` arrays
- no per-instruction decompression inside the execution loop

### 7.3 Constant-pool compression direction

Preferred direction:

- compress the serialized constant pool as a blob
- decompress once during load
- keep the in-memory constant-pool layout unchanged for the VM

Heatshrink is a plausible fit for the first pass because this is a file-size
optimization problem, not a random-access runtime format problem.

### 7.4 Header and loader implications

Stage 2 likely requires additional header information:

- compression flags per section
- compressed size
- uncompressed size

That lets the loader:

- detect whether a section is raw or compressed
- allocate the final buffer once
- restore the regular in-memory structures before linking or threading

### 7.5 Open definition needed before implementation

Before stage 2 starts, we still need:

- the exact code-stream packing algorithm
- the section-level header changes
- the compatibility story for mixed old/new modules

## 8. Stage 3: build-time linker

### 8.1 Problem statement

Today module linking happens at runtime. That is necessary for dynamic loading,
but it does not reduce file size for known module sets built together.

This stage should also assume that the supporting refactor in section 6 has
been completed first.

### 8.2 Preferred direction

Preferred direction for the first linker milestone:

- accept multiple module inputs
- resolve static imports/exports at build time where possible
- emit one packaged output
- retain module boundaries internally for correctness and simpler migration

There are two reasonable container directions:

1. reuse the existing "multiple serialized modules in one file" capability
2. define a new linked-container format

Preferred option: `1` for the first pass

Reason:

- the loader already understands repeated module records
- it minimizes the amount of new format machinery needed early
- it leaves room for a later fully linked single-image format if that becomes
  worthwhile

### 8.3 Non-goal for the first linker pass

The first linker should not try to replace runtime linking entirely.

Runtime linking is still needed for:

- `METALOADMODULE` and similar late-load cases
- native plugins
- mixed static/dynamic deployments

### 8.4 Later size wins the linker may unlock

A later linker pass could additionally:

- deduplicate shared string constants across input modules
- deduplicate shared float constants across input modules
- strip unused exports or metadata
- precompute more relocation/import information

Those are useful extensions, but they are not required to begin stage 3.

## 9. Cross-cutting constraints

The whole programme should preserve these properties:

- `rxbin` remains platform-neutral on supported targets
- the hot interpreter loop continues to see normal decoded data structures
- debugging, disassembly, and diagnostics keep working on new files
- versioning is explicit rather than heuristic

## 10. Approval points

The main approval points from this note are:

1. stage 1 should move to file format `002` with no backward compatibility for
   `001`
2. stage 1 should keep the existing float instructions and make their float
   operands pool-backed in `002`
3. the serialized/runtime state split should be added to the programme as the
   next structural step after stage 1 and before compression/linking
4. stage 2 is accepted as a two-part load-time decompression design:
   - code-stream packing algorithm to be specified
   - constant-pool compression via Heatshrink or similar
5. stage 3 should begin as a build-time packager/linker built on the existing
   multi-module file capability

## 11. Immediate next step after approval

With stage 1 and the runtime-state split complete, the next implementation
step should be stage 2:

- define the exact code-stream packing algorithm for the current 64-bit slot
  storage
- define how packed code is expanded back into normal `bin_code` arrays at
  load time
- add constant-pool compression with Heatshrink or equivalent load-time
  decompression
