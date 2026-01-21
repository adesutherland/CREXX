# RXPP User Documentation

## 1. Introduction

RXPP is a **source-to-source pre-compiler** for CREXX (levelb) Rexx programs. It extends standard Rexx with compile-time features such as macros, conditional compilation, structured includes, stem handling, and syntactic conveniences. RXPP runs *before* execution and produces plain, valid CREXX Rexx output with no runtime dependency on RXPP itself.

RXPP is intended for developers who want to structure larger Rexx programs cleanly while keeping the runtime environment simple and transparent.

---

## 2. Basic Usage and Workflow

RXPP is used as a pre-compilation step. It reads a Rexx source file, applies all RXPP directives and rewrites, and writes a transformed Rexx file.

Typical workflow:

1. Write RXPP-enabled Rexx source
2. Run RXPP on the source file
3. Execute or distribute the generated Rexx output

RXPP never modifies the original input file.

### Command-line usage (conceptual)

```
rxpp -I input.rxpp -O output.rexx -M maclib.rexx
```

Options:
- `-I` / `-IN`   input file
- `-O` / `-OUT`  output file
- `-M`           macro library
- `-VERBOSE0`    suppress informational output

### Output artifacts

- Precompiled Rexx source file
- Optional linker include file listing imported modules

---

## 3. RXPP Language Extensions (User View)

### 3.1 RXPP Macros

This section documents the RXPP macro system in more depth. RXPP macros are **compile-time only** constructs: they are fully expanded by the RXPP preprocessor and do not exist at runtime. After preprocessing, only standard CREXX Rexx code remains.

Macros are intended to improve readability, reduce repetition, and provide small, reusable language extensions without runtime overhead.

---

#### 3.1.1 Defining macros

Macros are defined using the `##DEFINE` directive:

```rexx
##DEFINE NAME(args...) { body }
```

- `NAME` is case-insensitive at call sites
- `args` are optional
- `body` is the expansion text
- The body may expand to an expression, a statement, or multiple statements

Example:

```rexx
##define SQUARE(x) { x*x }
```

Usage:

```rexx
say SQUARE(5)   /* expands to: say 5*5 */
```

---

####  3.1.2 Expression helper macros

Expression macros expand to a value and can be used anywhere an expression is valid.

```rexx
##define CUBE(x)    { x*x*x }
##define double(x)  { 2*x }
```

Example:

```rexx
n = 3
say CUBE(n) + double(n)
```

---

####  3.1.3 Statement and block macros

Macros may expand to one or more Rexx statements. Multiple statements are typically separated using semicolons.

```rexx
##define swap(a,b) { temp=a; a=b; b=temp }
```

Usage:

```rexx
swap(x,y)
```

---

####  3.1.4 Control-flow shortcuts

Macros can encapsulate common control-flow patterns.

```rexx
##define repeat(n)       { do __i=1 to n }
##define guard(cond,act) { if cond<>0 then do; act; return; end }
```

Examples:

```rexx
repeat(5)
   say "hello"
end
```

```rexx
guard(user='', 'say "missing user"')
```

---

####  3.1.5 Stem and array convenience macros

Macros are frequently used to simplify common stem and array operations.

```rexx
##define hi(stem)          { stem.0 }
##define foreach(stem,ix)  { do ix=1 to stem.0 }
##define forpair(array,k,v){ do k=1 to array.0; v=array.k }
```

Example:

```rexx
foreach(items,i)
   say i items.i
end
```

---

####  3.1.6 File helper macros

Macros can wrap native helper functions (provided by `precomp` or other modules) to form concise I/O helpers.

```rexx
##define readfile(stem, file)  { stem.1='' ; call readall stem,file,-1 }
##define writefile(file, stem) { call writeall(file, stem, 1, stem.0) }
```

Example:

```rexx
readfile(lines, "input.txt")
say hi(lines) "lines loaded"
writefile("out.txt", lines)
```

---

#### 3.1.7 Variadic macros

RXPP supports variadic macros using `...`. Variadic macros generate repeated expansions for each supplied argument.

```rexx
##define SetStem(name, ...) { name.$indx=arglist.$indx }
```

Example:

```rexx
SetStem(a, "x", "y", "z")
```

Conceptually, this expands into a sequence of assignments, one per argument.

---

####  3.1.8 Multi-line macro definitions

Macro bodies may span multiple lines using `\` as a continuation marker. This is useful for more complex expansions.

```rexx
##define cparse(string,template) \
   {_pass_variable.1='' ; _pass_variable_content.1='' \
    string2Parse=string; parsetemplate=template \
    call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content \
   }
```

---

####  3.1.9 Notes and guidelines

- RXPP macros are **textual expansions**, not syntax-tree transformations.
- Macros should expand to valid Rexx when copied verbatim into surrounding code.
- Prefer unique or prefixed temporary variable names to avoid collisions.
- Inspect the generated output if macro behavior is unclear.

Macros are a powerful but deliberately simple mechanism; they trade structural awareness for predictability, speed, and ease of implementation.


### 3.2 Conditional compilation

```rexx
##SET DEBUG 1
##IF DEBUG
say "Debug"
##END
```

### 3.3 File inclusion

```rexx
##INCLUDE common.rexx
```

### 3.4 Arrays and globals

RXPP provides two related preprocessor directives, `##ARRAY` and `##GLOBAL`, to simplify the declaration of multiple variables with the same type and lifetime.

#### Defining arrays with `##ARRAY`

```rexx
##ARRAY STRING string-var1, string-var2, ... , string-varn
##ARRAY INT    int-var1,    int-var2,    ... , int-varn
##ARRAY FLOAT  float-var1,  float-var2,  ... , float-varn
```

The `##ARRAY` directive declares one or more variables as typed arrays in a single statement. Each name listed is expanded by RXPP into an initialized array of the specified type.

This is a compile-time convenience only: the directive is expanded into ordinary Rexx assignments, and no runtime overhead or special runtime support is introduced.

#### Defining global variables with `##GLOBAL`

```rexx
##GLOBAL STRING string-var1, string-var2, ... , string-varn
##GLOBAL INT    int-var1,    int-var2,    ... , int-varn
##GLOBAL FLOAT  float-var1,  float-var2,  ... , float-varn
```

The `##GLOBAL` directive declares one or more variables as global values of a given type. RXPP ensures that these variables are visible across the program by collecting them into the generated `namespace ... expose` clause.

Global arrays can be declared explicitly by using `[]`:

```rexx
##GLOBAL STRING string-var1[], string-var2[], ... , string-varn[]
##GLOBAL INT    int-var1[],    int-var2[],    ... , int-varn[]
##GLOBAL FLOAT  float-var1[],  float-var2[],  ... , float-varn[]
```

In this form, each variable is expanded as a global array rather than a scalar value.

As with `##ARRAY`, `##GLOBAL` is resolved entirely during preprocessing. It exists to improve readability and maintainability of Rexx programs by grouping related declarations, not to introduce new runtime semantics.
```

### 3.5 OO-style calls

```rexx
map.put("a",1)
x = map.get("a")
```

## 3.6 Stem expressions and dynamic tails

RXPP supports **stem paths with multiple tails**, where each tail segment can be either:

- a **literal name** (`a.tail1.tail2.tail3`)
- a **dynamic name** taken from a variable (`a.t1.t2.t3`)
- a **computed tail** using `.( ... )` expressions (`Customer.(abc||4711).name`)

This allows stems to be used as *multi-dimensional associative structures* (tables or records), while preserving readable Rexx syntax.

---

### Constant tails (baseline)

Literal tail names define a fixed structure:

```rexx
a.tail1.tail2.tail3 = "something"
say a.tail1.tail2.tail3
```

---

### Dynamic tails via variables

If a tail segment matches a variable name, RXPP treats that segment as **dynamic**, meaning it is replaced with the *value* of that variable:

```rexx
t1 = "b"
t2 = "c"
t3 = "d"
a.t1.t2.t3 = "v3"
say a.t1.t2.t3
```

This enables table-style access patterns:

```rexx
customer = "IBM"
Customer.customer.name = "International Business Machines"
say Customer.customer.name
```

---

### Computed tail segments with `.( ... )`

When a tail must be derived from an expression (concatenation, arithmetic, or function calls), RXPP supports **computed tail segments**:

```rexx
Customer.(abc||4711).name = "Microsoft"
say Customer.(abc||4711).name
```

The expression inside `.( ... )` is evaluated first and then used as the effective stem segment.

---

### Multiple stems in one statement

RXPP can safely rewrite multiple stem expressions appearing in the same line:

```rexx
if List.(213+4711).(harry||rose) \= '' then say List.(213+4711).(harry||Mary)
```

---

### Join-style lookups (stem-to-stem indirection)

A common pattern is storing a key in one stem and using it to access another stem (similar to a foreign-key join):

```rexx
Order.1001.cust = "IBM"
custref = Order.1001.cust
say Customer.custref.name
```

---

### Stem cardinality rules (important)

By default, RXPP follows **CREXX stem semantics**:

- A name with **one tail** (e.g. `a.b`) is treated as a **standard CREXX array access**.
- A name is considered a **RXPP stem expression** only if it has **at least two tail segments**:

```rexx
/* CREXX array */
a.b = 1

/* RXPP stem */
a.b.c = 2
```

This rule prevents accidental reinterpretation of ordinary CREXX arrays.

#### Lifting the restriction with `##CFLAG dotisstem`

The minimum-tail restriction can be relaxed explicitly:

```rexx
##CFLAG dotisstem
```

With this flag enabled, RXPP treats names with **a single tail** (`a.b`) as stem expressions as well. This is useful for projects that consistently use dotted names as associative keys.

Use this flag carefully, as it changes how RXPP interprets dotted variable names globally.

---

### Rules summary (what RXPP considers a valid stem)

1. A stem consists of a **root** followed by one or more **tail segments** separated by dots.
2. By default, RXPP recognizes a stem only if it has at least two tail segments; otherwise, the expression is interpreted as a standard CREXX array access.
3. Each tail segment may be:
    - a literal name
    - a variable whose value is substituted dynamically
    - a computed expression written as `.( expression )`
4. Computed tail expressions may contain arithmetic, concatenation, function calls, and other stem references.
5. Parentheses used for computed tails must be **balanced and unambiguous**.
6. Bare parentheses immediately following a computed tail (e.g. `stem.(a)(b).x`) are **illegal** and rejected.
7. RXPP evaluates computed tails **before** accessing the stem to ensure correct ordering.
8. Multiple stem expressions in the same line are handled independently and safely.

---

### Do / Don’t quick guide

#### Do

- Use stems to model tables and records
- Use variables or computed expressions for dynamic keys
- Inspect generated output when using complex expressions
- Enable `##CFLAG dotisstem` only when your naming scheme requires it

#### Don’t

- Rely on ambiguous dot patterns
- Use bare parentheses after computed tails
- Assume single-tail names are stems unless `dotisstem` is set
- Hide complex logic inside a single stem expression

---

### Example: before and after expansion (conceptual)

**Source:**

```rexx
Customer.customer.name = "IBM"
```

**After RXPP (conceptual):**

```rexx
src = putstem("Customer." || customer || ".name", "IBM")
```

(The exact generated code may vary, but RXPP always makes stem access explicit.)

---

This section describes user-visible semantics only. Internal rewriting details are intentionally hidden from the user.


## 3.7 FOR convenience

RXPP provides a **C-style `FOR` loop** as a compile-time convenience. This construct is intended to offer a compact and familiar loop form for developers coming from C-like languages, while still compiling down to standard CREXX Rexx control flow.

The `FOR` construct is rewritten entirely at **precompile time** and does not require any special runtime support.

---

### Basic example

```rexx
FOR i=1; i<=10; i=i+1
   say i
END
```

This loop consists of three parts:

1. **Initialization** (`i=1`)
2. **Loop condition** (`i<=10`)
3. **Iteration step** (`i=i+1`)

These parts closely mirror the semantics of a C `for` loop.

---

### Semantics

At compile time, RXPP rewrites the `FOR` construct into an equivalent Rexx `DO FOREVER` loop with explicit initialization, condition checking, and iteration logic.

Conceptually, the above example is transformed into logic similar to:

```rexx
i = 1
do forever
   if i > 10 then leave
   say i
   i = i + 1
end
```

The exact generated code may differ slightly, but the execution semantics are equivalent.

---

### Multiple statements in the loop body

The loop body may contain multiple statements without requiring an explicit `DO / END` block:

```rexx
FOR idx=10; idx>=1; idx=idx-1
   say "countdown:" idx
   total = total + idx
END
```

---

### Notes and guidelines

- All three components of the `FOR` clause are required.
- The loop condition is evaluated **before each iteration**.
- `LEAVE` and `ITERATE` may be used inside the loop body to control execution.
- Nesting of `FOR` loops is supported.

---

### When to use FOR

The `FOR` construct is particularly useful when:

- Loop initialization, condition, and iteration are closely related
- A compact, single-line loop definition improves readability
- Porting logic from C or similar languages

For more complex iteration patterns, native Rexx `DO` loops may still be preferable.

---

As with all RXPP extensions, `FOR` exists only at compile time and is expanded into valid CREXX Rexx code before execution.



### 3.8 SELECT / WHEN convenience

RXPP provides a structured `SELECT / WHEN / OTHERWISE` control-flow construct as a **compile-time convenience**. It closely resembles the traditional Rexx `SELECT / WHEN / OTHERWISE` statement, but is implemented entirely by the RXPP precompiler.

The construct improves readability for multi-branch conditional logic and avoids deeply nested `IF / THEN / ELSE` chains. It is expanded into standard CREXX Rexx code before execution and therefore requires **no special runtime support**.

---

#### Basic example

```rexx
SELECT
   WHEN x = 1 THEN say "one"
   WHEN x = 2 THEN say "two"
   OTHERWISE say "other"
END
```

RXPP rewrites this construct into an equivalent sequence of `IF / THEN / ELSE` statements during precompilation.

---

#### Multiple statements per branch

Each `WHEN` or `OTHERWISE` branch may contain multiple statements using a `DO / END` block:

```rexx
SELECT
   WHEN count = 0 THEN do
      say "empty"
      total = 0
   end
   WHEN count > 0 THEN do
      say "items present"
      total = count * price
   end
   OTHERWISE do
      say "invalid"
      total = -1
   end
END
```

#### Semantics

- Conditions are evaluated **top to bottom**.
- The **first matching `WHEN`** branch is executed.
- If no `WHEN` matches, `OTHERWISE` is executed (if present).
- Execution exits the `SELECT` block automatically after a matching branch.

---

### Notes and guidelines

- Conditions are evaluated **top to bottom**.
- The first matching `WHEN` branch is taken.
- `OTHERWISE` is optional but recommended for clarity.
- Nesting of `SELECT` blocks is supported.

The `SELECT / WHEN` construct is most useful when multiple related conditions must be expressed clearly without deeply nested `IF / ELSE` chains.

---

As with all RXPP extensions, this construct exists only at compile time and is expanded into valid CREXX Rexx code before execution.


---

#### Compile-time nature

As with all RXPP control-flow extensions, `SELECT / WHEN` exists **only at compile time**. The RXPP precompiler expands the construct into valid CREXX Rexx code before execution.

This ensures full compatibility with standard Rexx runtimes while allowing clearer, more expressive source code.


### 3.9 SWITCH / CASE convenience

RXPP provides a **C-style `SWITCH / CASE` control-flow construct** as a compile-time convenience. Unlike `SELECT / WHEN`, which is condition-based, `SWITCH / CASE` is **value-based**: a single controlling expression is evaluated once and then compared against multiple `CASE` values.

As with all RXPP extensions, this construct is rewritten entirely at **precompile time** and translated into standard CREXX Rexx control flow. No special runtime support is required.

---

#### Basic example

```rexx
SWITCH x
   CASE 1 THEN say "one"
   CASE 2 THEN say "two"
   DEFAULT say "other"
END
```

The controlling expression (`x`) is evaluated once. Each `CASE` compares its value against that result.

---

#### Explicit case termination (`LEAVE`)

Unlike C, RXPP does **not** implicitly terminate a `CASE` branch after execution.

If execution reaches the end of a `CASE` branch **without a `LEAVE` statement**, processing will continue and the **next `CASE` condition will also be evaluated**.

To prevent this, each `CASE` branch that should terminate the switch must explicitly use `LEAVE`.

```rexx
SWITCH code
   CASE 1 THEN do
      say "case one"
      LEAVE
   end
   CASE 2 THEN do
      say "case two"
      LEAVE
   end
   DEFAULT do
      say "default"
   end
END
```

This makes control flow explicit and avoids hidden fall-through semantics.
At the same time, it allows multiple CASE labels to intentionally share a single action block, which is useful when different values should be handled in the same way without duplicating code.
```rexx
SWITCH status
  CASE "OPEN"
  CASE "PENDING"
     say "Order is active"
     LEAVE
  CASE "CLOSED"
     say "Order is closed"
     LEAVE
END
```

Here, both "OPEN" and "PENDING" are handled by the same rule through deliberate fall-through, and execution stops explicitly with LEAVE.

As in C, CASE branches do not terminate automatically.
This allows deliberate fall-through, but requires an explicit LEAVE to stop execution at the desired point.

---

#### Multiple statements per CASE

Each `CASE` branch may contain multiple statements:

```rexx
SWITCH status
   CASE "OPEN" THEN do
      say "processing"
      result = 1
      LEAVE
   end
   CASE "CLOSED" THEN do
      say "already closed"
      result = 0
      LEAVE
   end
   DEFAULT do
      say "unknown status"
      result = -1
   end
END
```

---

#### Difference between SELECT / WHEN and SWITCH / CASE

Although similar in appearance, the two constructs serve different purposes:

- **SELECT / WHEN**
   - Each `WHEN` contains an **independent condition**
   - Conditions may differ in structure and complexity
   - Best suited for **general conditional logic**

- **SWITCH / CASE**
   - A **single expression** is evaluated once
   - Each `CASE` compares a value against that expression
   - Requires explicit `LEAVE` to terminate a branch
   - Best suited for **value dispatch** and enumerations

In short:

> Use **SELECT / WHEN** for conditions.
> Use **SWITCH / CASE** for value-based dispatch.

---

#### Notes and guidelines

- `CASE` values are checked **top to bottom**.
- If no `LEAVE` is executed, evaluation continues with the next `CASE`.
- `DEFAULT` is optional but strongly recommended.
- Nesting of `SWITCH` blocks is supported.

---

As with all RXPP extensions, `SWITCH / CASE` exists only at compile time and is expanded into valid CREXX Rexx code before execution.

---


## 4. Common Patterns and Practical Examples

### 4.1 Use macros for clarity

```rexx
##DEFINE LOG(m) { say "[LOG] " || m }
```

### 4.2 Compile-time configuration

```rexx
##IF TRACE
say "Tracing"
##END
```

### 4.3 Structured includes

RXPP provides two include mechanisms, ##INCLUDE and ##USE, which allow large Rexx programs to be split into smaller, manageable source files without introducing any runtime overhead. All includes are resolved at pre-compile time and result in a single, flat Rexx source file.

##INCLUDE inserts the referenced source file exactly at the position where the directive appears. This is useful when the included code must participate directly in the surrounding control flow, for example when inserting executable statements, inline logic, or context-dependent definitions.

##USE, on the other hand, appends the referenced source file to the end of the program, independent of where the directive appears. This is particularly helpful for including procedure or function definitions that do not need to be interleaved with the main execution path.

By using ##USE, include files can be declared early in the source for clarity and documentation purposes, while still ensuring that their contents are placed in a suitable location in the final program. This helps keep the main program structure readable without imposing ordering constraints on the physical layout of the source files.### 4.4 OO syntax as sugar

OO syntax improves readability but is rewritten into plain function calls.

### 4.5 Stem handling

RXPP rewrites complex stem access into explicit helper calls for correctness.

---

## 5. Debugging and Understanding RXPP Output

### 5.1 Inspect generated output

The generated Rexx file is readable and is the primary debugging tool.

### 5.2 Macro tracing

RXPP can preserve macro and directive lines as comments in the output.

### 5.3 Conditional issues

If code disappears, check `##IF` conditions and compile-time variables.

### 5.4 Stem warnings

Warnings appear as comments and indicate ambiguous stem expressions.

---

## 6. Best Practices and Style Guidelines

- Think source-to-source, not runtime magic
- Prefer clarity over compactness
- Keep macros simple and intentional
- Regularly inspect generated output

---

## 7. FAQ and Common Pitfalls

**Why doesn’t my macro expand?**  
Check token boundaries, argument counts, and macro type.

**Why is code missing?**  
Conditional compilation evaluated false.

**Why do I see getstem/putstem?**  
RXPP makes stem access explicit for correctness.

---

## Appendix A: RXPP Directives Reference

### `##DEFINE`
Define macros.

### `##INCLUDE` / `##USE`
Inline file inclusion.

### `##SET` / `##UNSET`
Set compile-time variables.

### `##IF` / `##IFN` / `##ELSE` / `##END`
Conditional compilation.

### `##ARRAY`
Declare typed arrays.

### `##GLOBAL`
Declare globals for namespace exposure.

### `##STEM`
Force stem rewrite.

---
## Appendix B: Additional directives and behaviors (from legacy doc)
This section documents a few directives and behaviors that exist in RXPP but were not fully described in the user-guide text above.
### `##CFLAG` (compiler / preprocessor behavior flags)
`##CFLAG` sets internal behavior flags early, for example controlling whether RXPP keeps directive lines as comments, enables additional debug output, or relaxes stem detection rules.
Example:
```rexx
##CFLAG dotisstem parse iflink
```

Notable flag:

- `dotisstem`: **By default, RXPP requires at least two tail segments to recognize a stem**, else it is interpreted as a CREXX array. Setting `dotisstem` relaxes that rule so a single tail can be treated as a stem.

### `##DATA name` and `##END`

`##DATA` captures all subsequent lines up to the matching `##END` directive and converts them into array assignments of the form:

```
name.1 = ...
name.2 = ...
name.3 = ...
```

All lines between `##DATA` and `##END` are treated as **plain, free-form text**. They are **not parsed, tokenized, or interpreted** in any special way by RXPP. No quoting rules, delimiters, or formatting constraints apply.

This design allows you to write ordinary text exactly as it should appear, without escaping or syntactic decoration. RXPP simply stores each line verbatim into the associated array element.

This makes `##DATA` especially useful for embedding inline data blocks—such as templates, configuration fragments, scripts, messages, or documentation text—directly into the source code without relying on external files. Each line is preserved exactly as written and stored sequentially, which keeps later processing simple and predictable.

---

### `##SYSxxx`

RXPP recognizes a family of `##SYSxxx` directives (for example `##SYS001`) that behave like specialized forms of `##DATA`.

When a `##SYS001` directive is encountered, RXPP captures all following lines up to `##END` and expands them at precompile time into assignments stored in an array named after the directive itself:

```
SYS001.1 = ...
SYS001.2 = ...
```

As with `##DATA`, the captured lines are **free-form text** with no required structure or syntax. The content is taken verbatim and stored line by line.

The difference is primarily semantic: `##SYSxxx` directives are reserved for system-level or predefined data blocks, where the array name is implicitly defined by the directive itself rather than supplied explicitly.

These directives exist purely as **compile-time convenience mechanisms**. They do not introduce new runtime language features. When using them, it is recommended to inspect the generated output to see the exact assignments RXPP produces, especially when integrating with other tools or build steps.


---

## Appendix C: Build / pipeline notes (from legacy doc)

RXPP is commonly used as part of the larger CREXX toolchain. A typical script-driven pipeline is:

```text
macro1.rxpp + maclib.rexx
        │
        ▼
   [Precompile - RXPP]
        │
        ▼
  macro1.rexx (generated)
        │
        ▼
   [Compile - rxc]
        │
        ▼
    macro1.obj
        │
        ▼
   [Assemble - rxas]
        │
        ▼
   [Run - rxvm]
```

Some environments provide helper scripts (Windows `.bat`, Linux `.sh`) such as `rxCREXX`, `rxprecomp`, `rxcompile`, `rxasm`, and `rxrun` to run these phases consistently.

Practical debugging tip: if something goes wrong in the full pipeline, run the scripts step-by-step (precompile first, then compile, etc.) and inspect each intermediate artifact.

---

### Guarantees

- Output is valid CREXX levelb Rexx
- No RXPP directives remain at runtime (except optional comment traces)
- All transformations are explicit and readable in the generated file

