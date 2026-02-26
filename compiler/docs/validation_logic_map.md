# Walker Cartography & Validation Logic Map

This document maps the validation passes and "magic" logic within `compiler/rxcpbval.c`.

## Execution Pipeline

The `validate_ast` function (in `rxcp_val_orch.c`) orchestrates the following sequence. Passes 5-13 run within a **Fixpoint Iteration Loop** (up to 16 iterations).

### 1. Pre-Loop Setup
1.  **Initial Checks (`initial_checks_walker`)**: 
    *   Sets source location pointers for error reporting.
    *   **AST Restructuring**: Fixes the unfinished flat AST produced by the parser into a logical hierarchy (e.g., hoisting procedures from the flat parser list to become children of the file/namespace and nesting their instruction bodies).
    *   Converts `OP_SCONCAT` to `OP_CONCAT` when no physical whitespace exists.
    *   Removes redundant `NOP` instructions.
    *   Validates `ASSEMBLER` mnemonics and operands.

2.  **Float/Decimal Conversion (`float2decimal_walker` / `decimal2float_walker`)**: 
    *   Literal type normalization based on `OPTIONS` (Classic vs. Common).

3.  **Library Requirement Check (`needs_rxsysb_walker`)**: 
    *   Flags if the `_rxsysb` system library is needed (for `ADDRESS`, `EXIT`, or `IMPLICIT_CMD`).

4.  **Library Injection (`add_rxsysb_walker`)**: 
    *   Injects `IMPORT _rxsysb` into the `PROGRAM_FILE` if needed.

5.  **Import Scanning (`rxcp_scan_imports`)**:
    *   Loads and parses imported files. This is **Idempotent**; once a file is imported, it is marked and subsequent calls return immediately.

--- 
### 2. The Fixpoint Loop (`do...while(context->changed)`)

All walkers within this loop are **Idempotent**. Under debug mode `-d3`, the compiler forces at least 3 iterations and multiple calls per walker to verify this property and ensure AST/Symbol stability.

5.  **Exit Dispatch (`exit_dispatch_walker` - Pass A)**: 
    *   Consults the Bridge for `IMPLICIT_CMD` nodes. 
    *   Performs **Code Injection** if the plugin returns a Rexx string.
    *   Splices injected AST nodes and sets `context->changed = 1`.
    *   *Idempotency*: Guarded by `node->exit_obj_reg` to prevent re-processing.

6.  **Implicit Command Transformation (`rewrite_implicit_cmd_walker`)**:
    *   Rewrites non-handled `IMPLICIT_CMD` nodes.
    *   If the child is a `MEMBER_CALL`, `FACTORY_CALL`, or `FUNCTION`, it is promoted to a regular instruction.
    *   Otherwise, it is rewritten to `ADDRESS SYSTEM`.
    *   *Idempotency*: Mutates the node type, preventing re-execution on the same node.

7.  **Ordinal Assignment (`set_node_ordinals_walker`)**: 
    *   Refreshes execution order metadata (low/high range) for all nodes.
    *   *Idempotency*: Pure recalculation based on current tree state.

8.  **Symbol Harvesting (`build_symbols_walker`)**: 
    *   Constructs the Symbol Table and defines Scopes for the current tree state.
    *   *Idempotency*: Uses existing scopes if already created; `sym_adnd` prevents duplicate symbols.
    *   *Resolution*: Uses **Specialized Resolvers** (`sym_rslv_local`, `sym_rslv_attribute`, `sym_rslv_global`) to prevent "accidental" linkage.

9.  **Function Resolution (`resolve_functions_walker`)**: 
    *   Links function calls to their definitions (including newly injected code).
    *   *Idempotency*: Guarded by `!node->symbolNode`.

10. **Exposed Symbol Resolution (`exposed_symbols_walker`)**: 
    *   Handles variable exposure across procedure boundaries using `sym_hoist_to_namespace`.
    *   *Idempotency*: Guarded by `symbol->exposed` bit and namespace checks.

11. **Symbol Validation (`validate_symbols`)**: 
    *   Checks for duplicate definitions and semantic symbol errors.
    *   *Idempotency*: Skips symbols whose type is already resolved (`TP_UNKNOWN`).

12. **Exit Dispatch (`exit_dispatch_walker` - Pass B)**:
    *   Allows plugins to react to resolved symbols or types.

13. **Type Inference (`set_node_types_walker`)**: 
    *   Propagates types through the tree. Handles first-assignment inference.
    *   *Idempotency*: Skips nodes that already have a resolved type.

14. **System Instruction Rewriting (`rewrite_address_walker` / `rewrite_exit_walker`)**: 
    *   Transforms `ADDRESS` and `EXIT` into internal system function calls.
    *   *Idempotency*: Mutates node type to `ASSIGN` or `CALL`.

---
### 3. Post-Loop Finalization

15. **Type Safety (`type_safety_walker`)**: 
    *   Final verification of type compatibility using the promotion matrix.

16. **Function Call Type Safety (`func_type_safety_walker`)**: 
    *   Validates arguments and reference parameters.

17. **Decimal Configuration (`decimal_parameters_walker`)**: 
    *   Sets precision and format parameters per scope.

---
## Scope Hierarchy and Symbol Resolution

The compiler enforces an explicit scope hierarchy (defined in `rxcp_ast_val.c` and `rxcpsymb.c`) to prevent incorrect symbol linkage.

### 1. Scope Types (`ScopeType`)
*   `SCOPE_UNIVERSE`: The root of all compilation (owns global files).
*   `SCOPE_NAMESPACE`: File-level or explicit namespace scope.
*   `SCOPE_CLASS`: Class definition scope (contains attributes and methods).
*   `SCOPE_PROCEDURE`: Procedure, Method, or Factory body.
*   `SCOPE_LOCAL`: Nested blocks (e.g., `INSTRUCTIONS` now; `DO`, `IF` in the future).

### 2. Resolution Rules (Tiered Resolution)
Symbol resolution follows a strict tiered search via `sym_rslv_tiered`:
1.  **Local**: Searches upward from the current scope but stops at the `SCOPE_PROCEDURE` boundary (`sym_rslv_local`).
2.  **Attribute**: If inside a method/factory, jumps to the nearest `SCOPE_CLASS` to find class attributes (`sym_rslv_attribute`).
3.  **Global**: Walks upward through the `SCOPE_NAMESPACE` chain to `SCOPE_UNIVERSE` (`sym_rslv_global`).

### 3. Validation Invariants
The AST/Symbol validator (`rxcp_validate_ast_and_symbols`) asserts these rules in every compiler pass (under `-d2`):
*   `SCOPE_LOCAL` parent must be `SCOPE_PROCEDURE` or `SCOPE_LOCAL`.
*   `SCOPE_PROCEDURE` parent must be `SCOPE_NAMESPACE` or `SCOPE_CLASS`.
*   `SCOPE_CLASS` parent must be `SCOPE_NAMESPACE`.
*   `SCOPE_NAMESPACE` parent must be `SCOPE_UNIVERSE` or `SCOPE_NAMESPACE`.
*   `node->symbolNode->symbol->node == node` (Bi-directional linkage).

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
