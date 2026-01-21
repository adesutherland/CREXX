### cREXX Project Guidelines

This document provides a summary of the cREXX project structure, build process, and development workflow. cREXX is a complex project consisting of a compiler, assembler, and virtual machine.

#### Project Structure

The project is organized into several key subprojects:

*   **`compiler/`**: The cREXX compiler (`rxc`). It uses the Lemon parser generator (`rxcpbgmr.y`) and re2c scanner (`rxcpbscn.re`).
*   **`assembler/`**: The cREXX assembler (`rxas`). It also uses Lemon (`rxas.y`) and re2c (`rxas.re`).
*   **`interpreter/`**: The cREXX Virtual Machine (`rxvm` and `rxbvm`).
    *   `rxvm`: High-performance "Threaded Mode" interpreter.
    *   `rxbvm`: "Bytecode Mode" interpreter.
*   **`lib/`**: Standard libraries, implemented in Rexx and Assembler.
*   **`machine/`**: Defines the VM instruction set. `rxvmigen.rexx` generates instruction definitions.
*   **`re2c/`** & **`lemon/`**: External tools used for building the project. Do not modify these unless absolutely necessary.

#### Build and Configuration

The project uses CMake. It is recommended to use the provided CLion CMake profiles (Debug, Release).

**Key Build Targets:**
*   `rxc`: The compiler.
*   `rxas`: The assembler.
*   `rxvm`: The threaded VM.
*   `rxbvm`: The bytecode VM.
*   `library`: Builds the standard Rexx libraries (`library.rxbin`).

**Build Command Example:**
```bash
cmake --build cmake-build-debug --target rxc rxas rxvm library
```

#### Development Workflow

The standard cREXX development cycle involves compiling Rexx code to assembler, then to VM binary. Binaries are located in the `compiler/`, `assembler/`, and `interpreter/` subdirectories of the build directory.

1.  **Write Rexx code**: Use `options levelb` at the top of your `.rexx` files for cREXX (Level B) compatibility.
2.  **Compile to Assembler**:
    ```bash
    ./rxc -o output input.rexx # Creates output.rxas
    ```
3.  **Assemble to Binary**:
    ```bash
    ./rxas -o output.rxbin output.rxas
    ```
4.  **Run in VM**:
    ```bash
    ./rxvm output.rxbin
    ```

#### Testing Information

The project uses `ctest` for automated testing.

**Running Tests:**
```bash
cd cmake-build-debug
ctest
```

**Adding New Tests:**
Tests are typically added to `tests/CMakeLists.txt`. You can add a Rexx script to the `REXXTESTBINS` list, and CMake will handle the compile-assemble-run cycle.

**Manual Test Example (Verified):**
1. Create `test.rexx`:
   ```rexx
   options levelb
   say "Testing cREXX..."
   return 0
   ```
2. Run the toolchain:
   ```bash
   ./cmake-build-debug/compiler/rxc -o test test.rexx
   ./cmake-build-debug/assembler/rxas -o test.rxbin test.rxas
   ./cmake-build-debug/interpreter/rxvm test.rxbin
   ```

#### Additional Development Information

*   **Code Style**: The project follows ANSI C (C90) standards. Indentation is 4 spaces. Function braces and `if`/`while` braces are generally on the same line.
*   **Instruction Set**: If you need to modify or add VM instructions, look at `machine/rxvmigen.rexx`. Note that this requires a Rexx interpreter (like Regina or Brexx) to run during the build process.
*   **Scanning/Parsing**: `re2c` and `Lemon` files are the source of truth for lexing and parsing. Do not edit the generated `.c` files directly.
*   **Standard Library**: Many built-in functions (BIFs) are implemented in Rexx in `lib/rxfnsb/rexx/`. These are concatenated into `library.rxbin`.

# cREXX Level B Compiler: Ground Truth Primer

This section defines valid syntax for cREXX Level B based on verified test cases in `compiler/tests/rexx_src/`.

## 1. Valid Syntax Patterns

**Variables and Assignment:**
*   **Simple Assignment:** `a = 10` (Types are inferred from the value).
*   **Inference Rule:** Variables are implicitly typed upon first assignment. Changing the type later (e.g., assigning a string to an integer variable) is illegal and triggers a `#BAD_CONVERSION` error.

**Control Flow:**
*   **IF Statement:** `if condition then instruction; else instruction`.
*   **DO Groups:** Used for multi-line blocks in `IF` or loops. Ends with `END`.
*   **Iterative Loops:** `do i = 1 to 5 by 2 ... end`.
*   **Conditional Loops:** `do while condition ... end` and `do until condition ... end`.

**Logic and Math:**
*   **Basic Math:** `+`, `-`, `*`, `/`, `%` (Integer Division), `//` (Remainder), `**` (Power).
*   **Numeric Options:** 
    *   `options levelb numeric_classic`: Left-associative power (`2**2**3 == 64`), `-3**2 == 9`.
    *   `options levelb numeric_common`: Right-associative power (`2**2**3 == 256`), `-3**2 == -9`.
*   **Boolean Logic:** `&` (AND), `|` (OR), `\` (NOT).

**Procedures and Scope:**
*   **Definition:** `label: procedure`. Optional return type: `label: procedure = .int`.
*   **Arguments:** Must use explicit type prefixes.
    ```rexx
    MyProc: procedure
      arg x = .int
      return x * 2
    ```
*   **Scoping:** The `procedure` keyword isolates variable scope. Variables in the main script are not visible inside a procedure unless exposed (not fully verified in current Level B tests).

## 2. Data Types and Literals

*   **Literals:**
    *   **Decimal:** `123`
    *   **String:** `"Double Quotes"` or `'Single Quotes'`.
    *   **Hex String:** `'414243'x` (Converts to 'ABC').
    *   **Binary String:** `'1010'b`.
*   **Typed Arrays:**
    *   **Definition:** `ary = .int[5]` (Defines an array of 5 integers).
    *   **Access:** `ary[1] = 10` (1-based indexing).

## 3. Modules and Imports

*   **Namespace Definition:** `namespace myname expose myfunc`.
*   **Importing:** `import myname`.
*   **Calling Imported Functions:** Use direct symbol resolution. 
    *   **YES:** `say myfunc(10)`
    *   **NO:** `say myname.myfunc(10)` (Qualified names are not supported).

## 4. Critical Divergences (Known Unimplemented)

*   **SELECT Instruction:** `SELECT / WHEN / OTHERWISE` is **NOT implemented**.
*   **Stems:** Classic Rexx Stems (e.g., `a.i = 1`) are **NOT supported**. Use Level B Typed Arrays.
*   **XOR Operator:** The `&&` logic operator is **NOT implemented**.
*   **Qualified Imports:** Accessing functions via `Namespace.Function()` syntax is **NOT implemented**.
*   **EXIT Instruction:** Can trigger errors in some contexts; use `RETURN` for top-level script exit.
