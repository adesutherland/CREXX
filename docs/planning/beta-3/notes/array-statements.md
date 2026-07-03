# Level B Array Mutation Statements

Status: raw dynamic-array implementation complete in `rxc`; object/interface
collection lowering deferred to a post-Release 1 Level G enhancement.

This note records the intended Level B source surface for direct collection
mutation. The goal is to make common sequence operations readable in Rexx
source while keeping the lowering in `rxc`, not compiler exits, because the
standard libraries must be able to depend on the syntax during bootstrap.

## Statement Surface

The accepted statement forms are:

```rexx
append items with value
insert items with value at index
remove items at index
remove items at index for count
remove items at first to last
clear items
```

The statement heads are `APPEND`, `INSERT`, `REMOVE`, and `CLEAR`. Clause
words should reuse existing Rexx-like words where practical: `WITH`, `FOR`,
and `TO` are already established in the language. `AT` is implemented as a new
contextual clause word for these statements.

## Raw Array Semantics

The first implementation target is raw Level B typed arrays. That phase is now
implemented for one-dimensional dynamic `.T[]` arrays.

- `append array with value` appends one element at the current high-water mark.
  It is semantically equivalent to assigning at `array[array[0] + 1]`.
- `insert array with value at index` inserts one slot before/at the 1-based
  index, shifts later elements right, and stores `value` in the new slot.
- `remove array at index` removes one element and shifts later elements left.
- `remove array at index for count` removes `count` elements from `index`.
- `remove array at first to last` removes the inclusive 1-based range from
  `first` through `last`.
- `clear array` removes all elements and leaves the array object valid with a
  high-water mark of zero.

For `.T[]` arrays, the compiler checks that the target is a one-dimensional
dynamic raw array and that inserted/appended values are assignable to `.T`.
Fixed-size arrays, multi-dimensional arrays, class attributes, and future
object/interface targets are rejected by this first phase. Index, count, first,
and last expressions are integer expressions. The implemented raw-array
lowering uses the VM array opcodes directly, so insert/remove index validity
follows `INSATTRS1`/`DELATTRS1` rather than the older string helper BIF
clamping behavior. Inclusive range removal skips the opcode when
`last - first + 1 <= 0`.

## Lowering Target

The implemented raw-array lowering is:

- append: emit `GETATTRS array,0`, increment the high-water mark, emit
  `INSATTRS1 array,index,1`, then store the value in the new slot
- insert: emit `INSATTRS1 array,index,1`, then assign `array[index] = value`
- remove single/count: emit `DELATTRS1 array,index,count`
- remove range: evaluate `first` and `last`, compute `last - first + 1`, then
  emit `DELATTRS1 array,first,count` when the count is positive
- clear: emit `SETATTRS array,0`

This deliberately avoids public pseudo-intrinsic functions. The syntax is a
statement surface for operations where the compiler or VM can do better than a
Rexx-level element-copy loop.

## Existing Clear Surface

Array clearing already exists today as standard-library helper calls:

```rexx
call arraydrop items
call objectarraydrop objects
```

Those helpers lower to `SETATTRS array,0`. `clear items` is now the source-level
raw dynamic-array clear statement. `arrayhi(array, "SET", n)` can shrink a
string array but intentionally refuses `n < 1`, so it is not the helper-level
clear-to-empty surface. Classic/Level C `DROP` is a variable-pool operation and
should not be reused for typed-array element removal.

Do not retire `arraydrop` or `objectarraydrop` during Release 1. They remain
the compatibility surface and are still needed by collection class internals
where the first raw-array statement implementation does not apply, such as
class attributes and object-array helper use. `clear array` is the preferred
source statement for supported raw dynamic arrays because it lowers directly to
`SETATTRS array,0`.

## Object Extension Direction

The syntax is intentionally compatible with future collection objects:

```rexx
append list with value       -- list.append(value)
insert list with value at i  -- list.insert(i, value)
remove list at i             -- list.remove(i)
clear list                   -- list.clear()
```

The compiler already supports `MEMBER_CALL` AST nodes, class/interface method
lookup, method argument inference, and interface dispatch through
`srcmethodsel`.
That means object lowering can be considered after raw arrays, provided the
target type is known well enough for validation to bind the method contract.

Do not make arbitrary `.object` dynamic dispatch the first implementation
target. A bare `.object` does not by itself prove the presence or signature of
`append`, `insert`, `remove`, or `clear`. The safer first object extension is a
specific interface or concrete class contract whose methods are visible to the
compiler.

## Post-Release 1 Level G Collection Extension

Object/interface lowering is a post-Release 1 Level G enhancement, not Release 1
scope. The extension should be interface-led rather than name-only duck typing:

- Define explicit mutable sequence contracts before lowering object targets.
  Existing classlib method names are not enough because lists, linked lists,
  maps, and sets use overlapping names with different semantics and return
  shapes.
- Prefer statically proven interface or concrete-class targets first. The
  compiler can then validate that the required method contract exists before
  rewriting the statement to method dispatch.
- Runtime dynamic scenarios should also be contract-based. Values should be
  interface-typed, explicitly cast, or guarded before dispatch; arbitrary
  `.object` targets should not be accepted merely because a method might exist.
- Count/range removal needs a deliberate contract. Emitting repeated
  `remove(index)` calls would be slow and would not match all existing method
  meanings. A contract method such as `removeRange(index, count)` or equivalent
  should be specified before collection lowering is implemented.
- The compiler can then lower through existing member/interface dispatch,
  including `srcmethodsel`, using the required method descriptors.

## Non-Goals

- No generic `.T[]` helper functions are introduced by this syntax.
- No overload surface is implied before the language has overloads/generics.
- No `for each` loop syntax is implied. Current iteration remains ordinary
  method calls against classlib iterator interfaces.
- No change is made to `<clear>`, which remains a named integer bit operation.

## Test Expectations

The raw-array implementation has focused no-opt and opt coverage for:

- append/insert/remove/clear on `.string[]`
- append/insert/remove/clear on at least one numeric typed array, such as
  `.int[]`
- range removal and count removal edge cases
- compile-time diagnostics for non-array targets, non-integer index/count
  expressions, and incompatible value types

Still pending:

- post-Release 1 Level G append/insert/remove/clear lowering for explicit
  object/interface collection contracts
