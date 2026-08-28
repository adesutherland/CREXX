# cREXX Emitter Architecture

This document describes the architecture and internal logic of the modular code generator for the cREXX compiler (`rxc`), which is split into functional modules.

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
    - **Finalization**: The root node (`REXX_UNIVERSE`) calls `print_output()` during its `out` phase. Finalization flattens the fragment chain, applies compiler-owned exact-template combinations, and writes the resulting assembly to the output file.

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
| `node->deferred_register_mark` | `ASTNode` | The deferred-register boundary captured before a statement's descendants are allocated. |
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
- **Expression Blocks**: `BLOCK_EXPR` emits its enclosed statement block and then yields the register allocated for the block result.
- **Key Functions**: `type_promotion`, `format_constant`, `type_to_prefix`.

### Control Flow (`rxcp_emit_flow.c`)
- **Functionality**: Generating branch logic and labels for `IF`, `DO`, `LOOP`, `LEAVE`, `LEAVE_WITH`, `ITERATE`, and `SELECT`.
- **Labeling**: Uses `node->node_number` combined with suffixes (e.g., `l123dostart`, `l123iffalse`).
- **Block Exit Convention**: `LEAVE_WITH` copies its expression result into the parent `BLOCK_EXPR` register and branches to `l%dbexprend`.

### Procedures & Program Structure (`rxcp_emit_proc.c`)
- **Functionality**: Logic for `PROCEDURE`, `PROGRAM_FILE`, and `REXX_UNIVERSE`.
- **Key Functions**: `add_scope_initiators`.

### Metadata & Symbols (`rxcp_emit_meta.c`)
- **Functionality**: Emitting `.meta` directives and symbol-related metadata.
- **Key Functions**: `meta_set_symbol`, `add_global_variable_metadata`, `meta_narg`.

### Compiler-owned large instructions (`rxcp_emit_super.c`)

- **Functionality**: Combines exact generated templates whose intermediate
  registers, aliases, cleanup, or overwritten side effects are known to be
  compiler-owned. These are the NR-09 Class 2 transformations that RXAS cannot
  safely infer from arbitrary authored assembly.
- **Boundary**: This pass runs only over rxc's completed output. Source-step
  directives and labels are hard barriers. Metadata and trace directives may
  remain between component instructions only where the combiner preserves
  their ordering and retargets a trace reference when the removed temporary
  instruction was its subject.
- **Typed-input rule**: A combination is selected from the actual parsed
  emitted mnemonic and operands, not from an AST node's declared type,
  provenance, target history, or expected lowering. For example, alias cleanup
  fusion requires an emitted `icopy`; integer provenance is not proof that the
  completed output contains an integer copy. This boundary prevents string
  conversion paths such as C2d/X2d from being rewritten as integer operations.
- **Preference**: Emit a large instruction directly in the owning AST-node
  path only when that node owns the complete semantic unit and the exact
  emitted operation type is already fixed. Alias/copy/cleanup combinations
  remain in the final typed-instruction combiner. The combiner is also the
  bounded fallback for generated templates that span emitter fragments or
  adjacent AST nodes.
- **Non-goal**: This is not a general assembly peephole. Effect-clean Class 1
  sequences remain RXAS-owned backstops, so authored assembly receives the
  same safe optimization without relying on compiler provenance.
- **Key Function**: `rxcp_combine_superinstructions`.

## 4. Register Allocation (`rxcp_emit_reg.c`)

The register allocation logic is isolated in `rxcp_emit_reg.c`. It performs the first pass over the AST to ensure every node that requires a virtual register has one assigned before code emission begins.

## 4.1 Argument Copy Semantics

The VM calling convention is register-by-reference for all procedure calls. User-visible pass-by-value is therefore implemented by the compiler and emitter, not by the VM itself.

Current rules:

- `.ref` / `ARG expose ...` formals alias the incoming argument register and must be able to update the caller-visible value.
- Plain by-value formals must preserve caller-visible state if the callee writes to the formal.
- `mark_const_args()` in [`compiler/rxcp_opt.c`](/Users/adrian/CLionProjects/CREXX/compiler/rxcp_opt.c) marks by-value formals as `is_const_arg` when the formal symbol is provably read-only inside the callee. Those formals may safely share the incoming register in both `-n` and optimised builds.
- Writable by-value formals still require an isolated local register. The emitter materialises that copy in the `ARG` case in [`compiler/rxcpemit.c`](/Users/adrian/CLionProjects/CREXX/compiler/rxcpemit.c).
- Call ABI flags are centralized in [`binutils/include/rxflags.h`](/Users/adrian/CLionProjects/CREXX/binutils/include/rxflags.h). `REGTP_VAL` is `0x00000100`, and `REGTP_NOTSYM` is `0x00000200`; the low byte is reserved for VM-private readable status.
- For large values (strings, arrays, objects, binaries), `REGTP_NOTSYM` means the actual argument is not backed by a caller-visible symbol. In that case the callee may reuse or swap the incoming register instead of copying, because there is no caller binding to preserve.
- For small values (`.int`, `.float`, booleans), the emitter always copies writable by-value formals. That is an intentional cost tradeoff: copying is cheaper than propagating and checking a "non-symbol temporary" flag for those types.

This is semantic copy elision, not a change in language semantics. Any optimisation is valid only if the caller still observes pass-by-value behaviour.

## 4.2 Direct Call Lowering

The emitter has two direct-bytecode call paths with the same callee-visible
contract:

- a locally defined bytecode procedure, method, factory, or match with one to
  four actuals may use `CALL1` through `CALL4`, naming each actual register
  explicitly;
- the existing two-operand `CALL` remains the zero-argument form; and
- imported/native, dynamic/interface-selected, higher-arity, and unsupported
  repeated-status sites retain the counted contiguous-window path.

The fixed forms capture the named caller value pointers before frame
activation and bind them as the ordinary callee `a1...aN` registers. Procedure
code therefore cannot determine which call form entered it and must continue
to rely only on the established argument-register and status contract.
`REGTP_VAL` and `REGTP_NOTSYM` setup remains required and is emitted as
standalone `SETTP` work for fixed calls. If repeated actuals share one physical
register but need independent per-formal status, the emitter falls back to the
counted path and its snapshots. Fixed calls do not change NR-06 register
affinity, frame allocation/recycling, pass-by-value isolation, `.ref`, optional
argument, signal, or return semantics.

The serialized forms require RXBIN 007 feature bit
`RXBIN007_FEATURE_FIXED_CALLS`. RXAS/RXLINK derive that bit from the emitted
instruction stream, and readers reject a fixed-call opcode without it. See
`docs/ai-context/RXAS_ASSEMBLER.md`,
`docs/ai-context/RXBIN_007_SEMANTIC_GRAPH.md`, and
`docs/ai-context/RXVM_INTERPRETER.md` for the assembler, format, and runtime
contracts.

## 4.3 Nested Deferred-Register Lifetimes

Indexed and property reads can leave registers linked until the owning
statement emits its cleanup. A nested statement must therefore not return an
enclosing condition's or assignment target's deferred registers to the free
pool. The register walker records the current deferred-register boundary on
entry to every statement and, on exit, releases only registers added after
that boundary. The enclosing statement retains its prefix until its own exit.

This is an ownership rule, not a register-pressure preference. Reusing an
enclosing linked register can make an ordinary local assignment or call result
write through the link and corrupt attribute storage before `unlink` runs.
`deferred_register_outer_if_lifetime` covers direct outer-`IF`, typed-loop, and
loop-plus-`IF` shapes in both no-opt and optimized modes.

## 5. Risk Registry

| Risk | Description | Mitigation |
| :--- | :--- | :--- |
| **Structural Interference** | `WARNING` or `ERROR` nodes attached to expressions would break child-count assumptions in the Emitter. | Resolved: `rxcp_collect_and_prune_diagnostics` removes all diagnostic nodes from the AST before emission. |
| **Memory Leak** | `f_output` only frees a single fragment node, failing to traverse the `after` chain. | Update `f_output` to recursively free or iterate the chain. |
| **Validator Coupling** | Emitter depends on `target_type` and other fields being correctly set by the Validator. | Add assertions or validation checks in Emitter "in" passes. |
| **Manual Formatting** | Hardcoded `mprintf` strings for assembly templates are brittle. | Resolved: Potential bridge collisions avoided by renaming interpreter symbols to `rxvm_mprintf`. |
| **Label Collisions** | Relies on `node_number` and suffix conventions. | Formalize label generation into a dedicated utility. |
| **Duplication** | Operator emission is duplicated for constant vs. register cases. | Refactor into a unified `emit_op(op, target, left, right)` helper. |
| **Register Assignment Pressure** | Discarded standalone-call results and temporary member-call receivers are explicitly recycled and regression-capped by `rxc_call_statement_register_reuse`; broader inlining and future callable-lifetime attribute locals still need a clearer register lifetime model than per-node temporaries alone. | TODO: reserve long-lived locals explicitly, keep call-frame temporaries from clobbering them, and add diagnostics explaining remaining register growth. |
| **Typed Emission Drift** | AST provenance or an expected target type can differ from the operation actually emitted after conversion. | Match large-instruction templates only against the final parsed typed mnemonic/operands; require direct AST emission sites to own and fix the complete operation. |

## 6. AST Assumptions
The Emitter operates on a "clean" AST. Following the validation finalization phase, the tree is guaranteed to have:
*   All symbols resolved to their final definitions (no `UNRESOLVED` symbols remain).
*   All diagnostic nodes (`WARNING`, `ERROR`) removed and stashed in the `Context`.
*   All high-level system instructions (like `ADDRESS` or `EXIT`) rewritten to internal calls.
