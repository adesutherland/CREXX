# Walker Cartography & Validation Logic Map

This document maps the validation passes and "magic" logic within `compiler/rxcpbval.c`.

## Execution Pipeline

The `validate_ast` function (in `rxcp_val_orch.c`) orchestrates the following sequence. Passes 5-13 run within a **Fixpoint Iteration Loop** (up to 16 iterations).

1.  **Initial Checks (`initial_checks_walker`)**: 
    *   Sets source location pointers for error reporting.
    *   Restructures procedures and classes into a logical hierarchy.
    *   Converts `OP_SCONCAT` to `OP_CONCAT` when no physical whitespace exists.
    *   Removes redundant `NOP` instructions.
    *   Validates `ASSEMBLER` mnemonics and operands.

2.  **Float/Decimal Conversion (`float2decimal_walker` / `decimal2float_walker`)**: 
    *   Literal type normalization based on `OPTIONS` (Classic vs. Common).

3.  **Library Requirement Check (`needs_rxsysb_walker`)**: 
    *   Flags if the `_rxsysb` system library is needed (for `ADDRESS`, `EXIT`, or `IMPLICIT_CMD`).

4.  **Library Injection (`add_rxsysb_walker`)**: 
    *   Injects `IMPORT _rxsysb` into the `PROGRAM_FILE` if needed.

--- 
### The Fixpoint Loop (`do...while(context->changed)`)

5.  **Plugin Dispatch (`plugin_dispatch_walker` - Pass A)**: 
    *   Consults the Bridge for `IMPLICIT_CMD` nodes. 
    *   Performs **Code Injection** if the plugin returns a Rexx string.
    *   Splices injected AST nodes and sets `context->changed = 1`.

6.  **Implicit Command Transformation (`rewrite_implicit_cmd_walker`)**:
    *   Rewrites non-handled `IMPLICIT_CMD` nodes.
    *   If the child is a `MEMBER_CALL`, `FACTORY_CALL`, or `FUNCTION`, it is promoted to a regular instruction.
    *   Otherwise, it is rewritten to `ADDRESS SYSTEM`.

7.  **Ordinal Assignment (`set_node_ordinals_walker`)**: 
    *   Refreshes execution order metadata (low/high range) for all nodes.

8.  **Symbol Harvesting (`build_symbols_walker`)**: 
    *   Constructs the Symbol Table and defines Scopes for the current tree state.

9.  **Function Resolution (`resolve_functions_walker`)**: 
    *   Links function calls to their definitions (including newly injected code).

10. **Exposed Symbol Resolution (`exposed_symbols_walker`)**: 
    *   Handles variable exposure across procedure boundaries.

11. **Symbol Validation (`validate_symbols`)**: 
    *   Checks for duplicate definitions and semantic symbol errors.

12. **Plugin Dispatch (`plugin_dispatch_walker` - Pass B)**:
    *   Allows plugins to react to resolved symbols or types.

13. **Type Inference (`set_node_types_walker`)**: 
    *   Propagates types through the tree. Handles first-assignment inference.

14. **System Instruction Rewriting (`rewrite_address_walker` / `rewrite_exit_walker`)**: 
    *   Transforms `ADDRESS` and `EXIT` into internal system function calls.

---

15. **Type Safety (`type_safety_walker`)**: 
    *   Final verification of type compatibility using the promotion matrix.

16. **Function Call Type Safety (`func_type_safety_walker`)**: 
    *   Validates arguments and reference parameters.

17. **Decimal Configuration (`decimal_parameters_walker`)**: 
    *   Sets precision and format parameters per scope.

## Business Rule Inventory ("The Magic")

| Behavior | Logic Location | Description |
| :--- | :--- | :--- |
| **Array Length Intrinsics** | `set_node_types_walker` | `arr[]` or `arr[..., void]` is automatically typed as `TP_INTEGER` (returning the array length). |
| **Implicit IMPORT Injection** | `add_rxsysb_walker` | Automatically adds `import _rxsysb` if `ADDRESS` or `EXIT` is used, ensuring system functions are available. |
| **ADDRESS Instruction Rewrite** | `rewrite_address_walker` | Transforms `ADDRESS` into a standard function call and assignment to `rc`. |
| **EXIT Instruction Rewrite** | `rewrite_exit_walker` | Transforms `EXIT` into a call to the internal `_exit` function. |
| **SCONCAT Normalization** | `initial_checks_walker` | Converts "space concatenation" tokens to standard concatenation if no physical space exists. |
| **Automatic Type Inference** | `set_node_types_walker` | Variables are typed upon first assignment; subsequent assignments to different types trigger errors. |
| **Type Promotion** | `validate_node_promotion` | Uses a hardcoded `promotion` matrix to determine if one type can be implicitly cast to another. |

## State Coupling (Global Variables & Shared State)

| Global / Shared State | Pass(es) | Access Mode |
| :--- | :--- | :--- |
| `Context *context` | All | Read/Write. Main carrier of compiler state (current scope, current procedure, options). |
| `static const ValueType promotion[9][9]` | `set_node_types_walker`, `type_safety_walker` | Read-only. Defines rules for type compatibility and promotion. |
| `context->need_rxsysb` | `needs_rxsysb_walker`, `add_rxsysb_walker` | `needs` writes 1, `add` reads it to perform injection. |
| `node->value_type` / `node->target_type` | `set_node_types_walker` onwards | `set_node_types` writes; subsequent passes read for validation. |
| `context->current_scope` | Almost All | Managed by walkers to track the active symbol scope. |
