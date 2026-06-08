# crexx Debugging Workflow

When investigating compiler bugs, follow this exact workflow to isolate the issue:

### 1. The Build Step
Always ensure the compiler is built before testing.
Command: `cmake --build cmake-build-debug` (or standard cmake build dir).

### 2. AST Verification (-d2)
If a bug involves parsing, scoping, or grammar logic, dump the AST.
Command: `./rxc -d2 <test_file.rexx>`
*Analysis:* Look at the tree output to ensure the Lemon parser (`compiler/rxcpbgmr.y` or `rxcpopgr.y`) pushed and popped scopes correctly and attached the right variable nodes.

### 3. Symbol Table / IR Verification (-d3)
If the AST is correct but the emitted assembly is wrong, dump the compiler internals.
Command: `./rxc -d3 <test_file.rexx>`
*Analysis:* Trace the variable lifecycle and verify the register allocation before it hits the `rxas` assembler.

### 4. Running the Code
To test end-to-end execution:
1. Compile: `./rxc test.crexx` (produces `test.rxas`)
2. Assemble: `./rxas test.rxas` (produces `test.rxbin`)
3. Execute: `./rxvm test.rxbin`

### 5. Isolating Assembler Keyhole Optimiser Bugs
If generated `.rxas` looks correct but the `.rxbin` or disassembly looks wrong,
compare assembler output with and without the keyhole optimiser:

```sh
./rxas -n -o test_noopt test.rxas
./rxas -o test_opt test.rxas
./rxdas -o test_noopt.dis test_noopt.rxbin
./rxdas -o test_opt.dis test_opt.rxbin
diff -u test_noopt.dis test_opt.dis
```

`rxc -n` disables compiler optimisation when compiling source REXX. `rxas -n`
specifically disables the assembler keyhole optimiser. Use `rxas -n` when the
assembly input is already known and the question is whether peephole rules,
instruction-flow metadata, or hidden register-use handling changed the bytecode
incorrectly.

### 6. RXDB Trace Debugger

`debugger/rxdb.crexx` is the early Level B debugger prototype. It now delegates
source lookup, ASM instruction decoding, module/procedure lookup, breakpoint
enable/disable, and default trace filtering to `rxfnsb.trace` classes:

- `.tracecontroller`
- `.tracecontext`

Debugger presentation lives outside the runtime library in
`debugger/rxdb_gui.crexx`, which exposes `.rxdbtextgui` for banners, prompts,
ANSI cursor control, and plain text output.

The default `rxdb` UI still uses ANSI cursor control. For log-friendly or
LLM-readable output, run:

```sh
rxdb llm
```

`text` and `plain` are accepted aliases. The text mode is intentionally a
small prototype surface for debugging the trace runtime without escape
sequences. `llm` mode batches Enter-driven stepping in groups of 50 trace
events and prints that policy in the banner and running prompt.

RXDB is experimental for the Release 1 beta line. Keep automated coverage to a
small launch/usage smoke test until the debugger command model and UI contract
are promoted into release scope.

The certified `TRACE` compiler exit also has a log-friendly `TRACE LLM` mode.
It writes escaped JSON-lines-style records through the trace runtime and can be
combined with `TO STDERR` or `TO FILE expr` when debugger automation needs a
separate trace stream.

See `docs/ai-context/CREXX_TRACE_REQUIREMENTS.md` for the TRACE compatibility
target, current implementation status, output formats, and enhancement roadmap.

Keep watch-value reads in the interrupt handler unless the VM exposes a
frame-safe abstraction: `metalinkpreg` must inspect the interrupted child frame,
and moving that operation behind an ordinary method call changes the frame being
linked. The shared `rxfnsb.trace` controller should own metadata lookup and
target selection; generated/debugger handlers should limit themselves to the
unavoidable frame-local register link and then hand the value back to the
shared trace runtime.

### 7. High-Risk Compiler Change Checklist

For broad compiler work such as new source syntax, type-system changes,
metadata changes, reference/value ownership, or inliner behaviour, treat the
work as a staged pipeline change rather than a single parser patch.

Process lessons:

* Start with the smallest semantic slice that is executable end to end. For the
  reference work, RXAS/VM operations and generated-code contracts came before
  public Rexx syntax; explicit `reference` / `dereference` syntax came before
  live member/index sugar through references.
* Keep deferred language surface explicit in docs and tests. If `itemsRef[i]`
  or `listRef.add()` is not implemented yet, say so in the working design and
  avoid writing positive tests that accidentally rely on it.
* Update every compiler layer that knows about a new type or AST node: scanner
  tokens, Lemon grammar, AST node names, AST clone/free helpers, symbol copy
  helpers, fixed-point validation, promotion rules, emitter register planning,
  RXAS emission, optimiser/inliner gates, metadata stringification, and
  diagnostics.
* Composite type metadata usually needs both value and target sidecars. A plain
  `TP_REFERENCE` or `TP_OBJECT` enum is not enough when diagnostics, metadata,
  assignment checks, and `typeof()` must preserve the referent class, array
  dimensions, or nested type information.
* Add source-level positive tests and negative tests together. Boundary errors
  are part of the contract, not cleanup work.
* Run the affected feature through noopt and opt, both `rxvm` and `rxbvm`, and
  linked optimized runtime paths. Optimizer or linker-only failures are common
  when metadata, imports, inlining, or register lifetimes change.
* Use ASAN for ownership-sensitive changes. In sanitizer build trees, build the
  test helper explicitly when needed:

  ```sh
  cmake --build cmake-build-asan --target crexx_test_driver
  ```

* If a test unexpectedly fails in a broad setup target such as
  `linked_opt_runtime_artifacts_build`, check whether CTest is building a
  prerequisite rather than hanging. Under ASAN that setup can take much longer
  than the actual focused tests.
* Do not stage generated `re2c/bootstrap/src/parse/parser.cc` or `.h` churn
  unless the task is specifically about those files.

Reference source-syntax lessons:

* The grammar has several expression spines. Prefix syntax such as
  `reference target` and `dereference ref` must be added to normal, command,
  and concatenation expression paths as appropriate, not only one convenient
  rule.
* Adding keyword trap productions can create large Lemon conflict spikes. Prefer
  the smallest grammar surface that parses the accepted forms and lets ordinary
  diagnostics handle rejected forms.
* `reference target` must validate that the operand is aliasable storage, not a
  temporary value expression. It must also reject references-to-references until
  nested reference policy is deliberately designed.
* `dereference ref` is a snapshot operation. It should preserve ordinary value
  copy semantics and should be the required spelling when passing a reference
  where a plain `.T` is expected.
* `refvalid(ref)` is only a validity preflight. Invalid use must still raise the
  VM's catchable `REFERENCE_INVALID` signal through operations such as `deref`,
  `linkref`, and `setref`.
* When emitting `mkref` from a linked child expression, append the child cleanup
  after the `mkref` instruction. Cleaning up first can unlink attribute/array
  storage before the reference is made.
* Treat references as rare on hot paths. Avoid making ordinary scalar/object
  operations chase references implicitly; use explicit opcodes such as `mkref`,
  `deref`, `linkref`, `setref`, and `refvalid`.
* Inline import/export is a danger area. Reference operations and
  reference-typed symbols should fail closed. Mutating methods and factories
  should not be blanket-disabled just because they write class attributes; rely
  on the inliner's receiver, copyback, and portable-attribute-shape checks so
  existing object/member optimisations and compiler exits keep working.
* Top-level executable assignments synthesize implicit `main`. In a file with
  an explicit `main: procedure`, initialize namespace-exposed globals inside
  `main` or another procedure rather than adding executable top-level
  assignments.

Useful focused commands from the reference source slice:

```sh
cmake --build cmake-build-debug --target rxc rxas rxvm rxbvm library
ctest --test-dir cmake-build-debug -R 'reference_source_' --output-on-failure
ctest --test-dir cmake-build-debug -R 'reference_(iterator|generated|source)|type_ops|arg_semantics_(scalar|array|object)|object_reference_regression|inline_test_ref_|inline_ref_array_count' --output-on-failure
cmake --build cmake-build-asan --target rxc rxas rxvm rxbvm library crexx_test_driver
ctest --test-dir cmake-build-asan -R 'reference_source_|reference_(iterator|generated)' --output-on-failure
```

### 8. Known Build and Platform Issues
When encountering unusual build or execution errors on new platforms (e.g., macOS ARM, Windows), keep these documented issues in mind:
*   **OpenSSL Resolution:** If CMake cannot find OpenSSL on macOS or Linux, ensure `CREXX_FORCE_SYSTEM_OPENSSL` is correctly handled. In `lib/plugins/socket/CMakeLists.txt`, hardcoded MSYS2 paths (`C:/msys64/...`) must be protected by an `if(WIN32)` check to prevent them from breaking path resolution on other OSs.
*   **Massive Memory Leaks / Explosions (50+GB):** If the compiler (`rxc`) or assembler unexpectedly consumes gigabytes of memory and hangs, suspect a failure in `file2buf` (`platform/platform.c` or `S370/cmsutil.c`). Endpoint security tools (like ThreatLocker) or stream abstractions can cause `ftell()` or `fseek()` to fail, returning `-1`. If this `-1` is cast directly to a `size_t` without error checking, it overflows to `SIZE_MAX`. A subsequent `malloc(*bytes + 2)` wraps around to `1`, and `fread()` then performs a massive heap buffer overflow trying to read `SIZE_MAX` bytes, corrupting allocator headers and causing the OS to infinitely allocate memory. Always ensure `ftell()` is stored in a signed integer (e.g., `long`) and checked for errors (`< 0`) before casting to `size_t`.
*   **Allocation Panic Diagnostics:** Core tools use the shared `RX_PANIC_OOM` / `RX_REPORT_OOM` helpers from `platform/platform.h` for unrecoverable allocation failures. The panic output includes the allocation operation, requested byte count, source file/line/function, `errno`, process id, and best-effort system memory status. On Windows this includes commit available to the current process from `GlobalMemoryStatusEx`, system commit total/limit/available from `GetPerformanceInfo`, physical memory, page-file total, virtual memory, and memory-load values; use these fields to distinguish a genuinely unreasonable request from process commit headroom or system-wide commit pressure during parallel builds.
