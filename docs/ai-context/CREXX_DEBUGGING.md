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
1. Compile: `./rxc test.rexx` (produces `test.rxas`)
2. Assemble: `./rxas test.rxas` (produces `test.rxbin`)
3. Execute: `./rxvm test.rxbin`

### 5. Known Build and Platform Issues
When encountering unusual build or execution errors on new platforms (e.g., macOS ARM, Windows), keep these documented issues in mind:
*   **OpenSSL Resolution:** If CMake cannot find OpenSSL on macOS or Linux, ensure `CREXX_FORCE_SYSTEM_OPENSSL` is correctly handled. In `lib/plugins/socket/CMakeLists.txt`, hardcoded MSYS2 paths (`C:/msys64/...`) must be protected by an `if(WIN32)` check to prevent them from breaking path resolution on other OSs.
*   **Massive Memory Leaks / Explosions (50+GB):** If the compiler (`rxc`) or assembler unexpectedly consumes gigabytes of memory and hangs, suspect a failure in `file2buf` (`platform/platform.c` or `S370/cmsutil.c`). Endpoint security tools (like ThreatLocker) or stream abstractions can cause `ftell()` or `fseek()` to fail, returning `-1`. If this `-1` is cast directly to a `size_t` without error checking, it overflows to `SIZE_MAX`. A subsequent `malloc(*bytes + 2)` wraps around to `1`, and `fread()` then performs a massive heap buffer overflow trying to read `SIZE_MAX` bytes, corrupting allocator headers and causing the OS to infinitely allocate memory. Always ensure `ftell()` is stored in a signed integer (e.g., `long`) and checked for errors (`< 0`) before casting to `size_t`.