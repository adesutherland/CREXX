# RXPP Macro Facility

## Overview

RXPP provides a compile-time macro facility for generating CREXX source code.

A macro may:

* generate source directly at the macro call position;
* append generated source to the end of the current source;
* collect generated source in named sections;
* emit a named section at a selected position;
* access information about the macro call and its source location.

The generated result is ordinary CREXX source code. The macro facility does not introduce a runtime dependency.

---

## Practical Examples

The following examples show why a macro is useful. In both cases, the
repetition is handled while RXPP is running; the generated program contains
ordinary CREXX statements only.

### Example 1: Generate a Small Lookup Table

This example does not use sections. The macro creates an integer array and
generates all of its entries from one compact call. The loop is a
compile-time loop, so no loop is required in the generated program just to
build the table.

```rexx
/* lookup_table.rxpp */

options levelb
import rxfnsb

##MACRO MAKE_TABLE name, count, title
    name  = unquote(name)
    title = unquote(title)

    .gen &name = .int[]
    do i = 1 to count
        .gen &name[&i] = &i * &i
    end
    .gen say "&title has &count entries"
    .tail say "Table &name was generated at compile time"
##MEND

##make_table squares, 5, "Squares"

say "square 3 =" squares[3]
say "square 5 =" squares[5]
exit 0
```

The call generates source equivalent to:

```rexx
squares = .int[]
squares[1] = 1
squares[2] = 4
squares[3] = 9
squares[4] = 16
squares[5] = 25
say "Squares has 5 entries"
...
say "Table squares was generated at compile time"
```

This is useful when a table, enumeration, or repetitive declaration is known
from the source parameters. The application describes the table once; the
macro performs the repetition and arithmetic.

### Example 2: Generate Settings and Deferred Validation

This example uses a named section. Each macro call generates the setting at
the call position and contributes its validation statement to the
`validation` section. The validation code is inserted later with `##emit`.

```rexx
/* settings.rxpp */

options levelb
import rxfnsb

##MACRO SETTING name, value, minimum, maximum
    name = unquote(name)

    .gen &name = &value
    .section validation if &name < &minimum | &name > &maximum then say "Invalid setting &name: " &name
##MEND

##setting "retries", 3, 0, 10
##setting "buffer_size", 256, 64, 4096
##setting "timeout", 0, 1, 60

say "Settings have been loaded"

/* All checks are inserted at this point. */
##emit validation

say "Settings have been checked"
exit 0
```

Conceptually, the generated source contains the definitions where the macro
calls occurred:

```rexx
retries = 3
buffer_size = 256
timeout = 0
```

At `##emit validation`, RXPP inserts the accumulated checks:

```rexx
if retries < 0 | retries > 10 then say "Invalid setting retries: " retries
if buffer_size < 64 | buffer_size > 4096 then say "Invalid setting buffer_size: " buffer_size
if timeout < 1 | timeout > 60 then say "Invalid setting timeout: " timeout
```

The third setting deliberately fails its range check. This makes the example
useful without requiring a large framework: one declaration describes the
value and its validation rule, while the section lets the program choose
where all validation runs.

Use a section when generated code belongs somewhere other than the macro call
position. Use `.gen` alone when the generated statements belong exactly where
the macro is invoked.

---

## Basic Macro Structure

A macro is defined with `##MACRO` and terminated with `##MEND`.

```rexx
##MACRO macro-name parameter-list
    macro statements
##MEND
```

The macro is invoked with a corresponding `##` directive:

```rexx
##macro-name arguments
```

Example:

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

### External script-macro packages

RXPP discovers `.rxpm` package names in the selected macro-library directory
first, then in the system path. An autoloaded package is named for its directive,
for example
`widget.rxpm` for `##WIDGET`, and its first record must be the `##MACRO`
header. A package containing one or more differently named macros can instead
be brought into the source with `##INCLUDE package.rxpm`.

When the same package name exists in both locations, the copy beside the
selected `maclib` wins. This precedence applies both to discovery and to lazy
loading, so `maclist` and expansion select the same file.

An RXPP source file can register another macro directory with:

```text
##loadMacro path-to-macros
```

The directive scans that directory for `.rxpm` names during the initial
preprocessor scan, before macro expansion. Package contents are still loaded
only when their directive is used for the first time. Each later
`##loadMacro` overrides an earlier registration of the same name. The effective
precedence is `maclib`, then `syspath`, then `##loadMacro` directives in source
order, with the last matching load winning.

Relative `##INCLUDE` and `##USE` paths are resolved against that macro-library
directory. Scripted directives use complete-name matching: `##UI` and
`##UI_NODE` can coexist without the shorter name capturing the longer one.

---

## RexxScript Statements Inside a Macro

The body of an RXPP macro may contain RexxScript statements.

RexxScript statements are executed while the macro is being expanded. They can perform calculations, process parameters, control loops, and prepare values used by `.gen`, `.tail`, and `.section` statements.

The RexxScript statements themselves are not copied into the generated CREXX source unless they explicitly generate source.

### Factorial Macro

The following macro calculates a factorial during macro expansion and generates the corresponding CREXX statements:

```rexx
##MACRO factorial name, upto
    .gen &name=1

    do i=2 to upto
        .gen &name=&name*&i
    end

    _mline=unquote(_mline)

    .gen say "factorial macro calls=&_mcalls"
    .gen say "Total macro calls (_sysndx) at factorial=&_sysndx"
    .gen say "factorial of &i is &name"
    .gen say "factorial call is &_mline"
    .gen say "factorial call at &_mlino"
##MEND
```

The statements:

```rexx
do i=2 to upto
    ...
end
```

are RexxScript control statements executed by the macro processor.

For example, the macro call:

```rexx
##factorial result,5
```

causes the loop to execute for the values `2` through `5`.

Each loop iteration executes this macro-generation statement:

```rexx
.gen &name=&name*&i
```

The generated CREXX source is conceptually:

```rexx
result=1
result=result*2
result=result*3
result=result*4
result=result*5
```

The RexxScript assignment:

```rexx
_mline=unquote(_mline)
```

removes the quotes from the stored macro invocation text before it is used in generated output.

The remaining `.gen` statements generate diagnostic information:

```rexx
say "factorial macro calls=<macro-call-count>"
say "Total macro calls (_sysndx) at factorial=<system-index>"
say "factorial of <final-index> is result"
say "factorial call is <original-macro-call>"
say "factorial call at <source-line-number>"
```

Values shown between angle brackets depend on the macro invocation and the current RXPP processing state.

### Compile-Time and Generated Statements

The macro contains two different kinds of statements.

RexxScript statements are executed during preprocessing:

```rexx
do i=2 to upto
    ...
end

_mline=unquote(_mline)
```

Macro output statements generate CREXX source:

```rexx
.gen &name=1
.gen &name=&name*&i
.gen say "factorial macro calls=&_mcalls"
```

This distinction allows a macro to use normal RexxScript logic to decide what source code should be generated.

RexxScript may therefore be used inside a macro for:

* loops;
* assignments;
* string processing;
* parameter processing;
* calculations;
* conditional generation;
* preparing text for `.gen`, `.tail`, or `.section`.

The RexxScript logic runs at preprocessing time. Only the source produced by the macro output statements is passed to the CREXX compiler.



---

# Output Destinations

A macro can generate source into three different destinations.

## `.gen`

The `.gen` statement generates a source line at the position of the macro call.

```rexx
.gen say "Generated at the macro call"
```

For example:

```rexx
say "Before"

##example

say "After"
```

If `example` generates one `.gen` line, the result is:

```rexx
say "Before"

say "Generated at the macro call"

say "After"
```

Use `.gen` for declarations and statements that belong directly at the macro invocation point.

---

## `.tail`

The `.tail` statement appends a generated source line to the end of the generated CREXX source.

```rexx
.tail say "Generated at the end"
```

This is useful when a macro call must also generate:

* a procedure;
* support code;
* diagnostic output;
* initialization or cleanup code that belongs at the end of the source.

Multiple `.tail` statements retain their generation order.

---

## `.section`

The `.section` statement appends a generated source line to a named section.

```rexx
.section section-name generated-source-line
```

Example:

```rexx
.section validation say "Perform generated validation"
```

The line is not generated immediately. It remains in the named section until an `##EMIT` directive is encountered.

Multiple macro calls may contribute lines to the same section.

```rexx
.section validation say "Check field A"
.section validation say "Check field B"
.section validation say "Check field C"
```

The section preserves the order in which its lines were added.

---

## `##EMIT`

The `##EMIT` directive inserts the accumulated contents of a named section at the current source position.

```rexx
##EMIT section-name
```

Example:

```rexx
say "Before validation"

##EMIT validation

say "After validation"
```

If the `validation` section contains:

```rexx
say "Check field A"
say "Check field B"
```

the generated source becomes:

```rexx
say "Before validation"

say "Check field A"
say "Check field B"

say "After validation"
```

The emitted lines are subsequently processed as normal RXPP input.

This allows emitted section lines to contain:

* macro substitutions;
* macro calls;
* RXPP directives;
* further generated source.

---

## Emitting Sections from Within a Macro

Normally, a section is emitted explicitly in the source:

```rexx
##EMIT validation
```

RXPP also allows a macro to generate an `##EMIT` directive itself.

This makes it possible for a macro to define a complete code-generation workflow, including the placement of generated sections.

For example:

```rexx
##MACRO EXAMPLE

    .section methods hello: method
    .section methods     say "Hello"
    .section methods     return

    .gen ##EMIT methods

##MEND
```

The generated `##EMIT` directive is subsequently processed by RXPP and inserts the accumulated section contents into the generated source.

This capability makes it possible to build higher-level abstractions in which a macro generates not only individual source lines, but complete source structures assembled from multiple sections.

# Why Sections Are Useful

A single macro call may need to generate related source in several different locations.

For example, one macro call may:

1. define a value at the macro call position;
2. generate a checking statement in a validation section;
3. append diagnostic information to the end of the source.

This is similar to an assembler macro that contributes a field to a `DSECT` while also generating initialization or checking instructions in another control section.

Without named sections, the user would need to maintain the declaration and its checking code separately.

With named sections, one macro invocation describes the item once and generates all associated source fragments automatically.

---

# Complete ENUM Example

The following example demonstrates the new macro facility.

```rexx
/* RXPP macro facility example */

options levelb
import rxfnsb

##cflags def nset niflink n1buf n2buf n3buf nvars nmaclist includes nosrcmap nmaclog

##MACRO ENUM upto,const,cmt
    .gen say "caller &_module &_file"

    cmt   = unquote(cmt)
    const = unquote(const)
    w2    = subword(cmt, 2)

    .gen say "w2 is &w2"
    .gen say "Enumerate &const up to &upto"
    .gen &const = .int[]

    do i = 1 to upto
        .gen &const[&i] = &i
        .gen say "gen from &const=&i &cmt"

        .tail say "tail from &const=&i"

        .section 1234 say "emitted section 1 &cmt"
    end

    .gen say "enum macro calls=&_mcalls"
    .gen say "Total macro calls (_sysndx) at enum=&_sysndx"

    .tail /* enum call is &_mline */
    .tail say "enum call at &_mlino"
##MEND

##enum 2,"fifth","this is a comment from a macro PEJ"

say "**** this is the last line of the REXX ****"

##emit 1234
```

---

# ENUM Macro Parameters

The `ENUM` macro accepts three parameters:

| Parameter | Purpose                                  |
| --------- | ---------------------------------------- |
| `upto`    | Number of array entries to generate      |
| `const`   | Name of the generated integer array      |
| `cmt`     | Text used in generated diagnostic output |

The macro call is:

```rexx
##enum 2,"fifth","this is a comment from a macro PEJ"
```

After removing the quotes, the effective parameter values are:

```text
upto  = 2
const = fifth
cmt   = this is a comment from a macro PEJ
```

---

# Compile-Time Processing

The following statements are executed while RXPP processes the macro:

```rexx
cmt   = unquote(cmt)
const = unquote(const)
w2    = subword(cmt, 2)
```

They do not become part of the generated CREXX source.

`unquote()` removes the surrounding quotes from the macro arguments.

The statement:

```rexx
w2 = subword(cmt, 2)
```

extracts the comment beginning with its second word.

For the supplied comment, `w2` becomes:

```text
is a comment from a macro PEJ
```

---

# Source Generated with `.gen`

The following macro statements generate source at the location of the `##enum` call:

```rexx
.gen say "caller &_module &_file"
.gen say "w2 is &w2"
.gen say "Enumerate &const up to &upto"
.gen &const = .int[]
```

For the example call, the array declaration becomes:

```rexx
fifth = .int[]
```

The loop generates two array assignments:

```rexx
fifth[1] = 1
fifth[2] = 2
```

It also generates a diagnostic statement for each entry.

Conceptually, the generated source at the invocation point is:

```rexx
say "caller <module> <file>"
say "w2 is is a comment from a macro PEJ"
say "Enumerate fifth up to 2"

fifth = .int[]

fifth[1] = 1
say "gen from fifth=1 this is a comment from a macro PEJ"

fifth[2] = 2
say "gen from fifth=2 this is a comment from a macro PEJ"

say "enum macro calls=<macro-call-count>"
say "Total macro calls (_sysndx) at enum=<system-index>"
```

Values shown between angle brackets depend on the source file and the current RXPP processing state.

---

# Source Generated with `.tail`

Each loop iteration contributes one line to the source tail:

```rexx
.tail say "tail from &const=&i"
```

For the example, this produces:

```rexx
say "tail from fifth=1"
say "tail from fifth=2"
```

The following lines are also appended to the tail:

```rexx
.tail /* enum call is &_mline */
.tail say "enum call at &_mlino"
```

They record the original macro call and its source line number.

The `.tail` source is generated after the normal source and emitted sections, according to the macro facility's tail-processing rules.

---

# Source Generated with `.section`

Inside the loop, the macro adds a line to section `1234`:

```rexx
.section 1234 say "emitted section 1 &cmt"
```

Because the loop executes twice, section `1234` receives two lines:

```rexx
say "emitted section 1 this is a comment from a macro PEJ"
say "emitted section 1 this is a comment from a macro PEJ"
```

These lines are not placed at the macro invocation point.

They are retained until the following directive is processed:

```rexx
##emit 1234
```

The section is therefore emitted after:

```rexx
say "**** this is the last line of the REXX ****"
```

The generated result is conceptually:

```rexx
say "**** this is the last line of the REXX ****"

say "emitted section 1 this is a comment from a macro PEJ"
say "emitted section 1 this is a comment from a macro PEJ"
```

This demonstrates the main benefit of sections: a macro invoked earlier in the source can contribute code to a location selected later by the application.

---

# Macro Context Variables

The example uses several predefined macro context variables.

## `&_module`

Identifies the module containing the macro call.

```rexx
.gen say "caller &_module &_file"
```

## `&_file`

Identifies the source file containing the macro call.

```rexx
.gen say "caller &_module &_file"
```

## `&_mcalls`

Contains the macro call count available to the current macro.

```rexx
.gen say "enum macro calls=&_mcalls"
```

## `&_sysndx`

Contains the current system macro expansion index.

```rexx
.gen say "Total macro calls (_sysndx) at enum=&_sysndx"
```

## `&_mline`

Contains the source text of the macro invocation.

```rexx
.tail /* enum call is &_mline */
```

## `&_mlino`

Contains the source line number of the macro invocation.

```rexx
.tail say "enum call at &_mlino"
```

---

# Simplified Section Example

The essential section mechanism can be demonstrated without the additional diagnostic variables:

```rexx
/* Simple RXPP section example */

options levelb
import rxfnsb

##MACRO VALUE name,number
    name   = unquote(name)
    number = unquote(number)

    .gen &name = &number
    .section checks if &name < 0 then say "Invalid value: &name"
##MEND

##value "minimum",10
##value "maximum",100

say "Values have been defined"

/* Insert all generated checks here. */
##emit checks

exit 0
```

The macro calls generate the values at their invocation positions:

```rexx
minimum = 10
maximum = 100
```

At the `##emit checks` position, RXPP inserts:

```rexx
if minimum < 0 then say "Invalid value: minimum"
if maximum < 0 then say "Invalid value: maximum"
```

One macro call therefore creates both:

* the value definition;
* the related checking code.

This is the principal use case for named sections.

---

# Summary

The new RXPP macro output statements have distinct purposes:

| Statement              | Destination                 |
| ---------------------- | --------------------------- |
| `.gen source`          | Macro invocation position   |
| `.tail source`         | End of the generated source |
| `.section name source` | Named deferred section      |
| `##EMIT name`          | Current source position     |

The section facility allows a macro to generate related source fragments for different parts of a program while keeping the complete definition in one macro invocation.

# Advanced Features

- Nested macro calls (`##MCALL`)
- Named source blocks (`##BEGIN`, `##COPYBLOCK`)
- Source maps (`srcmap`)
- Task-aware code generation (`CHANNELFIELD`, `CHANNELCLASS`)
