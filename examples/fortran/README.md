# FORTRAN IV support in cREXX

This directory contains the cREXX FORTRAN IV/FORTRAN 66 interpreter and
working examples. The interpreter is implemented as a cREXX module; it does
not add a FORTRAN type system to the cREXX VM.

## Installation

Before using the FORTRAN support, compile and link `FORTRAN.CREXX` from this
directory. The resulting `fortran` module must be available on the cREXX
module search path. RXPP programs containing FORTRAN source import this module
with `import fortran`.

## Basic use

Import the interpreter and place the FORTRAN source in a free-form RXPP block:

```rexx
options levelg
import fortran
##external fortran

FORTRAN RUN
      PROGRAM HELLO
      WRITE (*,*) 'Hello from FORTRAN'
      STOP
      END
END FORTRAN
```

`FORTRAN RUN ... END FORTRAN` collects the source and invokes the imported
`fortran` module automatically. FORTRAN labels and `GOTO` statements are
interpreted by the guest program counter; they do not jump to cREXX labels and
the interpreter state remains available across the transfer.

## Procedures

Procedure units can be defined separately:

```rexx
FORTRAN PROCEDURES
      SUBROUTINE DOUBLE_VALUE(I)
      INTEGER I
      I = I * 2
      RETURN
      END
END FORTRAN
```

They are combined with a `FORTRAN RUN` block. A FORTRAN program can call them
with its normal `CALL` statement. The initial implementation supports scalar
arguments and array-element actual arguments with copy-in/copy-out behavior.

## Host array bindings

A cREXX typed array can be imported with a `C$RX BIND` directive:

```rexx
measurements = .float[]
measurements[1] = 10.0
measurements[2] = 20.0
measurements[0] = 2

FORTRAN RUN
      REAL VALUES(10)
C$RX BIND VALUES = measurements
      ...
END FORTRAN
```

The array count is taken from `measurements[0]`; elements `[1]` through
`[measurements[0]]` are copied into the FORTRAN array.

FORTRAN arrays can be returned with `C$RX EXPORT`:

```rexx
results = .float[]

FORTRAN RUN
      REAL RESULTS(10)
C$RX EXPORT RESULTS = results
      ...
END FORTRAN
```

The destination cREXX array receives the result count in `[0]` and values in
`[1]` through `[count]`. Multiple bind and export directives are supported.

## Current supported subset

The examples cover the following practical FORTRAN IV features:

- fixed-format comments, labels, continuation lines, `PROGRAM`, `END`, and
  `STOP`
- `INTEGER`, `REAL`, fixed-width `CHARACTER`, arrays, `DATA`, and `PARAMETER`
- arithmetic expressions and the intrinsics `ABS`, `MIN`, `MAX`, `SQRT`, `INT`,
  and `MOD`
- labeled `DO`, `CONTINUE`, `GOTO`, arithmetic and logical `IF`, block `IF`,
  `ELSE`, and `END IF`
- `WRITE (*,*)` and labeled formatted output with `F`, `I`, `A`, `X`, and
  quoted text descriptors
- character `LEN`, `INDEX`, substring operations, and `//` concatenation
- `SUBROUTINE`, `CALL`, and `RETURN`
- elapsed-time reporting after interpreter execution

The implementation is intentionally incremental. `COMMON`, `EQUIVALENCE`,
external FORTRAN procedures, full FORTRAN I/O control lists, and complete
FORMAT coverage are not yet part of the supported subset.

## Examples

- `fortran_matrix.rxpp` — matrix multiplication and formatted output
- `fortran_features.rxpp` — control flow, intrinsics, parameters, and strings
- `fortran_loop.rxpp` — loop execution and timing
- `fortran_call.rxpp` — procedures, calls, and array arguments
- `fortran_interpreter.rxpp` — small interpreter smoke example

The RXPP preprocessor regression is registered as `rxpp_fortran_block`.
Generated `.inc`, `.rxas`, `.rxbin`, and driver-lock files are build artifacts;
the source `.crexx` and `.rxpp` files are the files intended for version
control.

## Design status

The detailed implementation history and future roadmap are kept in
`docs/planning/fortran_interpreter_plan.md`. This README describes the current
user-facing interface and supported subset.
