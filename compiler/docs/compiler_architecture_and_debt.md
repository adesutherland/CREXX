# cREXX Compiler Architecture and Technical Debt Analysis

## 1. High-Level Flow
The cREXX compiler (`rxc`) follows a traditional multi-pass architecture, transforming Rexx source code into cREXX Assembly (`.rxas`).

1.  **Initialization**: `rxcmain` (in `rxcpmain.c`) parses command-line arguments and initializes the `Context`.
2.  **Options Parsing**: A preliminary scan (`rxcposcn.re` / `rxcpopgr.y`) identifies the Rexx language level (e.g., Level B) and other options.
3.  **Lexical Analysis & Parsing**:
    *   For Level B, the compiler uses `rxcpbscn.re` (re2c) and `rxcpbgmr.y` (Lemon).
    *   The parser builds an Abstract Syntax Tree (AST) using structures defined in `rxcpmain.h`.
4.  **Semantic Validation**: `rxcpbval.c` performs type checking, scope validation, and semantic analysis on the AST.
5.  **Optimization**: `rxcp_opt.c` applies AST-level optimizations (e.g., constant folding, dead code elimination).
6.  **Code Emission**: `rxcpemit.c` walks the AST and generates `.rxas` assembly instructions.
7.  **Assembly (External)**: The resulting `.rxas` file is typically passed to `rxas` to produce the binary `.rxbin`.

---

## 2. File Map (Compiler Source)

| File | Description |
| :--- | :--- |
| `rxcpmain.c` | CLI entry point, high-level orchestration, and argument handling. |
| `rxcpmain.h` | **Monolithic Header**. Contains all core data structures (ASTNode, Token, Symbol, Scope, Context). |
| `rxcpast.c` | **AST Monolith**. Functions for AST node creation, walking, printing, and utility checks (`nodeis`). |
| `rxcpsymb.c` | Symbol table and Scope management. Uses AVL trees for symbol lookups. |
| `rxcpemit.c` | **Emitter Monolith**. Translates AST nodes into RXAS instructions. |
| `rxcpbval.c` | **Validator Monolith**. Complex semantic validation and type inference logic for Level B. |
| `rxcp_opt.c` | AST-level optimization passes. |
| `rxcpbscn.re` | re2c source for the Level B scanner. |
| `rxcpbgmr.y` | Lemon grammar for the Level B parser. |
| `rxcpopgr.y` | Lemon grammar for the OPTIONS pass. |
| `rxcposcn.re` | re2c source for the OPTIONS scanner. |

---

## 3. Modularization Plan (Proposed Seams)

To reduce technical debt and improve maintainability, the following refactoring is proposed:

### 3.1 Header Decomposition
`rxcpmain.h` should be decomposed into:
*   `rxcp_ast.h`: `ASTNode`, `Token`, and AST-related enums.
*   `rxcp_sym.h`: `Symbol`, `Scope`, and Symbol Table definitions.
*   `rxcp_ctx.h`: The `Context` structure.
*   `rxcp_type.h`: `ValueType` and related type-system definitions.

### 3.2 Logic Extraction
*   **AST Utilities**: Split `rxcpast.c` into `rxcpast_core.c` (creation/deletion), `rxcpast_walk.c` (tree traversal), and `rxcpast_print.c` (debugging).
*   **Validation**: Break `rxcpbval.c` into `rxcp_val_expr.c` (expression checking), `rxcp_val_stmt.c` (statement checking), and `rxcp_type_inference.c`.
*   **Emission**: Break `rxcpemit.c` into `rxcp_emit_core.c` and specialized emitters for procedures, control flow, and expressions.
*   **Common Utilities**: Create `rxcp_util.c/h` for string handling, error reporting, and memory management.

---

## 4. Symbol Limit Audit (8-Character Legacy)

The 8-character symbol limit is a legacy constraint rooted in mainframe compatibility (S/370) and older Rexx standards.

### 4.1 Identified Constraints
*   **Truncation in `rxcpast.c`**: The functions `nodeis()` and `tokenis()` currently truncate comparisons to **14 characters** (likely an earlier expansion from 8).
*   **Plugin Limits**: `lib/plugins/recv390/recv390.c` contains several hardcoded `char[8]` and `char[9]` arrays for member names and node IDs, reflecting mainframe constraints.
*   **Magic Numbers**: Usage of `9` (8 + null) in various Promotion tables and hex-bin buffers throughout `rxcpast.c`, `rxcpbval.c`, and `rxcpemit.c`.

### 4.2 Blast Radius of Expansion
*   **Internal Structures**: `Symbol` and `Token` already use dynamically allocated `char*` names, so they are mostly ready for expansion.
*   **Binary Format (`.rxbin`)**: The format already supports variable-length strings for procedure names and constants.
*   **Assembler (`rxas`)**: The assembler uses variable-length ID tokens, so it does not impose a hard 8-character limit.
*   **Target Specifics**: For S/370 targets, external symbols might still need truncation to 8 characters during emission, which should be handled as a backend constraint rather than a frontend limit.

---

## 5. Issue Log (Code Smells)

| Severity | Issue | Description |
| :--- | :--- | :--- |
| **High** | Monolithic Files | `rxcpast.c` (>3k lines) and `rxcpemit.c` (>3.7k lines) are difficult to navigate and maintain. |
| **High** | Static Globals | Extensive use of static globals for state management in `rxcpemit.c` and `rxcpbval.c` prevents reentrancy. |
| **Medium** | Error Handling | Inconsistent error reporting; often relies on setting flags in `Context` without immediately returning or using structured error objects. |
| **Medium** | Magic Numbers | Hardcoded limits (like the 14-char limit in `nodeis`) and array sizes (e.g., `promotion[9][9]`). |
| **Low** | Memory Management | Manual `malloc`/`free` throughout the code without a clear ownership model or pool allocation for AST nodes. |
