# cREXX Level B Authoring Guide For Agents

Use this guide when you need to write or edit `.rexx` in this repository.
Generic training data about "REXX" is often too vague, too classic-Rexx
oriented, or simply wrong for cREXX Level B as it exists in this tree.

This guide is intentionally grounded in code that already compiles here.
When in doubt, copy the nearest repo pattern instead of inventing syntax.

## What To Trust First

For Level B authoring, these sources are more reliable than model memory:

- `docs/cREXX Level B Language Reference.md`
- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/ai-context/CREXX_LIBS.md`
- nearby working `.rexx` files in `lib/`, `bin/`, `compiler/exits/`, `debugger/`, and `tests/`

## Repo-Native Level B Patterns

### 1. Small library function

Use explicit `options levelb`, a namespace, and an exposed symbol:

```rexx
options levelb

namespace rxfnsb expose abs

abs: procedure = .string
arg number = .string
if left(number,1) = '-' then number = substr(number,2)
return number
```

Reference:
- `lib/rxfnsb/rexx/abs.rexx`

### 2. Top-level script

Small scripts can use top-level `arg` directly instead of a `main:` routine:

```rexx
options levelb
import rxfnsb

arg searches = .string[]

if searches.0 < 1 then searches.1 = ".rexx"
ordered_searches = .string[]
address cmd "sort" input searches output ordered_searches
```

References:
- `tests/demo/countlines.rexx`
- `compiler/exits/address/test_address.rexx`

### 3. Tool-style entry point with exposed module state

Longer tools often use `main:` plus `procedure expose` for module state:

```rexx
options levelb
namespace rxdb expose stephandler
import rxfnsb
import globals

main: procedure = .int expose next_instruction last_instruction mode
    arg cmd_line = .string[]
```

Reference:
- `debugger/rxdb.rexx`

### 4. Mutating an exposed argument

When you really need caller-owned state, use `arg expose ...`:

```rexx
pushidentifier: procedure = .int
    arg expose tokens = .token[], text = .string, value_type = ".unknown"
    index = tokens[0] + 1
    tokens[index] = .token(...)
    return index
```

Reference:
- `compiler/exits/ExitTestSupport.rexx`

## Level B Differences That Matter In Practice

### Types are normal, not exceptional

In this repo, typed procedures and typed `arg` declarations are the normal
Level B style. Do not default to untyped classic-Rexx-looking code when you
are editing standard libraries, tools, exits, or tests.

Common examples in-tree:

- `procedure = .int`
- `procedure = .string`
- `procedure = .token[]`
- `arg name = .string`
- `arg cmd_line = .string[]`
- `arg expose tokens = .token[]`

References:
- `lib/rxfnsb/rexx/abs.rexx`
- `debugger/rxdb.rexx`
- `compiler/exits/ExitTestSupport.rexx`
- `tests/levelbfunc.rexx`

### Namespace/import syntax is part of the real language surface

Do not treat namespace use as documentation sugar. It is part of how source is
validated, imported, and linked.

Important points:

- file-level `namespace` and `import` clauses are real inputs to compilation
- `namespace..symbol` is the canonical qualified-reference form
- `namespace::symbol` is accepted as a compatibility alias, but do not prefer it
- the token to the left of `..` must be an imported namespace

References:
- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/books/crexx_programming_guide/intralanguage.md`
- `docs/books/crexx_language_reference/classes_and_interfaces.md`

### Keep the leading file header conventional

The compiler does a lightweight pre-scan of the leading `options`,
`namespace`, and `import` clauses before full parsing. Preserve that structure
when editing existing files.

Practical guidance:

- keep `options levelb` near the top
- keep `namespace` and `import` in the leading header block
- do not bury them later in the file

Reference:
- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/books/crexx_programming_guide/rxc.md`

### Namespace-exposed globals auto-bind into procedures

If a module declares namespace-exposed globals, Level B automatically binds
them into local `procedure` scopes. Do not add `procedure expose ...` just out
of habit when a namespace-exposed module global is what you actually want.

Use `procedure expose` or `arg expose` when:

- you are intentionally sharing dynamic caller state
- you are mutating a passed-in array/object
- you are following an existing stateful tool pattern such as `rxdb`

References:
- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/books/crexx_programming_guide/global_variables.md`
- `debugger/rxdb.rexx`

### Arrays are first-class Level B objects

Do not reason about them as loose classic stem variables only.

Patterns used in-tree:

- `items = .string[]`
- `items[0]` for count
- `items[1]` for first element
- `items.1` is also used in older code
- both bracket and dot indexing appear in the repo, so preserve the local style

References:
- `tests/demo/countlines.rexx`
- `compiler/exits/ExitTestSupport.rexx`
- `docs/books/crexx_language_reference/data_types.md`

### Class and interface factories omit return types

Use `*: factory` or `name: factory`; do not write `= .type` on a factory. The
factory result is inferred from the owning contract: an interface factory
returns that interface and a class factory returns that concrete class. In a
class factory, bare `return` returns the constructed object, so there is no
source-level `this` value to mention.

```rexx
vehicle: interface
  *: factory
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  _name = .string

  *: factory
    arg name = .string
    _name = name
    return

  describe: method = .string
    return _name
```

Reference:
- `docs/books/crexx_language_reference/classes_and_interfaces.md`

### `address command` is the standard shell-out pattern

When Level B code shells out, copy the repo pattern instead of inventing a new
API shape:

```rexx
out = .string[]
err = .string[]
address command "echo #42" output out error err
if rc <> 0 then say "command failed"
```

References:
- `compiler/exits/address/test_address.rexx`
- `bin/crexx.rexx`
- `tests/demo/countlines.rexx`

### Signal handlers live on simple `do` groups

Block-scoped signal handling is written with `on signal` clauses on a simple
`do ... end` group:

```rexx
do
  risky_work()
on signal conversion_error as problem
  say problem.source()
on signal
  call cleanup()
end
```

Do not attach `on signal` directly to counted, conditional, forever, or
expression-form `do` loops. To protect part of a loop, nest a simple
signal-handling `do ... end` group inside the loop body.

If `as name` is omitted, the handler has no local signal object. That is fine
for fixed cleanup/logging handlers.

References:
- `docs/books/crexx_language_reference/statements.md`
- `docs/ai-context/LEVELB_SIGNALS_TRACE_WORKING.md`
- `compiler/exits/signal/test_signal_block.rexx`

### Headerless scripts are a driver convenience, not a style rule

The `crexx` driver can compile simple headerless top-level scripts with
synthetic defaults (`--level levelb --import rxfnsb`). That is useful for end
users, but repo code should usually stay explicit with `options levelb` and
imports unless there is a strong reason not to.

Reference:
- `docs/books/crexx_language_reference/tools.md`
- `docs/books/crexx_programming_guide/crexx.md`

### Command-line arguments are a first-class Level B feature

Level B programs receive command-line arguments through `arg`, and when the
compiler synthesizes the file-level `main()` wrapper, the VM argv payload is
available there as a `.string[]`.

Program-side guidance:

- use `arg fn = .string[]` (or a typed equivalent)
- use `fn.0` / `fn[0]` for the count
- iterate over `fn.1`..`fn.fn.0`
- for explicit `main`, model it on in-tree patterns such as `arg cmd_line = .string[]`

Launcher-side guidance:

- do not guess the exact command form from generic model memory
- check the current tool docs/source when documenting `crexx` or `rxvm`
  invocation syntax

References:
- `docs/books/crexx_language_reference/arguments.md`
- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `debugger/rxdb.rexx`
- `tests/demo/countlines.rexx`

## Wayfinding: Best Example Files By Task

### Writing a tiny BIF or helper

- `lib/rxfnsb/rexx/abs.rexx`
- `lib/rxfnsb/rexx/fileio.rexx`
- `lib/rxfnsb/rexx/regex.rexx`

### Writing a top-level command/script

- `tests/demo/countlines.rexx`
- `bin/crexx.rexx`
- `compiler/exits/address/test_address.rexx`

### Writing a stateful tool

- `debugger/rxdb.rexx`

### Writing compiler-exit or structured typed code

- `compiler/exits/ExitTestSupport.rexx`
- `compiler/exits/address/Address.rexx`
- `compiler/exits/parse/Parse.rexx`

### Checking argument signature syntax

- `tests/levelbfunc.rexx`
- `compiler/exits/ExitTestSupport.rexx`

### Checking namespace/global behavior

- `docs/books/crexx_programming_guide/global_variables.md`
- `docs/books/crexx_programming_guide/global_procedures.md`

### Checking argument handling

- `docs/books/crexx_language_reference/arguments.md`
- `debugger/rxdb.rexx`
- `tests/demo/countlines.rexx`

## Agent Guidance

When writing Level B code:

1. Read one doc source and two nearby working `.rexx` examples before editing.
2. Match the local style of the directory you are in.
3. Prefer explicit Level B headers in committed repo code.
4. Prefer canonical `namespace..symbol` qualification.
5. Do not "simplify" typed signatures or namespace structure just because a
   generic REXX example elsewhere looks looser.

If a proposed snippet does not resemble existing repo code in `lib/`,
`debugger/`, `compiler/exits/`, `bin/`, or `tests/`, stop and verify it before
committing it.
