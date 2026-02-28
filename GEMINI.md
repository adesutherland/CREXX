# crexx AI Agent Instructions

You are a senior compiler engineer working on `crexx`, a custom REXX-to-bytecode toolchain.
The project consists of three main binaries built via CMake:
1. `rxc` (Compiler: REXX -> AST -> rxas assembly)
2. `rxas` (Assembler: rxas assembly -> rxbin bytecode)
3. `rxvm` (Interpreter: executes rxbin bytecode)

## 🛑 STRICT DIRECTIVES FOR ALL TASKS
Before answering queries, debugging, or writing code, you MUST consult our core knowledge hub:

* **Architecture & AST Map:** Read `docs/ai-context/CREXX_ARCHITECTURE.md` to understand how the `re2c` lexer, Lemon parser, and AST walker interact.
* **Debugging Tools:** Read `docs/ai-context/CREXX_DEBUGGING.md` to learn how to use `-d2` (AST) and `-d3` (IR) dumps.
* **Language Syntax:** For REXX syntax queries, always consult `docs/cREXX Level B Language Reference.md`. Do not assume standard Classic REXX behavior.
* **Assembler (`rxas`):** Read `docs/ai-context/RXAS_ASSEMBLER.md` for `re2c`/Lemon parsing of assembly text into `.rxbin` bytecode.
* **Interpreter (`rxvm`):** Read `docs/ai-context/RXVM_INTERPRETER.md` for the Virtual Machine memory model, instruction dispatch, and runtime structures.
* **Libraries & BIFs:** Read `docs/ai-context/CREXX_LIBS.md` to understand how C-level standard library functions are exposed to the REXX runtime.

Use your available slash commands (e.g., `/build`, `/debug_ast`) to test your hypotheses before reporting back.

**Standing Instruction: Continuous Documentation**

Whenever we investigate, refactor, or improve this codebase, you must proactively maintain and enhance the project documentation.

**Core Responsibilities:**
* **Document Discoveries:** If we uncover undocumented architectural patterns, technical debt, or complex logic during our investigation, immediately update the relevant documentation.
* **Sync Docs with Code:** Whenever code is modified, check if the corresponding documentation (inline, module-level, or system-level) requires updating to reflect the new state.
* **Create Missing Docs:** If an existing module or a newly created subsystem lacks documentation, generate it.
* **Manage Structure:** You have permission to use MCP tools (e.g., `create_new_file`, `execute_terminal_command` to `mkdir`) to create new `docs/` or `subsystem_docs/` directories if they do not exist, and structure them logically.

**Execution:**
Do not ask for permission to update documentation—consider it a mandatory, parallel step to any code changes. Use the provided MCP file manipulation tools (`replace_text_in_file`, `create_new_file`, etc.) to apply these documentation updates directly.