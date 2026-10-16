# Optional condition facts qualification

AST7-STEP01 / AST7-AC01, Darwin arm64, 15 October 2026.

- Product stage and functional executables built without new compiler warnings.
- 36 existing flow/structured-flow/AST controls passed unchanged. The initial
  16 new tests exposed two incorrect test expectations: SET always writes on
  the selected nested path, and a replaced reference may report expired.
- All 16 corrected condition tests pass: LL(1), LR(1), packrat and Earley,
  optimized/unoptimized consumers, threaded and switch VMs.
- The standalone no-opt Level B bootstrap passed. Source and RXAS generation
  matched optimized output at L0/L1/L2; 34 delivery commands passed against the
  no-opt library, including both consumer modes and both VMs.
- A fresh explicit-prefix installation passed 12 wrapper commands: `.rxl`,
  `.crexx` with `options levell`, combined program, separate library/program,
  incremental no-op and both VMs. The worked conditions example ran as documented.

`inputs.json` identifies qualified implementation/build inputs. Receipts retain
commands, outputs and image hashes. Normal hosted Build/CodeQL will run on the
published develop commit; these local results do not claim hosted completion.
This substantive commit is intended for immediate develop publication.
