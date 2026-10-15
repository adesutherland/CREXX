# Purposeful source trees and semantic tables

The source-tree designs below are approved for the forthcoming frontends.
The implemented generic IR records and storage are defined in
the [Level L IR design](../../doc/ir-design.md). This document describes the
compiler-owned source trees and their relationship to that IR.

## Each structure answers a different question

| Structure | Question it answers |
|---|---|
| Source and token stream | What exact bytes were written, and which words/operators do they form? |
| Selected parse | Which grammar rules recognised this complete input? |
| Language AST | Which language constructs did the programmer write? |
| Binding/type tables | Which declaration or dynamic name does this use mean, and what are its types? |
| Level L IR | In what order are operations evaluated, and where can execution go? |
| Analysis facts | What has been established about this specific IR revision? |
| RXAS preparation | Which target instructions, call windows and registers implement it? |

The source AST is an owned tree. Binding IDs, loop targets and analysis facts
are references in separate checked tables; adding them does not attach a child
to two parents. Language-specific AST schemas may reuse shapes without sharing
the meaning of every field or operator.

## Shared source shapes

`node` means one owned child, `node?` an optional child, and `node*` an ordered
list. Text is source spelling; it is not an already-converted number. Every
node has a source origin using the existing exact/composite/derived/synthetic/
recovered contract. Absent optional children have zero count, not fake nodes.

| Node | Fields | Meaning |
|---|---|---|
| Program / Block | `items: node*` | Authored order of contained statements/declarations. |
| Literal | `spelling: text`, `form: text` | Quoted string, integer, decimal-point or explicit decimal form. Source bytes preserve quotes and escapes. |
| Name | `spelling: text` | A source name before binding or pool resolution. |
| Assign | `name: text`, `value: node?` | Assignment; missing RHS is admitted only where the source language permits it. |
| Add / Compare / Logical | `left: node`, `right: node`; operator identity in kind or a declared text field | Source operator and operand order. A Logical node does not imply eager evaluation. |
| If | `condition: node`, `yes: node`, `no: node?` | Condition expression and its selected instruction/group arms. |
| While | `condition: node`, `body: node` | Test before each iteration. |
| Leave / Iterate | no target child in the first profile | A binding table identifies the innermost enclosing loop. |
| Say | `value: node?` | Output an expression or empty line. |
| Call | `name: text`, `arguments: node*`, `form: text` | Authored CALL versus expression call; entries preserve omitted positions. |
| Argument / MissingArgument | `value: node` / no fields | Present value versus omitted argument; empty string is a present value. |
| Return / Exit | `value: node?` | Return from the current invocation versus terminate the program. |

The following excerpt uses the existing attached-mapping style. It is a grammar
fragment: `ifword`, `expression`, `instruction` and other named rules must be
defined by the complete frontend grammar.

```rexx
conditional: rule = ifword expression as condition thenword instruction as yes (elseword instruction as no)?
  then tree If(condition = condition, yes = yes, no = no?)
```

`condition`, `yes` and `no` on the right are captures made by this rule. On the
left of each `=` they name fields in the produced If node. They are neither
flow node names nor external variables. This teaching fragment omits the
matched/unmatched productions required for complete ELSE association.

For `if total > 4 then say total else say 0`, the tree is:

```text
If
  condition: Greater
    left:  Name(spelling="total")
    right: Literal(spelling="4", form="integer")
  yes: Say
    value: Name(spelling="total")
  no: Say
    value: Literal(spelling="0", form="integer")
```

An existing Level L `branch condition then yes else no` flow mapping describes
potential paths through this tree. Compiler lowering additionally emits the
operations that evaluate `condition`, its language-specific Boolean conversion,
and an IR branch to explicit blocks. A fact that the condition is true can prune
an edge; it cannot erase a side-effecting condition evaluation.

## G additions

| Node | Fields and checking |
|---|---|
| Declare | `name: text`, `type: node` | Declared type and its default initialisation. A later assignment remains a separate node. |
| TypeName / Cast | `spelling: text` / `value: node`, `type: node` | Type syntax first, resolved type ID in a side table. |
| Procedure | `name: text`, `parameters: node*`, `result: node`, `body: node` | Parameter names/types, result type and owned body. |
| Import | `namespace: text` | Import a prebuilt contract; no implicit invocation of an old compiler. |

Binding assigns stable IDs to declarations and use sites. The first profile's
explicit locals get `core.slot` identities with the declared initialisation
semantics. An implicit function's entry and procedures are separate IR
functions. A typed declaration is not an uninitialised C-language local.

## Classic additions

| Node | Fields and checking |
|---|---|
| Label | `name: text` | Authored position in the instruction sequence. |
| Procedure | `exposures: node*` | A Classic instruction that changes the visible pool. Exposures are outside Phase A. |
| Arg | `positions: node*` | Simple target or omitted template at each argument position in Phase A. |
| Numeric | `setting: text`, `value: node?` | Dynamic numeric setting or reset. |

A Classic label and PROCEDURE remain separate nodes in Program.items. Classic
labels are not intrinsically functions. A semantic routine table recognises
the admitted label-plus-PROCEDURE entry, its body interval and return sites.
The first profile diagnoses other routine-entry/fallthrough forms. Later label
dispatch can extend that analysis without replacing this source tree with a
fictional G procedure tree.

Classic source variables stay pool names. Reads/writes lower to the Classic
operation family; they do not acquire the behaviour of typed slots. Unset-name,
argument presence, caller state and numeric context are retained explicitly.
Both operands of an eager logical operation are evaluated in source order.

## Side tables and invariants

| Table | Key, value and invalidation |
|---|---|
| Binding | AST owner/revision/node ID -> scope and declaration ID, or dynamic-name descriptor. Invalid after relevant source-tree reconstruction. |
| Type | AST owner/revision/node ID -> resolved type and required conversion. Missing means unchecked, never a guessed default type. |
| Routine/loop | AST owner/revision/node ID -> admitted entry/body/target. Targets must belong to the same source unit. |
| Lowering origins | IR owner/revision/operation ID -> owned source origin and source-AST identity when retained. Remains diagnostically useful if the AST is released. |
| Analysis | IR owner/revision/pass/semantic profile -> facts and provenance. Missing means unknown. |

The verifier checks field cardinality, kinds, reference ownership and all byte
ranges. Semantic checks establish binding, type and legal control positions.
Neither validator proves program equivalence. A rewrite must pass both the
structural checks and its transformation contract before replacing a result.

Register numbering is deliberately absent from source trees. RXAS register
assignment is a later map from typed slots and temporary values to target
locations, with separate call-window and cleanup checks.
