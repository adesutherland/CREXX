# Level B Array Mutation Statements

Status: design accepted, implementation pending.

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
and `TO` are already established in the language. `AT` is a new contextual
clause word for these statements unless a better existing spelling is chosen
during implementation.

## Raw Array Semantics

The first implementation target is raw Level B typed arrays.

- `append array with value` appends one element at the current high-water mark.
  It is equivalent to assigning at `array[array[0] + 1]`, using the compiler's
  existing dynamic-array growth path.
- `insert array with value at index` inserts one slot before/at the 1-based
  index, shifts later elements right, and stores `value` in the new slot.
- `remove array at index` removes one element and shifts later elements left.
- `remove array at index for count` removes `count` elements from `index`.
- `remove array at first to last` removes the inclusive 1-based range from
  `first` through `last`.
- `clear array` removes all elements and leaves the array object valid with a
  high-water mark of zero.

For `.T[]` arrays, the compiler should check that the target is an array and
that inserted/appended values are assignable to `.T`. Index, count, first, and
last expressions are integer expressions. Negative or out-of-range runtime
behavior should match the existing `array*` helper semantics where practical
and be documented by the implementation.

## Lowering Target

The intended raw-array lowering is:

- append: use normal indexed assignment to `array[array[0] + 1]`
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

Those helpers lower to `SETATTRS array,0`. `arrayhi(array, "SET", n)` can
shrink a string array but intentionally refuses `n < 1`, so it is not the
current clear-to-empty surface. Classic/Level C `DROP` is a variable-pool
operation and should not be reused for typed-array element removal.

## Object Extension Direction

The syntax is intentionally compatible with future collection objects:

```rexx
append list with value       -- list.append(value)
insert list with value at i  -- list.insert(i, value)
remove list at i             -- list.remove(i)
clear list                   -- list.clear()
```

The compiler already supports `MEMBER_CALL` AST nodes, class/interface method
lookup, method argument inference, and interface dispatch through `srcmethod`.
That means object lowering can be considered after raw arrays, provided the
target type is known well enough for validation to bind the method contract.

Do not make arbitrary `.object` dynamic dispatch the first implementation
target. A bare `.object` does not by itself prove the presence or signature of
`append`, `insert`, `remove`, or `clear`. The safer first object extension is a
specific interface or concrete class contract whose methods are visible to the
compiler.

## Non-Goals

- No generic `.T[]` helper functions are introduced by this syntax.
- No overload surface is implied before the language has overloads/generics.
- No `for each` loop syntax is implied. Current iteration remains ordinary
  method calls against classlib iterator interfaces.
- No change is made to `<clear>`, which remains a named integer bit operation.

## Test Expectations

Implementation should add focused no-opt and opt coverage for:

- append/insert/remove/clear on `.string[]`
- append/insert/remove/clear on at least one numeric typed array, such as
  `.int[]`
- append/insert/remove/clear on `.object[]` once boxing/object values are
  usable enough for the test shape
- range removal and count removal edge cases
- compile-time diagnostics for non-array targets, non-integer index/count
  expressions, and incompatible value types

