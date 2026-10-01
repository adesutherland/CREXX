# RexxScript Developer Guide

This guide describes the current RexxScript implementation and the rules for
extending it during the Release 1 beta 3 work.

## Source Layout

RexxScript lives under `rexxscript/`:

```text
RexxScriptEvaluator.crexx
    namespace rexxscriptcore
    owns the core evaluator class, parser state, output buffer, labels,
    sandbox RexxVariablePool, and BIF call context.

RexxScriptRuntime.crexx
    namespace rexxscript
    public facade around the core evaluator and transitional last-result API.

RexxScriptEvaluateCompat.crexx
    namespace rxfnsb
    compatibility facade for the original evaluate()/evaluate_exposed()
    prototype surface.

RexxScriptRunner.crexx
    standalone file runner packaged as bin/rexxscript.

tests_functional/
    runtime, compatibility, compiler-exit integration, and CLI smoke coverage.
```

The runtime builds `bin/rexxscript.rxbin`. The standalone executable packages
`RexxScriptRunner.crexx` with `library`, the core `classlib`, `rxfnsc`, and
`rexxscript`, and statically links the `system` plugin for the file-I/O path.
The current classlib collection code is Rexx-only; RexxScript no longer needs
the historical `treemap` plugin, and it does not link the opt-in
`classlib_native.rxbin` adapters for `Id`, `KeyDB`, or `Os`.

## Instance Model

The public `.rexxscript..rexxscriptevaluator()` facade owns one
`.rexxscriptcore..rexxscriptcoreevaluator()` instance.

Each core evaluator owns:

- sandbox variable arrays and a `RexxVariablePool`;
- captured output lines;
- exported variable names and values;
- label tables;
- statement and block stacks;
- expression parser state;
- temporary intrinsic argument state.

`evaluate()` and `evaluate_exposed()` both call the core `_evaluate_impl()`.
Every evaluation starts with `_reset()`, so reusing an evaluator instance does
not retain old script variables or output.

The procedural helpers preserve compatibility:

- `rexxscript_evaluate(script, debug)`
- `rexxscript_evaluate_exposed(script, names, values, debug)`
- `rexxscript_output()`
- `rexxscript_value(name)`

Those helpers store the last facade result in module-level arrays. They are not
the right API for code that needs multiple live evaluator instances.

## Evaluation Pipeline

The current evaluator is deliberately simple:

1. Reset instance state.
2. Seed sandbox variables from exposed names and values.
3. Split the source into semicolon-separated statements while respecting
   string literals.
4. Scan labels.
5. Execute statements with a program counter.
6. Export sandbox variables, captured output, and any COLLECT values.

Structured `DO` blocks are tracked with parallel block arrays. `SIGNAL label`
and `GOTO label` set a jump target from the precomputed label table.

## BIF Dispatch

RexxScript intrinsics are not general Rexx function calls. The evaluator owns
the allow-list in `_builtin_known()` and the name controller in
`_dispatch_builtin()`. The controller constructs the shared context and calls
the specific `rxfnsc` export:

```rexx
context = .RexxBifCallContext(upper_name)
call context.setArguments(bif_args, bif_exists)
call context.setCallerPool(reference script_pool)
result = rexxclassicbif_substr(reference context)
```

`bif_args` is a `.RexxValue[]`, and `bif_exists` is the provided-argument
mask. Each `rexxclassicbif_*` entry returns a `RexxValue`; check
`context.hasError()` for validation failure, then convert the result back to
the evaluator's string value with `result.asString()`. Direct Level C library
harnesses call these exports without using the RexxScript controller. Current
compiler output may retain the deprecated `rxfnsc` compatibility dispatcher
until the later bulk lowering change.

The caller pool is always the RexxScript sandbox pool. Do not pass the host
CREXX pool into the BIF context.

`COLLECT expression` is a scalar application-value statement. It appends to the
evaluator-local ordered collection stream without terminating execution or
changing the `SAY` output stream. The embedded compiler exit exposes that stream
through the `VALUES target` clause, where the target is a `.string[]`.

When changing intrinsic parsing, remember that intrinsic calls can be nested.
`_eval_builtin()` must copy parsed argument text and omitted-argument flags
before evaluating each argument, because evaluating an argument can recursively
reuse the evaluator's temporary `fn_args`, `fn_exists`, and `fn_arg_count`
attributes.

## Adding An Intrinsic

1. Add the name to `_builtin_known()` and `_dispatch_builtin()`.
2. Implement the shared behavior in `lib/rxfnsc/RexxClassicBifs.crexx` when it
   belongs to the Classic-compatible shared layer.
3. Keep argument validation and message construction in the shared BIF helper,
   not in RexxScript.
4. Add RexxScript tests for normal use, omitted arguments where applicable,
   conversion/error behavior, and nested calls if the BIF can appear as an
   argument to another BIF.
5. Update [user-guide.md](user-guide.md) and any generated API comments.

## Adding A Classic Function

Use this checklist when wiring a new Classic-compatible function into
RexxScript:

1. Add the function name to `_builtin_known()` in
   [RexxScriptEvaluator.crexx](../RexxScriptEvaluator.crexx).
2. Add the dispatch case in `_dispatch_builtin()` in the same file.
3. Put the implementation in `lib/rxfnsc/RexxClassicBifs.crexx` if the function
   should be shared by RexxScript and other classic callers.
4. Export the helper from `rexxclassicbifs` and add any extra imports the shared
   bundle needs, such as character-scan helpers.
5. Keep the evaluator thin: it should only gate names, build the call context,
   and forward to the shared helper.
6. Add or update shared BIF tests in `lib/rxfnsc/tests_functional/`.
7. Add or update RexxScript integration tests if the new function changes
   evaluator behavior, error propagation, or nested-call handling.

## Source Comments And Generated Docs

Use Javadoc-style comments for public classes, public procedures, and future
user-visible helpers where practical. Prefer tags such as:

```text
@param name description
@return description
@throws or @error condition
```

The goal is for generated user documentation to come from source comments
without making `docs/` and `rexxscript/doc/` drift.

## Build And Test

Focused build:

```sh
cmake --build cmake-build-debug --target testrexxscript
```

Focused tests:

```sh
ctest --test-dir cmake-build-debug --output-on-failure \
  -R 'testRexxScript(Runtime|Compat)_(noopt|opt)|test_rexxscript|rexxscript_cli_smoke'
```

Related shared BIF tests:

```sh
cmake --build cmake-build-debug --target testrxfnsc
ctest --test-dir cmake-build-debug --output-on-failure \
  -R 'testRexx(Value|RuntimePools|ClassicBifs)_(noopt|opt)'
```

The compiler-exit tests prove that the `REXXSCRIPT` statement lowers to the
runtime facade and that direct exit behavior remains available.

## Documentation Ownership

This directory is the product master view:

- `user-guide.md` owns user behavior.
- `developer-guide.md` owns implementation guidance.
- General CREXX docs should link here and provide only brief context.
- `docs/ai-context/REXXSCRIPT*.md` may contain agent-oriented notes, but should
  not become a competing product manual.

## Current Extension Boundaries

Keep these boundaries explicit during beta 3:

- RexxScript is interpreted and sandboxed.
- RexxScript is not the Level C compiler path.
- Shared BIF behavior belongs in `rxfnsc` where possible.
- Host integration must remain explicit through `EXPOSE` and captured output.
- Level B defects found while extending RexxScript should be called out rather
  than hidden behind broad workarounds.
