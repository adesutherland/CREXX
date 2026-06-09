# Level B References Working Design

Status: working design note, not an approved language specification.

This note captures the current direction for durable references in Level B. It
is deliberately design-space documentation: the explicit Level B source spelling
is now implemented, while live member/index convenience syntax through
references, variance, and richer collection API choices remain later language
work. The convenience forms are treated as Level G candidates unless a future
Level B requirement proves they are essential.

## Agreed Direction

The following decisions are agreed for the VM/RXAS reference work and the first
Rexx source surface:

- References are weak. A reference does not keep stack, object, array, or global
  storage alive. When target storage dies, the reference becomes invalid. This is
  the intended end-state model, not just a prototype simplification.
- The first useful implementation should support references to locals,
  arguments, globals, and object/array attributes. Native storage references are
  out of scope for the reference work.
- A reference to an array element follows the physical child storage value when
  attribute slots move, rather than continuing to mean the old numeric index.
- Overwriting a live referenced storage location keeps the reference valid; the
  reference observes the new contents.
- Destroying or deleting referenced storage invalidates the reference. Later use
  raises a dedicated invalid-reference signal.
- Invalid-reference signals should participate in normal signal handling and be
  catchable, with the default action still expected to halt.
- The first post-reference pure Rexx container direction is unsynchronized live
  iteration. The broader Release 1 iterator policy, including snapshot or
  synchronized variants, remains part of the later Rexx surface/API discussion.
- The public return type of `Iterator.next()` is out of scope for the reference
  work and remains to be decided with the collection API.
- Native collection classes are deprecated for the Release 1 public collection
  direction. Native handle/reference migration is therefore not a blocker for
  this reference feature.
- Rexx source syntax direction is now agreed for the Level B essential source
  surface: word-form `reference`, `dereference`, and `snapshot` expressions, `refvalid(ref)`,
  and `reference .T` as the reference type modifier. Convenience live-access
  syntax such as `itemsRef[i]`, `itemsRef[i] = value`, and `listRef.add(value)`
  is not required for Level B and is deferred as a Level G feature candidate.
- Reference variance and casts are deferred. Level B and Level G may choose
  different policies.

## Implementation Progress

- Step 1 is implemented: `rxvm_reference_cell` is now part of the VM value
  model, `value` has `reference_identity` and `reference_payload` slots, and
  `rxvmref.c` provides allocation, retain/release, invalidation, retargeting,
  and storage identity helpers.
- Step 2 is implemented in the working tree: VM value transfer now distinguishes
  live content replacement from storage destruction. `value_zero()` releases
  reference payloads and invalidates active child-attribute storage while
  preserving the current value's own reference identity; `clear_value_contents()`
  clears contents without invalidating that identity; `destroy_value_storage()`
  and the existing `clear_value()` teardown path invalidate it. `copy_value()`
  retains copied reference payloads while preserving destination identity, and
  `move_value()` transfers source identity/payload to the destination after
  invalidating any old destination identity.
- Focused helper coverage lives in `interpreter/tests/ts_regvalue_tester.c`.
- Focused lifecycle coverage now exercises reference payload copy/clear,
  destination identity preservation on copy, identity/payload transfer on move,
  and physical child-storage invalidation for attribute deletion/shrink.
- The first performance foundation pass is implemented in the working tree:
  reference hot paths use direct null guards plus header-local cheap helpers,
  attribute shrink/delete now lazily resets removed storage for reuse, extreme
  shrink can compact and reclaim attribute storage, and reference cells can be
  allocated from context-local root buckets with a bounded free-list and
  context-local ids.
- Step 3 is implemented in the working tree: RXAS can create and use reference
  values with `mkref`, `deref`, `linkref`, `setref`, `refvalid`, and `unref`.
  Invalid reference use raises the dedicated, catchable `REFERENCE_INVALID`
  signal. This remains the lower-level operation contract that the Rexx source
  syntax targets.
- The first container-shaped regression slice is implemented in the working
  tree under `tests/reference_iterators`: RXAS fixtures model parent-backed and
  backing-array-backed iterator state with live references, explicit snapshot
  copies via `deref`, invalid backing/parent detection, and checksum-only
  performance smoke coverage. The fixtures run through optimized and
  non-optimized assembly under both `rxvm` and `rxbvm`.
- The first compiler-shaped contract slice is implemented in the working tree:
  `reference_generated_contract.rxas` models `ArrayList`/iterator helper code
  that creates references from receiver arguments and backing attributes, uses
  `deref` for explicit snapshots, checks validity before use, and handles
  invalid references through `REFERENCE_INVALID`. It remains the generated-code
  canary beneath the public Rexx syntax.
- The first explicit Rexx source slice is implemented in the working tree:
  `reference .T` declares reference values, `reference target` creates a weak
  reference to aliasable storage, `snapshot ref` makes an explicit snapshot
  copy, `refvalid(ref)` checks validity, and method `self` can be referenced
  explicitly. The source fixture runs noopt/opt through both `rxvm` and `rxbvm`
  and negative fixtures cover value/reference boundary errors, non-storage
  targets, reference-to-reference targets, nested reference types, and non-ref
  operands to `snapshot`/`refvalid`.
- The source iterator fixture now covers both intended iterator shapes:
  snapshot iterators copy the parent/backing storage once in the factory, while
  dynamic iterators store weak references and use `dereference` into local
  scoped links at each access. The tests prove snapshot stability after later
  mutation, dynamic visibility of later mutation/append, and dead-provider
  behaviour for both parent and backing references.

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
- Do not use the first source spelling to settle deferred variance, casts,
  arrays-of-references, or public collection API policy.

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
- RXAS first: the VM and assembler support references before higher-level Rexx
  sugar depends on them.
- Container usability: pure Rexx iterators should be able to retain access to a
  live container without copying the container body.
- Native interop: native-backed containers should be able to use reference-like
  lifecycle validation rather than unchecked integer handles.

## Container Iterator Proof Fixtures

The container proof point remains at the RXAS/generated-code level. The
`tests/reference_iterators` fixtures are intended to look like code the compiler
or libraries could eventually emit:

- a parent iterator stores a reference to the container object plus a cursor;
- a backing iterator stores a reference to the array/object attribute that holds
  the backing storage plus a cursor;
- `linkref` is used only at the point of access, so ordinary copies still
  snapshot values unless `deref` is deliberately used for a deep snapshot;
- `refvalid` gives client code a cheap preflight check, and invalid use remains
  catchable through `REFERENCE_INVALID`;
- both optimized and non-optimized RXAS output are exercised by both VM
  implementations (`rxvm` and `rxbvm`).

The performance fixture is intentionally a smoke test rather than a benchmark
gate. It prints elapsed times and asserts checksums so regressions in semantics
are caught without making CI depend on local machine speed.

The source-level performance smoke under `tests/performance` mirrors the same
direct, snapshot-parent, snapshot-backing, dynamic-parent, and dynamic-backing
shapes. It records factory and iteration timings for opt/noopt runs on both VM
modes; the timing output is observational, while checksums remain enforced.

## Internal Generated-Code Contract

Compiler and library experiments should continue to target the following
RXAS-level operation contract. The explicit Rexx source syntax maps onto the
same operations, and later live member/index syntax should preserve this
contract:

- **Create a reference**: expose or link the desired storage location, then emit
  `mkref` into the destination value. For object or array attributes, generated
  code must use `linkattr*` first so the reference targets the physical child
  storage, not a temporary copy. The VM marks the frame that owns the target
  storage for lifetime cleanup, which may be a caller frame when a generated
  helper creates a reference from a receiver argument.
- **Store iterator state**: store the reference value in an iterator attribute
  and store cursor/count state as ordinary scalar attributes. Copying the
  iterator object copies the reference value, not the referenced target.
- **Live access**: when the iterator needs the target, link the iterator's
  reference attribute, emit `linkref`, perform normal attribute access through
  the linked target, copy the result into a return register, then unlink in the
  reverse order.
- **Snapshot access**: emit `deref` only when generated code deliberately wants
  a deep copy of the current referenced target. A snapshot must not observe later
  mutations.
- **Validity check**: emit `refvalid` on the stored reference value when client
  code wants to test whether a weak reference can still be used.
- **Invalid reference handling**: generated code may either preflight with
  `refvalid` or rely on a catchable `REFERENCE_INVALID` signal around `deref`,
  `linkref`, or `setref`. Silent stale-reference reads are not part of the
  contract.

For method-shaped code, a reference created from a receiver argument (`a1`) is
a reference to the caller-owned receiver storage while that storage remains
alive. Returning an iterator that references a callee local is allowed by the VM
but the reference must become invalid when that frame is released; tests should
continue to cover this as the negative lifetime case.

This contract is intentionally spelling-free. The eventual Rexx surface can map
to these operations, but the operation set above is the compatibility target for
compiler and library experiments.

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

- references to frame-owned locals and nested frame-owned attribute storage
  become invalid on the reference cleanup rare path;
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

### Release 1 Native Collection Positioning

For release 1, native collection classes should be deprecated from the public
container direction in favour of Rexx-first collection implementations with a
consistent iterator interface.

This is an architectural release decision, not a criticism of the native
collection work. The native collections were valuable enabling work when cREXX
was not yet capable enough to host practical collection implementations in
Level B itself. They helped exercise the runtime, gave the project usable
container behaviour earlier, and exposed the lifetime and iterator problems
that this reference design is now trying to solve.

The release-1 direction should now be:

- public collection APIs are Rexx-first where possible;
- native collection classes are treated as deprecated or experimental for the
  release-1 surface;
- native implementations remain useful as comparison points, migration aids,
  performance baselines, and native interop tests;
- any native-backed collection that remains visible should follow the same
  iterator and reference semantics as the Rexx-first containers.

The practical consequence is that references become more urgent, not less:
without a safe way for a Rexx iterator to retain access to live collection
state, Rexx-first collections are forced into snapshot/deep-copy iteration.
That would make the preferred release-1 architecture inefficient and
semantically weaker than the native bridge it is intended to replace.

## RXAS Surface

RXAS has an explicit reference surface that the source compiler should target.

### Implemented Instructions

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

## Approved Source Syntax Direction

The public source surface should use word-form reference syntax rather than
constructor-like or symbolic forms. This keeps the location semantics visible:
creating a reference is not an ordinary value construction.

### Reference Types

Use `reference` as a type modifier anywhere a Level B type is accepted:

```rexx
list_ = reference .ArrayList
items_ = reference .string[]

*: factory
  arg list = reference .ArrayList
```

The first implementation should support references to ordinary scalar, object,
interface, and array types. `reference .string[]` means a reference to a string
array.

Precedence is:

```text
array suffix binds to the type immediately before it;
reference applies to the complete type that follows it;
parentheses disambiguate nested reference and array-of-reference cases.
```

So the intended meanings are:

```rexx
reference .string        /* reference to string */
reference .string[]      /* reference to string array */
(reference .string)[]    /* array of references to string: deferred */
reference (reference .string)[]  /* reference to array of references: deferred */
```

### Reference Expressions

Use `reference target` to create a durable weak reference value:

```rexx
countRef = reference count
listRef = reference list
iter = .ArrayListIterator(reference self)
```

The operand must be an aliasable storage expression: local storage, exposed
argument or global storage, `self`, an object/array attribute, or an array
element. It must not be an arbitrary value expression.

Use `snapshot ref` for an explicit snapshot copy of the current target:

```rexx
copy = snapshot listRef
```

The snapshot operation raises `REFERENCE_INVALID` when the reference target has
expired.

Use `local = dereference ref` for a scoped live link to the current target:

```rexx
do
  list = dereference listRef
  call list.add("next")
end
```

The destination must be a local variable in the current procedure or block
scope. Attribute, array-element, argument, global, and expression destinations
are rejected. The compiler emits `unlink` when that local scope exits; frame
exit also resets linked locals.

Use `refvalid(ref)` to test whether a weak reference can currently be used:

```rexx
if \refvalid(listRef) then return 0
```

Level G convenience syntax may allow reference values to be used as the base of
object/array access and method calls without spelling `dereference`; the
compiler would emit a scoped `linkref` / `unlink` pair for the operation:

```rexx
value = itemsRef[i]
itemsRef[i] = value
listRef.add(value)
```

This convenience rule is not part of the Level B source surface. Level B keeps
the boundary explicit: assigning or passing a reference where a plain `.T` is
required is an error; use `local = dereference ref` for a scoped live link or
`snapshot ref` for an explicit copy. Passing a plain `.T` where `reference .T`
is required is also an error; use `reference target` explicitly.

### Receiver References

`self` should be a normal receiver storage expression, not an intrinsic
reference value. Parent-backed iterators should create a reference explicitly:

```rexx
iterator: method = .Iterator
  return .ArrayListIterator(reference self)
```

This preserves ordinary receiver access while making escaping aliases visible in
source.

### Copy Warnings

After the explicit semantics settle, the compiler should warn on obvious
by-value container/receiver copies when a reference is likely intended,
especially for `self`, arrays, and object attributes passed to plain `.T`
formals. Reference boundary mistakes are hard type errors:

```rexx
.ArrayListIterator(self)              /* error: expected reference .ArrayList */
helper(listRef)                       /* error if helper expects .ArrayList */
helper(snapshot listRef)              /* explicit snapshot copy */
.ArrayListIterator(reference self)    /* explicit live reference */
```

### Deferred Syntax

These forms are not part of the Level B essential surface. They remain Level G
feature candidates unless a later Level B need is established:

```rexx
(reference .T)[]            /* array of references */
reference (reference .T)[]  /* reference to an array of references */
ref as reference .U         /* reference casts / variance */
ref is reference .T         /* reference type tests */
itemsRef[i]                 /* live index access through a reference */
itemsRef[i] = value         /* live indexed assignment through a reference */
listRef.add(value)          /* live method call through a reference */
```

Reference variance, casts, nested reference containers, and implicit live access
remain design work. Level B and Level G may choose different policies, but the
current Release 1 Level B direction is to avoid these conveniences and require
explicit reference boundaries.

### Rejected Syntax

These forms are rejected, not deferred:

```rexx
holder = .reference(.int)
holder = .ref(.int)
holder = .ref(value)
countRef = [count]
typeRef = [.int]
arg reference items = .ArrayList
ref = reference (a + b)
ref = reference makeObject()
iter = .ArrayListIterator(self)  /* when the formal is reference .ArrayList */
```

Reasons:

- `.reference(.T)`, `.ref(.T)`, and `.ref(value)` look like ordinary
  constructor/function calls but require storage-location semantics.
- Bracket forms are too close to array syntax.
- `arg reference items = .ArrayList` does not generalize to stored durable
  references; use `arg items = reference .ArrayList`.
- `reference` must target storage, not temporary expression results.
- Bare `self` must not mean "reference to self"; use `reference self`.

## Container Impact

Before references are implemented:

- pure Rexx iterators should be documented as snapshot iterators;
- pure Rexx containers should avoid claiming live iterator semantics;
- native-backed iterators can remain live only when their provider validates
  lifetime explicitly.
- native collection classes should be considered deprecated or experimental for
  the release-1 public collection surface, because the release-1 goal is a
  Rexx-first container model with one consistent iterator pattern.

After references exist:

- `ArrayList.iterator()` can construct an iterator with a reference to the list
  object or directly to the backing `val` attribute.
- Referencing the parent object is cleaner for future mutation counters and
  bounds checks.
- Referencing the backing array is cheaper and may be sufficient for the first
  live iterator.
- A live iterator should probably carry an expected modification count once the
  collection API grows mutation tracking.
- native-backed and Rexx-backed containers should converge on the same iterator
  contract instead of presenting separate release-1 behaviours.

Example design sketch:

```rexx
ArrayListIterator: class
  list_ = reference .ArrayList
  index_ = .int

  *: factory
    arg list = reference .ArrayList
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

## Remaining Design Choices

The core VM/RXAS reference direction and first source spelling are now narrowed
by the agreed direction above. The remaining choices should be handled after the
first Rexx source slice:

1. Should `reference .T` be assignable only to another exact `reference .T`, or
   should checked casts between compatible reference types be allowed?
2. How should interface references work? A reference to `.Shape` can refer to a
   `.Box`, but dereferencing must preserve normal `istype`/`asserttype`
   behaviour. Level B and Level G may choose different policies.
3. What should the Release 1 collection API promise for snapshot/live/fail-fast
   or synchronized iteration variants? The first reference-backed Rexx container
   experiment should use unsynchronized live iteration, but the wider public API
   decision remains separate.
4. What should `Iterator.next()` return in the public collection contract? This
   is collection API work, not reference machinery.

## Proposed Programme

1. Add VM reference-cell data structures and internal retain/release helpers.
2. Split value clearing into content replacement versus storage destruction
   where needed.
3. Teach `copy_value()`, `move_value()`, frame teardown, and attribute deletion
   about reference identity and reference payloads.
4. Add RXAS instructions: `mkref`, `deref`, `linkref`, `setref`, `refvalid`,
   and `unref`.
5. Add VM/RXAS tests covering copies, snapshots, frame exit, attribute
   references, invalid `deref`/`linkref`/`setref`, non-reference operands, and
   `linkref`/`unlink`.
6. Use references internally in a small compiler-generated case before exposing
   source syntax.
7. Implement the approved explicit source syntax with examples from containers,
   ADDRESS objects, and native-backed handles.
8. Convert `ArrayListIterator` or a minimal new test container to live
   reference-backed iteration.
