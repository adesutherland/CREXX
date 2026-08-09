# Large And Fused Instructions

These instructions combine recurring compiler or assembler sequences into one
VM dispatch. They are ordinary public RXAS instructions, although most source
programs should let `rxc` or the RXAS keyhole optimiser select them.

Each fused form preserves the ordered state changes, signals, and externally
observable result of its documented component sequence. Opcode numbers for
withdrawn experimental forms remain reserved so an old or provisional RXBIN
cannot be decoded as a different instruction.

## `fdivsub`

Divide into the divisor register, then subtract a literal into a result.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0129` | `fdivsub rResult,rNumerator,rDivisor,float` | Set `rDivisor = rNumerator / rDivisor`, then `rResult = rDivisor - float`. |

### Operands And Semantics

Both writes are observable and occur in that order. The quotient is retained
in `rDivisor`; this is not the proposed result-only replacement that would keep
the intermediate in a VM-local C value.

### Signals

Like `fdiv` and `fsub`, this instruction does not signal for binary64 zero
division, overflow, underflow, infinity, or NaN.

### Example

<!-- rxas-example name="large-fdivsub" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,8.0
    load r2,2.0
    fdivsub r0,r1,r2,1.5
    ret
```

### Related

`fdiv`, `fsub`.

## `fmulticopy`

Multiply one float payload in place, then copy an independent integer payload.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x012a` | `fmulticopy rFloat,float,rIntResult,rIntSource` | Multiply `rFloat` by the literal, then integer-copy `rIntSource` to `rIntResult`. |

### Operands And Semantics

The float write happens before the integer copy. Only `rFloat.float` and
`rIntResult.int` change; the source integer and other payloads are unchanged.

### Signals

This instruction does not signal for binary64 overflow, underflow, infinity,
or NaN. The integer copy does not signal.

### Example

<!-- rxas-example name="large-fmulticopy" test="run" -->
```rxas
.globals=0

main() .locals=4
    load r0,4.0
    load r3,82
    fmulticopy r0,2.5,r2,r3
    ret
```

### Related

`fmult`, `icopy`.

## `igetunlink`

Read an integer through an alias and then restore that alias register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x011d` | `igetunlink rResult,rAlias` | Save `rAlias.int`, unlink `rAlias`, then write the saved integer to `rResult`. |

### Operands And Semantics

The value is saved before the binding is restored, so `rResult` receives the
aliased storage value. Only its integer payload changes.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-igetunlink" test="run" -->
```rxas
.globals=0

main() .locals=3
    setattrs r1,1
    linkattr1 r2,r1,1
    load r2,7
    igetunlink r0,r2
    ret
```

### Related

`icopy`, `linkattr1`, `unlink`.

## `iloadsetunlink`

Store an integer literal through an alias, then restore the alias register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x011e` | `iloadsetunlink rAlias,integer` | Load the integer through `rAlias`, then unlink it. |

### Operands And Semantics

The literal is written to the currently aliased storage before the local
register binding is restored to its frame-owned base value.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-iloadsetunlink" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r1,1
    linkattr1 r0,r1,1
    iloadsetunlink r0,21
    ret
```

### Related

`load`, `linkattr1`, `unlink`.

## `iloadsetunlinkn`

Store a literal through an alias and restore two local bindings.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0121` | `iloadsetunlinkn rAlias,integer,rOther` | Load through `rAlias`, then unlink `rAlias` and `rOther`. |
| `0x01a6` | `iloadsetunlinkn rLoaded,rAlias,integer,rOther` | Load `rLoaded`, copy its integer through `rAlias`, then unlink both aliases. |

### Operands And Semantics

The compact form has no intermediate register. The wider form deliberately
retains `rLoaded` because compiler TRACE metadata can observe that generated
write. Prefer the compact form only when that observation is not required.

### Signals

Neither form signals.

### Example

<!-- rxas-example name="large-iloadsetunlinkn" test="run" -->
```rxas
.globals=0

main() .locals=5
    setattrs r1,2
    linkattr1 r2,r1,1
    linkattr1 r3,r1,2
    iloadsetunlinkn r2,7,r3
    linkattr1 r2,r1,1
    linkattr1 r3,r1,2
    iloadsetunlinkn r4,r2,8,r3
    ret
```

### Related

`load`, `icopy`, `unlink`, `unlinkn`.

## `isetattr1`

Copy an integer directly into a constant one-based attribute.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0196` | `isetattr1 rObject,index,rSource` | Set the selected attribute integer payload from `rSource`. |

### Operands And Semantics

The object must already have the selected attribute. Only that attribute's
integer payload changes; no temporary alias register is created.

### Signals

Raises `OUT_OF_RANGE` when `index` is less than one or greater than the
object's logical attribute count.

### Example

<!-- rxas-example name="large-isetattr1" test="run" -->
```rxas
.globals=0

main() .locals=3
    setattrs r1,1
    load r2,35
    isetattr1 r1,1,r2
    ret
```

### Related

`icopy`, `linkattr1`, `setattrs`.

## `isetunlink`

Copy an integer through an alias, then restore that alias register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x011c` | `isetunlink rAlias,rSource` | Integer-copy `rSource` through `rAlias`, then unlink `rAlias`. |

### Operands And Semantics

Only the aliased target's integer payload is copied. The alias binding is
restored after the write; `rSource` is unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-isetunlink" test="run" -->
```rxas
.globals=0

main() .locals=4
    setattrs r1,1
    linkattr1 r2,r1,1
    load r3,20
    isetunlink r2,r3
    ret
```

### Related

`icopy`, `linkattr1`, `unlink`.

## `isetunlinkn`

Copy an integer through one alias, then restore two alias registers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x011f` | `isetunlinkn rAlias,rSource,rOther` | Store through `rAlias`, then unlink `rAlias` and `rOther`. |

### Operands And Semantics

The integer write precedes both binding restorations. The second alias is
restored without copying a value.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-isetunlinkn" test="run" -->
```rxas
.globals=0

main() .locals=5
    setattrs r1,2
    linkattr1 r2,r1,1
    linkattr1 r3,r1,2
    load r4,22
    isetunlinkn r2,r4,r3
    ret
```

### Related

`icopy`, `unlink`, `unlinkn`.

## `linksetattrslinkadd`

Link an outer attribute, size it, and link a calculated nested attribute.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x019c` | `linksetattrslinkadd rOuter,rObject,index,count,rNested,rBase,offset` | Link the outer attribute, set its count, then link nested index `rBase+offset`. |

### Operands And Semantics

The outer one-based index is checked before `rOuter` is linked. That linked
value is resized to `count`; the checked integer sum then selects the one-based
nested attribute for `rNested`. Both result registers remain aliases.

### Signals

Raises `OUT_OF_RANGE` for either invalid one-based index and
`OVERFLOW_UNDERFLOW` when `rBase+offset` is not representable. Attribute
allocation failure is fatal rather than catchable.

### Example

<!-- rxas-example name="large-linksetattrslinkadd" test="run" -->
```rxas
.globals=0

main() .locals=5
    setattrs r1,1
    load r2,2
    linksetattrslinkadd r3,r1,1,3,r4,r2,1
    unlink r4
    unlink r3
    ret
```

### Related

`linkattr1`, `setattrs`, `iadd`.

## `loadsettp2`

Load an integer into one register and update another register's public flags.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x010c` | `loadsettp2 rLoaded,integer,rStatus,flags` | Load `rLoaded`, then apply `settp` semantics to `rStatus`. |

### Operands And Semantics

The integer load is complete before the status update. The status operand uses
the same band-aware public write as `settp`; VM-private flags are preserved.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-loadsettp2" test="run" -->
```rxas
.globals=0

main() .locals=3
    loadsettp2 r1,11,r2,512
    ret
```

### Related

`load`, `settp`.

## `loadsettpswap`

Load an integer, set status flags, then exchange two complete bindings.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x010d` | `loadsettpswap rLoaded,integer,rStatus,flags,rOther` | Load, update `rStatus`, then swap `rStatus/rOther`. |

### Operands And Semantics

The three component effects occur in operand order. The final swap exchanges
complete register bindings, not just scalar payloads.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-loadsettpswap" test="run" -->
```rxas
.globals=0

main() .locals=4
    load r2,2
    load r3,3
    loadsettpswap r1,12,r2,256,r3
    ret
```

### Related

`load`, `settp`, `swap`.

## `minlinkattr1`

Ensure sufficient attribute capacity and link a one-based attribute.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0125` | `minlinkattr1 rAlias,rObject,index` | Grow as needed and link the literal index. |
| `0x0126` | `minlinkattr1 rAlias,rObject,rIndex,offset` | Grow as needed and link `rIndex+offset`. |

### Operands And Semantics

The object grows only when the selected index exceeds its logical count.
Existing attributes are preserved. `rAlias` remains bound to the selected
attribute until it is unlinked.

### Signals

Raises `OUT_OF_RANGE` for an index below one and
`OVERFLOW_UNDERFLOW` when the register-plus-offset sum is not representable.
Allocation failure is fatal rather than catchable.

### Example

<!-- rxas-example name="large-minlinkattr1" test="run" -->
```rxas
.globals=0

main() .locals=5
    minlinkattr1 r2,r1,3
    unlink r2
    load r4,3
    minlinkattr1 r2,r1,r4,1
    unlink r2
    ret
```

### Related

`minattrs`, `linkattr1`, `unlink`.

## `nulln`

Reset two, three, or four registers to empty values.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0112` | `nulln r1,r2` | Clear two complete values in order. |
| `0x0113` | `nulln r1,r2,r3` | Clear three complete values in order. |
| `0x0114` | `nulln r1,r2,r3,r4` | Clear four complete values in order. |

### Operands And Semantics

Each operand receives normal `null`/`value_zero` semantics: payloads, owned
storage, attributes, type metadata, flags, and references are reset.

### Signals

These forms do not signal.

### Example

<!-- rxas-example name="large-nulln" test="run" -->
```rxas
.globals=0

main() .locals=4
    load r0,"a"
    load r1,"b"
    load r2,"c"
    load r3,"d"
    nulln r0,r1,r2,r3
    ret
```

### Related

`null`, `erase`.

## `parseplan`

Execute a compiler-prepared PARSE descriptor without decoding a textual plan
at runtime.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x019a` | `parseplan rResults,rSource,"descriptor"` | Populate the reusable result vector from `rSource`. |

### Operands And Semantics

`rResults` receives one attribute per stored target; dropped fields do not
consume an attribute. The string constant is the versioned compact descriptor
emitted by `rxc`, not PARSE source text. Version 1 stores frozen item kinds and
flags, literal byte and character lengths, numeric movements, item count, and
result count in a portable little-endian payload.

Version 2 also supports dynamic delimiter and position items. Each such item
contains a compact index that selects either an earlier completed result or a
temporary external-value slot after the public result slots. Compiler-generated
code populates external slots before `parseplan`; the instruction reads them,
executes the plan, and shrinks `rResults` to its declared public result count.
Dynamic numeric values must convert to a nonnegative native integer.

The VM validates the header and each item boundary before using it. It reuses
the result vector's attribute storage across executions where possible. Normal
compiler lowering evaluates and copies the source first and then assigns vector
elements to source targets in order.

### Signals

Raises `INVALID_ARGUMENTS` for a malformed, truncated, unsupported-version, or
internally inconsistent descriptor. A dynamic numeric value that is not a
nonnegative native integer raises `CONVERSION_ERROR`. Allocation failure is
fatal.

### Example

`parseplan` is intended for compiler output. The final operand below is an
illustrative hex-string placeholder, not a complete hand-authored descriptor:

```rxas
    parseplan r0,r1,"<versioned-descriptor-bytes>"x
```

### Related

`parsepos2`, `parsewords3`, `parsewords3d`.

## `parsepos2`

Split a source at a fixed PARSE character position and capture the next
blank-delimited word.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0198` | `parsepos2 rPrefix,rWord,rSource,split` | Write the fixed prefix and the next word. |

### Operands And Semantics

`split` is the number of source characters placed in `rPrefix`. The VM then
skips ASCII blanks and writes the following blank-delimited word to `rWord`;
the remainder is discarded. Character counting is by Unicode code point in
UTF builds and by byte in non-UTF builds. Source/output aliasing is supported.

### Signals

If an aliased source must be snapshotted and allocation fails, the VM raises
`FAILURE`. Ordinary empty or short input produces empty/truncated fields rather
than a signal.

### Example

<!-- rxas-example name="large-parsepos2" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r2,"abcde next remainder"
    parsepos2 r0,r1,r2,5
    ret
```

### Related

`parseplan`, `parsewords3`, `parsewords3d`.

## `parsewords3`

Capture two blank-delimited words and the unparsed remaining tail directly
into three registers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0195` | `parsewords3 rFirst,rSecond,rTail,rSource` | Write two words and the remaining tail. |

### Operands And Semantics

The VM skips leading ASCII blanks before each of the first two fields. It then
writes the rest of the source, unchanged, to `rTail`. The instruction is also
the chaining primitive for longer eligible implicit-word templates. Any output
may alias the source; the VM snapshots source bytes before ordered writes when
needed.

### Signals

If an aliased source snapshot cannot be allocated, the VM raises `FAILURE`.
Empty or short input yields empty fields without a signal.

### Example

<!-- rxas-example name="large-parsewords3" test="run" -->
```rxas
.globals=0

main() .locals=4
    load r3,"one two three four"
    parsewords3 r0,r1,r2,r3
    ret
```

### Related

`parseplan`, `parsepos2`, `parsewords3d`.

## `parsewords3d`

Capture three blank-delimited words and discard the remaining tail.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0199` | `parsewords3d rFirst,rSecond,rThird,rSource` | Write three words and discard the tail. |

### Operands And Semantics

Leading ASCII blanks are skipped before every captured word. Text after the
third word is ignored. Any output may alias the source; the VM snapshots source
bytes before ordered writes when needed.

### Signals

If an aliased source snapshot cannot be allocated, the VM raises `FAILURE`.
Empty or short input yields empty fields without a signal.

### Example

<!-- rxas-example name="large-parsewords3d" test="run" -->
```rxas
.globals=0

main() .locals=4
    load r3,"one two three ignored"
    parsewords3d r0,r1,r2,r3
    ret
```

### Related

`parseplan`, `parsepos2`, `parsewords3`.

## `setlinkattr1`

Set exact attribute capacity and link a one-based attribute.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0123` | `setlinkattr1 rAlias,rObject,count,rIndex` | Resize to `count`, then link `rIndex`. |
| `0x0124` | `setlinkattr1 rAlias,rObject,count,rIndex,offset` | Resize, then link `rIndex+offset`. |

### Operands And Semantics

Capacity is set before the selected index is checked. Growth and shrink have
normal `setattrs` semantics; `rAlias` remains a live binding to the selected
attribute until `unlink`.

### Signals

Raises `OUT_OF_RANGE` for an invalid one-based index and
`OVERFLOW_UNDERFLOW` when the register-plus-offset sum is not representable.
Attribute allocation failure is fatal rather than catchable.

### Example

<!-- rxas-example name="large-setlinkattr1" test="run" -->
```rxas
.globals=0

main() .locals=5
    load r4,1
    setlinkattr1 r2,r1,4,r4
    unlink r2
    load r4,2
    setlinkattr1 r2,r1,4,r4,1
    unlink r2
    ret
```

### Related

`setattrs`, `linkattr1`, `unlink`.

## `setlinkiload`

Set attribute capacity, link an attribute, and load an independent integer.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x019f` | `setlinkiload rAlias,rObject,count,rIndex,rLoaded,integer` | Resize/link, then load the independent register. |

### Operands And Semantics

The object is resized first and the one-based register index is checked before
`rAlias` is linked. The integer load occurs only after a successful link.

### Signals

Raises `OUT_OF_RANGE` for an invalid selected index. Attribute allocation
failure is fatal rather than catchable.

### Example

<!-- rxas-example name="large-setlinkiload" test="run" -->
```rxas
.globals=0

main() .locals=5
    load r2,2
    setlinkiload r3,r1,4,r2,r4,42
    unlink r3
    ret
```

### Related

`setattrs`, `linkattr1`, `load`.

## `settpcall`

Set the final call-window value's flags, then call a named procedure.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x011a` | `settpcall rResult,function(),rCount,rValue,flags` | Apply `settp` to `rValue`, then execute the mapped call. |

### Operands And Semantics

`rCount.int` is the argument count and the call window begins at the next
numbered register. Hand-written RXAS must place `rValue` in that window as
required by the component sequence. The VM records the same window metadata as
ordinary `call`.

### Signals

The status update does not signal. The call propagates
`FUNCTION_NOT_FOUND`, frame-allocation failure, and callee/native signals as
documented for `call`.

### Example

<!-- rxas-example name="large-settpcall" test="run" -->
```rxas
.globals=0

main() .locals=12
    load r10,1
    load r11,43
    settpcall r0,return_arg(),r10,r11,512
    ret

return_arg() .locals=0
    ret a1
```

### Related

`settp`, `call`, `settpswapcall`.

## `settpswap`

Update one register's public flags, then exchange two complete bindings.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x010b` | `settpswap rStatus,flags,rOther` | Apply `settp` to `rStatus`, then swap `rStatus/rOther`. |

### Operands And Semantics

The status update happens before the pointer swap, so the updated complete
value is the one moved into `rOther`.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-settpswap" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,1
    load r2,2
    settpswap r1,256,r2
    ret
```

### Related

`settp`, `swap`.

## `settpswapcall`

Set and swap the final call-window value, then call a named procedure.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0119` | `settpswapcall rResult,function(),rCount,rValue,flags,rWindow` | Set `rValue`, swap it with `rWindow`, then call. |

### Operands And Semantics

`rCount.int` is the argument count and the call window begins at the next
numbered register. The status write and pointer swap happen before call
mapping. The call records the same window base/count metadata as ordinary
`call`, including for native signal unwind.

### Signals

The setup does not signal. Call resolution, allocation, native, and callee
signals are the same as `call`.

### Example

<!-- rxas-example name="large-settpswapcall" test="run" -->
```rxas
.globals=0

main() .locals=13
    load r10,1
    load r11,0
    load r12,42
    settpswapcall r0,return_arg(),r10,r12,256,r11
    ret

return_arg() .locals=0
    ret a1
```

### Related

`settp`, `swap`, `call`, `settpcall`.

## `settpswapsettpswap`

Prepare two values with the same status mask and swap each into place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0111` | `settpswapsettpswap rFirst,flags,rFirstWindow,rSecond,rSecondWindow` | Set/swap the first pair, then set/swap the second pair. |

### Operands And Semantics

The ordered sequence is `settp rFirst,flags`, swap the first pair,
`settp rSecond,flags`, then swap the second pair. Both swaps exchange complete
bindings.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-settpswapsettpswap" test="run" -->
```rxas
.globals=0

main() .locals=5
    load r1,1
    load r2,2
    load r3,3
    load r4,4
    settpswapsettpswap r1,512,r2,r3,r4
    ret
```

### Related

`settp`, `swap`, `settpswap`.

## `swapcall`

Swap the final call-window value into place, then call a named procedure.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0118` | `swapcall rResult,function(),rCount,rWindow,rValue` | Swap `rWindow/rValue`, then execute the mapped call. |

### Operands And Semantics

`rCount.int` is the argument count and its next numbered register is the first
argument. The pointer swap happens before the call window is mapped. Normal
return and cold signal unwind use the same window contract as ordinary `call`.

### Signals

The swap does not signal. The call has the same resolution, allocation,
native, and callee signals as `call`.

### Example

<!-- rxas-example name="large-swapcall" test="run" -->
```rxas
.globals=0

main() .locals=13
    load r10,1
    load r11,0
    load r12,41
    swapcall r0,return_arg(),r10,r11,r12
    ret

return_arg() .locals=0
    ret a1
```

### Related

`swap`, `call`, `settpswapcall`.

## `swapn`

Exchange two, three, or four register-binding pairs in operand order.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0108` | `swapn r1,r2,r3,r4` | Swap two pairs. |
| `0x0109` | `swapn r1,r2,r3,r4,r5,r6` | Swap three pairs. |
| `0x010a` | `swapn r1,r2,r3,r4,r5,r6,r7,r8` | Swap four pairs. |

### Operands And Semantics

Each pair uses ordinary `swap` pointer semantics, so complete values and their
storage move together. Pairs are processed from left to right; repeated
register operands therefore observe earlier swaps.

### Signals

These forms do not signal.

### Example

<!-- rxas-example name="large-swapn" test="run" -->
```rxas
.globals=0

main() .locals=4
    load r0,20
    load r1,22
    load r2,1
    load r3,2
    swapn r0,r1,r2,r3
    ret
```

### Related

`swap`.

## `swapsettp`

Exchange one pair, then update another register's public flags.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x010e` | `swapsettp rLeft,rRight,rStatus,flags` | Swap `rLeft/rRight`, then apply `settp` to `rStatus`. |

### Operands And Semantics

The pointer swap is complete before the band-aware public status update.
Operand aliasing therefore observes the post-swap binding.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-swapsettp" test="run" -->
```rxas
.globals=0

main() .locals=4
    load r1,1
    load r2,2
    load r3,3
    swapsettp r1,r2,r3,512
    ret
```

### Related

`swap`, `settp`.

## `swapsettpswap`

Swap one pair, update status, then swap the updated value with another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x010f` | `swapsettpswap rLeft,rRight,rStatus,flags,rOther` | Swap, set `rStatus`, then swap `rStatus/rOther`. |

### Operands And Semantics

All three effects occur left to right using complete binding swaps and
band-aware `settp` semantics.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-swapsettpswap" test="run" -->
```rxas
.globals=0

main() .locals=5
    load r1,1
    load r2,2
    load r3,3
    load r4,4
    swapsettpswap r1,r2,r3,256,r4
    ret
```

### Related

`swap`, `settp`, `settpswap`.

## `unlinkbr`

Restore a local register binding and then branch unconditionally.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0120` | `unlinkbr rAlias,label` | Unlink `rAlias`, then transfer control to `label`. |

### Operands And Semantics

The binding restoration happens before control transfers. There is no
fall-through path.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-unlinkbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r1,1
    linkattr1 r0,r1,1
    unlinkbr r0,done
    ret 1
done:
    ret
```

### Related

`unlink`, `br`.

## `unlinkn`

Restore two local register numbers to their frame-owned base storage.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x011b` | `unlinkn rFirst,rSecond` | Unlink both registers in operand order. |

### Operands And Semantics

Like two `unlink` instructions, this changes bindings rather than copying
values. Calling it on an already restored register simply assigns the same
base pointer.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="large-unlinkn" test="run" -->
```rxas
.globals=0

main() .locals=4
    setattrs r1,2
    linkattr1 r2,r1,1
    linkattr1 r3,r1,2
    unlinkn r2,r3
    ret
```

### Related

`unlink`, `linkattr1`.
