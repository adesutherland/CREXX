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

### 7. Known Build and Platform Issues
When encountering unusual build or execution errors on new platforms (e.g., macOS ARM, Windows), keep these documented issues in mind:
*   **OpenSSL Resolution:** If CMake cannot find OpenSSL on macOS or Linux, ensure `CREXX_FORCE_SYSTEM_OPENSSL` is correctly handled. In `lib/plugins/socket/CMakeLists.txt`, hardcoded MSYS2 paths (`C:/msys64/...`) must be protected by an `if(WIN32)` check to prevent them from breaking path resolution on other OSs.
*   **Massive Memory Leaks / Explosions (50+GB):** If the compiler (`rxc`) or assembler unexpectedly consumes gigabytes of memory and hangs, suspect a failure in `file2buf` (`platform/platform.c` or `S370/cmsutil.c`). Endpoint security tools (like ThreatLocker) or stream abstractions can cause `ftell()` or `fseek()` to fail, returning `-1`. If this `-1` is cast directly to a `size_t` without error checking, it overflows to `SIZE_MAX`. A subsequent `malloc(*bytes + 2)` wraps around to `1`, and `fread()` then performs a massive heap buffer overflow trying to read `SIZE_MAX` bytes, corrupting allocator headers and causing the OS to infinitely allocate memory. Always ensure `ftell()` is stored in a signed integer (e.g., `long`) and checked for errors (`< 0`) before casting to `size_t`.
*   **Allocation Panic Diagnostics:** Core tools use the shared `RX_PANIC_OOM` / `RX_REPORT_OOM` helpers from `platform/platform.h` for unrecoverable allocation failures. The panic output includes the allocation operation, requested byte count, source file/line/function, `errno`, process id, and best-effort system memory status. On Windows this includes commit available to the current process from `GlobalMemoryStatusEx`, system commit total/limit/available from `GetPerformanceInfo`, physical memory, page-file total, virtual memory, and memory-load values; use these fields to distinguish a genuinely unreasonable request from process commit headroom or system-wide commit pressure during parallel builds.
