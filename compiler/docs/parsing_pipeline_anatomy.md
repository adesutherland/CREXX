# cREXX Compiler Parsing Pipeline Anatomy & Syntax Report

## 1. Introduction
This document details the internal anatomy of the cREXX compiler's front-end parsing pipeline. It analyzes the flow of source code from raw text to the Abstract Syntax Tree (AST), identifying how syntax rules are enforced at each stage: Lexing, Glue (Parser Wrapper), Parsing, and the initial AST Walker.

## 2. Initialization: Language Level Detection
Before the main compilation pipeline begins, the compiler performs a lightweight "pre-parse" to determine the Rexx language level (e.g., Level B, Level G, Level C). This is crucial because different levels may enable different syntax or processing rules.

*   **Mechanism**: The function `opt_pars` (in `compiler/rxcpopar.c`) drives this process.
*   **Components**:
    *   **Scanner**: `compiler/rxcposcn.re` (re2c). It scans for the `OPTIONS` keyword and level tokens (e.g., `LEVELB`), while skipping comments and whitespace. It stops scanning upon encountering any token that is not a valid option.
    *   **Parser**: `compiler/rxcpopgr.y` (Lemon). A simplified grammar that only recognizes the `OPTIONS` instruction.
*   **Outcome**: The process sets the `context->level` (defaults to Level C if unspecified) and `context->numeric_standard`. Once the options are processed or a non-option token is found, the context is reset (tokens freed, buffer rewound), and the main pipeline (`rexbpars`) begins.

## 3. Pipeline Anatomy

### 3.1 Lexical Analysis (Scanner)
**Source**: `compiler/rxcpbscn.re` (re2c)

The scanner converts the character stream into tokens.

*   **Comment Handling**:
    *   **Block Comments (`/* ... */`)**: Handled internally by the lexer using a recursion depth counter. They are completely consumed and **ignored**; no tokens are emitted to the parser.
    *   **Line Comments (`#`, `//`, `--`)**: The lexer consumes characters up to the end of the line. Crucially, it emits a `TK_EOL` (End of Line) token upon finishing the comment.
*   **Whitespace**: Spaces and tabs are ignored and skipped.
*   **String Literals**: Supports both single (`'`) and double (`"`) quotes.
*   **Numbers**: Distinguishes between `INTEGER`, `FLOAT`, and `DECIMAL` at the lexical level.
*   **Stems**: Complex regex rules handle compound variables (e.g., `stem.index`).

### 3.2 Glue Layer (Parser Wrapper)
**Source**: `compiler/rxcpbpar.c` (`rexbpars` function)

The Glue Layer sits between the Scanner and the Parser, transforming the raw token stream before it reaches the grammar. This is where "Rexx-isms" like implicit semicolons are handled.

*   **Significant Newline Transformation**: The parser does not understand `TK_EOL`. The glue layer intercepts `TK_EOL` tokens and converts them into `TK_EOC` (End of Clause/Semicolon) tokens.
    *   *Rule*: `if (token->token_type == TK_EOL) token->token_type = TK_EOC;`
*   **Line Continuation**: Implements the Rexx line continuation rule.
    *   *Rule*: If a `TK_COMMA` is immediately followed by `TK_EOL`, **both** tokens are discarded. This merges the physical lines into a single logical clause.
*   **Terminator Collapsing**: Consecutive `TK_EOC` tokens are effectively ignored/collapsed to prevent "empty statement" noise in the parser (implemented via loop continuation).
*   **Stream Termination**: Guarantees that the parser always receives a clean shutdown sequence (`TK_EOC` followed by `TK_EOS`), injecting `TK_EOC` if the source file lacks a final newline.

### 3.3 Syntactic Analysis (Parser)
**Source**: `compiler/rxcpbgmr.y` (Lemon)

The parser defines the structural grammar of the language.

*   **Instruction Termination**:
    *   The grammar enforces that every instruction must be terminated by `TK_EOC`.
    *   Because of the Glue Layer, this `TK_EOC` can come from either a literal `;` in the source code OR a newline.
    *   *Implication*: `if a then say 'b' else say 'c'` (single line) is **invalid** because `say 'b'` is an instruction that consumes tokens until it hits a `TK_EOC`. It encounters `else` instead, causing a syntax error.
*   **Raw AST Construction**:
    *   AST nodes are created immediately in the reduction actions.
    *   At this stage, the AST is relatively flat; procedure bodies are not yet nested under their procedure nodes.
    *   *Example*: `if(I) ::= TK_IF ... else(F). { I = ast_f(...); add_ast(I, F); }`

#### 3.3.1 Unfinished vs. Hierarchical AST
It is critical to understand that the AST produced by the Lemon parser is **unfinished**. Due to the single-lookahead nature of the grammar, some relationships (especially procedure and class bodies) cannot be established during initial parsing. The compiler relies on subsequent "fixup" walkers to restructure the flat list of instructions into a proper logical hierarchy. Developers must not assume the AST is hierarchical until after the fixup stage (Step 3.4).

### 3.4 Semantic Analysis & AST Stitching (Initial Walker)
**Source**: `compiler/rxcp_val_check.c` (`initial_checks_walker`)

After the parser builds the initial "raw" AST, the `initial_checks_walker` performs a critical pass to restructure and validate the tree. This is where the flat list of instructions is transformed into a true logical hierarchy.

*   **Implicit Main Procedure**: If the program does not start with a `PROCEDURE`, the walker wraps the top-level instructions into an implicit `main` procedure and determines its return type (VOID or inferred).
*   **Procedure Restructuring**: The parser sees procedures as just another instruction. The walker moves all subsequent instructions (until the next procedure) to be *children* of the procedure node, creating a proper scope and body.
*   **Declaration Hoisting**: Special instructions like `ARGS`, `DIGITS`, `FUZZ`, and `FORM` are identified and moved to the top of the procedure definition, enforcing that they appear before other executable instructions.
*   **Source Location Stitching**: Calculates precise source code ranges (line/column) for all nodes by propagating token information up from the leaves. It also "stitches" parentheses back into the source range for accurate error reporting.
*   **Loop Correction**: In `DO` loops, if an assignment is present without a `BY` clause, the walker injects an implicit `BY` to prevent infinite loops.
*   **Assembler Validation**: Validates `ASSEMBLER` blocks against the target architecture's instruction set (Level B specific).

### 3.5 Validation Orchestrator & Fixpoint Loop
**Source**: `compiler/rxcp_val_orch.c` (`validate_ast`)

The final stage of the front-end is the Validation Orchestrator. Unlike previous stages, this stage employs a **Fixpoint Iteration Loop** to handle interdependent symbol resolution and code injection.

*   **Fixed Point Iteration**: The orchestrator wraps subsequent validation passes in a `do { ... } while (context->changed_flags)` loop. The `changed_flags` context variable uses bitmasks (like `FLAG_VAL_TYPE`) to identify exactly which walker forces another iteration, which provides immediate diagnostic convergence paths upon failure.
*   **Explicit Symbol Lifecycle**: The pipeline uses a state-driven approach for symbol resolution. Every name is assigned an explicit `SymbolStatus` (e.g., `SYM_STATUS_UNRESOLVED`), allowing walkers to cleanly transition names from tentative placeholders to global imports or local variables across loop iterations.
*   **Idempotency & Stress Testing**: Every walker in the fixpoint loop is designed to be **Idempotent**. In debug mode `-d3`, the compiler forces 3 iterations and multiple calls per walker to prove this and ensure the AST and Symbol table converge to a stable state. Furthermore, a strict max iterations threshold exists (16 passes); if breached and `changed_flags` is non-zero, it triggers a hard bypass abort with the active bits logged.
*   **AST Validation**: An integrated validator (`rxcp_ast_val.c`) runs between passes (in `-d2`) to assert AST structural integrity and Symbol↔Node linkage consistency.
*   **Plugin Dispatch**: Intercepts `IMPLICIT_CMD` nodes and consults the **Bridge Plugin** (see `rxcp_val_plugin.c` and [Bridge Plugins](bridge_plugins.md)).
*   **Code Injection**: Plugins can return Rexx source strings which are parsed into AST fragments and grafted into the main tree. This sets the `changed` flag, triggering another loop iteration to resolve symbols in the new code.
*   **Rewrite Walkers**: Final transformations (like `ADDRESS` and `EXIT` rewriting) occur within the loop to ensure they interact correctly with injected code.

## 4. Syntax Rules Implemented by Step

### Step 1: Lexical Rules (Scanner)
Rules enforced on the character stream:
*   **Identifiers**: `[a-zA-Z_][a-zA-Z0-9_]*` (Case-insensitive token generation).
*   **Numeric Literals**:
    *   Integer: `[0-9]+`
    *   Float: Scientific notation or dot notation (e.g., `1.2`, `1e5`).
    *   Decimal: Numbers suffixed with `d` (e.g., `1.2d`).
*   **Operators**:
    *   Classic/Common Math: `+`, `-`, `*`, `/`, `%` (Div/Mod depends on `options levelb`).
    *   Comparison: `=`, `<>`, `\=`, `>`, `<`, etc.
    *   Logic: `&`, `|`, `\` (Not), `...` (Ellipsis).

### Step 2: Stream Rules (Glue)
Rules enforced on the token stream:
*   **Implicit Semicolons**: A newline is structurally identical to a semicolon.
*   **Continuation**: A trailing comma negates the following newline.
*   **Comment Transparency**: Comments do not exist in the syntactic view, but line comments do trigger the "End of Clause" effect via the EOL they sit on.

### Step 3: Structural Rules (Parser)
Rules enforced on the hierarchy:
*   **Program Structure**:
    1.  `OPTIONS` (Optional)
    2.  `NAMESPACE` / `IMPORT` (Optional)
    3.  `INSTRUCTIONS`
*   **Block Scoping**:
    *   `DO` ... `END`
    *   Expression-form `DO` ... `END` (`BLOCK_EXPR`)
    *   `IF` ... `THEN` ... `ELSE`
    *   `SELECT` (Not fully implemented/verified in Level B yet, but keywords exist).
*   **Procedures**:
    *   Must start with a label.
    *   Format: `Label: PROCEDURE [= Type] [EXPOSE List]`.
*   **Typing (Level B)**:
    *   Variable types are inferred or explicit in definitions (`.int`, `.float`).
    *   Arrays: `type[size]`.
*   **Dangling Else**: Resolved by associating `ELSE` with the nearest preceding `IF`.

### Step 4: Semantic & Restructuring Rules (Walker)
Rules enforced on the logic:
*   **Procedure Scope**: Instructions following a label are bound to that procedure until the next label or end of file.
*   **Argument Declaration**: `ARGS` must be the first instruction in a procedure (checked and hoisted by walker).
*   **Loop Safety**: Iterative loops with assignments must have a step value (implicit or explicit).
*   **Assembler**: Instructions must match the valid operand types defined in the architecture.
*   **Expression Blocks**: `DO ... END` in primary-expression position is parsed as a `BLOCK_EXPR`; the block yields through `LEAVE WITH expr`.

## 5. Syntax Error Detection
The compiler employs a multi-layered strategy for error detection, with each stage of the pipeline catching specific classes of errors.

### 5.1 Lexical Errors (Scanner)
The scanner identifies invalid characters or malformed tokens.
*   **Mechanism**: Fallback rules in `re2c`.
*   **Key Errors**:
    *   `TK_UNKNOWN`: Any character not matching a valid token pattern. Mapped to `BADCHAR` in the AST.
    *   `TK_BADCOMMENT`: Unterminated block comments (`/*` without matching `*/`). Mapped to `BAD_COMMENT`.

### 5.2 Syntactic Errors (Parser)
The parser enforces grammar rules. When a rule is violated, Lemon's `error` token logic triggers.
*   **Mechanism**: Specific error productions in `rxcpbgmr.y` capture invalid sequences and insert `AST_ERROR` nodes into the tree.
*   **Error Codes** (Selected):
    *   `MISSING_OPTIONS`: Program starts with invalid tokens.
    *   `EXTRANEOUS`: Unexpected tokens after a valid program.
    *   `MISSING_THEN`, `MISSING_END`, `UNEXPECTED_ELSE`, `UNEXPECTED_END`: Control flow structure violations.
    *   `INCOMPLETE_DO`: Unterminated `DO` constructs, including expression-form blocks.
    *   `BADEXPR`: Malformed expressions (e.g., consecutive operators).
    *   `BAD_NAMESPACE_SYNTAX`, `BAD_IMPORT_SYNTAX`.
    *   `INVALID_IN_ARRAY_DEF`: Invalid elements in array definitions.

### 5.3 Semantic Errors (Walker)
The walker (`initial_checks_walker`) validates logic that cannot be expressed purely in grammar (or is deferred for checking).
*   **Mechanism**: The walker traverses the AST and calls `mknd_err` to attach errors to specific nodes.
*   **Error Codes** (Selected):
    *   **Structure**: `CANT_DEFINE_PROC_HERE` (Nested procedures).
    *   **Arguments**: `REPEATED_ARG`, `ARG_NOT_FIRST_INST`, `INVALID_ARG_SYNTAX`.
    *   **Numeric Options**: `REPEATED_NUMERIC_DIGITS`, `NUMERIC_DIGITS_NOT_FIRST_INST`, `DECIMAL_DIGITS_RANGE`.
    *   **Assembly**: `ASSEMBLER_ONLY_LEVELB`, `INVALID_ASSEMBLER`.
    *   **Namespaces**: `MULTIPLE_NAMESPACE`, `DUPLICATE` (redirects).

### 5.4 Diagnostic Pruning
To protect the structural integrity of the AST for code generation, all `WARNING` and `ERROR` nodes are "pruned" from the tree during the finalization pass (`rxcp_collect_and_prune_diagnostics`). They are moved to a detached list in the `Context`, ensuring that the Emitter only sees a pure instruction tree while still allowing the compiler to report all findings.

This approach allows the compiler to continue processing (in some cases) or provide precise feedback by localizing the error to the specific AST node.

## 6. Status - Lemon Grammar Conflict Analysis
The Lemon grammar (`rxcpbgmr.y`) was analyzed for parsing conflicts.
*   **Shift/Reduce Conflicts**: 0
*   **Reduce/Reduce Conflicts**: 0
*   **Status**: The grammar is **conflict-free**.
*   **Resolution Mechanism**: The potential ambiguity in `IF/THEN/ELSE` structures is resolved explicitly using precedence directives:
*   **Special Case**: Expression-form `DO` introduces a statement/expression ambiguity at command start. The grammar resolves this with a restricted `command_expression` entry point for bare commands while preserving `BLOCK_EXPR` in the normal expression grammar.
    ```yacc
    %nonassoc TK_IF.
    %nonassoc TK_ELSE.
    ```
