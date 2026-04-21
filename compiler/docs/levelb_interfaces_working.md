# Level B Interfaces and Callable Contracts Working Design

Status: working design; tracer-bullet slice implemented; full runtime stage approved for planning

Last updated: 2026-04-21

## Purpose

This note records the Level B interface direction in one place.

The first tracer-bullet slice is now implemented across the parser, validator,
metadata emitter/importer, assembler, and compiler import path. The rest of the
note still documents the broader intended design beyond that first slice.

It exists because the public language docs already describe interfaces,
multi-interface classes, and interface-centred factories, but the current
compiler/runtime only implement:

- classes
- methods
- the wildcard `*: factory` model
- concrete-class method dispatch
- `.ClassName()` factory calls that bind directly to `§factory`

The Phase 3.2 `ADDRESS` work exposed the gap clearly: bridge/native code can
name a concrete class when calling a method, but ordinary Rexx source cannot
yet express a shared callable contract across multiple classes.

This document therefore treats "interface" and "callable contract" as the same
Level B feature. "Interface" is the source-language term. "Callable contract"
is the compiler/runtime term.

## Level B Scope Boundary

Level B is the low-level system foundation. This working note therefore draws a
hard boundary around what this slice will and will not standardize.

Explicitly in scope for Level B:

- interfaces / callable contracts
- multi-interface classes
- interface-centred factory dispatch
- native-backed objects only to the extent needed for typed dispatch and normal
  assignment

Explicitly out of scope for Level B and deferred to Level G or later:

- singleton declarations or singleton-specific language syntax
- destructor syntax, finalizers, or automatic lifecycle hooks
- dedicated source-language syntax for adopting, wrapping, or borrowing
  external/native objects

Level B guidance for those areas is intentionally minimal:

- singleton-like behaviour is expressed by ordinary global variables managed by
  user code
- native-backed classes/interfaces may use ordinary Rexx declarations; if a
  placeholder factory surface is needed on the Rexx side, a trivial/empty
  factory may be declared and the resulting variable may later be overwritten by
  ordinary assignment from bridge/native code
- resource cleanup is done through ordinary methods and user-land conventions,
  not through language-defined destructors

## Current Reality

The existing codebase now behaves as follows.

- The public docs describe interfaces as first-class source constructs, classes
  implementing one or more interfaces, and interface-centred factory usage.
- The parser now accepts `interface` definitions and `class implements .iface`.
- `.ClassName(...)` is parsed as a `FACTORY_CALL`.
- Type checking now supports one implemented interface path by resolving an
  interface factory/method call to the sole known implementing class.
- Type compatibility now allows assignment from a concrete class to an
  implemented interface.
- for class methods and factories, the compiler already emits an internal RXAS
  procedure label by prefixing the fully qualified symbol name with `§`, while
  the exposed/imported name remains the unmangled fully qualified name
- `.rxbin` now carries interface headers and class-interface links through
  `META_INTERFACE`, `META_IMPLEMENTS`, and `META_MEMBER`, and the compiler reads
  those back during import to synthesize contract stubs.
- `rxvml` already scans `META_CLASS` records directly from module metadata.
- Historical VM docs already sketched a `ptable` style runtime mapping for
  interface dispatch, but no active assembler/interpreter path currently uses
  that mechanism.

This means the interface work should extend the current metadata/import chain,
not introduce a separate binary header system in parallel with it.

## Implemented Tracer Bullet

The implemented first slice is intentionally narrow.

Implemented now:

- source syntax for `interface` and `class implements .iface`
- abstract interface methods
- default `*` factory declarations on interfaces
- class-side implementations of the same method names and `*` factory
- class-to-interface assignment compatibility
- single-implementation lowering at compile time for interface factories and
  interface method calls
- import/export of interface/class header metadata through `.rxbin`
- same-module, source-import, and `rxbin`-only import coverage for the
  one-interface, one-implementation path

Deliberately not implemented in this tracer bullet:

- interface method bodies
- named interface factories
- `match`
- multi-implementation runtime selection
- VM-level interface dispatch instructions

Current tracer-bullet rule:

- if an interface call site has exactly one known implementation, the compiler
  lowers that call to the concrete class
- if it has zero implementations, compilation fails
- if it has more than one implementation, compilation fails

Two implementation details matter for import stability:

- modules only export contract metadata for locally defined classes/interfaces,
  not for imported stubs
- when duplicate imported contract headers are discovered, the importer keeps
  the richer stub with the real member list

Relevant current implementation files:

- `compiler/rxcpbgmr.y`
- `compiler/rxcp_emit_meta.c`
- `compiler/rxcpfunc.c`
- `compiler/rxcp_val_type.c`
- `compiler/rxcp_emit_proc.c`
- `compiler/rxcp_emit_reg.c`
- `compiler/rxcp_fixup.c`
- `compiler/rxcp_val_sym.c`
- `assembler/rxasassm.c`
- `interpreter/rxvmload.c`
- `interpreter/rxvalue.h`
- `interpreter/rxvml.c`

## Current Project Status

The project is now at a clean staging point.

- the tracer-bullet slice is implemented and tested
- the remaining core Level B design decisions have been approved
- the next step is no longer design discovery; it is staged implementation of
  the full runtime interface model
- regressions should be treated as bugs to fix now unless they clearly depend
  on later approved capabilities such as runtime multi-provider dispatch

The approved full-stage direction is:

- metadata-driven runtime dispatch
- runtime lookup and candidate selection built in C during VM load/link
- the existing `META_INTERFACE`, `META_IMPLEMENTS`, and `META_MEMBER` metadata
  family as the stable header direction
- structured internal mangling as an implementation detail only; metadata
  selectors remain the semantic ABI
- Level B continuing to exclude interface state, interface inheritance, method
  overloading, factory overloading, singleton syntax, destructor syntax, and
  dedicated external-object lifecycle syntax

## Confirmed Requirements

The intended Level B direction is:

1. A class may implement multiple interfaces.
2. Each class also has an intrinsic interface of its own name.
3. An interface is a callable contract made of method declarations and, in
   some cases, method definitions.
4. Interface method definitions in Level B are final shared utilities or
   wrappers. They are not overridden by classes in Level B.
5. Interface method names must not overlap across the effective interface set
   of a class. Level B keeps this simple by forbidding ambiguous names.
6. Interface method dispatch is name-preserving: `interface.xxx()` is
   implemented by `class.xxx()`.
7. Users should construct objects via an interface-centred syntax. Classes
   supply factories for the interfaces they implement.
8. Factory names are defined by the interface and implemented by each class
   using the same label.
9. `*` remains the default factory member name and therefore maps to the call
   surface `.interface(...)` without explicit naming.
10. Each interface factory member may have a same-named class-side `match`
   companion. `match` is not declared in the interface itself; its signature is
   inherited from the paired factory and its return type is always `.int`.
11. If a class does not define a same-named `match` for an implemented factory,
   the compiler/runtime behaves as if an implicit `match` exists that returns
   `1`.
12. Every candidate implementation of an interface factory must have its
   effective `match` called, even if only one candidate implementation exists.
13. Factory selection uses the highest positive `match` score; tied scores are
   broken alphabetically by concrete class name.
14. Because Level B forbids overlapping public names across the effective
   interface set of a class, a class that implements multiple interfaces may
   only use `*` for one of those interfaces.
15. The compiler and VM need object concrete-type metadata so a method call made
   through an interface can find the concrete class implementation at runtime.
16. The compiler must be able to read `.rxbin` metadata and reconstruct class
   and interface headers without needing to inspect procedure bytecode bodies.
17. Runtime interface tables and factory candidate lists should be built from
   module metadata during load/link, rather than handwritten in Rexx code.
18. The VM likely needs a lookup-plus-call path for interface methods, and a
   runtime-assisted factory-selection path for interface factories.

## Recommended Level B Simplifications

The older docs are broader than what is sensible for the first working Level B
implementation. The recommended first slice is narrower.

Recommended simplifications for Level B v1:

- interfaces are source-level callable contracts
- no interface inheritance
- no method overloading
- no factory overloading
- no overlapping interface method names on a class
- no overlapping factory or `match` names across the effective interface set of
  one class
- as a direct consequence of the no-overlap rule, a multi-interface class may
  only implement one interface that uses `*`
- no class override of an interface default/final method body
- no interface-defined storage/attributes in v1
- no singleton declarations in Level B; use globals in user code instead
- no destructor/finalizer feature in Level B
- no dedicated external-object adoption syntax in Level B
- no dynamic runtime mutation of a class's implemented-interface set
- no implicit package scanning for implementations; only normally imported and
  linked modules participate

The "no interface-defined storage" recommendation is deliberate. It keeps
Level B interfaces as callable contracts and shared wrappers, and leaves object
state entirely in the concrete class.

## Source-Language Model

### Interface Shape

Recommended semantic model:

- an interface may declare abstract methods
- an interface may define final/default methods
- a class must implement every abstract method of every interface it declares
- if an interface supplies a method body, the class may not provide another
  method of the same name in Level B v1

This deliberately revises the older docs that described default interface
methods as overridable.

### Class Shape

Each class implements one or more explicit interfaces plus its intrinsic
same-name interface.

Recommended source rule:

- the intrinsic interface exists automatically and is not listed explicitly

### Dispatch Rule

There are two dispatch modes.

- Concrete-class dispatch: if the compiler knows the receiver's concrete class
  and the call is against that class contract, the current direct class-method
  lookup path remains valid.
- Interface dispatch: if the receiver is only known through an interface type,
  the call must be compiled against the interface contract and resolved at
  runtime using the receiver's concrete type metadata.

### Optional Namespace Qualification

The next implementation stage introduces optional namespace qualification as a
disambiguation mechanism for imported symbols.

Recommended source forms:

- `mylib::open_file(...)` for procedures
- `.mylib::stream(...)` for interface/class factory calls
- `value as .mylib::stream` for type assertions/annotations
- `class file_stream implements .mylib::stream`

Important rule:

- the token to the left of `::` must be a namespace name
- qualification does not bypass `import`; the namespace must already be brought
  into scope by normal module import rules
- qualification is therefore an exact imported-namespace disambiguation
  mechanism, not a second global symbol access path

Recommended Level B rule:

- unqualified references keep the current search behaviour across the current
  module namespace and imported namespaces
- qualified references bypass search order and resolve directly against the
  named imported namespace
- using `.` for qualification is explicitly avoided because `.` already names
  class/interface surfaces and member calls

### Type Compatibility Rule

Recommended Level B v1 compatibility:

- `.car` is assignable to `.vehicle` if `car` implements `vehicle`
- `.car` is assignable to `.car` through its intrinsic interface
- `.vehicle` is assignable to `.vehicle`
- `.vehicle` is not implicitly assignable to `.roadworthy` even if some class
  implements both
- exact-class equality is no longer the only object compatibility rule

No cast syntax is proposed in this first note.

## Syntax Options

This section is now much narrower than the first draft. The current recommended
direction is to keep `*` and make factory labels part of the interface
contract, just like methods.

### Recommended Factory and Match Syntax

Example:

```rexx
vehicle: interface

  *: factory = .vehicle /* = .vehicle is optional */

  from_name: factory = .vehicle /* = .vehicle is optional */
    arg initial_name = .string
    
  /* Note no match records needed in the interface */ 
    
  /* Abstract */  
  type: method = .string   

  /* Final */ 
  summary: method = .string
    return "Vehicle of type" type()
    
  start: method = .void
  stop: method = .void

car: class implements .vehicle
  name = .string

  /* We do not need to include this as it will be implicit */
  *: match /* Must be .int */
    return 1

  *: factory /* Must be .vehicle as method names are unique across interfaces implemented by a class */
    name = ""

  from_name: match
    arg initial_name = .string
    if left(initial_name,4) = "car " then return 100
    return 0

  from_name: factory
    arg initial_name = .string
    name = initial_name

  start: method
    say "car start"

  stop: method
    say "car stop"
```

Usage:

```rexx
v1 = .vehicle()
v2 = .vehicle.from_name("Mini")
c1 = .car()
```

Semantics:

- `.vehicle()` targets the `*` factory member of `vehicle`
- `.vehicle.from_name()` targets the `from_name` factory member of `vehicle`
- `.car()` works through the intrinsic interface of `car`
- every candidate implementation of the targeted factory member is asked to
  perform its effective `match` using the same argument list
- if a class did not define that `match`, an implicit `match` returning `1` is
  used
- the winning implementation is then called through its same-named factory
- if several candidates return the same highest positive score, the concrete
  class with the alphabetically earliest class name wins

Pros:

- preserves the short default call form
- keeps `*` meaningful as the default member name
- makes factory labels part of the interface contract
- keeps the probe/select mechanism explicit instead of encoding it in factory
  failure behaviour
- allows simple implementations to omit `match` and still participate with the
  default score
- keeps user syntax interface-centred
- preserves `.ClassName()` via the intrinsic interface

Cons:

- needs new grammar for `implements`
- needs a new `match` member kind
- needs synthesis/defaulting rules for omitted class-side `match` members
- preserves the Level B edge-case restriction that only one implemented
  interface on a class may use `*`

### Class Declaration Options

There is also a separate class-declaration syntax choice.

```rexx
car: class implements .vehicle .roadworthy
```


Recommendation: prefer `implements`. It is clearer, scales better to multiple
interfaces, and does not overload `=`.

### Recommended Source Direction

If choosing one path to prototype first, the recommended source-language
direction is:

- `interface`
- `class implements .interface1 .interface2`
- interface-declared factory signatures
- no interface-declared `match` signatures
- class factory implementations written as bare `label: factory`
- optional class `match` implementations written as bare `label: match`
- class-side factory and `match` signatures inherited from the interface
  factory declaration
- `.interface()` as the `*` factory call surface
- `.interface.factory_name()` as the named factory call surface

## Method and Factory Semantics

### Interface Methods

Recommended rules:

- a method with no body in an interface is abstract
- a method with a body in an interface is final/default in Level B v1
- a class must define all abstract methods
- a class may not redefine a final/default interface method
- name matching is exact; no renaming adaptor mechanism is proposed

### Interface Method Calls from Interface Default Methods

If an interface default method calls another interface method, that nested call
should remain an interface call, not a direct hard-wired call to one concrete
class.

That allows a final/default wrapper in the interface to delegate to the
concrete class implementation of another abstract method.

### Factories

Recommended rules:

- factory names are part of the interface contract
- `*` is the default factory member name
- `.interface()` targets `*`
- `.interface.named_factory()` targets `named_factory`
- if an interface factory omits its explicit return type, the effective return
  type defaults to the owning interface type
- classes implement the same factory labels declared by the interface
- class-side factory implementations inherit the interface-declared signature;
  they do not need to restate the return type or arguments
- the intrinsic interface of a class allows `.car()` and `.car.named_factory()`
  to remain valid
- because Level B forbids overlapping public names across interfaces, a class
  implementing multiple interfaces may only use `*` for one of them

### Factory Match Members

Recommended rules:

- `match` is not declared in the interface source or interface metadata
- each interface factory may have a same-named class-side `match`
- `match` inherits the same argument signature as its paired factory
- `match` returns `.int`
- `match <= 0` means reject this request
- `match > 0` means accept this request with the given score
- a class may omit the `match` implementation; in that case an implicit
  implementation returning `1` is used

The name `match` is preferred over `probe` or `accepts` because it naturally
supports scoring instead of only boolean acceptance.

### Factory Selection Algorithm

Recommended Level B v1 selection algorithm:

1. Resolve the target interface and target factory label.
2. Gather all linked candidate class implementations for that interface/factory.
3. For each candidate, obtain the same-named effective `match` implementation,
   using the explicit class implementation if present or an implicit `return 1`
   implementation otherwise.
4. Call that effective `match` on every candidate with the same argument list
   that will later be passed to the factory, even if there is only one
   candidate.
5. Discard candidates whose score is `<= 0`.
6. Select the highest positive score.
7. If several candidates share the same highest positive score, select the
   concrete class whose class name is alphabetically earliest.
8. Call the same-named factory on the selected class.
9. If no candidate returns a positive score, raise a "no matching factory"
   failure.

This makes the driver/provider use case explicit and deterministic.

## Level B User-Land Patterns

### Singleton-Like Usage

This note does not propose a Level B singleton language feature.

If user code wants singleton-like behaviour in Level B, it should use an
ordinary global variable and initialize it explicitly in user code.

That keeps Level B focused on low-level callable contracts while leaving richer
singleton facilities for Level G.

### Native / External Objects

This note does not propose new Level B syntax for adopting or wrapping
external/native objects.

For Level B, the intended pattern is:

- declare the relevant class/interface contract in Rexx
- if a Rexx-visible factory surface is needed for completeness, declare a
  trivial placeholder factory
- assign the real bridge/native object into the relevant Rexx variable through
  ordinary assignment

In other words, native-backed objects are usable in Level B, but no new
singleton or ownership syntax is added for them in this slice.

### Cleanup / Lifetime

This note does not propose destructors, finalizers, or implicit cleanup hooks.

If cleanup matters, it should be expressed as ordinary methods and conventions
in user code or bridge code.

## Compiler Changes

### Parser and AST

The compiler will need at least:

- an `INTERFACE_DEF` node
- an interface-member representation
- a class implemented-interface list
- interface-aware factory declaration nodes or metadata
- class-side `match` declaration nodes or metadata
- synthesis support for implicit class-side `match` members

The existing `FACTORY_CALL` and `MEMBER_CALL` nodes may still be usable if they
carry resolved interface metadata. A new dedicated interface-call node is not
strictly required, but it may make later emission cleaner.

### Symbol and Validation Work

The symbol/validation layer will need to:

- create symbols for interfaces
- create the intrinsic same-name interface for every class
- record which interfaces each class implements
- record stable member ordinals per interface
- record stable factory ordinals or factory-name selectors per interface
- derive the effective `match` selector for a factory from the factory label,
  rather than from a separate interface declaration
- synthesize an implicit class-side `match` implementation returning `1` when a
  class implements a factory but omits the same-named `match`
- validate that all abstract interface methods are implemented
- reject redefinition of interface final/default methods
- reject overlapping interface method names across a class's effective
  interface set
- reject overlapping factory or `match` names across a class's effective
  interface set
- explicitly diagnose the edge case where a class implements multiple
  interfaces that both use `*`
- validate that each explicit `match` declaration has the same argument
  signature as its paired factory

### Type Checking

The type system will need to move from exact-class-only object compatibility to
contract compatibility.

At minimum:

- a variable or return type may be an interface type
- assignment from concrete class to implemented interface is valid
- member resolution on an interface-typed value uses the interface contract
- member resolution on a concrete-class-typed value may still use the direct
  class lookup path
- interface factory calls resolve against the interface factory contract, not a
  concrete class's private factory namespace

### Imports and Stubs

The compiler already imports class metadata today through the constant-pool
`META_CLASS`, `META_ATTR`, and `META_FUNC` records. Interface support should
extend that same import path rather than introduce a second binary-description
mechanism.

Recommended import rule:

- referencing `.vehicle` as a type or as a factory surface imports the
  interface stub for `vehicle`
- referencing a concrete class continues to use the existing class-stub import
  path built from `.rxbin` metadata
- the intrinsic same-name interface of a class is synthesized from the class
  metadata already present; it does not need a second explicit interface
  declaration in the binary
- concrete implementing classes still need to be part of the normal import/link
  set; Level B v1 does not invent hidden discovery of implementations
- the importer should remain metadata-only: it should not need to inspect
  procedure bytecode in order to reconstruct contracts

## Binary Metadata and Import Model

The implementation base already gives a useful starting point:

- `META_CLASS` and `META_ATTR` already exist in `rxbin.h`
- the compiler emitter already writes them
- the assembler already serializes them
- the compiler importer already reads them back from `.rxbin`

The interface work should therefore add the minimum extra metadata needed to
describe callable contracts, implemented-interface links, and importable
signatures.

### Recommended New Metadata Records

Recommended new constant-pool metadata records:

- `META_INTERFACE`: declares one source-defined interface
- `META_IMPLEMENTS`: links one concrete class to one explicitly implemented
  interface
- `META_MEMBER`: declares one interface member signature

Recommended payload shape:

- `META_INTERFACE`: fully qualified interface name plus the usual option/type
  fields, mirroring the current `META_CLASS` shape for implementation
  convenience
- `META_IMPLEMENTS`: fully qualified concrete class name plus fully qualified
  interface name
- `META_MEMBER`: fully qualified interface-member name, member kind/options,
  return type, and serialized argument signature

Important points:

- `META_MEMBER` describes methods and factories
- abstract interface members need metadata even though they have no procedure
  body
- final/default interface methods still also have ordinary `META_FUNC` /
  procedure metadata for their executable body
- no separate `META_MATCH` record is needed, because `match` is a class-side
  companion inferred from the factory label
- no explicit metadata is needed for the intrinsic same-name interface of a
  class in the first slice; the compiler and runtime can synthesize it from the
  already-emitted class metadata
- the metadata selector is the semantic contract; the mangled procedure label
  is only the internal code-entry name

### Recommended RXAS Surface

At the RXAS layer, the recommended direction is to extend the existing `.meta`
family rather than invent a second header-directive family.

Illustrative forms:

```rxas
.meta "demo.vehicle"="b" "" .interface
.meta "demo.car"="demo.vehicle" "" .implements

.meta "demo.vehicle.*"="factory" ".vehicle" "" .member
.meta "demo.vehicle.from_name"="factory" ".vehicle" "initial_name=.string" .member
.meta "demo.vehicle.type"="method abstract" ".string" "" .member
.meta "demo.vehicle.summary"="method final" ".string" "" .member
```

Meaning:

- the interface and implements records are pure header metadata
- the member records describe interface shape only; they do not duplicate code
- final/default interface methods still emit their normal procedure entries
- the compiler importer can reconstruct interface contracts entirely from the
  metadata chain and exposed procedures

### Long-Term Mangling Scheme

The mangling scheme needs to support two separate goals:

- unique internal procedure labels for emitted code
- future growth toward overloading, bridge thunks, and compiler-generated
  generic specializations

It should not carry the semantic meaning of polymorphism by itself. Interface
polymorphism should be driven by metadata selectors and runtime tables, not by
string-parsing mangled names.

Recommended design rule:

- metadata names and selectors are the stable semantic ABI
- mangled procedure labels are the internal code ABI
- runtime dispatch should use metadata selectors to choose an implementation,
  then call the resolved procedure label

Recommended mangled-label grammar:

```text
§<owner-fqn>.§<kind>.§member.<member-name>
    [.§iface.<interface-fqn>]
    [.§sig.<signature-key>]
    [.§spec.<specialization-key>]
```

Where:

- `<owner-fqn>` is the concrete class or interface that owns the emitted body
- `<kind>` identifies the emitted artifact kind:
  `method`, `factory`, `match`, `idefault`, `impl`, `bridge`
- `<member-name>` is the source member label, including `*` for the default
  factory member
- `.§iface.<interface-fqn>` qualifies interface-bound artifacts where the same
  concrete class may later need more than one binding surface for one member
  name
- `.§sig.<signature-key>` is reserved for future overload sets; it may be
  omitted in Level B v1
- `.§spec.<specialization-key>` is reserved for future compiler-generated
  generic specialization or other monomorphized code; it may be omitted in
  Level B v1

Recommended examples:

```text
§demo.car.§method.§member.start
§demo.vehicle.§idefault.§member.summary
§demo.car.§factory.§iface.demo.vehicle.§member.*
§demo.car.§factory.§iface.demo.vehicle.§member.from_name
§demo.car.§match.§iface.demo.vehicle.§member.from_name
§demo.car.§impl.§iface.demo.vehicle.§member.start
§demo.vector.§method.§member.push.§sig.T.§spec.int
```

Notes:

- the reserved markers `§method`, `§factory`, `§match`, `§iface`, `§member`,
  `§sig`, and `§spec` make the string self-delimiting without needing to ban
  `.` inside fully qualified names
- Level B v1 can omit `.§sig...` and `.§spec...`, but the slot ordering should
  be fixed now so later features are additive
- the current simple scheme `§` + fully qualified symbol name can remain a
  compatibility shorthand for existing concrete-class methods and intrinsic
  default factories during transition
- interface dispatch must not reconstruct semantics from these strings; it
  should resolve by metadata selector first and treat the mangled label only as
  the target code symbol

Recommended long-term consequence:

- for Level B v1, deriving one concrete implementation label from metadata plus
  the mangling rule is acceptable where there is exactly one emitted body
- once bridge thunks, overloads, or generic specializations exist, the binding
  metadata should carry the exact resolved procedure label rather than forcing
  the runtime to recompute it from first principles
- this keeps future polymorphism and generic code generation as a cheap additive
  extension rather than a redesign of the dispatch model

## VM and Runtime Model

## Runtime Object Metadata

The VM needs concrete-type metadata on object values.

Recommended direction:

- extend `value` with an `object_type` or `class_meta` field
- represent it as a resolved runtime class-registry pointer or small class-id,
  not as a raw constant-pool index
- do not overload the current `status` typeflag with concrete class identity

Rationale:

- the existing typeflag/status bits are already used for dynamic value typing
  and optional-argument state
- interface dispatch needs stable concrete-class identity, not only "this is an
  object"

Each factory must stamp the created object with its concrete class metadata
before returning it. Native/bridge-created objects entering Rexx must do the
same.

### Runtime Dispatch Tables

Recommended runtime model:

- keep `.rxbin` lightweight and metadata-oriented
- build concrete runtime dispatch tables in C during module link, after exposed
  procedures have been resolved
- maintain a runtime class registry keyed by concrete class name
- maintain a runtime interface registry keyed by interface name
- for each class/interface pair, build a dispatch row mapping interface member
  ordinals to resolved concrete or default-method procedures
- for each interface factory selector, build an ordered candidate list of
  concrete classes with their resolved factory procedure and optional `match`
  procedure
- candidate rows that do not resolve an explicit `match` procedure behave as if
  they had an implicit score of `1`

Candidate order should be alphabetical by concrete class name so the required
tie-break rule falls directly out of registry order.

This is conceptually very close to the older `ptable` design already described
in the historical VM documentation, but the recommended first implementation is
to synthesize those tables at load/link time rather than store literal
`.ptable` / `.ftable` rows in the binary.

### Load / Link Phase Recommendation

The best place to build the runtime interface/factory registry is during VM
module link, not in Rexx land.

Reasoning:

- by link time, the full set of participating modules is known
- exposed procedures have already been resolved into callable `proc_constant`
  entries
- the factory candidate set is therefore a runtime concern tied to the linked
  module graph, not a source-language concern
- keeping selection in C avoids forcing the compiler to emit a general-purpose
  candidate iteration loop for every interface factory call site

Recommended high-level link steps:

- scan metadata chains of linked modules for interfaces, members, classes, and
  implements-links
- resolve default/final interface methods through the exposed procedure tree
- resolve class-side interface methods and factories through the exposed
  procedure tree
- resolve optional class-side `match` procedures through the same mechanism
- when an emitted binding record later carries an explicit mangled procedure
  label, prefer that exact label over recomputing one from the semantic name
- sort each interface factory candidate list alphabetically by concrete class
  name once at link time
- keep the linked registry in native VM structures for fast lookup at runtime

## Instruction Family Proposals

The VM does not need one giant new four-operand "call interface method"
instruction. A lookup-plus-call design still fits the current instruction style
for method dispatch, but factory selection should use the runtime-linked
registry in C rather than expose candidate-table iteration in Rexx bytecode.

Recommended minimal additions:

1. Set concrete object type metadata on a register value:

```text
setobjtype object_reg, class_meta
```

2. Read back concrete object type metadata:

```text
getobjtype out_reg, object_reg
```

3. Resolve an interface member implementation for one receiver:

```text
srcptable proc_reg, object_reg, iface_member_selector
```

4. Resolve an interface factory call to the best factory procedure:

```text
srcfproc proc_reg, iface_factory_selector, arg_array_reg
```

5. Call the selected factory through the existing dynamic-call path:

```text
dcall result_reg, proc_reg, arg_array_reg
```

Notes:

- `iface_member_selector` should be a constant-pool selector bundling
  interface-id plus member ordinal
- `iface_factory_selector` should bundle interface-id plus factory ordinal or
  name-id
- `srcfproc` consults the runtime-linked interface registry
- `srcfproc` is responsible for calling every effective `match`, applying the
  highest-positive-score rule, and applying alphabetical tie-break
- candidate rows may omit an explicit `match` procedure and rely on the
  implicit score `1`
- this keeps the heavy selection logic in C while still reusing the existing
  `DCALL` path for the actual factory invocation

### Alternative Single-Instruction Option

Possible but not recommended for the first implementation:

```text
calliface result_reg, object_reg, iface_member_selector, arg_array_reg
callifactory result_reg, iface_factory_selector, arg_array_reg
```

This is simpler conceptually but pushes the VM and assembler toward new wide
instruction formats earlier than necessary.

## Approved Next Stage Plan

The next stage starts from the implemented tracer bullet and replaces the
single-implementation lowering path with the approved full runtime model.

1. Add optional namespace-qualified references using `namespace::name`.
   This applies to procedures, class/interface symbols, type references,
   `implements`, `of`, `as`, and factory calls. The left side of `::` must be
   a namespace and must already be imported.
2. Complete the lightweight interface header metadata so `.rxbin` import can
   reconstruct interface surfaces, implemented-interface links, and factory
   members without inspecting procedure bodies.
3. Build runtime interface dispatch registries and factory candidate lists in C
   during VM load/link, keyed by fully qualified interface identity and member
   name/selector.
4. Extend runtime values and VM support so interface method calls resolve by
   interface contract plus receiver concrete type, then invoke through the
   existing dynamic-call path.
5. Add full interface-centred factory dispatch:
   same-named class-side factory implementations, optional same-named `match`,
   implicit `match = 1`, highest positive score wins, and alphabetical
   tie-break by concrete class name.
6. Add final/shared interface method bodies while keeping Level B interfaces
   stateless and non-overridable.
7. Harden validation and diagnostics for name collisions, `*` conflicts,
   ambiguous implementations, missing implementations, rejected factories, and
   qualified-name/import misuse.
8. Expand tests and documentation so same-module, source-import, and `rxbin`
   import cases cover the full runtime dispatch/factory path and its negative
   cases.

## Explicit Non-Goals for the Next Stage

Not proposed for the approved next implementation stage:

- interface inheritance
- interface attributes/state
- class override of interface default methods
- overloaded methods or factories
- singleton declarations or singleton-specific syntax
- destructor / finalizer support
- dedicated external-object adoption / ownership syntax
- dynamic plugin registration of new interface implementations after link
- hidden implementation discovery outside the normal import/link model
- source-level cast syntax

## Closed Design Decisions

The following design points are approved and should be treated as closed unless
later explicitly reopened.

1. Use `implements` for class declarations.
2. Treat Level B interfaces and callable contracts as the same feature.
3. Make interface method bodies final in Level B v1.
4. Forbid interface-defined attributes/state in Level B v1.
5. Keep `*` as the default factory member name.
6. Keep `match` class-side only, inferred from the paired factory declaration.
7. If a class omits the same-named `match` for a factory, synthesize an
   implicit `match` returning `1`.
8. Accept the Level B restriction that a class implementing multiple
   interfaces may only use `*` for one of them.
9. Define factory selection as "highest positive score wins; ties break
   alphabetically by concrete class name".
10. Extend the existing `.meta` constant-pool metadata chain with interface,
    implements, and interface-member records, while synthesizing intrinsic
    class interfaces from the existing class metadata path.
11. Build runtime interface dispatch tables and factory candidate lists during
    VM link in C, rather than in Rexx or as literal serialized `.ptable` /
    `.ftable` rows.
12. Use `srcptable` + `srcfproc` + `dcall` as the initial VM execution model.
13. Keep singletons, destructors, and richer external-object lifecycle syntax
    out of Level B and defer them to Level G or user-land conventions.
14. Adopt the structured internal mangling scheme above, while treating
    metadata selectors rather than mangled strings as the semantic dispatch ABI.
15. Add optional namespace qualification using `namespace::name`, where the
    left side of `::` is always a namespace and must already be imported.
