# RXPP Macro System: How It Works

## 📚 Table of Contents

* [🔧 What is RXPP?](#-what-is-rxpp)
* [💪 What RXPP Macros Do](#-what-rxpp-macros-do)
* [✅ Macro Definition](#-macro-definition)
* [🤖 Macro Invocation](#-macro-invocation)
* [📥 Macro Inclusion](#-macro-inclusion)
* [📊 How It Works Internally](#-how-it-works-internally)
* [✨ Example](#-example)
* [🚀 Benefits](#-benefits)
* [📊 Common Use Cases](#-common-use-cases)
* [🧪 Invocation Syntax](#-invocation-syntax)
* [📚 Sample Macros](#-sample-macros)
* [📦 RXPP Build System](#-overview)
* [🚀 Usage Example](#-usage-example)
* [📂 Input/Output Example](#-inputoutput-example)
* [📂 Scripts: Windows Batch (.bat) and Linux Shell (.sh)](#-inputoutput-example)
* [🧭 Pipeline Flow Diagram](#-pipeline-flow-diagram)
* [🛠 Troubleshooting Guide](#-troubleshooting-guide)


This document explains how the RXPP (REXX Preprocessor for CREXX) macro system functions. It focuses on the mechanics and design rather than macro syntax content.

---

## 🔧 What is RXPP?

**RXPP** is a precompiler for REXX scripts designed to run within the **CREXX** environment. It provides a lightweight macro system that allows developers to define and expand code snippets before the script is interpreted.

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

* Macros must be defined **before** they're used as the pre-compiler is a one-pass compiler
* The macro body must be code on a **single line**; there is **no continuation separator** supported
* Multiple REXX statements within a macro must be separated by a semicolon (`;`)
* The macro body is enclosed in `{}` and treated as replacement text

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

Note: A macro may contain an incomplete `do` statement. In such cases, the macro expansion must be completed manually in your source code. For example:

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
set preCompiler=rxpp
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
set rxpre=%pluglib%/%preCompiler%
```

### rxconfig.sh
```bash
echo "Configuration File loaded"

preCompiler="rxpp"
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
rxpre="$pluglib/$preCompiler"
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
