# Walker Cartography & Validation Logic Map

This document maps the validation passes and "magic" logic within `compiler/rxcpbval.c`.

## Execution Pipeline

The `rxcp_val` function orchestrates the following sequence of walkers:

1.  **Initial Checks (`initial_checks_walker`)**: 
    *   Sets source location pointers for error reporting.
    *   Converts `OP_SCONCAT` to `OP_CONCAT` when no whitespace exists between tokens.
    *   Removes redundant `NOP` instructions and converts empty `INSTRUCTIONS` blocks to `NOP`.
    *   Validates `ASSEMBLER` instruction mnemonics and operand types.
    *   Converts `OP_ARG_VALUE` with 0 or `NOVAL` index to `OP_ARGS` (count).

2.  **Float/Decimal Conversion (`float2decimal_walker` / `decimal2float_walker`)**: 
    *   Conditionally converts literal types based on `options levelb numeric_...` settings.

3.  **Library Requirement Check (`needs_rxsysb_walker`)**: 
    *   Flags if the `_rxsysb` system library is needed (used by `ADDRESS` and `EXIT`).

4.  **Library Injection (`add_rxsysb_walker`)**: 
    *   Injects `IMPORT _rxsysb` into the `REXX_UNIVERSE` if needed.

5.  **Symbol Harvesting (`build_symbols_walker`)**: 
    *   Constructs the Symbol Table and defines Scopes.

6.  **Function Resolution (`resolve_functions_walker`)**: 
    *   Links function calls to their definitions.

7.  **Exposed Symbol Resolution (`exposed_symbols_walker`)**: 
    *   Handles `EXPOSED` variables in procedures.

8.  **Symbol Validation (`validate_symbols`)**: 
    *   Checks for duplicate definitions and other symbol-level errors.

9.  **Type Inference (`set_node_types_walker`)**: 
    *   Determines `value_type` and `target_type` for all AST nodes.
    *   Implements implicit typing on first assignment.
    *   Handles array indexing and length intrinsics.

10. **ADDRESS Rewriting (`rewrite_address_walker`)**: 
    *   Converts `ADDRESS` instructions to `rc = _address(...)`.
    *   Converts `REDIRECT` nodes to internal function calls (e.g., `_array2redir`).

11. **EXIT Rewriting (`rewrite_exit_walker`)**: 
    *   Converts `EXIT` instructions to `CALL _exit(...)`.

12. **Ordinal Assignment (`set_node_ordinals_walker`)**: 
    *   Sets execution order metadata (high/low ordinals).

13. **Re-Validation (Repeat of Steps 5-9)**: 
    *   Re-runs symbol and type passes to handle changes introduced by `rewrite` walkers.

14. **Type Safety (`type_safety_walker`)**: 
    *   Performs final type compatibility checks using the `promotion` matrix.

15. **Function Call Type Safety (`func_type_safety_walker`)**: 
    *   Validates argument types for function calls, including reference arguments.

16. **Decimal Configuration (`decimal_parameters_walker`)**: 
    *   Sets numeric precision and format parameters for each scope.

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
