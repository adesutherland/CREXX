# cREXX and rxas Syntax Highlighting Review

## Executive Summary

- `rxas` already has a usable parser-mode highlighter. It is simple and mostly flat, but it is structurally correct, produces diagnostics, and matches the assembler's source model well enough.
- `rxc` should be treated as a prototype, not a finished architecture. The current parser-mode implementation runs the full compile-time validation and rewrite pipeline, then throws most of that structure away and emits a flattened token stream.
- The agreed solution is not a second grammar or a separate parser. It is one shared parser and early fixup path, one separate `rxc` highlighting controller, one source-aligned tree for editor-facing truth, and one derived work tree for compilation.
- The agreed construction order is important: the mutable compiler AST is first cleaned up into the authored source shape, the immutable source tree is copied from that source-shaped work tree, and only then is the mutable work tree allowed to diverge through later compiler rewrites.
- The source tree becomes the canonical home for diagnostics, symbol/type conclusions, source metadata, and highlighting projection. The work tree remains free to be rewritten, lowered, inlined, and optimized.
- A robust primary mapping from every work-tree node back to its source-tree node is required, and transformed work nodes may also need additional source-reporting anchors. `is_compiler_added` and similar synthetic flags remain useful, but they are not enough provenance on their own.
- Parser-mode performance and side-effect risk are addressed by retaining caches across highlight calls for imports, external symbols, and exit loading/discovery. Exits may still be re-run against a fresh work tree, but discovery/loading must not happen from scratch on every keystroke.
- `rxas` is considered complete for this work. The current `rxc` parser-mode implementation has no significant architectural reuse value beyond a few token-mapping ideas.

## Implementation Signpost

This is the implementation handoff summary.

Build toward this target state:

- keep one common parser and common early structural cleanup logic
- build the mutable work AST to the authored source shape first
- copy an immutable source tree from that source-shaped work AST before any heavy rewrite stages
- attach user-visible diagnostics and semantic conclusions to the source tree
- require every work-tree node to resolve back to one primary source-tree node, with optional additional reporting anchors where transformed output must still mention removed authored constructs
- generate DSLSH output from the source tree, not from the rewritten work tree
- cache imports, external symbols, and exit loading/discovery between parser calls

Do not build toward these alternatives:

- do not create a separate standalone cREXX highlighting grammar
- do not continue investing in the current flattened `rxc` token-stream highlighter as architecture
- do not invert the ownership model by treating the immutable source tree as the compiler's mutable master tree
- do not make the source tree depend on later compile-time rewrites such as implicit `main()` insertion, syntax sugar lowering, `ADDRESS` rewriting, inlining, or optimization

Recommended implementation order:

1. lock down parser-mode and source-metadata behaviour with tests
2. split source-facing state from the mutable work AST while keeping compilation behaviour stable
3. add mandatory work-to-source provenance mapping
4. replace `rxc` parser mode with a dedicated highlighting controller over the source tree
5. add retained caches for imports, external symbols, and exits
6. sync semantic results back onto source-tree nodes and only then consider reducing redundant work-tree state

Success criteria for implementation agents:

- `rxc --syntaxhighlight` always emits a valid `PARSE_TREE_FILE` root
- syntax errors, warnings, and source metadata anchor to the correct authored source location
- transformed emission such as inlining can still surface the removed call site in source metadata and tracing
- generated/internal failures are clearly labelled as internal while still pointing to the originating authored instruction
- highlighting and folding come from the source tree shape, not post-rewrite artefacts
- compilation still runs from the separate work tree
- late compiler diagnostics can always be mapped back to the correct source-tree node

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
| C | Dual-tree architecture: immutable `source_tree` + mutable `work_ast` | Very high | Medium | Medium | Very high | Best strategic direction |
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

- maintain a mutable `work_ast` for compilation
- copy an immutable source-facing tree from the source-shaped `work_ast`
- carry explicit origin links from later `work_ast` nodes back to that immutable source tree

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

## Agreed Direction

The agreed direction is:

- keep one strong common parser and common early fixup logic; do not create a separate standalone cREXX highlighter grammar
- keep `rxas` as-is; its highlighting is considered complete for this work
- replace the `rxc` parser-mode prototype with a separate syntax-highlighting controller that reuses the parser and selected existing walkers, but does not reuse the current flattened token-stream implementation
- introduce two trees for `rxc`:
  a source-aligned tree for highlighting, diagnostics, source metadata, and editor-facing semantic state, and a work tree for rewriting, lowering, inlining, optimization, and emission
- require a robust primary mapping from every work-tree node back to its source-tree node
- treat stronger source provenance and better metadata insertion as a primary goal, not just a side benefit

## Feasibility Assessment

This approach is feasible and sensible.

The key reasons are:

- the compiler already has a parser, early fixup walkers, and source-location propagation that are useful before the heavy rewrite stages
- the compiler already knows how to duplicate trees and already carries partial source provenance through duplication and grafting
- the DSLSH middleware already supports a hierarchical projection model, so generating a syntax tree from a source-aligned tree is straightforward once the source tree exists
- the parser server process persists between edits, so caching external symbol state and exit loading is practical

The main risks are not conceptual. They are refactoring risks:

- much of the current compiler assumes one canonical `context->ast`
- many helpers assume diagnostics, symbols, and source spans live directly on the actively mutated tree
- several rewrite paths perform manual tree surgery and will need provenance updates

Those risks are manageable if the change is staged and if the immutable source tree remains a straightforward copy of the early work-tree shape.

### Source Tree vs Parse Tree

The original review text favored an AST-compatible source tree first.
The implementation direction agreed later is slightly different and safer:
keep the compiler's mutable AST as the work tree,
and copy that early source-shaped work tree into a separate immutable `SourceNode` tree for editor-facing truth.

Reasons:

- it avoids making the compiler depend on an editor-owned immutable tree
- it limits the source tree factory to one job: copy from the already-shaped work AST
- it keeps source pointers, tokens, and authored structure aligned without asking later compiler passes to mutate `SourceNode`
- it still satisfies the real requirement, which is a tree aligned to the source and cleaned up for editor use

Conceptually this source tree is closer to a parse tree than the final rewritten work tree:

- it stays structurally close to the authored code
- it receives only the cleanup needed to overcome limited parser lookahead and produce correct block/folding shape
- it does not receive the later compile-time rewrites

That is now also how the code is structured:
`compiler/rxcp_source_tree.c` builds immutable `SourceNode` objects by copying the early work-tree shape.

## Agreed Architecture

### 1. Separate `rxc` Highlighting Controller

`rxc` should gain a dedicated highlighting controller, separate from the current parser-mode wrapper logic.

This controller should:

- own the parser-mode caches
- build and retain the source tree
- read from the immutable source tree for highlighting and diagnostics projection
- never treat the immutable source tree as the compiler's mutable master tree
- run the required semantic passes on the work tree
- project syntax highlighting, diagnostics, and editor metadata from the source tree

This separation is important because it avoids injecting parser-mode concerns into the main compiler orchestration and reduces regression risk in normal compilation.

### 2. Source Tree

The source tree is the authoritative tree for:

- exact source ranges
- folding structure
- syntax highlighting projection
- diagnostics and warnings shown to the user
- metadata source insertion
- editor-facing semantic state such as symbol/type information once known

The source tree should be built from:

1. the mutable work AST after raw parse output
2. the early structural cleanup needed to repair limited-lookahead weaknesses
3. source-location propagation over that mutable work AST
4. a copy step into immutable `SourceNode` storage

That cleanup includes examples such as:

- nesting instructions under procedures and methods
- fixing `IF` / `ELSE` structure
- fixing `SELECT` / `WHEN` / `OTHERWISE` structure
- producing a block shape suitable for code folding

This tree should not receive later transformations such as:

- implicit `main()` insertion
- exit-driven code expansion
- syntax sugar lowering
- `ADDRESS` rewriting
- inlining
- optimization

### 3. Work Tree

The work tree is the compiler's mutable AST.
It is source-shaped first, then used as the input for the immutable source-tree copy step.

The work tree is then the only tree that may be:

- rewritten
- expanded
- lowered
- inlined
- optimized
- emitted

This preserves the current compiler behaviour while isolating it from the editor-facing tree.
The immutable source tree is therefore a copy of the work tree's early source-faithful shape, not the producer of the work tree.

### 4. Canonical Ownership of Diagnostics and Semantics

Errors, warnings, symbol/type conclusions, and metadata anchors should be attached to the source tree as the canonical user-visible state.

The work tree may still carry transient symbol/type links for convenience during compilation, but the authoritative user-facing result must be recoverable from the source tree.

In practice this means:

- passes may compute on the work tree
- results must be written back to the mapped source-tree node
- diagnostics must be attached to source-owned sidecar state, not left only on work-tree nodes

The agreed implementation detail is:

- keep the immutable source tree structurally clean; do not inject fake `ERROR`, `WARNING`, or documentation-only syntax nodes into it just to carry reporting state
- store user-visible diagnostics, semantic conclusions, and other reporting-only facts in source-owned sidecars keyed by `SourceNode`
- let parser mode, error reporting, and later source metadata consumers read from those sidecars as the canonical authored view

### 5. Work-to-Source Mapping

This mapping is mandatory.

Each work-tree node must carry:

- a primary source-tree node pointer
- provenance mode information, at least distinguishing exact, inherited, synthetic, and composite mapping
- optional additional source-tree reporting anchors for transformed cases where one work node must still report more than one authored construct

This mapping is required for:

- error reporting
- warning reporting
- source metadata insertion
- editor semantic projection
- future robustness checks

The additional reporting anchors are needed for cases such as:

- inlining, where the call site may disappear from the work tree but must still appear in emitted source metadata and tracing
- exit-generated or interpolated code, where internal failures must point at the originating authored instruction rather than at opaque generated text
- composite rewrites, where the best user-visible report may need both the surviving source anchor and one or more removed authored anchors

The reverse mapping from source tree to work tree is not required as a hard invariant in the first implementation.

The agreed approach is:

- make work-to-source mapping mandatory from day one
- keep one primary source anchor for compiler logic and deterministic ownership
- allow one-to-many reporting anchors when user-visible tracing or diagnostics would otherwise lose authored intent
- add reverse mapping only if a specific consumer needs it
- if reverse mapping becomes useful, prefer an explicit primary-work mapping or an on-demand index rather than forcing a full bidirectional graph immediately

### 6. Diagnostic Semantics for Internal and Generated Failures

The compiler invariant remains that user syntax/semantic errors should be resolved before optimization and late-stage failures are internal.

That invariant is only useful to users if the reporting contract is clear:

- diagnostics caused by generated, interpolated, or rewritten compiler-only code must still anchor to the originating authored source instruction
- those diagnostics must be clearly labelled as internal or generated-code failures so users know the message is not claiming their written source text is itself malformed
- the label should live in canonical source-owned diagnostic state, not in ad hoc emitter strings or parser-mode-only formatting

This is especially important for:

- exit-generated code
- interpolated fragments
- post-parse rewrites such as `ADDRESS`
- inline-expanded procedures where removed call-site source still matters for tracing and metadata

### 7. Caching Model

The concern about per-keystroke side effects is valid.

The mitigation is not to avoid all semantic work. The mitigation is to cache the expensive external state between highlighting calls.

The highlighting controller should therefore retain between parses:

- external symbol/procedure/class discovery results
- import resolution results
- exit module loading and exit registry discovery
- any VM bridge state required only for loading/discovery

Exits may still be re-run on each parse against the fresh work tree, but:

- loading/discovery must be cached
- parser-mode state must be retained across calls
- caches must be invalidated when relevant inputs change, for example search paths, imports, relevant file timestamps, or compiler option state

The cache should store discovery and lookup state, not previous mutated AST results.

### 8. Compiler Invariant

The current invariant remains in force:

- optimization runs only on syntax-error-free input
- any later-stage errors after that point are internal compiler failures

The new architecture should strengthen that invariant by making the source tree the canonical place where syntax and user-facing semantic errors are recorded.

## Agreed Highlighting Projection

Once the source tree exists, syntax-tree generation becomes straightforward.

The highlighting projection should:

- create a `PARSE_TREE_FILE` root
- create DSLSH container nodes from the source tree for foldable or semantically useful regions
- use an authoritative compiler-token callback for leaf tokens rather than relying on generic gap filling
- emit errors and warnings directly from source-tree diagnostics

Suggested initial DSLSH container mapping:

| Source Tree Node | DSLSH Container |
| --- | --- |
| `PROGRAM_FILE`, `REXX_UNIVERSE` | `PARSE_TREE_FILE` |
| `NAMESPACE`, `CLASS`, `CLASS_DEF` | `PARSE_TREE_SCOPE` |
| `PROCEDURE`, `METHOD`, `FACTORY` | `PARSE_TREE_FUNCTION` |
| `INSTRUCTIONS`, `DO`, `SELECT`, `WHEN`, `OTHERWISE` | `PARSE_TREE_CODEBLOCK` or `PARSE_TREE_STATEMENT` |
| `IF`, `ASSIGN`, `CALL`, `RETURN`, `EXIT`, `ADDRESS`, `IMPLICIT_CMD` | `PARSE_TREE_STATEMENT` |
| arithmetic/comparison/logical expression nodes, `BLOCK_EXPR` | `PARSE_TREE_EXPR` |

This keeps the structure editor-friendly without forcing every AST node to become a DSLSH container.

## Why Synthetic Flags Alone Are Not Enough

`is_compiler_added` and related flags remain useful, but they are not sufficient.

They do not answer:

- which exact source node caused this generated node
- whether the source relationship is exact or inherited
- which source construct should receive a later warning
- which source location should be used for metadata insertion

The agreed design therefore requires explicit provenance, not just synthetic flags.

## Agreed Step-by-Step Implementation Plan

### Stage 0: Lock Down Behaviour and Add Tests

1. Add or extend parser-mode tests for `rxc` similar to the existing `rxas` parser tests.
2. Cover at least:
   root node correctness, syntax errors, compound names, procedures/methods/classes, folding shape, and files that mention exit syntax.
3. Add tests for source metadata insertion and error reporting where practical, because regressions are expected to surface there.

This stage creates the safety net for the refactor.

#### Status Summary on April 15, 2026

- Status: complete
- Delivered: the original Stage 0 parser-mode safety-net tests are in `compiler/tests` and remain part of the active regression suite:
  `syntaxhighlight_root_file`,
  `syntaxhighlight_syntax_error`,
  `syntaxhighlight_compound_names`,
  `syntaxhighlight_structure_nodes`,
  and `syntaxhighlight_exit_keyword`
- Current suite position:
  the later Stage 1 test `syntaxhighlight_top_level_statements` brings the active `syntax_highlighting` label to `6` tests total
- Current verified baseline in this worktree:
  `ctest --output-on-failure -L syntax_highlighting`
  passes `6/6`, and
  `ctest --output-on-failure -j8`
  in `cmake-build-debug/compiler/tests`
  passes `408/408`
- Safety-net conclusion:
  the compiler suite is broad and currently reliable enough to act as the non-highlighting regression net for the remaining stages

#### Journal

##### April 14, 2026 - Initial Stage 0 scaffolding and baseline cleanup

Stage 0 test scaffolding was added in `compiler/tests`.

Implementation notes:

- The tests are wired through `compiler/tests/CMakeLists.txt` and use `parser_tester` plus `compiler/tests/highlighting/dump_ast.txt`.
- They are labeled `syntax_highlighting`, `compiler`, and `stage0`.
- They intentionally asserted the target behaviour described in this review, not the then-current broken behaviour.

Historical parser-mode result at that point:

- `ctest --output-on-failure -L syntax_highlighting` failed all 5 original Stage 0 tests, which was expected at that time.
- Those failures confirmed the already-reviewed gaps:
  missing `PARSE_TREE_FILE` root, missing `SYNTAX_ERROR` projection, bad compound-name tokenization, no structural/folding nodes, and exit syntax not projected as editor-visible syntax.

Compiler-suite adequacy review from that round:

- The existing compiler suite was already broad enough to act as the non-highlighting regression net once green:
  codegen goldens, parser AST goldens, runtime round-trip tests, failure tests, warning tests, robustness tests, and exit runtime tests were already present.
- Source metadata and source-anchored diagnostics already had meaningful coverage through:
  emitted `.rxas` goldens, parser-position goldens in `golden/parsing`, failure tests in `golden/errors`, and warning checks such as `test_disjoint_detailed_warn`.

Important harness fix from that round:

- An apparent non-highlighting baseline failure turned out to be a test-harness race, not a compiler regression.
- Cause:
  `crexx_test_driver` copies sources into the shared test working directory using the original basename, so `_noopt` and `_opt` variants could stomp each other's temporary files under parallel `ctest`.
- Fix applied:
  `compiler/tests/CMakeLists.txt` now applies a shared `RESOURCE_LOCK` to all `crexx_test_driver`-based tests.
- Verified result after the fix:
  `ctest --output-on-failure -LE syntax_highlighting -j8`
  passed `401/401` in `cmake-build-debug/compiler/tests`.

Historical environment note:

- In `cmake-build-debug/compiler/tests`, `ctest -N` reported `406` compiler-suite tests after adding the 5 original Stage 0 syntax-highlighting tests.
- In the top-level `cmake-build-debug`, `ctest -N` reported `670` total tests.
- A top-level `ctest --output-on-failure -j8` run from this terminal still showed one extra non-compiler failure, `socket_test`, from the embedded `DSL-Syntax-Highlighter` project.
- That `socket_test` failure was outside `compiler/tests` and aborted before opening a loopback port (`assert(port > 0)` in `DSL-Syntax-Highlighter/codebuffer/socket_test.c`), so it should still be treated as an environment-specific top-level harness issue unless reproduced outside the sandbox.

### Stage 1: Introduce the Tree Split

1. Refactor parsing/fixup entry points so the compiler can explicitly build source-facing state from the mutable work AST.
2. Run only the early source-shape walkers on the mutable work AST:
   `ast_structure_fixup_walker`, `source_location_walker`, and the minimal syntax/structure cleanup needed for correct source shape.
3. Copy the immutable source tree from that source-shaped work AST.
4. Change the normal compile pipeline to continue from the mutable work tree.

Goal of this stage:

- compilation still works
- the source tree exists
- no highlighting changes are required yet

#### Status Summary on April 15, 2026

Stage 1 is now complete in the agreed form where the immutable source tree is copied from the early source-shaped mutable work AST.

Implemented in this stage:

- `rxc` parser mode now routes through a dedicated controller in `compiler/rxcp_highlight_controller.c`.
- The compiler can now build a source-faithful tree explicitly via `rxcp_prepare_source_ast()`.
- The source-facing restructuring has been split out from compile-only restructuring with:
  `ast_source_structure_walker()` and `ast_work_structure_walker()`.
- The source-facing tree is now held directly on the main `Context` as an immutable `SourceNode` tree with its own free list:
  `source_tree` and `source_free_list`.
- `ASTNode` now carries a `source_node` pointer so later work-tree nodes can retain a primary source anchor.
- A new parser-mode test, `syntaxhighlight_top_level_statements`, confirms the source tree does not project an implicit `main()` for top-level statements.

Important implementation detail:

- The temporary `source_context` / `source_ast` landing has now been replaced.
- The current design keeps the immutable source tree on the main compile context, but with a separate source-node allocation path in:
  `compiler/rxcp_source_tree.c`.
- The source tree is built from the mutable AST after the source-shape walkers run, before the later compile-only structure / rewrite stages continue on the mutable tree.

Current compiler-pipeline status:

- `source_tree` exists and is source-faithful.
- `source_tree` is copied from the mutable AST after the source-shape walkers run.
- The separate highlighting controller projects from `source_tree`.
- The normal compiler pipeline intentionally continues on the mutable AST / `work_ast` path.
- `work_ast` is still the compiler's mutable AST, while `source_tree` is the separate immutable copy used for editor-facing truth.

Why there is no second mutable tree copy:

- A first attempt to make the compiler execute from a duplicated `work_ast` caused widespread regressions and crashes.
- The immediate causes were:
  shared mutable compiler-context state leaking between source/work preparation, and then node-identity / node-number coupling inside later rewrite and emission paths.
- The agreed architecture was then clarified:
  the immutable source tree should be a copy of the source-shaped mutable work AST, not the producer of a second mutable compiler tree.
- The remaining architecture work after Stage 1 is therefore provenance, semantic sync, and caching, not reviving a second mutable AST copy unless a new concrete need appears.

Verification:

- Current verified state in `cmake-build-debug/compiler/tests`:
  `ctest --output-on-failure -j8`
  passes `408/408`,
  including `6/6` syntax-highlighting tests and the new Stage 2 provenance test.

Implication for future agents:

- Treat the tree split as the current stable baseline.
- Do not invert the ownership model:
  the immutable source tree is copied from the source-shaped mutable work AST.
- Do not reintroduce true duplicated `work_ast` execution in the main compiler path without a new concrete requirement and a plan for node identity / provenance coupling.

#### Journal

##### April 14, 2026 - Recovery checkpoint

This section is a historical crash-recovery checkpoint for the in-progress refactor state at that time.
It is useful for reconstruction, but its open-problem list is superseded by the completed debugging worklog below.

Code already changed:

- Added `SourceNode` forward declaration in `compiler/rxcp_types.h`.
- Added immutable source-tree types and allocation / free logic in:
  `compiler/rxcp_source_tree.h` and `compiler/rxcp_source_tree.c`.
- Added `SourceNode *source_node` to `ASTNode` in `compiler/rxcp_ast.h`.
- Added source-node propagation / inheritance in:
  `ast_ft()`, `ast_dup()`, `add_ast()`, `add_sbtr()`, and `ast_rpl()` in `compiler/rxcp_ast_core.c`.
- Replaced `source_context` / `source_ast` with `source_tree` / `source_free_list` in `compiler/rxcp_ctx.h`.
- Switched parser-mode highlighting projection from AST nodes to source-tree nodes in:
  `compiler/rxcp_highlight_controller.c`.
- Changed `rxcp_prepare_source_ast()` so it runs:
  `ast_source_structure_walker()`,
  `source_location_walker()`,
  `syntax_validation_walker()`,
  then `source_tree_build()`.
- Changed `validate_ast()` so the compiler now runs:
  `rxcp_prepare_source_ast()`,
  `rxcp_prepare_work_ast()`,
  then compile-only structure work via `ast_work_structure_walker()`.
- Added the missing parser-tester script file:
  `compiler/tests/highlighting/dump_ast.txt`.

Crash that was already found and fixed:

- A reproducible crash occurred in `build_symbols_walker()` while compiling `inline_test2_opt`.
- Root cause:
  the first split-walker implementation incorrectly used the same boundary rule for callable bodies and class-member hoisting.
- That left `METHOD` and `FACTORY` nodes as siblings of `CLASS_DEF` instead of children.
- The resulting malformed tree caused missing procedure symbol state during argument processing in `build_symbols_walker()`.
- The fix is already in place in `compiler/rxcp_val_check.c`:
  `is_callable_boundary()` and `is_class_member_boundary()` are now separate.
- After that fix, `inline_test2_opt` passes again.

Known open problems at that checkpoint:

- The mutable AST is still being source-shaped before compilation continues, so existing `-dp` parser goldens can move.
- Compound dotted names are still not projected correctly in parser mode:
  dots currently surface as `LEXER_COMMENT` leaf nodes in the failing test output.
- Syntax-error projection is still missing the expected `SYNTAX_ERROR` node in parser mode for `err_02_syntax.rexx`.
- Exit/import-driven syntax-highlighting still differs from expectations in `syntaxhighlight_exit_keyword`.
- Some parser / error goldens currently disagree with the new source-shaped early tree or source-span computation.

##### April 14, 2026 - Recovery debugging worklog

Step 0. Recovery checkpoint and document refresh

- Status: completed on April 14, 2026.
- Purpose:
  record the exact post-crash state before more debugging work.
- Result:
  this section replaces the now-stale description of the temporary `source_context` design.

Step 1. Triage and classify current regressions

- Status: completed on April 14, 2026.
- Target:
  determine which current failures are real regressions and which are intended source-shape / span changes that need new baselines.
- Reproduced failures at the start of this step:
  `err_loop_class_fail`,
  `16_classes_parse`,
  `do_simple_parse_parse`,
  `do_counted_parse_parse`,
  `do_while_parse_parse`,
  `do_until_parse_parse`,
  `syntaxhighlight_syntax_error`,
  `syntaxhighlight_compound_names`,
  `syntaxhighlight_exit_keyword`.
- Classification from this step:
  the five `-dp` parser-golden failures were a real regression in callable child ordering,
  while `err_loop_class_fail` is a source-span drift candidate.
- Root cause of the parser-golden regression:
  once the source-shape pass had already attached an `INSTRUCTIONS` child,
  the later work-shape pass was appending an implicit empty `ARGS` node after that block.
- Affected outputs:
  `16_classes_parse`,
  `do_simple_parse_parse`,
  `do_counted_parse_parse`,
  `do_while_parse_parse`,
  `do_until_parse_parse`.
- Separate classification:
  `err_loop_class_fail` now reports `".foo(5)"` instead of `".foo"`.
  That looks like improved source-fidelity rather than malformed structure, but it still needs an explicit baseline decision.

Step 2. Repair compiler regressions or update intended baselines

- Status: completed on April 14, 2026.
- Completed in this step:
  fixed the callable child-order regression in `compiler/rxcp_val_check.c` by normalizing `ARGS` placement ahead of `INSTRUCTIONS`,
  then updated the intended source-span baseline in `compiler/tests/golden/errors/err_loop_class.txt`.
- Verified result after the fix:
  `16_classes_parse`,
  `do_simple_parse_parse`,
  `do_counted_parse_parse`,
  `do_while_parse_parse`,
  and `do_until_parse_parse`
  all pass again.
- Final baseline decision:
  preserve the expanded source span in `err_loop_class_fail`.
  The new output `".foo(5)"` is the more faithful Stage 1 source representation, so the golden was updated to match.

Step 3. Re-run targeted and wider compiler suites

- Status: completed on April 14, 2026.
- Verified command:
  `ctest --output-on-failure -LE syntax_highlighting -j8`
  in `cmake-build-debug/compiler/tests`.
- Result:
  `401/401` non-highlighting compiler tests passed.
- End-of-round verification:
  after the Stage 4 highlighting fixes landed,
  `ctest --output-on-failure -j8`
  in `cmake-build-debug/compiler/tests`
  passed `407/407`.

Step 4. Revisit remaining syntax-highlighting failures

- Status: completed on April 14, 2026.
- Starting failures for this step:
  `syntaxhighlight_syntax_error`,
  `syntaxhighlight_compound_names`,
  `syntaxhighlight_exit_keyword`.
- Root causes identified:
  parser mode had not been configuring the normal executable/import search path,
  so exit-primary keywords such as `fsay` were not promoted during parse;
  dotted stem segments such as `.lino` were reaching the projection layer as `TK_STEMVAR` / `TK_STEMINT` tokens and were being materialized as comment gaps;
  syntax diagnostics could still disappear if only the source-tree projection path was consulted.
- Fixes applied in `compiler/rxcp_highlight_controller.c`:
  parser mode now configures `context->import_locations` from `.` plus `exepath()`,
  `TK_EXIT_PRIMARY` and `TK_EXIT_TOKEN` are mapped explicitly as keywords,
  stem-tail tokens (`TK_STEMVAR`, `TK_STEMINT`, `TK_STEMSTRING`, `TK_STEMNOVAL`) are projected as an explicit `LEXER_SEPARATOR` plus the remaining identifier/number segment,
  and diagnostics now use more robust span fallback plus an AST-backed emission pass so parser errors such as `MISSING_END` cannot be dropped.
- Verified command:
  `ctest --output-on-failure -L syntax_highlighting`
  in `cmake-build-debug/compiler/tests`.
- Result:
  all `6/6` syntax-highlighting tests now pass.

Step 5. Final documentation pass for this debugging round

- Status: completed on April 14, 2026.
- Final recovered state for future agents:
  the immutable source-tree design remains in place on the main `Context`,
  the compiler suite in `compiler/tests` is currently green at `408/408`,
  and the Stage 0 / Stage 1 highlighting tests now pass against the new controller.
- Practical takeaway:
  this is now a stable handoff point for the next syntax-highlighting stages rather than a crash-recovery checkpoint with known red tests.

### Stage 2: Introduce Primary Provenance Mapping

1. Extend `ASTNode` with explicit work-to-source mapping fields.
2. Ensure the source-to-work duplication step initializes those fields.
3. Update duplication and rewrite helpers such as:
   `ast_dup()`, `ast_execute_rewrite()`, `ast_grft_interpolated()`, inlining helpers, and manual tree surgery sites.
4. Route diagnostics produced during work-tree validation back to source-owned sidecars keyed by the mapped source-tree node.
5. Add support for additional reporting anchors where transformed work-tree output must still preserve removed authored constructs for tracing or source metadata.

Goal of this stage:

- every work node can resolve back to a source node
- user-visible diagnostics can be attached to canonical source-owned state
- transformed output can preserve authored tracing anchors even when the corresponding work-tree node has been removed

#### Status Summary on April 15, 2026

- Status: complete
- Delivered:
  `ASTNode` now carries explicit provenance state in addition to the primary `source_node` pointer, common helpers / rewrite paths propagate `exact`, `inherited`, and `synthetic` ownership in the current worktree, source-owned diagnostics are rebuilt from the current AST into canonical `SourceNode` sidecars, and transformed emission can preserve removed authored constructs through primary plus additional reporting anchors
- Test coverage:
  a dedicated internal regression test,
  `source_provenance`,
  is now wired through `compiler/tests/CMakeLists.txt`
- Current verified baseline:
  `ctest --output-on-failure -R '^source_provenance$'`
  passes,
  `ctest --output-on-failure -L syntax_highlighting`
  passes `6/6`, and
  `ctest --output-on-failure -j8`
  in `cmake-build-debug/compiler/tests`
  passes `408/408`
- Practical outcome:
  source-owned diagnostics are now the canonical user-visible reporting state for compiler and parser-mode consumers, stale diagnostics from earlier validation passes are dropped during sync, internal/generated diagnostics require explicit internal-generation context, and optimized / inlined `.src` metadata can retain the removed call-site anchor alongside the surviving transformed body

#### Journal

##### April 14, 2026 - Initial provenance pass and regression-fix round

Implemented in this stage:

- `ASTNode` now carries explicit provenance state in addition to the primary `source_node` pointer:
  `source_provenance` with `exact`, `inherited`, `synthetic`, and `composite` modes.
- Common helpers now exist in `compiler/rxcp_ast_core.c`:
  `ast_set_primary_source_node()` and `ast_copy_source_anchor()`.
- Exact work-to-source links are assigned when the immutable source tree is built in `compiler/rxcp_source_tree.c`.
- Inherited provenance now propagates through the common tree-surgery helpers:
  `add_ast()`, `add_sbtr()`, and `ast_rpl()`.
- Synthetic provenance is now assigned in key compiler-generated paths:
  rewrite construction in `compiler/rxcp_ast_rewrite.c`,
  interpolated exit fragments in `compiler/rxcp_util.c`,
  inline-generated nodes in `compiler/rxcp_inline.c`,
  and injected symbol-definition helpers in `compiler/rxcp_val_sym.c`.
- `source_tree_free()` now clears only source-node links owned by the current context before freeing that immutable tree, avoiding stale pointers into freed `SourceNode` storage.

Regression notes from that round:

- The first provenance pass regressed:
  `test_select_c_noopt`,
  `err_class_assign_fail`,
  and `err_loop_class_fail`.
- Root causes:
  diagnostic nodes must not inherit token bounds, because that narrows errors to a single token;
  rewrite-created nodes must not inherit `token_end`, because the no-opt emitter uses `token_end->token_next` to derive follow-on source metadata.
- Those regressions were then fixed in:
  `compiler/rxcp_ast_core.c` and `compiler/rxcp_ast_rewrite.c`.

##### April 15, 2026 - Canonical source-reporting completion round

Implemented in this stage:

- `SourceNode` sidecars now carry canonical diagnostics rebuilt from the current AST rather than accumulating historical pass artifacts.
- Diagnostic sync now clears stale mirrored state, rescans the current AST, and only falls back to detached diagnostics when the AST produced no source-owned diagnostics.
- Parser-mode syntax diagnostics now come from source-owned state without needing the earlier AST-backed fallback when a source tree exists.
- Internal/generated diagnostic labelling is now explicit:
  exit-bridge and compiler-generated internal fragments mark diagnostics as internal,
  while ordinary synthetic/provenance-carrying rewrite nodes do not automatically become internal.
- Emission metadata now supports a primary source-reporting anchor plus additional reporting anchors, allowing optimized / inlined output to preserve removed call-site `.src` lines without fake syntax nodes.

Curated regressions from this round:

- optimized goldens were updated where preserving call-site `.src` anchors is the intended new tracing behaviour
- several compiler-failure goldens were updated to reflect canonicalized spans, duplicate suppression, and the removal of stale cascade diagnostics

### Stage 3: Build the Separate Highlighting Controller

1. Add a new controller module for parser mode, for example `rxcp_highlight_controller.c`.
2. Move `rxc` parser-mode entry logic onto that controller.
3. Replace the current `rxc` flat token-stream builder; it has no real reuse value beyond narrow token mapping ideas.
4. Generate the DSLSH tree from the source tree.
5. Add a compiler-token callback so punctuation, separators, brackets, and identifiers are projected authoritatively.

Goal of this stage:

- `rxc --syntaxhighlight` becomes source-tree based
- the current fragile flattened implementation is retired

#### Status Summary on April 15, 2026

- Status: complete
- Delivered:
  parser mode now routes through `compiler/rxcp_highlight_controller.c`, emits a proper `PARSE_TREE_FILE` root from the immutable source tree, projects DSLSH structure from source-tree containers, and uses a compiler-token callback for the currently covered token classes
- Diagnostics state:
  parser-mode diagnostics are emitted from canonical source-owned sidecars whenever a source tree exists; the detached AST diagnostic path remains only for the no-source-tree parse-failure fallback
- Current verified baseline:
  `ctest --output-on-failure -L syntax_highlighting`
  passes `6/6`, and the same controller remains green under the full `408/408` compiler-suite run
- Remaining dependency:
  Stage 4 caching is still separate and not yet implemented

#### Journal

##### April 14, 2026 - Controller replacement and highlighting-fix round

Implemented in this stage:

- parser mode now routes through `compiler/rxcp_highlight_controller.c`
- `rxc --syntaxhighlight` emits a proper `PARSE_TREE_FILE` root from the immutable source tree
- DSLSH structure nodes are projected from source-tree containers rather than from the rewritten work AST
- the token callback now maps separators, brackets, arithmetic operators, assignment, exit-primary tokens, and dotted stem-tail segments authoritatively enough for the current tests
- parser mode configures import/executable search paths and now consumes diagnostics from the canonical source-owned state built in Stage 2

### Stage 4: Add Caching for External Symbols and Exits

1. Add persistent caches to the highlighting controller for:
   imports, external procedures/classes/symbols, exit discovery, and exit module loading.
2. Reuse those caches between syntax-highlight calls in the same parser server process.
3. Re-run exits per parse if required, but do not repeatedly reload/discover them.
4. Add cache invalidation based on relevant inputs such as search paths, module identity, file timestamps, and language options.

Goal of this stage:

- semantic highlighting remains responsive
- per-keystroke side effects are mitigated by retained state

#### Status Summary on April 15, 2026

- Status: complete
- Delivered:
  parser mode now retains a dedicated master context inside `compiler/rxcp_highlight_controller.c`,
  reuses cached import inventory / loaded external state through that retained root,
  warms exit discovery and module loading once per stable cache generation,
  and invalidates the retained state when the document identity, effective search paths, or watched import/module timestamps change
- Cache model details:
  parser mode now derives the authored file name and source directory from the DSLSH `CodeBuffer` document id,
  uses that document directory plus the parser server working directory and `exepath()` as the retained search-path basis,
  snapshots importable files plus `library` / exit-module binaries for timestamp checks,
  and tracks exit discovery readiness explicitly so a valid "no exits discovered" result is still cached rather than rediscovered every parse
- Test coverage:
  a dedicated parser-mode regression test,
  `highlight_cache`,
  is now wired through `compiler/tests/CMakeLists.txt`
- Current verified baseline:
  `ctest --output-on-failure -L syntax_highlighting`
  in `cmake-build-debug/compiler/tests`
  passes `7/7`
- Dependency view:
  this retained-state layer is now ready for the later Stage 5 semantic-sync work without changing the separate source-tree / work-tree ownership model

#### Journal

##### April 15, 2026 - Retained parser-mode cache implementation

- Added a retained parser-mode master context in `compiler/rxcp_highlight_controller.c` so each highlight request still gets a fresh child parse context while imports, external symbol state, exit discovery, and VM module loading can persist across calls in the same parser process.
- Parser mode now derives source-aware cache keys from the DSLSH document id, including the authored file name and containing directory rather than relying on the earlier hard-coded `dsl_buffer.rexx` placeholder.
- The retained cache now snapshots search directories, discovered importable files, and the `library` / configured exit-module binaries so timestamp changes invalidate the cache before the next highlight pass.
- Exit discovery now has an explicit readiness flag, preventing repeated rediscovery when a parse generation has already checked for exits, even if that discovery produced an empty registry.
- Added `compiler/tests/src/test_highlight_cache.c` to verify:
  stable same-process parses reuse the retained generation,
  dependency timestamp changes invalidate and rebuild it,
  and changing the document search path also forces a rebuild.

### Stage 5: Sync Semantic Results Back to the Source Tree

1. Populate symbol/type conclusions onto source-tree nodes through the work-to-source mapping.
2. Make the source tree the definitive view of:
   parsed code, clarified argument types, diagnostics, and metadata anchors.
3. Populate DSLSH `identifier_id` and richer semantic token distinctions from this source-owned state.

Goal of this stage:

- reading the source tree yields the canonical view of the user's code
- the work tree remains an implementation detail for compilation

#### Status Summary on April 15, 2026

- Status: not started
- Current code reality:
  provenance exists, but symbol/type conclusions and user-visible diagnostics still primarily live on the mutable work AST rather than on immutable source-tree nodes as canonical state
- Not yet delivered:
  source-owned semantic state,
  source-owned canonical diagnostics,
  `identifier_id`,
  and richer semantic token projection derived from that source-owned state
- Dependency view:
  this stage should follow the remaining Stage 2 work so the source tree can become the authoritative target for mirrored results rather than an additional partially-populated view

#### Journal

##### April 15, 2026 - Stage reserved pending completion of the remaining Stage 2 gap

- No semantic-sync implementation has been started yet.
- The source tree is still not the single canonical store for diagnostics and semantic conclusions.

### Stage 6: Reduce Redundant Work-Tree State Only After Proof

1. Review whether some work-tree attributes such as direct source spans or some symbol ownership should be reduced or removed.
2. Do this only after:
   mapping, diagnostics, metadata insertion, and tests are all stable.
3. Use this stage to prove that critical logic is genuinely going through the source tree.

Goal of this stage:

- strengthen the architecture without taking unnecessary early risk

#### Status Summary on April 15, 2026

- Status: intentionally deferred
- Current guidance:
  do not remove direct source spans, mutable-AST convenience fields, or other redundant work-tree state yet
- Dependency view:
  this stage should only begin after Stages 2 through 5 are complete and the source tree is demonstrably the authoritative path for diagnostics, semantic projection, and metadata anchoring

#### Journal

##### April 15, 2026 - No work started by design

- No Stage 6 reduction work has been started.
- The current priority remains correctness and canonical source ownership, not state reduction.

## Testing Requirements Per Stage

At each stage run the existing compiler test suite and parser-mode tests.

Special attention should go to:

- parser-mode highlighting output
- syntax-error reporting
- warning anchoring
- source metadata insertion
- import/external symbol handling
- exit-driven transformations
- any change that affects provenance

This work is explicitly expected to uncover existing weaknesses in error reporting and metadata insertion. That is a success criterion, not a failure mode.

## Final Decision

The agreed way forward is:

- one common parser and common early fixup logic
- one new source-aligned tree for editor-facing truth
- one derived work tree for compilation
- one separate `rxc` highlighting controller with retained caches
- one mandatory primary mapping from work tree back to source tree

And specifically:

- `rxas` is complete and out of scope
- the current `rxc` highlighting implementation has no significant reuse value as architecture
- better provenance, better error reporting, and better source metadata insertion are part of the core objective
