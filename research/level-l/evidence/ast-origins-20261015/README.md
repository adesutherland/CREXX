# Generated and recovered AST origins qualification

AST7-STEP03 / AST7-AC03, Darwin arm64, 15 October 2026.

- Product build and final functional build passed without new warnings.
- The regression panel passed 108 CTests: existing AST, analysis, flow,
  structured-flow, conditions, lowering, named projection and RexxScript AST
  checks, plus the first 16 origin tests. The final 20 origin tests passed after
  adding the LR/Pratt composite fixture: 112 distinct tests overall. Both VMs
  and both consumer optimization modes are included.
- The origin matrix covers LL(1), LR(1), packrat, Earley and LR/Pratt regional
  parsing, original-byte diagnostics, CRLF, Unicode, malformed UTF-8, metadata
  ordering/association, request release, copy independence, clean-input gates,
  synthetic/end/empty anchors, invalid origins/ranges and exact metadata budgets.
  It verifies preserved damage after rewriting, lowering and root replacement.
- 160 existing recovery scenarios and two specification-resync controls passed.
- The standalone no-opt Level B bootstrap passed. L0/L1/L2 source/RXAS output
  matched the optimized generator. 34 final no-opt-library delivery commands
  passed. Twelve installed-wrapper commands passed against the same unchanged
  runtime and example inputs: both extensions, combined/separate builds,
  incremental no-op and both VMs.
- Manual relative links passed. Logs normalize trailing whitespace only.

The small origin grammar admits ASCII identifiers and alpha explicitly to test
multibyte positions without rebuilding full Unicode property DFAs in each
backend fixture. Existing Unicode implementation and qualification are unchanged.
The composite fixture includes a real Pratt addition inside LR record recovery.

Input hashes identify this qualification. Existing strict RexxScript parse and
AST-status gates remain in place and their regression panel passed. This
substantive increment is intended for immediate develop publication. Normal
hosted Build/CodeQL are separate post-publication evidence, not a local claim.
