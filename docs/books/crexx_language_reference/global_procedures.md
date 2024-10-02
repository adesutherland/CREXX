# Global Procedures {#global-procedures}

## **Exposing Global Procedures** {#exposing-global-procedures}

A procedure defined in a module file can be made available to other modules (provided it is in an imported namespace) by including the procedure in the namespace instruction.

*EXAMPLE:*

namespace mynamespace expose myfunc1 myfunc2

## **Importing Global Procedures** {#importing-global-procedures}

When compiling a program, the compiler looks for procedures that are called but not defined in the current module. It searches in various locations in the following order:

1. Current directory: REXX files, then RXAS files, then RXBIN files, and finally CREXX/C native function libraries. This is for project-specific modules and libraries.  
2. Imported locations (specified with the rxc \-i option): REXX files, then RXAS files, then RXBIN files, and finally CREXX/C native function libraries. This is for user or optional system libraries.  
3. Directory where the compiler executable (rxc) is located: REXX files, then RXAS files, then RXBIN files, and finally CREXX/C native function libraries. This is for distribution or system-wide libraries.

The compiler only uses external procedures that are in the same namespace as the module being compiled or for namespaces listed in the "import" instruction. 

*EXAMPLE:*

import rxfnsb mynamespace anothernamespace

Metadata encoded with the procedures allows the compiler to determine the type and argument types of imported procedures.

The crexx level b standard library namespace is rxfnsb, to access these procedures programs should import this namespace.

*EXAMPLE:*

import rxfnsb

