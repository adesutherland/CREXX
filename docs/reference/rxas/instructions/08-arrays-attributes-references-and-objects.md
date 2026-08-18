# Arrays, Attributes, References, And Objects

These instructions manage attribute-backed arrays and objects, live register
and attribute bindings, first-class storage references, and runtime object
type/initialization metadata.

Attribute counts and bulk edits distinguish zero-based forms from the `*1`
one-based forms. `link*` instructions rebind storage pointers rather than
copying payloads; `unlink*` restores retained base storage without copy-back.
Reference values track storage lifetime and raise `REFERENCE_INVALID` when a
raising operation encounters a dead target.

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
payload, attribute, flag, and reference operations through `rLocal`
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

## `stemget`

Read one string key from the private native stem representation.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0282` | `stemget rDst,rStem,rKey` | Copy the key's current value or the current default into `rDst`. |

### Operands And Semantics

`rStem` owns a private versioned hash-table representation. A receiver with no
binary payload is initialized lazily. `rKey` is hashed and compared as exact
UTF-8 bytes; the key and stem are not modified. A stored entry from an older
default generation reads as the current default until that key is written
again. Destination replacement is allocation-failure atomic.

### Signals

Raises `UNICODE_ERROR` for an invalid key, `FAILURE` for allocation failure,
capacity overflow, or corrupt private metadata.

### Example

<!-- rxas-example name="stem-stemget" test="run" -->
```rxas
.globals=0

main() .locals=4
    steminit r0
    load r1,"answer"
    load r2,"42"
    stemset r0,r1,r2
    stemget r3,r0,r1
    ret
```

### Related

`stemget2`, `stemset`, `stemreset`.

## `stemget2`

Read a two-segment key without materializing the joined key on the lookup path.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0285` | `stemget2 rDst,rStem,rLeft,rRight` | Read `rLeft || "." || rRight`. |

### Operands And Semantics

The VM streams both UTF-8 byte sequences and the single separator through the
same hash and equality contract as a materialized key. Hit and miss paths do
not allocate a key string. The logical key is therefore exactly equivalent to
one passed to `stemget`, including empty left or right segments.

### Signals

Raises `UNICODE_ERROR` for either invalid segment and the same `FAILURE`
conditions as `stemget`.

### Example

<!-- rxas-example name="stem-stemget2" test="run" -->
```rxas
.globals=0

main() .locals=5
    steminit r0
    load r1,"left"
    load r2,"right"
    load r3,"value"
    stemset2 r0,r1,r2,r3
    stemget2 r4,r0,r1,r2
    ret
```

### Related

`stemget`, `stemset2`.

## `steminit`

Initialize an empty receiver for native stem operations.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0281` | `steminit rStem` | Create an empty generation-zero native stem. |

### Operands And Semantics

The receiver's ordinary VM-owned binary payload holds the private
little-endian header, 256 bucket heads, and fixed-width entry metadata. Three
ordinary VM attribute values own insertion-ordered keys, insertion-ordered
values, and the current default. These bytes are process-local value state,
not an RXBIN section or public ABI. Initializing a receiver that already has a
binary payload is corruption rather than a reset.

### Signals

Raises `FAILURE` if the receiver already has binary state or if initial binary
or attribute storage cannot be allocated.

### Example

<!-- rxas-example name="stem-steminit" test="run" -->
```rxas
.globals=0

main() .locals=2
    steminit r0
    stemsize r1,r0
    ret
```

### Related

`stemget`, `stemset`, `stemreset`, `stemsize`.

## `stemkeyat`

Return a stored key by insertion-order position.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0288` | `stemkeyat rDst,rStem,rIndex` | Copy the one-based key at `rIndex`. |

### Operands And Semantics

Indexes are one-based and stable across default resets and existing-key
updates. Keys are added only after an absent lookup is proved, so a failed
insertion does not create an observable position. The destination receives a
string copy; the stem is unchanged.

### Signals

Raises `INVALID_ARGUMENTS` outside `1..stemsize`, or `FAILURE` for allocation
failure or corrupt private metadata.

### Example

<!-- rxas-example name="stem-stemkeyat" test="run" -->
```rxas
.globals=0

main() .locals=5
    steminit r0
    load r1,"first"
    load r2,"value"
    stemset r0,r1,r2
    load r3,1
    stemkeyat r4,r0,r3
    ret
```

### Related

`stemsize`, `stemvalueat`.

## `stemreset`

Replace a stem's default value in constant time.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0284` | `stemreset rStem,rDefault` | Copy the default and advance the generation. |

### Operands And Semantics

Reset scans no entries. It copies `rDefault`, then increments the receiver's
64-bit generation. Existing keys remain in insertion order but read as the new
default until individually rewritten in the new generation. The default and
generation are unchanged if allocation fails; generation wrap is rejected.

### Signals

Raises `UNICODE_ERROR` for an invalid default string and `FAILURE` for
allocation failure, generation overflow, or corrupt private metadata.

### Example

<!-- rxas-example name="stem-stemreset" test="run" -->
```rxas
.globals=0

main() .locals=3
    steminit r0
    load r1,"missing"
    stemreset r0,r1
    load r2,"key"
    stemget r2,r0,r2
    ret
```

### Related

`stemget`, `stemset`, `stemvalueat`.

## `stemset`

Insert or update one string key in a native stem.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0283` | `stemset rStem,rKey,rValue` | Set `rStem[rKey]` to `rValue`. |

### Operands And Semantics

An existing key is traversed once, its value is replaced, and its entry is
stamped with the current generation. An absent key is materialized once and
appended to insertion order after lookup. Key, value, receiver, and linked
storage aliases retain ordinary VM value semantics. Allocation failures leave
the logical entry count, bucket chains, values, and generation unchanged;
successfully reserved private capacity may remain available for retry.

### Signals

Raises `UNICODE_ERROR` for an invalid key or value and `FAILURE` for allocation
failure, capacity overflow, or corrupt private metadata.

### Example

<!-- rxas-example name="stem-stemset" test="run" -->
```rxas
.globals=0

main() .locals=3
    steminit r0
    load r1,"key"
    load r2,"value"
    stemset r0,r1,r2
    ret
```

### Related

`stemget`, `stemset2`, `stemreset`.

## `stemset2`

Insert or update a two-segment key with a streamed lookup.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0286` | `stemset2 rStem,rLeft,rRight,rValue` | Set `rStem[rLeft || "." || rRight]`. |

### Operands And Semantics

Hashing and equality stream the two segments plus one separator. Existing-key
updates allocate no joined key. A new insertion materializes the canonical
joined key exactly once after absence is proved and otherwise follows
`stemset` generation, ordering, alias, and failure-atomicity rules.

### Signals

Raises `UNICODE_ERROR` for either segment or the value and the same `FAILURE`
conditions as `stemset`.

### Example

<!-- rxas-example name="stem-stemset2" test="run" -->
```rxas
.globals=0

main() .locals=4
    steminit r0
    load r1,"left"
    load r2,"right"
    load r3,"value"
    stemset2 r0,r1,r2,r3
    ret
```

### Related

`stemget2`, `stemset`.

## `stemsize`

Return the number of stored keys in a native stem.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0287` | `stemsize rCount,rStem` | Store the insertion-order entry count. |

### Operands And Semantics

The count includes entries from earlier default generations because their keys
remain stored and extractable. Default resets and existing-key updates do not
change it. An empty receiver is initialized lazily and returns zero.

### Signals

Raises `FAILURE` for initial allocation failure or corrupt private metadata.

### Example

<!-- rxas-example name="stem-stemsize" test="run" -->
```rxas
.globals=0

main() .locals=2
    steminit r0
    stemsize r1,r0
    ret
```

### Related

`stemkeyat`, `stemvalueat`.

## `stemvalueat`

Return a value by insertion-order position with generation-aware defaulting.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0289` | `stemvalueat rDst,rStem,rIndex` | Copy the one-based entry value. |

### Operands And Semantics

For an entry written in the current generation, the destination receives its
stored value. An older entry receives the current default, matching `stemget`
for the corresponding key. Extraction does not update the entry generation or
otherwise mutate the stem.

### Signals

Raises `INVALID_ARGUMENTS` outside `1..stemsize`, or `FAILURE` for allocation
failure or corrupt private metadata.

### Example

<!-- rxas-example name="stem-stemvalueat" test="run" -->
```rxas
.globals=0

main() .locals=5
    steminit r0
    load r1,"first"
    load r2,"value"
    stemset r0,r1,r2
    load r3,1
    stemvalueat r4,r0,r3
    ret
```

### Related

`stemget`, `stemkeyat`, `stemreset`, `stemsize`.

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
