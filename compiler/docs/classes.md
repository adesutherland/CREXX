# Classes and Interfaces in the Compiler

Status: implemented Level B core

This note records the shipped compiler/runtime model for Level B classes and
interfaces. It replaces the earlier working design note.

## Source Surface

The implemented Level B surface is:

- `label: interface`
- `label: class implements .iface ...`
- abstract interface methods
- interface methods with bodies, which are emitted as final/default methods
- interface-declared default `*` factories and named factories
- same-named class-side factory implementations
- optional same-named class-side `match` for factory selection
- namespace-qualified contract references such as `.pkg::thing()`

Each class also has an intrinsic interface of its own name, so `.box()` and
`.box`-typed references remain valid even when the class also implements other
interfaces.

Class state continues to use the existing attribute machinery. Ordinary class
fields should use implicit storage; explicit `with register...` mappings remain
the low-level interop path rather than part of the interface-dispatch model.

Level B does not currently implement:

- interface inheritance
- interface attributes/state
- interface factory bodies
- overloads
- singleton syntax
- destructor/finalizer syntax

## Validation Rules

Validation currently enforces the following contract rules:

- a concrete class must implement every abstract interface method it inherits
- an interface method with a body is exported as `method final`
- a class may not redefine an inherited final/default interface method
- a class-side `match` is only valid when the same factory name exists on the
  class/interface contract
- a class-side `match` inherits the paired factory signature and must return
  `.int`
- if a class omits `match`, the runtime behaves as if an implicit `match`
  returning `1` exists

Assignment compatibility is widened from a concrete class to any implemented
interface.

## AST and Emission Shape

The parser accepts:

- `interface`
- `implements`
- factory-call syntax such as `.vehicle()` and `.vehicle.from_name()`
- namespace-qualified symbols such as `.qifa::vehicle()`

Interface member calls and interface factory calls are preserved as contract
calls through validation. They are no longer lowered to a unique concrete class
during typing.

The relevant emitted VM lookups are:

- `srcmethod` for interface method dispatch
- `srcfproc` for interface default and named factory dispatch

The dynamic call itself still goes through the existing `dcall` path once the
lookup step resolves the concrete procedure.

## Metadata Model

Cross-module import/export uses the normal `.rxbin` metadata stream. The
interface work extends that stream with:

- `META_INTERFACE`
- `META_IMPLEMENTS`
- `META_MEMBER`

These records carry the contract header information needed to reconstruct
imported interface/class stubs without scanning procedure bodies.

Current import behaviour:

- only locally defined class/interface contracts are re-exported
- imported stubs are not re-emitted as new local contract definitions
- when duplicate imported stubs are discovered, the richer stub wins

Interface methods with bodies are exported with the member kind `method final`,
which is how the importer preserves the default-method bit.

## Runtime Dispatch

### Method dispatch

Created objects are stamped with their concrete runtime class identity. During
VM link, the runtime builds a method registry keyed by concrete class plus
member name.

For each effective member:

1. prefer the concrete class method, if present
2. otherwise bind the interface default method when the member is `method final`

At execution time `srcmethod` resolves through that registry, then `dcall`
invokes the bound procedure.

### Factory dispatch

During VM link, the runtime also builds a factory-provider registry keyed by:

- interface fully qualified name
- factory member label (`*` or a named factory)

Each candidate row stores:

- the concrete class fully qualified name
- the resolved concrete factory procedure
- the optional resolved concrete `match` procedure

At execution time `srcfproc`:

1. finds all candidates for the requested interface/member pair
2. evaluates the effective `match` for every candidate, even if there is only
   one candidate
3. treats omitted `match` as score `1`
4. rejects scores `<= 0`
5. chooses the highest positive score
6. breaks ties alphabetically by fully qualified concrete class name

If no provider survives selection, the VM raises `FUNCTION_NOT_FOUND`.

## Namespace Qualification

Qualified source references use `namespace::symbol`. The token to the left of
`::` must be a namespace name already made visible through `import`.

The compiler normalizes `namespace::name` to the internal fully qualified form
`namespace.name`, but qualification remains import-gated. It is a
disambiguation mechanism, not a second global lookup path.

## Coverage

The shipped implementation is covered across:

- same-module compiler/runtime tests
- source-import tests
- binary-import tests
- compile-fail tests
- direct RXAS/VM tests

Representative examples live under:

- `compiler/tests/rexx_src/interface_showcase_same_module.rexx`
- `compiler/tests/rexx_src/interface_multi_interface_same_module.rexx`
- `compiler/tests/rexx_src/qualified_interface_main.rexx`
- `interpreter/tests/tests_interfaces.rxas`

## Related Docs

- `docs/books/crexx_language_reference/classes_and_interfaces.md`
- `docs/books/crexx_language_reference/namespace.md`
- `docs/books/crexx_programming_guide/intralanguage.md`
- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/ai-context/RXAS_ASSEMBLER.md`
- `docs/ai-context/RXVM_INTERPRETER.md`
