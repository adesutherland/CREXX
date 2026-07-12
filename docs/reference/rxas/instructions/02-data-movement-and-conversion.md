# Data Movement And Conversion

These instructions copy, transfer, clear, load, or bind register values. Scalar
loads change only their named payload unless stated otherwise; full-value copy
and move include every payload, cursor, attribute, type, flag, and reference
association.

## `acopy`

Copy the complete status-flag word between values.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x000d` | `acopy rDestination,rSource` | Copy all internal and public flags. |

### Operands And Semantics

Only `rDestination`'s status word changes. This includes VM-private validity and
cursor-related flags as well as compiler, library, user, and reserved bands;
payloads, cursors, attributes, and type metadata are untouched. The source is
unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="movement-acopy" test="run" -->
```rxas
.globals=0

main() .locals=2
    setortp r1,65536
    acopy r0,r1
    ret
```

### Related

`copy`, `icopy`, `scopy`, `gettp`.

## `copy`

Deep-copy a complete value while leaving the source intact.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0008` | `copy rDestination,rSource` | Replace destination with an independent copy. |

### Operands And Semantics

The destination's existing storage is replaced by copies of scalar, decimal,
string and binary payloads, both cursors, attributes recursively, object type,
status flags, and reference payload/identity state. Mutable buffers and
attributes are not aliased to the source. Self-copy is safe.

### Signals

There is no translated VM signal; allocation failure is fatal.

### Example

<!-- rxas-example name="movement-copy" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"value"
    copy r0,r1
    ret
```

### Related

`move`, `load`, `acopy`.

## `erase`

Reset a register to the VM's empty value state.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01fc` | `erase rValue` | Clear every value component. |

### Operands And Semantics

The register releases or clears scalar storage, decimal/string/binary payloads,
cursors, attributes, object type, flags, and reference associations, then
becomes a zero/empty value. The operation is semantically the same as `null`.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="movement-erase" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,"temporary"
    erase r0
    ret
```

### Related

`null`, `move`, `endlife`.

## `load`

Load a scalar literal or bind one register slot to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0001` | `load rDestination,integer` | Set integer payload. |
| `0x0002` | `load rDestination,float` | Set float payload. |
| `0x0003` | `load rDestination,"string"` | Set string payload/cursor from constant. |
| `0x0004` | `load rDestination,rSource` | Rebind destination slot to source value. |
| `0x0005` | `load rDestination,decimal` | Parse decimal constant through the plugin. |
| `0x0006` | `load destinationIndex,sourceIndex` | Rebind raw numbered register slots. |
| `0x0007` | `load destinationIndex,rSource` | Bind raw numbered slot to register value. |
| `0x00b7` | `load rDestination,binary` | Replace binary payload with literal bytes. |

### Operands And Semantics

Integer and float forms write only that scalar payload. String and binary forms
replace their payload and reset the matching cursor. Decimal form replaces the
decimal payload using the current decimal context. Register forms change the
frame's register-pointer table persistently: later reads and writes through the
destination access the same complete value as the source until another binding
operation changes it; they are not copies. Raw indexes are zero-based and are
not checked.

### Signals

Binary allocation failure raises `FAILURE`. Decimal parsing propagates the
decimal plugin's signal. Other forms do not signal; invalid raw register indexes
are not translated into a VM signal.

### Example

<!-- rxas-example name="movement-load" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,42
    ret
```

### Related

`copy`, `link`, `unlink`, `loadsettp`.

## `loadsettp`

Load a scalar literal and set its externally writable status flags atomically.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0206` | `loadsettp rDestination,integer,flags` | Load integer and replace public flags. |
| `0x0207` | `loadsettp rDestination,float,flags` | Load float and replace public flags. |
| `0x0208` | `loadsettp rDestination,"string",flags` | Load string and update public flag bands. |

### Operands And Semantics

Only compiler, library, and user flag bands are writable. Integer and float
forms replace all public-writable flags with the masked literal and discard
VM-private/reserved bits. The string form replaces its payload/cursor but uses
band-aware public write: VM-private flags survive, a zero public request clears
all public bands, and nonzero requested bands replace only those bands. Other
payloads and attributes remain intact.

### Signals

These forms do not signal; string allocation failure is fatal. Bits outside the
public-writable masks are ignored.

### Example

<!-- rxas-example name="movement-loadsettp" test="run" -->
```rxas
.globals=0

main() .locals=1
    loadsettp r0,7,65536
    ret
```

### Related

`load`, `settp`, `setortp`, `gettp`.

## `move`

Transfer a complete value and its owned storage to another register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01fd` | `move rDestination,rSource` | Move the value, then reinitialize source. |

### Operands And Semantics

The destination's old storage is destroyed, then all scalar payloads, buffers,
cursors, attributes, type metadata, flags, and reference associations transfer
from the source. The source becomes an empty initialized value. When both
operands resolve to the same value, the instruction is a no-op. This mnemonic
is retained for source compatibility but marked deprecated by the VM.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="movement-move" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"value"
    move r0,r1
    ret
```

### Related

`copy`, `erase`, `swap`.

## `null`

Reset a register to the VM's empty value state.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x000e` | `null rValue` | Clear every value component. |

### Operands And Semantics

All payloads and owned storage, both cursors, attributes, object metadata,
status flags, and reference associations are cleared or released. The register
remains allocated and can be reused immediately. This has the same runtime
implementation as `erase`.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="movement-null" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,9
    null r0
    ret
```

### Related

`erase`, `move`, `endlife`.
