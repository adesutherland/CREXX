# cRexx /PA \- Plugin Architecture

This facility allows for the compilation, linking, and execution of additional functionality (developed in C) alongside Rexx code. It enables the decoupling of native modules (plugins), which can be developed and packaged either as separate entities or linked statically to the main cRexx core solution. 

The architecture is designed to decouple dynamic plugins from the internal
cRexx libraries. The installed development surface starts at
`<rxpa/crexxpa.h>` and includes its public generated version and integer-type
headers through the `CREXX::RXPA` CMake target.

Plugin developers have the flexibility to structure their code in a way that allows for both dynamic or static linking. While the source code can remain the same, it needs to be built differently based on the desired linking approach, as it relies on macro expansion.

During the Rexx compilation process, the Rexx compiler inspects the plugin to determine the type and argument of any provided functions. This inspection helps ensure type safety. It is recommended for plugin developers to load or initialise dependencies only when an explicitly exposed initialisation Rexx function is called or when the first function is invoked (lazy initialisation) to avoid overhead during build (rather than run). In addition, for the static builds, there is macro definition **DECL\_ONLY** which allows definition / implementation code to be excluded from the static library designed to be linked to the compiler only.

The compiler discovers dynamically packaged declarations (`*.rxplugin`) on its
binary import roots. A used native declaration records its stable provider ID
in the compiled module. At runtime, a matching static provider is preferred;
otherwise `rxvm` opens the trusted `<provider-id>.rxplugin` and verifies the
binary's manifest ID and function signature before linking it.

The Plugin Architecture offers a comprehensive set of resources for developers, including a header file, macros, and a cmake configuration. These tools aim to create a convenient and efficient development environment for plugin creation.

## Dynamic Plugins Recommended

User-provided plugins are recommended to be provided as dynamic `.rxplugin` files. Dynamic plugins offer several advantages: easy site-wide distribution by placing them in the same directory as cRexx binaries, project-specific customization by locating them in the project Rexx files directory, and flexible placement in any desired location using `rxc` and `rxvm` options.

In contrast, static plugin packaging is more complex in terms of linking and is intended for core cRexx components that are part of every cRexx release. Static plugins are shipped within the cRexx binaries, ensuring their availability and consistency across distributions.

A supported provider can publish dynamic and static forms as one package:

```cmake
add_dynamic_plugin_target(_example example.c)
add_static_plugin_target(_example example.c)
add_rxpa_provider_package(_example)
```

The helper places the dynamic artifact, canonical static archive
(`<provider-id>.a` or `.lib`), and compatibility `_static` archive under
`bin/providers`. Ordinary `rxvm` execution then needs no `-p`, and
`crexx -native` selects and retains the static archive automatically from the
compiled dependency record.
Application-local providers may instead be placed in a `providers` directory
beside the main `.rxbin`; explicit trusted directories use
`rxvm --provider-path` or `CREXX_PROVIDER_PATH`.

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

## Concurrency and per-VM sessions

An existing plugin needs no source change. A current host treats it as legacy:
one VM uses the direct adapter, while concurrent legacy-capable VMs use a
process-wide recursive compatibility lane. This is the safe default because
the host cannot infer whether plugin statics and native dependencies are
thread-safe.

After auditing every published procedure, writable static, dependency and
cleanup path, a plugin that permits concurrent entry can add one file-scope
declaration:

```c
RXPA_PLUGIN_PROCESS_REENTRANT
```

This is an assertion about defined concurrent behavior, not an assertion that
the functions have no side effects. The optional capability symbol leaves the
existing initializer and call ABI unchanged, and older hosts ignore it.

For a mixed plugin, use `RXPA_PLUGIN_PROCEDURE_CAPABILITIES(query)` and return
`RXPA_PROCEDURE_CAP_PROCESS_REENTRANT` only for audited procedures. Return `0`
for procedures that must remain on the legacy lane.

A plugin that owns a connection, device, interpreter or similar mutable native
resource per VM can use:

```c
RXPA_PLUGIN_SESSION_AWARE(create_session, destroy_session,
                          enter_session, leave_session,
                          procedure_capabilities)
```

The query returns `RXPA_PROCEDURE_CAP_SESSION_AFFINE` for procedures that use
the per-VM session. The host creates one session when that VM loads the plugin,
preselects the call adapter, and destroys the session before unloading plugin
code. `enter_session` receives the selected session and must return the
previous thread-local session as a cookie; `leave_session` restores that cookie
so nested plugin calls work correctly. A failed factory fails the plugin load
and rolls back the DSO and any already-created sessions.

V2-aware hosts fail closed: malformed manifests, unknown or combined flags,
and a session-affine procedure without all four session hooks become legacy.
Older hosts call the unchanged procedures and never invoke the V2 hooks. If
such hosts must remain supported, the plugin procedures must fall back to a
plugin-owned default session when no per-VM session is active. Do not retain
RXPA register handles or VM-owned values in either kind of session.

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
| RXPA_PLUGIN_PROCESS_REENTRANT | Asserts that every published procedure permits concurrent process entry. |
| RXPA_PLUGIN_PROCEDURE_CAPABILITIES() | Supplies a load-time per-procedure reentrant/legacy policy query. |
| RXPA_PLUGIN_SESSION_AWARE() | Supplies the per-procedure query and per-VM session lifecycle/entry hooks. |

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

```bash
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

Test or replacement implementations that intentionally publish an existing
stable provider identity under a different CMake target may pass
`PROVIDER_ID`, for example
`add_dynamic_plugin_target(des_mock PROVIDER_ID rxdes mock.c)`. The output
artifact must still use the matching canonical provider stem; this option does
not create a runtime alias.

Configure with the selected scratch or system prefix explicitly:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/crexx-prefix
cmake --build build --target des --verbose
```

`find_package(CREXX CONFIG)` provides:

- the header-only `CREXX::RXPA` target and the
  `add_dynamic_plugin_target()` helper;
- imported executable targets such as `CREXX::rxc`, `CREXX::rxas`,
  `CREXX::crexx-contract`, product `CREXX::rxvm`, concrete `CREXX::rxbvm`, and
  optional concrete `CREXX::rxtvm`, with corresponding
  `CREXX_<tool>_EXECUTABLE` path variables (`CREXX_rxtvm_FOUND` is false when
  the installed compiler could not build direct threading);
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

There is no manual loading of a declaratively packaged dynamic or static
provider. The Rexx program does not need to distinguish the delivery form, and
no Rexx wrapper is required: RXPA publishes the typed signature directly.
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

```bash
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
