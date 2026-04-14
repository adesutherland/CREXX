# cREXX and rxas Syntax Highlighting Review

## Executive Summary

- `rxas` already has a usable parser-mode highlighter. It is simple and mostly flat, but it is structurally correct, produces diagnostics, and matches the assembler's source model well enough.
- `rxc` should be treated as a prototype, not a finished architecture. The current parser-mode implementation runs the full compile-time validation and rewrite pipeline, then throws most of that structure away and emits a flattened token stream.
- The agreed solution is not a second grammar or a separate parser. It is one shared parser and early fixup path, one separate `rxc` highlighting controller, one source-aligned tree for editor-facing truth, and one derived work tree for compilation.
- The source tree becomes the canonical home for diagnostics, symbol/type conclusions, source metadata, and highlighting projection. The work tree remains free to be rewritten, lowered, inlined, and optimized.
- A robust primary mapping from every work-tree node back to its source-tree node is required. `is_compiler_added` and similar synthetic flags remain useful, but they are not enough provenance on their own.
- Parser-mode performance and side-effect risk are addressed by retaining caches across highlight calls for imports, external symbols, and exit loading/discovery. Exits may still be re-run against a fresh work tree, but discovery/loading must not happen from scratch on every keystroke.
- `rxas` is considered complete for this work. The current `rxc` parser-mode implementation has no significant architectural reuse value beyond a few token-mapping ideas.

## Implementation Signpost

This is the implementation handoff summary.

Build toward this target state:

- keep one common parser and common early structural cleanup logic
- build a source-aligned AST-shaped tree first
- duplicate that tree into a work tree before any heavy rewrite stages
- attach user-visible diagnostics and semantic conclusions to the source tree
- require every work-tree node to resolve back to one primary source-tree node
- generate DSLSH output from the source tree, not from the rewritten work tree
- cache imports, external symbols, and exit loading/discovery between parser calls

Do not build toward these alternatives:

- do not create a separate standalone cREXX highlighting grammar
- do not continue investing in the current flattened `rxc` token-stream highlighter as architecture
- do not make the source tree depend on later compile-time rewrites such as implicit `main()` insertion, syntax sugar lowering, `ADDRESS` rewriting, inlining, or optimization

Recommended implementation order:

1. lock down parser-mode and source-metadata behaviour with tests
2. split source tree and work tree while keeping compilation behaviour stable
3. add mandatory work-to-source provenance mapping
4. replace `rxc` parser mode with a dedicated highlighting controller over the source tree
5. add retained caches for imports, external symbols, and exits
6. sync semantic results back onto source-tree nodes and only then consider reducing redundant work-tree state

Success criteria for implementation agents:

- `rxc --syntaxhighlight` always emits a valid `PARSE_TREE_FILE` root
- syntax errors, warnings, and source metadata anchor to the correct authored source location
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

Those risks are manageable if the change is staged and if the source tree is kept AST-compatible at first.

### Source Tree vs Parse Tree

The agreed implementation should start with a source-aligned AST-shaped tree, not a wholly separate data structure.

Reasons:

- existing walkers and utilities already operate on `ASTNode`
- this minimizes regression risk
- it keeps duplication into the work tree simple
- it still satisfies the real requirement, which is a tree aligned to the source and cleaned up for editor use

Conceptually this source tree is closer to a parse tree than the final working tree:

- it stays structurally close to the authored code
- it receives only the cleanup needed to overcome limited parser lookahead and produce correct block/folding shape
- it does not receive the later compile-time rewrites

If a later stage shows that a true separate parse-tree representation is materially more robust, that can be revisited. It should not be the initial implementation.

## Agreed Architecture

### 1. Separate `rxc` Highlighting Controller

`rxc` should gain a dedicated highlighting controller, separate from the current parser-mode wrapper logic.

This controller should:

- own the parser-mode caches
- build and retain the source tree
- derive a fresh work tree from the source tree
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

1. raw parse output
2. early structural cleanup needed to repair limited-lookahead weaknesses
3. source-location propagation

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

The work tree is produced by duplicating the source tree after the source-tree cleanup phase.

The work tree is then the only tree that may be:

- rewritten
- expanded
- lowered
- inlined
- optimized
- emitted

This preserves the current compiler behaviour while isolating it from the editor-facing tree.

### 4. Canonical Ownership of Diagnostics and Semantics

Errors, warnings, symbol/type conclusions, and metadata anchors should be attached to the source tree as the canonical user-visible state.

The work tree may still carry transient symbol/type links for convenience during compilation, but the authoritative user-facing result must be recoverable from the source tree.

In practice this means:

- passes may compute on the work tree
- results must be written back to the mapped source-tree node
- diagnostics must be attached to source-tree nodes, not left only on work-tree nodes

### 5. Work-to-Source Mapping

This mapping is mandatory.

Each work-tree node must carry:

- a primary source-tree node pointer
- provenance mode information, at least distinguishing exact, inherited, synthetic, and composite mapping

This mapping is required for:

- error reporting
- warning reporting
- source metadata insertion
- editor semantic projection
- future robustness checks

The reverse mapping from source tree to work tree is not required as a hard invariant in the first implementation.

The agreed approach is:

- make work-to-source mapping mandatory from day one
- add reverse mapping only if a specific consumer needs it
- if reverse mapping becomes useful, prefer an explicit primary-work mapping or an on-demand index rather than forcing a full bidirectional graph immediately

### 6. Caching Model

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

### 7. Compiler Invariant

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

#### Stage 0 Status on April 14, 2026

Stage 0 test scaffolding has now been added in `compiler/tests`.

New parser-mode tests:

- `syntaxhighlight_root_file`
- `syntaxhighlight_syntax_error`
- `syntaxhighlight_compound_names`
- `syntaxhighlight_structure_nodes`
- `syntaxhighlight_exit_keyword`

Implementation notes:

- The tests are wired through `compiler/tests/CMakeLists.txt` and use `parser_tester` plus `compiler/tests/highlighting/dump_ast.txt`.
- They are labeled `syntax_highlighting`, `compiler`, and `stage0`.
- They intentionally assert the target behaviour described in this review, not the current broken behaviour.

Current parser-mode result:

- `ctest --output-on-failure -L syntax_highlighting` fails all 5 tests, which is expected at this stage.
- The failures confirm the already-reviewed gaps:
  missing `PARSE_TREE_FILE` root, missing `SYNTAX_ERROR` projection, bad compound-name tokenization, no structural/folding nodes, and exit syntax not projected as editor-visible syntax.

Compiler-suite adequacy review:

- The existing compiler suite is broad enough to act as the non-highlighting regression net once it is green:
  codegen goldens, parser AST goldens, runtime round-trip tests, failure tests, warning tests, robustness tests, and exit runtime tests are already present.
- Source metadata and source-anchored diagnostics already have meaningful coverage through:
  emitted `.rxas` goldens, parser-position goldens in `golden/parsing`, failure tests in `golden/errors`, and warning checks such as `test_disjoint_detailed_warn`.

Important baseline note:

- An apparent non-highlighting baseline failure turned out to be a test-harness race, not a compiler regression.
- Cause:
  `crexx_test_driver` copies sources into the shared test working directory using the original basename, so `_noopt` and `_opt` variants can stomp each other's temporary files under parallel `ctest`.
- Fix applied:
  `compiler/tests/CMakeLists.txt` now applies a shared `RESOURCE_LOCK` to all `crexx_test_driver`-based tests.
- Verified result after the fix:
  `ctest --output-on-failure -LE syntax_highlighting -j8`
  passes `401/401` in `cmake-build-debug/compiler/tests`.
- Verified serial result:
  `ctest --output-on-failure -LE syntax_highlighting -j1`
  also passes `401/401`.

Implication for future agents:

- The compiler-suite discrepancy from the first Stage 0 report is resolved.
- The non-highlighting compiler suite is a reliable safety net again in this build after the resource-lock fix.
- The new highlighting tests are still useful and correctly failing.
- In `cmake-build-debug/compiler/tests`, `ctest -N` reports `406` compiler-suite tests after adding the 5 Stage 0 syntax-highlighting tests.
- In the top-level `cmake-build-debug`, `ctest -N` reports `670` total tests.
- A top-level `ctest --output-on-failure -j8` run from this terminal still shows one extra non-compiler failure, `socket_test`, from the embedded `DSL-Syntax-Highlighter` project.
- That `socket_test` failure is outside `compiler/tests` and aborts before opening a loopback port (`assert(port > 0)` in `DSL-Syntax-Highlighter/codebuffer/socket_test.c`), so treat it as an environment-specific top-level harness issue unless reproduced outside the sandbox.
- A CLion run that reports only the 5 `syntaxhighlight_*` failures is therefore consistent with the compiler Stage 0 status and is not in conflict with the `compiler/tests` result.
- Use:
  `ctest -L syntax_highlighting`
  to track highlighting progress, and
  `ctest -LE syntax_highlighting`
  to measure unrelated compiler-suite drift.

### Stage 1: Introduce the Tree Split

1. Refactor parsing/fixup entry points so the compiler can explicitly build a source tree first.
2. Run only the early source-shape walkers on that tree:
   `ast_structure_fixup_walker`, `source_location_walker`, and the minimal syntax/structure cleanup needed for correct source shape.
3. Duplicate the source tree into a work tree.
4. Change the normal compile pipeline to continue from the work tree.

Goal of this stage:

- compilation still works
- the source tree exists
- no highlighting changes are required yet

#### Stage 1 Status on April 14, 2026

Stage 1 is now partially implemented with the agreed variation of bringing the separate highlighting controller forward early.

Implemented in this stage:

- `rxc` parser mode now routes through a dedicated controller in `compiler/rxcp_highlight_controller.c`.
- The compiler can now build a source-faithful tree explicitly via `rxcp_prepare_source_ast()`.
- The source-facing restructuring has been split out from compile-only restructuring with:
  `ast_source_structure_walker()` and `ast_work_structure_walker()`.
- A new parser-mode test, `syntaxhighlight_top_level_statements`, confirms the source tree does not project an implicit `main()` for top-level statements.

Important implementation detail:

- The source tree is currently built in a separate `source_context`, not inside the main compile context.
- This was necessary because allocating `source_ast` in the main context changed node numbering for later compiler-added rewrite nodes, which broke many codegen goldens despite leaving logic unchanged.
- Keeping the source tree in its own context preserves compiler stability while still giving parser mode a faithful editor-facing tree.

Current compiler-pipeline status:

- `source_ast` exists and is source-faithful.
- The separate highlighting controller projects from `source_ast`.
- The normal compiler pipeline is intentionally still running from the legacy working tree in this stage.

Why that boundary was kept:

- A first attempt to make the compiler execute from a duplicated `work_ast` caused widespread regressions and crashes.
- The immediate causes were:
  shared mutable compiler-context state leaking between source/work preparation, and then node-identity / node-number coupling inside later rewrite and emission paths.
- That means the full "compile from duplicated work tree" part of the original Stage 1 outline is not yet safe to claim complete.
- It should resume only once work/source identity and provenance are made explicit enough to survive later rewrites.

Verification:

- `ctest --output-on-failure -LE syntax_highlighting -j8`
  passes `401/401` in `cmake-build-debug/compiler/tests`.
- `ctest --output-on-failure -L syntax_highlighting`
  now passes `2/6`.
- Passing highlighting tests:
  `syntaxhighlight_root_file`, `syntaxhighlight_top_level_statements`.
- Remaining failing highlighting tests:
  `syntaxhighlight_syntax_error`, `syntaxhighlight_compound_names`, `syntaxhighlight_structure_nodes`, `syntaxhighlight_exit_keyword`.

Implication for future agents:

- Treat this as a safe Stage 1 landing point:
  source-faithful tree exists, parser mode uses it, compiler regression net is green again.
- Do not reintroduce `work_ast` execution in the main compiler path without also addressing node identity / provenance and later rewrite coupling.
- The next useful work is to improve highlighting projection against the current `source_ast`, then return to true work-tree execution once provenance machinery is ready.

### Stage 2: Introduce Primary Provenance Mapping

1. Extend `ASTNode` with explicit work-to-source mapping fields.
2. Ensure the source-to-work duplication step initializes those fields.
3. Update duplication and rewrite helpers such as:
   `ast_dup()`, `ast_execute_rewrite()`, `ast_grft_interpolated()`, inlining helpers, and manual tree surgery sites.
4. Route diagnostics produced during work-tree validation back to the mapped source-tree node.

Goal of this stage:

- every work node can resolve back to a source node
- user-visible diagnostics can be attached to the source tree

### Stage 3: Build the Separate Highlighting Controller

1. Add a new controller module for parser mode, for example `rxcp_highlight_controller.c`.
2. Move `rxc` parser-mode entry logic onto that controller.
3. Replace the current `rxc` flat token-stream builder; it has no real reuse value beyond narrow token mapping ideas.
4. Generate the DSLSH tree from the source tree.
5. Add a compiler-token callback so punctuation, separators, brackets, and identifiers are projected authoritatively.

Goal of this stage:

- `rxc --syntaxhighlight` becomes source-tree based
- the current fragile flattened implementation is retired

### Stage 4: Add Caching for External Symbols and Exits

1. Add persistent caches to the highlighting controller for:
   imports, external procedures/classes/symbols, exit discovery, and exit module loading.
2. Reuse those caches between syntax-highlight calls in the same parser server process.
3. Re-run exits per parse if required, but do not repeatedly reload/discover them.
4. Add cache invalidation based on relevant inputs such as search paths, module identity, file timestamps, and language options.

Goal of this stage:

- semantic highlighting remains responsive
- per-keystroke side effects are mitigated by retained state

### Stage 5: Sync Semantic Results Back to the Source Tree

1. Populate symbol/type conclusions onto source-tree nodes through the work-to-source mapping.
2. Make the source tree the definitive view of:
   parsed code, clarified argument types, diagnostics, and metadata anchors.
3. Populate DSLSH `identifier_id` and richer semantic token distinctions from this source-owned state.

Goal of this stage:

- reading the source tree yields the canonical view of the user's code
- the work tree remains an implementation detail for compilation

### Stage 6: Reduce Redundant Work-Tree State Only After Proof

1. Review whether some work-tree attributes such as direct source spans or some symbol ownership should be reduced or removed.
2. Do this only after:
   mapping, diagnostics, metadata insertion, and tests are all stable.
3. Use this stage to prove that critical logic is genuinely going through the source tree.

Goal of this stage:

- strengthen the architecture without taking unnecessary early risk

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
