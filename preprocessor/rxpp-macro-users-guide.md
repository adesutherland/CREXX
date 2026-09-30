# RXPP Macro User's Guide

RXPP macros are compile-time facilities. RXPP reads an `.rxpp` source file,
expands its macros, and passes the resulting CREXX source to the compiler.
Macros do not exist at runtime.

For the complete reference, including macro libraries, conditional processing,
sections, diagnostics, and implementation details, see
[`macro-facility.md`](macro-facility.md).

## 1. Choose a macro style

RXPP has two commonly used macro styles:

- `##define` creates a textual replacement macro, useful for short
  expressions or statement fragments.
- `##MACRO` creates a RexxScript-backed macro, useful for calculations, loops,
  conditional generation, and multiple generated lines.

Use the simplest style that expresses the task.

## 2. Textual macros with `##define`

Define a replacement body in braces:

```rexx
##define SQUARE(x) { x * x }

say SQUARE(7)
```

RXPP expands this before compilation, conceptually producing:

```rexx
say 7 * 7
```

Textual macros can also expand to several statements. Separate statements in
a multi-statement body with semicolons:

```rexx
##define SWAP(a,b) { temp = a; a = b; b = temp }

swap(left, right)
```

Textual macros are substitutions, not syntax-tree transformations. Use
distinctive temporary names to reduce collisions with the surrounding source.

## 3. Script macros with `##MACRO`

A script macro has a name, parameters, a RexxScript body, and an `##MEND`:

```rexx
##MACRO HELLO name
    .gen say "Hello &name"
##MEND

##hello "World"
```

The generated CREXX source is:

```rexx
say "Hello World"
```

The statements in the body run while RXPP expands the macro. Assignments,
loops, calculations, and conditionals are macro-time logic; they are not
copied into the generated program unless they use one of the output forms.

```rexx
##MACRO MAKE_TABLE name, count
    name = unquote(name)
    .gen &name = .int[]
    do i = 1 to count
        .gen &name[&i] = &i * &i
    end
##MEND

##make_table squares, 3
```

This produces source equivalent to:

```rexx
squares = .int[]
squares[1] = 1
squares[2] = 4
squares[3] = 9
```

The `squares` array belongs to the generated CREXX program. It is not an
array in the macro's RexxScript state.

## 4. Generating source

Script macros have three output destinations:

```rexx
.gen     source-line
.tail    source-line
.section section-name source-line
```

`.gen` inserts the generated line at the macro-call position:

```rexx
.gen say "created here"
```

`.tail` appends the line to the end of the generated source:

```rexx
.tail say "created at the end"
```

`.section` stores the line in a named section. Place the accumulated section
later with `##EMIT`:

```rexx
##MACRO CHECK name
    name = unquote(name)
    .section checks if &name = '' then say "Missing &name"
##MEND

##check "user"
##check "address"

say "Before checks"
##EMIT checks
say "After checks"
```

Section lines keep the order in which they were added. Their variables are
evaluated when the macro runs, not when `##EMIT` later inserts the section.

## 5. Macro-time variables and generated variables

Macro code runs in RexxScript during preprocessing. Its variables exist only
for macro expansion. The generated text is compiled later as ordinary CREXX.

In a generated-output expression, `&variable` inserts a macro-time value into
the source line:

```rexx
do i = 1 to count
    .gen &name[&i] = &i * &i
end
```

Here `name` and `i` are macro-time values. `&name[&i]` constructs text such as
`squares[2]`; it does not access a macro-time array.

The same rule applies when generating CREXX stems or other variables. A
generated declaration or assignment creates the variable in the resulting
CREXX program, not inside the macro evaluator.

Do not confuse these forms:

- `&variable` inserts a RexxScript/macro-time value into generated output.
- `{variable}` is RXPP preprocessor-variable substitution, normally used with
  directives such as `##SET`.
- A generated CREXX variable is runtime program state and exists only after
  the generated source is compiled and executed.

Macro arguments may retain their call-site quotes. Use `unquote()` when an
argument will become an identifier or when surrounding quotes must not appear
in the generated source:

```rexx
name = unquote(name)
```

Predefined context values can also be inserted into generated output:

| Context value | Meaning |
| --- | --- |
| `&_module` | Module containing the current macro call. |
| `&_file` | Source file containing the current macro call. |
| `&_mcalls` | Number of macro calls available to the current macro context. |
| `&_sysndx` | Current system-wide macro expansion index. |
| `&_macro_calls` | Readable alias for `&_sysndx`, the current macro expansion index. |
| `&_mline` | Source text of the current macro invocation. |
| `&_mlino` | Source line number of the current macro invocation. |

For example:

```rexx
.gen say "expanded in &_module from &_file"
.tail /* call &_mline at line &_mlino */
.gen say "macro call count=&_mcalls, expansion index=&_macro_calls"
```

These values describe the preprocessing event that expanded the macro. They
are not runtime variables in the generated CREXX program unless the generated
source explicitly declares or assigns variables using their values. The
traditional name `&_sysndx` is retained for compatibility; `&_macro_calls`
can be used when a descriptive name is preferred.

## 6. Nested macro calls with `.mcall`

`.mcall` generates RXPP source for another preprocessing pass. It does not
generate final CREXX source directly.

```rexx
##MACRO EMITSCRIPT
    say 'say "scripted"'
    .mcall SQUARE 14
##MEND

##MACRO SQUARE x
    .gen say &x * &x
##MEND

##EMITSCRIPT
```

`.mcall SQUARE 14` emits an RXPP call equivalent to `##SQUARE 14`, which can
then produce `say 14 * 14`. If the emitted text already starts with `##`, that
prefix is preserved.

RXPP permits `.mcall` nesting through 16 levels. A deeper `.mcall` expansion
is rejected with an `RXPP_MCALL_DEPTH` diagnostic. This limit protects against
runaway `.mcall` recursion; it does not attempt to detect recursion created by
other generated-source techniques such as `.gen ##MACRO`.

## 7. Practical advice

- Define a macro before its first use.
- Use `##define` for small textual substitutions and `##MACRO` for generation
  logic.
- Keep macro-time helper names distinctive.
- Use `.gen` for code belonging at the call site, `.tail` for end-of-source
  support code, and `.section` plus `##EMIT` for deferred code.
- Inspect the generated `.crexx` file when debugging a macro.
- Add `##CFLAG format` when you want RXPP to apply its indentation-only
  formatter to the final generated source. Add `4buf` to inspect the formatted
  buffer before it is written; `n4buf` is enabled by default.
- Remember that RXPP output must still be valid CREXX source.
