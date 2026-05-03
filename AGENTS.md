# Repository Instructions

`AGENTS.md` is the canonical repository instruction file. Keep repository-specific agent guidance here. `GEMINI.md` should only refer back to this file so instructions do not drift.

## Project Context

`crexx` is a custom REXX-to-bytecode toolchain with four main CMake-built binaries:

1. `rxc`: compiler from REXX source to `rxas` assembly
2. `rxas`: assembler from `rxas` assembly to `rxbin` bytecode
3. `rxlink`: linker for combining one or more `rxbin` modules into a shared-pool linked image
4. `rxvm`: interpreter for register-based `rxbin` bytecode

## Core Knowledge Sources

Before changing compiler logic or making claims about syntax, AST shape, validation, or runtime behaviour, consult the relevant project docs instead of guessing:

- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/ai-context/CREXX_DEBUGGING.md`
- `docs/ai-context/CREXX_LEVELB_AUTHORING.md`
- `docs/books/crexx_language_reference/classes_and_interfaces.md`
- `docs/books/crexx_language_reference/data_types.md`
- `docs/books/crexx_language_reference/statements.md`
- `docs/ai-context/RXAS_ASSEMBLER.md`
- `docs/ai-context/RXLINK_LINKER.md`
- `docs/ai-context/RXVM_INTERPRETER.md`
- `docs/ai-context/CREXX_LIBS.md`

Read only what is needed for the task, but do not rely on memory for cREXX syntax or compiler internals when the docs cover it.

For tasks that write or edit Level B `.rexx`, start with `docs/ai-context/CREXX_LEVELB_AUTHORING.md` and copy patterns from the referenced repo examples instead of inventing syntax from generic training data.

For ADDRESS environment work, `docs/ai-context/RXVM_INTERPRETER.md` is the current protocol reference. The pre-release command-only native callback registration form has been retired; use the current environment object/function protocol and `rxvml_address_register_callback_environment(ctx, name, id, command_cb, function_cb, userdata)`. ADDRESS host-variable anchors (`:name` and `${name}`) are compiler auto-expose syntax only; their command meaning belongs to the selected environment handler.

## Working Rules

- For tasks that change compiler logic, syntax, scoping, or architecture, present a numbered implementation plan before editing.
- Pause for user approval before making language-design decisions, syntax changes, or architectural shifts. The user is the final authority on language direction.
- For complex bugs or crashes, start with a minimal reproducer in cREXX where practical before changing core C code.
- Run focused tests frequently during compiler work. If a change causes regressions, stop, report them clearly, and distinguish expected from unintended fallout.
- Keep documentation in sync with code. If you uncover important undocumented behaviour or architecture, update the relevant docs as part of the change.

## Debugging Output Discipline

- When debugging anything that can emit large or unbounded output, redirect both stdout and stderr to a temp log file created with `mktemp`, then inspect that file with focused reads such as `tail`, `sed`, or `grep`.
- Apply the temp-log workflow to verbose builds, parser-mode sessions, tracing, fixed-point loops, and any command that might otherwise flood the terminal or crash the agent session.
- Treat `rxc -d*`, parser traces, fixed-point validation traces, and syntax-highlighting debug runs as mandatory temp-log cases: do not stream their raw output directly to the terminal.
- Keep the terminal-facing output short: print the temp log path, then read back only the relevant slices needed for diagnosis.
