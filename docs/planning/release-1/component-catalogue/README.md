# Release 1 Component Catalogue

Status: working catalogue through approved stages 2-7. Classifications remain provisional until the separate approval stage.

## Purpose

This catalogue records the user-visible CREXX language, library, BIF, class, plugin, compiler-exit, host-integration, tool, example, and proof-of-concept surfaces needed to make Release 1 product and packaging decisions. It is also the evidence base for later Level B core quality and performance re-engineering. That later implementation work is not part of this catalogue.

The catalogue separates:

- contract level;
- product role;
- delivery status;
- implementation composition and implementation source level;
- maturity, quality evidence, and assessment confidence.

A C-language implementation is not automatically a Level C contract. Level C Classic Rexx contracts are separate catalogue items even when a B/G item has the same name or can share implementation machinery.

## Canonical files

| File | Purpose |
|---|---|
| `raw-language-syntax.md` | Typed/compiler, Classic front-end, and RXPP syntax/capability discovery. |
| `raw-exports-rxfnsb.md` | Namespace-exposed Level B library source symbols. |
| `raw-exports-other.md` | Namespace-exposed class, G, C-runtime, L, vector, and RexxScript symbols. |
| `raw-members-*.md` | Public members of namespace-exposed classes and interfaces. |
| `raw-native-*.md` | Native RXPA `ADDPROC` registrations. |
| `raw-levelc-bifs.md` | Level C ANSI BIF names recognised by the compiler. |
| `raw-intrinsic-and-reserved-bifs.md` | Typed intrinsic, reserved, and documentation-only BIF names discovered during reconciliation. |
| `raw-rxpp-macros.md` | Shipped RXPP macro definitions and `##USE`-included procedures. |
| `raw-packages-and-demos.md` | Plugin directories, exits, tools, demonstrations, examples, and PoCs. |
| `discovery-reconciliation.md` | Stage 3 source/build/docs/test reconciliation and discrepancy register. |
| `purpose-index.md` | Stage 4 purpose-oriented component view. |
| `quality-assessment.md` | Stage 5 dependency, test, documentation, language-use, and performance-risk assessment. |
| `provisional-classification.md` | Stage 6 contract-level, product-role, delivery, composition, and maturity proposals. |
| `cross-cutting-conclusions.md` | Stage 7 conclusions, gaps, priorities, and decisions requiring approval. |
| `stage-reviews.md` | Completeness and task-compliance review performed after every stage. |

## Stable ID conventions

IDs describe identity, not the classification decision:

| Prefix | Entity |
|---|---|
| `SYN-` | Language or preprocessor syntax/capability. |
| `FN-`, `FAC-`, `CLS-`, `INT-`, `SYM-` | Namespace-exposed cREXX symbol. |
| `MEM-` | Public class/interface member. |
| `RXPA-` | Native RXPA function registration. |
| `CBIF-` | Classic/Level C BIF contract name. |
| `BIF-` | Typed intrinsic, reserved, or documentation-only BIF identity. |
| `MACRO-`, `RXPPPROC-` | RXPP macro or source-included procedure. |
| `PKG-` | Plugin or runtime package. |
| `EXIT-` | Compiler exit. |
| `TOOL-`, `LIB-`, `SRC-`, `INFRA-`, `DEMO-`, `EXAMPLE-`, `POC-` | Tool, residual library/source candidate, shared infrastructure, demonstration, example, or proof of concept. |

The same B/G contract can carry both levels. A Level C contract always has its own `CBIF-` or Classic-syntax entry and links to a related B/G entry rather than sharing it.

## Assessment vocabulary

Product role values are `bootstrap core`, `level core`, `standard`, `integration`, `optional`, `example`, `experimental`, `deprecated`, and `removal candidate`.

Delivery values are `required`, `default`, `opt-in`, `developer-only`, and `source-only/not shipped`.

Implementation values are `pure Rexx`, `pure RXAS`, `pure native`, `hybrid call path`, `mixed package`, and `generated`.

Performance is assessed as risk (`unknown`, `low`, `medium`, or `high`) unless repeatable measurements already exist. There is no composite quality score.

## Scope boundary

The leaf catalogue covers public contracts. Private Rexx helpers, static C helpers, internal compiler/assembler/VM functions, vendored third-party internals, individual test cases, and individual RXAS opcodes are evidence for a containing component rather than independent product entries. The existing RXAS reference remains the canonical opcode catalogue.

## Stage status

| Stage | State |
|---|---|
| 1. Approve taxonomy and boundaries | Approved by Adrian. |
| 2. Build raw catalogue | Complete and reviewed. |
| 3. Reconcile completeness | Complete and reviewed. |
| 4. Organise by purpose | Complete and reviewed. |
| 5. Initial assessment | Complete and reviewed. |
| 6. Provisional classifications | Complete and reviewed; not yet approved. |
| 7. Cross-cutting conclusions | Complete and reviewed; decisions are awaiting the later approval stage. |
