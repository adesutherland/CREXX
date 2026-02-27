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

To run the full test suite, use `ctest` from your build directory:

```bash
cd cmake-build-debug
ctest -R compiler
```

To run a specific test:

```bash
ctest -R 01_assign
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
  ./compiler/rxc -n -o ./01_assign_noopt ../compiler/tests/rexx_src/01_assign.rexx
```

### Batch Update

If many tests fail due to a global change (like register renumbering), you can use a script to apply the update flag to all relevant tests. 

Example (Linux/macOS):
```bash
ctest -R "_noopt|_opt" -VV | grep "crexx_test_driver" | sed 's/.*Test command: //; s/"//g; s/crexx_test_driver/crexx_test_driver --update-gold/' | bash
```

**Warning:** Always verify that the changes in the golden files are actually what you expect before committing them. Use `git diff` to review the changes in `compiler/tests/golden/`.

## 4. Adding New Tests

1. Create a new Rexx source file in `compiler/tests/rexx_src/`.
2. Add the test to `compiler/tests/CMakeLists.txt` using one of the provided macros:
   - `add_crexx_compiler_test(NAME SOURCE)`: Adds both `_noopt` and `_opt` assembly comparison tests.
   - `add_crexx_run_test(NAME SOURCE)`: Adds a functional test that runs the program and compares its output.
   - `add_crexx_robustness_test(NAME MAIN_FILE [DEPS...])`: Adds a multi-file functional test verifying complex resolution scenarios.
3. Run the test. It will fail because the golden file is missing.
4. Use `--update-gold` (as described above) to create the initial golden file.
5. Verify the created golden file and commit it.
