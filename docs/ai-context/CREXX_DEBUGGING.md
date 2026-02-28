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