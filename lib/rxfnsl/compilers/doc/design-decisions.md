# Compiler and IR design decisions

This is the review record for [STEP01](../PLAN.md). It records reasons and
tradeoffs; the architecture/reference pages carry the actual contracts.

| ID | Decision | Reason | State |
|---|---|---|---|
| IR-D01 | Level L owns extensible IR structure, storage, views and generic services; compiler families own language meaning. | Other languages can use the same infrastructure without cREXX rules. | Approved; foundation implemented in STEP02. |
| IR-D02 | Compact numeric storage with named APIs and readable views. | Separate processing/storage cost from usability; exact constants and sparse facts retain their own representation. | Fixed-width row layout retained after the [STEP02 screen](../evidence/step02-20261015/README.md); representative performance remains open. |
| IR-D03 | Separate Classic and G source ASTs; binding and type facts in side tables. | Preserve source constructs and Classic label/PROCEDURE semantics without coercing them into G trees. | Proposed. |
| IR-D04 | Explicit typed slots and block-local temporary values before full SSA. | Straightforward loops, joins and verification; no phi placement/dominance machinery in the first backend. | Proposed. |
| IR-D05 | Plain Level B registration/build APIs before new IR declaration syntax. | Reuse familiar language facilities and test contracts before inventing another surface. | Proposed. |
| IR-D06 | Registered effect/type/evaluation/lowering contracts; unknown facts grant no optimisation permission. | Generic algorithms remain sound under their stated semantic premises. | Proposed. |
| IR-D07 | Generated DFA plus canonical LR(1) frontends; explicit Classic token classification. | Reuse established algorithms; handle unreserved words and expression/control syntax without speculative parser adoption. | Proposed; actual grammar admission must be demonstrated. |
| IR-D08 | First profile is POC-01–10, compiled by both frontends through one backend. | Exercise genuine semantic differences, local calls, loops and imports before full migration. | Proposed. |
| IR-D09 | Compiler-host Level B closure separate from generated-program runtime packages. | Current full `rxfnsc` packaging depends on G; importing it into the host would undermine bootstrap independence. | Initial B-only semantic-family closure verified in STEP02; full C/G semantics remain later work. |
| IR-D10 | Preserve runtime error timing; a fold that would fail keeps the operation. | Dead-path arithmetic cannot become an unconditional compiler error. | Proposed. |
| IR-D11 | Retain native B as a seed; build proper B through Level L, then rebuild the full B/L/compiler closure twice to check convergence. | Make the normal B compiler self-hosted and expose every dependency on seed-built products. | Proper B destination clarified by Adrian; detailed stage/comparison procedure proposed for review. |
| IR-D12 | Build C and G compiler hosts independently from qualified B/L. | Avoid a G dependency in the foundation and C compiler; track the existing Classic target-runtime packaging dependency separately. | B-only C host direction agreed; transitive closure and any runtime split remain to qualify. |

Adrian approved this foundation design and authorised STEP02 on 15 October
2026. Rows marked proposed describe the reviewed design, not implemented APIs.
STEP02 implements the reviewed graph/service APIs and records its physical
encoding comparison. Later compiler and bootstrap delivery gates remain in the plan. Changes preserve
these IDs and explain any approved replacement.
