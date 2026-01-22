# cREXX Emitter Architecture

This document describes the architecture and internal logic of `rxcpemit.c`, the code generator for the cREXX compiler (`rxc`).

## 1. High-Level Execution Flow

The emission process transforms the validated Abstract Syntax Tree (AST) into cREXX Assembly (`.rxas`). It is initiated by the `emit()` function and consists of two primary passes using the standard `ast_wlkr` mechanism.

### Pass 1: Register Assignment (`register_walker`)
- **Direction**: Both `in` (Top-Down) and `out` (Bottom-Up).
- **Purpose**: Assigns virtual registers to AST nodes and symbols.
- **Logic**: 
    - Determines if a node can reuse a symbol's register or needs a temporary one.
    - Implements "don't assign" optimization (`DONT_ASSIGN_REGISTER`) to avoid redundant copy instructions for constant-to-variable assignments.
    - Handles register pressure by interacting with the `scope` (via `get_regs`).

### Pass 2: Code Emission (`emit_walker`)
- **Direction**: Both `in` (Top-Down) and `out` (Bottom-Up).
- **Purpose**: Generates assembly text fragments.
- **Logic**:
    - **Top-Down (`in`)**: Used for block-level initialization (e.g., `INSTRUCTIONS` node triggers `add_scope_initiators`).
    - **Bottom-Up (`out`)**: The core of the emitter. Each node type generates its corresponding assembly by concatenating fragments from its children and adding its own instructions.
    - **Finalization**: The root node (`REXX_UNIVERSE`) calls `print_output()` during its `out` phase, which writes the entire concatenated fragment chain to the output file.

## 2. State Map

The emitter avoids most global variables by storing state directly in the `ASTNode` structure or passing it via the `walker_payload`.

| State Element | Location | Purpose |
| :--- | :--- | :--- |
| `emit_promotion` | Static Global | 9x9 matrix mapping `(ValueType, TargetType)` to conversion opcodes (e.g., `itof`, `stod`). |
| `node->output` | `ASTNode` | Primary linked list of `OutputFragment` for the node's code. |
| `node->cleanup` | `ASTNode` | Fragments for register unlinking or cleanup after an expression. |
| `node->loop...` | `ASTNode` | Specialized fragments for loops: `loopstartchecks`, `loopinc`, `loopendchecks`. |
| `node->register_num` | `ASTNode` | The assigned register index. |
| `node->register_type` | `ASTNode` | The register type prefix (`i`, `s`, `f`, `d`, `a`, `o`, `b`). |
| `payload->file` | `walker_payload` | The output `FILE` pointer. |
| `payload->globals` | `walker_payload` | Counter for global variables. |

## 3. Logic Categorization (The Seams)

### Core & Marshalling
- **Functionality**: Linked list management for `OutputFragment`, file I/O, and source-mapped comment generation (`get_metaline...`).
- **Key Functions**: `output_concat`, `output_append_text`, `print_output`, `get_metaline_clause`.

### Expressions & Operators
- **Functionality**: Handling binary/unary operators and constants. 
- **Complexity**: High. Significant code duplication in `emit_walker` handles combinations of constant vs. register operands.
- **Optimization**: Relies on a prior optimization pass (e.g., `rxcp_opt.c`) to perform constant folding and type tagging. The emitter itself does not perform peephole optimization but implements "don't assign" register optimization to reduce assembly `copy` instructions.
- **Key Functions**: `type_promotion`, `format_constant`, `type_to_prefix`.

### Control Flow
- **Functionality**: Generating branch logic and labels for `IF`, `DO`, `LOOP`, `LEAVE`, and `ITERATE`.
- **Labeling**: Uses `node->node_number` combined with suffixes (e.g., `l123dostart`, `l123iffalse`).

### Metadata & Symbols
- **Functionality**: Emitting `.meta` directives and procedure headers.
- **Key Functions**: `meta_set_symbol`, `add_global_variable_metadata`, `meta_narg`.

## 4. Proposed Refactoring Plan

To modularize the ~3.7k line monolith, the following breakdown is proposed:

1.  **`rxcp_emit_core.c`**: `OutputFragment` utilities, `print_output`, and the `emit_promotion` matrix.
2.  **`rxcp_emit_reg.c`**: `register_walker` and register assignment logic.
3.  **`rxcp_emit_expr.c`**: Logic for operators, constants, and function calls (extracted from `emit_walker`).
4.  **`rxcp_emit_flow.c`**: Logic for `IF`, `DO`, `LOOP`, `LEAVE`, and `ITERATE`.
5.  **`rxcp_emit_proc.c`**: Logic for `PROCEDURE`, `PROGRAM_FILE`, and `REXX_UNIVERSE`.
6.  **`rxcp_emit_meta.c`**: Metadata and symbol-related emission functions.

## 5. Risk Registry

| Risk | Description | Mitigation |
| :--- | :--- | :--- |
| **Memory Leak** | `f_output` only frees a single fragment node, failing to traverse the `after` chain. | Update `f_output` to recursively free or iterate the chain. |
| **Validator Coupling** | Emitter depends on `target_type` and other fields being correctly set by the Validator. | Add assertions or validation checks in Emitter "in" passes. |
| **Manual Formatting** | Hardcoded `mprintf` strings for assembly templates are brittle. | Introduce a set of instruction-building macros or functions. |
| **Label Collisions** | Relies on `node_number` and suffix conventions. | Formalize label generation into a dedicated utility. |
| **Duplication** | Operator emission is duplicated for constant vs. register cases. | Refactor into a unified `emit_op(op, target, left, right)` helper. |
