# Compiler and IR design decisions

These are the enduring decisions from [the compiler plan](../PLAN.md). They record reasons and
tradeoffs; the architecture/reference pages carry the actual contracts.

| ID | Decision | Reason | State |
|---|---|---|---|
| IR-D01 | Level L owns extensible IR structure, storage, views and generic services; compiler families own language meaning. | Other languages can use the same infrastructure without cREXX rules. | Approved; foundation implemented in STEP02. |
| IR-D02 | Compact numeric storage with named APIs and readable views. | Separate processing/storage cost from usability; exact constants and sparse facts retain their own representation. | Fixed-width row layout retained after the [current plan](../PLAN.md); representative performance remains open. |
| IR-D03 | Separate Classic and G source ASTs; binding and type facts in side tables. | Preserve source constructs and Classic label/PROCEDURE semantics without coercing them into G trees. | Approved design; implementation status below. |
| IR-D04 | Explicit typed slots and block-local temporary values before full SSA. | Straightforward loops, joins and verification; no phi placement/dominance machinery in the first backend. | Approved design; implementation status below. |
| IR-D05 | Plain Level B registration/build APIs before new IR declaration syntax. | Reuse familiar language facilities and test contracts before inventing another surface. | Approved design; implementation status below. |
| IR-D06 | Registered effect/type/evaluation/lowering contracts; unknown facts grant no optimisation permission. | Generic algorithms remain sound under their stated semantic premises. | Approved design; implementation status below. |
| IR-D07 | Generated DFA plus canonical LR(1) frontends; explicit Classic token classification. | Reuse established algorithms; handle unreserved words and expression/control syntax without speculative parser adoption. | Implemented and qualified for the admitted C/G scope. |
| IR-D08 | First profile is POC-01–10, using shared Level L IR and emitter services. | Exercise genuine semantic differences, local calls, loops and imports before full migration. | Approved design; implementation status below. |
| IR-D09 | Compiler-host Level B closure separate from generated-program runtime packages. | Current full `rxfnsc` packaging depends on G; importing it into the host would undermine bootstrap independence. | B/L host closure for families and both frontends verified separately from target-runtime metadata in STEP03. |
| IR-D10 | Preserve runtime error timing; a fold that would fail keeps the operation. | Dead-path arithmetic cannot become an unconditional compiler error. | Approved design; implementation status below. |
| IR-D11 | Retain native B as a seed; build proper B through Level L, then rebuild the full B/L/compiler closure twice to check convergence. | Make the normal B compiler self-hosted and expose every dependency on seed-built products. | Proper B destination clarified by Adrian; detailed stage/comparison procedure proposed for review. |
| IR-D12 | Build C and G compiler hosts independently from qualified B/L. | Avoid a G dependency in the foundation and C compiler; track the existing Classic target-runtime packaging dependency separately. | Independent C/G source/build closures verified in the directory reorganisation; target-runtime split remains separate. |

STEP02–06 implemented and qualified the bounded PoC, including backend
execution and safe optimisation. STEP07A adds the reviewed routine/import/default
contracts. The current independent C and typed/G owners use shared Level L IR,
writer and tool helpers; no common language-selecting main remains. Runtime
error timing is covered by reached/dead-path arithmetic regressions.

Proper B implementation and convergence under IR-D11 remain STEP07B, deferred
behind complete Classic delivery. Existing C/G PoC qualification does not close
full Classic, INTERPRET, replacement or platform acceptance. The plan owns those
open criteria. Preserve these decision IDs when changing their contracts.
