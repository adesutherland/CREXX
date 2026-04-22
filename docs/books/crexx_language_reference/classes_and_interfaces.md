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

```rexx
vehicle: interface
  *: factory = .vehicle
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
  *: factory = .shape

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
that is needed, use the existing `with register...` mapping forms described in
the architecture and data-type references rather than treating them as the
normal class style.

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
  *: factory = .asset
  arg spec = .string
  from_size: factory = .asset
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
  *: factory = .named
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
