# Metadata And Introspection

These instructions expose runtime type information, loaded-module and procedure
tables, raw instruction/metadata records, and dynamic procedure selection. Most
module and instruction indexes are deliberately low-level: module numbers are
one-based, instruction addresses are zero-based, and callers must supply valid
values unless an instruction's entry states otherwise.

## `asserttype`

Require a runtime value to match a requested source type name.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x003d` | `asserttype rValue,"source.Type"` | Continue only when the runtime type matches. |

### Operands And Semantics

The type descriptor is a string constant. Matching uses the runtime object and
interface registry, not a string comparison alone, and leaves `rValue` wholly
unchanged.

### Signals

Raises `CONVERSION_ERROR` on mismatch, with actual/requested type detail when
the diagnostic message can be allocated.

### Example

<!-- rxas-example name="metadata-asserttype" test="run" -->
```rxas
.globals=0

main() .locals=1
    setobjtype r0,"example.Widget"
    asserttype r0,"example.Widget"
    ret
```

### Related

`istype`, `typeof`, `setobjtype`.

## `getandtp`

Read selected status-flag bits from a value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x020a` | `getandtp rFlags,rValue,mask` | Store `rValue.readable_flags & mask`. |

### Operands And Semantics

The literal becomes a 32-bit mask and is restricted by
`RXFLAG_READABLE_MASK`, so the reserved sign bit is never returned. Only the
destination integer payload changes; source and flags are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="metadata-getandtp" test="run" -->
```rxas
.globals=0

main() .locals=2
    setortp r1,512
    getandtp r0,r1,512
    ret
```

### Related

`gettp`, `settpmask`, `brtpandt`.

## `gettp`

Read all nonreserved status flags from a value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0204` | `gettp rFlags,rValue` | Store `rValue.flags & 0x7fffffff`. |

### Operands And Semantics

The result includes readable VM-private bits plus compiler, library, and user
bands, but excludes the reserved sign bit. Only `rFlags`' integer payload is
written.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="metadata-gettp" test="run" -->
```rxas
.globals=0

main() .locals=2
    setortp r1,512
    gettp r0,r1
    ret
```

### Related

`getandtp`, `settp`, `setortp`.

## `istype`

Probe whether a runtime value matches a requested source type.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x003c` | `istype rResult,rValue,"source.Type"` | Store integer `1` for a match, otherwise `0`. |

### Operands And Semantics

The destination is cleared and receives an integer Boolean. Matching consults
runtime concrete/interface relationships. The source value is unchanged, and
a mismatch is an ordinary false result.

### Signals

This non-raising probe does not signal for a type mismatch.

### Example

<!-- rxas-example name="metadata-istype" test="run" -->
```rxas
.globals=0

main() .locals=2
    setobjtype r1,"example.Widget"
    istype r0,r1,"example.Widget"
    ret
```

### Related

`asserttype`, `typeof`.

## `metadecodeinst`

Decode an opcode number into a seven-field instruction-description object.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0212` | `metadecodeinst rDescription,rOpcode` | Look up the VM instruction metadata map. |

### Operands And Semantics

The destination is cleared and receives seven attributes: opcode integer,
instruction name string, description string, operand count, and the three
operand-type codes. `rOpcode` supplies an integer and remains unchanged.

### Signals

There is no opcode-range check or VM signal; `rOpcode` must be a valid metadata
map index. Attribute allocation failure is fatal.

### Example

<!-- rxas-example name="metadata-metadecodeinst" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,48
    metadecodeinst r0,r1
    ret
```

### Related

`metaloadinst`, `metaloadioperand`, `metaloadsoperand`.

## `metalinkpreg`

Temporarily bind a local register name to a register in the caller's frame.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0218` | `metalinkpreg rLocal,rIndex` | Link `rLocal` to caller register number `rIndex.int`. |

### Operands And Semantics

`rIndex` supplies a zero-based integer index into the immediate parent frame's
local-register table. The instruction rebinds the destination register slot;
subsequent reads and writes through `rLocal` access the caller's complete value,
including its cursor, attributes, type metadata, and flags. `unlink rLocal`
restores the callee's original local-register binding.

### Signals

This instruction performs no parent-frame or index validation and does not
translate invalid access into a VM signal. It must be used only in a called
procedure with a valid caller-local index.

### Example

<!-- rxas-example name="metadata-metalinkpreg" test="assemble" -->
```rxas
.globals=0

main() .locals=1
    ret

worker() .locals=2
    metalinkpreg r0,r1
    unlink r0
    ret
```

### Related

`link`, `unlink`.

## `metaloadcalleraddr`

Build a two-field object identifying the current procedure's call site.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0219` | `metaloadcalleraddr rAddress` | Store `[caller module, return address]`. |

### Operands And Semantics

The destination is cleared and receives two integer attributes. Attribute zero
is the caller module number; attribute one is the return program-counter offset
within the caller module. In a root frame, both fields are `-1`. No source
register is read.

### Signals

This instruction does not signal. Attribute-allocation failure is fatal.

### Example

<!-- rxas-example name="metadata-metaloadcalleraddr" test="run" -->
```rxas
.globals=0

main() .locals=1
    call worker()
    ret

worker() .locals=1
    metaloadcalleraddr r0
    ret
```

### Related

`metaloadmodule`, `metaloadinst`.

## `metaloaddata`

Load all metadata records attached to one instruction address in a module.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0217` | `metaloaddata rMetadata,rModule,rAddress` | Decode metadata at `rAddress.int`. |

### Operands And Semantics

`rModule` is a one-based loaded-module number and `rAddress` is a zero-based raw
instruction address. The destination is cleared; its integer payload and
attribute count become the number of records found. Each attribute is a tagged
metadata value. Implemented tags are `.meta_source_step`, `.meta_trace_event`,
`.meta_func`, `.meta_reg`, `.meta_const`, `.meta_clear`, `.meta_interface`,
`.meta_implements`, `.meta_member`, and `.meta_inline`, with their stored fields
in binary-record order. Class and attribute records are currently returned as
cleared, untagged entries. No match leaves an empty value with integer zero.

### Signals

There is no module or address bounds check and no translated VM signal. Inputs
must identify a loaded module and a valid address domain; allocation failure is
fatal.

### Example

<!-- rxas-example name="metadata-metaloaddata" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,1
    load r2,0
    metaloaddata r0,r1,r2
    ret
```

### Related

`metaloadinst`, `metaloadmodule`, `metadecodeinst`.

## `metaloadedeprocs`

Build an array describing a loaded module's locally exported procedures.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0210` | `metaloadedeprocs rArray,rModule` | Enumerate non-imported exposed procedures. |

### Operands And Semantics

`rModule` contains a one-based module number. `rArray` is cleared, its integer
payload is set to the entry count, and it receives that many attributes. Each
entry is a two-attribute object: exposed name string and runtime procedure
pointer in an integer payload. Imported exposure records are omitted.

### Signals

The opcode performs no module-number bounds check and raises no range signal;
callers must use a number returned by `metaloadedmodules`/`metaloadmodule`.
Allocation failure in legacy attribute helpers is fatal.

### Example

<!-- rxas-example name="metadata-metaloadedeprocs" test="run" -->
```rxas
.globals=0

main() .locals=2 .expose=main
    load r1,1
    metaloadedeprocs r0,r1
    ret
```

### Related

`metaloadedprocs`, `metaloadedmodules`, `metaloadmodule`.

## `metaloadedmodules`

Build an array containing every module name currently loaded in the VM.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x020e` | `metaloadedmodules rArray` | Enumerate modules in runtime number order. |

### Operands And Semantics

The destination is cleared, resized to `context.num_modules`, and receives one
module-name string per attribute. Its integer payload is also set to the count,
following the cREXX array convention. Attribute index zero corresponds to
one-based module number one.

### Signals

The instruction raises no VM signal; allocation failure in the legacy
attribute allocator is fatal.

### Example

<!-- rxas-example name="metadata-metaloadedmodules" test="run" -->
```rxas
.globals=0

main() .locals=1
    metaloadedmodules r0
    ret
```

### Related

`metaloadmodule`, `metaloadedprocs`, `metaloadedeprocs`.

## `metaloadedprocs`

Build an array describing every procedure record in a loaded module.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x020f` | `metaloadedprocs rArray,rModule` | Enumerate all procedure constants. |

### Operands And Semantics

`rModule` is one-based. The cleared destination receives one two-attribute
entry per procedure: procedure name and runtime procedure pointer. Its integer
payload equals the number of entries. Imported/internal procedure records are
included when present in the module procedure chain.

### Signals

There is no module-number bounds signal; invalid numbers access outside the
module array. Allocation failure is fatal.

### Example

<!-- rxas-example name="metadata-metaloadedprocs" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1
    metaloadedprocs r0,r1
    ret
```

### Related

`metaloadedeprocs`, `metaloadedmodules`, `metaloadmodule`.

## `metaloadfoperand`

Load a float constant referenced by one raw operand slot.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0214` | `metaloadfoperand rFloat,rModule,rAddress` | Resolve the slot's constant-pool index as `FLOAT_CONST`. |

### Operands And Semantics

The one-based module and raw slot address select an index into that module's
constant pool. The decoded binary64 value replaces the destination float
payload. Source registers are unchanged.

### Signals

No module, address, pool-index, or constant-type validation is performed;
incompatible inputs are undefined behavior.

### Example

<!-- rxas-example name="metadata-metaloadfoperand" test="assemble" -->
```rxas
.globals=0

main() .locals=3
    load r1,1
    load r2,1
    metaloadfoperand r0,r1,r2
    ret
```

### Related

`metaloadioperand`, `metaloadsoperand`, `metadecodeinst`.

## `metaloadinst`

Read an opcode from a loaded module's raw expanded instruction stream.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0211` | `metaloadinst rOpcode,rModule,rAddress` | Read the instruction slot at an address. |

### Operands And Semantics

`rModule` is one-based and `rAddress` is a raw `bin_code` slot index, not an
instruction ordinal. The opcode field is written to `rOpcode`'s integer
payload; sources are unchanged.

### Signals

No module/address bounds or slot-kind check is performed. Invalid inputs are
undefined behavior rather than a catchable signal.

### Example

This low-level example is assembly-tested because stable raw addresses depend
on the assembled module layout.

<!-- rxas-example name="metadata-metaloadinst" test="assemble" -->
```rxas
.globals=0

main() .locals=3
    load r1,1
    load r2,0
    metaloadinst r0,r1,r2
    ret
```

### Related

`metadecodeinst`, `metaloadioperand`, `metaloaddata`.

## `metaloadioperand`

Read an integer or index value from one raw operand slot.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0213` | `metaloadioperand rValue,rModule,rAddress` | Read the slot's `iconst` field. |

### Operands And Semantics

The module number is one-based and the address is a raw slot index, normally
the operand slot following an instruction. The slot bits are interpreted as a
VM integer/index and written to the destination integer payload.

### Signals

No module, address, or operand-type validation is performed. Callers must first
decode the instruction and select a compatible slot.

### Example

<!-- rxas-example name="metadata-metaloadioperand" test="assemble" -->
```rxas
.globals=0

main() .locals=3
    load r1,1
    load r2,1
    metaloadioperand r0,r1,r2
    ret
```

### Related

`metaloadinst`, `metadecodeinst`, `metaloadfoperand`.

## `metaloadmodule`

Load and link an RXBIN module named by a string register at runtime.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x020d` | `metaloadmodule rModule,rPath` | Load `rPath` and return the loader's module result. |

### Operands And Semantics

The VM NUL-terminates `rPath`'s private string buffer, calls the module loader,
prepares threaded dispatch tables for newly loaded modules when required, and
relinks the runtime on success. `rModule` receives the loader result, described
by the assembler as the one-based number of the last loaded module.

### Signals

This opcode does not translate loader failure into a VM signal; callers inspect
the integer result. Dispatch-table allocation failure is fatal.

### Example

The example is assembly-tested because a portable repository example cannot
name an additional runtime RXBIN that exists in every installation.

<!-- rxas-example name="metadata-metaloadmodule" test="assemble" -->
```rxas
.globals=0

main() .locals=2
    load r1,"library.rxbin"
    metaloadmodule r0,r1
    ret
```

### Related

`metaloadedmodules`, `metaloadedprocs`, `metaloadedeprocs`.

## `metaloadpoperand`

Load the procedure name referenced by one raw procedure operand slot.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0216` | `metaloadpoperand rName,rModule,rAddress` | Resolve a `PROC_CONST` and copy its name. |

### Operands And Semantics

The module is one-based and the address is a raw operand slot whose index must
refer to a procedure constant. The destination string is replaced and its
cursor resets. This returns only the name, not a callable procedure object.

### Signals

The VM performs no bounds or constant-type checks. The documented name-only
result reflects the current implementation's explicit incomplete-function
comment.

### Example

<!-- rxas-example name="metadata-metaloadpoperand" test="assemble" -->
```rxas
.globals=0

main() .locals=3
    load r1,1
    load r2,1
    metaloadpoperand r0,r1,r2
    ret
```

### Related

`metaloadsoperand`, `metaloadedprocs`, `metadecodeinst`.

## `metaloadsoperand`

Load a string constant referenced by one raw operand slot.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0215` | `metaloadsoperand rString,rModule,rAddress` | Resolve the slot index as `STRING_CONST`. |

### Operands And Semantics

The one-based module and raw operand address select a constant-pool record. The
destination string is replaced from that constant and its cursor resets;
source registers remain unchanged.

### Signals

No bounds, pool-index, or constant-type check is performed. Callers must decode
the instruction and operand format first.

### Example

<!-- rxas-example name="metadata-metaloadsoperand" test="assemble" -->
```rxas
.globals=0

main() .locals=3
    load r1,1
    load r2,1
    metaloadsoperand r0,r1,r2
    ret
```

### Related

`metaloadfoperand`, `metaloadpoperand`, `metadecodeinst`.

## `setortp`

OR selected public writable status flags into a value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0209` | `setortp rValue,flags` | Add compiler, library, or user-band bits. |

### Operands And Semantics

The literal is masked by `RXFLAG_PUBLIC_WRITABLE_MASK` before OR. Existing
flags remain set. VM-private and reserved bits in the literal are ignored;
payloads and cursors remain unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="metadata-setortp" test="run" -->
```rxas
.globals=0

main() .locals=1
    setortp r0,512
    ret
```

### Related

`settp`, `settpmask`, `gettp`.

## `settp`

Update public writable status-flag bands while preserving VM-private state.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0205` | `settp rValue,flags` | Apply requested compiler, library, and user bands. |

### Operands And Semantics

The literal cannot write VM-private or reserved bits. If it requests no public
bit, all public bands are cleared. Otherwise each nonzero requested band
replaces that band while a zero band preserves its current bits. Payloads and
cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="metadata-settp" test="run" -->
```rxas
.globals=0

main() .locals=1
    settp r0,512
    ret
```

### Related

`setortp`, `settpmask`, `gettp`.

## `settpmask`

Replace selected source-writable status bits from an integer register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0203` | `settpmask rValue,rRequested,mask` | Masked replacement in library/user bands. |

### Operands And Semantics

Only bits selected by both the literal mask and
`RXFLAG_SOURCE_WRITABLE_MASK` (library plus user bands) change. Selected bits
take values from `rRequested`'s integer payload. Compiler, VM-private,
reserved, and unselected bits are preserved.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="metadata-settpmask" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,65536
    settpmask r0,r1,65536
    ret
```

### Related

`settp`, `setortp`, `getandtp`.

## `srcfprocsel`

Resolve the best registered factory procedure for an interface descriptor and
argument list.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x003a` | `srcfprocsel rProcedure,"descriptor",rCount` | Store the selected runtime procedure pointer. |

### Operands And Semantics

The descriptor is a string constant. `rCount` contains the integer argument
count; the argument values occupy the consecutive registers immediately after
`rCount`. Registered providers score those values, nonpositive matches are
rejected, and the highest score wins; equal scores are resolved by
fully-qualified class name. On success the destination is cleared and its
integer payload receives the opaque runtime procedure pointer. The count and
argument registers are unchanged.

### Signals

Raises `FUNCTION_NOT_FOUND` when no registered factory matches, normally with
resolver detail. Raises `FAILURE` if that diagnostic cannot be produced. The
argument block is not bounds-checked by the instruction.

### Example

<!-- rxas-example name="metadata-srcfprocsel" test="assemble" -->
```rxas
.globals=0

main() .locals=2
    load r1,0
    srcfprocsel r0,"rxsig1|example.Factory.create|.example.Factory|",r1
    ret
```

### Related

`srcmethodsel`, `call`, `calld`.

## `srcmethodsel`

Resolve a concrete runtime method for an object's registered class.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0039` | `srcmethodsel rProcedure,rObject,"descriptor"` | Store the resolved runtime procedure pointer. |

### Operands And Semantics

The method descriptor is a string constant. Resolution combines it with
`rObject`'s runtime class metadata. On success the destination is cleared and
its integer payload receives the opaque runtime procedure pointer. The object,
its attributes, cursor, and flags remain unchanged.

### Signals

Raises `OBJECT_NOT_INITIALIZED` for an explicitly uninitialized object. Raises
`FUNCTION_NOT_FOUND` when the value has no concrete runtime class or the class
does not provide the descriptor. A diagnostic-allocation failure raises
`FAILURE`.

### Example

<!-- rxas-example name="metadata-srcmethodsel" test="assemble" -->
```rxas
.globals=0

main() .locals=2
    setobjtype r1,"example.Widget"
    srcmethodsel r0,r1,"rxsig1|method|.unknown|"
    ret
```

### Related

`srcfprocsel`, `setobjtype`, `assertinitialized`.

## `typeof`

Return the canonical source spelling of a value's runtime object type.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x003b` | `typeof rTypeName,rValue` | Replace `rTypeName` with a source type string. |

### Operands And Semantics

An internal runtime class name is converted to its canonical source spelling.
Values without concrete object metadata return `.object`. The destination is
cleared, receives a string, and has its cursor reset; the source is unchanged.

### Signals

Raises `FAILURE` if the returned name cannot be allocated.

### Example

<!-- rxas-example name="metadata-typeof" test="run" -->
```rxas
.globals=0

main() .locals=2
    setobjtype r1,"example.Widget"
    typeof r0,r1
    ret
```

### Related

`istype`, `asserttype`, `setobjtype`.
