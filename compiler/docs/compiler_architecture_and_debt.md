# cREXX Compiler Architecture and Technical Debt Analysis

## 1. High-Level Flow
The cREXX compiler (`rxc`) follows a traditional multi-pass architecture, transforming Rexx source code into cREXX Assembly (`.rxas`).

1.  **Initialization**: `rxcmain` (in `rxcpmain.c`) parses command-line arguments and initializes the `Context`.
2.  **Options Parsing**: A preliminary scan (`rxcposcn.re` / `rxcpopgr.y`) identifies the Rexx language level (e.g., Level B) and other options.
3.  **Lexical Analysis & Parsing**:
    *   For Level B, the compiler uses `rxcpbscn.re` (re2c) and `rxcpbgmr.y` (Lemon).
    *   The parser builds an Abstract Syntax Tree (AST) using structures defined in `rxcp_ast.h`.
4.  **Semantic Validation**: `rxcp_val_orch.c` orchestrates a multi-pass pipeline (check, sym, type, trans) to validate the AST.
5.  **Optimization**: `rxcp_opt.c` applies AST-level optimizations (e.g., constant folding, dead code elimination).
6.  **Code Emission**: The modular emitter (`rxcp_emit_*.c`) walks the AST and generates `.rxas` assembly instructions.
7.  **Assembly (External)**: The resulting `.rxas` file is typically passed to `rxas` to produce the binary `.rxbin`.

---

## 2. File Map (Compiler Source)

| File | Description |
| :--- | :--- |
| `rxcpmain.c` | CLI entry point, high-level orchestration, and argument handling. |
| `rxcp_ast.h` | AST Node definitions and related enums. |
| `rxcp_sym.h` | Symbol table and Scope management. |
| `rxcp_types.h` | Type-system definitions (`ValueType`). |
| `rxcp_val_*.c` | Validation pipeline (check, sym, type, trans, orch). See [Bridge Plugins](bridge_plugins.md). |
| `rxcp_emit_*.c` | Modular Emitter (core, reg, expr, flow, proc, meta). |
| `rxcp_opt.c` | AST-level optimization passes. |
| `rxcpbscn.re` | re2c source for the Level B scanner. |
| `rxcpbgmr.y` | Lemon grammar for the Level B parser. |
| `rxcp_diag_fallback.c` | Structural fallback diagnoser used only when parsing fails to produce an AST. |

---

## 3. Modular Structure (Resolved)

The previously identified technical debt regarding monolithic files has been **RESOLVED**. The compiler logic is now split into the following module categories:

### 3.1 AST & Symbols
*   `rxcp_ast.h`: Defines the `ASTNode` structure and AST-related constants.
*   `rxcp_sym.h`: Defines `Symbol`, `Scope`, and Symbol Table management.
    *   **Scope Hierarchy**: Explicitly typed scopes (`SCOPE_UNIVERSE`, `SCOPE_NAMESPACE`, `SCOPE_CLASS`, `SCOPE_PROCEDURE`, `SCOPE_LOCAL`).
    *   **Specialized Resolvers**: Tiered symbol resolution (`sym_rslv_tiered`) using specialized helpers for Local, Attribute, and Global resolution to ensure predictable linkage.
*   `rxcp_types.h`: Centralized `ValueType` definitions.

### 3.2 Validation Pipeline
The semantic validation logic (formerly `rxcpbval.c`) is now split into a multi-pass pipeline:
*   `rxcp_val_orch.c`: The orchestrator that runs the pipeline.
*   `rxcp_val_check.c`: Basic structural and syntax checks.
*   `rxcp_val_sym.c`: Symbol resolution and scope population.
*   `rxcp_val_type.c`: Type inference and type checking.
*   `rxcp_val_trans.c`: AST transformations (e.g., adding implicit conversions).

**Architectural Rule: Idempotency**
Every walker in the fixpoint loop must be **Idempotent**. This ensures that the compiler converges to a stable state regardless of how many times it processes the tree, which is critical for supporting code injection and interdependent symbol resolution.

### 3.3 Modular Emitter
The code generator (formerly `rxcpemit.c`) is now split into 6 functional modules:
*   `rxcp_emit_core.c`: Infrastructure, I/O, and conversion matrices.
*   `rxcp_emit_reg.c`: Register allocation and virtual register management.
*   `rxcp_emit_expr.c`: Emission logic for math, strings, and operators.
*   `rxcp_emit_flow.c`: Control flow (IF, DO, SELECT).
*   `rxcp_emit_proc.c`: Procedures and program-level structure.
*   `rxcp_emit_meta.c`: Metadata generation.

---

## 4. Symbol Limit Audit (8-Character Legacy)

The 8-character symbol limit is a legacy constraint rooted in mainframe compatibility (S/370) and older Rexx standards.

### 4.1 Identified Constraints
*   **Truncation in AST Utilities**: Functions in the AST modular files (formerly `rxcpast.c`) currently truncate comparisons to **14 characters** (likely an earlier expansion from 8).
*   **Plugin Limits**: `lib/plugins/recv390/recv390.c` contains several hardcoded `char[8]` and `char[9]` arrays for member names and node IDs, reflecting mainframe constraints.
*   **Magic Numbers**: Usage of `9` (8 + null) in various Promotion tables and hex-bin buffers throughout the modularized Validator and Emitter files.

### 4.2 Blast Radius of Expansion
*   **Internal Structures**: `Symbol` and `Token` already use dynamically allocated `char*` names, so they are mostly ready for expansion.
*   **Binary Format (`.rxbin`)**: The format already supports variable-length strings for procedure names and constants.
*   **Assembler (`rxas`)**: The assembler uses variable-length ID tokens, so it does not impose a hard 8-character limit.
*   **Target Specifics**: For S/370 targets, external symbols might still need truncation to 8 characters during emission, which should be handled as a backend constraint rather than a frontend limit.

---

## 5. Issue Log & Future Work

| Severity | Issue | Description | Status |
| :--- | :--- | :--- | :--- |
| **High** | Monolithic Files | `rxcpast.c` and `rxcpemit.c` were too large. | **RESOLVED** |
| **High** | Optimizer Refactor | `rxcp_opt.c` needs refactoring to support new decimal types. | Pending |
| **Medium** | Plugin Interface | Implement "Analyze-Consult-Negotiate" loop in `rxcp_val_orch.c`. | **RESOLVED** |
| **Medium** | Static Globals | Extensive use of static globals prevents reentrancy. | In Progress |
| **Low** | Memory Management | Lack of clear ownership model or pool allocation for AST nodes. | Open |
| **Low** | Bridge Overhead | Compiler now links with `rxvml`, increasing binary size and complexity. | Open |

### 5.1 Parser Recovery Strategy
The compiler now has an explicit escape hatch for grammar fragility:

*   Keep **working Lemon recovery rules** when they preserve a useful partial AST and allow later passes to continue producing errors.
*   Use the **fallback diagnoser** only for terminal parse failures where `context->ast` was never produced.
*   Do **not** use the fallback diagnoser as a general substitute for grammar design. It is structural and token-driven by design, and should remain limited to unmatched terminators and misplaced structural keywords.

This gives future parser refactors a clear rule:

*   If a recovery production is helping AST continuity, keep it.
*   If a recovery production exists only to emit a top-level syntax message and is destabilizing the grammar, consider removing it and letting `rxcp_diag_fallback.c` handle the terminal failure instead.

At present, the intended use is conservative: preserve current working grammar recovery and use the fallback only to replace "Failure to create AST" with standard compiler diagnostics.
