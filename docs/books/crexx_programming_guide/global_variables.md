# Global Variables

Global variables are shared among modules and are linked to the namespace of the modules, similar to exposed procedures.

**Auto-Exposing (Recommended):** Variables exposed through the module `namespace` instruction are automatically mapped into the local scope of all procedures within the module file. This eliminates the need to repeatedly `expose` the variable in every procedure.

*EXAMPLE:*

```rexx
namespace myproject expose global_var
```

Alternatively, the `expose` keyword in the `procedure` instruction can be used to explicitly import variables from a dynamic caller or when managing variables not declared in the top-level namespace.

*EXAMPLE:*

```rexx
proc: procedure expose caller_var
```

The availability of global variables is limited to modules within the same namespace, excluding imported namespaces. To access a variable from a module in a different namespace (via importing), a wrapping variable access procedure is required.

The compiler determines global variable types from rexx, rxas, and rxbin files within the same search paths as exposed procedures. However, only modules in the same namespace are processed. Inconsistency across module files results in an error.
