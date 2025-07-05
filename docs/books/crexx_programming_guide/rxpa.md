# CREXX/PA \- Plugin Architecture

This facility allows for the compilation, linking, and execution of additional functionality (developed C) alongside REXX code. It enables the decoupling of native modules (plugins), which can be developed and packaged either as separate entities or linked statically to the main CREXX core solution. 

The architecture is designed to completely decouple the plugins from the rest of CREXX, and the plugin client library only consists of a single header file (rexxpa.h)

Plugin developers have the flexibility to structure their code in a way that allows for both dynamic or static linking. While the source code can remain the same, it needs to be built differently based on the desired linking approach, as it relies on macro expansion.

During the REXX compilation process, the REXX compiler inspects the plugin to determine the type and argument of any provided functions. This inspection helps ensure type safety. It is recommended for plugin developers to load or initialise dependencies only when an explicitly exposed initialisation REXX function is called or when the first function is invoked (lazy initialisation) to avoid overhead during build (rather than run). In addition, for the static builds, there is macro definition **DECL\_ONLY** which allows definition / implementation code to be excluded from the static library designed to be linked to the compiler only.

For dynamically packaged plugins (with the extension \*.rxplugin), the search process for these plugins mirrors how CREXX locates REXX modules (extensions such as \*.rexx, \*.rxas, or \*.rxbin). In contrast, for static builds, the provided functions are loaded before the execution of the main() function in the core crexx solution, however these functions are placed at the end of the search order, meaning users can override static function definitions with local native or crexx modules.

The Plugin Architecture offers a comprehensive set of resources for developers, including a header file, macros, and a cmake configuration. These tools aim to create a convenient and efficient development environment for plugin creation.

## Dynamic Plugins Recommended

User-provided plugins are recommended to be provided as dynamic "\*.rxplugin" files. Dynamic plugins offer several advantages: easy site-wide distribution by placing them in the same directory as CREXX binaries, project-specific customization by locating them in the project REXX files directory, and flexible placement in any desired location using "rxc" and "rxvm" options.

In contrast, static plugin packaging is more complex in terms of linking and is intended for core CREXX components that are part of every CREXX release. Static plugins are shipped within the CREXX binaries, ensuring their availability and consistency across distributions.

The choice between dynamic and static plugin packaging depends on the specific requirements and use cases: dynamic plugins provide flexibility and customization for users, while static plugins are designed to provide a robust solution for core CREXX components.

## Example Plugin

The following code demonstrates a decryption plugin.

* The PROCEDURE macro starts the function, and argument access is via macros like ARG().   
* Each argument is a CREXX register. This means it holds multiple types or values and these are accesses by macros like GETSTRING() and SETSTRING().  
* Errors are handled by defining and returning a SIGNAL via the RETURNSIGNAL macro.

  *// Decrypt a string using DES \- the first argument is the key,*

  *// the second the data*

  **PROCEDURE**(RxDesDecrypt)

  {

     char   des\_out\[8\];

     char   des\_in\[8\];

     char   key\[8\];

     char   result\[17\];


     if( **NUM\_ARGS** \!= 2)

         **RETURNSIGNAL**(*SIGNAL\_INVALID\_ARGUMENTS*, "2 arguments expected")


     if (strlen(**GETSTRING**(ARG(0))) \!= 16 || strlen(**GETSTRING**(ARG(1))) \!= 16)

         **RETURNSIGNAL**(*SIGNAL\_INVALID\_ARGUMENTS*, "Both arguments must be 16 hex digits")


     if( hex2bin(**GETSTRING**(ARG(0)), key) \< 0)

         **RETURNSIGNAL**(*SIGNAL\_INVALID\_ARGUMENTS*, "Key is not valid hex")


     if( hex2bin(**GETSTRING**(ARG(1)), des\_in) \< 0)

         **RETURNSIGNAL**(*SIGNAL\_INVALID\_ARGUMENTS*, "Data is not valid hex")


     *// Decrypt*

     desinit(key);

     dedes(des\_in, des\_out);


     *// Set return and make sure the signal is reset/ok*

     bin2hex(des\_out, result);

     **SETSTRING**(RETURN, result);

     **RESETSIGNAL**

  }

The following shows how the defined functions are published to CREXX.

*// Functions to be provided to rexx \- these are loaded either when the*  
*// plugin is loaded (dynamic) or before main() is called (static)*  
**LOADFUNCS**  
*//      C Function\_\_, REXX namespace & name, Option\_, Return Type\_,*   
*// Arguments*  
**ADDPROC**(RxDesDecrypt, "rxdes.decrypt",       "b",     ".string",     
   "key=.string,data=.string");  
**ADDPROC**(RxDesEncrypt, "rxdes.encrypt",       "b",     ".string",      
   "key=.string,data=.string");  
**ENDLOADFUNCS**

The ADDPROC Macro published the function to CREXX. This has the namespace and name, options/level (should be b), the function return type (in level b format) and the arguments (again in level b format). This allows the Compiler and VM to search and “fix-up” procedure calls using the same search order and syntax as for procedures developed in REXX.

## Macros

The following MACROs are provided for plugin developers (defined in rexxpa.h)

* LOADFUNCS / ENDLOADFUNCS \- Starts and finishes the block of ADDFUNC()’s  
* ADDFUNC() \- Published a function to CREXX  
* NUM\_ARGS \- Is the number of arguments passed the the function  
* ARG() \- Returns the nth argument (which is an opaque pointer to the CREXX register)  
* RETURN \- Returns the register used to pass the function’s returned value.  
* GETSTRING() / SETSTRING() \- Gets / Sets the String value of a register  
* GETINT() / SETINT() \- Gets / Sets the Integer value of a register  
* GETFLOAD() / SETFLOAT() \- Gets / Sets the float (double) value of a register  
* SIGNAL \- Returns the registers used to pass and Signal Information.  
* RESETSIGNAL \- Ensures that the signal register is set no “no signal”  
* RETURNSIGNAL() \- Sets the signal register and returns from the function. Used for error conditions.

The Signal values are:

* SIGNAL\_NONE  
* SIGNAL\_ERROR \- Syntax error  
* SIGNAL\_OVERFLOW\_UNDERFLOW \- numeric overflow or underflow   
* SIGNAL\_CONVERSION\_ERROR \- conversion error between types  
* SIGNAL\_UNKNOWN\_INSTRUCTION \- unknown RSAS instruction  
* SIGNAL\_FUNCTION\_NOT\_FOUND \- attempt to execute an unknown function  
* SIGNAL\_OUT\_OF\_RANGE \- attempt to access an array element that is out of range  
* SIGNAL\_FAILURE \- error in an external function or subroutine  
* SIGNAL\_HALT \- an external request to halt execution  
* SIGNAL\_NOTREADY \- IO error, such as a file not being ready  
* SIGNAL\_INVALID\_ARGUMENTS \- invalid arguments are passed to a function  
* SIGNAL\_OTHER \- Other (or unknown) error condition

In addition 

* **BUILD\_DLL** is defined if the dynamic (rather than static) version of the plugin is being built. Developers may check for this macro.  
* **DECL\_ONLY** is designed for static builds linked to the compiler, enabling plugin developers to package a smaller library with declaration support only. The build process would create a separate static library with full functionality separately to link with rxvm. This facility is not provided for dynamic plugins to prevent confusion between declaration-only and full-function files.

## Build Script

The following example shows a CMake script (CMakeLists.txt) to build a plugin. It uses a provided CMake function **add\_plugin\_target** from **RXPluginFunction.cmake**.

cmake\_minimum\_required(VERSION 3.5)  
project(rxdes C)

set(CMAKE\_C\_STANDARD 90)

*\# Including RXPlugin Build System*  
include(${CMAKE\_SOURCE\_DIR}/rxpa/RXPluginFunction.cmake)

*\# Create module*  
add\_dynamic\_plugin\_target(des rxdes.c desbase.c desbase.h)

This script builds a dynamic version of the plugin, and supports Windows (mingw and VS), OSX and Linux.

## Static Builds

The CREXX Cmake build configuration should be referenced for static build support. Building the static library is simple, but proper linking is crucial to guarantee that the linker recognizes the need to link in the plugin and that the plugin's initialization function is called at program load across various platforms.

## 

## Execution from REXX

The following is a REXX Level B example using the plugin. 

Note that there is no manual loading of the dynamic or static plugin, instead CREXX loads the plugins using the same search rules as it uses for other REXX modules. This means that the REXX program (or programmer) does not need to be concerned about how the external function is provided \- REXX, Native, Dynamic, Static \- it all has the same calling syntax. This is designed to meet the objective to simplify programming.

options levelb  /\* This is a rexx level b program \*/

import rxfnsb   /\* Import the crexx level B functions \*/  
**import rxdes**    /\* Import the rxdes plugin functions  \*/

/\* Note that the input and output to the des functions are in hex strings \*/

Plaintext \= "0000000000000000"  
key \=       "08192A3B4C5D6E7F"  
                                        
**Ciphertext \= Encrypt(key,Plaintext)**

In **bold**, the import statement loads the namespace meaning that any REXX modules or native plugins in the rxdes namespace will be loaded as needed; the function call, encrypt(), follows REXX syntax, and the compiler can check parameters and return types as normal.

## Future Changes

* Extending the Architecture to support Address environments and variable “pool” access  
* JSON Remote Plugin Support implementation (aka CREXXSAA)  
* Additional Core Plugins (e.g.NCurses, SQLLite, Curl, etc)  
* Object and Decimal Support
