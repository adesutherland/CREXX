# STEP01 source-example checks

15 October 2026, Darwin ARM64. This is source-design evidence, not qualification
of the proposed compilers or IR.

Six review examples were compiled with the existing integration binaries, using
`rxc-c` for C and `rxc` for G. Compiler and assembler optimisation were disabled.
Each image was linked and run on `rxbvm` and `rxtvm`: 30 successful commands,
12 executions. Outputs matched the explicit `.expected` files.

The pipeline pair computes 5, calls `twice` to print 10, counts down to 0 and
calls the separately linked LENGTH implementation to print 3. The evaluation
pair shows Classic eager AND/OR and G short-circuiting. The numeric pair shows
Classic literal spelling, precision changes and unset-name behaviour; G shows
typed literal comparison, decimal arithmetic and a declared integer's default.
Inspection of G RXAS confirms an actual `rxfnsb.length()` call and imported
signature; the example does not rely solely on an intrinsic or folded answer.

[source-controls.json](source-controls.json) retains commands, tool/library and
source hashes, outputs, log locations and the planning HEAD. The executable
version banner is older than that HEAD and carries `dirty`; hashes identify
what ran. These checks validate the examples against available binaries, not an
exact-HEAD product build. No shared build tree was rebuilt or modified. Timing
fields are command receipts, not a performance claim.

Full source/AST/IR/RXAS parity and the new compiler regressions remain open in
[PLAN.md](../../PLAN.md). The proposed API and IR dumps are design examples and
have not been executed.
