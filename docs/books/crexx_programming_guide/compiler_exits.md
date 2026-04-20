# Compiler exits

This chapter introduces cREXX compiler exits: what they are, how to enable them, and how to use them effectively. It also shows concrete examples, including how variable shadowing works inside exit-generated code.

## What are compiler exits?

A compiler exit is a compile-time hook written in Rexx that can inspect tokens and inject replacement code into the current compilation unit. Exits are packaged as a module that the compiler (`rxc`) loads on demand.

This chapter primarily describes the current general-exit model. The ongoing
`ADDRESS` / REXXSAA work has also introduced a certified/system-exit model for
core-team-curated exits that may own reserved keywords and use a richer
compiler-facing planning contract. The working design record is
[address_rexxsaa_working.md](/Users/adrian/CLionProjects/CREXX/compiler/docs/address_rexxsaa_working.md:1).

Typical uses:

- Lightweight source transforms for diagnostics and logging (e.g., a `dump` helper that expands into runtime `say` statements).
- Compile-time queries over the token stream (`query` exit).
- Project-specific code generation patterns.

Injected code is grafted into the AST and then compiled like normal source.

## Enabling exits

The compiler looks for an exit bundle (a packed `.rxbin`) in the import path.

- Provide an import path that contains your bundle (e.g., the build `bin` directory):

```bash
  rxc -i /path/to/bin -o out in.rexx
```

- Select the exit module to load using the `RXCP_EXIT_MODULE` environment variable. For the built-in bundle this is `rxcexits`:

```bash
  RXCP_EXIT_MODULE=rxcexits rxc -i /path/to/bin -o out in.rexx
```

Notes:

- The exit module is discovered dynamically at compile time.
- All exits within the bundle are registered (e.g., `dump`, `query`).

## Built-in demo exits

The project ships a small bundle of exits primarily for demonstration and testing:

- `dump` — emits `say` statements to print variables (including arrays) with type information.
- `query` — queries token properties.

You can find their sources under `compiler/exits/` in the repository.

## Certified/System Exits

Current user exits are intended for non-reserved keywords. The compiler now also
has certified/system exits for core-team-policed language features.

Current certified exits:

- `ADDRESS`
- `PARSE`

Key distinctions in the current model:

- Certified/system exits may own reserved keywords.
- General user exits should not be able to override those reserved bindings.
- Certified/system exits may use a richer planning response than the legacy
  string-only `pre_process()` contract.

Treat the working note as the design authority for the evolving model:
[address_rexxsaa_working.md](/Users/adrian/CLionProjects/CREXX/compiler/docs/address_rexxsaa_working.md:1).

## Example: Dumping variables and arrays

Given

```rexx
options levelb
import rxfnsb
main: procedure
  a = .int[5]
  a[1] = 42
  dump a
  return 0
```
Compiling with the exits bundle:

```bash
RXCP_EXIT_MODULE=rxcexits rxc -i ./cmake-build-debug/bin -o test ./path/to/file.rexx
```

At compile time, `dump a` is replaced with a small loop that prints every element:

```bash
a (.int[5])[1] = 42
a (.int[5])[2] = 0
a (.int[5])[3] = 0
a (.int[5])[4] = 0
a (.int[5])[5] = 0
```

## Shadowing and scoping inside exits

Exit replacements are grafted back into the main AST and then normalized like
ordinary source. Structured replacements are supported, including nested
`DO ... END`, `IF ... THEN/ELSE`, and nested instruction blocks. Local scopes
for those generated blocks are materialized during the normal symbol-building
passes, so identifiers created by injected code do not collide with or
overwrite variables in the host program.

Practical consequences:

- A typed declaration inside an exit fragment (e.g., `i = .int`) creates an exit-local variable that is not visible outside the replacement block.

- If the host program already has a variable named `i`, it is not affected by the exit’s internal `i`.

Illustration (conceptual)

```rexx
options levelb
main: procedure
  i = .int; i = 7
  dump i   -- exit may internally use a loop variable named i
  say i    -- still prints 7; host i is untouched
  return 0
```

Behind the scenes, any generated grouped blocks are compiled with their own
local scopes so that temporary loop counters and helper variables remain
confined to the exit-generated block structure.

## Writing your own exit (overview)

An exit is a Rexx module that exports a `process` procedure taking a token descriptor and returning a replacement string or an empty string.

High-level shape

```rexx
namespace myexits expose process

process: procedure
  arg ti = .token
  -- Inspect ti.get_type(), ti.get_text(), ti.get_value_type(), ...
  -- Optionally build and return source replacement text
  return ""
```

Package your exit module into a bundle (`.rxbin`) and place it in the compiler’s import path, then set `RXCP_EXIT_MODULE` to the bundle name.

## Planning Hooks

Current behaviour:

- `pre_process()` is used for early structural planning such as hoisting
  implicit variables.
- General exits may still communicate that planning back to the compiler as a
  space-separated list of 1-based token indices.
- Certified exits can now return a structured `rxcp.exitplan` object for
  binding hoists, contextual keyword claims, and planning diagnostics.

Planned direction:

- the Stage 1 `ADDRESS` work continues to extend the structured planning model
  with richer typed planning data and broader address-environment contracts.

See the working note for the proposal details:
[address_rexxsaa_working.md](/Users/adrian/CLionProjects/CREXX/compiler/docs/address_rexxsaa_working.md:1).

Tips

- Prefer uncommon temporary names and make typed declarations for any locals you introduce in the replacement.

- For detailed compiler diagnostics, use `rxc -d2` or `rxc -d3`, but redirect
  both stdout and stderr to a temp log and inspect the log with `tail`, `sed`,
  or `grep` rather than streaming the raw trace to the terminal.

## Troubleshooting

- `EXIT_BRIDGE_DISPATCH_FAILED` — The compiler could not find a handler for the exit keyword. Ensure your bundle is on the import path and `RXCP_EXIT_MODULE` names it correctly.

- Replacement compiles but behaves oddly — verify your replacement string is valid Level B Rexx and, when using loops, that all loop variables are declared locally inside the replacement (typed) so they do not depend on host context.
