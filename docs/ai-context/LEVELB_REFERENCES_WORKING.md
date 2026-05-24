# Level B References Working Design

Status: working design note, not an approved language specification.

## Problem

Level B object values currently have clear copy and move behaviour, but there is
no durable user-visible reference concept. That matters for patterns such as a
collection returning an iterator over itself: the iterator needs to retain access
to the collection instance, not to a deep copy that silently diverges.

Adding a `this`-like receiver expression would solve only the immediate naming
problem. It would not by itself solve ownership, copying, invalidation, or stack
frame lifetime. Durable references need a VM-level design before the language can
expose them safely.

## Current Baseline

- A Rexx object is a VM `value`, not a user-visible pointer. Object dispatch is
  based on the stamped object type and metadata.
- `copy_value(dest, source)` preserves the source and duplicates strings,
  decimals, binaries, and object attributes into destination-owned storage.
- `move_value(dest, source)` transfers owned buffers and attribute arrays into
  the destination and clears the source. Procedure returns use this only when
  returning a real local register; returning an argument, global, or linked
  attribute is copied.
- Procedure calls initially link caller argument registers into callee locals.
  The compiler/emitter adds defensive copies for by-value writable formals when
  the language contract requires them.
- `arg expose` already has useful call-time alias semantics: the callee is
  operating on caller-owned state. It is not a durable reference that can safely
  be stored beyond the call.
- The emitter currently uses two compiler call/register flag bits:
  `REGTP_VAL = 0x00000100` for "argument value supplied" and `REGTP_NOTSYM =
  0x00000200` for "large by-value argument is not a symbol and does not need
  preservation".

## Design Goals

- Keep default object assignment and argument passing understandable: copying a
  value must not unexpectedly become pointer sharing.
- Make durable references explicit in the VM model and eventually in the
  language surface.
- Prevent use-after-free corruption even if the programmer keeps a reference to
  an object whose owner has gone out of scope.
- Keep lifecycle responsibility visible to the programmer: an iterator over a
  collection is only meaningful while the collection is alive.
- Preserve efficient move returns for factory-created and local unique objects.
- Support compiler-generated references cleanly without hand-written assembler
  in Level B libraries.

## Register Flags

The current 32-bit register/value flag field is formally partitioned in
`binutils/include/rxflags.h`:

- `0x000000FF`: VM-private, externally readable and VM-writable only
- `0x0000FF00`: compiler call semantics, including `REGTP_VAL` and
  `REGTP_NOTSYM`
- `0x00FF0000`: stable library/runtime contracts
- `0x7F000000`: documented user/experimental space
- `0x80000000`: reserved to avoid signed integer ambiguity

The low VM-private band is intentionally reserved for runtime storage/cache
state such as UTF-8 validity, codepoint-count validity, reference/invalid state,
and future GC/lifetime bookkeeping. External `SETTP`, `SETORTP`, and
`LOADSETTP` operands are masked before mutation, so those low bits are read-only
outside the VM.

## VM Reference Model Options

### Option A: Register-to-Register References

A register can hold a reference to another register. Every VM instruction either
checks and follows the reference on access, or the compiler emits an explicit
dereference instruction when entering a procedure or method.

Assessment: this is attractive for `arg expose`, but weak for durable
references. Registers belong to stack frames. If the target frame exits, stored
references must be invalidated. Making every instruction implicitly chase a
reference is also broad and easy to get wrong.

### Option B: VM Reference Cells

References point to VM-managed cells or handles, not raw stack registers. The
cell owns or observes a `value` identity and can be invalidated when that owner
is destroyed. A reference value stores the cell handle and each dereference can
check whether the cell is still valid.

Assessment: this is the preferred direction. It separates reference identity
from stack register numbers, gives the VM one place to clear stale references,
and supports references stored inside objects. It is more work than register
aliases, but it is the route most likely to remain mathematically safe.

### Option C: Borrow-Only References

Keep references limited to call-time aliasing, equivalent to stronger `expose`
semantics. Do not allow storing a reference into an object.

Assessment: this is useful as a compiler implementation technique, but it does
not solve iterators over collections or other durable relationship patterns.

## Candidate RXAS Surface

Names are placeholders until the assembler design is approved.

- `mkref target, source`: create a reference value to `source`
- `deref target, ref`: copy or link the current referenced value into `target`,
  raising a reference-invalid signal if the cell is stale
- `refvalid target, ref`: set a boolean indicating whether a reference is still
  valid
- `unref ref`: clear a reference value

The compiler can use a single explicit dereference near procedure entry when a
reference is known to be stable for the procedure body. General stored
references still need validity checks at each explicit dereference point.

## Candidate Language Surface

Do not reserve final syntax yet. These are design sketches only.

- Extend `arg expose` as the call-time aliasing form. It already matches user
  expectations for "the actual object is exposed and used".
- Add a durable reference type only when VM reference cells exist, for example a
  `.ref(.Collection)` style object or a typed reference modifier.
- Add a receiver expression only with reference rules defined. A future `this`
  or equivalent can then be a value or a reference according to explicit
  compiler rules rather than an accidental pointer.

## Iterator Plan Before Durable References

- Prefer snapshot iterators for pure Rexx collections: the iterator receives a
  copied array or lightweight snapshot of keys/values.
- A collection may implement an iteration interface itself for single-cursor or
  stream-like cases. This works when the collection is the iterator, but it is
  not a general substitute for many independent iterators.
- Native-backed collections may expose stable VM/native tokens if their provider
  can validate lifetime explicitly.
- Avoid pure Rexx iterators that store their owner object until reference cells
  exist. They will either copy too much or depend on unsafe object identity.

## Proposed Programme

1. Document and centralize register/value flag masks.
2. Add VM reference-cell data structures and invalidation hooks.
3. Add RXAS instructions and focused assembler/VM tests.
4. Add compiler internal support for reference-typed values and dereference
   insertion.
5. Add a minimal user-facing syntax and tests.
6. Revisit iterators and receiver expressions once references are real.

## Open Questions

- Should invalid dereference raise an existing signal such as `ERROR`, or a new
  reference-specific signal?
- Should a reference ever keep the target alive, or should the initial model be
  weak-only with explicit invalidation?
- Can attributes be reference-typed, or should durable references first be
  limited to locals and procedure arguments?
- How should references interact with interface dispatch and factory `match`
  selection?
