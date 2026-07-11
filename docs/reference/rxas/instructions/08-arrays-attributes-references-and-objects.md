# Arrays, Attributes, References, And Objects

These instructions manage attribute-backed arrays and objects, live register
and attribute bindings, first-class storage references, process redirect
endpoints, and runtime object type/initialization metadata.

Attribute counts and bulk edits distinguish zero-based forms from the `*1`
one-based forms. `link*` instructions rebind storage pointers rather than
copying payloads; `unlink*` restores retained base storage without copy-back.
Reference values track storage lifetime and raise `REFERENCE_INVALID` when a
raising operation encounters a dead target.

## `arr2redir`

Create a process-input redirect whose producer reads strings from an
attribute-backed array.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01d6` | `arr2redir rRedirect,rArray` | Feed array elements to a child-process input pipe. |

### Operands And Semantics

`rRedirect` is cleared and receives a VM-native redirect endpoint retaining a
pointer to `rArray`. The redirect must be passed to `spawn`, normally inside
its input/output/error redirect array, so process cleanup closes the pipe and
worker resources. The source array must remain live while the redirect runs.

### Signals

The constructor itself raises no VM signal. Allocation or pipe setup failure is
recorded in, or may leave absent, the endpoint and is reported by `spawn`.

### Example

This example is assembly-tested because a redirect is only safely executed as
part of a complete `spawn` lifecycle.

<!-- rxas-example name="redirect-arr2redir" test="assemble" -->
```rxas
.globals=0

main() .locals=2
    setattrs r1,0
    arr2redir r0,r1
    ret
```

### Related

`str2redir`, `redir2arr`, `nullredir`, `spawn`.

## `assertinitialized`

Require that a register is not marked as an uninitialized typed object.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0055` | `assertinitialized rObject` | Continue only when `rObject` is initialized. |

### Operands And Semantics

The register is inspected but not modified. Ordinary scalar and untyped values
are considered initialized; the check specifically recognizes the marker set
by `setobjuninit`.

### Signals

Raises `OBJECT_NOT_INITIALIZED` for a marked value, including its runtime type
name in the message when that message can be allocated.

### Example

<!-- rxas-example name="object-assertinitialized" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    assertinitialized r0
    ret
```

### Related

`isinitialized`, `setobjuninit`, `setobjtype`.

## `delattrs`

Delete a strict zero-based range of logical attributes.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x022b` | `delattrs rObject,rIndex,rCount` | Register index and count. |
| `0x022c` | `delattrs rObject,rIndex,count` | Register index, literal count. |
| `0x022d` | `delattrs rObject,index,rCount` | Literal index, register count. |
| `0x022e` | `delattrs rObject,index,count` | Literal index and count. |

### Operands And Semantics

The range is removed and later attributes shift left. Removed child storage is
reset and references to it become invalid. A zero count is a no-op, but its
index must still be in `0..logical_count`. Existing links move with surviving
attribute slots; private capacity may shrink after a large deletion.

### Signals

Raises `OUT_OF_RANGE` for negative register operands, an index outside the
allowed range, or a range extending past the logical count.

### Example

<!-- rxas-example name="attributes-delattrs" test="run" -->
```rxas
.globals=0

main() .locals=1
    setattrs r0,3
    delattrs r0,1,1
    ret
```

### Related

`delattrs1`, `insattrs`, `setattrs`.

## `delattrs1`

Delete a strict one-based range of logical attributes.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x022f` | `delattrs1 rObject,rIndex,rCount` | Register index and count. |
| `0x0230` | `delattrs1 rObject,rIndex,count` | Register index, literal count. |
| `0x0231` | `delattrs1 rObject,index,rCount` | Literal index, register count. |
| `0x0232` | `delattrs1 rObject,index,count` | Literal index and count. |

### Operands And Semantics

This is `delattrs` with the source index translated by subtracting one. Later
attributes shift left and removed storage is reset. For a zero count, valid
one-based insertion-style positions are `1..logical_count+1`.

### Signals

Raises `OUT_OF_RANGE` for an index less than one, negative count, or a range
past the logical attributes.

### Example

<!-- rxas-example name="attributes-delattrs1" test="run" -->
```rxas
.globals=0

main() .locals=1
    setattrs r0,3
    delattrs1 r0,2,1
    ret
```

### Related

`delattrs`, `insattrs1`, `setattrs`.

## `deref`

Copy the current value of referenced storage into a destination register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00c3` | `deref rDst,rReference` | Snapshot the referenced value into `rDst`. |

### Operands And Semantics

`rReference` must hold a reference payload created by `mkref`. `rDst` receives
a full value copy, including nested attributes; it is not linked to later
updates. The reference and target are unchanged.

### Signals

Raises `REFERENCE_INVALID` if the register has no live reference target.

### Example

<!-- rxas-example name="reference-deref" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"value"
    mkref r0,r1
    deref r2,r0
    ret
```

### Related

`mkref`, `linkref`, `setref`.

## `getabufs`

Read an object's current private attribute-storage capacity.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00dc` | `getabufs rCapacity,rObject` | Store `rObject.max_num_attributes`. |

### Operands And Semantics

`rCapacity` receives an integer count of allocated attribute slots, which may
exceed the logical count returned by `getattrs`. The source and all attribute
values are unchanged. Capacity is an implementation detail for diagnostics and
tuning, not a logical array bound.

### Signals

This instruction does not raise a signal.

### Example

<!-- rxas-example name="attributes-getabufs" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r1,2
    getabufs r0,r1
    ret
```

### Related

`getattrs`, `setattrs`, `minattrs`.

## `getattrs`

Return an object's logical attribute count, optionally with an integer bias.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00d6` | `getattrs rCount,rObject` | Store the logical attribute count. |
| `0x00d7` | `getattrs rCount,rObject,adjustment` | Store the count plus an integer literal. |

### Operands And Semantics

Only `rCount`'s integer payload is written. The object, attribute values,
links, and capacity are unchanged. The adjustment form is useful when adapting
between zero- and one-based source conventions.

### Signals

This instruction performs no bounds check and raises no signal; integer
addition follows the VM integer representation.

### Example

<!-- rxas-example name="attributes-getattrs" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r1,3
    getattrs r0,r1
    ret
```

### Related

`getabufs`, `setattrs`, `minattrs`.

## `insattrs`

Insert cleared attributes before a zero-based logical position.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0223` | `insattrs rObject,rIndex,rCount` | Register index and count. |
| `0x0224` | `insattrs rObject,rIndex,count` | Register index, literal count. |
| `0x0225` | `insattrs rObject,index,rCount` | Literal index, register count. |
| `0x0226` | `insattrs rObject,index,count` | Literal index and count. |

### Operands And Semantics

Valid indexes are `0..logical_count`; the end position appends. Existing
attribute storage pointers, values, links, and reference identities move right
with their logical elements. Inserted slots use cleared base storage. A zero
count changes nothing but still requires a valid index.

### Signals

Raises `OUT_OF_RANGE` for negative register operands, an index beyond the end,
or count/size arithmetic overflow.

### Example

<!-- rxas-example name="attributes-insattrs" test="run" -->
```rxas
.globals=0

main() .locals=1
    setattrs r0,2
    insattrs r0,1,1
    ret
```

### Related

`insattrs1`, `delattrs`, `setattrs`.

## `insattrs1`

Insert cleared attributes before a one-based logical position.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0227` | `insattrs1 rObject,rIndex,rCount` | Register index and count. |
| `0x0228` | `insattrs1 rObject,rIndex,count` | Register index, literal count. |
| `0x0229` | `insattrs1 rObject,index,rCount` | Literal index, register count. |
| `0x022a` | `insattrs1 rObject,index,count` | Literal index and count. |

### Operands And Semantics

Valid positions are `1..logical_count+1`; the last position appends. The VM
subtracts one and otherwise applies `insattrs` semantics, preserving the
identity and bindings of shifted attributes. A zero count is a no-op after
position validation.

### Signals

Raises `OUT_OF_RANGE` for an index less than one, a negative count, a position
past the append point, or size arithmetic overflow.

### Example

<!-- rxas-example name="attributes-insattrs1" test="run" -->
```rxas
.globals=0

main() .locals=1
    setattrs r0,2
    insattrs1 r0,2,1
    ret
```

### Related

`insattrs`, `delattrs1`, `setattrs`.

## `isinitialized`

Test whether a register is free of the uninitialized-object marker.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0056` | `isinitialized rResult,rValue` | Store integer `1` when initialized, otherwise `0`. |

### Operands And Semantics

The destination is first cleared and then receives an integer Boolean. The
source is unchanged. Scalars and untyped values return `1`.

### Signals

This non-raising probe does not signal for an uninitialized value.

### Example

<!-- rxas-example name="object-isinitialized" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1
    isinitialized r0,r1
    ret
```

### Related

`assertinitialized`, `setobjuninit`.

## `link`

Rebind one local register number to another register's current storage.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00d4` | `link rLocal,rTarget` | Make `rLocal` a live alias of `rTarget`. |

### Operands And Semantics

The current frame's register-pointer entry for `rLocal` is replaced. All later
payload, attribute, flag, reference, and cursor operations through `rLocal`
affect the target storage. Links may be chained. `unlink rLocal` restores that
register number's original base storage, not a copy of the target value.

### Signals

This instruction performs no copy or allocation and does not signal.

### Example

<!-- rxas-example name="attributes-link" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,"base"
    load r1,"target"
    link r0,r1
    load r0,"changed"
    unlink r0
    ret
```

### Related

`unlink`, `linkref`, `linkattr`.

## `linkarg`

Rebind a local register to argument storage in the current frame.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01ff` | `linkarg rLocal,index` | Link to argument slot `index`. |
| `0x0200` | `linkarg rLocal,rBase,adjustment` | Link to slot `rBase + adjustment`. |

### Operands And Semantics

Argument storage follows the frame's global and local register regions; the VM
adds those region sizes internally. `rLocal` becomes a live alias and can later
be restored with `unlink`. The first form uses an integer literal; the dynamic
form uses an integer register plus a literal.

### Signals

The VM performs no runtime argument-bound check in this opcode. The compiler or
hand-written caller must ensure the computed slot exists; an invalid slot is
undefined behavior rather than `OUT_OF_RANGE`.

### Example

This complete example is assembly-tested only because executing `worker`
requires a caller-created argument frame.

<!-- rxas-example name="attributes-linkarg" test="assemble" -->
```rxas
.globals=0

main() .locals=0
    ret

worker() .locals=1
    linkarg r0,0
    unlink r0
    ret
```

### Related

`link`, `unlink`, `linkref`.

## `linkattr`

Rebind a local register to a zero-based attribute storage slot.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00c8` | `linkattr rLocal,rObject,rIndex` | Use an integer register index. |
| `0x00c9` | `linkattr rLocal,rObject,index` | Use an integer literal index. |

### Operands And Semantics

`rLocal` becomes a live alias of the attribute's currently bound storage.
Writes through it update that attribute. `unlink rLocal` restores the local's
base storage. The object and index are otherwise unchanged.

### Signals

Raises `OUT_OF_RANGE` for a negative index or an index at or beyond the logical
attribute count.

### Example

<!-- rxas-example name="attributes-linkattr" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r1,1
    linkattr r0,r1,0
    load r0,"value"
    unlink r0
    ret
```

### Related

`linkattr1`, `linktoattr`, `unlink`.

## `linkattr1`

Rebind a local register to a one-based attribute storage slot.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ca` | `linkattr1 rLocal,rObject,rIndex` | Use an integer register index. |
| `0x00cb` | `linkattr1 rLocal,rObject,index` | Use an integer literal index. |

### Operands And Semantics

The VM subtracts one from the index and otherwise applies `linkattr` semantics.
The local remains a live alias until `unlink`; updates are not copied back as a
separate step.

### Signals

Raises `OUT_OF_RANGE` for an index less than one or greater than the logical
attribute count.

### Example

<!-- rxas-example name="attributes-linkattr1" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r1,1
    linkattr1 r0,r1,1
    load r0,"value"
    unlink r0
    ret
```

### Related

`linkattr`, `linktoattr1`, `unlink`.

## `linkref`

Rebind a local register to the storage named by a reference value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00c4` | `linkref rLocal,rReference` | Make `rLocal` a live alias of the reference target. |

### Operands And Semantics

The current frame's register binding is replaced with the target storage
pointer. Subsequent reads and writes through `rLocal` affect that storage until
`unlink rLocal` restores its original base storage. `rReference` is unchanged.

### Signals

Raises `REFERENCE_INVALID` when the reference has no live target.

### Example

<!-- rxas-example name="reference-linkref" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"before"
    mkref r0,r1
    linkref r2,r0
    load r2,"after"
    unlink r2
    ret
```

### Related

`mkref`, `unlink`, `setref`.

## `linktoattr`

Bind a zero-based attribute slot to external register storage.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00cc` | `linktoattr rIndex,rObject,rTarget` | Use an integer register index. |
| `0x00cd` | `linktoattr index,rObject,rTarget` | Use an integer literal index. |

### Operands And Semantics

The attribute pointer is replaced by `rTarget`'s currently bound storage.
Reads or writes through the attribute therefore affect `rTarget`. The
attribute's original base storage remains retained by the object and can be
restored with `unlinkattr`.

### Signals

Raises `OUT_OF_RANGE` for a negative index or an index at or beyond the logical
attribute count.

### Example

<!-- rxas-example name="attributes-linktoattr" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r0,1
    load r1,"external"
    linktoattr 0,r0,r1
    unlinkattr 0,r0
    ret
```

### Related

`linkattr`, `unlinkattr`, `linktoattr1`.

## `linktoattr1`

Bind a one-based attribute slot to external register storage.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ce` | `linktoattr1 rIndex,rObject,rTarget` | Use an integer register index. |
| `0x00cf` | `linktoattr1 index,rObject,rTarget` | Use an integer literal index. |

### Operands And Semantics

The VM subtracts one from the index and applies `linktoattr` semantics. The
link is live and does not copy the target value into base attribute storage.
`unlinkattr1` restores the retained base pointer.

### Signals

Raises `OUT_OF_RANGE` for an index less than one or beyond the logical count.

### Example

<!-- rxas-example name="attributes-linktoattr1" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r0,1
    load r1,"external"
    linktoattr1 1,r0,r1
    unlinkattr1 1,r0
    ret
```

### Related

`linkattr1`, `unlinkattr1`, `linktoattr`.

## `minattrs`

Ensure an object has at least a requested number of logical attributes without
shrinking an existing larger object.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00dd` | `minattrs rObject,rCount` | Use an integer register count. |
| `0x00de` | `minattrs rObject,count` | Use an integer literal count. |
| `0x00df` | `minattrs rObject,rBase,adjustment` | Require `rBase + adjustment`. |
| `0x00e0` | `minattrs rObject,base,adjustment` | Require the sum of two literals. |

### Operands And Semantics

If the requested count exceeds the logical count, new cleared, unlinked
attribute storage is appended. Otherwise the object is unchanged. Existing
attribute values and links are preserved.

### Signals

The VM performs no explicit negative, overflow, or allocation signal check in
these legacy forms. Portable RXAS must supply a nonnegative representable
count; allocation failure is fatal rather than catchable.

### Example

<!-- rxas-example name="attributes-minattrs" test="run" -->
```rxas
.globals=0

main() .locals=1
    setattrs r0,1
    minattrs r0,3
    ret
```

### Related

`setattrs`, `getattrs`, `getabufs`.

## `mkref`

Create a reference value that tracks a register's current storage.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00c2` | `mkref rReference,rSource` | Point `rReference` at `rSource` storage. |

### Operands And Semantics

Existing links on `rSource` are resolved first, so the reference names the
currently bound storage. `rReference` is cleared before receiving the reference
payload. The target is marked for lifetime tracking and remains live while its
owning storage remains live.

### Signals

Raises `FAILURE` with an out-of-memory message if a reference identity cannot
be allocated.

### Example

<!-- rxas-example name="reference-mkref" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"value"
    mkref r0,r1
    ret
```

### Related

`deref`, `linkref`, `unref`.

## `nullredir`

Create a bidirectional process redirect endpoint backed by the platform null
device.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01d7` | `nullredir rRedirect` | Open `/dev/null` or Windows `NUL` for child I/O. |

### Operands And Semantics

The destination is cleared and receives a native redirect payload with read and
write handles. Pass it to `spawn` for normal handle cleanup; copying the value
shares the reference-counted endpoint cell.

### Signals

The constructor raises no VM signal. OS open or allocation failure is retained
as redirect state, or leaves no endpoint, for later `spawn` handling.

### Example

<!-- rxas-example name="redirect-nullredir" test="assemble" -->
```rxas
.globals=0

main() .locals=1
    nullredir r0
    ret
```

### Related

`redir2str`, `str2redir`, `spawn`.

## `redir2arr`

Create a process-output redirect that appends captured output to an
attribute-backed array.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01d4` | `redir2arr rRedirect,rArray` | Capture child output into array elements. |

### Operands And Semantics

The destination is cleared and receives a native output endpoint retaining the
array register. Its reader worker grows the array as output arrives. The array
must stay live, and the redirect must be consumed by `spawn` for synchronized
cleanup.

### Signals

Construction raises no VM signal. Endpoint setup errors are deferred to the
redirect/`spawn` error path.

### Example

<!-- rxas-example name="redirect-redir2arr" test="assemble" -->
```rxas
.globals=0

main() .locals=2
    setattrs r1,0
    redir2arr r0,r1
    ret
```

### Related

`redir2str`, `arr2redir`, `spawn`.

## `redir2str`

Create a process-output redirect that appends captured bytes to a string.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01d3` | `redir2str rRedirect,rString` | Capture child output into `rString`. |

### Operands And Semantics

`rRedirect` is cleared and receives a native output endpoint retaining
`rString`. The reader worker appends output to the existing logical string.
Keep the string live and pass the endpoint to `spawn` so the worker and pipe
are closed and joined.

### Signals

Construction raises no VM signal. Allocation or pipe errors are reported by
the later `spawn` operation.

### Example

<!-- rxas-example name="redirect-redir2str" test="assemble" -->
```rxas
.globals=0

main() .locals=2
    load r1,""
    redir2str r0,r1
    ret
```

### Related

`redir2arr`, `str2redir`, `spawn`.

## `refvalid`

Probe whether a reference payload still names live storage.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00c6` | `refvalid rResult,rReference` | Store integer `1` for a live reference, otherwise `0`. |

### Operands And Semantics

The destination is cleared before receiving the Boolean. A non-reference,
cleared reference, or reference whose target lifetime ended returns `0`.
`rReference` is unchanged.

### Signals

This is the non-raising validity check and does not raise `REFERENCE_INVALID`.

### Example

<!-- rxas-example name="reference-refvalid" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"value"
    mkref r0,r1
    refvalid r2,r0
    ret
```

### Related

`mkref`, `unref`, `deref`.

## `setattrs`

Set an object's exact logical attribute count.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00d8` | `setattrs rObject,rCount` | Use an integer register count. |
| `0x00d9` | `setattrs rObject,count` | Use an integer literal count. |
| `0x00da` | `setattrs rObject,rBase,adjustment` | Set the count to a register plus a literal. |
| `0x00db` | `setattrs rObject,base,adjustment` | Set the count to two literals' sum. |

### Operands And Semantics

Growth appends cleared, unlinked attribute storage. Shrink resets removed
values, restores their base storage bindings, invalidates references to that
removed child storage, and may trim excess private capacity. Surviving
attributes retain their values and bindings.

### Signals

There is no explicit negative, overflow, or allocation signal check. Counts
must be nonnegative and representable; allocation failure is fatal.

### Example

<!-- rxas-example name="attributes-setattrs" test="run" -->
```rxas
.globals=0

main() .locals=1
    setattrs r0,3
    ret
```

### Related

`minattrs`, `getattrs`, `delattrs`.

## `setobjtype`

Stamp a value with a concrete runtime object type and mark it initialized.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0038` | `setobjtype rObject,"fully.qualified.Type"` | Set runtime type metadata. |

### Operands And Semantics

The type name is a string constant retained by pointer from the module constant
pool. Existing scalar, string, binary, decimal, attribute, reference, and
status payloads remain intact. Any uninitialized-object marker is cleared.

### Signals

This metadata update does not allocate or signal.

### Example

<!-- rxas-example name="object-setobjtype" test="run" -->
```rxas
.globals=0

main() .locals=1
    setobjtype r0,"example.Widget"
    assertinitialized r0
    ret
```

### Related

`setobjuninit`, `typeof`, `assertinitialized`.

## `setobjuninit`

Reset a value to an empty typed object placeholder and mark it uninitialized.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0054` | `setobjuninit rObject,"fully.qualified.Type"` | Clear the value, set its type, and mark it uninitialized. |

### Operands And Semantics

Unlike `setobjtype`, this instruction first performs `value_zero`: existing
payloads, attributes, flags, references, and native payloads are cleared. It
then stores the constant-pool type name and sets the uninitialized marker.

### Signals

The instruction itself does not signal. A later `assertinitialized` raises
`OBJECT_NOT_INITIALIZED` until initialization clears the marker.

### Example

<!-- rxas-example name="object-setobjuninit" test="run" -->
```rxas
.globals=0

main() .locals=2
    setobjuninit r0,"example.Widget"
    isinitialized r1,r0
    ret
```

### Related

`setobjtype`, `isinitialized`, `assertinitialized`.

## `setref`

Copy a complete value into storage named by a reference.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00c5` | `setref rReference,rSource` | Full-copy `rSource` into the reference target. |

### Operands And Semantics

The target receives all value payloads and attributes using normal full-copy
semantics. The reference object and source are unchanged. Other aliases of the
target observe the new value.

### Signals

Raises `REFERENCE_INVALID` if the reference target is no longer live.

### Example

<!-- rxas-example name="reference-setref" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"before"
    mkref r0,r1
    load r2,"after"
    setref r0,r2
    ret
```

### Related

`mkref`, `deref`, `linkref`.

## `str2redir`

Create a process-input redirect whose producer reads bytes from a string.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01d5` | `str2redir rRedirect,rString` | Feed `rString` to a child-process input pipe. |

### Operands And Semantics

The destination is cleared and receives a native input endpoint retaining
`rString`. The source must remain live while the worker writes it. Pass the
endpoint to `spawn` so the pipe, thread, and reference-counted endpoint cell are
cleaned up.

### Signals

Construction raises no VM signal. Setup errors are held in redirect state or
leave the endpoint absent and are surfaced by `spawn`.

### Example

<!-- rxas-example name="redirect-str2redir" test="assemble" -->
```rxas
.globals=0

main() .locals=2
    load r1,"input"
    str2redir r0,r1
    ret
```

### Related

`arr2redir`, `redir2str`, `spawn`.

## `unlink`

Restore a local register number's original base storage binding.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00d5` | `unlink rLocal` | Replace the live binding with the frame's base pointer. |

### Operands And Semantics

The value in aliased target storage is not copied into the restored local.
Instead, `rLocal` again exposes the storage allocated for that register when
the frame was created. Calling `unlink` on an already unlinked local simply
reassigns the same base pointer.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="attributes-unlink" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,"base"
    load r1,"target"
    link r0,r1
    unlink r0
    ret
```

### Related

`link`, `linkref`, `linkattr`.

## `unlinkattr`

Restore a zero-based attribute slot's original base storage binding.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00d0` | `unlinkattr rIndex,rObject` | Use an integer register index. |
| `0x00d1` | `unlinkattr index,rObject` | Use an integer literal index. |

### Operands And Semantics

The object's active attribute pointer is reset to its retained base pointer.
No value is copied from the external linked target, and that target is not
cleared. Calling the instruction on an already unlinked attribute is harmless.

### Signals

Raises `OUT_OF_RANGE` for a negative index or an index outside the logical
attribute range.

### Example

<!-- rxas-example name="attributes-unlinkattr" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r0,1
    load r1,"external"
    linktoattr 0,r0,r1
    unlinkattr 0,r0
    ret
```

### Related

`linktoattr`, `unlinkattr1`, `linkattr`.

## `unlinkattr1`

Restore a one-based attribute slot's original base storage binding.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00d2` | `unlinkattr1 rIndex,rObject` | Use an integer register index. |
| `0x00d3` | `unlinkattr1 index,rObject` | Use an integer literal index. |

### Operands And Semantics

After subtracting one from the index, the VM restores the retained base
attribute pointer without copying the linked target's current value.

### Signals

Raises `OUT_OF_RANGE` for an index less than one or greater than the logical
attribute count.

### Example

<!-- rxas-example name="attributes-unlinkattr1" test="run" -->
```rxas
.globals=0

main() .locals=2
    setattrs r0,1
    load r1,"external"
    linktoattr1 1,r0,r1
    unlinkattr1 1,r0
    ret
```

### Related

`linktoattr1`, `unlinkattr`, `linkattr1`.

## `unref`

Clear a reference value and release its retain on the reference cell.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00c7` | `unref rReference` | Clear every value payload in the register. |

### Operands And Semantics

The register is reset to an empty value, not merely stripped of its reference
payload. The referenced target storage itself is not overwritten or
invalidated; other reference values remain usable.

### Signals

This instruction does not signal, including when the register has no reference.

### Example

<!-- rxas-example name="reference-unref" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"value"
    mkref r0,r1
    unref r0
    ret
```

### Related

`mkref`, `refvalid`, `endlife`.
