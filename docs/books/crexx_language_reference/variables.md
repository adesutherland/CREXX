# Variables {#variables}

Level B variables are typed. The compiler can infer many local variable types,
but once a variable has a type, later assignments must be compatible with that
type.

## Keywords

Keywords, instruction names, and operators cannot be used as variable names.

## Declaration by Assignment

A variable is often declared by its first assignment:

```rexx
count = 0
price = 1.25
name = "Ada"
ready = .boolean(1)
```

The inferred types are based on the assigned expression. Use constructors when
the intended type needs to be explicit:

```rexx
count = .int(0)
name = .string("Ada")
ratio = .float(1.25)
money = .decimal("1.25")
payload = .binary()
```

The canonical integer type name is `.int`.

## The System Variable `rc`

`rc` has the integer type `.int`, including with `options numeric_classic`.
The [ADDRESS statement](statements.md#address) stores the command's integer
status in it. It can also hold an integer result assigned by the program, but
it is not a general-purpose string result variable.

Use a separate variable for a function that returns text:

```rexx
result_text = MyFunc()
say result_text
```

Assigning nonnumeric text to `rc` requires an invalid string-to-integer
conversion. The compiler reports `BAD_CONVERSION` when it can establish the
invalid value during compilation; a value obtained at runtime raises
`CONVERSION_ERROR`. The same conversion rules apply to an ordinary `.int`
variable. Declaring `rc = .string` does not change its system type.

## Block Scope

A first assignment inside a `DO ... END` block creates a binding in that block
unless the variable already exists in an enclosing scope. Sibling blocks do
not share newly created bindings. This differs from Classic Rexx's procedure
scope and matters when computing a result in alternative branches.

Declare the shared result before the conditional:

```rexx
choose: procedure = .string
  arg flag = .string
  text = .string
  if flag = "1" then do
    text = "from-if"
  end
  else do
    text = "from-else"
  end
  return text
```

Without `text = .string`, the assignments and final read create separate
bindings. The compiler warns `NOT_IN_SAME_SCOPE`, and the final read returns
the uninitialized variable's name instead of either branch's string. A bare
type declaration is sufficient; no dummy initial value is needed.

## Arrays

Array variables are declared with a typed array expression:

```rexx
args = .string[]
scores = .int[10]
grid = .int[10, 10]
```

These bare type expressions declare storage; they do not create and assign a
fresh empty array whenever execution reaches the declaration. In particular,
repeating `args = .string[]` in a procedure that exposes `args` does not clear
its existing module-global elements. With `import rxfnsb`, use
`call arraydrop args` when the program needs to empty an existing array.

Array arguments use the same notation in procedure signatures:

```rexx
main: procedure = .int
  arg args = .string[]
```

## Object Variables

Class and interface values are object-shaped. Factories use dotted class or
interface names:

```rexx
asset = .asset("log.txt")
box = .box()
```

When a class implements an interface, a class instance can be assigned to that
interface contract. Use `expr is .type`, `expr as .type`, and `typeof(expr)`
when code needs runtime type checks or concrete type information.

## Globals and Expose

Top-level values in a namespace are global to that module. Exposed globals can
be imported by other modules. Procedures have their own local scope unless
state is deliberately exposed through the current Level B expose mechanisms.

Prefer explicit arguments and return values for ordinary application code. Use
global exposed state for library constants, runtime integration points, and
cases where shared module state is genuinely the simplest contract.
