# Walker Cartography & Validation Logic Map

This document maps the validation passes and "magic" logic within `compiler/rxcpbval.c`.

## Execution Pipeline

The `validate_ast` function (in `rxcp_val_orch.c`) orchestrates the following sequence. Passes 5-13 run within a **Fixpoint Iteration Loop** (up to 16 iterations).

### 1. Pre-Loop Setup
1.  **AST Structure Fixup (`ast_structure_fixup_walker`)**:
    *   **AST Restructuring**: Fixes the unfinished flat AST produced by the parser into a logical hierarchy (e.g., hoisting procedures from the flat parser list to become children of the file/namespace and nesting their instruction bodies).
    *   *Note on Exits*: Suppresses the generation of implicit `RETURN` nodes for AST fragments coming from the exit framework to avoid `RETVAL_MISSING` conflicts when grafted into typed procedures.

2.  **Source Location Tracking (`source_location_walker`)**:
    *   Sets token boundary pointers and source location metadata bottom-up for accurate error reporting.

3.  **Syntax Validation (`syntax_validation_walker`)**:
    *   Converts `OP_SCONCAT` to `OP_CONCAT` when no physical whitespace exists.
    *   Removes redundant `NOP` instructions.
    *   Validates `ASSEMBLER` mnemonics, operands, and `REPEAT` loops.

4.  **Float/Decimal Conversion (`float2decimal_walker` / `decimal2float_walker`)**: 
    *   Literal type normalization based on `OPTIONS` (Classic vs. Common).

5.  **Library Requirement Check (`needs_rxsysb_walker`)**: 
    *   Flags if the `_rxsysb` system library is needed (for certified exit instructions, `EXIT`, or `IMPLICIT_CMD`).

6.  **Library Injection (`add_rxsysb_walker`)**: 
    *   Injects `IMPORT _rxsysb` into the `PROGRAM_FILE` if needed.

7.  **Import Scanning (`rxcp_scan_imports`)**:
    *   Loads and parses imported files. This is **Idempotent**; once a file is imported, it is marked and subsequent calls return immediately.

--- 
### 2. The Fixpoint Loop (`do...while(context->changed_flags)`)

All walkers within this loop are **Idempotent**. Under debug mode `-d3`, the compiler forces at least 3 iterations and multiple calls per walker to verify this property and ensure AST/Symbol stability.

4.5. **Clear Node Types (`clear_node_types_walker`)**:
    *   Intelligently resets dynamically inferred node types at the start of each validation pass, cleanly erasing stale target types while preserving the types of intrinsic literals and structural blocks.

5.  **Exit Dispatch (`exit_dispatch_walker` - Pass A)**: 
    *   Consults the Bridge for `IMPLICIT_CMD` nodes. 
    *   Performs **Code Injection** if the plugin returns a Rexx string.
    *   Splices injected AST nodes and sets `context->changed_flags |= FLAG_VAL_PLUGIN`.
    *   *Idempotency*: Guarded by `node->exit_obj_reg` to prevent re-processing.

6.  **Implicit Command Transformation (`rewrite_implicit_cmd_walker`)**:
    *   Rewrites non-handled `IMPLICIT_CMD` nodes.
    *   If the child is a `MEMBER_CALL`, `FACTORY_CALL`, or `FUNCTION`, it is promoted to a regular instruction.
    *   Otherwise, it remains on the certified `ADDRESS` path.
    *   Any implicit-address warning policy is owned by the `ADDRESS` certified exit rather than by the generic compiler walker.
    *   *Idempotency*: Mutates the node type, preventing re-execution on the same node.

7.  **Ordinal Assignment (`set_node_ordinals_walker`)**: 
    *   Refreshes execution order metadata (low/high range) for all nodes.
    *   *Idempotency*: Pure recalculation based on current tree state.

8.  **Symbol Harvesting (`build_symbols_walker`)**: 
    *   Constructs the Symbol Table and defines Scopes for the current tree state.
    *   **Block Scoping**: Creates `SCOPE_LOCAL` for simple `DO` groups, `BLOCK_EXPR` nodes, and `IF` branches that use a `DO` block (confinement).
        *   *Intentional Inconsistency*: Single-instruction `IF` branches (e.g., `if cond then x = 1`) execute in the parent scope, whereas `DO` blocks (e.g., `if cond then do; x = 1; end`) create a local scope. This is as designed.
    *   *Idempotency*: Uses existing scopes if already created; `sym_adnd` prevents duplicate symbols.
    *   *Resolution*: Uses **Specialized Resolvers** (`sym_rslv_local`, `sym_rslv_attribute`, `sym_rslv_global`) to prevent "accidental" linkage.
    *   **Symbol Lifecycle**: Every name encountered in the AST is assigned a `Symbol` with an explicit `SymbolStatus` (e.g., `SYM_STATUS_UNRESOLVED`, `SYM_STATUS_LOCAL_DEF`).
    *   **Shadowing Protection**: This replaces the previous "deferred creation" hack. By tracking `UNRESOLVED` vs. `LOCAL` status, the compiler can safely prioritize global resolution in later passes while respecting intentional local definitions.
    *   **Auto-Expose Mechanics**: `NAMESPACE` nodes are now explicitly processed. When a variable is listed in `namespace ... expose X`, the `X` symbol is instantly tagged as `exposed = 1`. Subsequent local uses or `DEFINE` statements (`X = .int`) automatically bind directly to the namespace global, eliminating the need for boilerplate `PROCEDURE EXPOSE X` declarations.

9.  **Function Resolution (`resolve_functions_walker`)**: 
    *   Links function calls to their definitions (including newly injected code).
    *   *Idempotency*: Guarded by `!node->symbolNode`.
    *   *Global Search*: If not found locally, it searches across all PROGRAM_FILE and IMPORTED_FILE roots (`sym_rvfc`).
    *   **Status Transition**: Transitions `SYM_STATUS_UNRESOLVED` symbols to `SYM_STATUS_RESOLVED_GLOBAL` and redirects the AST node to the global symbol if a match is found.

10. **Exposed Symbol Resolution (`exposed_symbols_walker`)**: 
    *   Handles variable exposure across procedure boundaries using `sym_hoist_to_namespace`.
    *   *Idempotency*: Guarded by `symbol->exposed` bit and namespace checks.

11. **Symbol Validation (`validate_symbols`)**: 
    *   Checks for duplicate definitions and semantic symbol errors.
    *   *Idempotency*: Skips symbols whose type is already resolved (`TP_UNKNOWN`).
    *   **Shadowing Matrix**:
        *   *Variables shadowing variables* (e.g. from parent block scopes or globals): **Warning** (`#SHADOWING`).
            *Note*: This warning can be triggered by "forward shadowing" (a local created in a branch before a procedure-level variable is used later). Investigating these warnings is crucial as they may indicate that a variable intended to be global/procedure-wide is actually being isolated in a subscope.
        *   *Variables shadowing functions* (e.g. `pos`, `length`): **Silent** (allowed for developer ergonomics).
        *   *Functions shadowing imported functions*: **Warning**.
        *   *Classes shadowing imported classes*: **Warning**.
        *   *Same-namespace collisions*: Hard Error (`#DUPLICATE_SYMBOL`).

12. **Exit Dispatch (`exit_dispatch_walker` - Pass B)**:
    *   Allows plugins to react to resolved symbols or types.

13. **Type Inference (`set_node_types_walker`)**: 
    *   Propagates types through the tree. Handles first-assignment inference.
    *   *Idempotency*: Skips nodes that already have a resolved type. 

13.1 **ToString Rewrite (`tostring_rewrite_walker`)**:
    *   Detects `TP_OBJECT` nodes requiring `TP_STRING` conversion and verifies that the `.tostring()` method exists.
    *   Rewrites the AST to wrap the object expression in a `MEMBER_CALL("tostring")`.
    *   *Idempotency*: Modifies the AST, triggering the `FLAG_VAL_TYPE` loop continuation flag to re-evaluate types for the new structure.

13.2 **Type Safety (`type_safety_walker`)**: 
    *   Verification of type compatibility using the promotion matrix.
    *   **Block Expressions**: `BLOCK_EXPR` derives its value type from enclosed `LEAVE_WITH` nodes. Multiple `LEAVE_WITH` exits must agree exactly on value type and dimensions.
    *   **Block Exit Diagnostics**: `LEAVE_WITH` outside a block expression yields `#NOT_IN_BLOCK_EXPR`; plain `LEAVE` inside a block expression without a loop target yields `#LEAVE_WITH_REQUIRED`.
    *   *Idempotency*: Can run natively inside the fixed loop. Uses an `is_final_pass` flag to suppress type and argument-related errors inside the iterative resolution loop, only firing them during the final pass.

13.3 **Function Call Type Safety (`func_type_safety_walker`)**: 
    *   Validates arguments and reference parameters.

14. **Certified Exit Lowering and System Rewrite (`exit_dispatch_walker` / `rewrite_exit_walker`)**: 
    *   Lowers certified exits such as `ADDRESS` through the exit bridge and rewrites `EXIT` into the internal `_exit` call.
    *   If exits are disabled, certified primaries that fell back to `IMPLICIT_CMD` are diagnosed before `_rxsysb` import/hoisting.
    *   Certified exits may inject required runtime imports during `REPLACE`; `PARSE` uses this to add `rxfnsb`.
    *   Inline branch statements lowered from certified exits may be wrapped in synthetic `INSTRUCTIONS` blocks for structural safety; these wrappers can inherit the parent lexical scope so hoisted bindings stay visible after the branch.
    *   *Idempotency*: Exit lowering replaces the original node; `rewrite_exit_walker` mutates `EXIT` into a call.

---
### 3. Post-Loop Finalization

15. **Diagnostic Pruning (`rxcp_collect_and_prune_diagnostics`)**:
    *   Collects all `WARNING` and `ERROR` nodes from the AST and moves them to a detached list in the `Context`.
    *   This ensures the AST remains structurally clean for the emitter, while preserving all reported diagnostic information.

16. **Final Type Inference (`set_node_types_walker`)**:
    *   Restored right before the final `type_safety_walker` invocation, ensuring values and arguments retain their expected state before code generation.

17. **Final Type Safety (`type_safety_walker` / `func_type_safety_walker`)**:
    *   Invoked with `is_final_pass` set to true to emit any remaining type compatibility or argument errors.

18. **Decimal Configuration (`decimal_parameters_walker`)**: 
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

## Namespace and Import Behavior

### 1. The `import` Instruction
*   `import` brings an entire **namespace** into the local resolution scope.
*   It does **not** simply include a file; it populates the `importable_function_tree` in the `master_context` with all symbols exposed by that namespace.
*   **Resolution Order**: Local -> Attribute (if in class) -> Current Namespace -> Imported Namespaces.
*   **Qualified Names**: Qualified access (e.g., `Namespace.Function()`) is **NOT supported**. All calls must be direct.

### 2. Exposing Symbols
*   Only symbols explicitly listed in the `expose` clause of a `namespace` instruction are visible to other files.
*   **Syntax**: `namespace name expose sym1 sym2 sym3` (Space-separated, commas are **NOT** used).
*   **Surprises**:
    *   **Variables**: Global variables are eagerly resolved across all imported namespaces. However, they must be explicitly promoted to the namespace scope via an `expose` or `namespace ... expose` clause in the defining file to be visible to importers (due to late hoisting).
    *   **Runtime**: `rxvm` requires all involved `.rxbin` files to be provided on the command line to resolve symbols at runtime.

### 3. Class Imports
*   Classes are imported as **Stubs**. The compiler parses the imported file, extracts the class signature (methods, factories), and injects a stub definition into the `master_context`'s AST.
*   **On-Demand Import**: Currently, class stubs are only imported if the class name is **explicitly referenced** in the source code (e.g., via a factory call `.ClassName()` or a typed variable definition). Merely having a function return an object of that class does **not** automatically trigger the class stub import, which may lead to unresolved method calls at the call site.
*   **Import Precedence**: When multiple importable artifacts share the same module stem, the scanner prefers `.rexx` over `.rxas`, and `.rxas` over `.rxbin`. The compiler also skips generated side-products for the module currently being compiled so stale build artifacts cannot shadow the active source file.

## Business Rule Inventory ("The Magic")

| Behavior | Logic Location | Description |
| :--- | :--- | :--- |
| **Array Length Intrinsics** | `set_node_types_walker` | `arr[]` or `arr[..., void]` is automatically typed as `TP_INTEGER` (returning the array length). |
| **Implicit IMPORT Injection** | `add_rxsysb_walker` | Automatically adds `import _rxsysb` if a certified system exit, `EXIT`, or `IMPLICIT_CMD` needs it. |
| **ADDRESS Certified Exit Lowering** | `exit_dispatch_walker` | Lowers `ADDRESS` through the certified exit bridge into the `_address(...)` call shape. |
| **EXIT Instruction Rewrite** | `rewrite_exit_walker` | Transforms `EXIT` into a call to the internal `_exit` function. |
| **SCONCAT Normalization** | `syntax_validation_walker` | Converts "space concatenation" tokens to standard concatenation if no physical space exists. |
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
