# cREXX Emitter Architecture

This document describes the architecture and internal logic of the modular code generator for the cREXX compiler (`rxc`), which is split into six functional modules.

## 1. High-Level Execution Flow

The emission process transforms the validated Abstract Syntax Tree (AST) into cREXX Assembly (`.rxas`). It is initiated by the `emit()` function (in `rxcp_emit_core.c`) and consists of two primary passes using the standard `ast_wlkr` mechanism.

### Pass 1: Register Assignment (`rxcp_emit_reg.c`)
- **Walker**: `register_walker`
- **Direction**: Both `in` (Top-Down) and `out` (Bottom-Up).
- **Purpose**: Assigns virtual registers to AST nodes and symbols.
- **Logic**: 
    - Determines if a node can reuse a symbol's register or needs a temporary one.
    - Implements "don't assign" optimization (`DONT_ASSIGN_REGISTER`) to avoid redundant copy instructions for constant-to-variable assignments.
    - Handles register pressure by interacting with the `scope` (via `get_regs`).

### Pass 2: Code Emission (Modular Walkers)
- **Walker**: `emit_walker` (Orchestrated by `rxcp_emit_core.c`, implemented across `rxcp_emit_*.c`).
- **Direction**: Both `in` (Top-Down) and `out` (Bottom-Up).
- **Purpose**: Generates assembly text fragments.
- **Logic**:
    - **Top-Down (`in`)**: Used for block-level initialization (e.g., `INSTRUCTIONS` node triggers `add_scope_initiators` in `rxcp_emit_proc.c`).
    - **Bottom-Up (`out`)**: Each node type generates its corresponding assembly by concatenating fragments from its children and adding its own instructions.
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

## 3. Logic Categorization (Modular Structure)

### Core & Marshalling (`rxcp_emit_core.c`)
- **Functionality**: Linked list management for `OutputFragment`, file I/O, and the `emit_promotion` matrix.
- **Key Functions**: `output_concat`, `output_append_text`, `print_output`, `emit`.

### Expressions & Operators (`rxcp_emit_expr.c`)
- **Functionality**: Handling binary/unary operators, constants, and function calls. 
- **Complexity**: High. Significant code handles combinations of constant vs. register operands.
- **Optimization**: Implements "don't assign" register optimization to reduce assembly `copy` instructions.
- **Key Functions**: `type_promotion`, `format_constant`, `type_to_prefix`.

### Control Flow (`rxcp_emit_flow.c`)
- **Functionality**: Generating branch logic and labels for `IF`, `DO`, `LOOP`, `LEAVE`, `ITERATE`, and `SELECT`.
- **Labeling**: Uses `node->node_number` combined with suffixes (e.g., `l123dostart`, `l123iffalse`).

### Procedures & Program Structure (`rxcp_emit_proc.c`)
- **Functionality**: Logic for `PROCEDURE`, `PROGRAM_FILE`, and `REXX_UNIVERSE`.
- **Key Functions**: `add_scope_initiators`.

### Metadata & Symbols (`rxcp_emit_meta.c`)
- **Functionality**: Emitting `.meta` directives and symbol-related metadata.
- **Key Functions**: `meta_set_symbol`, `add_global_variable_metadata`, `meta_narg`.

## 4. Register Allocation (`rxcp_emit_reg.c`)

The register allocation logic is isolated in `rxcp_emit_reg.c`. It performs the first pass over the AST to ensure every node that requires a virtual register has one assigned before code emission begins.

## 5. Risk Registry

| Risk | Description | Mitigation |
| :--- | :--- | :--- |
| **Memory Leak** | `f_output` only frees a single fragment node, failing to traverse the `after` chain. | Update `f_output` to recursively free or iterate the chain. |
| **Validator Coupling** | Emitter depends on `target_type` and other fields being correctly set by the Validator. | Add assertions or validation checks in Emitter "in" passes. |
| **Manual Formatting** | Hardcoded `mprintf` strings for assembly templates are brittle. | Introduce a set of instruction-building macros or functions. |
| **Label Collisions** | Relies on `node_number` and suffix conventions. | Formalize label generation into a dedicated utility. |
| **Duplication** | Operator emission is duplicated for constant vs. register cases. | Refactor into a unified `emit_op(op, target, left, right)` helper. |
