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

For compiler work, the reliable regression gate is the compiler test subtree rather than a top-level regex filter.

To run the full compiler regression suite:

```bash
cmake --build cmake-build-debug -j4
ctest --test-dir cmake-build-debug/compiler/tests --output-on-failure
```

This is the recommended gate for optimisation and inlining iterations because it covers:

- compiler output golden tests in both `noopt` and `opt`
- runtime golden tests in both `noopt` and `opt`
- parsing, warning, error, and robustness tests

If you want the entire repository test suite instead of just compiler coverage:

```bash
ctest --test-dir cmake-build-debug --output-on-failure
```

For cREXX system work that touches libraries, BIFs, plugins, TRACE, or linked
artifacts, use the focused system subsets before the full suite:

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

The easiest way to update all failing golden tests is to run the test commands with the flag added. Since `ctest` abstracts the command, you can find the command line by running `ctest -V`.

Example of manually updating a single golden file:

```bash
# Assuming you are in the build directory
./compiler/tests/crexx_test_driver --update-gold \
  ../compiler/tests/golden/noopt/01_assign.rxas \
  ./01_assign_noopt.rxas \
  ./compiler/rxc -n -o ./01_assign_noopt ../compiler/tests/rexx_src/01_assign.crexx
```

### Batch Update

If many tests fail due to a global change (like register renumbering), you can use a script to apply the update flag to all relevant tests. 

Example (Linux/macOS):
```bash
ctest -R "_noopt|_opt" -VV | grep "crexx_test_driver" | sed 's/.*Test command: //; s/"//g; s/crexx_test_driver/crexx_test_driver --update-gold/' | bash
```

**Warning:** Always verify that the changes in the golden files are actually what you expect before committing them. Use `git diff` to review the changes in `compiler/tests/golden/`.

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
3. Run the test. It will fail because the golden file is missing.
4. Use `--update-gold` (as described above) to create the initial golden file.
5. Verify the created golden file and commit it.
