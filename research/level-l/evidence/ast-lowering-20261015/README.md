# Checked lowering qualification

AST7-STEP02 / AST7-AC02, Darwin arm64, 15 October 2026.

- Successful product build and fresh warning-free functional target rebuild.
  The earlier product/test log includes test-local binding warnings corrected
  before the final build; retained logs normalize trailing whitespace only.
- 20 unchanged AST/condition controls passed; all 16 lowering tests passed on
  the final example module (four backends, two consumer modes, two VMs).
- Controls compare original Unless and lowered If execution with explicit
  expected traces for both Boolean values, nested syntax and flow facts. They
  cover a 2,000-level tree, missing translations, schema failures, stale/foreign
  owners, immutable intermediates, sharing, budget failure and atomic finish.
- Standalone no-opt Level B bootstrap passed. Source/RXAS output matched the
  optimized generator at L0/L1/L2. 42 no-opt-library delivery commands plus 12
  installed-wrapper commands passed, including combined and separate library
  use, both file extensions, incremental no-op and both VMs.

Input hashes and command/image receipts are retained. This increment is intended
for immediate separate develop publication after these local gates. Normal
hosted Build/CodeQL are separate post-publication checks, not claimed here.
