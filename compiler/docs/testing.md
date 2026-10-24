# cREXX Compiler Testing Guide

The `rxc` compiler uses a regression testing suite located in `compiler/tests`. These tests ensure that changes to the compiler do not break existing functionality or change the generated assembly in unexpected ways.

## 1. Test Suite Structure

The testing infrastructure consists of:
- **`rexx_src/`**: Rexx source files used as input for the tests.
- **`golden/`**: "Golden" reference files that represent the expected output.
    - `noopt/`: Expected `.rxas` (assembly) files when optimization is disabled (`-n`).
    - `opt/`: Expected `.rxas` files when optimization is enabled (default).
    - `run/`: Expected standard output when running the compiled program in the VM.
    - `parsing/`: Expected AST output for parsing tests.
    - `errors/`: Expected compiler error messages for negative tests.
    - `robustness/`: Comprehensive scenarios for symbol resolution and cross-module linkage (functional tests).
- **`src/test_driver.c`**: A custom C program that runs the compiler, captures its output, and compares it against the golden files.
- **`CMakeLists.txt`**: Defines the tests and how they are run using `ctest`.

## 2. Running Tests

Select development checks for the changed mechanism under `AGENTS.md`: build
its component and prerequisites, inspect representative RXAS, and run the
affected value, error, state and cleanup contracts. Reuse unchanged prepared
tools and images. Ordinary optimisation or inlining iterations do not require
the whole compiler suite or every compiler/assembler/VM combination.

The compiler subtree is the compiler-wide regression scope at an agreed
integration milestone. Prepare the selected tests through the owning QA
targets (`CREXX_PREP_TARGETS`) before invoking CTest. The repository-wide
`qa-prep-comprehensive` target belongs to a complete ordinary baseline:

```bash
ctest --test-dir cmake-build-debug/compiler/tests --output-on-failure
```

This scope covers:

- compiler output golden tests in both `noopt` and `opt`
- runtime golden tests in both `noopt` and `opt`
- parsing, warning, error, and robustness tests

If you want the entire repository test suite instead of just compiler coverage:

```bash
ctest --test-dir cmake-build-debug --output-on-failure
```

For cREXX system work that touches libraries, BIFs, plugins, TRACE, or linked
artifacts, select relevant checks from these system subsets. They do not
automatically commission a later full suite:

```bash
# Rexx BIF/library functional tests
cmake --build cmake-build-debug --target testbifs
ctest --test-dir cmake-build-debug -R '^ts_.*_(noopt|opt)$' --output-on-failure

# Native system plugin smoke test
ctest --test-dir cmake-build-debug -R '^test_system$' --output-on-failure

# TRACE/debug metadata and source-stripped linker behavior
ctest --test-dir cmake-build-debug \
  -R '^(trace_event_metadata|test_trace_|ts_trace_|rxlink_format_check|rxlink_rxdas_strip_smoke)' \
  --output-on-failure
```

The BIF build is intentionally different from a user program build. Most
`lib/rxfnsb/rexx/*.crexx` files are compiled with `rxc -x`, which disables
certified compiler exits to avoid bootstrap cycles while building the library
that exits depend on. An explicit `TRACE`, `PARSE`, or `ADDRESS` statement in a
BIF source file will therefore fail with `#CERTIFIED_EXIT_DISABLED`.

To debug a Rexx BIF or standard-library helper, prefer a normal fixture or
scratch program that imports `rxfnsb` and calls the BIF with exits enabled. If
you need to see library frames, add `TRACE UNSUPPRESS NAMESPACE rxfnsb`; TRACE
hides standard library and `_rxsys*` namespaces by default. For native or linked
debugging, keep source/TRACE metadata with `crexx -native --link-keep-source`
or an unstripped `rxlink` image.

To run a specific test:

```bash
ctest --test-dir cmake-build-debug/compiler/tests -R 01_assign --output-on-failure
```

## 3. Updating Golden Files (`--update-gold`)

When you intentionally change the compiler's behavior (e.g., changing the register allocation strategy or improving optimizations), the generated assembly will change, causing the golden tests to fail.

If you have verified that the new output is correct, you can automatically update the golden files using the `--update-gold` flag of the `crexx_test_driver`.

### How to use `--update-gold`

Select the exact affected golden tests after reviewing their complete emitted
RXAS against independent expectations. `ctest -N -V` shows their commands.
Separate register/frame or truthful TRACE changes from material wrong values,
control, state, ownership and required diagnostics. A passing runtime check
alone does not justify a snapshot refresh.

Example of manually updating a single golden file:

```bash
# Assuming you are in the build directory
./compiler/tests/crexx_test_driver --update-gold \
  ../compiler/tests/golden/noopt/01_assign.rxas \
  ./01_assign_noopt.rxas \
  ./compiler/rxc -n -o ./01_assign_noopt ../compiler/tests/rexx_src/01_assign.crexx
```

Refresh only the reviewed snapshots, then inspect their diff and replay those
affected tests. Do not automatically replace all failing goldens. Preserve
runtime expectations and prefer stable structural assertions for operand,
ownership and observation obligations over new register snapshots.

### BIF/library changes and import goldens

Standard-library BIF changes can alter consumer `.rxas` import declarations even
when the tests never call the new BIF entry point. If a change under
`lib/rxfnsb/rexx/` makes compiler golden tests fail while the matching runtime
tests still pass, first rebuild the linked library image so the compiler imports
the current provider metadata:

```bash
cmake --build cmake-build-debug --target library
cmake --build cmake-build-debug --target testbifs
```

Then rerun the focused compiler tests, inspect the generated/golden diff, and
only update the goldens if the RXAS shape change is intentional:

```bash
ctest --test-dir cmake-build-debug/compiler/tests -R '13_stems' --output-on-failure
ctest --test-dir cmake-build-debug -R '^ts_stem_(noopt|opt)$' --output-on-failure
git diff -- compiler/tests/golden
```

When the diff is only an import snapshot change, prefer checking whether the
consumer RXAS still imports exactly the callables it needs for link/runtime
rather than treating every added provider method as a required golden update.

## 4. Adding New Tests

1. Create a new Rexx source file in `compiler/tests/rexx_src/`.
2. Add the test to `compiler/tests/CMakeLists.txt` using one of the provided macros:
   - `add_crexx_compiler_test(NAME SOURCE)`: Adds both `_noopt` and `_opt` assembly comparison tests.
   - `add_crexx_run_test(NAME SOURCE)`: Adds a functional test that runs the program and compares its output.
   - `add_crexx_robustness_test(NAME MAIN_FILE [DEPS...])`: Adds a multi-file functional test verifying complex resolution scenarios.
3. State the independent expected behavior and select meaningful normal,
   negative and lifecycle observations before running the test. Choose modes
   and engines for a named risk; the macros' historical defaults do not require
   new nested matrices.
4. Prepare once, review source and representative RXAS, and run the selected
   checks. Use `--update-gold` only for a reviewed snapshot oracle.
5. Keep the source, structural/runtime oracle and registration together.

## 5. Scope, axes and shared preparation

Compiler optimisation, assembler optimisation, import representation and VM
dispatch are separate axes. A coupled `-n` flag does not establish an independent
compiler/assembler matrix. Add a combination only when it crosses a distinct
contract or closes an unresolved risk. Share identical isolated producer inputs
between consumers and retain file-level freshness edges as well as producer
targets. Distinct source/binary import roots must remain isolated during reuse.

Frontend rejection precedes optimisation in `rxcpmain.c`. The global-import
harnesses therefore check each independent malformed metadata, bounds,
dimension and return-type envelope once, preserving every diagnostic oracle.
Their successful source, RXAS, RXBIN, shared-pool, B/G and compiler-mode paths
remain. `binary_global_import_types` additionally retains all four positive
compiler/assembler combinations, including mixed modes, to cover metadata
preservation through assembly and pool remapping.
The six RXAS/RXBIN legacy explicit-caller fallback contracts run once in the
opt/opt envelope; their difference is frontend type admission, while the main
positives retain the transformed instruction and mixed-mode metadata proof.

`levelc_call_resolution_result` and its noopt counterpart own two prepared
compiler images using the existing default assembler policy. Direct and linked
opt consumers share that same opt image and retain their exact output oracles.
Missing preparation is an error naming its owner; the runtime consumer does
not silently launch another producer. Other runtime-slice helper sites keep
their original preparation route.

`classic_frontend_parity` compiles both `rxc` and `rxc-c`, then compares their
complete RXAS bytes. Exact equality permits one downstream assemble/link/run
sequence; different emissions retain both runtime paths and output parity.
Both compiler modes, B/G source brokers, Unicode paths and diagnostic parity
remain. A genuinely distinct threaded engine retains its original control.
Successful scratch is deleted; failure diagnostics remain transient for
inspection and cleanup.

Ordinary `highlight_editor_diagnostics` reuses one stdio parser connection and
one logical document ID for four full reloads. Every reload still owns a fresh
CodeBuffer, joins its asynchronous parser work and frees its buffer and rules.
The aggregate owns the connection and closes it after all cases. Existing tree
and editor-character diagnostic checks remain, with stale-diagnostic negatives
between reloads. Manual no-argument and instrumented runs retain the original
fresh-process-per-case workload. `highlight_cache` owns same-document warming
and document/path invalidation; `stdio_shutdown_test`, `protocol_test` and the
separate CLI cases retain transport and process boundaries. Keep the
syntax-highlighter resource lock.

The verified `levelc_{queue,implicit,call_resolution,return,address,select,
procedure,exit,call_expression,signal,arg_template}_source_tree.cmake` checks
observe authored RAW, canonical LEVELC_LOWERED, and where required FIXUP and
SYMBOLS stages. They use existing `-n -dp` to preserve validation and those
observations while avoiding unrelated optimisation and emission. Runtime
opt/noopt pairs such as `levelc_call_resolution_result`,
`levelc_call_expression_actuals`, `levelc_return_whole`, `levelc_select_instruction`
and the ARG/SIGNAL suites continue to own executable behaviour; lowered-tree
and code-generation contracts retain their separate assertions.

`rxvm` is a compatibility alias for a concrete engine. Nested import harnesses
run each canonical executable path once; distinct executables are retained.
The explicit `rxasdispatchcontract-rxbvm` and `rxasdispatchcontract-rxtvm`
contracts cover genuine engine dispatch. `metadata_contract_conformance`,
`rxas_optimizer_metadata`, `rxas_exposed_global_bounds`, `rxlink_format_check`
and `rxlink_core_provider_requirements` retain their metadata, assembler and
linker boundary obligations. These checks complement the import positives;
they do not replace their mixed-mode metadata proof.

An agreed full functional baseline still requires failing cells to be repaired,
inputs frozen, and the complete ordinary regression suite green once. The
bounded component-memory and resource panels follow that gate under their
owning guides; ordinary correctness does not require an instrumented functional
matrix. Keep transient diagnostics in scratch, report conclusions in
conversation and delete task-created logs after diagnosis.

## 6. Shared byte-source and clock preparation

`ContractFixture` remains the single preparation/cache owner for expression,
BIF, source-data and clock contracts. Extend it with exact byte sources and
outputs, explicit source filenames and auxiliary provider sources/flags.
Source-data setup prepares both compiler modes once per B/C/G/L frontend;
all sixteen direct/linked runtime cells remain. L continues to import its B
provider. Clock setup likewise prepares two images for all four runtime cells.
The assembler policy stays unchanged. No second cache or runtime algorithm is
introduced, and no source byte, NUL, newline or diagnostic location is normalized
away to gain reuse.

Only ordinary CMake commands opt into these shared preparations. With no
`--fixture-dir`, manual and instrumented commands use the same generator and
helper's fresh route: each case invokes the original selected-mode compiler,
assembler, optional linker and VM, without cache reuse. Instrumented scheduling
stays unchanged pending the later sanitizer scope review. Text consumers keep
their existing newline treatment; byte consumers compare raw output, with
platform stream newlines constructed explicitly in each fixture expectation.
Preparation identity includes native shared-library inputs as well as tool,
library/plugin and generator bytes. Required RXAS/RXBIN/provider outputs are
checked independently of a manifest's possibly incomplete artifact inventory.
Cold preparation resets its owned generated-source tree; exact source-file
inventory is checked before admission and publication so removed/renamed
providers cannot remain undeclared import candidates.
Ordinary prepared consumers use their per-root lock and private runtime state;
they do not require a global serial slot. Cold setup remains serialized, and
instrumented consumer scheduling retains its original policy.

1. **CFT-AC01 — todo:** every existing source-data/clock value and byte oracle
   remains; invalid NUL source is checked once per distinct frontend level with
   its original filename/location requirement.
2. **CFT-AC02 — done:** helper, producer source, tools, library/plugin inputs,
   compiler mode and provider bytes/flags determine valid preparation. Missing,
   partial or changed inputs/outputs are refused before runtime; publication and
   cleanup retain the existing owner lock and atomic manifest boundary.
3. **CFT-AC03 — done:** a tiny real string consumer preserves the old helper
   route; exact-byte/provider consumers and independent missing-output/changed-
   provider controls prove the new boundary without a full expression/BIF matrix.
4. **CFT-AC04 — todo:** successful preparation exposes diagnostics, preserves
   all named runtime cells and reuses valid images. Transient test scratch is
   removed after diagnosis; prepared manifests remain only executable cache
   validation inputs.

1. **CFT-STEP01 — done:** record this bounded contract before helper edits.
2. **CFT-STEP02 — done:** extend the existing helper, adapt source-data/clock
   generators and register their setup ownership; serves CFT-AC01–04.
3. **CFT-STEP03 — done:** independently review source, run the small helper
   integration/refusal control and affected source-data/clock cells centrally.
   Old expression/BIF setup keys will invalidate because the helper changed;
   defer their complete setup to the agreed final ordinary suite unless a
   directly observed compatibility risk requires an affected check earlier.
4. **CFT-STEP04 — todo:** reuse the valid preparations and complete the agreed
   final ordinary suite, including every retained runtime cell and the existing
   expression/BIF setups; serves the remaining CFT-AC01 and CFT-AC04 gate.
