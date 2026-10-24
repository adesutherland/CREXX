# RexxScript Developer Guide

This guide covers two explicitly selected implementations. **Modern
`rexxscript1` uses a Level L grammar and executes its selected tree.** The
legacy `rexxscript` namespace/command and `REXXSCRIPT` compiler exit retain the
earlier handwritten evaluator and compatibility API. They are not the same
parser, dialect or host contract. The modern path is an unreleased `rxlang`
review implementation; the legacy path is the current beta 3 WIP consumer.
The [Level C architecture design](../../compiler/docs/levelc_working_architecture.md)
records the shared `rxfnsc` runtime boundary. Both products use its
`RexxValue`, `RexxVariablePool`, and overlapping Classic BIF implementations.
The legacy evaluator converts results to strings; modern execution retains
typed values. Their sandbox and host policies are documented separately below.

The product direction is an **untyped, simplified Level G syntax/operator
profile with simplified classes**, retaining shared Classic BIFs. The current
implementations supply reusable services and consumer/migration evidence; their
organic dialect details are not the future language specification. The
[owning roadmap](../../research/level-l/PLAN.md#accepted-first-product-direction--rexxscript-10-october)
retains the selected scalar baseline and the open class/host/migration contracts.
The [shared semantic-IR execution route](../../research/level-l/compilers/doc/architecture.md#dynamic-execution-strategy)
is approved as the direction; detailed integration contracts precede implementation
and neither interpreter has been replaced. G operators,
Classic-compatible BIF contracts, host capabilities and intentional Unicode
policy differences remain separate adapter decisions over shared algorithms.

## Modern Level L implementation (local review)

The [Level L user guide](../../langtools/doc/user-guide.md) explains grammar
authoring, generated modules, token stages and source trees independently of
RexxScript. Its [diagnostic assessment](../../langtools/doc/diagnostics.md)
records current strict/record-recovery behavior and the remaining recovery work.

The modern command is `rexxscript1`; it does not replace the legacy modules.
The [modern user contract](user-guide.md#rexxscript-1-modern-level-l-review-implementation)
records the selected subset and remaining boundaries. Its ownership is:

1. Level L admits `grammar/rexxscript.levell`, emits portable packed images in
   RXAS, and `rxas` produces compiler-importable bytecode. CMake publishes the
   generated artifact only after successful admission.
2. `RexxScriptProgram.crexx` owns source and a named Level L tree. Layout and
   implicit-concatenation normalization are explicit consumer passes; the
   grammar declares the DFA, LR rules and finite keyword stages. A module
   initializer creates the generated language definition once. Calling
   `rexxscript1..compile(source)` returns an already parsed reusable program.
3. `RexxScriptExecution.crexx` owns fresh mutable variables, configuration,
   output, result and budgets. A run is idempotent after completion; construct
   another execution to run again. `RexxScriptOperators` maps the selected Level
   G scalar rules onto typed RexxValue operations. `RexxScriptBifs` dispatches
   directly to the existing rxfnsc functions using a live call context.

This is an end-to-end Level L interpreter example, like the
[calculator](../../langtools/examples/calculator/calculator.rxl), with a
larger language and application semantics. Parsing decides syntax and nesting;
the named-view adapter exposes the selected tree; the validator checks structural
constraints; execution follows control flow. Evaluating an IF visits only its
selected branch, and logical AND/OR short-circuit. A generic exhaustive tree
walk cannot replace these execution rules. The expression evaluator reads
already recognized precedence rules rather than parsing expression text again.

### Which handwritten code should move?

The direction is to move **syntax mechanics** into reusable Level L facilities,
while retaining a clear application evaluator. Code reduction is useful only
when the shared service preserves the actual language and source contracts.

| Current responsibility | Owner today | Proposed treatment and proof needed |
|---|---|---|
| Words/numbers/strings/operators, nested comments, keyword classification, IF attachment and precedence | `grammar/rexxscript.levell` and Level L | Already generated/shared. Do not add a second parser in a visitor. Generalized line/block/nested comments are now a shared source-layer facility; RexxScript retains its existing nested-comment language policy. |
| Physical-newline continuation, parenthesis nesting and THEN/ELSE layout | First pass in `RexxScriptProgram.compile()` | Good migration target for a declared layout stage. Its unbounded parenthesis counter exceeds the current finite token-stage facility. Specify reset/progress and test physical/logical line boundaries before deleting it. |
| Blank/abuttal concatenation and function-call adjacency | Second pass in `RexxScriptProgram.compile()` | Good migration target for source-gap-aware token insertion, including explicit synthetic origins. Current finite transforms cannot emit an extra synthetic operator while retaining both adjacent tokens. Test whitespace/comments/adjacency and continuation jointly. |
| Grammar wrappers and positional child plumbing | `RexxScriptExecution` and `lltree` | Statement lists use repetition and appear as direct ordered children. Execution iterates those children without following recursive tail nodes. Named captures can further reduce positional access; preserve source origins when changing a field. |
| Literal values, NUMERIC_COMMON conversions/operators, variables, loops, branch selection, BIF effects and budgets | `RexxScriptExecution`, `RexxScriptOperators`, shared `rxfnsc` | Keep as application semantics. Share a proven operator/lowering contract where useful; Level L should not become a Rexx-specific evaluator. |
| Legacy scanner/parser/evaluator and facades | `RexxScriptEvaluator`, `RexxScriptRuntime`, compatibility modules and compiler exit | Retain while these consumers use the legacy dialect/host API. Removal needs an explicit migration decision, classified compatibility tests and the still-pending modern host contract. |

Replacing the two token passes is therefore worthwhile, after the common stage
contracts are implemented. Automatic derivation of arbitrary handwritten
fixups is not implied. Keep the old passes as independent parity controls while
the generated replacements prove token kind/span/origin, tree, diagnostics and
execution equivalence. Then remove the duplicate runtime route together with
its source/build entries and update this guide.

Pratt is not a shortcut for deleting all modern expression code. RexxScript
has calls/omitted arguments, concatenation, typed comparisons, short-circuit
operators, and power versus unary precedence. The initial Pratt profile has
one token atom, one grouping pair and prefix operators tighter than every
infix; it does not express all those rules unchanged. The existing LR grammar
is appropriate until a richer checked operator profile has parity proof. The
calculator deliberately stays inside today's smaller profile.

For example, compile a caller with imports `rxfnsb`, `rexxscript1` and
`rexxscript1run`, then use:

```rexx
program = rexxscript1..compile('x=1+2*3; return x' as .binary)
run = .rexxscript1run..execution(program)
status = run.run()
value = run.result()
```

Link `rexxscript1`, `rxfnsl`, `rxfnsc`, `rxfnsg`, `classlib` and `library`.
The shared Unicode BIF module requires the `rxfnsg` Unicode runtime even when
a particular script does not call Unicode normalization. The native runner also
links the shared `cstream` and `fs` providers required by those imported modules;
script access remains governed by the BIF capability checks. The CMake runner
and tests depend on the actual library files so an incremental library change
relinks their embedded bytecode. This API owns
returned values; ordinary VM object/binary assignment copies storage. It is not
a copy-on-write design or a claim that all request initialization is zero-copy.
The parser's packed table lookups use a private synchronous `EXPOSE` borrow,
with no escaping alias. Creating a weak reference per lookup was rejected after
sampling showed repeated lifetime-owner discovery. The [Level L guide](../../langtools/README.md)
records image representation, bounds and loader validation.

Build `testrexxscript1` and `testlevell_generated`, then run the corresponding
CTest names. They launch prebuilt bytecode directly on each available concrete
VM. Ordinary `rexxscript1_{rxbvm,rxtvm}` cells select the semantic workload:
all value, syntax, failure, source-depth/byte, fuel, cancellation, output and
program-ownership checks remain, with a short repeated-statement input.
`rexxscript1_resources` reuses the same optimized image on the product VM for
the original 3,000-NOP sequence and final output, guarded by the actual 3,001
AST statement count. No argument retains the full original manual workload;
instrumented cells keep that full route until their separate memory-scope review.
The SDK and interpreter are already prepared once. The generated-language
tests retain the 3,000-item list on both engines; the four `ts_llast` cells
retain tree depth/width and owner behavior. Both small RexxScript engine cells
still execute loops, calls and cleanup, so the large application-size control
needs one ordinary execution rather than an engine duplicate. The AST folding
matrix retains its existing compiler/assembler modes and both engines.
The separate resource entry is ordinary-only. Programs, grammar caches and
execution state are process-private, so ordinary tests reserve six scheduling
slots as a conservative memory allowance rather than lock the entire family.
Instrumented tests retain the original family lock and full workload.
`tests_functional/build-review.sh opt|noopt [rxas|crexx]` supplies isolated
full-module parity builds. `check-scalar-conformance.py BIN WORK` compares the
same 28 scalar expressions with the current Level G compiler in both compiler
modes and VMs. After adopting develop `54f736804`, all observations agree.
The permanent `float-comparison-reproducer.rexx` now requires false in both
compiler modes; the scalar harness rejects every optimization disagreement.
The [current plan](../../research/level-l/PLAN.md)
records RS-F01 closure and preserves the earlier discrepancy as historical evidence. The interpreter
uses typed integer/float/decimal comparisons and string-targeted Rexx comparisons,
not the Classic RexxValue decimal-comparison helper. This is overlap evidence,
not complete Level G compatibility. The compiler can also diagnose a literal
division by zero inside a short-circuited expression before execution; the
interpreter checks the executed operand only.

`check-cli.py BIN WORK` exercises the native executable, shared provider linkage,
source I/O, UTF-8, syntax, undefined-variable and capability failures. Generator
determinism/atomic failure and dual-format packed-byte checks are in
`langtools/tests/check-generator.py` and `check-image-parity.py`.

Current local qualification is macOS ARM64 Debug. Linux, Windows, sanitizer,
install/package and hosted CI qualification of the new product are not claimed.
Host registration/exposure/write-back and the broader research programme remain
open. The authoritative criteria and retained evidence live in
`research/level-l/PLAN.md`; distinguish this product phase from the seven still
open research criteria.

## Legacy: Source Layout

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

## Legacy: Instance Model

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

## Legacy: Evaluation Pipeline

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

## Legacy: BIF Dispatch

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

RexxScript-local one-dimensional arrays use dotted compound names such as
`items.1` and `items.i`. The evaluator normalizes the suffix to a non-negative
integer and stores the resulting string under the existing environment name
table. Host array sharing is implemented by the RexxScript compiler exit: an
exposed host array must have a `.string[]` type, and non-string arrays are
rejected before generated code is emitted. The `EXPOSE` clause contains only
the host variable name because its declared type supplies the distinction.

When changing intrinsic parsing, remember that intrinsic calls can be nested.
`_eval_builtin()` must copy parsed argument text and omitted-argument flags
before evaluating each argument, because evaluating an argument can recursively
reuse the evaluator's temporary `fn_args`, `fn_exists`, and `fn_arg_count`
attributes.

## Legacy: Adding An Intrinsic

1. Add the name to `_builtin_known()` and `_dispatch_builtin()`.
2. Implement the shared behavior in `lib/rxfnsc/RexxClassicBifs.crexx` when it
   belongs to the Classic-compatible shared layer.
3. Keep argument validation and message construction in the shared BIF helper,
   not in RexxScript.
4. Add RexxScript tests for normal use, omitted arguments where applicable,
   conversion/error behavior, and nested calls if the BIF can appear as an
   argument to another BIF.
5. Update [user-guide.md](user-guide.md) and any generated API comments.

## Legacy: Adding A Classic Function

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

## Legacy: Source Comments And Generated Docs

Use Javadoc-style comments for public classes, public procedures, and future
user-visible helpers where practical. Prefer tags such as:

```text
@param name description
@return description
@throws or @error condition
```

The goal is for generated user documentation to come from source comments
without making `docs/` and `rexxscript/doc/` drift.

## Legacy: Build And Test

Focused build:

```sh
cmake --build cmake-build-debug \
  --target testrexxscript test_rexxscript_bin test_rexxscript_direct_bin
```

Focused tests:

```sh
ctest --test-dir cmake-build-debug --output-on-failure \
  -R 'testRexxScript(Runtime|Compat|ArrayExpose)_(noopt|opt)|test_rexxscript|rexxscript_cli_smoke'
```

Related shared BIF tests:

```sh
cmake --build cmake-build-debug --target testrxfnsc
ctest --test-dir cmake-build-debug --output-on-failure \
  -R 'testRexx(Value|RuntimePools|ClassicBifs)_(noopt|opt)'
```

The compiler-exit tests prove that the `REXXSCRIPT` statement lowers to the
runtime facade and that direct exit behavior remains available.
`testRexxScriptArrayExpose` also uses that statement, so its CMake compile
commands must enable exits and depend on `rxcexits.rxbin`. The runtime API and
compatibility fixtures retain `-x`. Generated exit helper variables have
explicit types so multiple statements in one procedure use intentional local
bindings without scope warnings.

## Legacy: Documentation Ownership

This directory is the product master view:

- `user-guide.md` owns user behavior.
- `developer-guide.md` owns implementation guidance.
- General CREXX docs should link here and provide only brief context.
- `docs/ai-context/REXXSCRIPT*.md` may contain agent-oriented notes, but should
  not become a competing product manual.

## Legacy: Current Extension Boundaries

Keep these boundaries explicit during beta 3:

- RexxScript is interpreted and sandboxed.
- RexxScript is not the Level C compiler path.
- Shared BIF behavior belongs in `rxfnsc` where possible.
- Host integration must remain explicit through `EXPOSE` and captured output.
- Level B defects found while extending RexxScript should be called out rather
  than hidden behind broad workarounds.
