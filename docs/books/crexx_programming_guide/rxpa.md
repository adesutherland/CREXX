# cRexx /PA \- Plugin Architecture

This facility allows for the compilation, linking, and execution of additional functionality (developed in C) alongside Rexx code. It enables the decoupling of native modules (plugins), which can be developed and packaged either as separate entities or linked statically to the main cRexx core solution. 

The architecture is designed to decouple dynamic plugins from the internal
cRexx libraries. The installed development surface starts at
`<rxpa/crexxpa.h>` and includes its public generated version and integer-type
headers through the `CREXX::RXPA` CMake target.

Plugin developers have the flexibility to structure their code in a way that allows for both dynamic or static linking. While the source code can remain the same, it needs to be built differently based on the desired linking approach, as it relies on macro expansion.

During the Rexx compilation process, the Rexx compiler inspects the plugin to determine the type and argument of any provided functions. This inspection helps ensure type safety. It is recommended for plugin developers to load or initialise dependencies only when an explicitly exposed initialisation Rexx function is called or when the first function is invoked (lazy initialisation) to avoid overhead during build (rather than run). In addition, for the static builds, there is macro definition **DECL\_ONLY** which allows definition / implementation code to be excluded from the static library designed to be linked to the compiler only.

For dynamically packaged plugins (with the extension \*.rxplugin), the search process for these plugins mirrors how cRexx locates Rexx modules (extensions such as \*.rexx, \*.rxas, or \*.rxbin). In contrast, for static builds, the provided functions are loaded before the execution of the main() function in the core crexx solution, however these functions are placed at the end of the search order, meaning users can override static function definitions with local native or crexx modules.

The Plugin Architecture offers a comprehensive set of resources for developers, including a header file, macros, and a cmake configuration. These tools aim to create a convenient and efficient development environment for plugin creation.

## Dynamic Plugins Recommended

User-provided plugins are recommended to be provided as dynamic `.rxplugin` files. Dynamic plugins offer several advantages: easy site-wide distribution by placing them in the same directory as cRexx binaries, project-specific customization by locating them in the project Rexx files directory, and flexible placement in any desired location using `rxc` and `rxvm` options.

In contrast, static plugin packaging is more complex in terms of linking and is intended for core cRexx components that are part of every cRexx release. Static plugins are shipped within the cRexx binaries, ensuring their availability and consistency across distributions.

The choice between dynamic and static plugin packaging depends on the specific requirements and use cases: dynamic plugins provide flexibility and customization for users, while static plugins are designed to provide a robust solution for core cRexx components.

## Example Plugin

The following code demonstrates a decryption plugin.

- The PROCEDURE macro starts the function, and argument access is via macros like ARG().

- Each argument is a cRexx register. This means it holds multiple types or values and these are accesses by macros like GETSTRING() and SETSTRING().

-  Errors are handled by defining and returning a SIGNAL via the RETURNSIGNAL macro.

```C
/*------------------------------------------------------------------*/
/*                                                                  */
/* Name: rxdes.C                                                    */
/*                                                                  */
/* Copyright René Jansen june 1st 1993                              */
/*                                                                  */
/* Function: crexx external functions RxDesEncrypt and RxDesDecrypt */
/*                                                                  */
/* dependencies/calls: desbase.c CREXX/rxpa                         */
/*                                                                  */
/* Ported to CREXX by Adrian Sutherland 2024                        */
/*------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <rxpa/crexxpa.h> // cRexx/PA - installed Plugin Architecture header
#include "desbase.h"    // DES encryption/decryption functions

// Encrypt a string using DES - the first argument is the key, the second the data
PROCEDURE(RxDesEncrypt)
{
    char   des_out[8];
    char   des_in[8];
    char   key[8];
    char   result[17];

    if( NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")

    if (strlen(GETSTRING(ARG(0))) != 16 || strlen(GETSTRING(ARG(1))) != 16)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Both arguments must be 16 hex digits")

    if( hex2bin(GETSTRING(ARG(0)), key) < 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Key is not valid hex")

    if( hex2bin(GETSTRING(ARG(1)), des_in) < 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Data is not valid hex")

    // Encrypt
    desinit(key);
    endes(des_in, des_out);

    // Set return and make sure the signal is reset/ok
    bin2hex(des_out, result);
    SETSTRING(RETURN, result);
    RESETSIGNAL
}

// Decrypt a string using DES - the first argument is the key, the second the data
PROCEDURE(RxDesDecrypt)
{
    char   des_out[8];
    char   des_in[8];
    char   key[8];
    char   result[17];

    if( NUM_ARGS != 2)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")

    if (strlen(GETSTRING(ARG(0))) != 16 || strlen(GETSTRING(ARG(1))) != 16)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Both arguments must be 16 hex digits")

    if( hex2bin(GETSTRING(ARG(0)), key) < 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Key is not valid hex")

    if( hex2bin(GETSTRING(ARG(1)), des_in) < 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Data is not valid hex")

    // Decrypt
    desinit(key);
    dedes(des_in, des_out);

    // Set return and make sure the signal is reset/ok
    bin2hex(des_out, result);
    SETSTRING(RETURN, result);
    RESETSIGNAL
}

// Functions to be provided to rexx - these are loaded either when the plugin is loaded (dynamic) or
// before main() is called (static)
LOADFUNCS
//      C Function__, REXX namespace & name, Option_, Return Type_, Arguments
ADDPROC(RxDesDecrypt, "rxdes.decrypt",       "b",     ".string",    "key=.string,data=.string");
ADDPROC(RxDesEncrypt, "rxdes.encrypt",       "b",     ".string",    "key=.string,data=.string");
ENDLOADFUNCS

// End of file
```

The following shows how the defined functions are published to cRexx.

```C
// Functions to be provided to rexx \- these are loaded either when the*  
// plugin is loaded (dynamic) or before main() is called (static)*  
LOADFUNCS  
//      C Function\_\_, REXX namespace & name, Option\_, Return Type\_,*   
// Arguments*  
ADDPROC (RxDesDecrypt, "rxdes.decrypt",       "b",     ".string",     
   "key=.string,data=.string");  
ADDPROC (RxDesEncrypt, "rxdes.encrypt",       "b",     ".string",      
   "key=.string,data=.string");  
ENDLOADFUNCS**
```

The ADDPROC Macro published the function to cRexx. This has the namespace and name, options/level (should be b), the function return type (in level b format) and the arguments (again in level b format). This allows the Compiler and VM to search and “fix-up” procedure calls using the same search order and syntax as for procedures developed in Rexx.

The return and argument strings are Level B declarations, not comma-separated
type lists. Each argument therefore needs its source-level name and declaration,
for example `left=.int,right=.int`; `.int,.int` is invalid. Empty components,
malformed separators, and malformed return or argument type syntax are rejected
at the importing cREXX call site with `RXPA_IMPORT_SIGNATURE_INVALID`. The
diagnostic identifies the plugin, routine, failing field, and declaration.

## Macros

The following macros are provided for plugin developers (defined in
`rxpa/crexxpa.h`).

| Macro     | Purpose                                     |
|-----------|---------------------------------------------|
| LOADFUNCS | Starts and finishes the block of ADDFUNC()’s|
| ADDFUNC() | Published a function to cRexx|
| NUM\_ARGS | Is the number of arguments passed the the function|
| ARG() | Returns the nth argument (which is an opaque pointer to the cRexx register)|
| RETURN | Returns the register used to pass the function’s returned value.|
| GETSTRING() | Gets the String value of a register|
| SETSTRING() | Sets the String value of a register|
| GETINT() | Gets the Integer value of a register|
| SETINT() | Sets the Integer value of a register|
| GETFLOAT() | Gets the float (double) value of a register|
| SETFLOAT() | Sets the float (double) value of a register|
| SIGNAL | Returns the registers used to pass and Signal Information.|
| RESETSIGNAL | Ensures that the signal register is set no “no signal”|
| RETURNSIGNAL() | Sets the signal register and returns from the function. Used for error conditions.|
| ENDLOADFUNCS | Starts and finishes the block of ADDFUNC()’s|

`SETSTRING`, `RETURNSTR`, and the detail text passed to `RETURNSIGNAL` accept
`const char *`. The VM copies the null-terminated bytes into register-owned
storage during the call; it does not mutate or retain the caller's buffer. The
caller therefore retains ownership and may reuse or release a mutable source
buffer as soon as the macro returns. Pass a non-null, null-terminated string;
use `""` for an empty value.

The Signal values are:

| Signal     | Purpose                                     |
|-----------|---------------------------------------------|
| SIGNAL\_NONE ||
| SIGNAL\_ERROR | Syntax error | SIGNAL\_OVERFLOW\_UNDERFLOW | numeric overflow or underflow| 
| SIGNAL\_CONVERSION\_ERROR | conversion error between types|
| SIGNAL\_UNKNOWN\_INSTRUCTION | unknown RSAS instruction|
| SIGNAL\_FUNCTION\_NOT\_FOUND | attempt to execute an unknown function|
| SIGNAL\_OUT\_OF\_RANGE | attempt to access an array element that is out of range|
| SIGNAL\_FAILURE | error in an external function or subroutine|
| SIGNAL\_HALT | an external request to halt execution|
| SIGNAL\_NOTREADY | IO error, such as a file not being ready|
| SIGNAL\_INVALID\_ARGUMENTS | invalid arguments are passed to a function|
| SIGNAL\_OTHER | Other (or unknown) error condition |

In addition 

* **BUILD\_DLL** is defined if the dynamic (rather than static) version of the plugin is being built. Developers may check for this macro.  
* **DECL\_ONLY** is designed for static builds linked to the compiler, enabling plugin developers to package a smaller library with declaration support only. The build process would create a separate static library with full functionality separately to link with rxvm. This facility is not provided for dynamic plugins to prevent confusion between declaration-only and full-function files.

## Build Script

Install cRexx to a development prefix, then consume its installed CMake package.
The project must not copy `crexxpa.h`, generated headers, or
`RXPluginFunction.cmake` from a cRexx source or build tree.

```cmake
cmake_minimum_required(VERSION 3.24)
project(rxdes C)

# The package version is the core semantic version. CREXX_VERSION_STRING below
# contains the exact release/prerelease/build identity.
find_package(CREXX 1.0.0 EXACT CONFIG REQUIRED)

if(NOT CREXX_VERSION_STRING STREQUAL "1.0.0-beta.3")
    message(FATAL_ERROR
            "This plugin requires cRexx 1.0.0-beta.3; found ${CREXX_VERSION_STRING}")
endif()

# Optional; the default for an external project is <build>/bin.
set(CREXX_RXPA_PLUGIN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugin")
add_dynamic_plugin_target(des rxdes.c desbase.c desbase.h)
```

Configure with the selected scratch or system prefix explicitly:

```console
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/crexx-prefix
cmake --build build --target des --verbose
```

`find_package(CREXX CONFIG)` provides:

- the header-only `CREXX::RXPA` target and the
  `add_dynamic_plugin_target()` helper;
- imported executable targets such as `CREXX::rxc`, `CREXX::rxas`,
  `CREXX::crexx-contract`, `CREXX::rxvm`, and `CREXX::rxbvm`, with corresponding
  `CREXX_<tool>_EXECUTABLE` path variables;
- the `crexx_add_operation_contract()` helper for the build-time, JSON contract
  surface documented in [Operation Contracts](operation_contracts.md);
- `CREXX_IMPORT_DIR`, `CREXX_PLUGIN_DIR`, and `CREXX_RUNTIME_DIR`, which name
  the installed locations. A project-local plugin directory must still be
  supplied explicitly;
- `CREXX_VERSION_STRING`, `CREXX_VERSION_DISPLAY`, `CREXX_PACKAGE_PREFIX`,
  and `CREXX_BUILDINFO_FILE` for machine-readable identity checks.

The helper builds the platform-appropriate module with the `.rxplugin` suffix
on Windows, macOS, and Linux. Dynamic plugins receive the RXPA callback table
from the VM and do not link a private cRexx archive.

## Static Builds

The cRexx Cmake build configuration should be referenced for static build support. Building the static library is simple, but proper linking is crucial to guarantee that the linker recognizes the need to link in the plugin and that the plugin's initialization function is called at program load across various platforms.

## Execution from Rexx

The following is a Rexx Level B example using the plugin. 

Note that there is no manual loading of the dynamic or static plugin, instead cRexx loads the plugins using the same search rules as it uses for other Rexx modules. This means that the Rexx program (or programmer) does not need to be concerned about how the external function is provided \- Rexx, Native, Dynamic, Static \- it all has the same calling syntax. This is designed to meet the objective to simplify programming.
```rexx <!--execdes.rexx-->
options levelb 
import rxfnsb
import rxdes    /* Import the rxdes plugin functions */

/* Note that the input and output to the des functions are in hex strings */
Plaintext = "0000000000000000"  
key =       "08192A3B4C5D6E7F"  
                                        
Ciphertext = Encrypt(key,Plaintext)
```

In line 4, the import statement loads the namespace meaning that any Rexx modules or native plugins in the rxdes namespace will be loaded as needed; the function call, encrypt(), follows Rexx syntax, and the compiler can check parameters and return types as normal.

The compiler and VM paths are separate and explicit. For a plugin produced as
`/path/to/plugin/rxdes.rxplugin` and a cRexx installation at
`/path/to/crexx-prefix`, a complete optimized build and run is:

```console
/path/to/crexx-prefix/bin/rxc \
  -i /path/to/plugin -i /path/to/crexx-prefix/bin \
  -o execdes execdes.crexx
/path/to/crexx-prefix/bin/rxas -o execdes.rxbin execdes.rxas
/path/to/crexx-prefix/bin/rxvm \
  execdes.rxbin /path/to/plugin/rxdes \
  /path/to/crexx-prefix/bin/library
```

Use `-n` with both `rxc` and `rxas` for a non-optimized build. Omitting the
plugin directory from the compiler import path makes its declarations
unavailable; omitting the plugin module path from the VM invocation prevents
the implementation from loading. The `.rxplugin` and `.rxbin` suffixes may be
omitted from VM module arguments, but the directory must remain unambiguous.

<!-- ## Future Changes -->

<!-- * Extending the Architecture to support Address environments and variable “pool” access   -->
<!-- * JSON Remote Plugin Support implementation (aka CREXXSAA)   -->
<!-- * Additional Core Plugins (e.g.NCurses, SQLite, Curl, etc)   -->
<!-- * Object and Decimal Support -->
