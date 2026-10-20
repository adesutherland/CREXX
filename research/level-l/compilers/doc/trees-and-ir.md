# Purposeful source trees and semantic tables

The source trees below are implemented by the C/G frontends.
The implemented generic IR records and storage are defined in
the [Level L IR design](../../../../langtools/doc/ir-design.md). This document describes the
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

## Implemented source schemas

The two `.rxl` files are the schema source of truth. Every child belongs to one
owned AST; node handles borrow its lifetime. Text retains source spelling. All
nodes retain exact source byte ranges and origin kind. Nonterminal ranges may
include trivia before the next token; named token captures retain only their
spelling, so comments cannot change a semantic field. `?`, `*` and `+` below
mean optional, zero-or-more and one-or-more children. A missing optional child
has count zero, rather than a fake value.

| Kind | Fields | Purpose |
|---|---|---|
| Program | `body: Sequence`; G also `routines: GProcedure*` | Top-level unit; Classic labels remain in the statement sequence. |
| Sequence / Block (G) | `items: node*` / `body: Sequence` | Authored order / typed-language DO group. |
| Number / String / Name | `spelling: text` | Exact numeral, quoted literal (including an adjacent Classic X/B suffix) or unresolved name. |
| Chain | `operands: node+`, `operators: node*` | One precedence level, left-to-right order. Exactly one fewer operator than operand. A zero-operator chain is transparent during lowering. |
| Operator tokens | no fields; kind is `plus`, `minus`, `times`, `divide`, `assign`, `ne`, `lt`, `le`, `gt`, `ge`, `strict`, `concat`, `blank`, `abut`, `and`, `or`, `xor`, `not` | Operator identity and its original span; `assign` means equality when used in a comparison chain. |
| Prefix / Group | `operator: node, value: node` / `value: node` | Unary operation / explicit parentheses. |
| Assignment | `name: text`, `value: node` (optional for C) | Assignment is separate from declaration. Missing C RHS means empty value. |
| IfThen / IfElse | `condition: node, yes: node`; IfElse adds `no: node` | Matched/unmatched grammar preserves nearest-IF ELSE association. |
| While (G) | `condition: node, body: Sequence` | Typed-language pre-test DO WHILE. |
| Leave / Iterate / Nop | no fields | Innermost loop transfer / no operation. |
| Say / Return / Exit | `value: node?` | Optional authored value; each instruction has its own semantics. |
| Call / Invoke | `name: text, arguments: Arguments` | Expression call / CALL instruction. |
| Arguments / Argument | `items: Argument+` / `value: node?` | Positional presence. A single empty Argument represents a zero-argument call; two empty entries are two omitted positions. |
| COptions | `value: node?` | Classic retains every optional expression, including the first bare-word header. Only that complete initial clause selects static settings; all operands keep ordinary evaluation. |
| Options (G) | `level: text` | If present, must select G and start the source. |

Each expression precedence level produces a Chain, including transparent
single-operand levels. These preserve the selected grammar structure; the
mapper does not emit operations for transparent wrappers. Operator order is
never inferred from a flattened text expression.

### G nodes

| Kind | Fields |
|---|---|
| GDeclare | `name: text, type: Type` |
| Type | `spelling: text` |
| Cast | `value: node, targets: Type*`, applied in authored order |
| GProcedure | `name: text, result: Type, body: Sequence` |
| GArg | `items: node+` containing the parameter alternatives below |
| Parameter / OptionalParameter | `name: text, type: Type` (required / primitive default) |
| DefaultParameter | `name: text, value: Number/String/Prefix` (literal default) |
| Presence | `spelling: text`, retaining `?name` |
| GNamespace / Export | `name: text, exports: Export+` / `name: text` |
| GImport | `name: text` |

A procedure's first GArg declares its positional parameters. Body intervals,
parameter/result types, optional flags, default parameter nodes and export
identities are kept in the routine table. Namespace exports are child nodes,
not a second text-list representation. The call inventory records only observed
target/presence shapes and their selected scalar contracts. G declarations and
inferred bindings have separate scope/name/type/default records. The mapper
creates typed IR slots and entry stores, rather than treating a typed declaration
as an uninitialised C-language variable.

### Classic nodes

Classic DO uses `CDo(header:DoHeader, body:Sequence, ending:DoEnd)` for all
grouping and repetitive forms. Optional repetition/condition children, raw
control/END names and ordered modifiers follow the
[approved DO schema](architecture.md#approved-do-contract); G's Block/While
schema is unchanged.

| Kind | Fields |
|---|---|
| CLabel | `name: text` |
| CProcedure | no fields |
| CArg | `templates: ParseTemplates`, exactly one required child; implicit ARG source/UPPER is private lowering policy |
| CNumeric | `setting: text, value: node?` |
| CForm | `form: text`, the optional SCIENTIFIC/ENGINEERING token spelling; empty means reset |

CArg reuses the existing ParseTemplate and Parse* item shapes; `ArgTarget`
and `CArg.items` are absent. Its complete authored instruction span, every
template/item span and empty boundary remain exact, without a fabricated UPPER
or ParseSource token. The [approved ARG contract](architecture.md#proposed-arg-contract)
also retires the scalar read operation/export in C v8/53 operations and the
33-export target. The plan tracks instruction status and remaining shared work.

CLabel and CProcedure remain separate nodes. The routine table recognises an
admitted label followed immediately by PROCEDURE and records the interval up
to the next label. It rejects shared-pool labels and reachable fall-through into
a label. Dynamic source names retain authored spelling for shared Classic pool
resolution. They never become G
local variables or acquire typed default semantics.

### Where IF's fields come from

The actual matched production is:

```rexx
if.closed: rule = ifword expression as condition thenword closed.statement as yes elseword closed.statement as no
  then tree IfElse(condition = condition, yes = yes, no = no)
```

The right sides `condition`, `yes` and `no` refer to captures in that rule.
The left sides name fields of the new IfElse node. Each field contains the AST
produced by the captured rule; it is not a separately declared flow node.
The condition is an ordinary expression tree. Lowering emits its computation,
converts the language value to a core Boolean, and creates a branch with two
block targets. A later checked constant fact can choose an edge without erasing
observable condition evaluation. See the [generated walkthrough](pipeline-walkthrough.md).

## Side tables and invariants

| Table | Key, value and invalidation |
|---|---|
| Binding | Request-local scope/name -> stable binding index, type and explicit-declaration flag. G use sites resolve through this table; C retains dynamic keys. Rebuilt for each request. |
| Type | Inferred G binding types reach a fixed point; expression types/conversions are queried during lowering. Missing is an error, never an arbitrary default. |
| Routine/loop | Case-insensitive routine name -> signature and body interval; a lowering stack holds innermost LEAVE/ITERATE block targets. |
| Lowering origins | IR owner/revision/operation ID -> owned source origin and source-AST identity when retained. Remains diagnostically useful if the AST is released. |
| Analysis | IR owner/revision/pass/semantic profile -> facts and provenance. Missing means unknown. |

The verifier checks field cardinality, kinds, reference ownership and all byte
ranges. Semantic checks establish binding, type and legal control positions.
Neither validator proves program equivalence. A rewrite must pass both the
structural checks and its transformation contract before replacing a result.

Register numbering is deliberately absent from source trees. RXAS register
assignment is a later map from typed slots and temporary values to target
locations, with separate call-window and cleanup checks.


## Implemented target preparation

The [RXAS backend](backend.md#structures-and-algorithm) reads the immutable graph
in two passes per function. Integer maps assign slots and result IDs to stable
registers; a separate contiguous window holds calls. Selected instruction
records carry mnemonic, already checked final operands and source origin. They
are rendered only through `llir.writer` sections. No target registers or RXAS
strings are stored in either source tree.

G decimal defaults are represented by `crexx.g.decimal.default`, with one decimal
result and no inputs. This keeps the VM's absent decimal (`nan` on display)
distinct from a decimal numeric zero. Exact float constants remain eight-byte
IR values through target selection. Classic argument, numeric and result packets
stay different IR types even though the target adapter uses the existing
activation class for their storage; only the appropriate operations consume
each type.
