# cREXX Inlining Design

## Overview

Inlining in `rxc` is an AST rewrite performed during optimisation. The immediate goal is a buildable, staged implementation where the inliner only selects call sites whose rewrite strategy is already implemented.

That means selection is opportunity-based, not just callee-based. A procedure may be generally eligible for inlining, but only some uses of it may be rewritten in a given phase.

The selector is now best understood in terms of capability buckets rather than one-off scenarios:

- statement rewrites where the enclosing statement can be replaced directly
- eager value consumers such as `SAY` and `RETURN`

Contexts outside those buckets stay uninlined until their rewrite strategy exists.

## Current Strategy

### April 2026 review stance

The current implementation is not a completely generic AST rewrite engine, and
that is intentional. The general part is the reusable inline expansion core:

- bind actuals into an isolated inline scope
- duplicate callee-local scopes and symbols into that inline scope
- remap by-reference actuals, varargs, `§this`, and `§factory`
- clone the callee instruction tree
- rewrite returns according to the call-site bucket

The call-site buckets are deliberately specific. They are the points where
control-flow, expression value delivery, receiver copyback, and register
liveness differ. Treating those as "just tree copying" is unsafe, because a
syntactically valid copied tree can still change evaluation order, aliasing,
register lifetime, or object receiver state.

So the current direction is broadly correct: keep one reusable clone/bind core,
but keep explicit rewrite strategies for each parent-expression or statement
bucket. Future work should improve the clarity and test coverage of those
buckets rather than attempting one fully generic splice operation.

### Supported scenarios

The currently supported scenarios are:

```rexx
x = func(a)
```

and

```rexx
call func(a)
```

and

```rexx
say func(a)
```

and

```rexx
return func(a)
```

```rexx
if func(a) then ...
```

More precisely:

- The call may be a local plain-procedure `FUNCTION`, a local class `MEMBER_CALL`, or a local class `FACTORY_CALL`.
- For assignment inlining, the call must be the entire RHS of the enclosing `ASSIGN`.
- For standalone call inlining, the enclosing statement must be `CALL func(...)`.
- For expression inlining, the call must be the direct child of a single-value consumer such as `SAY`, `RETURN`, `IF`, `WHILE`, `UNTIL`, or a loop bound node. Dedicated statement rewrites still own standalone `CALL` statements and whole-RHS `ASSIGN` sites.
- The callee body must be available in the current compilation unit. Imported callees remain a later milestone.
- Arguments and return values must stay within the currently implemented safe slice: scalars, object values, binary values, optional/default formals, and array-shaped formals/returns are now supported across local procedure and local class-call inlining.
- The callee must satisfy the existing safety checks: small body, no recursion cycle, and no unsupported vararg indexing. Value-producing procedures still require a final `RETURN`; void statement-call sites may inline through bare-return and fallthrough shapes.
- Procedure-level `expose` clauses are not inlineable yet. This is separate from `arg expose` / by-reference formals: a procedure-level expose changes global binding semantics and must stay as a real call until the clone/bind core models that environment explicitly.
- Callable bodies containing raw assembler aliasing statements such as `link`, `linkattr*`, `linktoattr*`, or `unlink` are not inlineable yet. These statements encode VM aliasing semantics that are not visible in the ordinary AST contract, so they stay as real calls until the inliner has instruction-specific proofs.
- Factory inlining creates and initializes the callee's internal `§factory` object in the inline scope, including `setattrs` and `setobjtype`, before cloning the factory body.
- Method inlining binds the receiver once into the callee's internal `§this` object. Statement-position and simple whole-RHS assignment rewrites copy a direct, non-attribute receiver symbol back after a mutating method body. Mutating method calls in general expression position, receiver expressions, indexed receivers, and class-attribute receivers stay as normal calls until receiver copyback is modelled for those shapes.
- Named interface factory selector metadata must be preserved during cloning so `.iface.named(...)` still emits `srcfproc "...iface..named"` after inlining.
- `expose`/by-reference formals are supported when the actual argument is an aliasable variable-like target, including indexed and stem-style forms.
- For nontrivial by-reference actuals, the inline rewrite captures the locator expressions once into inline-scope temps so the callee still sees call-time binding semantics.
- Optional formals now inline through the same rewrite path as other supported local plain-procedure calls, with omitted-actual/default-formal semantics preserved during binding.

At present, expression rewriting is intentionally narrow. A `BLOCK_EXPR` inline is allowed only when the call is consumed directly by a single-value statement or control node. Composed expression parents remain fail-closed because the current register allocator does not yet prove that a `BLOCK_EXPR` body cannot reuse registers holding already-evaluated sibling expression results.

The discriminator therefore keeps these sites out of the `BLOCK_EXPR` path:

- whole-RHS `ASSIGN` sites, which are rewritten as assignment statements rather than expression children
- standalone `CALL func(...)` statements, which use the statement-position rewrite
- call arguments and receiver-position `MEMBER_CALL` expressions
- arithmetic, concatenation, comparison, and short-circuit operator parents

Composed inline sites should not be re-enabled until the emitter has a real liveness model for nested `BLOCK_EXPR` scopes or another proof that temporary register reuse cannot clobber sibling expression results.

The statement-position cases rewrite the enclosing statement. The expression-position case rewrites the call node itself to `BLOCK_EXPR`.

### Why selection must stay narrow
The inliner must not mark a procedure as globally "inline now" and then rewrite every call. That breaks the incremental rollout plan. Instead:

1. Identify procedures that are structurally inlineable.
2. Classify each call site by context capability bucket.
3. Rewrite only call sites whose bucket is already supported.

This preserves the ability to build and test the compiler while inlining support is still partial.

## Compiler-Added Statement Blocks vs `BLOCK_EXPR`

Both forms are still needed, but they no longer need separate node types.

### Compiler-added statement block

Use a compiler-added `INSTRUCTIONS` node for statement-position rewrites that need:

- scope isolation
- a sequence of injected statements
- no expression value of their own

This is the right tool for the current statement-position phases, because `x = func(a)` should become a statement block that contains:

1. argument binding
2. cloned callee body
3. `RETURN expr` rewritten to `x = expr`

The original assignment statement is replaced as a whole by the block.

### `BLOCK_EXPR`

Use this only when the inline expansion itself must remain an expression and yield a value to its parent expression tree.

This is now used for the first expression-position slice, and remains the path for broader later phases such as:

```rexx
x = func(a) + y
```

or other contexts where hoisting to surrounding statements would change semantics.

`BLOCK_EXPR` already has the compiler machinery for:

- scoped expression blocks
- `LEAVE_WITH` result propagation
- emitter support for yielding a register result

So for expression-valued inlining, the target form should be a `BLOCK_EXPR`, not a raw statement block.

### Decision

Do not collapse statement rewrites into `BLOCK_EXPR`.

Use:

- compiler-added `INSTRUCTIONS` for statement-only inlining rewrites
- `BLOCK_EXPR` for expression-valued inlining rewrites

Also avoid wrapping a `BLOCK_EXPR` around a compiler-added statement block unless a specific implementation need appears. For expression contexts, prefer constructing the final `BLOCK_EXPR` shape directly.

## AST Rewrite Plan

### Phase 1 rewrite: `x = func(a)`

Given:

```rexx
x = func(a)
```

rewrite the enclosing assignment into:

1. a compiler-added `INSTRUCTIONS` block
2. containing argument binding statements
3. containing the cloned callee statements
4. with the callee `RETURN expr` rewritten to `x = expr`

Important details:

- Formal parameter binding must target the formal variable node, not the `ARG` node.
- Actual arguments must be read from the real call shape used by `FUNCTION` nodes.
- The cloned body must use isolated local symbols and scopes.
- The result should be a valid statement tree without forcing a statement-only block into expression position.

### Phase 2 rewrite: `call func(a)`

Given:

```rexx
call func(a)
```

rewrite the enclosing call statement into:

1. a compiler-added `INSTRUCTIONS` block
2. containing argument binding statements
3. containing the cloned callee statements
4. with the callee `RETURN expr` rewritten so the expression is still evaluated even though the caller ignores the result

Important details:

- A bare `RETURN` with no expression can simply disappear in the inlined form.
- A `RETURN expr` must not be dropped, because the expression may still have side effects.
- The current implementation handles this by sinking the ignored return value inside the compiler-added block.

### Phase 3 rewrite: direct single-consumer expression inlining

Given:

```rexx
say func(a)
```

rewrite the `FUNCTION` node into:

1. a `BLOCK_EXPR`
2. containing argument binding statements
3. containing the cloned callee statements
4. with the callee `RETURN expr` rewritten to `LEAVE_WITH expr`

Important details:

- The `BLOCK_EXPR` must preserve the original call node's value and target typing.
- The current implementation admits supported local calls only as the direct child of a single-value consumer. Ordinary operators, short-circuit boolean parents, call arguments, and receiver-position member expressions remain excluded until register liveness is proven for composed `BLOCK_EXPR` expressions.
- Whole-RHS `ASSIGN` and standalone `CALL` sites still use dedicated statement-position rewrites instead of the expression path.

## Procedure Eligibility

For the current local-call slice, a callable is structurally eligible only if all of the following hold:

- not `main`
- a normal local procedure, a local class method, or a local class factory
- not a class-scope plain procedure
- local body available in the current compilation unit
- argument and return shapes stay within the implemented local-call slice, including aggregates, binary values, by-reference formals, by-value varargs, and the current constant-index `.ref` vararg support
- value-producing procedures still satisfy the current return-shape proof used by the inliner
- body size is below the configured node threshold
- no recursion cycle is introduced by the inline ancestry chain
- no unsupported dynamic `.ref` vararg access is required

Call-site selection remains separate from structural eligibility: an eligible callable may still have uninlined call sites if the parent rewrite bucket is not yet implemented or needs receiver copyback that the current expression rewrite cannot provide.

## Milestones

The milestone names below describe user-visible capability goals. They are intentionally broader than a single implementation slice, so each milestone is expected to contain several staged iterations.

### Milestone 1: all local procedures work

This milestone was delivered in two internal stages:

- `M1a`: local plain procedures within the current safe scalar slice, including fixed arguments, optional arguments, by-reference formals, and by-value varargs, across the currently supported call-site capability buckets
- `M1b`: local plain procedures beyond the current leaf restrictions, removing the temporary exclusions around nested calls, nested scopes, and single-trailing-`RETURN` only shapes

Notes:

- The current implementation now supports by-value varargs, nested-call local procedures, and nested callee scopes through a bounded fixed-point pass with explicit cycle blocking for self-recursive and mutually recursive expansions.
- Milestone 1 deliberately excluded methods/factories and class-scope procedures. Object by-value formals now follow the same split as normal calls: read-only formals alias the incoming binding, while writable formals materialise an isolated local copy. Array-shaped formals and returns are now handled by the same inline rewrite machinery as other supported local plain-procedure calls.
- Selection should remain opportunity-based throughout milestone 1: a structurally inlineable procedure may still have uninlined call sites if their rewrite bucket is not yet implemented.

### Milestone 1 review

Milestone 1 is now complete for local plain procedures.

That does not mean every syntactic use of a local procedure is now inlined. It means the remaining non-inlined cases are no longer caused by missing local-procedure body semantics such as varargs, nested calls, nested scopes, multi-return shapes, or aggregate value binding. Those local-procedure capability gaps are now closed.

The remaining discriminator behaviour at milestone-1 closure was intentional and milestone-driven:

- method and factory body inlining belonged to milestone 2
- imported callees belonged to milestone 3

So the milestone 1 closure is: local plain procedures are structurally and semantically covered; the remaining exclusions are later-milestone boundaries, not plain-procedure semantic gaps.

Decision taken at milestone-1 closure:

- keep the implementation on top of existing RXAS primitives rather than adding new VM or assembler instructions just for inlining
- use compiler-side capture and rewrite helpers to preserve evaluation-once and alias semantics
- support `.ref` / `expose` varargs for the common inlineable cases: `arg[]`, constant `arg[n]`, and constant `arg(n, "E")`
- keep forwarded constant `.ref` vararg elements semantically correct by allowing them to flow into inner `expose` calls while leaving those inner calls as normal calls instead of inventing a locator-table model mid-milestone
- defer dynamic `.ref` vararg indexing (`arg[ix]`, `arg(ix, "E")`) to a later design iteration; this is now treated as a post-milestone enhancement, not a milestone-1 blocker

### Milestone 2: local class method and factory inlining

The current implementation admits local class methods and local class factories into the same fixed-point rewrite loop as local plain procedures.

Implemented behaviour:

- `MEMBER_CALL` and `FACTORY_CALL` sites participate in call-site selection alongside `FUNCTION` sites.
- Method bodies get an inline-scope `§this` binding and class attribute reads/writes resolve against that inlined receiver.
- Direct-receiver mutating method calls in statement position and simple whole-RHS assignment position copy the receiver object back after the cloned method body.
- Direct-receiver mutating method calls in supported single-consumer expression position copy the receiver object back before each generated `LEAVE_WITH`, so the returned value and receiver mutation are both preserved.
- Factory bodies get an inline-scope `§factory` object initialized with the owning class's attribute count and concrete object type before the cloned factory body executes.
- Cloned `BLOCK_EXPR` and compiler-generated block associations are preserved so nested inline scopes continue to resolve `§this`, `§factory`, and `LEAVE_WITH` targets correctly.

Remaining guardrail:

- Mutating methods still stay uninlined when the receiver is not a direct symbol copyback target, for example receiver-producing expressions, indexed receivers, and class-attribute receivers.

### Milestone 3: imported procedures and methods

Milestone 3 requires an explicit design phase before implementation.

The key open question is how inlineable callee bodies are made available across module boundaries. Current import handling is sufficient for signature-driven resolution, but not yet for general cross-module body cloning.

The main design options currently in scope are:

- metadata carries a reusable AST/symbol representation for inlineable imported bodies
- imported REXX source is used as the body source for inlining

Those options should be evaluated during the milestone 3 design phase. Additional approaches may emerge, but milestone 3 should not be treated as just another selector extension until body transport and ownership are understood.

Also note that imported sources are not all equivalent:

- source imports may plausibly provide full bodies
- metadata-only imports such as `.rxbin` and plugin-driven imports may only provide signatures or stubs

So milestone 3 should likely begin with the narrower target of source-imported procedures and methods whose bodies are definitely available.

### Cross-file inline body transport working design

The preferred cross-file direction is an explicit inline-body export format,
stored as metadata in the imported module and reconstructed by `rxc` before the
normal optimisation/inlining pass. The existing `META_FUNC.inliner` string is
a natural first attachment point because it is already read during binary
import and currently carries an empty string. If the payload grows beyond what
is comfortable as a string, the same design can move to a dedicated
`META_INLINE_BODY` record while leaving `META_FUNC.inliner` as a pointer or
version marker.

The inline body should be a compiler-owned transport language, not source text
and not raw C struct memory. Source text is attractive for source imports, but
it is not enough for `.rxbin` imports and it reopens parse/validation drift.
Raw AST memory is not portable and cannot survive compiler version changes.

The original source language should be irrelevant by this stage. Level B,
Level C, exit-generated code, and other front-end inputs should all export the
same canonical inline format once they have been lowered into the common
validated AST. Source language, source file, and original text can remain as
debug/provenance fields, but they must not be required to reconstruct or prove
an inline body.

Cross-file inlining must remain subset-based. The exported subset may grow over
time, but the first rule is that a scenario which cannot be proved safe locally
must not become eligible merely because the callee came from another file. The
writer and reader therefore both have eligibility responsibilities:

- The writer exports inline-body metadata only for callables that are locally
  structurally inlineable independent of a particular call site.
- The reader reconstructs the imported callable as a template and still runs
  the normal inliner eligibility, arity, recursion, argument, and call-site
  bucket checks before rewriting a specific call.
- A callable can be exportable but not inlineable at a particular target site.
  For example, the callee body may be safe, while the caller uses it under an
  unsupported composed expression parent.
- A callable that fails writer-side structural gates should not carry inline
  metadata at all, because the metadata consumes space and cannot be used
  correctly by any reader.

Writer-side export should happen after local optimisation has reached its
inline fixed point. If a procedure has locally inlined another procedure, the
exported body for the outer procedure should include that already-inlined AST
shape, subject to the same total node-count and safety gates. This is required
so cross-file inlining can naturally support "inlining an already-inlined
function" without inventing a second expansion model.

Each callable still owns its own export decision and, if eligible, its own
exported body. For example, if `b()` inlines `c()` locally and `a()` inlines
`b()` locally, the writer may export the final body of `a()` and the final body
of `b()` separately, with each body containing whatever local inlining was
accepted before export. The node-count limit applies to each exported final
body, not merely to the original source body before local inlining.

This also means AST serialization and local tree surgery must agree on a stable
post-optimisation shape. Export must not serialize transient call-site state,
inline ancestry stacks, or mutable template nodes that the local fixed-point
pass still expects to revisit.

All procedures may eventually have serialized AST payloads for tooling,
diagnostics, or future compiler uses, including `main`. That is separate from
inline-body eligibility. A payload for `main` or another non-inlineable callable
must not be marked as an inline-consumable body unless the writer-side gates say
it is structurally inlineable and the reader-side call-site gates also accept
the target use.

A compact text form is used for the first implementation slice. It is versioned
and structured as a preorder tree walk with explicit scope and symbol records.
It is not JSON and it is not raw source text. The active version is `I2` and is
stored in the existing `META_FUNC.inliner` string. The importer still accepts
`I1` payloads for compatibility with the original scope-flattened prototype.

```text
I2
;q,<scope-id>,<parent-scope-id|-1>,<ScopeType>
;s,<id>,<scope-id>,<hex-name>,<ValueType>,<dims>,<flags>
;>,<scope-id>,<NodeType>,<value-type>,<target-type>,<value-dims>,<target-dims>,<flags>,<symbol-id|-1>,<int>,<bool>,<float>,<hex-node-string>,<hex-decimal>
;>
...
;<
;<
```

The important point is the `>` / `<` stream: it lets the importer reconstruct
the tree without depending on pointer addresses. Symbol records precede the
tree records and give the importer enough `Symbol` data for the current clone
and remap paths. Text fields are hex encoded so the payload remains safe inside
an RXAS quoted metadata string without carrying an escaping sub-language.

Scope id `0` is the imported procedure scope. Additional `q` records currently
describe local child scopes only; the node flags mark the node that owns each
scope so the normal clone path can duplicate scoped locals instead of flattening
them into the procedure.

The first reader/writer subset is intentionally narrow:

- exposed, optimized procedures only; methods and factories stay signature-only
  until their receiver and selector cases are handled explicitly
- scalar types only; arrays and object class names fail closed for now
- simple `IF` control flow is supported for scalar multi-return procedures,
  including `IF ... THEN DO ... END` blocks with local scalar temporaries
- no serialized associations yet, so `BLOCK_EXPR`, `LEAVE_WITH`, loop-control
  associations, class/interface dispatch, and remaining nested calls are not
  exported in this first slice
- simple expression/assignment/return/say nodes and ordinary scalar symbols

The reader reconstructs a detached compiler template. It does not attach the
body or its body-local symbols to the imported declaration in the caller AST,
so emission still produces a normal external declaration. The imported body is
only used as the symbol's `ast_template` during the optimiser and is cloned
into accepted call sites like local inline templates.

Binary, RXAS, and source import paths all feed the same metadata reader. Binary
and RXAS imports read the payload from `META_FUNC.inliner`; source imports run
the same writer while scanning exposed dependency procedures and store the
result in the import registry alongside the signature.

Full register allocation state should not be transported:
register assignment is a downstream compiler concern and must run after import
as it does for local source.

Full scope objects probably should not be serialized directly. Scope is mostly
derived from the tree and can be rebuilt by the normal fixup/validation
pipeline, provided the transport marks the nodes that introduce procedure,
class, method, factory, block, and generated local scopes. If later import work
finds a scope property that cannot be derived, add that property explicitly
rather than snapshotting the whole in-memory `Scope` structure.

Required contents as the cross-file subset grows:

- callable kind, fully qualified name, return type, argument signature, and
  inlining format version
- the callable body under its `INSTRUCTIONS` node
- local symbol table entries, including argument/ref/const/varg flags,
  aggregate dimensions, object class names, and default-init requirements
- scope-boundary markers sufficient for downstream walkers to rebuild
  procedure, generated-block, nested-block, method, factory, and class context
- associations needed by class/interface calls, `BLOCK_EXPR`, and generated
  `LEAVE_WITH` targets
- source anchors where available, with a clear flag for synthetic or stripped
  source
- a dependency list for any nested calls that remain as calls after inlining

Import behaviour:

1. `rxc` reads the existing function metadata as it does today.
2. If the imported function has a compatible inline-body payload, the compiler
   reconstructs an imported AST template, rebuilds symbol/scope information
   through the normal downstream walkers where possible, and attaches the
   resulting read-only template to the imported symbol as `ast_template`.
3. The reconstructed template must be good enough to re-enter the same
   validation, optimisation, clone, bind, and emitter assumptions used by local
   templates. Any missing data should fail closed at import time.
4. The normal `identify_inlinable_walker()` and call-site selection logic
   should not need a special imported-callee path beyond ownership checks and
   compatibility gates.
5. If the payload is missing, stripped, malformed, too new, or semantically
   incomplete, the compiler fails closed and treats the import as signature
   only.

Linker/strip behaviour:

- Library-oriented artifacts should preserve inline-body metadata by default,
  because they are intended to support downstream compilation and optimisation.
- Final linked binaries should strip inline-body metadata by default. It has no
  runtime purpose once linking is complete and it can consume significant
  space.
- The linker should expose explicit options for all three useful policies:
  preserve inline metadata, strip inline metadata, and use the default for the
  selected output kind.
- `STRIP INLINE` should remove inline-body payloads while preserving callable
  signatures.
- `STRIP SOURCE` may remove source/file metadata without removing inline-body
  metadata, unless a stronger deployable-strip mode is requested.
- When an inline body is stored through `META_FUNC.inliner`, stripping should
  rewrite the field to the canonical empty-string constant rather than remove
  the whole `META_FUNC` record.

Compatibility gates:

- inline-body format version must match a compiler-supported reader
- AST schema/compiler ABI must match or be explicitly declared compatible
- imported inline bodies must be treated as read-only templates and cloned
  into the caller, never mutated in place
- the writer must have proved the body is locally structurally eligible before
  exporting metadata
- the reader must independently prove the imported template is usable and the
  specific call site is supported before rewriting
- exported body dependencies must either be inlined too or remain resolvable
  as imports in the caller output

This design keeps cross-file inlining at the optimiser/inliner level: imports
produce callable templates, then the existing local inliner machinery does the
actual rewrite.

## Current Status

The implementation now covers:

- local plain procedures in statement and expression buckets across the current scalar slice
- local class methods and factories in the supported statement, assignment, and expression buckets
- optional/default formals with omitted-actual binding preserved in the inline body
- a production inline body cutoff of 200 AST nodes for local inlineable callables
- by-value trailing varargs
- dynamic vararg indexing and existence checks via inline-scope captured vararg arrays
- nested-call local procedures via repeated identify-and-inline passes until a bounded fixed point is reached
- nested callee-local scopes with duplicated scope and symbol remapping
- multi/early-return procedures, including branch returns and void fallthrough in statement-call sites
- object-typed returns and by-value object formals
- array-typed formals and array-valued returns, including assignment, expression, and temp-materialised return sites
- non-symbol aggregate actuals and non-symbol aggregate return expressions via inline temp materialisation
- broader aliasable by-reference actuals, including computed indexed/stem locator children
- `.ref` / `expose` varargs for `arg[]`, constant `arg[n]`, and constant `arg(n, "E")`, using existing RXAS argument/link primitives plus compiler-side capture helpers
- assignment-site inlining when the LHS itself has child selectors, by falling back to the RHS `BLOCK_EXPR` path
- binary-typed local plain procedures across the current statement and expression rewrite machinery
- preserved default-init requirements for duplicated inline locals and inline-created aggregate temporaries
- explicit cycle blocking so self recursion and mutual recursion do not expand indefinitely
- receiver copyback for direct-receiver mutating method calls in statement, simple assignment, and supported single-consumer expression rewrites
- factory object initialization for inlined class factories
- conservative emission-time pruning of private local plain procedures once no surviving local call node still targets them

The implementation still excludes:

- imported callees
- procedures with procedure-level `expose` clauses
- callables containing raw assembler aliasing statements
- dynamic-index `.ref` / `expose` vararg access
- mutating methods whose receiver is not a direct symbol copyback target
- receiver-position and call-argument inlining, such as
  `buildBox("seed").getName()`, pending a parent-expression liveness and
  evaluation-order proof
- composed expression parents, including ordinary operators and short-circuit
  boolean parents

## Post-Hardening Status

The temporary hardening phase for local plain procedures is complete.

The work that was previously fail-closed during broad-cutoff exploration is now part of the normal supported slice:

- scope/symbol duplication preserves default-init requirements for cloned locals and inline-created aggregate temporaries
- optional/default formals inline with omitted-actual/default-formal semantics preserved
- array-typed formals and array-valued returns inline in assignment, expression, and temp-materialised contexts
- whole-variable aggregate assignment emission preserves copy semantics for array and binary values in inline-expanded paths

The current production stance is:

- keep the 200-node body cutoff for local inlineable callables
- treat imported callees, unsupported mutating-method receiver copyback shapes, and dynamic-index `.ref` / `expose` vararg access as milestone-boundary exclusions, not temporary hardening gaps
- keep debug-visible inline rejection and rewrite diagnostics available for future tuning and milestone work

Validation for the current cutoff and supported slice:

- debug top-level `ctest` matrix passed
- release compiler suite passed
- debug and release `performance` label runs passed
- additional compiler gold drift exposed by the broader cutoff was refreshed only after the matching runtime tests stayed green

Future tuning in this area should prefer better profitability modeling over reintroducing structural fail-closed exclusions for already-supported local plain procedures. In particular, any later experimentation with larger limits should bias toward call-site-sensitive policy, such as hot/cold or loop-sensitive thresholds, rather than undoing the milestone-1 semantic coverage.

## Discriminator Review

The inline discriminator is now best understood as a two-stage filter.

Stage 1: structural procedure eligibility

- `identify_inlinable_walker()` answers whether a procedure body is inline-capable in principle
- it rejects `main`, class-scope plain procedures, unsupported vararg shapes, invalid value-return shapes, oversized bodies, and malformed vararg access
- it does not decide that every call must inline; it only marks the callee as structurally available

Stage 2: call-site capability selection

- `inline_validate_call_site()` answers whether a particular call is safe for the current rewrite machinery
- it applies recursion blocking, arity/varg checks, and the concrete binding rules for actual arguments
- statement rewrites then decide whether the enclosing node shape is one of the implemented buckets
- expression rewrites additionally pass through `inline_classify_expr_context()`, which currently admits only direct single-value consumers to the `BLOCK_EXPR` path and fails closed for composed expressions such as operators, call arguments, and receiver expressions

This split is the right shape for later milestones:

- new procedure semantics should extend stage 1 only when the cloned body model grows
- new parent-expression contexts should extend stage 2 without weakening structural safety checks

That separation is what keeps the inliner opportunity-based instead of turning `is_inlinable` into an over-broad “inline everywhere” flag.

### Known fail-closed local cases

The currently known local fail-closed cases are not all the same kind of gap.
They should be tracked separately so that future work fixes the right layer:

- Composed expression parents: examples such as
  `if identity(addOne(i)) | 0 then ...` and `if isTwo(i) | 0 then ...`
  remain normal calls because the direct child of `OP_OR`, `OP_AND`,
  concatenation, comparison, arithmetic, or call-like argument nodes is not yet
  a proven-safe `BLOCK_EXPR` insertion point.
- Short-circuit parents with by-reference side effects: examples such as
  `if refBump(values[idx]) & 1 then ...` are especially sensitive because the
  original language semantics include both evaluation order and aliasing.
- Receiver-position expressions: examples such as
  `buildBox("seed").getName()` and `.Box("direct").prefix("q")` require the
  receiver-producing expression to be evaluated once, materialised, and then
  used as the method receiver without clobbering siblings or losing copyback.
- Mutating methods in unsupported expression shapes: direct single-consumer
  sites such as `say box.setAndReport("epsilon")` are supported, but
  mutating methods still stay uninlined when the receiver cannot be copied
  back to a direct local symbol or when the call appears under an unsupported
  parent expression.
- Procedure-level `expose` clauses: these are deliberately distinct from
  `arg expose` reference formals. The latter is call-argument aliasing; the
  former changes the callee's global binding environment and remains a normal
  call until inline scope cloning models it directly.
- Raw assembler aliasing statements: callables using instructions such as
  `link`, `linkattr`, `linktoattr`, or `unlink` remain normal calls because
  those instructions can create aliases that are not represented as ordinary
  variable or receiver writes in the AST. Non-aliasing assembler does not by
  itself block inlining.
- Nested method calls inside an inlined method body: for example `prefix()`
  can inline its outer method body while leaving `label()` as a normal call
  when `label()` appears under a concatenation expression. This is expected
  until composed expression parents are supported.

The first implementation target should be chosen by semantic risk, not by how
often a fail-closed debug line appears. In particular, a repeated
`DEBUG_INLINE_FAILCLOSED` line can be caused by the fixed-point pass revisiting
an intentionally excluded site.

### Local follow-up: assembler alias register mapping

The local inliner should eventually support a safe subset of assembler-aliasing
helpers, but only after the register and lifetime model is explicit.

The current unsafe class is instructions that make one register an alias of
storage owned by another register or object, especially `link`, `linkattr*`,
`linktoattr*`, and `unlink`. The issue is register mapping, not assembler text
in general. The clone/remap core must be able to answer:

- what storage the linked destination register refers to after callee symbols
  have been remapped into the caller's inline scope
- whether the target storage outlives the inlined block
- whether an `unlink` must be preserved, moved, or paired with copyback
- whether the alias can observe mutations from both the caller and callee in
  the same order as a real call
- whether the linked register can be represented as an ordinary symbol/locator
  in the current inline machinery

Getter/setter-style methods that do not use these aliasing instructions should
remain inlineable. Cross-file inlining should inherit the same rule: do not
export or consume inline-body metadata for aliasing assembler bodies until the
local proof exists.

### Pruning fully inlined local callables

`rxcp_inline_prune()` now performs a conservative emission-time pruning pass
for private local plain procedures.

The safe pruning rule is deliberately conservative:

- never prune `main`
- never prune exposed procedures, methods, factories, class/interface contract members, imported stubs, or anything referenced by metadata needed at runtime
- never prune procedures with procedure-level `expose` clauses
- never prune a procedure while any remaining `FUNCTION`, assembler-operand `FUNC_SYMBOL`, `MEMBER_CALL`, `FACTORY_CALL`, or equivalent callable AST reference still targets it
- prune only local, private plain-procedure definitions whose body is no longer reachable after the fixed-point inlining pass

The important safety check is not whether the procedure was marked inlineable
in principle. It is whether all local call sites that target that procedure
have actually been rewritten. If even one surviving local call node still
resolves to the procedure symbol, pruning must leave the procedure emitted.

The implementation is a mark-and-sweep style loop:

1. Collect callable symbols reached from remaining AST call nodes and assembler function-symbol operands, skipping already-pruned callable bodies.
2. Mark private local plain procedures as pruned only when their symbol is absent from that live set.
3. Repeat until pruning one private helper no longer makes another private helper dead.

Physically removing AST definitions is more invasive because symbol tables,
metadata emission, and source provenance must be kept coherent. A lower-risk
first slice is therefore used: an emitter-visible prune flag tells procedure
emission to skip pruned private callable definitions. Physical tree removal can
come later if needed.

Each later iteration in this area should continue to use a full build, the compiler regression suite, the top-level `ctest` matrix, and the focused `performance` label as its validation gate:

- `cmake --build cmake-build-debug -j4`
- `ctest --test-dir cmake-build-debug/compiler/tests --output-on-failure`
- `ctest --test-dir cmake-build-debug --output-on-failure`
- `ctest --test-dir cmake-build-debug -L performance --output-on-failure`

## Symbol and Scope Handling

Inlining requires duplicating callee-local AST, symbols, and scopes into a new isolated scope under the caller.

The immediate goal is not a fully general subtree cloning framework. For phase 1, the duplication logic only needs to be correct for the supported procedure shape used by the first rewrite.

Requirements:

- caller-visible symbols must continue to resolve to the caller
- callee locals must be duplicated as fresh symbols
- duplicated nodes must link to duplicated symbols, not the original callee symbols
- nested local scopes inside the cloned body must remain isolated

If the existing generic duplication helpers are not yet sound for that, the phase 1 implementation should use a narrower remapping path rather than pretending the generic case is solved.

## Optimisation Pipeline

Inlining still happens during optimisation, after the main validation/fixup pipeline.

However, the implementation should not assume that any arbitrary post-validation AST mutation is automatically safe. The rewrite must produce a tree compatible with the downstream emitter and optimisation passes for the supported phase.

That is another reason phase 1 should replace the enclosing assignment statement with a statement block, rather than replacing an expression node with a statement-only node.

## Argument Semantics During Inlining

Inlining must preserve the same argument semantics as a normal call:

- `.ref` / `expose` formals must continue to alias the caller target selected at the call site.
- By-value formals that are written in the callee must behave as isolated locals, even if the VM's native call mechanism is by reference.
- Read-only by-value formals may reuse the incoming value when that is already allowed in the non-inlined path.
- Non-symbol temporaries may reuse storage when no caller-visible binding exists to preserve, but that remains an implementation choice, not a semantic change.

For validation purposes, any inline rewrite that changes whether a caller variable, array, or object binding can be observed after the call is wrong even if the emitted code is smaller or faster.

## Later Phases

### Next Phase
The next meaningful extensions are now milestone-driven rather than milestone-1 cleanup work:

- imported callees
- broader receiver copyback for mutating methods whose receiver is not a direct symbol
- any later reconsideration of dynamic `.ref` / `expose` vararg indexing

Recommended order from the April 2026 review:

1. Harden the existing local model with better fail-closed diagnostics and
   tests around the known excluded buckets. This keeps future inliner work
   auditable.
2. Implement one remaining local fail-closed capability gap, chosen by semantic
   risk rather than diagnostic frequency.
3. Design and prototype cross-file inline-body export/import as a separate
   milestone. Do not start it by threading special cases through the current
   local selector.

Outside milestone 2 and 3, the remaining exclusions now split cleanly into:

- post-milestone enhancements that may or may not be worth the complexity, such as dynamic `.ref` vararg indexing
- safety and cost controls that should remain in some form: recursion-cycle blocking and body-size gating
- non-goals or structural policy exclusions: `main`
- language-validity constraints that are not extra inline restrictions: arity mismatches and invalid non-void fallthrough/value-return shapes

## Verification

The local inlining design should be considered established when:

- the compiler rewrites `inline_test1`, `inline_test_call`, `inline_test_expr`, `inline_test_concat_expr`, `inline_test_say_expr`, `inline_test_return_expr`, `inline_test_unary_expr`, `inline_test_compare_expr`, `inline_test_call_arg_expr`, `inline_test_call_like_arg_expr`, `inline_test_nested_call_expr`, `inline_test_ref_opt`, `inline_test_ref_indexed`, `inline_test_ref_stem`, and `inline_test_class_methods` using the supported strategies
- excluded cases such as large procedures, imported callees, and mutating method expression calls remain uninlined under optimisation
- unsupported by-reference/short-circuit combinations such as those in `inline_test_ref_negative` remain uninlined under optimisation
- the resulting AST is structurally valid under `-dp`
- optimised codegen succeeds
- positive and negative compiler tests lock down the selector behaviour

## Non-Goals For The Current Design Boundary

- imported callees
- mutating methods in expression position until receiver copyback can be emitted before `LEAVE_WITH`
- aggressive pruning of fully inlined procedures
- claiming fully generic scope/symbol cloning before it is proven
