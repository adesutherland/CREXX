# PHP Phase 2 — Array Support

## 1. Objective

Extend the Phase 1 PHP translator so PHP arrays can be translated into an
ordinary cREXX runtime object. The compiler and VM do not gain PHP-specific
array syntax or a new built-in datatype.

```text
PHP array syntax
        -> Level L grammar
        -> Level B reader
        -> php_array runtime object
        -> generated cREXX
```

## 2. Phase 2 scope

The first array slice will support:

- empty arrays and list literals;
- associative arrays;
- non-negative integer and string keys;
- lookup and assignment;
- append using `[]`;
- nested arrays;
- `count()`;
- insertion-order `foreach`.

The runtime will preserve insertion order. Replacing an existing key changes
its value but not its position.

## 3. Runtime design

The public runtime type is `.php_array`. It owns ordered key/value storage and
does not expose its representation to translated PHP code. The initial API is:

```text
php_array()
set(key, scalar)
setArray(key, array)
append(scalar)
appendArray(array)
get(key)
getArray(key)
has(key)
count()
keys()
```

Scalar values are retained as text at the PHP boundary; cREXX numeric
conversion remains available when a generated expression uses the value.
Nested arrays are stored as `.php_array` objects. This keeps the first runtime
slice small and makes the value model explicit.

Automatic numeric keys start at zero. Explicit non-negative integer keys move
the next append key forward. Full PHP key-conversion rules and deletion are
outside this phase.

## 4. Explicit exclusions

Phase 2 does not implement references, copy-on-write compatibility, complete
PHP key-conversion rules, boolean/null/float keys, destructuring, spread
operators, the full `array_*` library, sorting, serialisation, JSON conversion,
mutation during iteration, or exact PHP notice/warning behavior.

Because cREXX objects are mutable, assignment of an array variable has
reference-like behavior in this phase; PHP copy-on-write is not promised.

## 5. Grammar and reader responsibilities

The grammar must add named nodes for array literals, array items, array access,
append targets, `foreach`, and `count()`. Array access must support chains such
as `$order["customer"]["name"]`.

The reader must lower those nodes into `.php_array` construction and method
calls. Generated cREXX must contain no PHP array syntax.

## 6. Acceptance criteria

- **AC-01**: [x] `php_array` stores scalar values by string or integer key.
- **AC-02**: [x] replacing a key preserves its insertion position.
- **AC-03**: [x] append assigns sequential non-negative integer keys.
- **AC-04**: [x] nested `php_array` values can be stored and retrieved.
- **AC-05**: [x] `count()` and ordered `keys()` work.
- **AC-06**: [x] focused runtime tests pass in the normal build.
- **AC-07**: the Phase 2 reader lowers list, associative, lookup, append,
  nested, count, and foreach syntax.
- **AC-08**: generated cREXX compiles and runs without PHP array syntax.
- **AC-09**: unsupported features produce clear PHP diagnostics.

## 7. Implementation steps

- **STEP-01**: [x] Add and test the ordered `.php_array` runtime object. Serves
  AC-01 through AC-06.
- **STEP-02**: Extend the Level L grammar with array syntax and named nodes.
  Serves AC-07 and AC-09.
- **STEP-03**: Extend the Level B reader with array lowering and nested access.
  Serves AC-07 and AC-08.
- **STEP-04**: Add list, associative, nested, append, count, and foreach
  migration fixtures. Serves AC-06 through AC-09.

## 8. Initial implementation status

Design preserved from the Phase 2 proposal on 24 October 2026. STEP-01 is
complete. The runtime and smoke test were compiled, assembled, linked, and run
in the normal Release build; output confirmed count, append order, replacement
without reordering, and nested-array lookup. Grammar and reader work remain
open.
