# Level B References Working Design

Status: working design note, not an approved language specification.

This note captures the current direction for durable references in Level B. It
is deliberately design-space documentation: names, syntax, signal names, and
exact opcode spellings are placeholders until reviewed.

## Problem

Level B object values currently have clear copy and move behaviour, but there is
no durable user-visible reference concept. That matters for patterns such as a
collection returning an iterator over itself. A pure Rexx iterator either
receives a deep copy of the collection state or tries to retain object identity
that the VM does not currently model.

The immediate container pressure is:

- `ArrayList.iterator()` currently returns the backing `.string[]`, which is
  copied when returned from a linked object attribute.
- `ArrayListIterator` then copies that array into its own attribute.
- `StemListIterator` has the same problem at object scale because `.stem`
  contains array-backed state.
- Native-backed containers avoid the big copy by carrying integer tokens, but
  those tokens have weak ownership and lifetime semantics.

Adding a `this`-like receiver expression would solve only the naming problem. It
would not define ownership, copying, invalidation, or stack-frame lifetime.
Durable references need a VM-level design before the language can expose them
safely.

## Non-Goals

- Do not make ordinary object assignment pointer-sharing by default.
- Do not expose raw C pointers as stable Level B values.
- Do not rely on integer-sized pointer tricks as the semantic model.
- Do not require every existing opcode to implicitly chase references.
- Do not settle the final source syntax in this note.

## Current Baseline

- A Rexx object is a VM `value`, not a user-visible pointer. Object dispatch is
  based on stamped object type metadata.
- `copy_value(dest, source)` preserves the source and duplicates strings,
  decimals, binaries, ordinary object attributes, and native payloads according
  to the payload copy hook.
- `move_value(dest, source)` transfers owned buffers and attribute arrays into
  the destination and clears the source. `RET_REG` uses this only when returning
  a real local register. Returning an argument, global, or linked attribute is
  copied so the caller-visible source remains valid.
- Procedure calls initially link caller argument registers into callee locals.
  The compiler/emitter adds defensive copies for by-value writable formals when
  required by the language contract.
- `arg expose` already has useful call-time alias semantics: the callee
  operates on caller-owned state. It is not a durable reference that can safely
  be stored beyond the call.
- Attribute access uses VM pointer-array storage. `linkattr*` links a local
  register to an object/array attribute slot, and `unlink` restores the local
  register to VM-owned storage.
- The emitter currently uses compiler call/register flag bits:
  `REGTP_VAL = 0x00000100` and `REGTP_NOTSYM = 0x00000200`.

## Requirements

References should satisfy these constraints:

- Explicitness: copying an ordinary object still copies the object value.
  Reference sharing happens only through an explicit reference value.
- Safety: a stale reference must be detected before it is used.
- Lifetime: references to stack-frame storage must be invalidated or moved
  when that storage leaves scope.
- Move preservation: VM move optimisations must not silently break a reference
  that is still part of the live semantic value.
- Child storage: references must have a clear story for object/array attributes
  reached through `linkattr*`.
- RXAS first: the VM and assembler can support references before Level B source
  syntax is finalized.
- Container usability: pure Rexx iterators should be able to retain access to a
  live container without copying the container body.
- Native interop: native-backed containers should be able to use reference-like
  lifecycle validation rather than unchecked integer handles.

## Terms

- **Value**: the current VM `value` payload: scalar slots, string/binary/decimal
  buffers, object type metadata, and attributes.
- **Storage location**: a durable VM place that can hold a value for some
  lifetime, such as a local register, global register, argument register, or
  object/array attribute slot.
- **Reference value**: a normal VM value whose visible type is a reference. It
  contains a handle to a VM-managed reference cell.
- **Reference cell**: a VM-managed record that points at a storage location and
  tracks validity, lifetime, and diagnostics.
- **Target storage**: the storage location currently observed by a reference
  cell.
- **Reference payload**: the handle stored in a reference value.

## Recommended Core Model

The recommended model is a VM reference cell, not a raw register pointer.

A reference value points to a reference cell. The cell points to target storage.
The cell can be invalidated when the target storage dies, or retargeted when a
VM move operation transfers the storage identity to a new `value`.

This gives the VM a single place to answer:

- Is the reference still valid?
- Which storage location does it currently observe?
- Is the target a local, global, argument, attribute, native handle, or other
  future storage class?
- What should diagnostics say when the reference is stale?

### Why Not Store Raw Pointers In `.int`

Using `int_value` as a pointer or reference id is attractive for quick tests,
but it should not become the contract:

- it is unsafe on platforms where pointer width and `rxinteger` width differ;
- ordinary integer copies would look like pointer copies;
- finalization/refcount behaviour would be invisible to `copy_value()` and
  `clear_value()` unless every call site remembered a side table;
- stringification and tracing could accidentally expose meaningless addresses.

`int_value` may still carry a small debug id or registry index for diagnostics,
but the semantic handle should be a VM-private reference payload.

### Storage Choice

There are two plausible implementation routes:

1. Add dedicated reference fields to `struct value`, for example
   `reference_payload` for values that are references and `reference_identity`
   for values that are targets of references.
2. Prototype the reference payload through the existing native-payload/binary
   mechanism, with a core `rxvm_native_payload_ops` descriptor that retains and
   releases reference cells.

The dedicated-field route is the cleaner long-term VM model. The native-payload
route is a useful stepping stone because `copy_value()`, `move_value()`, and
`clear_value()` already centralize copy/finalizer hooks there. If the prototype
uses native payloads, the public design should still describe them as
references, not binary values.

## Copy, Move, and Clear Semantics

Reference semantics must be explicit at the central VM value-transfer points.

### Copying an Ordinary Value

Copying an ordinary non-reference value still produces an independent value:

```text
copy_value(dest, source)
```

If `source` is an ordinary object, strings, binaries, decimals, and attributes
are duplicated into destination-owned storage. Any references to `source`
continue to observe `source`, not `dest`.

### Copying a Reference Value

Copying a reference value copies the reference, not the referenced target:

```text
copy_value(dest_ref, source_ref)
```

The destination retains the same reference cell. Both reference values observe
the same target storage and see the same invalidation state.

### Moving an Ordinary Referenced Value

Moving is an internal VM transfer of storage identity:

```text
move_value(dest, source)
```

If `source` has an active reference identity cell, the move should transfer that
identity to `dest` and update the cell's target pointer to `dest`. This keeps VM
move optimisations from changing reference semantics.

This is especially important for:

- returning local objects from factories and procedures;
- moving temporary objects that already contain references to their own state;
- future compiler optimisations that materialize a receiver then move it.

### Moving a Reference Value

Moving a reference value transfers the reference payload to `dest` without
incrementing the cell retain count. The source becomes a normal empty value.

### Overwriting a Referenced Storage Location

A reference observes a storage location, not a frozen value snapshot. Assigning
a new value into that storage should not by itself invalidate the reference.
After the assignment, the reference observes the new contents.

This means the implementation needs to distinguish:

- clearing/replacing the contents of a still-live storage location;
- destroying the storage location itself.

The current `clear_value()` path is used in several contexts, so the
implementation should add explicit helper boundaries rather than hiding this
inside one ambiguous clear operation.

Candidate helpers:

- `clear_value_contents(v)`: replace contents while preserving reference
  identity for the storage location.
- `destroy_value_storage(v)`: invalidate any reference identity cell, then clear
  contents for final storage teardown.
- `move_value(dest, source)`: transfer reference identity from source to dest.

Names are placeholders. The important point is preserving the difference
between "slot still exists" and "slot lifetime ended".

## Lifetime Rules

### Stack Frame Locals And Arguments

References to frame-owned local storage are valid only while the frame storage
is valid.

When a frame exits:

- references to locals that were not moved to a live destination become invalid;
- references to arguments linked to caller storage remain governed by the
  ultimate caller storage, not by the callee's temporary local alias;
- references to callee-local aliases created by `link` or `linkattr*` should
  resolve to the underlying target storage when the reference is created.

This avoids references that accidentally die just because a short-lived linked
local went out of scope.

### Globals

References to global storage are valid while the owning runtime/module context
keeps that global storage alive. Module unload or VM teardown invalidates them.

### Object And Array Attributes

References to object/array attributes are references to child storage locations.
They are not references to the integer index text.

If an array insert shifts an element, a reference to the element should follow
the physical child value, not necessarily the old numeric index.

If an attribute slot is deleted, dropped, or its owning object storage is
destroyed, references to that child storage become invalid.

### Parent vs Child References

Both parent and child references are useful, but they mean different things:

- a reference to a parent object observes the whole object storage;
- a reference to an attribute observes the child storage currently occupying
  that attribute slot;
- a reference created from a local that is linked to an attribute should bind to
  the attribute storage, not to the temporary local alias.

This lets code do either:

```text
reference to container object
reference to container backing array attribute
```

without pretending they are the same lifetime.

### Native-Backed Storage

Native-backed containers should not expose raw pointer integers as the final
ownership model. A native provider can expose a VM storage location or handle
cell whose validity is checked by the same reference machinery.

For the existing tree/hash containers, a later cleanup can move from integer
tokens toward native payloads or reference cells with provider-specific retain,
release, and validity hooks.

## RXAS Surface

RXAS should get an explicit reference surface before source syntax is finalized.
The exact mnemonic names are open.

### Candidate Instructions

- `mkref rRef,rSource`
  Create a reference value in `rRef` to the storage location currently named by
  `rSource`. If `rSource` is linked to another storage location, the reference
  targets the ultimate linked storage.

- `deref rDest,rRef`
  Copy the current referenced value into `rDest`. If the reference is invalid,
  raise `REFERENCE_INVALID` or the chosen signal.

- `linkref rLocal,rRef`
  Link local register `rLocal` directly to the referenced storage. This is the
  RXAS operation needed for efficient code that wants to read and write through
  a reference. Existing `unlink rLocal` should restore the local register after
  the link. Invalid references raise a signal.

- `setref rRef,rSource`
  Copy `rSource` into the referenced storage without first linking a local.
  This is the reference equivalent of assignment-through-reference.

- `refvalid rOut,rRef`
  Set `rOut` to `1` when `rRef` is a valid reference and `0` otherwise.

- `unref rRef`
  Clear a reference value, releasing its hold on the reference cell.

### Why `linkref`, Not Overloaded `link`

Overloading the existing `link` instruction would be compact, but it makes
assembler review harder because a register's runtime contents would change the
meaning of the instruction. A separate `linkref` mnemonic makes aliasing
obvious to both humans and optimiser rules.

`mkref`, `linkref`, `setref`, and `unref` should be treated as optimiser
barriers until alias effects are fully modelled. They create storage identity,
not just value flow.

### Reference Invalid Signal

The design should probably add a dedicated invalid-reference signal rather than
folding this into a generic `ERROR`. Placeholder names:

- `REFERENCE_INVALID`
- `INVALID_REFERENCE`
- `REFERENCE_STALE`

The error payload should include a useful description such as the source symbol
or storage kind when available.

## VM Data Structures

One possible shape:

```c
typedef enum rxvm_ref_state {
    RXVM_REF_VALID,
    RXVM_REF_INVALID
} rxvm_ref_state;

typedef enum rxvm_ref_owner_kind {
    RXVM_REF_LOCAL,
    RXVM_REF_GLOBAL,
    RXVM_REF_ARGUMENT,
    RXVM_REF_ATTRIBUTE,
    RXVM_REF_NATIVE
} rxvm_ref_owner_kind;

typedef struct rxvm_reference_cell {
    uint64_t id;
    unsigned int retain_count;
    rxvm_ref_state state;
    rxvm_ref_owner_kind owner_kind;
    value *target;
    void *owner;
    uint64_t owner_generation;
    const char *debug_name;
} rxvm_reference_cell;
```

The VM should canonicalize one reference identity cell per live storage location
that has ever been referenced. `value` then needs a way to find that identity
cell when it is moved or destroyed.

Potential `value` additions:

```c
rxvm_reference_cell *reference_identity; /* this value is a ref target */
rxvm_reference_cell *reference_payload;  /* this value is a ref value */
```

If a prototype uses native payloads, `reference_payload` may temporarily live in
the binary/native-payload fields, but the same logical split remains useful:

- target identity: "someone may hold a reference to this storage";
- reference payload: "this value is itself a reference".

## Interaction With `link` And `linkattr`

Existing `link`, `linkattr*`, and `linktoattr*` already create storage aliases.
References must respect those aliases.

Rules:

- Creating a reference from an unlinked local targets that local storage.
- Creating a reference from a local linked to an argument, global, or attribute
  targets the ultimate storage, not the local alias.
- `linkref` creates the same kind of temporary local alias as `linkattr*`, but
  the target is looked up through the reference cell.
- `unlink` on a `linkref` local restores the local's original backing storage.
- `unlinkattr*` and bulk attribute deletion need to invalidate child reference
  cells whose storage is removed from the live attribute array.

This lets RXAS code write:

```rxas
mkref r10,r2       /* r10 references whatever storage r2 names */
linkref r3,r10     /* r3 now aliases the referenced storage */
...
unlink r3
```

without requiring every ordinary instruction to know about references.

## RXAS Performance Notes

References should compose with, not replace, existing and planned RXAS attribute
facilities.

The first performance win is avoiding unintended container copies. `linkref`
lets generated code alias a referenced storage location for a short region, so
normal RXAS instructions can operate on the local link without every opcode
having to learn implicit reference chasing.

The second performance win is reducing repeated attribute lookup. This belongs
beside the existing `linkattr*` family and the planned register-attribute lookup
work, not inside the basic reference payload.

Possible later instructions:

- `mkrefattr rRef,rObj,attr`
  Create a reference directly to an object/array child storage slot.

- `linkrefattr rLocal,rObjRef,attr`
  Validate and dereference `rObjRef`, then link `rLocal` to one of the target
  object's attributes.

- `linkattrreg rLocal,rObj,rAttr`
  Use a precomputed or register-held attribute key/index to link an attribute
  without repeated symbolic lookup. This is the attribute-lookup optimisation
  already discussed for the RXAS roadmap, and it remains valuable with or
  without durable references.

These are deliberately second-phase candidates. The minimal reference
instruction set should be enough to express safe aliasing first. Once alias
effects are represented in tests, the assembler/compiler can add fused
reference-plus-attribute operations where profiling shows a real win.

## Source Syntax Candidates

No final syntax is proposed here. The source language should wait until the VM
and RXAS semantics are proven.

### Candidate A: Word Form

```rexx
countRef = reference count
listRef = reference items
```

Typed declaration sketch:

```rexx
countRef = .reference(.int)
listRef = .reference(.ArrayList)
```

This is the most REXX-like candidate: readable, word-oriented, and explicit.
The operand must be an aliasable storage location, not an arbitrary value
expression.

### Candidate B: Constructor-Like Form

```rexx
countRef = .ref(count)
listRef = .ref(items)
```

This fits existing type-constructor spelling, but it looks like an ordinary
function call even though it needs location semantics. That may hide the most
important part of the feature.

### Candidate C: Bracket Form

```rexx
countRef = [count]
countRef = [.int]
```

This is compact but too symbolic for the likely Level B style. It also sits
close to array indexing and could make parser and reader expectations worse.
Keep it as a discussion option, not the first recommendation.

### Candidate D: Parameter-Only First Step

Extend existing call-time aliasing before adding durable reference values:

```rexx
arg reference items = .ArrayList
```

This is safer as a language increment, but it does not solve iterators that need
to store a reference. It is useful only as a stepping stone.

### Recommended Syntax Direction

Prefer a word-form operator and an explicit reference type:

```rexx
iter = .ArrayListIterator(reference list)
ref = reference count
holder = .reference(.int)
```

The exact type spelling is open: `.reference(.int)` is clear but long;
`.ref(.int)` is shorter but less self-explanatory.

The compiler should reject:

```rexx
ref = reference (a + b)
ref = reference makeObject()
```

because those are values, not storage locations.

## Container Impact

Before references are implemented:

- pure Rexx iterators should be documented as snapshot iterators;
- pure Rexx containers should avoid claiming live iterator semantics;
- native-backed iterators can remain live only when their provider validates
  lifetime explicitly.

After references exist:

- `ArrayList.iterator()` can construct an iterator with a reference to the list
  object or directly to the backing `val` attribute.
- Referencing the parent object is cleaner for future mutation counters and
  bounds checks.
- Referencing the backing array is cheaper and may be sufficient for the first
  live iterator.
- A live iterator should probably carry an expected modification count once the
  collection API grows mutation tracking.

Example design sketch:

```rexx
ArrayListIterator: class
  list_ = .reference(.ArrayList)
  index_ = .int

  *: factory
    arg list = .reference(.ArrayList)
    list_ = list
    index_ = 0
    return
```

Generated RXAS would use `linkref` at method entry or at each operation point:

```rxas
linkref rObj,rListRef
...
unlink rObj
```

## Open Design Choices

1. Should references be weak by default, invalidating when target storage dies,
   or should some references keep heap-backed targets alive?
2. Should the first implementation allow references to attribute slots, or only
   top-level registers/globals?
3. If an array element is referenced and array attributes are reordered, should
   the reference follow the physical child value or the logical index? This note
   recommends following the physical child value.
4. Should `.reference(.T)` be assignable only to another `.reference(.T)`, or
   should checked casts between compatible reference types be allowed?
5. How should interface references work? A reference to `.Shape` can refer to a
   `.Box`, but dereferencing must preserve normal `istype`/`asserttype`
   behaviour.
6. Should invalid dereference be recoverable through the normal signal
   mechanism?

## Proposed Programme

1. Add VM reference-cell data structures and internal retain/release helpers.
2. Split value clearing into content replacement versus storage destruction
   where needed.
3. Teach `copy_value()`, `move_value()`, frame teardown, and attribute deletion
   about reference identity and reference payloads.
4. Add RXAS instructions: `mkref`, `deref`, `linkref`, `setref`, `refvalid`,
   and `unref`.
5. Add VM/RXAS tests covering copies, moves, frame exit, attribute references,
   invalid dereference, and `linkref`/`unlink`.
6. Use references internally in a small compiler-generated case before exposing
   source syntax.
7. Revisit source syntax with examples from containers, ADDRESS objects, and
   native-backed handles.
8. Convert `ArrayListIterator` or a minimal new test container to live
   reference-backed iteration.
