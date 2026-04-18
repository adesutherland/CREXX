# cREXX DSLSH Integration

This document is the enduring reference for `rxc --syntaxhighlight` as
implemented today. It covers the editor-facing contract with DSL Syntax
Highlighter (DSLSH), the parser-mode design in
`compiler/rxcp_highlight_controller.c`, and the current roadmap gaps.

For the wider compiler pipeline and the exact point where the immutable source
tree is created, see [Parsing Pipeline Anatomy](parsing_pipeline_anatomy.md).
For the top-level toolchain overview, see
[cREXX Architecture](../../docs/ai-context/CREXX_ARCHITECTURE.md).

## Scope

This document is about the cREXX compiler (`rxc`) parser-mode integration.

It does not attempt to describe:

- the general compiler pipeline in full,
- the assembler (`rxas`) highlighter,
- hypothetical future protocol designs that do not exist in the current code.

The active implementation lives primarily in:

- `compiler/rxcp_highlight_controller.c`
- `compiler/rxcp_source_tree.h`
- `compiler/rxcp_source_tree.c`

## Implemented Architecture

### Shared parser, separate responsibilities

cREXX does not have a separate highlighting grammar. Normal compilation and
parser mode share the same lexer, Lemon grammar, and early source-shaping
passes.

After the authored source shape and source spans are known, the compiler builds
an immutable `SourceNode` tree. From that point onward:

- `context->source_tree` is the canonical user-facing tree for authored
  structure, diagnostics, semantic sidecars, metadata anchors, and DSLSH
  projection.
- `context->ast` / `work_ast` remains the mutable compiler tree for import
  loading, exit dispatch, fixed-point rewrites, optimization, and emission.

This split is deliberate. The source tree is not a second mutable compiler AST,
and it must not be treated as the producer of later compile-time rewrites.

### Provenance and source-owned state

The mutable work tree carries explicit links back to source-owned state:

- each `ASTNode` has a primary `source_node` pointer,
- `source_provenance` records whether that mapping is exact, inherited,
  synthetic, or composite,
- transformed output paths can preserve additional reporting anchors where a
  removed authored construct still needs to appear in diagnostics or `.src`
  metadata.

User-visible state lives on the source tree:

- `source_tree_sync_diagnostics()` rebuilds canonical source-owned diagnostics
  from the current AST and detached diagnostics,
- `source_tree_sync_semantics()` mirrors final symbol and type conclusions onto
  immutable `SourceNode` sidecars.

### Highlight controller

`rxc --syntaxhighlight` routes through `compiler/rxcp_highlight_controller.c`.

The controller emits DSLSH from `source_tree`, not from the rewritten
`work_ast`. It also owns parser-mode-specific retained state for import and
exit caching, token projection, comment recovery, and editor-facing diagnostic
overlay.

## Parser-Mode Request Lifecycle

The current request flow is:

1. The editor applies the current document state to a DSLSH `CodeBuffer`.
2. The highlight controller derives the authored document identity from the
   `CodeBuffer` document id and captures the document path, source directory,
   current working directory, executable path, search-path key, and configured
   exit module.
3. The controller either reuses or invalidates a retained master context based
   on those inputs and watched filesystem state.
4. A fresh child parse context is created for the current request.
5. Parser mode runs the options pre-scan, parses the document, and syncs
   source-owned diagnostics.
6. If the source tree is syntax-clean, parser mode runs semantic validation and
   mirrors semantic conclusions back onto source-owned state. If syntax errors
   already exist, parser mode stays source-structure-only for that request.
7. The controller projects the source tree into a DSLSH tree, recovers comment
   and punctuation spans that are absent from the parser token stream, overlays
   diagnostics onto final leaf owners, and serializes the result.
8. DSLSH reconstructs the tree and flattens it into per-character editor
   attributes.

## DSLSH Features Used

cREXX parser mode currently uses these DSLSH concepts:

| DSLSH feature | Used by cREXX | cREXX mapping |
| --- | --- | --- |
| `PARSE_TREE_FILE` root | yes | whole document |
| Structural container nodes | yes | projected from immutable source-tree containers |
| Leaf token categories | yes | projected from lexer tokens, with semantic overrides |
| `identifier_id` | yes | mirrored from source-owned semantic state |
| `severity` | yes | syntax and validation diagnostics |
| `message` | yes | syntax and validation diagnostic text |
| `SYNTAX_ERROR` nodes | yes | explicit diagnostic nodes in the serialized tree |
| `message_code` | not yet | not populated by cREXX |
| `PARSE_TREE_COMMENT` blocks | not yet | comments are emitted as leaves only |

There is no separate hover, completion, code-action, or streaming side channel
in the current integration. The editor receives one parse-result snapshot per
request.

## cREXX to DSLSH Mapping

### Root and container structure

The controller now projects structure from the immutable source tree rather than
from the rewritten work AST.

Current source-node to DSLSH container mapping:

| cREXX source node | DSLSH container |
| --- | --- |
| file root | `PARSE_TREE_FILE` |
| `CLASS_DEF` | `PARSE_TREE_STRUCTURE` |
| `PROCEDURE`, `METHOD`, `FACTORY` | `PARSE_TREE_FUNCTION` |
| `INSTRUCTIONS`, `DO`, `SELECT`, `WHEN`, `OTHERWISE` | `PARSE_TREE_CODEBLOCK` |
| `IF`, `ASSIGN`, `CALL`, `RETURN`, `EXIT`, `EXIT_EXTENDED`, `IMPLICIT_CMD`, `SAY`, `PULL`, `PARSE` | `PARSE_TREE_STATEMENT` |
| `BLOCK_EXPR` and infix operators | `PARSE_TREE_EXPR` |

These container nodes are the basis for:

- fold discovery,
- outline-style navigation,
- subtree-aware selection,
- future scope-aware editor features.

### Leaf token mapping

cREXX starts from compiler lexer categories, then refines those leaves with
source-owned semantic state.

| cREXX token or semantic case | DSLSH leaf token |
| --- | --- |
| unknown or generic authored identifier | `LEXER_IDENTIFIER` |
| resolved function, procedure label, or call | `LEXER_FUNCTION_IDENTIFIER` |
| resolved type or class identifier | `LEXER_TYPE_IDENTIFIER` |
| resolved constant identifier | `LEXER_CONSTANT_IDENTIFIER` |
| strings | `LEXER_STRING_LITERAL` |
| numeric literals | `LEXER_NUMBER_LITERAL` |
| keywords and flow syntax | `LEXER_KEYWORD` |
| imports, namespace, and options | `LEXER_PREPROCESSOR` |
| assignment | `LEXER_OPERATOR_ASSIGN` |
| arithmetic operators | `LEXER_OPERATOR_ARITHMETIC` |
| logical and comparison operators | `LEXER_OPERATOR_LOGICAL` |
| punctuation | `LEXER_SEPARATOR`, `LEXER_LH_EXPR`, `LEXER_RH_EXPR`, `LEXER_STATEMENT_SEPARATOR` |
| comments recovered from source gaps | `LEXER_COMMENT` |

Important implementation details:

- semantic state can override raw lexer categories,
- authored procedure labels that lex as `TK_UNKNOWN` can still project as
  semantic function identifiers,
- repeated uses of the same authored symbol can share one non-zero
  `identifier_id`,
- dotted stem-tail segments are projected explicitly rather than being left to
  accidental gap filling.

### Comment and gap recovery

The compiler lexer intentionally skips comments rather than emitting them as
normal parser tokens. Parser mode therefore recovers missing spans from source
gaps during projection.

That recovery currently covers:

- nested `/* ... */` block comments,
- configured line-comment styles from parsed source options,
- punctuation or separators that are not represented by a direct projected leaf
  token.

## Diagnostics As Implemented

### What cREXX emits

cREXX emits diagnostics in two forms:

1. explicit `SYNTAX_ERROR` nodes in the DSLSH tree,
2. mirrored `severity` and `message` attributes on overlapping projected leaf
   tokens.

The second form exists because DSLSH flattens the parse tree into one final
owner per character for rendering. Without the overlay step, a later token leaf
can replace the per-character diagnostic severity from an earlier explicit error
node. cREXX therefore keeps the explicit diagnostic nodes and also mirrors the
best overlapping diagnostic onto the final leaf owner.

Current diagnostic mapping:

| cREXX state | DSLSH mapping |
| --- | --- |
| source-owned or detached `ERROR` | `CB_ERROR` plus message |
| source-owned or detached `WARNING` | `CB_WARNING` plus message |
| internal or generated-code diagnostic | message text explicitly labelled so the editor can treat it as generated-context reporting |

When multiple diagnostics overlap one leaf, the controller currently chooses:

1. higher severity first,
2. then the narrowest span,
3. then the earliest position.

### Current limitations

- `message_code` is not populated.
- DSLSH effectively exposes one diagnostic owner per character after flattening.
- Multiple overlapping diagnostics at the same position cannot all survive as
  separate editor-visible leaf states.
- There is no related-location or note chain for secondary spans.

## Caching and Invalidation

Parser mode keeps a retained master context inside the highlight controller so
repeated highlight requests in one parser process do not start from an empty
import and exit world on every keystroke.

Retained state currently covers:

- import inventory,
- loaded external symbol state carried by the retained root context,
- exit discovery readiness and loaded exit module state,
- watched directories and watched files used for invalidation.

The cache is invalidated when relevant inputs change, including:

- document identity,
- effective search-path key,
- configured exit-module name,
- watched import or module timestamps and sizes.

The controller also remembers when exit discovery has already completed for a
generation, including the valid "no exits discovered" case, so it does not
rediscover exits unnecessarily.

Each request still gets a fresh child parse context. The cache reduces setup and
discovery churn; it does not turn parser mode into one long-lived mutable work
tree.

## Folding Contract

DSLSH folding is not driven by a separate fold-range protocol. It is derived
from structural parse-tree nodes:

- the editor sees `PARSE_TREE_*` nodes,
- DSLSH marks the first character of a subtree with `subtree_type`,
- DSLSH records subtree height in `subtree_lines`.

This means folding needs support on both sides.

### cREXX side

cREXX must emit stable container nodes that reflect authored structure. That now
exists for file, structure, function, code block, statement, and expression
regions.

### Editor side

The editor decides which subtree types are actually foldable. In practice,
folding will usually want:

- `PARSE_TREE_STRUCTURE`,
- `PARSE_TREE_FUNCTION`,
- `PARSE_TREE_CODEBLOCK`,
- optionally `PARSE_TREE_COMMENT` if cREXX starts emitting grouped comment
  blocks later.

`PARSE_TREE_STATEMENT` and `PARSE_TREE_EXPR` are useful as structural markers
but are not usually good default fold targets.

### Folding gaps

- cREXX does not yet emit `PARSE_TREE_COMMENT`, so grouped multi-line comment
  folding is missing.
- There is no explicit fold kind or fold label beyond the DSLSH subtree type.
- There is no parser-supplied region name for folds such as procedure
  signatures or class headers.

## Implementation Map

The current implementation is spread across a small number of files:

| File | Responsibility |
| --- | --- |
| `compiler/rxcp_highlight_controller.c` | parser-mode entry, retained cache, DSLSH projection, comment recovery, diagnostic overlay |
| `compiler/rxcp_source_tree.h` / `compiler/rxcp_source_tree.c` | immutable source tree, source-owned diagnostics, source-owned semantic sidecars |
| `compiler/rxcp_val_orch.c` | build order for source-tree creation and validation pipeline |
| `compiler/rxcpmain.c` | normal compiler path sync of source-owned diagnostics and semantics |
| `compiler/tests/src/test_highlight_cache.c` | retained-cache regression coverage |
| `compiler/tests/src/test_source_semantics.c` | semantic sync and `identifier_id` coverage |
| `compiler/tests/src/test_highlight_editor_diagnostics.c` | real editor-path diagnostics coverage |

## Capability Gaps and Roadmap

### Available in DSLSH, not yet used by cREXX

- `message_code` for stable diagnostic identifiers,
- broader use of `identifier_id` for more symbol categories and editor actions.

### Likely needs DSLSH protocol or editor-model work

- multiple diagnostics per position or per leaf,
- related-location diagnostics and secondary notes,
- hover payloads with symbol or type details,
- go-to-definition and find-references data,
- rename support that reuses `identifier_id` plus scope metadata,
- completion items, signature help, and completion documentation,
- code actions or fix-its with replacement ranges,
- richer semantic token kinds than the current DSLSH leaf enum,
- progressive analysis updates instead of one parse-result snapshot,
- AI-driven editor messages that arrive in several updates or revisions.

### Likely cREXX follow-up work

- populate `message_code` from stable parser and validator identifiers,
- decide which fold kinds should be promoted from editor heuristics to explicit
  emitted structure,
- add grouped comment containers if multi-line comment folding is wanted,
- expand source-owned semantic metadata so hover, completion, and navigation can
  reuse the same source-tree path.
