# RXPP User Documentation

## 1. Introduction

RXPP is a **source-to-source pre-compiler** for CREXX (levelb) Rexx programs. It extends standard Rexx with compile-time features such as macros, conditional compilation, structured includes, data-block generation, and syntactic conveniences. RXPP runs *before* execution and produces plain, valid CREXX Rexx output with no runtime dependency on RXPP itself.

RXPP is intended for developers who want to structure larger Rexx programs cleanly while keeping the runtime environment simple and transparent.

For a focused introduction to creating and using RXPP macros, see the
[RXPP Macro User's Guide](rxpp-macro-users-guide.md). This document remains
the broader guide to RXPP preprocessing features.

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

#### External script macros

An RXPP source file can register script-macro packages from an additional
directory:

```rexx
##loadMacro path-to-macros
```

RXPP scans the directory for `.rxpm` filenames during its early preprocessing
pass, before any macro expansion takes place. Only the names are registered at
that point. The package body is loaded lazily the first time its directive is
used.

The selected `maclib` directory is searched first, followed by `syspath`, then
each `##loadMacro` directory in source order. If the same macro name is found
again, the later registration replaces the earlier one; therefore the last
matching `##loadMacro` wins. The directive is consumed by RXPP and does not
remain in generated CREXX.

With verbose output enabled, `CRX0170I` shows registration or replacement and
`CRX0171I` shows the file selected for lazy loading.

RXPP accepts relative paths for `##INCLUDE`, `##USE`, and `##loadMacro`.
They are resolved against the configured RXPP macro/system path and normalized
across `/` and `\\` separators, including `.` and `..` components.

For `.rxpp` inputs, `##buildDir path` tells the `crexx` driver where to place
the generated `.crexx`, `.rxas`, and `.rxbin` artifacts. The path is relative
to the driver’s current working directory when it is not absolute. The
driver’s working directory is unchanged. Place the directive within the first
64 physical source lines; comments and blank lines count toward this limit.

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

####  3.1.9 Keyword parameters

Macro definitions may declare keyword parameters after all positional
parameters. A keyword parameter is written as `name=default`; the default may
be empty.

```rexx
##define resize(width, height, quality=90, format='jpg') { \
   w = width; h = height; q = quality; f = format \
}
```

Calls may supply keywords as `name=value` after the positional arguments:

```rexx
resize(800, 600, quality=75, format='png')
```

Current RXPP also accepts TSO-style keyword calls such as `quality(75)`. The
definition still uses `quality=default`.

---

####  3.1.10 Command-style macros

Command-style macros are defined by using `cmd` or `command` as the first
pseudo-parameter. They are invoked as statement-level commands with
blank-separated arguments rather than function-style parentheses.

```rexx
##define cmd factorial(var num) { \
   var = 1; do i = 2 to num; var = var * i; end \
}

factorial result 10
say result
```

Command-style macros use the same textual substitution model as ordinary
macros. They are intended for statement-like expansions, not for use inside an
expression.

---

####  3.1.11 Notes and guidelines

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

### 3.5 OO syntax

This topic is included because older RXPP code sometimes used preprocessing
to simulate object-oriented operations or generate procedural equivalents.
That was a source-generation convention, not an RXPP runtime feature.

OO syntax is part of standard CREXX. RXPP passes object and method
expressions through to the compiler; it does not translate them into
procedural calls.


## 3.6 Stems and arrays

Stems are mentioned for the same historical reason: older RXPP-based code
could use generated source to approximate stem-like operations. That approach
must not be confused with the language implementation itself.

Stems are ordinary CREXX language values. RXPP passes stem expressions through
to the compiler; it does not reinterpret dotted names, evaluate computed tails,
or rewrite assignments into `getstem`/`putstem` calls.

Use the normal CREXX stem syntax and `.stem()` constructor described in the
language reference. RXPP directives such as `##DATA` may generate arrays or
stem data, but that is data-block generation rather than stem emulation.

---

### 4.3 Structured includes

RXPP provides two include mechanisms, ##INCLUDE and ##USE, which allow large Rexx programs to be split into smaller, manageable source files without introducing any runtime overhead. All includes are resolved at pre-compile time and result in a single, flat Rexx source file.

##INCLUDE inserts the referenced source file exactly at the position where the directive appears. This is useful when the included code must participate directly in the surrounding control flow, for example when inserting executable statements, inline logic, or context-dependent definitions.

##USE, on the other hand, appends the referenced source file to the end of the program, independent of where the directive appears. This is particularly helpful for including procedure or function definitions that do not need to be interleaved with the main execution path.

By using ##USE, include files can be declared early in the source for clarity and documentation purposes, while still ensuring that their contents are placed in a suitable location in the final program. This helps keep the main program structure readable without imposing ordering constraints on the physical layout of the source files.

## 5. Debugging and Understanding RXPP Output

### 5.1 Inspect generated output

The generated Rexx file is readable and is the primary debugging tool.

### 5.2 Macro tracing

RXPP can preserve macro and directive lines as comments in the output.

### 5.3 Conditional issues

If code disappears, check `##IF` conditions and compile-time variables.

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
`getstem` and `putstem` are runtime/plugin APIs, not code generated by RXPP.

---

## Appendix A: RXPP Directives Reference

### `##DEFINE`
Define macros.

### `##INCLUDE` / `##USE`
Inline file inclusion.

### `##loadMacro`
Register `.rxpm` script-macro names from an additional directory. Registration
happens before expansion; package contents are loaded lazily on first use.
Later registrations override earlier registrations of the same name.

### `##buildDir`
Route generated `.crexx`, `.rxas`, and `.rxbin` artifacts to the specified
directory when the source is compiled through `crexx`.

### `##EXTERNAL`

Adds an existing RXBIN module to the final build stage.

```rexx
##EXTERNAL ../lib/CallCatalog.rxbin
```

For RXBINs in the system binary directory, use the case-insensitive `&syslib/` prefix:

```rexx
##EXTERNAL &syslib/library.rxbin
##EXTERNAL &syslib/classlib.rxbin
##EXTERNAL &syslib/rxfnsg.rxbin
```

RXPP resolves `&syslib/` using the system path returned by `env.LoadPath()` and
writes the resulting full path into the `.inc` manifest. The compiler driver
uses that manifest path unchanged, so no corresponding `crexx.crexx` setting
is required. Other external paths are resolved relative to the `.rxpp` source
file.

The current `.rxpp` source is still preprocessed, compiled, and assembled
normally.

Without `##LINK`, the generated RXBIN and all declared external modules are
passed to `rxvme`:

```text
rxvme generated.rxbin external1.rxbin external2.rxbin ...
```

Multiple `##EXTERNAL` directives may be used when a program depends on more
than one external RXBIN module.

### `##LINK`

Requests creation of a linked RXBIN instead of executing the generated module.

```rexx
##LINK MyApplication
```

The source still passes through the normal RXPP, compiler, and assembler
stages. The generated RXBIN is then linked together with all modules declared
using `##EXTERNAL`:

```text
source.rxpp
  -> RXPP
  -> RXC
  -> RXAS
  -> generated.rxbin
  -> RXLINK generated.rxbin + external modules
  -> MyApplication.rxbin
```

`##LINK` accepts a bare member name, without a path or `.rxbin` suffix.

When combined with `##BUILDDIR`, the build directory determines where the
linked member is written:

```rexx
##BUILDDIR ../build
##EXTERNAL ../lib/CallCatalog.rxbin
##LINK MyApplication
```

This produces:

```text
../build/MyApplication.rxbin
```

Without `##LINK`, the equivalent build ends by running the generated RXBIN with
the external modules instead of producing a linked member.


### `##SET` / `##UNSET`
Set compile-time variables.

### `##INFO` / `##WARN` / `##ERROR`
Emit compile-time diagnostics. INFO and WARN continue preprocessing; ERROR
stops preprocessing with status 8. The directives are not copied to generated
source.

```rexx
##INFO "Using compatibility implementation"
##WARN "This configuration is deprecated"
##ERROR "This configuration is unsupported"
```

The corresponding output prefixes are `++RXPP-INFO`, `++RXPP-WARN`, and
`++RXPP-ERROR`. Failed `##ASSERT` directives use
`++RXPP-DIAG RXPP_ASSERT_FAILED`.

RXPP also supplies these built-in compile-time variables. Use braces to
expand them in source, macros, or `##DATA` content:

| Variable | Meaning |
| --- | --- |
| `{date}` | Compilation date |
| `{time}` | Compilation time |
| `{syslib}` | RXPP system binary/library path |
| `{syspath}` | RXPP system or macro search path |
| `{macpath}` | Directory containing the selected macro library |
| `{inpath}` | Directory containing the input source |
| `{buildpath}` | Directory containing the generated build output |
| `{sourcefile}` | Normalized path of the input source file |
| `{outputfile}` | Normalized path of the generated output file |
| `{platform}` | Host platform reported by the RXPP runtime |
| `{rxpp_version}` | CREXX version reported by the RXPP runtime |
| `{rxpp_rexx}` | Input module/file name (legacy compatibility) |
| `{rxpp_date}` | Combined compilation date and time (legacy compatibility) |

```rexx
say 'Build {date} {time}'
say 'Libraries: {syslib}'
say 'Source: {inpath}'
say 'Version: {rxpp_version} on {platform}'
```

Expansion happens during preprocessing, and the variable names are
case-insensitive. `{syslib}` is the value form of the `&syslib/` prefix used
in `##EXTERNAL` declarations. `cflags` and `printgen` are also initialized
control variables; see the `##CFLAG` and `PRINTGEN` sections. Additional
user-defined variables can be declared with `##SET`.

Ordinary comments are not expanded. For an opt-in metadata comment, place
`/* RXPP:EXPAND */` at the beginning of the line:

```rexx
/* RXPP:EXPAND */ -- Created {date} at {time}; RXPP {rxpp_version}
/* RXPP:EXPAND */ -- Source {sourcefile}; system library {syslib}
```

RXPP removes the marker and expands the rest of the line. Use `--` for the
remaining comment text; a remainder beginning with `/*` remains protected.

### `##IF` / `##IFN` / `##ELSE` / `##END`
Conditional compilation.

### `##ARRAY`
Declare typed arrays.

### `##GLOBAL`
Declare globals for namespace exposure.

---
## Appendix B: Additional directives and behaviors (from legacy doc)
## Appendix B: Additional directives and behaviors (from legacy doc)
This section documents a few directives and behaviors that exist in RXPP but were not fully described in the user-guide text above.
### `##CFLAG` (compiler / preprocessor behavior flags)
`##CFLAG` sets RXPP behavior flags during the earliest configuration pass.
It must appear at the beginning of the source, before other `##` directives.

`##CFLAG` is the compile-time control surface for a pre-compilation. Its
flags fall into three broad groups:

- **Preprocessing behavior:** flags such as `format`, `blockmacros`,
  `nosrcmap`, and `cmtblk` affect how RXPP interprets or emits source.
- **Buffer and state inspection:** flags such as `1buf`, `2buf`, `3buf`,
  `4buf`, `vars`, `maclist`, and `includes` show intermediate output or
  preprocessing state.
- **Diagnostics and logging:** flags such as `macrotrace`, `iftrace`,
  `includetrace`, `timing`, `maclog`, and `strictmacrocheck` report what RXPP
  did while compiling.

The inspection and diagnostic flags affect RXPP's pre-compilation output only;
they do not add runtime tracing or logging to the generated CREXX program.
Flags in the first group may change the generated source, but RXPP remains a
source-to-source tool and does not provide runtime implementations of the
features it configures.

When no `##CFLAG` is present, RXPP starts with:

```text
ndef nset svars siflink n1buf n2buf n3buf n4buf nvars nparse
strictmacrocheck nmaclog nblockmacros
```

An explicit `##CFLAG` replaces this complete set; it does not merge with the
defaults. Flags are exact words, and there is no general `n`-prefix rule.
The complete current flag table is maintained in the
[cREXX language reference](../docs/books/crexx_language_reference/rxpp.md).

The most commonly used flags are:

- `ndef`, `nset`: suppress generated directive comments.
- `1buf`, `2buf`, `3buf`, `4buf`: display successive RXPP buffers.
- `format`: apply the indentation-only output formatter.
- `vars`, `maclist`, `includes`: display preprocessing state.
- `nosrcmap`: disable source-map emission.
- `maclog`, `macrotrace`, `iftrace`, `includetrace`, `timing`: enable
  diagnostics.
- `flow`: emit structured `RXPP-FLOW` records for the major preprocessing
  decisions and pass boundaries.
- `stats`: emit a compact `RXPP-STATS` summary at the end of preprocessing.
- `strictmacrocheck`: warn about macro argument mismatches; enabled by
  default.
- `blockmacros`: expand macros while collecting free-form blocks such as
  `##DATA` and `##DALIAS`; disabled by default.
- `nblockmacros`: explicitly keep free-form block contents literal.
- `parse`, `trimparse`, `cmtblk`: enable the corresponding specialized
  preprocessing behavior.

`flow` and `stats` are diagnostic-only. Their output is written to RXPP's
diagnostic stream and is never inserted into the generated CREXX source.

### `##DATA name [callback]` and `##END`

`##DATA` captures all subsequent lines up to the matching `##END` directive and converts them into a cREXX string array.

### `##ASSERT condition, message`

Use `##ASSERT` to validate RXPP configuration before generated CREXX is
compiled. A bare variable checks existence; `==` and `!=` compare a variable's
stored string value exactly:

```rexx
##ASSERT DEBUG, "DEBUG must be defined"
##ASSERT platform == "windows 64", "This module requires Windows"
```

Assertions are evaluated during preprocessing and are not emitted. RXPP
initializes built-in variables such as `platform`, `sourcefile`, `outputfile`,
and `rxpp_version`; project-specific switches should be created with `##SET`.

### `REXXSCRIPT RUN` and `END REXXSCRIPT`

RXPP also accepts a multiline RexxScript body without requiring every line to
be manually quoted and concatenated:

```rexx
REXXSCRIPT RUN
  say 'hello'
  total = 1 + 2
  say total
END REXXSCRIPT
```

RXPP packages the body as one script string and emits the ordinary
`REXXSCRIPT` statement. Existing clauses may remain on the opening line, for
example `REXXSCRIPT RUN EXPOSE total OUTPUT output RESULT raw`. The block is a
source convenience only; RexxScript execution and its existing clause
semantics are unchanged.

### `FORTRAN RUN` and `END FORTRAN`

The FORTRAN interpreter can use a plain free-form text block without RXPP's
`##DATA` syntax:

```rexx
FORTRAN RUN
       PROGRAM DEMO
       END
END FORTRAN
```

RXPP stores the lines between the markers in the ordered `fortran_source`
array. The block contents are preserved as text, including fixed-format
spacing, labels, and comments. `END FORTRAN` is consumed by RXPP and does not
appear in the generated cREXX source; it emits
`call fortran_run fortran_source` after the source array.

Additional source units can be collected with a procedures block:

```rexx
FORTRAN PROCEDURES
       SUBROUTINE SCALE(A,N)
       ...
       END
END FORTRAN
```

`FORTRAN RUN` and `FORTRAN PROCEDURES` blocks are appended to the same
`fortran_source` array. RXPP emits one deferred `call fortran_run
fortran_source` after all blocks, allowing the procedure block to appear
before or after the main program. Procedure execution and `CALL` dispatch are
runtime features planned separately.

Inside either block, `C$RX BIND target = host` imports a typed cREXX array from
the surrounding REXX program. `host[0]` is the read-only typed-array count and
`host[1]` through `host[host[0]]` are copied into the declared FORTRAN target
array. This uses bracket-indexed typed arrays and does not treat a stem's
`.0` value as the count.

Use `C$RX EXPORT target = host` to copy FORTRAN arrays back to typed cREXX
arrays after the run. Multiple export directives are supported; the host
arrays receive values at `[1..count]`.

For example:

```rexx
##DATA names
Alice
Bob
Carol
##END
```

generates, in effect:

```rexx
names.1 = 'Alice'
names.2 = 'Bob'
names.3 = 'Carol'
names.0 = 3
```

All lines between `##DATA` and `##END` are treated as **plain, free-form text**. They are not parsed, tokenized, or interpreted specially by `##DATA`. Each line is collected in order and stored as an element of the generated array.

Macro calls or RXPP variables contained in the captured text may still be processed by later preprocessing stages.

#### Optional callback

A callback may optionally be specified after the array name:

```rexx
##DATA person prolog..PrologSourceAppendArray
  person(alice).
  person(bob).
##END
```

After the complete array has been generated, RXPP emits a normal cREXX call with the array as its argument. The example above therefore becomes, in effect:

```rexx
person.1 = 'person(alice).'
person.2 = 'person(bob).'
person.0 = 2

call prolog..PrologSourceAppendArray person
```

The callback is invoked **once after the complete block has been converted**, not once for each line.

The callback is optional. A normal:

```rexx
##DATA person
...
##END
```

continues to generate only the array.

An optional descriptive keyword may also be placed before the callback name:

```text
##DATA name keyword callback
```

The keyword does not change the callback semantics; it provides a more descriptive form of the declaration.

#### `##RELATION`, `##PROGRAM`, `##LIBRARY` and `##RULE`

`##RELATION`, `##PROGRAM`, `##LIBRARY` and `##RULE` are aliases for `##DATA`. They use the same block collection, array generation, and optional callback mechanism. The directive names have no semantic effect and are alternative spellings only.

#### `##DALIAS alias1 [, alias2 ...]`

`##DALIAS` defines application-specific directive names that use the `##DATA` block syntax and semantics. Alias names are registered for subsequent input, are matched without regard to case, and may be separated by commas or blanks.

For example, an application that processes configuration entries could define a more descriptive directive name:

```rexx
##DALIAS CONFIG

##CONFIG settings process_settings
host localhost
port 8080
mode development
##END
```

Here `##CONFIG` behaves exactly like `##DATA`: it generates the `settings.` stem from the block contents and invokes:

```rexx
process_settings settings
```

The alias declaration must appear before the aliased directive.

`##DALIAS` aliases the **directive name**. It does not define an alias for the generated stem or its contents.

##### Planned bare-block aliases

The current `##DALIAS` form requires the `##` prefix when the alias is used.
RXPP is planned to support an opt-in bare-block form with an alias-specific
terminator:

```rexx
##DALIAS FORTH

forth trace_input
  3 4 + .
  bye
end Forth
```

Only aliases previously declared by `##DALIAS` would be recognized as bare
block starters. Alias matching would be case-insensitive, so `forth`, `Forth`,
and `FORTH` would refer to the same registered block. `END FORTH` would
terminate the block, while existing `##FORTH ... ##END` syntax would remain
valid. A plain `END` would not be used, avoiding ambiguity with ordinary REXX
source.

Aliases are validated when `##DALIAS` is read. They must be ordinary RXPP
identifiers beginning with a letter or underscore and continuing with letters,
digits, or underscores. Empty names, malformed names, duplicates, RXPP
directive names, and reserved REXX words such as `DO`, `IF`, and `END` are
rejected immediately.

This is a planned extension and is not implemented in the current RXPP
preprocessor.

For example, a program can be described as:

```rexx
##PROGRAM prolog_sample
prolog_sample.rxpp
##END
```

and a library as:

```rexx
##LIBRARY prolog
prolog.crexx
crexxcallback.crexx
##END
```

A callback can be attached in exactly the same way:

```rexx
##LIBRARY prolog processLibrary
prolog.crexx
crexxcallback.crexx
##END
```

`##RELATION relation-name [keyword] [callback]` is simply an alternative spelling of the corresponding `##DATA` form.

After the `prolog` array has been generated, RXPP emits, in effect:

```rexx
call processLibrary prolog
```

At this level, `##PROGRAM`, `##LIBRARY` and `##RULE` do not themselves compile, link, execute, or otherwise interpret their contents. They collect the block into an array and, when specified, pass that completed array to a callback. The callback determines what further processing is performed.

This mechanism is useful for embedding not only ordinary data, but also configuration fragments, scripts, templates, messages, Prolog facts and rules, program member lists, library member lists, or other structured text directly in an RXPP source file.
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
