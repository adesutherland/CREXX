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

**Physical Placement and Hoisting:**
To ensure robust scoping, global variables physically reside in the **Namespace Scope** of the module. During the compilation process, when a variable is identified as global (either via the `namespace expose` list or a `procedure expose` list), the compiler physically **hoists** the variable symbol from its local discovery scope up to the Namespace Scope. This ensures that standard tiered resolution (Local -> Namespace -> Universe) correctly identifies the variable across all procedures and blocks within the module. This hoisting is strictly upward; the compiler never demotes a namespace-level variable to a local scope.

**Namespace Isolation of Global Variables:**
Unlike exposed functions, which can be called across imported modules, **exposed global variables are strictly isolated to their originating module's namespace**. The compiler explicitly prevents cross-namespace variable resolution. To read or write a global variable from a module in a different namespace, you must wrap the variable access in an exposed function (a getter/setter) and import that function.

**Typing and Shadowing of Global Variables:**
Global variables are typed based on their first use or declaration. All modules exposing the variable must agree on this type.
- **Top-Level Declarations:** In the top-level scope of a procedure (i.e. directly inside `procedure`), a typed declaration like `x = .int` does NOT create a local shadow variable. Instead, it asserts and binds the type to the global variable `x`.
- **Sub-Scope Declarations (Shadowing):** In sub-scopes (e.g. inside a `do...end` block or an `if` block), a typed declaration like `x = .int` **creates a shadow local variable** that hides the global variable for the duration of that block. Conversely, an untyped assignment like `x = 10` inside a sub-scope binds directly to and updates the global variable.
