# External interpreter RXPP example

This small example demonstrates an RXPP source file containing Regina,
Python, JavaScript, and PowerShell code. It does not use a Level L grammar or
an `rxbuild` plan.

The existing `##DALIAS` mechanism collects each tagged block into a cREXX
string array. Simple RXPP macros called inside the blocks are expanded into
the corresponding interpreter language before the arrays are generated. The
cREXX callbacks write those arrays to temporary source files and invoke the
external interpreters through the cREXX command environment.

The source opts into this behavior with `##CFLAG blockmacros`. Free-form
blocks remain literal by default; use `nblockmacros` to explicitly disable the
feature.

## Prerequisites

- A built `crexx.exe`.
- Regina installed at `C:\Program Files\rexx.org\Regina\regina.exe`, or
  configured through the `REGINA_EXE` environment variable.
- Python available as `python` on `PATH`.
- Node.js available as `node` on `PATH`.
- Windows PowerShell available as `powershell.exe` on `PATH`.

From the CREXX project root on Windows, run:

```powershell
.\cmake-build-release\bin\crexx.exe `
  .\examples\external_interpreters\simple_rxpp.rxpp -keep -verbose3
```

To use another Regina installation:

```powershell
$env:REGINA_EXE = 'D:\Tools\Regina\regina.exe'
```

The generated source files are written temporarily in the current working
directory and removed after each interpreter returns. Their names are:

```text
.rxpp_regina_source.rex
.rxpp_python_source.py
.rxpp_javascript_source.js
.rxpp_powershell_source.ps1
```

The program prints the output and diagnostics returned by each external
interpreter. A missing executable or a non-zero interpreter result causes the
example to return a non-zero status.
