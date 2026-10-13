# Resource, Unicode and diagnostic continuation — 12 October

This is the native `levell` continuation checkpoint for CONT-AC01–04 in
[PLAN.md](../../PLAN.md). The complete LANG and original research obligations
remain open. No integration, release, cross-platform or sanitizer qualification
is asserted by these functional checks.

| Evidence | Verified scope |
|---|---|
| `../resource-cleanup/limits-fixed/receipt.json`, `limits-noopt/receipt.json` | 35 commands in each library mode, successful-artifact invariance, lower-bound diagnostics, format ceilings, large-graph loader independence and 24 native boundary/loader checks per mode. |
| `../resource-cleanup/final-budgets/receipt.json`, `final-budgets-noopt/receipt.json` | 114 request cases and seven generator controls per mode across all initial parser routes and regional/recovery requests. |
| `../unicode-surface/property-data/receipt.json`, `property-data-noopt/receipt.json` | All 45 supplied property sets against independently read raw Unicode data; four native cells per library mode. Corrupt input checksums preserve the good output. |
| `../unicode-surface/lexer/receipt.json`, `lexer-noopt/receipt.json` | 12,523 original-byte input fixtures in eight cells: every XID interval edge/neighbor, deterministic random scalars, NUL, empty input and prefixed continuation membership. Unknown/wrong-unit properties fail explicitly. |
| `../unicode-surface/worked-example/receipt.json` | The seventh settings lesson, with full XID names, fields and Pratt/LR expressions, runs through source/RXAS on both VMs. |
| `../diagnostic-continuation/final-coordinates/receipt.json`, `final-coordinates-noopt/receipt.json` | 78 cases per mode, all six parser routes, exact byte/scalar/escaped-marker positions, CR/CRLF/EOF, authored context and speculative PEG suppression. |
| `../diagnostic-continuation/final-recovery/receipt.json`, `final-recovery-noopt/receipt.json` | 160 cases per mode plus two proper-reader controls: malformed UTF-8, source ordering, lexical/EOF damage, clean invariance and later records. |
| `../diagnostic-continuation/origins/receipt.json` | Eight native cells verify original bytes and error nodes associated with source-sorted messages. |
| `../diagnostic-continuation/specification/receipt.json`, `specification-noopt/receipt.json` | 24 declaration-candidate cases and twelve last-good-output controls per mode across quote/comment/group damage. Quoted/commented headers remain opaque. |
| `../resource-cleanup/frontend/receipt.json` | Seventeen independent controls across L0/L1/L2; identical optimized/unoptimized self-description. |
| `lifecycle/receipt.json` | Fresh standalone source copy, cold/no-op build, new Level B property generation and exact fixed point; no Level G directory or re2c invocation. |
| `../diagnostic-continuation/import-isolation/receipt.json` | Intentionally obsolete diagnostic metadata remains beside output files during a forced consumer compile; native packing and both interpreter consumers pass using exact current import roots. |
| `../unicode-surface/gennorm2/receipt.json` | Seventeen existing Unicode-workload inputs in eight cells; unchanged tokens/records/errors except the already documented legacy NUL correction. |
| `consumer-ctest.log`, `generated-current-ctest.log`, `noopt-interpreter.log` | Current RexxScript consumers pass both VMs; the rebuilt generated-image/ownership consumer passes both VMs. Complete unoptimized script/Level L modules pass all sixty interpreter cases on both VMs. |
| `../diagnostic-continuation/rexxscript-cli/receipt.json` | Six native CLI cases, including parse rejection before execution. |

`inputs.json` identifies source and executable/artifact inputs. Product and
standalone builds are warning-free in the retained current logs. Intermediate
warnings and fixture mistakes are not counted as passes.

The final constructor cleanup replaces a shadowed seed-array redeclaration with
an explicit empty value. `../resource-cleanup/artifact-preservation/receipt.json`
regenerates all 32 retained coordinate/recovery/property artifacts byte-identically.
Parser, scanner, recovery and formatter runtime sources did not change after
their panels, so the retained executions remain valid for those exact artifacts.
The source hash manifest records the final constructor too; it does not silently
pretend that an earlier run used that constructor's source bytes.

The first full-XID attempt exposed copying/full-NFA traversal costs and was
stopped during concurrent functional work. No formal speed comparison follows.
The sparse queue/key preservation argument is in MATH.md; subset construction
can still grow exponentially. Source/work/count settings are engineering
safeguards, not global time or resident-memory guarantees. Default workload
calibration, normalization/folding keys, display-cell/grapheme views and general
nested or token-stage recovery remain open. Unclosed target comments still stop;
proper-reader declaration candidates after lexical damage are inspection data
and never become generated code.
