# Level B Tutorial

This tutorial is for experienced Rexx programmers who want to write practical
cRexx Level B programs in the Release 1 beta 3 documentation line. It assumes
you know Rexx ideas such as `say`, `parse`, `do`, `arg`, and built-in
functions, but it does not assume you have used cRexx's typed compiler,
modules, classes, or interfaces before.

Level B is Rexx-family code, but it is not Classic Rexx compatibility mode. It
is the implemented typed cRexx language used by the toolchain, libraries, tests,
and examples in this repository. When this tutorial says "Level B", it means
what the current compiler accepts now, not a planned Level C or Level G feature.

For exact syntax details, keep these reference pages close:

- [Data types](../crexx_language_reference/data_types.md)
- [Statements](../crexx_language_reference/statements.md)
- [Classes and interfaces](../crexx_language_reference/classes_and_interfaces.md)
- [Namespaces](../crexx_language_reference/namespace.md)
- [Toolchain overview](toolchain.md)

For more implemented examples in a source download, start with these paths:

- `examples/hello.crexx`, 
- `examples/ooBank.crexx`,
- `lib/rxfnsb/rexx/stem.crexx`,
- `compiler/tests/rexx_src/interface_showcase_same_module.crexx`, and
- `compiler/tests/rexx_src/reference_source_iterator.crexx`.

## 1. Why Level B Still Feels Like Rexx

The first pleasant surprise is that much of the surface still reads like Rexx:

- `say` prints a value.
- `arg` receives command-line or procedure arguments.
- `do`, `if`, `select`, `leave`, `iterate`, and `return` shape control flow.
- `parse`, `address`, `signal`, and `trace` are present in the current beta
  surface.
- The `rxfnsb` library provides many familiar built-in functions.

The biggest difference is that Level B gives the compiler real information:
values have types, modules have namespaces, procedures declare arguments and
return types, and object contracts are checked.

That trade is worth leaning into. Do not write vague Classic Rexx and hope the
compiler guesses your intent. Write a small amount of explicit Level B so the
compiler and VM can help you.

## 2. First Program and Workflow

The usual source extension for new cRexx code is `.crexx`. For reusable code,
start with `options levelb` and import the Level B standard library when you
use it.

```rexx <!--levelbtut1.crexx-->
options levelb
import rxfnsb

say "Hello Level B"
say "length=" || length("Rexx")
```

From the repository root:

```bash
crexx hello.crexx
```

Expected output:

<!--splice--crexx levelbtut1.crexx-->

The `crexx` driver wraps the normal compile, assemble, and run path. The
underlying stages are still visible:

1. `rxc` compiles source to `.rxas`.
2. `rxas` assembles `.rxas` to `.rxbin`.
3. `rxvm` or `rxvme` runs `.rxbin`.
4. `rxlink` can combine modules into a linked image.

The commands in this tutorial assume cRexx has been installed and `crexx`,
`rxc`, `rxas`, and `rxvm` are on your `PATH`. In a source download before
installation, use the matching binaries from your build directory instead.
For day-to-day scripts, use `crexx`. For multi-module class/interface examples
where provider modules must be present at runtime, compile the modules and run
or link the resulting `.rxbin` files explicitly. The mini-project below shows
that shape.

## 3. Source File Conventions

Use this header shape for committed Level B code. This is a header fragment,
not a complete program:

```rexx <!--headerfragment.crexx-->
options levelb
namespace myapp expose public_symbol
import rxfnsb
```

Keep `options`, `namespace`, and `import` in the leading header block. The
compiler pre-scans that header before full parsing, and the rest of the repo
uses this layout consistently.

The pieces mean:

- `options levelb`: compile this file as Level B.
- `namespace myapp expose public_symbol`: publish selected symbols from this
  module.
- `import rxfnsb`: make another namespace visible to this source file.

Headerless top-level scripts run through the `crexx` driver get practical
defaults, including Level B and `rxfnsb`, but tutorial and library code should
be explicit.

Command-line arguments arrive through `arg`:

```rexx <!--levelbtut2.crexx-->
options levelb

arg words = .string[]

if words[0] = 0 then do
  say "no arguments"
  return
end

say "count=" || words[0]
do i = 1 to words[0]
  say i || ":" || words[i]
end
```

Run with arguments by putting `-args` last:

```bash <!--runargs.sh-->
crexx args.crexx -args alpha beta
```

Expected output:

<!--splice--crexx levelbtut2.crexx -args alpha beta-->

<!-- ```bash -->
<!-- count=2 -->
<!-- 1:alpha -->
<!-- 2:beta -->
<!-- ``` -->

## 4. Types You Will Use Immediately

Level B values have types. The most common built-in source spellings are:

| Type | Use |
| --- | --- |
| `.string` | UTF-8 text |
| `.int` | integer values |
| `.float` | binary floating-point values |
| `.decimal` | decimal numeric values |
| `.boolean` | truth values |
| `.binary` | arbitrary bytes |
| `.object` | object-shaped values |
| `.void` | no return value |

Write `.boolean` in current Level B source. Some Rexx or C habits may suggest
`.bool`, but `.boolean` is the documented Level B spelling.

A local variable can be inferred from its first assignment, or you can declare
the type first and assign the value next. The latter is useful when the value
will be filled later or when you want a specific target type.

```rexx <!--levelbtut3.crexx-->
options levelb
import rxfnsb

title = .string
count = .int
price = .float
ready = .boolean
payload = "4f4b"x as .binary
words = .string[]
people = .stem()

title = "Level B"
count = 2
price = 1.5
ready = 1

call arrayappend words, "alpha"
call arrayappend words, "beta"
words.3 = "gamma"

people.ada = "analytical"
people["grace.hopper"] = "compiler"
tail = "ada"

say title || ":" || count
say typeof(price)
say typeof(ready)
say binlength(payload)
say words[0] || ":" || words.1 || "," || words[2] || "," || words.3
say people.tail
say people["grace.hopper"]
```

Expected output:

<!--splice--crexx levelbtut3.crexx-->

<!-- ```bash -->
<!-- Level B:2 -->
<!-- .float -->
<!-- .boolean -->
<!-- 2 -->
<!-- 3:alpha,beta,gamma -->
<!-- analytical -->
<!-- compiler -->
<!-- ``` -->

Practical notes:

- Arrays are typed: `.string[]`, `.int[]`, `.object[]`, and so on.
- Default growable arrays are commonly used with one-based element indexes,
  but Level B also supports explicit bounds such as `.int[0 to 10]` and
  `.int[-2 to *]`.
- `array[0]` or `array.0` reads the current high water mark.
- Elements can be read and written with bracket notation (`words[1]`) or Rexx
  stem-style dot notation (`words.1`).
- Do not assign to index `0`; use ordinary element assignment or helpers such
  as `arrayappend`, `arrayinsert`, and `arraydelete`.
- The `.stem` class is a string-to-string keyed container for classic Rexx
  compound-variable style data. It supports dotted tails such as `people.ada`
  and bracket keys such as `people["grace.hopper"]`.
- `.string` is valid UTF-8 text in normal builds. Use `.binary` for bytes.

## 5. Procedures, Arguments, Returns, and Varargs

A Level B procedure can declare its return type after `=`, and it binds call
arguments with `arg`.

```rexx <!--levelbtut4.crexx-->
options levelb

main: procedure
  total = sum(2, 3, 5)
  x = 10
  call bump(x)

  say "total=" || total
  say "x=" || x
  return

sum: procedure = .int
  arg first = .int, ... = .int
  total = first
  do i = 1 to arg[]
    total = total + arg[i]
  end
  return total

bump: procedure = .void
  arg expose value = .int
  value = value + 1
  return
```

Expected output:

<!--splice--crexx levelbtut4.crexx-->

<!-- ```bash -->
<!-- total=10 -->
<!-- x=11 -->
<!-- ``` -->

The important habits are:

- `arg name = .type` is by value. Changes in the procedure do not update the
  caller's variable.
- `arg expose name = .type` is by reference. Use it only when the procedure is
  meant to update caller-owned state.
- A final `... = .type` accepts a variable-length tail.
- `arg[]` is the number of values in that tail, and `arg[i]` reads tail values.

`procedure expose` is different from `arg expose`: it binds module-global
storage into a procedure. Prefer explicit arguments and return values until you
really need shared module state.

## 6. Expressions, Assignment, Casts, and Comparisons

Level B keeps Rexx operators, but type checking happens before bytecode is
emitted.

Use these idioms:

- Assign a variable once with the type you mean; later assignments must be
  compatible.
- Use `expr as .type` when you need a checked conversion or object cast.
- Use `expr is .type` for object/interface tests.
- Use `typeof(expr)` for diagnostics and runtime type introspection.
- Use `=` for Rexx-style equality and `==` when you mean exact string
  comparison after string promotion.

For binary data, be explicit. This is a fragment:

```rexx <!--payload.crexx-->
payload = "4f4b"x as .binary
```

For objects, cast at the boundary. This is a fragment:

```rexx <!--cast.crexx-->
generic = selected as .object
restored = generic as .asset
```

If a cast cannot succeed, Level B raises a conversion signal instead of silently
changing the meaning of the value.

## 7. Control Flow

The usual Rexx control-flow tools are present. `select` has both Classic Rexx
condition style and a switch-like expression style. `leave` and `iterate` work
on loops. Expression-form `do ... end` can return a value with `leave with`.

```rexx <!--levelbtut5.crexx-->
options levelb

arg words = .string[]

if words[0] = 0 then words[1] = "red"

kept = 0
do i = 1 to words[0]
  select
    when words[i] = "skip" then iterate
    when words[i] = "stop" then leave
    otherwise say "word=" || words[i]
  end
  kept = kept + 1
end

summary = do
  if kept > 1 then leave with "many"
  leave with "few"
end

say "summary=" || summary
```

Run:

```bash <!--controlflow.sh-->
crexx control_flow.crexx -args alpha skip beta stop gamma
```


Expected output:

<!--splice--crexx levelbtut5.crexx -args alpha skip beta stop gamma-->

<!-- ```bash -->
<!-- word=alpha -->
<!-- word=beta -->
<!-- summary=many -->
<!-- ``` -->

## 8. Rexx-Flavoured Features

Level B keeps several Rexx features that make the language feel direct:

- `say` prints strings.
- `parse` is implemented through the certified `PARSE` exit.
- `address` sends commands to an external environment and can capture output.
- `signal` raises and handles condition objects.
- `trace` enables VM-backed tracing for debugging.

This example keeps the output small:

```rexx <!--levelbtut6.crexx-->
options levelb
import rxfnsb

main: procedure
  parse value "Ada Lovelace,1815" with first last "," year
  say last || ":" || year

  out = .string[]
  err = .string[]
  address crexx "echo 42" output out error err
  say "address=" || out[1]

  handled = ""
  do
    signal other "from block"
  on signal other as problem
    handled = problem.name() || ":" || problem.message()
  end
  say handled
  return
```

Expected output:

<!--splice--crexx levelbtut6.crexx-->

<!-- ```bash -->
<!-- Lovelace:1815 -->
<!-- address=#42 -->
<!-- OTHER:from block -->
<!-- ``` -->

Trace is useful but intentionally noisy. These are reference-only forms, not a
runnable tutorial example:

```text
trace results
trace asm
trace llm to file "trace.jsonl"
trace unsuppress namespace rxfnsb
trace off
```

Do not add `TRACE`, `PARSE`, or `ADDRESS` directly to `lib/rxfnsb/rexx/`
library sources while debugging the library build. Those files are built with
compiler exits disabled. Write a small caller program instead.

## 9. Modules and Libraries

Each source file is a module. Public module identity comes from `namespace`,
not from the filename alone. A module exposes selected symbols; another module
imports the namespace and calls those symbols.

Library module:

```rexx <!--greetings.crexx-->
options levelb
namespace greetings expose salutation count_items

salutation: procedure = .string
  arg name = .string
  return "Hello, " || name

count_items: procedure = .int
  arg items = .string[]
  return items[0]
```

Caller:

```rexx <!--modulesmain.crexx-->
options levelb
import greetings

names = .string[]
names[1] = "Ada"
names[2] = "Grace"

say greetings..salutation(names[1])
say "names=" || greetings..count_items(names)
```

Run the caller with the directory containing `greetings.crexx` on the source
import path:

```bash <!--runmodules.sh-->
crexx modules_main.crexx -s/path/to/examples
```

Expected output:

<!--splice--crexx modulesmain.crexx-->

<!-- ```bash -->
<!-- Hello, Ada -->
<!-- names=2 -->
<!-- ``` -->

Use `namespace..symbol` for qualified calls. The older
`namespace::symbol` spelling is accepted for compatibility, but new examples
should prefer the double-dot form.

## 10. Classes

Classes hold attributes and methods. A class factory is written as `*: factory`
or `name: factory`; do not write a return type on a factory. In a class
factory, bare `return` returns the object being constructed. This fragment is
from the complete interface example in the next section:

```rexx <!--tutclasses1-->
fileasset: class implements .asset
  _name = .string

  *: factory
    arg name = .string
    _name = name
    return

  kind: method = .string
    return "file"

  name: method = .string
    return _name
```

Ordinary application classes should let the compiler allocate attribute
storage. The `with register.N` form exists for low-level VM/native interop, not
for day-to-day business objects.

## 11. Interfaces

Interfaces define contracts. Classes implement them. Interface methods can be
abstract, or they can provide a default/final body. In current Level B, an
interface method with a body is final: a class can rely on it, but cannot
override it.

Here is a complete same-file interface and provider example:

```rexx <!--tutclasses2-->
options levelb
namespace tutorial_objects

main: procedure
  selected = .asset("log.txt")
  fallback = .asset("memo")

  say selected.describe()
  say fallback.describe()

  if selected is .fileasset then say "file selected"

  generic = .object
  generic = selected as .object
  restored = generic as .asset
  say typeof(restored)
  return

asset: interface
  *: factory
  arg name = .string

  describe: method = .string
    return kind() || ":" || name()

  kind: method = .string
  name: method = .string

fileasset: class implements .asset
  _name = .string

  *: match
    arg name = .string
    if name = "log.txt" then return 100
    return 0

  *: factory
    arg name = .string
    _name = name
    return

  kind: method = .string
    return "file"

  name: method = .string
    return _name

cacheasset: class implements .asset
  _name = .string

  *: factory
    arg name = .string
    _name = name
    return

  kind: method = .string
    return "cache"

  name: method = .string
    return _name
```

Expected output:

```text <!--outtex.txt-->
file:log.txt
cache:memo
file selected
.tutorial_objects..fileasset
```

The `asset` interface owns the public factory. `fileasset` and `cacheasset` are
providers. The `*: match` method is a class-side selector. Positive scores
accept the provider, zero or negative scores reject it, and the highest score
wins.

## 12. Object Use

Use object values through factories, methods, interfaces, type tests, and
checked casts:

- `.asset("log.txt")` calls an interface factory.
- `.fileasset("log.txt")` calls a concrete class factory.
- `value.method()` invokes a method.
- `value is .asset` tests whether the concrete object implements an interface.
- `value as .asset` casts after a runtime check.
- `typeof(value)` reports the concrete runtime type.

Level B does not implicitly call a `toString()` method for arbitrary objects.
If you want text, expose a method such as `format`, `describe`, or `name`, then
call it explicitly.

Level B also does not currently implement interface inheritance, interface
attributes, overloads, singleton declarations, or destructor/finalizer syntax.

## 13. Collections and Iteration

Current Level B collection patterns are explicit:

- Use typed arrays such as `.string[]`, `.int[]`, and `.object[]`.
- Default growable arrays are usually shown with one-based element indexes,
  but array bounds can be explicit.
- Read `array[0]` for the current count.
- Use bracket notation (`items[1]`) or stem-style dot notation (`items.1`) for
  array elements.
- Use `arrayappend`, `arrayinsert`, `arraydelete`, and related `rxfnsb` helpers
  for mutating array shape.
- Use `objectarrayappend`, `objectarrayinsert`, and related helpers for
  `.object[]` when you need object-array mutation helpers.
- Store concrete objects in `.object[]` with an explicit `as .object` upcast,
  then cast back to the expected interface or class at the read boundary.
- Use `.stem` when the natural model is a Rexx compound variable or keyed
  string map rather than an ordered typed array.

The mini-project below uses this pattern. This is a fragment:

```rexx <!--tutcoll1-->
items = .object[]
items[1] = .ledger..entry("coffee", -3) as .object

item = items[1] as .ledger..entry
say item.format()
```

For iterator-like classes, current Level B uses explicit references when the
iterator should see live container state. Use snapshots when the iterator should
see a stable copy.

## 14. References

References are explicit weak aliases to storage. They do not keep a target
alive. If the target storage goes away, `refvalid(ref)` returns false and
using the reference raises `REFERENCE_INVALID`.

Use the four source forms deliberately:

- `reference target`: create a weak reference to aliasable storage.
- `local = dereference ref`: link a current-scope local to the target.
- `snapshot ref`: make a deep copy of the current target.
- `refvalid(ref)`: test whether the target is still valid.

This example contrasts a live reference with a snapshot:

```rexx <!--tutref1.crexx-->
options levelb
import rxfnsb
namespace tutorial_references

main: procedure
  list = .Bag()
  call list.add("red")
  call list.add("blue")

  live = list.liveCounter()
  snap = list.snapshotCounter()

  call list.add("green")

  say "live=" || live.count()
  say "snapshot=" || snap.count()
  return

Bag: class
  values = .string[]
  item_count = .int

  *: factory
    initial = .string[]
    values = initial
    item_count = 0
    return

  add: method = .void
    arg value = .string
    call arrayappend values, value
    item_count = item_count + 1
    return

  size: method = .int
    return item_count

  liveCounter: method = .LiveCounter
    return .LiveCounter(reference self)

  snapshotCounter: method = .SnapshotCounter
    return .SnapshotCounter(reference self)

LiveCounter: class
  bag_ref = reference .Bag

  *: factory
    arg source = reference .Bag
    bag_ref = source
    return

  count: method = .int
    if \refvalid(bag_ref) then return -1
    bag = dereference bag_ref
    return bag.size()

SnapshotCounter: class
  bag_copy = .Bag

  *: factory
    arg source = reference .Bag
    bag_copy = snapshot source
    return

  count: method = .int
    return bag_copy.size()
```

Expected output:

<!--splice--crexx tutref1.crexx-->

<!-- ```text <\!--out2.txt-\-> -->
<!-- live=3 -->
<!-- snapshot=2 -->
<!-- ``` -->

Keep reference boundaries visible. Level B does not let you write convenience
forms such as `list_ref.add(...)` or `items_ref[i]` directly through the
reference.

## 15. Mini-Project: A Typed Ledger Module

This mini-project uses two source files: a reusable module with an interface
and class, and a small application that imports it.

`ledger.crexx`:

```rexx <!--ledger.crexx-->
options levelb
namespace ledger expose entry ledger_total

entry: interface
  *: factory
  arg label = .string, amount = .int

  label: method = .string
  amount: method = .int

  format: method = .string
    return label() || "=" || amount()

ledgerentry: class implements .entry
  _label = .string
  _amount = .int

  *: factory
    arg label = .string, amount = .int
    _label = label
    _amount = amount
    return

  label: method = .string
    return _label

  amount: method = .int
    return _amount

ledger_total: procedure = .int
  arg entries = .object[]
  total = 0
  do i = 1 to entries[0]
    item = entries[i] as .entry
    total = total + item.amount()
  end
  return total
```

`ledger_app.crexx`:

```rexx <!--ledgerapp.crexx-->
options levelb
import ledger

items = .object[]
items[1] = .ledger..entry("coffee", -3) as .object
items[2] = .ledger..entry("book", -12) as .object
items[3] = .ledger..entry("gift", 20) as .object

do i = 1 to items[0]
  item = items[i] as .ledger..entry
  say item.format()
end

say "total=" || ledger..ledger_total(items)
```

Compile both modules, then run with the standard library, provider module, and
application module loaded:

```bash
CREXX_BIN=$(dirname "$(command -v crexx)")
rxc -i "$CREXX_BIN" -o main ledger_app.crexx
rxas -o main.rxbin main
rxc -i "$CREXX_BIN" -o ledger ledger.crexx
rxas -o ledger.rxbin ledger
rxvm "$CREXX_BIN/library.rxbin" ledger.rxbin main.rxbin
```

Expected output:

```bash
coffee=-3
book=-12
gift=20
total=5
```

The explicit `rxvm` command matters here because interface factory providers
are discovered from the modules present in the runtime image. For deployable
programs, `rxlink` is the usual way to combine those modules into one image.

## 16. Migration Guide For Classic Rexx Habits

Classic Rexx habit:
: Rely on untyped variables.

Level B habit:
: Let obvious literals infer types, or declare variables with `.string`,
  `.int`, `.float`, `.boolean`, `.binary`, arrays, or object contracts.

Classic Rexx habit:
: Assume built-in functions are always visible.

Level B habit:
: `import rxfnsb` in reusable source, unless you are deliberately writing a
  headerless `crexx` driver script.

Classic Rexx habit:
: Use stems as flexible bags of values.

Level B habit:
: Use typed arrays and array helper functions for ordered typed data. Use
  `.stem` for classic compound-variable style keyed strings. Treat `array[0]`
  as a count, not as a writable stem slot.

Classic Rexx habit:
: Share state freely through globals.

Level B habit:
: Prefer arguments and returns. Use `namespace ... expose`, `procedure expose`,
  or `arg expose` only for intentional sharing.

Classic Rexx habit:
: Treat text and bytes as the same thing.

Level B habit:
: Use `.string` for valid UTF-8 text and `.binary` for arbitrary bytes. Convert
  explicitly.

Classic Rexx habit:
: Expect dynamic object behavior.

Level B habit:
: Define an interface, implement it with classes, and use checked casts and
  type tests at boundaries.

## 17. Known Beta 3 Boundaries

Level B is the main implemented Release 1 beta language, but this is still a
beta line. Current boundaries that matter to tutorial code:

- Level B is not Classic Rexx compatibility mode. Level C is where Classic
  compatibility work belongs, and normal Level C compilation is not a beta 3
  contract unless the beta 3 release notes explicitly say otherwise.
- Level G has real early library work, including `rxfnsg`, but it is not the
  baseline user language for this release line.
- Interface default methods are final.
- Interface inheritance, interface attributes/state, overloads, singleton
  declarations, external-object adoption syntax, and destructor/finalizer
  syntax are not current Level B features.
- Objects do not implicitly convert to strings.
- References are explicit and weak. They are powerful, but not a general
  replacement for ordinary arguments, returns, or object methods.
- Multi-module interface-provider programs need all provider modules loaded or
  linked at runtime.

<!-- ## Tested Example Appendix -->

<!-- The examples below were tested from the repository root with installed tools on -->
<!-- `PATH`. The commands use `$WORK` for the directory containing the tutorial -->
<!-- example files. -->

<!-- | Example | Command used | Expected output | Known limitation | -->
<!-- | --- | --- | --- | --- | -->
<!-- | First program | `crexx $WORK/hello.crexx` | `Hello Level B`<br>`length=4` | None. | -->
<!-- | Command-line args | `crexx $WORK/args.crexx -args alpha beta` | `count=2`<br>`1:alpha`<br>`2:beta` | `-args` must be the final driver option before user arguments. | -->
<!-- | Types, arrays, and stems | `crexx $WORK/types_arrays.crexx` | `Level B:2`<br>`.float`<br>`.boolean`<br>`2`<br>`3:alpha,beta,gamma`<br>`analytical`<br>`compiler` | Uses explicit declarations plus assignment, not constructor-call examples. | -->
<!-- | Procedures and varargs | `crexx $WORK/procedures.crexx` | `total=10`<br>`x=11` | None. | -->
<!-- | Control flow | `crexx $WORK/control_flow.crexx -args alpha skip beta stop gamma` | `word=alpha`<br>`word=beta`<br>`summary=many` | None. | -->
<!-- | Parse, address, signal | `crexx $WORK/rexx_features.crexx` | `Lovelace:1815`<br>`address=#42`<br>`OTHER:from block` | Shell output assumes `echo` is available. | -->
<!-- | Namespace import | `crexx $WORK/modules_main.crexx -s$WORK` | `Hello, Ada`<br>`names=2` | Demonstrates source import. Interface-provider programs need runtime loading too. | -->
<!-- | Classes and interfaces | `crexx $WORK/objects.crexx` | `file:log.txt`<br>`cache:memo`<br>`file selected`<br>`.tutorial_objects..fileasset` | Same-file providers keep the run command simple. | -->
<!-- | References | `crexx $WORK/references.crexx` | `live=3`<br>`snapshot=2` | None. | -->
<!-- | Ledger mini-project | See the command block below. | `coffee=-3`<br>`book=-12`<br>`gift=20`<br>`total=5` | All provider modules must be loaded or linked at runtime. | -->

<!-- Ledger mini-project command sequence: -->

<!-- ```bash <\!--commandsequence.sh-\-> -->
<!-- CREXX_BIN=$(dirname "$(command -v crexx)") -->
<!-- rxc -i "$CREXX_BIN" -o "$WORK/main" "$WORK/ledger_app.crexx" -->
<!-- rxas -o "$WORK/main.rxbin" "$WORK/main" -->
<!-- rxc -i "$CREXX_BIN" -o "$WORK/ledger" "$WORK/ledger.crexx" -->
<!-- rxas -o "$WORK/ledger.rxbin" "$WORK/ledger" -->
<!-- rxvm "$CREXX_BIN/library.rxbin" "$WORK/ledger.rxbin" "$WORK/main.rxbin" -->
<!-- ``` -->

<!-- The TRACE forms in section 8 are intentionally marked reference-only because -->
<!-- trace output is diagnostic and can be much larger than the tutorial needs. -->
