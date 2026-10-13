# Common surface and host-budget checkpoint

Local `levell` work on 11 October 2026. The authoritative scope and open
acceptance criteria remain in [PLAN.md](../../PLAN.md). This is a bounded
functional checkpoint, not the completion of LANG-AC01–14 or a release.

The grammar keeps its ordinary field-name vocabulary, including keywords.
Resource policy belongs to the generator invocation or host request. Six
connected settings distinguish exhaustion from syntax/admission failure:
`lr.states`, `source.bytes`, `scan.work`, `parse.work`, `earley.cells` and
`tree.entries`. The 8,192-state default accommodates the 4,291-state
self-grammar; it is not a calibrated memory or elapsed-time guarantee.
Representation, recursive-stack and remaining fixed construction bounds are
documented in the user guide and remain separate work.

| Evidence | Verified scope |
|---|---|
| `budget-final/receipt.json` | Optimized library: 114 runtime cases across general parsers, Pratt, regions and recovery; seven generator/option controls. |
| `budget-noopt/receipt.json` | The same cases with the complete Level L library and generator compiled without optimization. |
| `capture-final-noopt/receipt.json` | 80 field/list/optional/message cases, keyword field names, three lexer refusals and four executions of the actual final settings example through source/RXAS and both VMs. |
| `recovery-final-noopt/receipt.json` | 112 target recovery cases plus two proper-reader multi-error controls; includes the Earley EOF-span repair. |
| `frontend-budgets/receipt.json` | 17 independent specification controls across L0/L1/L2, plus identical optimized/unoptimized generated self-description. |
| `lifecycle-budgets/receipt.json` | Nine isolated configure/cold/no-op/API-change/grammar-change/runtime-change/failure/restoration cases; no re2c invocation or Level G source tree. |
| `unicode-budgets/receipt.json` | 17 Unicode-workload inputs across eight VM/consumer-optimization cells. The previously documented embedded-NUL correction remains the sole comparison exception. |
| `rexxscript-cli/receipt.json` | Six native interpreter checks, including invalid-input and capability failures with no execution. |
| `consumer-ctest.log` | Both VM variants of the Level L generated-image/ownership tests and RexxScript interpreter tests: four passes. |

The budget panels check an exact source-size boundary, invalid settings,
distinct resource outcomes and the same artifact under a larger successful
construction budget. A conflicting grammar still fails LR admission after
raising the budget. Required counts are observed lower bounds, not predictions
of eventual completion. Token passes and recovery retries receive separate
operation allowances; the policy is not a global time or memory governor.

`host-budget-inputs.json` records source and executable identities. Product,
unoptimized standalone and consumer build logs accompany the receipts.
Earlier panels in this directory retain evidence for their own inputs;
`recovery-panel` and `recovery-noopt` contain earlier failed attempts and must
not be counted as passes. Their repaired successor receipts are named above.

The lifecycle fixture is a copied source tree. It deliberately extends the
frontend API while older linked drivers remain in all three stage directories,
then checks incremental/no-op/failed-generation behavior. Its source mutations
are test inputs, not changes to the product. The build uses private declared
imports, and the RexxScript consumer now invokes the verified L2 tool directly.
The fixture predates the last resource-message and integer-division refinements; its build/import
edges are identical. Current product builds and the separate specification
controls cover the final source snapshot. The lifecycle receipt records its
own fixture outputs and tools; it is not an assertion that the deliberately
mutated frontend has the product's source hash.

Pinned Unicode property/comparison policies, broader composition and recovery,
application-tree shaping, full resource migration/calibration and the rest of
the programme's open acceptance criteria are not established by these receipts.
The C compiler/assembler scanners are unchanged. No merge or publication is
implied.
