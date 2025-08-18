# RXPP Macro System: How It Works


## RXPP + CREXX Build System Documentation


This document combines the functionality of the RXPP macro preprocessor and the full CREXX script processing pipeline, including both Windows batch and Linux shell versions.

---

## 💼 Table of Contents

* [RXPP + CREXX Build System Documentation](#rxpp--crexx-build-system-documentation)
* [💼 Table of Contents](#-table-of-contents)
  * [💼 Overview](#-overview)
* [🔧 What is RXPP?](#-what-is-rxpp)
* [💪 What RXPP Macros Do](#-what-rxpp-macros-do)
* [✅ Macro Definition](#-macro-definition)
* [🔤 Parameter Replacement Rules](#-parameter-replacement-rules)
* [🆕 Keyword Parameters in Macros](#-keyword-parameters-in-macros)
* [🤖 Macro Invocation](#-macro-invocation)
* [📥 Macro Inclusion](#-macro-inclusion)
* [📊 How It Works Internally](#-how-it-works-internally)
  * [1. **Registration**](#1-registration)
  * [2. **Detection**](#2-detection)
  * [3. **Substitution**](#3-substitution)
  * [4. **Variadic Macros**](#4-variadic-macros)
  * [5. **Emission**](#5-emission)
* [✨ Example](#-example)
  * [Input:](#input)
  * [Output:](#output)
* [🚀 Benefits](#-benefits)
* [📊 Common Use Cases](#-common-use-cases)
* [🧪 Invocation Syntax](#-invocation-syntax)
* [📚 Sample Macros](#-sample-macros)
* [🔧 RXPP Preprocessor Directives (##)](#-rxpp-preprocessor-directives-)
  * [`##USE file`](#use-file)
  * [`##DATA array-name` ](#data-array-name)
  * [`##SYSxxx`](#sysxxx)
  * [`##CFLAG values`](#cflag-values)
  * [`##SET var value`](#set-var-value)
  * [`##UNSET var`](#unset-var)
  * [`##INCLUDE file`](#include-file)
  * [`##IF var`](#if-var)
  * [`##IFN var`](#ifn-var)
  * [`##ELSE`](#else)
  * [`##ENDIF` or `##END`](#endif-or-end)
* [🧭 Pre-Compilation Flow](#-pre-compilation-flow)
* [⚙️ Behavior Notes](#-behavior-notes)
* [🧪 Example with Nesting](#-example-with-nesting)
* [RXPP + CREXX Build System Documentation](#rxpp--crexx-build-system-documentation)
  * [📦 Overview](#-overview)
* [🚀 Usage Example](#-usage-example)
  * [📂 Input/Output Example](#-inputoutput-example)
* [🧭 Pipeline Flow Diagram](#-pipeline-flow-diagram)
* [🛠 Troubleshooting Guide](#-troubleshooting-guide)
  * [📦 Overview](#-overview)
* [🚀 Usage Example](#-usage-example)
  * [📂 Input/Output Example](#-inputoutput-example)
* [📁 Scripts: Windows Batch (.bat) and Linux Shell (.sh)](#-scripts-windows-batch-bat-and-linux-shell-sh)
  * [rxCREXX.bat](#rxcrexxbat)
  * [rxCREXX.sh](#rxcrexxsh)
  * [rxflags.bat](#rxflagsbat)
  * [rxflags.sh](#rxflagssh)
  * [rxconfig.bat](#rxconfigbat)
  * [rxconfig.sh](#rxconfigsh)
  * [rxprecomp.bat](#rxprecompbat)
  * [rxprecomp.sh](#rxprecompsh)
  * [rxcompile.bat](#rxcompilebat)
  * [rxcompile.sh](#rxcompilesh)
  * [rxasm.bat](#rxasmbat)
  * [rxasm.sh](#rxasmsh)
  * [rxrun.bat](#rxrunbat)
  * [rxrun.sh](#rxrunsh)
  * [🧭 Pipeline Flow Diagram](#-pipeline-flow-diagram)
* [🛠 Troubleshooting Guide](#-troubleshooting-guide)

---

### 💼 Overview

* **RXPP** handles macro expansion for CREXX source files.
* **rxCREXX** is the master script (batch or shell) that handles: precompile → compile → assemble → run phases based on input flags.
* Scripts are modular and support plugin-based builds using REXX virtual machine tools.

This document explains how the RXPP (REXX Preprocessor for CREXX) macro system functions.

---

## 🔧 What is RXPP?

**RXPP** is a preprocessor for REXX scripts designed to run within the **CREXX** environment. It provides a lightweight macro system that allows developers to define and expand code snippets before the script is interpreted.

---

## 💪 What RXPP Macros Do

RXPP macros:

* Define code templates that can be reused
* Allow parameter substitution (including variadic parameters)
* Are expanded at *compile time* (before CREXX execution)
* Help simplify and modularize REXX code

---

## ✅ Macro Definition

Macros are defined using the syntax:

```rexx
##define MACRONAME(arg1, arg2) {macro body using arg1, arg2, ...}
```

Or, without arguments:

```rexx
##define MACRONAME {macro body}
```

* Macros must be defined **before** they're used as the preprocessor is a one-pass compiler.
* Multiple REXX statements within a macro must be separated by a semicolon (`;`).
* The macro body is enclosed in `{}` and treated as replacement text.
* RXPP supports multi-line macro definitions using C-style line continuation syntax:
```rexx
##define swap(a,b) {temp=a  \
a=b     \
b=temp }
```
***Note:*** The backslash (`\`) must be the final character on each continued line. Comments after the backslash are not allowed. However, `/* ... */`-style comments may appear before the backslash. `##` comments are not allowed on continued lines, as they indicate the end of line interpretation.

---

### 🔤 Parameter Replacement Rules

When expanding a macro, RXPP replaces parameter names in the macro body with the corresponding argument values from the macro call.

By default, a parameter name is only replaced if it appears as a **stand-alone identifier**. This means:

- The character **before** the parameter name (if any) is **not** an alphanumeric character or underscore (`A–Z`, `a–z`, `0–9`, `_`).
- The character **after** the parameter name (if any) is **not** an alphanumeric character or underscore.

This prevents accidental replacements inside longer identifiers.

**Example (default behavior):**
```
name         → replaced
name_keys    → not replaced
myname       → not replaced
```

If you want to join the parameter value directly to another identifier without triggering the boundary check, use the `##` operator immediately after the parameter name.

- `##` is **not output**; it is removed during expansion.
- The replacement occurs unconditionally, even if the next character is alphanumeric or underscore.

**Example (using `##`):**
```
Macro body:   ##define create(name)   {name##_keys}
Macro call:   create(foo)
Expansion:    foo_keys
```

**Before/After Macro Expansion:**
```rexx
-- Macro definition:
##define create(name, size) { \
  name##_keys.size = ''         \
  name##_values.size = ''       \ 
}

-- Source code before preprocessing:
create(myStem, 10)

-- Expanded code after preprocessing:
myStem_keys.10 = ''
myStem_values.10 = ''
```

**Summary:**
- Bare parameter names → replaced only at identifier boundaries.
- Parameter names followed by `##` → replaced always, with `##` removed.

---

## 🆕 Keyword Parameters in Macros

RXPP now supports **keyword parameters** in macro definitions.  
These work similarly to keyword arguments in many programming languages:  
they appear **after** all positional parameters and may have default values.

---

### 📜 Syntax

```rexx
##define MACRONAME(pos1, pos2, ..., key1=default, key2=) {macro body}
```

- **Positional parameters**: listed first, no `=` in their declaration.
- **Keyword parameters**: must follow positional parameters, declared with `=`.
  - A value after `=` is the **default** if not provided in the macro call.
  - If nothing follows `=`, the default is an **empty string**.

---

### 📌 Rules

1. **Ordering matters** — all positional parameters must be declared first, followed by all keyword parameters.
2. In a macro call:
  - Positional arguments are assigned in order.
  - Remaining arguments must be `name=value` pairs matching keyword parameters.
3. Defaults:
  - If a keyword parameter is not specified in the call, its default value is used.
  - Defaults can be quoted strings, numbers, or empty.
4. **Validation**:
  - Missing positional arguments cause a preprocessor error.
  - Unknown keyword names cause a preprocessor error.
  - Positional parameters **cannot** appear after a keyword parameter in the definition.

---

### 📦 Example: Defaults & Overrides

```rexx
##define myMacro(a,b,c,name='Fred',key=) {len=a; len2=b; mykey=key; len3=c; myname=name}

-- Using defaults:
myMacro(5,10,54321)
```

**Expansion:**
```rexx
len=5
len2=10
mykey=
len3=54321
myname='Fred'
```

---

### 📦 Example: Overriding keywords

```rexx
myMacro(5,10,54321,key=42,name='George')
```

**Expansion:**
```rexx
len=5
len2=10
mykey=42
len3=54321
myname='George'
```

---

### 📦 Example: Mixed order in call

Keyword arguments may appear in any order **after** all positional arguments:

```rexx
myMacro(5,10,54321,name='Anna',key='X')
myMacro(5,10,54321,key='X',name='Anna')   -- Same result
```

---

### 💡 Summary Table

| Definition Form | Allowed? | Notes |
|-----------------|----------|-------|
| `##define m(a,b,key=)` | ✅ | 2 positional, 1 keyword |
| `##define m(a,key=,b)` | ❌ | Positional after keyword not allowed |
| `##define m(a=1,b=2)`  | ✅ | All keyword parameters (no positional) |

---

## 📊 Parsing Positional vs Keyword Parameters

RXPP parses the `##define` argument list **left-to-right** and builds two sets:

- `positional[]` — parameters without `=`
- `keywords{name}` — parameters declared with `=` and optional default

Once the parser sees the **first `=`**, it switches into **keyword mode**. From then on, all parameters must be keyword form.

---


### 🧪 Valid & Invalid Examples

```rexx
/* Definition */
##define myMacro(a,b,c,name='Fred',key=) { ... }

/* ✅ OK */
myMacro(1,2,3)
myMacro(1,2,3, name='Ann')
myMacro(1,2,3, key=99, name='Ann')

/* ❌ Error: positional after keyword */
myMacro(1,2, name='X', 3)

/* ❌ Error: missing positional */
myMacro(1,2)

/* ❌ Error: unknown keyword */
myMacro(1,2,3, who=4)
```

---

## 🤖 Macro Invocation

A macro can be invoked like a function in REXX:

```rexx
say DOUBLE(4)
say debug()       /* note: macros defined without parameters must be called with an empty parameter list */
```

They can also act as commands which expand into a series of REXX statements. For example:

```rexx
##define Liststem(stem) {do _indx=1 to stem.0; say stem._indx ; end}
```

This macro, when invoked as `Liststem(fruits)`, would expand into a loop that prints all elements of the `fruits.` stem array.

***Note:*** A macro may contain an incomplete `do` statement. In such cases, the macro expansion must be completed manually in your source code. For example:

```rexx
##define repeat(n) {do __i=1 to n}
```

You must then close the `do` block with an `end` statement where you use the macro.

Macros and macro libraries can also be included in your code using the `##include` directive:

```rexx
##include path/to/macro_library.rexx
```

This will inject the contents of the specified file into the source before preprocessing continues.

During preprocessing, RXPP replaces the macro invocation with the expanded body:

```rexx
##define DOUBLE(x) {2*x}
say DOUBLE(4)
```

is transformed into:

```rexx
/* +++ say DOUBLE(4) +++ */
say 2*4
```

---

## 📥 Macro Inclusion

Macros and macro libraries can also be included in your code using the `##include` directive:

```rexx
##include path/to/macro_library.rexx
```

This directive inserts the contents of the specified file directly into the source code.

* RXPP supports **recursive includes** — included files can themselves contain further `##include` directives.
* Once the full source stream is assembled through all includes, the macro resolution phase begins.

Alternatively, you can also specify a macro library using the `-m` command-line option:

```sh
RXPP -i input.rexx -o output.rexx -m macro_library.rexx
```

This method automatically includes the macro library at the start of preprocessing, making it ideal for standard or shared macro definitions.

These options allow for clean, modular macro organization and reuse across multiple REXX programs.

---

## 📊 How It Works Internally

### 1. **Registration**

* `##define` lines are parsed and stored in arrays:

  * `macros_mname.`
  * `macros_margs.`
  * `macros_mbody.`

### 2. **Detection**

* RXPP scans each line of the source for macro invocations by looking for known macro names followed by `(`

### 3. **Substitution**

* Arguments are extracted from the call
* The macro body is copied and placeholder names are replaced with actual arguments

### 4. **Variadic Macros**

* Use `...` in the definition to accept multiple arguments
* RXPP generates a version of the macro body for each extra argument
* You can use the special variable `$indx` as a stem index
* Example:

```rexx
##define list2Stem(name, ...) {name.$indx=arglist.$indx}
```

This will repeat the macro body as many times as there are variadic arguments.

### 5. **Emission**

* Final expanded code lines are stored in `outbuf.` and written to the output file
* Original macro lines are commented out to preserve source readability and prevent reprocessing

---

## ✨ Example

### Input:

```rexx
##define DOUBLE(x) {2*x}
say DOUBLE(4)
```

### Output:

```rexx
/* +++ say DOUBLE(4) +++ */
say 2*4
```

---

## 🚀 Benefits

* Lightweight and fast preprocessing
* Makes code more readable and reusable
* Supports complex patterns like loops and variadic templates
* Plays well with standard CREXX tooling

---

## 📊 Common Use Cases

* **Debug macros** (`debug(expr)`)
* **Loop templates** (`foreach(stem, index)`)
* **Inline math expressions** (`SQUARE(x)`, `DOUBLE(x)`)
* **Data initialization** (`stemlist(name, ...)`)

---

## 🧪 Invocation Syntax

RXPP is called with the following syntax:

```sh
RXPP -i input-rexx-to-be-compiled -o compiled-rexx -m optional-macro-library
```

* `-i`: Input REXX file to be precompiled
* `-o`: Output file with expanded REXX code
* `-m`: Optional macro library file to include

---

## 📚 Sample Macros

Here are some useful sample macros and their purposes:

```rexx
##define DOUBLE(x) {2*x}             /* Multiplies a number by 2 */
##define SQUARE(x) {x*x}            /* Computes the square of a value */
##define debug(expr) {say '>>' expr '=' expr}   /* Prints a debug message with the evaluated expression */
##define foreach(stem, indx) {do indx=1 to stem.0}   /* Loop over stem array indices */
##define stemlist(name, ...) {name.$indx=substr(arglist.$indx,3)}  /* Create stem entries from quoted list */
##define list2Stem(name, ...) {name.$indx=arglist.$indx}  /* Assign each variadic value to stem.name */
##define log(msg) {call lineout('log.txt', msg)}   /* Write a log message to a file */
```

These macros simplify routine tasks and make your REXX code shorter and clearer.


## 🔧 RXPP Preprocessor Directives (##)

RXPP supports a set of preprocessor-style directives for conditional compilation, macro expansion control, and variable handling. These are executed during preprocessing, before REXX interpretation.

---


### `##USE file`

Like `##INCLUDE`, this directive injects the contents of the specified file into the source code, **but defers its inclusion to the end of the resulting REXX script**.

This is particularly useful for appending utility code, subroutines, or deferred content without disrupting the main control flow of the primary script.

**Syntax:**

```rexx
##USE myfooter.rexx
```

**Behavior:**

- Contents of `myfooter.rexx` are read during preprocessing.
- The code is **appended** at the **end** of the final output file.
- Files specified with `##USE` are processed after all `##INCLUDE` content and macro expansions.

---

### `##DATA array-name`

The **##DATA** directive allows you to define literal data lines directly within the source file. These lines are assigned to a REXX stem array under the specified array-name.

Each line is initially read as written, with surrounding quotes (single or double) preserved to retain the intended string content. Quotes within strings are not escaped or altered during this step.

***Note:*** If a line contains a macro call or preprocessor variable, it will be expanded in a later preprocessing stage. So while the line is treated as a literal string at first, it may still undergo transformation before reaching the final output.

**Use Case:** This mechanism is useful for embedding configuration values, data records, or script fragments directly in the source, without relying on external files.

**Syntax:**

```rexx
##DATA fruits
apple
banana
cherry
##end
```

**Behavior:**

- Populates the stem `fruits.` as follows:
  ```rexx
  fruits.1 = "apple"
  fruits.2 = "banana"
  fruits.3 = "cherry"
  fruits.0 = 3
  ```
- The `##end` line marks the termination of the data block.


**Example containing quote delimiters:**
```rexx
##DATA MYTEXT
This is a line with 'inner quotes'
This is another line
##end

Resulting Stem Array:
MYTEXT.0 = 2
MYTEXT.1 = "This is a line with 'inner quotes'"
MYTEXT.2 = 'This is a simple line'
```

### `##SYSxxx`

The ##SYSxxx directive serves as a shorthand for ##DATA SYSxxx, where xxx is an arbitrary identifier. It embeds the subsequent input lines directly into the stem array SYSxxx..

This mechanism is conceptually similar to MVS JCL, where the //SYSxxx prefix denotes system DD (Data Definition) statements. For example, //SYSIN DD * is used to pass inline data to a program, and //SYSLIB DD specifies a system library.
This approach is reminiscent of the MVS JCL //SYSIN DD * statement, where inline data is passed to programs.

Syntax:

##SYSIN
param1
param2
param3
##end

Behavior:

Creates a stem SYSIN. with each line as an entry:

SYSIN.1 = "param1"
SYSIN.2 = "param2"
SYSIN.3 = "param3"
SYSIN.0 = 3

##SYSUT1
"C:\temp\my_tempfile.txt"
##end

##SYSLIB
"C:\temp\my_macro_lib.rexx"
"C:\temp\general_macro_lib.rexx"
##end

Resulting Stem Arrays:
After preprocessing, the following stem variables will be populated:

SYSUT1.0 = 1
SYSUT1.1 = "C:\temp\my_tempfile.txt"

SYSLIB.0 = 2
SYSLIB.1 = "C:\temp\my_macro_lib.rexx"
SYSLIB.2 = "C:\temp\general_macro_lib.rexx"

In this example:

##SYSUT1 defines a single inline entry (SYSUT1.1) pointing to a temporary file.

##SYSLIB includes two library paths assigned to SYSLIB.1 and SYSLIB.2.

These stem arrays can then be processed in your program as needed, similar to how JCL uses DD statements like //SYSUT1 or //SYSLIB.

Use Case: Provides a concise method to define system input directly in the script, especially for batch-like workflows.

---
### `##CFLAG values`

##CFLAG — Sets the preprocessor variable from compiler flags or external input during the earliest configuration pass, before normal preprocessing begins.
The definition must be placed at the very beginning of the source file, before any other ## macro instructions appear.

Use the following flags in `cflags` to control diagnostic output during the pre-compilation process:

| Option       | Description                                                                                                                      |
|--------------|----------------------------------------------------------------------------------------------------------------------------------|
| **def**      | Displays all `##DEFINE` instructions present in the source file. Definitions from `maclib` are never shown.                      |
| **set**      | Displays all `##SET` instructions. If not set, these instructions are suppressed from output.                                    |
| **iflink**   | Shows the linkage between `##IF` / `##IFN` and their corresponding `##ELSE` and `##ENDIF` instructions.                          |
| **1buf**     | Displays the raw source input immediately after it is read from the file.                                                        |
| **2buf**     | Displays the source buffer after the second processing pass, where conditional instructions (`##IF` / `##ENDIF`) are structured. |
| **3buf**     | Displays the final source buffer just before it is passed to the preprocessor.                                                   |
| **vars**     | Prints all defined variables, including internal variables and those set via `##SET`.                                            |
| **maclist**  | Displays all loaded macro definitions, including those imported via `maclib`.                                                    |
| **includes** | Lists all modules imported via `##INCLUDE` and `##USE` directives, including recursively nested dependencies.                    |
If a specific flag is not set, the corresponding option is disabled by default. Alternatively, you can explicitly disable an option by prefixing the flag with n (e.g., nset, n1buf, etc.)

**Example:**
```rexx
##cflags def set iflink nbuf 2buf 3buf vars nmaclist  /* set early stage compiler flags */
```


### `##SET var value`

Defines or updates a preprocessor variable. These variables can be embedded within standard REXX statements, macros, or ##DATA content definitions.

The value assigned is processed as follows:
* Any trailing comment (defined by ##comment or /* comment */) on the same line is removed.
* The entire string after the variable name is taken.
* Leading and trailing spaces are stripped.
* The resulting string is assigned to the preprocessor variable.

Quoting is not required unless you need to preserve leading or trailing spaces. If you use quotes, they are included as part of the variable's value.

```rexx
##SET DEBUG 1        ## switch on debug mode 
##define log        {say time('l')' log record' ; say '{prefix} something'}
##SET prefix Log:    ## set a prefix string for the log statement
```
Usage:
```rexx
 log()                       ## and re-expand another log macro
 say {prefix}                ## output the current prefix->compiler variable
##DATA SYSIN
  current used prefix {prefix}
#end  
```

The **PRINTGEN** variable controls whether generation steps are logged as comments in the generated REXX script. This does not affect whether the generation steps are performed — it only affects what is visible in the output.
```rexx
##SET PRINTGEN ALL
```
Logs all generation steps, including nested ones, as comments in the generated REXX script.
```rexx
##SET PRINTGEN NONE
```
No generation steps are logged as comments. The steps are still executed but leave no trace in the output.
```rexx
##SET PRINTGEN NNEST
```
Logs only top-level generation steps (i.e., direct macro calls) as comments. Nested macro calls are not logged.

***Note:*** Use this setting to control the verbosity of the generated script for easier debugging or cleaner output.

### `##UNSET var`

Removes a previously defined variable from the preprocessor context.

**Example:**

```rexx
##UNSET DEBUG
```

### `##INCLUDE file`

Includes the contents of an external file into the source at the point of invocation. Nested includes are supported. By default, the file is resolved relative to the current working directory. If the file resides elsewhere, a fully qualified path must be provided. Quotation marks around the filename are not required.

Paths passed to `##include` may contain `.` and `..` segments to navigate relative directories. These are automatically **normalized** by the preprocessor before the file is included.

**Examples:**

```rexx
##INCLUDE myrexx.rexx
##INCLUDE ../shared/config.rxh
##INCLUDE /usr/lib/../local/lib/module.rxh
##INCLUDE C:/project/lib/./math/../utils/functions.rxh
```

These paths will be resolved to their normalized form before inclusion:

```
../shared/config.rxh                      → resolved relative to current directory
/usr/lib/../local/lib/module.rxh         → /usr/local/lib/module.rxh
C:/project/lib/./math/../utils/functions.rxh → C:/project/lib/utils/functions.rxh
```

#### Notes:
- Both **absolute** and **relative** paths are supported.
- Path separators `/` and `\` are both accepted; they are internally normalized to `/`.
- Multiple slashes are collapsed (`//` becomes `/`).
- Windows-style drive prefixes (`C:/`, `D:/`) are preserved.
- UNC paths (`//server/share/path`) are allowed.





### `##IF var`

Begins a conditional block that is processed only if the specified variable is defined in the preprocessor context. No evaluation of the variable’s content or value is performed—only its existence is checked.
**Example:**

```rexx
##IF DEBUG
  say "Debugging"
##ENDIF
```

### `##IFN var`

(Short for `IF NOT`)

Begins a conditional block that is processed only if the specified variable is not defined in the preprocessor context. The variable’s content or value is not evaluated—only its absence is considered.

**Example:**

```rexx
##IFN DEBUG
  say "Not in debug mode"
##ENDIF
```
### `##ELSE`

Begins a block that is executed when the condition in a preceding ##IF evaluates to false (i.e., the variable is not defined), or when a ##IFN condition evaluates to false (i.e., the variable is defined).

**Example:**

```rexx
##IFN DEBUG
  say "Not in debug mode"
##else 
  say "We are in debug mode"  
##ENDIF
```

### `##ENDIF` or `##END`

Closes the nearest open `##IF` or `##IFN` block.

**Example:**

```rexx
##IF DEBUG
  say "Debugging"
##IFN VERBOSE
  say "Minimal output"
##ENDIF
##ENDIF
```

## 🧭 Pre-Compilation Flow
This document illustrates the main routine of the pre-compilation process, showing when each buffer (`1buf`, `2buf`, `3buf`) and macros and variables are printed based on the `cflags` configuration.


```
┌────────────────────────────────────┐
│ call rxppinit infile               │
│ → Initializes global variables     │
└────────────────────────────────────┘
               │
               ▼
┌────────────────────────────────────┐
│ RXPPPassOne(infile, outfile, ...)  │
│ → Loads source & macro library     │
│ → Pass 1 completed                 │
└────────────────────────────────────┘
               │
    ┌──────────┴────────────┐
    ▼                       ▼
 [If '1buf' in cflags]   [Skip if not]
 call list_array source,...,"Buffer after Pass 1"
               │
               ▼
┌────────────────────────────────────┐
│ call RXPPPassTwo                   │
│ → Expands conditional blocks       │
│   (e.g., ##ELSE handling)          │
└────────────────────────────────────┘
               │
    ┌──────────┴────────────┐
    ▼                       ▼
 [If '2buf' in cflags]   [Skip if not]
 call list_array source,...,"Buffer after Pass 2"
               │
               ▼
┌────────────────────────────────────┐
│ call RXPPPassThree outfile         │
│ → Fully expands macros             │
└────────────────────────────────────┘
               │
    ┌──────────┴────────────┐
    ▼                       ▼
 [If '3buf' in cflags]   [Skip if not]
 call list_array outbuf,...,"Buffer after Pass 3"
               │
               ▼
┌────────────────────────────────────┐
│ call writeall outbuf, outfile      │
│ → Writes final output to file      │
└────────────────────────────────────┘
               │
               ▼
┌────────────────────────────────────┐
│ Additional Diagnostics:            │
│ - printvars (if 'vars' in cflags)  │
│ - printmacs (if 'maclist' set)     |
|  - includes  (if 'ìncludes' set)   │
└────────────────────────────────────┘
```


---

## ⚙️ Behavior Notes

| Feature                        | Description                                                          |
| ------------------------------ | -------------------------------------------------------------------- |
| ✅ **Nested `##IF`/`##IFN` blocks** | Fully supported, including combinations (e.g., `##IF` inside `##IFN`) |
| 🆎 Case                        | Variable names are case-insensitive                                  |
| 📄 Variable scope              | All variables are global to the preprocessor pass                     |
| 🔁 Processing stages           | All `##IF`/`##IFN` are evaluated before macro expansion              |
| 💥 Error handling              | Unmatched `##IF` or `##ENDIF` produces an error                      |

---

## 🧪 Example with Nesting

```rexx
##SET DEBUG 1
##SET VERBOSE 0

##IF DEBUG
  say "Debug mode"
  ##IFN VERBOSE
    say "Silent debug"
  ##ENDIF
##ENDIF
```

This will output:

```
Debug mode
Silent debug
```

---


---
## RXPP + CREXX Build System Documentation


This document combines the functionality of the RXPP macro preprocessor and the full CREXX script processing pipeline, including both Windows batch and Linux shell versions.

---

### 📦 Overview

* **RXPP** handles macro expansion for CREXX source files.
* **rxCREXX** is the master script (batch or shell) that handles: precompile → compile → assemble → run phases based on input flags.
* Scripts are modular and support plugin-based builds using REXX virtual machine tools.

---

## 🚀 Usage Example

To execute a full CREXX processing pipeline:

```bash
./rxCREXX.sh PCAR macro1.rxpp macro1.rexx maclib.rexx
```

Or on Windows:

```bat
rxCREXX.bat PCAR macro1.rxpp macro1.rexx maclib.rexx
```

**Where:**

* `PCAR` are the flags for each step:

  * `P`: Precompile
  * `C`: Compile
  * `A`: Assemble
  * `R`: Run
* `macro1.rxpp` is the source file to precompile
* `macro1.rexx` is the generated output file
* `maclib.rexx` is the macro library used during preprocessing

All output paths, build directories, and runtime libraries are configured in the `rxconfig` file.

### 📂 Input/Output Example

**Inputs:**

* `macro1.rxpp`: RXPP macro source
* `maclib.rexx`: macro definitions

**Generated Files:**

* `macro1.rexx`: RXPP-expanded output (precompiled REXX)
* `macro1.obj` or similar: compiled object/bytecode
* Final executable or linked result (e.g., binary or VM-loadable code)

**Directories (from config):**

* Input: `$sourcelib` (from home/lib/plugins/precomp)
* Output: `$build/lib/plugins/precomp` (pluglib)
* Dependencies: `$build/lib/rxfnsb/library` (library functions)

---

## 🧭 Pipeline Flow Diagram

```text
macro1.rxpp + maclib.rexx
        │
        ▼
   [Precompile - RXPP]
        │     (rxprecomp.sh → macro1.rexx)
        ▼
  macro1.rexx (generated REXX)
        │
        ▼
   [Compile - rxc]
        │     (rxcompile.sh → macro1.obj)
        ▼
    macro1.obj (compiled)
        │
        ▼
   [Assemble - rxas]
        │     (rxasm.sh → macro1 binary)
        ▼
   [Run - rxvm]
              (rxrun.sh → executes macro1)
```

---

## 🛠 Troubleshooting Guide

| Issue                          | Cause                                        | Resolution                                                                                      |
| ------------------------------ | -------------------------------------------- | ----------------------------------------------------------------------------------------------- |
| `command not found`            | Script path or permissions                   | Ensure `chmod +x *.sh` is run and the script is in your PATH or called directly (`./script.sh`) |
| `No such file or directory`    | File path typo or missing input              | Check that input files like `macro1.rxpp` or `maclib.rexx` exist and match case exactly         |
| Compilation fails              | Missing macro expansion or syntax error      | Verify `rxpp` macro resolves correctly and input REXX syntax is valid                           |
| `member` variable is empty     | `basename` failed or wrong file name passed  | Make sure the third parameter is a valid filename (e.g. `macro1.rexx`)                          |
| Output missing                 | Incorrect config path or script failure      | Check values in `rxconfig.sh` and run with `set -x` to debug                                    |
| No execution / No output shown | `RUN` flag missing or script silently failed | Include `R` in flags and add `echo`/`set -x` in run script to trace it                          |

> Tip: If a step fails, test each script individually (e.g., `source rxprecomp.sh`) to isolate the issue.

---

### 📦 Overview

* **RXPP** handles macro expansion for CREXX source files.
* **rxCREXX** is the master script (batch or shell) that handles: precompile → compile → assemble → run phases based on input flags.
* Scripts are modular and support plugin-based builds using REXX virtual machine tools.

---

## 🚀 Usage Example

To execute a full CREXX processing pipeline:

```bash
./rxCREXX.sh PCAR macro1.rxpp macro1.rexx maclib.rexx
```

Or on Windows:

```bat
rxCREXX.bat PCAR macro1.rxpp macro1.rexx maclib.rexx
```

**Where:**

* `PCAR` are the flags for each step:

  * `P`: Precompile
  * `C`: Compile
  * `A`: Assemble
  * `R`: Run
* `macro1.rxpp` is the source file to precompile
* `macro1.rexx` is the generated output file
* `maclib.rexx` is the macro library used during preprocessing

All output paths, build directories, and runtime libraries are configured in the `rxconfig` file.

### 📂 Input/Output Example

**Inputs:**

* `macro1.rxpp`: RXPP macro source
* `maclib.rexx`: macro definitions

**Generated Files:**

* `macro1.rexx`: RXPP-expanded output (precompiled REXX)
* `macro1.obj` or similar: compiled object/bytecode
* Final executable or linked result (e.g., binary or VM-loadable code)

**Directories (from config):**

* Input: `$sourcelib` (from home/lib/plugins/precomp)
* Output: `$build/lib/plugins/precomp` (pluglib)
* Dependencies: `$build/lib/rxfnsb/library` (library functions)


## 📁 Scripts: Windows Batch (.bat) and Linux Shell (.sh)

### rxCREXX.bat
```bat
@echo off
setlocal
:: Input parameters
set flags=%~1
set inrexx=%~2
set genrexx=%~3
set maclib=%~4

for %%f in ("%~3") do (
    set "member=%%~nf"
)

setlocal enabledelayedexpansion
call rxflags.bat

if "!precomp!"=="P" call rxprecomp.bat
if "!compile!"=="C" call rxcompile.bat
if "!asm!"=="A" call rxasm.bat
if "!run!"=="R" call rxrun.bat
```

### rxCREXX.sh
```bash
#!/bin/bash
flags="$1"
inrexx="$2"
genrexx="$3"
maclib="$4"

member=$(basename "$genrexx" | cut -d. -f1)

source ./rxflags.sh

if [ "$PRECOMP" = "P" ]; then source ./rxprecomp.sh; fi
if [ "$COMPILE" = "C" ]; then source ./rxcompile.sh; fi
if [ "$ASM" = "A" ]; then source ./rxasm.sh; fi
if [ "$RUN" = "R" ]; then source ./rxrun.sh; fi
```

### rxflags.bat
```bat
for /l %%i in (0,1,5) do (
    set "char=!flags:~%%i,1!"
    if "!char!"=="P" set "precomp=P"
    if "!char!"=="C" set "compile=C"
    if "!char!"=="A" set "asm=A"
    if "!char!"=="R" set "run=R"
)
set conf="L"
```

### rxflags.sh
```bash
PRECOMP=""
COMPILE=""
ASM=""
RUN=""

for (( i=0; i<5 && i<${#flags}; i++ )); do
    char="${flags:$i:1}"
    case "$char" in
        P) PRECOMP="P" ;;
        C) COMPILE="C" ;;
        A) ASM="A" ;;
        R) RUN="R" ;;
    esac
done

conf="L"
```

### rxconfig.bat
```bat
@echo off
echo Configuration File loaded
set preprocessor=rxpp
set plugin=precomp
set conf=L

set home=C:/Users/PeterJ/CLionProjects/CREXX/250601
set build=%home%/cmake-build-debug
set pluglib=%build%/lib/plugins/%plugin%
set sourcelib=%home%/lib/plugins/%plugin%
set lib=%build%/lib/rxfnsb/library
set rxc=%build%/compiler
set rxas=%build%/assembler
set rxvm=%build%/interpreter
set rxpre=%pluglib%/%preprocessor%
```

### rxconfig.sh
```bash
echo "Configuration File loaded"

preprocessor="rxpp"
plugin="precomp"
conf="L"

home="$HOME/CLionProjects/CREXX/250601"
build="$home/cmake-build-debug"
pluglib="$build/lib/plugins/$plugin"
sourcelib="$home/lib/plugins/$plugin"
lib="$build/lib/rxfnsb/library"
rxc="$build/compiler"
rxas="$build/assembler"
rxvm="$build/interpreter"
rxpre="$pluglib/$preprocessor"
```

### rxprecomp.bat
```bat
if NOT "%conf%"=="L" call rxconfig.bat
pushd "%pluglib%"
set cmd=%rxvm%/rxvm %rxpre% rx_%plugin% %lib% -a -i "%sourcelib%/%inrexx%" -o "%sourcelib%/%genrexx%" -m "%sourcelib%/%maclib%"
%cmd%
popd
```

### rxprecomp.sh
```bash
if [ "$conf" != "L" ]; then source ./rxconfig.sh; fi

pushd "$pluglib" > /dev/null || exit 1
cmd="$rxvm/rxvm $rxpre rx_$plugin $lib -a -i "$sourcelib/$inrexx" -o "$sourcelib/$genrexx" -m "$sourcelib/$maclib""
eval $cmd
popd > /dev/null
```

### rxcompile.bat
```bat
if NOT "%conf%"=="L" call rxconfig.bat
pushd "%pluglib%"
set cmd=%rxc%/rxc -i %build%/lib/rxfnsb;%pluglib% -o %member% %sourcelib%/%member%
%cmd%
popd
```

### rxcompile.sh
```bash
if [ "$conf" != "L" ]; then source ./rxconfig.sh; fi

pushd "$pluglib" > /dev/null || exit 1
cmd="$rxc/rxc -i $build/lib/rxfnsb:$pluglib -o $member $sourcelib/$member"
eval $cmd
popd > /dev/null
```

### rxasm.bat
```bat
if NOT "%conf%"=="L" call rxconfig.bat
pushd "%pluglib%"
set cmd=%rxas%/rxas -l %pluglib% -o %member% %member%
%cmd%
popd
```

### rxasm.sh
```bash
if [ "$conf" != "L" ]; then source ./rxconfig.sh; fi

pushd "$pluglib" > /dev/null || exit 1
cmd="$rxas/rxas -l $pluglib -o $member $member"
eval $cmd
popd > /dev/null
```

### rxrun.bat
```bat
if NOT "%conf%"=="L" call rxconfig.bat
pushd "%pluglib%"
set cmd=%rxvm%/rxvm %member% rx_%plugin% %lib% -a
%cmd%
popd
```

### rxrun.sh
```bash
if [ "$conf" != "L" ]; then source ./rxconfig.sh; fi

pushd "$pluglib" > /dev/null || exit 1
cmd="$rxvm/rxvm $member rx_$plugin $lib -a"
eval $cmd
popd > /dev/null
```



### 🧭 Pipeline Flow Diagram

```text
macro1.rxpp + maclib.rexx
        │
        ▼
   [Precompile - RXPP]
        │     (rxprecomp.sh → macro1.rexx)
        ▼
  macro1.rexx (generated REXX)
        │
        ▼
   [Compile - rxc]
        │     (rxcompile.sh → macro1.obj)
        ▼
    macro1.obj (compiled)
        │
        ▼
   [Assemble - rxas]
        │     (rxasm.sh → macro1 binary)
        ▼
   [Run - rxvm]
              (rxrun.sh → executes macro1)
```

---

## 🛠 Troubleshooting Guide

| Issue                          | Cause                                        | Resolution                                                                                      |
| ------------------------------ | -------------------------------------------- | ----------------------------------------------------------------------------------------------- |
| `command not found`            | Script path or permissions                   | Ensure `chmod +x *.sh` is run and the script is in your PATH or called directly (`./script.sh`) |
| `No such file or directory`    | File path typo or missing input              | Check that input files like `macro1.rxpp` or `maclib.rexx` exist and match case exactly         |
| Compilation fails              | Missing macro expansion or syntax error      | Verify `rxpp` macro resolves correctly and input REXX syntax is valid                           |
| `member` variable is empty     | `basename` failed or wrong file name passed  | Make sure the third parameter is a valid filename (e.g. `macro1.rexx`)                          |
| Output missing                 | Incorrect config path or script failure      | Check values in `rxconfig.sh` and run with `set -x` to debug                                    |
| No execution / No output shown | `RUN` flag missing or script silently failed | Include `R` in flags and add `echo`/`set -x` in run script to trace it                          |

> Tip: If a step fails, test each script individually (e.g., `source rxprecomp.sh`) to isolate the issue.

---
