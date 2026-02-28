# cREXX Standard Libraries and Built-In Functions (BIFs)

The `crexx` toolchain implements its Standard Libraries and Built-In Functions (BIFs) using a hybrid approach. Some core functions are implemented natively in C via the **cREXX Plugin Architecture (RXPA)**, while many standard library functions (like those in Classic REXX) are actually implemented in cREXX itself.

Libraries are housed in the `lib/` directory, which is divided into domains like:
- `lib/rxfnsb/` (Classic REXX Built-In Functions for Level B)
- `lib/rxfnsc/` (Level C standard library functions)
- `lib/rxmath/` (Math extensions)
- `lib/plugins/` (General-purpose extensions like `fileio`, `regex`, `strings`, `socket`, etc.)

## 1. BIFs Implemented in cREXX (`lib/rxfnsb/rexx/`)

A significant portion of the Classic REXX Built-In Functions (such as `abs()`, `date()`, `length()`, `substr()`) are written entirely in cREXX. These are located in `lib/rxfnsb/rexx/`. 

This approach minimizes the VM footprint and demonstrates the capability of the cREXX compiler to handle system-level logic.

### Anatomy of a cREXX BIF
Functions written in cREXX follow standard language rules, utilizing namespaces and type enforcement:

```rexx
/* lib/rxfnsb/rexx/abs.rexx */
options levelb

namespace rxfnsb expose abs

abs: procedure = .string
  arg number = .string
  if left(number, 1) = '-' then number = substr(number, 2)
  return number
```

**Key Features:**
1. **Namespaces:** Functions must declare `namespace rxfnsb expose <function_name>` so they correctly bind into the Standard Library space that user scripts import.
2. **Inline Assembly (`assembler`)**: When low-level access is required (such as fetching the current system time in `date.rexx`), cREXX BIFs can drop down into inline bytecode using the `assembler` keyword.
3. **Compilation:** These `.rexx` files are compiled into `.rxbin` bytecodes during the build process and are packaged or shipped exactly like user-compiled binaries.

## 2. RXPA (cREXX Plugin Architecture)

For functions requiring native performance or access to C-level system libraries (like cryptography or sockets), `crexx` provides the RXPA framework. This macro-driven C API (defined in `rxpa/crexxpa.h` and `rxpa/rxpa.h`) allows developers to write REXX-callable functions without interacting directly with the VM's internal `stack_frame` or `value` structures.

Plugins can be compiled in two ways:
1. **Dynamic Plugins (`.rxplugin`)**: Recommended for user extensions. They are discovered and loaded at runtime using the same search paths as regular `.rexx` modules.
2. **Static Plugins**: Built directly into the `crexx` binaries. These are typically reserved for core Standard Libraries to guarantee they are always available.

## 3. Writing a Native Function

A native C function meant to be exposed to REXX is defined using the `PROCEDURE` macro. 

### Argument Access and Returns
The VM passes arguments as opaque handles mapped to internal VM registers. The RXPA headers provide macros to extract native C types from these registers and to write results back:
- `NUM_ARGS`: The count of arguments passed from REXX.
- `ARG(n)`: Retrieves the opaque handle for the *n*th argument.
- `GETINT()`, `GETFLOAT()`, `GETSTRING()`: Extracts the native C value from a register handle.
- `SETINT()`, `SETFLOAT()`, `SETSTRING()`: Writes a native C value into a target register.
- `RETURN`: The specific target register designated for the function's return value.

### Error Handling
Errors are thrown using the `RETURNSIGNAL` macro, which halts execution and raises a specific `RXSIGNAL_*` exception within the VM. Successful execution must conclude with `RESETSIGNAL`.

### Example Native Function

```c
#include "crexxpa.h"

// Example: Add two integers together
PROCEDURE(add_integers)
{
    int result;

    // 1. Validate argument count
    if (NUM_ARGS != 2) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    }

    // 2. Extract values and perform logic
    result = GETINT(ARG(0)) + GETINT(ARG(1));

    // 3. Set the return value
    SETINT(RETURN, result);

    // 4. Clear the signal state and exit
    RESETSIGNAL
}
```

## 4. Registering Functions with the VM

Once the C functions are written, they must be registered so the REXX compiler (`rxc`) and interpreter (`rxvm`) can map REXX namespace calls to the native C function pointers. 

This is accomplished using the `LOADFUNCS` mapping block, which binds the C function pointer to a REXX namespace, declares its return type, and defines its expected argument signature.

```c
// Publish functions to the cREXX compiler and VM
LOADFUNCS
//       C Function       REXX Namespace & Name      Opt  Return Type   Argument Signature
ADDPROC( add_integers,    "rxmath.add_integers",     "b", ".int",       "i1 = .int, i2 = .int" );
ADDPROC( string_concat,   "rxstr.string_concat",     "b", ".string",    "s1 = .string, s2 = .string" );
ENDLOADFUNCS
```

### Registration Breakdown:
1. **C Function**: The literal name of the C function defined by `PROCEDURE(...)`.
2. **REXX Namespace & Name**: How the function will be called from REXX code (e.g., `import rxmath; x = add_integers(1, 2)`).
3. **Option**: The target cREXX language level (`"b"` for Level B, `"c"` for Level C).
4. **Return Type**: A string literal dictating the exact cREXX type returned (`".int"`, `".string"`, `".void"`).
5. **Arguments**: The exact type signature expected by the compiler. It supports standard types, array syntax (`.int[]`), and reference passing (`expose`). 

During compilation, `rxc` parses this block to strictly enforce type safety when the REXX code invokes the native plugin. During execution, `rxvm` uses this block to dynamically link the `.rxplugin` shared library symbol into the execution space.