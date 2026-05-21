# Classes and Interfaces {#classes-and-interfaces}

Level B now implements the core class/interface model:

- interfaces and classes
- classes implementing one or more interfaces
- abstract interface methods
- final/default interface methods
- interface-centred default and named factories
- factory provider selection through class-side `match`
- checked casts with `expr as .type`
- boolean type tests with `expr is .type`
- concrete type introspection with `typeof(expr)`
- namespace-qualified contract references such as `.pkg..thing()`

Level B does **not** currently implement interface inheritance, interface
attributes/state, interface factory bodies, overloads, singleton declarations,
or destructor/finalizer syntax.

## Core Model

Interfaces define callable contracts. Classes implement those contracts.
Factory declarations do not carry an explicit return type. The return contract
is implied by the owner: an interface factory returns that interface contract,
and a class factory returns that concrete class. In a class factory, bare
`return` returns the constructed object; there is no source-level `this` value
to return explicitly.

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
    return "car:" || _name
```

Instances are normally created through the interface:

```rexx
current = .vehicle("mini")
say current.describe()
```

Each class also has an intrinsic interface of its own name, so `.car()` remains
valid when you want to work directly with the concrete class contract.

## Defining Interfaces

An interface may declare:

- abstract methods
- final/default methods with bodies
- the default `*` factory
- named factories

An interface method with a body is final in Level B. A class may rely on that
body, but it may not override it.

```rexx
shape: interface
  *: factory

  describe: method = .string
    return prefix() || ":" || summary()

  prefix: method = .string
    return "iface"

  summary: method = .string
```

In this example `describe` and `prefix` are final/default methods, while
`summary` remains abstract.

## Defining Classes

Classes implement one or more interfaces:

```rexx
box: class implements .shape
  *: factory
    return

  summary: method = .string
    return "box"
```

The class must implement every abstract member from its effective interface
set. If an interface already provides a final/default method body, the class may
omit that member.

Factories are the exception to the ordinary `name: callable = .type` spelling:
write `*: factory` or `name: factory`, then declare arguments with `arg` as
usual. A factory return type written after `=` is not part of the Level B source
syntax.

Assignment compatibility in the current Level B implementation flows from a
concrete class value to an implemented interface. In practice that means a
statement such as `iface = .widget(...)` is supported, while interface-to-interface
assignment should not be relied on.

## Class State

Class attributes are declared in the class block:

```rexx
box: class implements .shape
  _label = .string
  _count = .int
```

For ordinary classes, let the compiler allocate storage automatically and keep
external callers on factories and methods.

Explicit physical layout should be reserved for genuine low-level interop. When
that is needed, append `with register.N` to the attribute definition, where `N`
is the one-based VM object-attribute slot:

```rexx
raw_event: class
  _code = .int with register.1.int
  _module = .int with register.2.int
  _address = .int with register.3.int
  _name = .string with register.4.string
```

The optional suffix after the index is the VM register value view to use for the
slot. Valid views are `.int`, `.float`, `.string`, and `.object`:

```rexx
  _message = .string with register.5.string
  _payload = .object with register.5.object
```

The compiler emits the attribute linking code for methods that read or write
these attributes. Source code should still access them through methods, not
through hand-written assembler. It is valid for VM-integration classes to define
more than one typed view over the same physical slot, as shown for a signal
payload/message slot above. Ordinary application classes should not use explicit
register mappings unless they are matching a fixed VM or native object layout.

## Factory Selection with `match`

Factory members are declared on the interface and implemented by the class using
the same name.

- `*` is the default factory member
- `name: factory` is a named factory member
- `name: match` is the optional class-side selector for that same factory

`match` is class-side only. Its signature must match the paired factory and it
must return `.int`.

Selection rules:

1. Every candidate provider is scored through its effective `match`, even when
   there is only one candidate.
2. If `match` is omitted, the candidate behaves as if it returned `1`.
3. Scores `<= 0` reject that candidate.
4. The highest positive score wins.
5. Ties are broken alphabetically by concrete class name.

### Example

```rexx
asset: interface
  *: factory
  arg spec = .string
  from_size: factory
  arg size = .int

  describe: method = .string
    return kind() || ":" || name() || ":" || size()

  kind: method = .string
  name: method = .string
  size: method = .int

fileasset: class implements .asset
  _name = .string
  _size = .int

  *: match
    arg spec = .string
    if spec = "log.txt" then return 100
    return 0

  *: factory
    arg spec = .string
    _name = spec
    _size = 8
    return

  from_size: match
    arg size = .int
    if size = 8 then return 50
    return 0

  from_size: factory
    arg size = .int
    _name = "sized-file"
    _size = size
    return

  kind: method = .string
    return "file"

  name: method = .string
    return _name

  size: method = .int
    return _size

cacheasset: class implements .asset
  _name = .string
  _size = .int

  *: factory
    arg spec = .string
    _name = spec
    _size = 1
    return

  from_size: factory
    arg size = .int
    _name = "cache-" || size
    _size = size
    return

  kind: method = .string
    return "cache"

  name: method = .string
    return _name

  size: method = .int
    return _size
```

```rexx
selected = .asset("log.txt")
fallback = .asset("memo")
say selected.describe()   /* file:log.txt:8 */
say fallback.describe()   /* cache:memo:1 */
```

This complete example is mirrored by the test
`compiler/tests/rexx_src/interface_showcase_same_module.rexx`.

## Multiple Interfaces

A class may implement more than one interface as long as the public member
surface remains coherent.

```rexx
named: interface
  *: factory
  arg label = .string, length = .int
  name: method = .string

measured: interface
  size: method = .int

widget: class implements .named .measured
  _label = .string
  _length = .int

  *: factory
    _label = label
    _length = length
    return

  name: method = .string
    return _label

  size: method = .int
    return _length
```

```rexx
item = .widget("gear", 4)
by_name = item
by_size = item
say by_name.name()
say by_size.size()
```

This example is mirrored by the test
`compiler/tests/rexx_src/interface_multi_interface_same_module.rexx`.

## Namespace Qualification

If two imported namespaces expose contracts with the same name, qualify the
reference with `namespace..`.

```rexx
import qifa
import qifb

left = .qifa..vehicle("one")
right = .qifb..vehicle.from_name("two")
```

The token to the left of `..` must be a namespace name that has already been
imported. Qualification does not bypass `import`. `namespace::symbol` remains
accepted as a compatibility alias, but `namespace..symbol` is the canonical
form.

## Same-File Contract and Provider Pattern

An interface and one or more implementation classes may live in the same source
file and namespace. This is often the clearest shape when the contract and
providers are developed together.

```rexx
options levelb
namespace automotive expose vehicle mycar

vehicle: interface
  *: factory
  arg name = .string
  describe: method = .string

mycar: class implements .vehicle
  _name = .string

  *: factory
  arg name = .string
  _name = name
  return

  describe: method = .string
  return "dep:" || _name
```

The interface owns the public factory contract. The class supplies the
implementation with the same factory name and argument signature; its concrete
class return is accepted because the class implements the interface.

This same-file shape is mirrored by
`compiler/tests/rexx_src/interface_dep_contract.rexx`.

## Split-File Contract and Provider Pattern

When an interface and its provider class live in different source files, each
file is still compiled as a separate module. Multiple files may contribute
symbols to the same namespace, just as the Level B standard library contributes
many `rxfnsb` functions from separate files, but the provider compile must still
be able to find the interface contract through the import path.

The provider may use the same namespace as the contract:

```rexx
/* vehicle.rexx */
options levelb
namespace automotive expose vehicle

vehicle: interface
  *: factory
  arg name = .string
  describe: method = .string
```

```rexx
/* mycar.rexx */
options levelb
namespace automotive expose mycar

mycar: class implements .vehicle
  _name = .string

  *: factory
  arg name = .string
  _name = name
  return

  describe: method = .string
  return "dep:" || _name
```

It is also valid to put provider classes in a separate provider namespace when
that better matches packaging or ownership:

```rexx
options levelb
namespace automotive_provider expose mycar
import automotive

mycar: class implements .vehicle
  /* implementation as above */
```

Pure contract modules are valid: a source file that contains only interface or
class/interface metadata can compile and assemble into a metadata-only `.rxbin`.
A provider that consumes that binary contract must compile with the contract's
directory on the binary import path, for example `rxc -i build-dir provider.rexx`.
Keeping source roots and binary roots separate prevents stale `.rxbin` side
products beside edited source from silently satisfying interface imports.

The class factory must have the same argument signature as the interface
factory. The return contract is implicit on both sides: the interface factory
returns the interface, and the class factory returns the concrete class. The
compiler checks that the concrete class value is assignable to the interface
return type.

If a provider reports `#INTERFACE_NOT_FOUND` or
`#INTERFACE_MEMBER_SIGNATURE_MISMATCH`, check these first:

- the provider compile can find the source module (`.crexx`, `.crx`, `.rexx`,
  or the initial file's arbitrary extension in a source root) or binary module
  (`.rxbin` in a directory passed with `-i`) that exposes the interface
- the class factory argument list exactly matches the interface factory argument
  list
- a binary contract is intentional; sibling `.rxbin` files are not searched
  unless their directory is a binary root
- the provider module is loaded or linked into any program that calls the
  interface factory

This example is mirrored by:

- `compiler/tests/rexx_src/qualified_interface_main.rexx`
- `compiler/tests/rexx_src/qualified_interface_dep_a.rexx`
- `compiler/tests/rexx_src/qualified_interface_dep_b.rexx`

## Casts and Type Tests

Level B now supports explicit object casts and runtime type inspection:

```rexx
vehicle = .car("roadster") as .vehicle
current = vehicle as .car

say typeof(current)
say current is .vehicle
say current is .car
say current is .truck
```

Rules:

- `expr as .interface` succeeds when the concrete class implements that
  interface
- `expr as .class` succeeds only for that exact concrete class
- failed object casts raise `CONVERSION_ERROR`
- `expr is .interface` checks interface implementation
- `expr is .class` checks the exact concrete class
- `typeof(expr)` returns the concrete runtime class for objects and the
  canonical built-in type name for scalars

These operations are mirrored by:

- `compiler/tests/rexx_src/type_ops_showcase.rexx`
- `compiler/tests/rexx_src/type_ops_fail.rexx`

## Notes and Current Boundaries

- Interface default methods are final in Level B.
- Factory providers are selected at runtime from the linked modules.
- Interface method dispatch works for both interface-typed and concrete-class
  receivers.
- Namespace qualification is a disambiguation mechanism, not a second global
  symbol path.
- Singleton declarations, external-object adoption syntax, and destructor
  hooks are intentionally deferred to higher levels.
