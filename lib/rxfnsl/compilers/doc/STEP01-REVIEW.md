# STEP01 review: C/G scope and Level L IR

Prepared and approved for STEP02 on 15 October 2026. The implementation has
not started in this baseline. [PLAN.md](../PLAN.md) remains the acceptance authority.

## Proposed decisions

1. Adopt [POC-01–10](poc-scope.md): real C/G scalar/control/call compilers, with
   explicit unsupported-feature errors and a shared RXAS backend.
2. Preserve [purposeful C/G source trees](trees-and-ir.md). Resolve names, types,
   routine entries and loop targets through checked side tables.
3. Adopt the first [Level L IR profile](../../doc/ir-design.md): blocks, typed
   slots and block-local immutable results, plus registered semantic families.
4. Use the proposed Level B schema/builder/service APIs. Keep the ordinary
   grammar's `then tree` surface. Review a new declaration language only if
   concrete use later justifies it.
5. Begin with the fixed-width numeric layout candidate, then compare it with
   parallel packed columns before selecting the working encoding. Preserve
   named views, exact constants and checked text/binary round trips.
6. Reuse generated DFA and LR(1) parsing, with explicit Classic contextual token
   classification and expression grammar. Demonstrate admission before claiming
   the frontend design works.
7. Use the [native-seed/proper-B bootstrap](architecture.md#bootstrap-native-seed-to-self-hosted-b-and-l): native B builds L and B1; B1 and then B2 rebuild the B/L/compiler closure to B2 and B3. Verify convergence and independent semantics. Build C and G compiler hosts from qualified B/L; retain the native B seed.
8. Keep C's compiler host independent of G. Treat the current Classic runtime
   package's G dependency separately; a fully B-only Classic runtime needs its
   own reviewed split. Proper B/self-hosting is an explicit later delivery under
   STEP07B, while the initial executable PoC remains the paired C/G slice.

## What is ready

| Review requirement | Evidence |
|---|---|
| C/G scope and broader coverage inventory | [Scope table and migration inventory](poc-scope.md). |
| Bootstrap/component ownership | [Architecture](architecture.md), including the actual `rxfnsc`/G packaging dependency. |
| Trees and links between fields, flow and IR | [Tree schemas](trees-and-ir.md) and [paired walkthrough](pipeline-walkthrough.md). |
| Compact IR, types, lifetime, extension and builder API | [Generic Level L IR design](../../doc/ir-design.md). |
| Algorithms, effects, proof assumptions and limits | [Algorithms](algorithms.md) and [decision ledger](design-decisions.md). |
| Real source syntax and observable language differences | [Six examples](../examples/step01/) and [30-command existing-tool receipt](../evidence/step01-20261015/README.md). |

The source checks are a comparison baseline, not a new-compiler or hosted QA
pass. The IR/API snippets are proposed. Storage efficiency, grammar admission,
extension implementation and target-code correctness remain open criteria.

## Authorised next action

STEP02 implements the generic Level L foundation
and the smallest independent operation-family consumer, with malformed-input,
lifetime, resource and round-trip regressions. It documents measured storage
choices and verifies the host dependency closure before either full frontend
is built. Architecture, tree/schema and algorithm pages change with the code.

Remaining Phase A work is STEP03 frontends, STEP04 complete RXAS backend,
STEP05 shared optimisations and STEP06 end-to-end qualification. Full migration,
proper B/self-hosting, INTERPRET and production replacement keep their later gates.
