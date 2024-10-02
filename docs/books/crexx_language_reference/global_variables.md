# Global Variables {#global-variables}

Global variables are shared among modules and are linked to the namespace of the modules, similar to exposed procedures.

Variables can be exposed through the module namespace instruction like procedures, making them accessible across all procedures within the module file.

*EXAMPLE:*

Namespace myproject expose global\_var

Alternatively, the expose keyword in the procedure instruction can be used to make variables available only within that specific procedure.

*EXAMPLE:*

proc: procedure expose global\_var

The availability of global variables is limited to modules within the same namespace, excluding imported namespaces. To access a variable from a module in a different namespace (via importing), a wrapping variable access procedure is required.

The compiler determines global variable types from rexx, rxas, and rxbin files within the same search paths as exposed procedures. However, only modules in the same namespace are processed. Inconsistency across module files results in an error.
