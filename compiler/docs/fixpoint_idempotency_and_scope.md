# Compiler Internals: Fixpoint Idempotency, Scope Hierarchy, and Symbol Resolution

This document summarizes an important change to the cREXX compiler (crexx-dev-260110): the fixpoint validation loop has been hardened for idempotency, and the scope/symbol system has been clarified and validated. It also documents new debug/validation modes and the specialized symbol resolution APIs.

## What changed and why it matters
- Every walker that runs inside the fixpoint loop is now idempotent. Re-running the loop or an individual walker produces a stable AST and Symbol table with no duplication or drift.
- A built-in AST/Symbol validator runs (under debug) between passes to catch structural and linkage issues early.
- Scopes are explicitly typed (`SCOPE_UNIVERSE`, `SCOPE_NAMESPACE`, `SCOPE_CLASS`, `SCOPE_PROCEDURE`, `SCOPE_LOCAL`) and validated; this prevents “wrongly linked symbols.”
- Block-local scoping is now supported for simple `DO ... END` groups and `IF ... THEN/ELSE` branches (confinement of variables to their lexical block).
- Name resolution now uses tiered, specialized resolvers (Local → Attribute → Global) to avoid accidental cross-scope binding.
- EXPOSE/hoisting of variables from procedures to the namespace scope is robust and idempotent.
- Import duplication preserves original scope types (procedures remain procedures, etc.), avoiding misclassification of local symbols as globals.

## Fixpoint validation loop (validate_ast)
- Location: `compiler/rxcp_val_orch.c` (`validate_ast`).
- Property: All walkers in the loop are idempotent. The `changed_flags` context variable is a bitmask used to identify exactly which walker forces another iteration, ensuring that non-converging phases can be instantly pinpointed and fixed. Under `-d3`, the compiler stress-tests by:
  - Invoking selected walkers multiple times per iteration.
  - Forcing extra loop iterations even when `context->changed_flags == 0`.
- Diagnostic Override: If the loop does hit its maximum pass limit (16 iterations), it bypasses the standard AST error logger to instantly abort via `exit(255)` while dumping the unhandled walker flags.
- Benefit: Safer future enhancements (e.g., block-scoped shadowing in DO blocks) without introducing order/iteration bugs.

## AST/Symbol validator (debug-only)
- Location: `compiler/rxcp_ast_val.c` (invoked by the orchestrator).
- Enabled via debug flags:
  - `-d2`: Runs validator checkpoints between passes.
  - `-d3`: Validator + stress testing (multiple walker invocations and forced extra iterations).
- Initial invariants enforced:
  - AST parent/child consistency; no broken parent links or cycles (detected by mismatch/sanity checks).
  - Symbol ↔ Node bi-directional linkage (each `node->symbolNode` points to a `Symbol` that, in turn, contains that connector exactly once).
  - Scope ↔ Defining-node consistency and scope tree parentage.
  - Scope hierarchy constraints (see below).

## Explicit scope hierarchy
Scope types (`ScopeType`) and their allowed parents:
- `SCOPE_UNIVERSE`: Root; no parent.
- `SCOPE_NAMESPACE`: Parent is `SCOPE_UNIVERSE` or `SCOPE_NAMESPACE`.
- `SCOPE_CLASS`: Parent is `SCOPE_NAMESPACE`.
- `SCOPE_PROCEDURE`: Parent is `SCOPE_NAMESPACE` or `SCOPE_CLASS`.
- `SCOPE_LOCAL`: Parent is `SCOPE_PROCEDURE` or `SCOPE_LOCAL` (nested code blocks).

Validator checks these relationships at `-d2` and above.

## Specialized symbol resolution APIs
To make resolution predictable and robust across passes, the compiler uses specialized resolvers (in `rxcpsymb.c`):
- `sym_rslv_local(scope, node)`: Resolve within the current routine’s locals (current `SCOPE_LOCAL` up to `SCOPE_PROCEDURE`).
- `sym_rslv_attribute(scope, node)`: Resolve class attributes in the nearest enclosing `SCOPE_CLASS`.
- `sym_rslv_global(scope, node)`: Resolve in the `SCOPE_NAMESPACE` chain up to `SCOPE_UNIVERSE`.
- `sym_rslv_tiered(scope, node, flags)`: Applies Local → Attribute → Global in order.

These replace broad, error-prone queries and are used by the walkers (e.g., `build_symbols_walker`, `resolve_functions_walker`).

## Robust EXPOSE (hoisting)
- Hoisting uses `sym_hoist_to_namespace` to promote a symbol from a procedure to its namespace scope with precondition checks and idempotence guards.
- Prevents duplicate hoisting or accidental promotion of invalid symbol kinds (e.g., arguments).
- **Auto-Exposing:** Variables defined in the top-level `namespace ... expose` statement are now automatically mapped into the local scope of all procedures in that file, meaning developers no longer need to write `PROCEDURE EXPOSE` unless they are explicitly passing variables between dynamic caller scopes.

## Import duplication fix (scope preservation)
- When importing/duplicating AST from other files (e.g., via `add_dast`), original scope types are preserved (procedures remain `SCOPE_PROCEDURE`, classes remain `SCOPE_CLASS`, etc.).
- Fixes previous misclassification that could cause local variables to appear as globals.

## Level B parser vs. initial AST fixer
The raw AST shape produced by the Lemon parser is intentionally adjusted by the initial checks walker (`initial_checks_walker`):
- Procedures are re-nested under the file/namespace; bodies are attached to their procedure nodes.
- Special instructions (e.g., `ARGS`) are hoisted.
- Minor normalizations (e.g., `SCONCAT` → `CONCAT`) are applied.

Code comments have been added in the walkers to clarify this distinction and avoid confusion when reading ASTs before and after fixups.

## Developer workflow and debug usage
- Validate with structure checks:
  ```bash
  ./rxc -d2 -o out input.rexx
  ```
- Stress-test idempotency and fixed-point convergence:
  ```bash
  ./rxc -d3 -o out input.rexx
  ```
- Run the full test suite (from the build directory):
  ```bash
  ctest -j1   # sequential first, then try -j8
  ```

## API notes and migration
- Prefer the specialized resolvers (`sym_rslv_local`, `sym_rslv_attribute`, `sym_rslv_global`, `sym_rslv_tiered`) over any deprecated generic resolution helpers.
- Hoist via `sym_hoist_to_namespace` instead of calling low-level merge routines directly.

## Impact summary
- User-visible behavior is unchanged except for improved diagnostics and stability.
- Compiler developers benefit from stronger invariants and simpler, more predictable symbol lookups.
