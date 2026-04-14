# cREXX and rxas Syntax Highlighting Review

## Executive Summary

- `rxas` already has a usable parser-mode highlighter. It is simple and mostly flat, but it is structurally correct, produces diagnostics, and matches the assembler's source model well enough.
- `rxc` should be treated as a prototype, not a finished architecture. The current parser-mode implementation runs the full compile-time validation and rewrite pipeline, then throws most of that structure away and emits a flattened token stream.
- The best near-term solution is not a second grammar or a separate parser. It is a separate highlighting pipeline over a source-aligned AST.
- The best long-term solution is a dual-tree model with a source AST for editor/highlighting/diagnostics and a work AST for exits, sugar, lowering, inlining, and optimization.
- `is_compiler_added` and similar synthetic flags are useful, but they are not enough provenance on their own.

## Scope

This review compares:

- the current `rxc` syntax-highlighting path in `compiler/rxcp_dsl.c`
- the current `rxas` syntax-highlighting path in `assembler/rxas_dsl.c`
- the existing AST/source-location machinery in the compiler
- the DSLSH middleware expectations and capabilities

The evaluation criteria are:

- source fidelity
- editor safety and side effects
- syntax and diagnostic accuracy
- semantic power
- implementation cost
- usefulness for future error reporting

## Current State

### `rxas`

`rxas` uses a straightforward lexical projection:

- `assembler/rxas_dsl.c:143-146` creates a proper `PARSE_TREE_FILE` root.
- `assembler/rxas_dsl.c:152-175` emits leaf tokens directly from the assembler token stream.
- `assembler/rxas_dsl.c:177-239` converts assembler diagnostics into DSLSH `SYNTAX_ERROR` nodes with real source spans.
- `assembler/parsingtests` already exercises parser-mode behaviour through `parser_tester`.

This is simple, but for the assembler that simplicity is mostly fine:

- the assembler surface syntax is close to its execution model
- there is very little tree rewriting compared with `rxc`
- token-level classification is already useful

Limitations still exist:

- the tree is effectively flat
- there is little or no folding structure
- `identifier_id` is not populated
- semantic distinctions such as declaration vs reference are not projected

### `rxc`

The `rxc` path is very different:

- `compiler/rxcp_dsl.c:138-145` explicitly initializes exits and runs full `rxcp_val()`.
- `compiler/rxcp_dsl.c:147-220` then emits only a linear token stream and a diagnostics side list.
- `compiler/rxcp_dsl.c:149-166` never creates a `PARSE_TREE_FILE` root.
- `compiler/rxcp_dsl.c:154-167` drops tokens whenever `pos < last_end`.
- `compiler/rxcp_dsl.c:175-210` reads only `context->diagnostics_list`.
- `compiler/rxcp_dsl.c:241-249` seeds emergency parsing with a small hard-coded keyword/operator set.

That means the current `rxc` highlighter is not actually AST-based, even though the compiler already computes AST source positions.

## Verified Observations

I verified the current behaviour with `parser_tester`:

- Running `rxc --syntaxhighlight` on `tests/demo/hello.rexx` produced a DSLSH root node of type `LEXER_STATEMENT_SEPARATOR`, not `PARSE_TREE_FILE`.
- Running the same path on `tests/demo/syntaxerror.rexx` produced no `SYNTAX_ERROR` nodes, even though the source is intentionally incomplete.
- In that broken `rxc` sample, compound names such as `args.0` and `arg.i` were not projected cleanly; punctuation was being materialized by gap-filling rather than by an authoritative compiler token stream.
- Running `rxas --syntaxhighlight` on `assembler/parsingtests/test_04_errors.rxas` did produce `SYNTAX_ERROR` nodes with useful messages and ranges.

So the difference is not just "simple vs powerful". `rxas` is operationally correct. `rxc` currently has correctness gaps as well as architectural limits.

## Why `rxc` Is Harder Than `rxas`

The central issue is that the compiler tree is not stable:

- `compiler/rxcp_val_orch.c:935-939` performs early AST fixup and source-range propagation.
- `compiler/rxcp_val_orch.c:967-969` scans imports and initializes exits.
- `compiler/rxcp_val_orch.c:995-1202` enters a fixed-point loop with exit dispatch and rewrites.
- `compiler/rxcp_val_orch.c:1030-1035` rewrites implicit commands.
- `compiler/rxcp_val_orch.c:1150-1155` applies syntax sugar after types are known.
- `compiler/rxcp_val_orch.c:1181-1185` rewrites `ADDRESS`.

That is the right compile pipeline. It is the wrong editor pipeline.

For editor highlighting, running this full path has four direct problems:

- it is expensive
- it can have side effects
- it mutates the tree away from source structure
- it makes diagnostics harder to anchor to the original source

## Source Location and Provenance: What Already Exists

The compiler already has strong building blocks for a source-oriented solution.

### Existing Source Spans

`compiler/rxcp_val_check.c:350-438` computes:

- `token_start`
- `token_end`
- `source_start`
- `source_end`
- `line`
- `column`

This walker also expands ranges to include matching brackets where appropriate, which is useful for folding and error reporting.

### Existing Shallow Provenance

- `compiler/rxcp_ast_core.c:324-330` copies source-location fields in `ast_dup()`.
- `compiler/rxcp_ast_rewrite.c:124-133` lets rewrite-created nodes inherit source tracking from a chosen location node.
- `compiler/rxcp_util.c:676-735` applies a source map to exit-generated fragments before grafting.
- `compiler/rxcp_ast_core.c:335-341` marks compiler-generated blocks and suppresses some downstream behaviour.

This is useful, but it is still shallow provenance:

- it copies spans
- it does not establish durable origin links
- it does not distinguish exact origin from approximate origin
- it does not handle many-to-one and one-to-many rewrites in a principled way

## Main Findings

### 1. `rxas` and `rxc` should not share the same mental model

`rxas` can stay mostly token-stream based because the assembler syntax stays close to source.

`rxc` cannot rely on the same model because the compiler deliberately rewrites the tree and even executes exit code.

### 2. The current `rxc` parser-mode path is on the wrong abstraction boundary

The editor server should not be calling the full compile pipeline:

- not `rxcp_scan_imports()`
- not `rxcp_init_exits()`
- not exit dispatch
- not fixed-point convergence
- not rewrite-heavy lowering

An interactive highlight pass must be pure and source-facing.

### 3. The current `rxc` implementation has correctness bugs, not just missing features

The biggest ones are:

- malformed DSLSH root creation
- diagnostics collected from the wrong place
- token dropping through `last_end`
- too much dependence on flat gap-filling

### 4. Synthetic flags alone are not enough

`is_compiler_added` answers "is this generated?"

It does not answer:

- generated from which source node?
- generated from which call site?
- generated from which exit token?
- is this an exact source interval or an approximate inherited one?

That is not enough for robust error reporting on transformed code.

### 5. A completely independent second parser is not the right answer

A separate grammar would:

- duplicate syntax knowledge
- drift over time
- lose access to compiler-quality diagnostics
- still need a provenance story for semantic editor features

The better split is:

- separate highlighting pipeline
- shared parser and shared source-location model

## Option Comparison

| Option | Description | Power | Feasibility | Risk | Error Reporting Value | Verdict |
| --- | --- | --- | --- | --- | --- | --- |
| A | Stabilize the current flat token-stream model | Low to medium | High | Medium | Low | Good only as a short-lived stopgap |
| B | Dedicated source-AST highlighter pipeline | High | Medium to high | Low to medium | High | Best near-term direction |
| C | Dual-tree architecture: `source_ast` + `work_ast` | Very high | Medium | Medium | Very high | Best strategic direction |
| D | Keep one work AST, add more synthetic/origin spans | Medium | Medium | High | Medium | Not a good final architecture |
| E | Entirely separate standalone highlighter/parser | Low to medium | Medium | High | Low | Not recommended |

## Option Details

### Option A: Stabilize the Current Flat Token Model

This would mean:

- create a real `PARSE_TREE_FILE` root
- stop depending only on `diagnostics_list`
- remove or rethink `last_end` token dropping
- emit a more complete token mapping
- keep the output mostly flat

Pros:

- fast to start
- low code churn
- fixes obvious correctness problems

Cons:

- still weak on structure and folding
- still weak on semantic colouring
- still does not solve the source-vs-work-tree problem
- still encourages thinking in terms of post-rewrite tokens

Recommendation:

- do this only as Phase 0 recovery work, not as the end state

### Option B: Dedicated Source-AST Highlighter Pipeline

This means:

- parse source once
- build a source-aligned AST
- project that AST into DSLSH containers and diagnostics
- use a dedicated token callback for leaf tokens
- do not run exits or compile rewrites

Pros:

- high source fidelity
- safe for editor use
- enables folding and structural highlighting
- makes it possible to disambiguate context-sensitive tokens such as `=`
- naturally supports better diagnostics

Cons:

- needs a new projection layer
- needs a dedicated validation subset, not `rxcp_val()` as-is
- requires test coverage around tricky syntax shapes

Recommendation:

- this is the best near-term implementation target

### Option C: Dual-Tree Architecture

This means:

- maintain `source_ast` for source-facing features
- derive `work_ast` for exits, sugar, lowering, inlining, optimization, emission
- carry explicit origin links from `work_ast` back to `source_ast`

Pros:

- clean separation of concerns
- best foundation for diagnostics, editor tooling, and future refactors
- avoids forcing the editor to reason about transformed code
- gives later compiler stages a principled way to report source errors

Cons:

- touches more of the compiler core
- requires disciplined provenance updates in all rewrite paths
- needs a staged rollout

Recommendation:

- this should be the strategic architecture, even if it arrives after the near-term highlighter fix

### Option D: Single Work AST with Extra Synthetic Flags

This would extend the current model but keep only one mutated tree.

Pros:

- smaller refactor than full dual-tree
- reuses current compiler structure

Cons:

- still forces the editor and diagnostics to reason about transformed code
- origin gets muddled after repeated rewrites
- many-to-one and one-to-many transformations remain awkward

Recommendation:

- better than nothing, but not a durable answer

### Option E: Completely Separate Standalone Highlighter

This means a second cREXX parser or lexer dedicated only to editor features.

Pros:

- hard separation from compile pipeline
- can be made fast and side-effect free

Cons:

- duplicated grammar
- duplicated bug surface
- drift between compiler syntax and editor syntax
- weak diagnostic sharing

Recommendation:

- not recommended, except as a purely heuristic fallback; DSLSH emergency parsing already serves that role

## Recommended Direction

### Recommendation 1: Build a Dedicated `rxc` Highlighter Pipeline

Use a source-facing validation subset, not full `rxcp_val()`.

The easiest path is to add a new function, for example `rxcp_hvl()`:

- start from the shape of `rxcp_bvl()` in `compiler/rxcp_val_orch.c:1261-1288`
- remove passes that lower or distort source shape
- keep only source-faithful structure and symbol work

Suggested contents of `rxcp_hvl()`:

- `ast_structure_fixup_walker`
- `source_location_walker`
- `syntax_validation_walker`
- `structure_symbols_walker`
- `build_symbols_walker`
- `resolve_functions_walker`
- `exposed_symbols_walker`

Suggested exclusions:

- `rxcp_scan_imports()`
- `rxcp_init_exits()`
- `needs_rxsysb_walker`
- exit dispatch
- fixed-point loop
- `rewrite_implicit_cmd_walker`
- `syntax_sugar_walker`
- `rewrite_address_walker`
- most lowering passes

Special note on `rewrite_constructor_walker`:

- for editor highlighting I would exclude it initially
- it is a lowering step, not a source-shape step
- if some type information later depends on it, add a highlight-specific variant instead of reusing the compile-time form directly

### Recommendation 2: Make Parser Mode Side-Effect Free

Even if `rxcp_hvl()` is introduced, parser mode should also explicitly force safe behaviour:

- set `context->disable_exits = 1`
- do not scan imports in the interactive parse path
- do not attach project-wide semantics unless they come from a separate cache

Editor parsing should never execute bridge plugins on every keystroke.

### Recommendation 3: Add a Real AST-to-DSLSH Projection Layer

Add a dedicated module, for example `compiler/rxcp_highlight.c`, responsible for:

- creating a `PARSE_TREE_FILE` root
- traversing the source AST
- creating DSLSH container nodes
- emitting diagnostics from AST `ERROR` and `WARNING` nodes
- providing a custom token callback over `context->token_head`

This should follow the DSLSH "AST containers plus token callback" pattern rather than the current "flatten then patch gaps" pattern.

## Suggested Projection Design

### Root and Containers

Always create:

- `PARSE_TREE_FILE` for the whole file

Then add containers for selected AST node classes.

Suggested initial mapping:

| Compiler AST Node | DSLSH Container |
| --- | --- |
| `PROGRAM_FILE`, `REXX_UNIVERSE` | `PARSE_TREE_FILE` or `PARSE_TREE_SCOPE` |
| `NAMESPACE`, `CLASS`, `CLASS_DEF` | `PARSE_TREE_SCOPE` |
| `PROCEDURE`, `METHOD`, `FACTORY` | `PARSE_TREE_FUNCTION` |
| `INSTRUCTIONS`, `DO`, `SELECT`, `WHEN`, `OTHERWISE` | `PARSE_TREE_CODEBLOCK` or `PARSE_TREE_STATEMENT` |
| `IF`, `ASSIGN`, `CALL`, `RETURN`, `EXIT`, `ADDRESS`, `IMPLICIT_CMD` | `PARSE_TREE_STATEMENT` |
| arithmetic/comparison/logical expression nodes, `BLOCK_EXPR` | `PARSE_TREE_EXPR` |

Do not try to containerize every AST node. Start with the ones that materially improve folding and bracket ownership.

### Leaf Tokens

Do not rely on `cb_default_get_token_callback()` for cREXX.

Instead:

- build a custom token callback that reads the compiler token list
- map each compiler token to DSLSH leaf type
- return authoritative punctuation, brackets, separators, identifiers, keywords, literals, and operators

This avoids the current gap-filling artefacts.

### Diagnostics

Do not depend on `context->diagnostics_list` in parser mode.

Instead:

- walk the AST directly
- emit a DSLSH diagnostic node for every `ERROR` and `WARNING`
- anchor it to `token_start/token_end` or `source_start/source_end`

If later stages still want to prune diagnostics into a side list, that should be optional and explicit.

## Why AST Projection Gives More Power

The AST can answer questions the token stream cannot.

Examples:

- `=` can be rendered as assignment or comparison depending on AST context
- a name can be distinguished as type, function, class, variable target, or variable reference
- function and procedure bodies can become foldable regions
- `BLOCK_EXPR` can be treated as an expression region instead of a statement block
- generated warnings can be attached to the source construct that triggered them

The current token-only mapping in `compiler/rxcp_dsl.c:47-105` cannot provide that level of disambiguation.

## Why Synthetic Flags Alone Are Not Enough

Keep the existing flags. They are still useful.

But they are not enough to support transformed diagnostics or a future source/work split.

Concrete cases:

- Exit replacement can turn one source command into many generated instructions.
- Inlining can duplicate or wrap a callee body at multiple call sites.
- Lowering of `SELECT`, `ADDRESS`, constructors, and object-string conversions can replace one node with a structurally unrelated subtree.
- Grafted exit fragments already need line/column remapping in `compiler/rxcp_util.c:676-735`.

What is missing is explicit origin data, for example:

- `origin_node`
- `origin_token_start`
- `origin_token_end`
- provenance flags such as exact, inherited, synthetic, composite

Without that, later diagnostics can only say "generated" instead of "generated from this exact source construct".

## Recommended Long-Term Provenance Model

The strategic answer is:

- `source_ast` remains source-aligned and editor-facing
- `work_ast` is derived from it and free to be rewritten
- each `work_ast` node points back to source provenance

That provenance can initially be one primary origin pointer plus a source span.

It does not need to be perfect on day one, but it does need to exist as a first-class concept.

## Implementation Plan

### Phase 0: Correctness Recovery

1. Fix `rxc` parser mode to always create `PARSE_TREE_FILE`.
2. Stop reading diagnostics only from `context->diagnostics_list`.
3. Remove `last_end` token suppression from the highlight path.
4. Add parser-mode regression tests for `rxc`, similar to `assembler/parsingtests`.

### Phase 1: Source-AST Highlighting

1. Add `rxcp_hvl()`.
2. Switch `compiler/rxcp_dsl.c` to use `rxcp_hvl()` instead of `rxcp_val()`.
3. Add AST-to-DSLSH projection with containers plus custom token callback.
4. Keep `cb_order_tree()`, `cb_tweak_tree_positions()`, and `cb_validate_tree()`.

### Phase 2: Semantic Enrichment

1. Populate `identifier_id` for names that resolve to symbols.
2. Distinguish declarations from references.
3. Distinguish type identifiers, function identifiers, class names, and member calls from raw lexical identifiers.
4. Expand editor seed config so emergency parsing better resembles the real language before the full parse returns.

### Phase 3: Provenance Architecture

1. Extend `ASTNode` with explicit origin fields.
2. Update `ast_dup()`, `ast_execute_rewrite()`, `ast_grft_interpolated()`, and manual rewrite sites to maintain provenance.
3. Introduce `source_ast` and `work_ast` in `Context` or an adjacent structure.
4. Route late diagnostics through source provenance by default.

## Test Plan

Add parser-mode tests for `rxc` covering at least:

- valid hello-world file: root type and basic highlighting
- broken syntax file: syntax errors visible in DSLSH output
- compound names: `args.0`, `arg.i`, `.string[]`
- assignment vs comparison: `a = b` in assignment context and compare context
- procedures, methods, classes, and namespaces: folding containers
- `DO`, `IF`, `SELECT`, and `BLOCK_EXPR`: structural containers
- files that mention exit syntax: parser mode must not execute bridge plugins

For `rxas`, keep the current tests and only add structure tests if containers are introduced later.

## Final Recommendation

The practical answer is:

- do not build a separate standalone cREXX parser for highlighting
- do build a separate highlighting pipeline over a source-aligned AST
- do keep `rxas` mostly as it is
- do plan for a source/work dual-tree provenance model in the compiler

And the answer to the provenance question is:

- synthetic flags are useful
- synthetic flags alone are not sufficient
- explicit origin links are the right long-term solution
