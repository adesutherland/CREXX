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
- For expression inlining, the call is rewritten to a `BLOCK_EXPR` when the parent bucket is supported. Supported buckets now include direct single-value consumers such as `SAY`, `RETURN`, `IF`, `WHILE`, `UNTIL`, and loop bounds, plus eager operators, comparisons, concatenation, short-circuit boolean operands, and function/factory/method argument positions. Dedicated statement rewrites still own standalone `CALL` statements and whole-RHS `ASSIGN` sites.
- The callee body must be available in the current compilation unit. Imported callees remain a later milestone.
- Arguments and return values must stay within the currently implemented safe slice: scalars, object values, binary values, optional/default formals, and array-shaped formals/returns are now supported across local procedure and local class-call inlining.
- The callee must satisfy the existing safety checks: small body, no recursion cycle, and no unsupported vararg indexing. Value-producing procedures still require a final `RETURN`; void statement-call sites may inline through bare-return and fallthrough shapes.
- Procedure-level `expose` clauses are not inlineable yet. This is separate from `arg expose` / by-reference formals: a procedure-level expose changes global binding semantics and must stay as a real call until the clone/bind core models that environment explicitly.
- Callable bodies containing raw assembler aliasing statements such as `link`, `linkattr*`, `linktoattr*`, or `unlink` are not inlineable yet. Stateful string-register assembler helpers such as `setstrpos`, `substcut`, and `dropchar` are inlineable when they operate through ordinary formal/local symbol bindings: assembler operands are marked read/write, writable by-value formals are copied like a normal call prologue, and `SCOPY` resets the destination cursor.
- Factory inlining creates and initializes the callee's internal `§factory` object in the inline scope, including `setattrs` and `setobjtype`, before cloning the factory body.
- Method inlining binds the receiver once into the callee's internal `§this` object. Statement-position and simple whole-RHS assignment rewrites copy a direct, non-attribute receiver symbol back after a mutating method body. Mutating method calls in general expression position, receiver expressions, indexed receivers, and class-attribute receivers stay as normal calls until receiver copyback is modelled for those shapes.
- Named interface factory selector metadata must be preserved during cloning so `.iface.named(...)` still emits `srcfproc "...iface..named"` after inlining.
- `expose`/by-reference formals are supported when the actual argument is an aliasable variable-like target, including indexed and stem-style forms.
- For nontrivial by-reference actuals, the inline rewrite captures the locator expressions once into inline-scope temps so the callee still sees call-time binding semantics.
- Optional formals now inline through the same rewrite path as other supported local plain-procedure calls, with omitted-actual/default-formal semantics preserved during binding.

Expression rewriting is still bucketed, but the `BLOCK_EXPR` bucket now covers composed expression parents whose evaluation order is already modelled by the emitter. The register allocator must not return a child register when the parent expression has adopted that same register as its own result; otherwise a later sibling, especially a nested `BLOCK_EXPR`, can reuse and clobber the value before the parent expression emits.

The discriminator therefore keeps these sites out of the `BLOCK_EXPR` path:

- whole-RHS `ASSIGN` sites, which are rewritten as assignment statements rather than expression children
- standalone `CALL func(...)` statements, which use the statement-position rewrite
- receiver-position `MEMBER_CALL` expressions
- mutating method expression sites that do not have a direct receiver copyback target
- imported method expression sites whose receiver is not direct

The current local proof is concrete rather than global: composed inline tests cover arithmetic, unary operators, comparisons, concatenation, nested call arguments, aggregate/object expression arguments, short-circuit side effects, and class-method sibling liveness. New composed buckets should get the same no-opt/opt runtime comparison before being opened.

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
- The current implementation admits supported local calls as direct single-value consumers and as supported composed expression children. This includes ordinary operators, short-circuit boolean parents, and call/factory/method argument positions. Receiver-position member expressions remain excluded until receiver materialisation and copyback are modelled for that shape.
- For call/factory/method argument positions, argument expression output is emitted before the caller's argument-count register is loaded and before argument marshalling starts. This is required because an inlined argument `BLOCK_EXPR` may legitimately use temporary registers in the caller's call frame before the final `swap`/`call` sequence is assembled.
- Whole-RHS `ASSIGN` and standalone `CALL` sites still use dedicated statement-position rewrites instead of the expression path.

## Procedure Eligibility

For the current local-call slice, a callable is structurally eligible only if all of the following hold:

- not `main`
- a normal local procedure, a local class method, or a local class factory
- not a class-scope plain procedure
- local body available in the current compilation unit
- argument and return shapes stay within the implemented local-call slice, including aggregates, binary values, by-reference formals, by-value varargs, and the current constant-index vararg support
- value-producing procedures still satisfy the current return-shape proof used by the inliner
- body size is below the configured node threshold
- no recursion cycle is introduced by the inline ancestry chain
- no unsupported dynamic vararg access is required

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
- defer dynamic vararg indexing (`arg[ix]`, `arg(ix, "E")`) to a later design iteration; this is now treated as a post-milestone enhancement, not a milestone-1 blocker

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
normal optimisation/inlining pass. The `META_FUNC.inliner` string was used only
for the `I4` proof of concept and is being retired before release. The durable
carrier is a first-class `META_INLINE` metadata record; `META_FUNC` keeps the
callable signature and procedure reference only.

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

The next format should be understood as "AST plus semantic metadata", not just
"more AST node types". The AST records describe what tree to clone. The
metadata records describe what that cloned tree means after it is imported into
another compile context. This distinction matters for source locations,
residual function calls, member/factory calls, class contracts, and any later
debugging or tooling use.

The constant pool is the right long-term carrier for this data. `rxas` already
turns `.srcfile`, `.src`, and `.meta` directives into linked metadata records,
and `rxlink` already rewrites, preserves, and strips those records by kind.
Inline metadata extends that path rather than introducing a separate binary
side channel. The RXAS source spelling is:

```rxas
.meta "fully.qualified.callable"=".inline" "I4;..."
```

The payload is still the compact compiler-owned inline transport. `rxas` stores
it as a `META_INLINE` record whose fields reference normal constant-pool string
entries, so the existing rxbin constant-pool compression path is used first.
Any later binary packing layer must be justified by measured size, validation,
or round-trip needs rather than by assumption.

Existing source metadata can be leveraged, but not by reverse-mapping old
instruction addresses. `META_SRC` and `META_FILE` are address-stamped after
assembly; an imported inline body is re-emitted at new addresses in a different
caller. The portable unit is therefore the AST source anchor:

- source file identity
- line and column
- source text/span used for the `.src` payload
- source provenance, such as exact, inherited, synthetic, composite, or stripped

When an inline body is imported, the reader should restore those source anchors
onto the reconstructed AST. Normal code emission can then emit fresh `.srcfile`
and `.src` directives at the new instruction addresses, and `rxas` can continue
to populate the constant pool through its existing metadata machinery. This
keeps panic reports, tracing, and debugging tied to the original inlined source
without treating old instruction addresses as stable.

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

The first implemented dependency slice follows that rule rather than
serializing unresolved nested calls. A callable whose nested helper call has
already been rewritten locally may export the resulting final AST. A callable
that still contains a `FUNCTION`, method, or factory call in its payload remains
fail-closed until the dependency-list format and reader-side resolution rules
are designed.

Compiler-added statement blocks produced by local inlining may still carry an
association back to the local callee definition as inline provenance. That
association is not a runtime control-flow edge and is intentionally omitted
from the cross-file payload. Other associations, such as loop-control targets
or `BLOCK_EXPR` / `LEAVE_WITH` links, remain fail-closed until their target
serialization is explicit.

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

A compact text form is used for the implementation slice. It is versioned and
structured as a preorder tree walk with explicit file, source, scope, symbol,
argument, and body records. It is not JSON and it is not raw source text. The
active version is `I4`. The `I1`, `I2`, `I3`, and `META_FUNC.inliner` variants
were internal prototypes; none of the inline formats has been released, and the
development team can rebuild all source artifacts. Backward compatibility with
these prototype inline payloads is therefore not required.

`I4` uses semicolon-separated records. Text fields are hex encoded so the
payload remains safe inside an RXAS quoted metadata string without carrying an
escaping sub-language. Dimension lists use colon-separated integers, and `-`
means absent or zero-dimensional.

```text
I4
;f,<file-id>,<hex-source-file-name>
;u,<source-id>,<file-id|-1>,<line>,<column>,<SourceProvenance>,<hex-source-text>
;q,<scope-id>,<parent-scope-id|-1>,<ScopeType>
;s,<symbol-id>,<scope-id>,<hex-name>,<SymbolKind>,<ValueType>,<dims>,<dim-base-list>,<dim-elements-list>,<hex-class>,<flags>,<register-type>,<register-number>
;d,<dependency-id>,<hex-fully-qualified-callable>,<hex-return-type>,<hex-arg-signature>
;a
;>,<scope-id>,<source-id|-1>,<NodeType>,<value-type>,<target-type>,<value-dims>,<target-dims>,<value-base-list>,<value-elements-list>,<target-base-list>,<target-elements-list>,<hex-value-class>,<hex-target-class>,<flags>,<symbol-id|-1>,<dependency-id|-1>,<symbol-read>,<symbol-write>,<int>,<bool>,<float>,<hex-node-string>,<hex-decimal>
;<
;b
;>,<scope-id>,<source-id|-1>,<NodeType>,<value-type>,<target-type>,<value-dims>,<target-dims>,<value-base-list>,<value-elements-list>,<target-base-list>,<target-elements-list>,<hex-value-class>,<hex-target-class>,<flags>,<symbol-id|-1>,<dependency-id|-1>,<symbol-read>,<symbol-write>,<int>,<bool>,<float>,<hex-node-string>,<hex-decimal>
;<
```

The important point is the `>` / `<` stream: it lets the importer reconstruct
the tree without depending on pointer addresses. Symbol records precede the
tree records and give the importer enough `Symbol` data for the current clone
and remap paths. The final register fields are used for class attributes in
imported member bodies, where the method body must still address the owning
object's attribute slot after the template is reconstructed. Node records also
carry the original symbol-node read/write usage; this is semantically important
for nodes such as loop `BY`, where the node is not a `VAR_TARGET` but still
writes the loop control variable.

The `a` section contains the callable `ARGS` tree. Carrying that tree is
required for binary imports because ordinary function metadata captures arity
and type signatures but not default-expression ASTs for optional formals. The
`b` section contains the callable `INSTRUCTIONS` body.

Dependency records precede the tree records. The first implemented dependency
slice is intentionally narrow: a residual direct `FUNCTION` node may refer to a
namespace-exposed plain `PROCEDURE` by dependency id. The writer records the
fully-qualified callable name plus the normal metadata return type and argument
signature. The reader resolves the fully-qualified dependency through the
current AST/import registry, verifies the import metadata signature when it is
available, attaches the resulting `FUNCTION_SYMBOL`, and fails closed if that
proof cannot be made. Statement-position `CALL` is transported as its wrapper
node plus the child `FUNCTION` dependency. `MEMBER_CALL` and `FACTORY_CALL`
remain closed because they need receiver, selector, dispatch, and class-layout
proofs that are not in this table.

The source table is deliberately separate from node records so repeated source
anchors and file names deduplicate naturally. Callable namespace identity is
not stored as a scope table entry; it is carried by the metadata key
(`.meta "fully.qualified.callable"=".inline" ...`) and the ordinary callable
signature metadata.

The source-anchor proof deliberately serializes only safe source spans: short
text spans with a valid start/end order and no embedded NUL bytes. Unsafe or
oversized spans are omitted from the inline payload and the reader treats the
affected node as having no portable source anchor. The emitter also refuses to
write invalid or pathological source spans as `.src` directives. This keeps the
proof robust while still preserving the useful debugger/tracing case: ordinary
callee source lines from imported inline bodies are re-emitted at the caller's
new instruction addresses with the original `.srcfile`.

Member and factory dependencies still need richer proof data: owner class or
interface identity, member kind, selector or `match` contract, receiver
requirements, and any class layout facts needed to prove attribute access. They
should not be unlocked just because direct function dependencies are now safe.

Scope id `0` is the imported procedure scope. Additional `q` records currently
describe local child scopes only; the node flags mark the node that owns each
scope so the normal clone path can duplicate scoped locals instead of flattening
them into the procedure.

The first reader/writer subset is intentionally narrow:

- exposed, optimized procedures
- source-imported scalar getter-style methods whose body can be reconstructed
  from source contract metadata and whose receiver is a direct symbol at the
  call site
- local methods and factories, including simple scalar getters and setters
- RXAS/RXDAS transport of eligible class method metadata for round-trip
  coverage, even though binary-imported member bodies are not consumed yet
- scalar, array-shaped, binary, and object-class shapes are transported for
  plain procedures
- optional/default formal argument trees are transported so omitted-actual
  binding works for binary imports as well as source imports
- by-value trailing varargs are transported for `arg[]`, constant `arg(n)`,
  and constant existence/value nodes that are already proven safe by the local
  inliner. Dynamic vararg indexing is transported by the codec but rejected by
  the writer/reader eligibility gates until the generated expression-block
  lowering is register-safe.
- non-aliasing raw `ASSEMBLER` nodes are transported as ordinary AST nodes.
  The writer and reader still reject aliasing assembler such as `link`,
  `linkattr*`, `linktoattr*`, and `unlink`.
- simple `IF` and simple loop control flow is supported for multi-return
  procedures, including `IF ... THEN DO ... END`, counted `DO`, and
  `DO WHILE` / `DO UNTIL` blocks with local temporaries. `LEAVE` and
  `ITERATE` remain excluded by the association gate until jump-target
  serialization is designed.
- no serialized associations yet, so `BLOCK_EXPR`, `LEAVE_WITH`, loop-control
  associations, class/interface dispatch, imported factory bodies, binary-imported
  member bodies, and remaining nested calls are not exported or consumed in
  this first slice
- simple expression/assignment/return/say nodes and ordinary scalar, array,
  binary, and object symbols

The reader reconstructs a detached compiler template. It does not attach the
body or its body-local symbols to the imported declaration in the caller AST,
so emission still produces a normal external declaration. The imported body is
only used as the symbol's `ast_template` during the optimiser and is cloned
into accepted call sites like local inline templates.

Binary, RXAS, and source import paths all feed the same metadata reader for
plain procedures. Binary and RXAS imports read `META_INLINE`; source imports run
the same writer while scanning exposed dependency procedures and store the
result in the import registry alongside the signature. `rxas` and `rxdas`
round-trip the same `.meta ... ".inline" "I4;..."` spelling, and the binary
cross-file test deliberately reassembles the RXDAS output before importing it
so source/RXAS/binary drift is caught. Imported member-body templates are
currently attached only for source contracts. Binary class contract metadata
does not yet preserve enough class layout information to prove arbitrary
runtime-library getters/setters safe, so binary-imported methods and factories
deliberately remain normal calls until that metadata grows.

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

Initial implementation status:

1. `rxc` writes `I4` payloads through first-class `META_INLINE` records.
   `META_FUNC` remains the callable signature and procedure-reference record.
2. The writer emits `f` and `u` source-anchor sections plus a per-node
   `source-id` field. The reader restores `file_name`, line, column, source
   provenance, and source text/span for imported inline templates.
3. Emission handles source-file changes inside inlined code by emitting the
   appropriate `.srcfile` before `.src` when an inlined node's source file
   differs from the currently active emitted source file.
4. The focused cross-file fixture validates a non-aliasing assembler helper and
   asserts that optimized caller RXAS contains meaningful callee `.src` text
   rather than empty source markers.
5. The same fixture validates source import, RXAS assembly, RXDAS disassembly,
   reassembly, binary-only import, and runtime output. It covers plain
   procedures, nested local scopes, early returns, counted and conditional
   loops, scalar/array/object/binary shapes, optional defaults, by-reference
   formals, by-value varargs, class method transport, and fail-closed callable
   signatures for unsupported bodies.
6. Only after this proof should the format grow the next table. The likely next
   table is a dependency table for residual direct `FUNCTION`/`CALL`
   dependencies. Member and factory dependencies should remain a later step
   because they need richer class/interface proof metadata.

The initial `META_INLINE` implementation deliberately stores the payload as a
normal string-pool entry referenced by metadata. This assesses the existing
rxbin constant-pool compression path in production rather than adding a second
packing scheme prematurely.

Dedicated round-trip coverage keeps the metadata transport from drifting
between source, RXAS, and binary paths. The harness proves:

- `rxc` emits `META_INLINE` for exportable callables and does not put inline
  payloads in `META_FUNC`
- `rxas` assembles the RXAS source spelling into `META_INLINE`
- `rxdas` emits `META_INLINE` back into the same logical RXAS spelling
- `rxc` can import the binary metadata and reconstruct the same inlining
  template used by source imports
- source anchors survive import and are re-emitted as new `.srcfile` / `.src`
  directives at the caller's instruction addresses
- unsupported nodes may be transported by the codec only when explicitly
  supported by the writer, and inlining still fails closed at reader/call-site
  gates

The expansion rule is intentionally stepwise: add one metadata table or one
node-family capability, prove it locally and cross-file, update diagnostics and
goldens, then move to the next gate. A payload that is richer than the reader
can fully prove should still fail closed.

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
- Stripping inline metadata should remove `META_INLINE` records while leaving
  `META_FUNC` callable signatures intact.

Compatibility gates:

- inline-body format version must match a compiler-supported reader
- AST schema/compiler ABI must match or be explicitly declared compatible
- imported inline bodies must be treated as read-only templates and cloned
  into the caller, never mutated in place
- the writer must have proved the body is locally structurally eligible before
  exporting metadata
- the reader must independently prove the imported template is usable and the
  specific call site is supported before rewriting
- exported body dependencies must either be inlined before export or, after the
  future dependency table exists, remain resolvable as imports in the caller
  output. The current writer fails closed on residual calls.

This design keeps cross-file inlining at the optimiser/inliner level: imports
produce callable templates, then the existing local inliner machinery does the
actual rewrite.

## Current Status

The implementation now covers:

- local plain procedures in statement and expression buckets across the current scalar slice
- local class methods and factories in the supported statement, assignment, and expression buckets
- optional/default formals with omitted-actual binding preserved in the inline body
- a production inline body cutoff of 300 AST nodes for local inlineable callables
- by-value trailing varargs with count and constant-index access
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
- source, RXAS, and binary imported plain procedures when the imported module
  carries compatible `I4` inline metadata
- cross-file transport of non-aliasing raw `ASSEMBLER` statements, so helpers
  built on instructions such as `strlen` can export and inline
- cross-file transport and inlining of by-value vararg plain procedures when
  the body only needs count or constant-index vararg access
- RXAS/RXDAS round-trip preservation of class method inline metadata, with
  source-imported getter bodies and binary-imported scalar getter/setter
  method bodies consumable by the inliner

The implementation still excludes:

- imported callees that have no compatible inline-body payload
- binary-imported factories and non-scalar member layouts until class-layout,
  factory-selection, and object-lifetime metadata is rich enough to prove them
- procedures with procedure-level `expose` clauses
- callables containing raw assembler aliasing statements
- dynamic-index vararg access (`arg.i`, `arg(i)`, `arg(i, "E")`)
- mutating methods whose receiver is not a direct symbol copyback target
- receiver-position inlining, such as `buildBox("seed").getName()` or
  `items[i].setName("x")`, pending a parent-expression liveness,
  materialisation, and copyback proof
- remaining expression parent shapes whose `BLOCK_EXPR` cannot yet be proved as
  a direct single-value consumer with call-equivalent side-effect order

## Post-Hardening Status

The temporary hardening phase for local plain procedures is complete.

The work that was previously fail-closed during broad-cutoff exploration is now part of the normal supported slice:

- scope/symbol duplication preserves default-init requirements for cloned locals and inline-created aggregate temporaries
- optional/default formals inline with omitted-actual/default-formal semantics preserved
- array-typed formals and array-valued returns inline in assignment, expression, and temp-materialised contexts
- whole-variable aggregate assignment emission preserves copy semantics for array and binary values in inline-expanded paths

The current production stance is:

- keep the 300-node body cutoff for local inlineable callables
- treat imported callees without compatible payloads, unsupported
  mutating-method receiver copyback shapes, and dynamic-index `.ref` /
  `expose` vararg access as milestone-boundary exclusions, not temporary
  hardening gaps
- keep debug-visible inline rejection and rewrite diagnostics available for
  future tuning and milestone work. `-d2` and higher also emit
  `DEBUG_INLINE_EXPORT` reasons when a callable does not receive exported
  inline metadata.

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

- Composed expression parents are now supported for the proven local buckets:
  arithmetic, unary operators, comparisons, concatenation, short-circuit
  boolean operands, and call-like argument nodes. Any new composed bucket still
  needs a register-liveness and evaluation-order test before it is opened.
  Call-like argument tests must include a nested inlined argument expression so
  the caller's argument-count register cannot be loaded too early and then
  clobbered by the child expression.
- Short-circuit parents with by-reference side effects are supported for the
  same local call slice as other `BLOCK_EXPR` expression sites. They remain a
  useful regression focus because the original language semantics include both
  evaluation order and aliasing.
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

### Inline gate inventory

This inventory is the working tracker for closed inline paths. It separates
writer/export gates from caller/call-site gates because the fix is often at a
different layer. A body can be locally inlineable but not exportable, and an
exported body can still be rejected at a particular call site.

The May 2026 inventory was gathered from the release compiler with `rxc -d2`,
using each library's CMake-style import flags and counting only diagnostics
whose source anchor was the file being compiled. The Level B BIF sweep uses the
111 modules listed in `lib/rxfnsb/rexx/CMakeLists.txt`; four extra scratch or
example `.rexx` files in that directory were excluded from the build-module
counts. The class library sweep covers the 19 `.rexx` sources in
`lib/classlib`, and the simple Level G sweep covers `lib/rxfnsg/rexx/llm.rexx`.

`DEBUG_INLINE_FAILCLOSED` lines are raw visitor diagnostics. The fixed-point
pass can revisit the same source site, so those counts are a shape signal, not
a callable denominator.

| Source set | Sources | Files with `.inline` | `.inline` records | Writer/export rejects | Caller fail-closed lines | Assessment |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Level B BIFs (`rxfnsb`) | 111 | 75 | 185 | 208 | 583 | Coverage is materially better than the earlier 33-file/110-record baseline, and the 300-node cutoff admits five more BIF source files than the 200-node sweep. The remaining rejects are still concentrated in large BIFs, private helpers, procedure expose, address/trace/signal helpers, and wider class layouts. |
| Class library (`classlib`) | 19 | 19 | 190 | 43 | 23 | Transport coverage is broad: every classlib source emits metadata. The remaining blockers are mostly collection/iterator class layout and factory semantics rather than missing metadata plumbing. |
| Simple Level G (`rxfnsg`) | 1 | 1 | 7 | 5 | 18 | The `llm` module is represented and useful as a higher-level smoke case. Its residual gates are object-valued attributes, one member call in an exported body, and mutating method expression positions. |
| Total | 131 | 95 | 382 | 256 | 624 | This is the release inventory for the 300-node cutoff and a good handover baseline for post-release inliner work. |

Writer/export reject counts:

| Gate shape | BIFs | classlib | Level G | Total | Assessment |
| --- | ---: | ---: | ---: | ---: | --- |
| Unportable class attribute shape | 46 | 26 | 4 | 76 | Largest remaining class-related gate. Scalar attribute layouts are open; aggregate/object/binary attributes still need an explicit layout and copy/reference proof. |
| Body exceeds `INLINE_MAX_NODES` | 32 | 0 | 0 | 32 | Pure profitability/size policy, mostly large BIFs. The release cutoff is 300 nodes; this drops the cutoff bucket from 50 to 32 compared with the 200-node sweep. |
| Callable is not namespace-exposed | 32 | 1 | 0 | 33 | Mostly policy. Private helpers should usually inline into exposed wrappers locally, not be exported individually. |
| Unsupported `BLOCK_EXPR` association | 21 | 9 | 0 | 30 | A transport/tree-shape gate, not the same as the caller-side expression rewrite. It often appears after local inlining creates richer payload structure. |
| Unsupported `LEAVE` association | 20 | 0 | 0 | 20 | Loop/control-flow transport gate. Keep separate from simple expression inlining. |
| Procedure-level `expose` | 16 | 0 | 0 | 16 | Environment-binding semantic gate. |
| Assembler aliasing instruction | 13 | 0 | 0 | 13 | Correctly closed until aliasing effects can be represented per operand. |
| Residual plain function dependency without proof data | 11 | 0 | 0 | 11 | Direct residual calls are supported when the target has importable proof data; these remaining cases are generated/private/procedure-expose helpers that still lack it. |
| Unsupported `FACTORY_CALL` node | 1 | 7 | 0 | 8 | Material for classlib iterator factories; depends on factory/match semantics and object lifetime proof. |
| Decimal/numeric-control nodes | 5 | 0 | 0 | 5 | Small codec expansion candidate. |
| Inline metadata collection failed | 4 | 0 | 0 | 4 | Investigate individually; keep closed until the exact missing tree shape is known. |
| Unsupported `ITERATE` association | 4 | 0 | 0 | 4 | Loop/control-flow transport singleton family after the 300-node sweep. |
| Unsupported vararg access | 2 | 0 | 0 | 2 | Dynamic vararg access remains semantically risky. |
| Unsupported `MEMBER_CALL` node | 1 | 0 | 1 | 2 | Receiver/dispatch proof is still missing for residual member dependencies. |

Caller-side fail-closed counts:

| Gate shape | BIFs | classlib | Level G | Total | Assessment |
| --- | ---: | ---: | ---: | ---: | --- |
| Expression context belongs to a dedicated statement rewrite | 444 | 23 | 15 | 482 | Raw and pass-noisy, but still the top caller-side signal. Some contexts are already handled by dedicated rewrites; the remaining sites need to be reviewed by parent shape rather than opened as a blanket rule. |
| `BLOCK_EXPR` expression needs a direct single-value consumer | 102 | 0 | 0 | 102 | Semantically valid only when the destination can be proven. The count rises with the 300-node cutoff because more near-threshold BIF bodies are analysed deeply enough to expose their next call-site blocker. |
| Recursive inline cycle detected | 24 | 0 | 0 | 24 | Safety gate; keep closed. |
| Failed to bind inline call arguments | 10 | 0 | 0 | 10 | Usually arity/default/vararg proof rather than an export problem. |
| Mutating method multi-return assignment needs statement-position copyback | 3 | 0 | 3 | 6 | Receiver/copyback gate; important for Level G and future iterator-style APIs. |

Current tests give good coverage for the open slice: 56 local `inline_test_*`
compiler tests cover the supported statement/expression buckets, refs,
varargs, recursion negatives, nested scopes, aggregate/object/binary returns,
class methods/factories, and known negative buckets. The cross-file fixture
then covers source import, RXAS assembly, RXDAS disassembly, reassembly,
binary-only import, source-anchor preservation, class method metadata
round-trip, and fail-closed callable signatures.

Reason classes:

- `semantic dependency`: blocked until a missing language/runtime/compiler model exists
- `policy`: intentionally closed unless the project changes the export/visibility policy
- `profitability`: semantically possible but needs a cost/size decision
- `codec/transport`: blocked by inline metadata format coverage
- `safety`: must remain closed or tightly guarded
- `complete`: opened for the current proven slice

| ID | Layer | Gate | Reason class | BIF signal | Assessment | Next review |
| --- | --- | --- | --- | --- | --- | --- |
| `W1` | Writer/export | Residual callable nodes in payload: direct `FUNCTION` dependencies and statement `CALL` wrappers are supported when proof data exists; `MEMBER_CALL`, `FACTORY_CALL`, and generated associations such as `BLOCK_EXPR`, `LEAVE`, `ITERATE` remain closed | mixed: complete for direct calls with proof data, semantic dependency for the rest | Current export counts: residual function dependency 11/0/0, `BLOCK_EXPR` 21/9/0, `LEAVE` 20/0/0, `ITERATE` 4/0/0, `MEMBER_CALL` 1/0/1, `FACTORY_CALL` 1/7/0 for BIFs/classlib/Level G. | Direct residual procedure calls now use the `;d` dependency table and reader-side FQN/signature validation, so imported inline bodies can leave helper calls as normal calls after the outer body is inlined. | Keep member/factory dependencies and generated associations closed until their richer proof data exists. Treat each association family as its own gate. |
| `W2` | Writer/export | Class attribute shape must be portable for method/factory payloads; scalar integer/boolean/float/string attributes are allowed and binary-imported method bodies can consume those layouts, aggregate/object/binary class attributes are not | mixed: complete for scalar method layouts, semantic dependency for wider layouts/factories | Current export counts: 46 BIF, 26 classlib, 4 Level G rejects. This is the largest remaining class-related gate and the main classlib/Level G blocker. | Scalar member metadata now carries explicit attribute register fields, and binary-imported method bodies attach payloads. Factory payload consumption remains source-contract-only for now because constructor semantics and match/provider selection need a wider proof. | Leave aggregate/object/binary attrs and binary factory payload consumption closed until class-layout and factory semantics are explicitly proved. |
| `W3` | Writer/export policy | Callable must be namespace-exposed to carry `.inline` metadata | policy | Current export counts: 32 BIF, 1 classlib, 0 Level G private helper rejects. | Mostly deliberate. Private helpers should not be exported individually just to improve metadata counts. If an exposed procedure locally inlines a private helper, the helper's final AST is already folded into the exposed export. | Leave closed for standalone export. Revisit only with a dependency-table design or if tooling wants non-inline AST payloads distinct from inline-consumable bodies. |
| `W4` | Structural/export | Body cutoff: `INLINE_MAX_NODES` is 300 for the release slice | profitability | Current export counts: 32 BIF, 0 classlib, 0 Level G cutoff rejects; the earlier 200-node sweep had 50 BIF rejects. | Valuable but not a semantic proof issue. The cutoff is intentionally a profitability/metadata-size policy, not a safety proof. Raising it admitted the near-threshold BIFs while leaving very large helpers closed. | For post-release work, consider call-site-sensitive profitability, loop-sensitive thresholds, or a library-only export threshold rather than treating 300 as semantically special. |
| `W5` | Structural/export | Procedure-level `expose` clauses are not inlineable/exportable | semantic dependency | Current export counts: 16 BIF, 0 classlib, 0 Level G rejects, mostly runtime-state helpers. | Semantically real. `arg expose` is already modelled; procedure-level `expose` changes global/caller environment binding. | Leave closed until inline scope cloning explicitly models procedure expose. Good candidate for a design note before code. |
| `W6` | Structural/export | Raw assembler aliasing through `link`, `linkattr*`, `linktoattr*`, `unlink`; non-aliasing stateful helpers rely on call-prologue-equivalent copy isolation | semantic dependency | Current export counts: 13 BIF, 0 classlib, 0 Level G aliasing rejects. Stateful string helpers are covered by `SCOPY` cursor reset and formal read/write binding tests. | Aliasing remains correctly closed because it creates storage identity not represented in normal AST symbol links. Stateful string helpers are no longer closed as a class: `setstrpos`, `substcut`, and `dropchar` may inline when their operands are ordinary formals/locals and no aliasing instruction is present. | Keep aliasing closed until assembler instruction effects can be modelled per operand and locally proved. |
| `W7` | Writer/export codec | Decimal/numeric-control AST nodes such as `DEC_STANDARD`, `DEC_FUZZ`, `DEC_FORM`, `DEC_DIGITS`, `DEC_CASE` are not transported | codec/transport | Current export counts: 5 BIF, 0 classlib, 0 Level G rejects. | Likely low-risk codec work, but only useful for the few numeric-control helpers. | Candidate for a small later slice if numeric BIF coverage matters. Add focused transport tests first. |
| `W8` | Writer/export codec | Unsupported scope/symbol/node families outside the current whitelist | codec/transport | Low in current BIF scan beyond the named gates | Keep whitelist-based. Expanding the codec is cheap only when the downstream imported template can still be validated and emitted. | Add one node family at a time with source/RXAS/RXDAS/binary tests. |
| `C1` | Caller/call-site | Composed-expression `BLOCK_EXPR` inlining | mixed: complete for proven parent shapes, semantic dependency for the remaining expression parents | Current raw caller counts: expression-context gate 444/23/15 and direct-consumer `BLOCK_EXPR` gate 102/0/0 for BIFs/classlib/Level G. | Opened for the local proven slice: arithmetic, unary operators, comparisons, concatenation, short-circuit operands, and call-like argument positions. Raw residual counts must be reviewed by parent shape because fixed-point revisits inflate them and some contexts are intentionally handled by separate rewrites. Receiver-position expressions remain separate. | Keep covered through the composed-expression runtime tests; add one no-opt/opt runtime test for each new composed bucket. |
| `C2` | Caller/call-site | Receiver-position inlining remains closed for general cases | semantic dependency | Covered by existing negative tests; BIF impact appears through `MEMBER_CALL` export rejects | A lowering such as `__receiver = items[i]; __receiver.setName("x"); items[i] = __receiver` is possible, but it performs two general object/attribute copies and needs locator capture so computed receivers are evaluated once. Keep closed until receiver references, move semantics, or a deliberate materialise/copyback protocol is designed. | Leave closed until receiver reference/move/materialisation semantics are agreed. Do not keep revisiting as a generic inliner gap. |
| `C3` | Caller/call-site | Mutating method inline requires a direct receiver copyback target | semantic dependency | Current raw caller counts: 3 BIF, 0 classlib, 3 Level G copyback rejects. | Correctly conservative. Non-direct receivers need materialisation plus copyback semantics. | Leave closed until receiver materialisation/copyback is designed. |
| `C4` | Import/call-site | Source-imported getter-style member templates and scalar binary-imported method templates are supported; binary-imported factories and non-scalar member layouts remain closed | mixed: complete for scalar methods, semantic dependency for the rest | Cross-file tests now prove RXAS/RXDAS round-trip plus binary import consumption for scalar getter/setter methods. All 19 classlib files emit inline metadata, but classlib still has 26 non-scalar layout rejects and 7 factory-call rejects. | This opens the class-library getter/setter path without pretending general receiver, factory, or aggregate layout semantics are solved. | Keep the remaining member/factory cases tied to their explicit W2/C2/C3 dependency notes. |
| `C5` | Caller/call-site | Dynamic vararg indexing is closed for both by-value and `.ref` / `expose` varargs | semantic dependency | Current export counts: 2 BIF dynamic-vararg rejects; current raw caller counts include 10 BIF argument-binding rejects. | Semantically important. Reference varargs need locator arrays or per-index alias capture. By-value dynamic access currently lowers through generated expression blocks and captured arrays; the April 2026 regex regression showed that float comparison parents can alias a generated result register and change `max(1, len)` into an unsafe advance. | Leave closed until dynamic vararg access is lowered without BLOCK_EXPR register-alias risk, or until the expression/register allocator has a proof that parent result registers cannot clobber child values. |
| `C6` | Safety | Recursive inline cycles are blocked | safety | Current raw caller counts: 24 BIF, 0 classlib, 0 Level G recursive-cycle rejects. | Must remain closed as a safety gate. | Keep. Only consider bounded/manual inline hints much later. |
| `C7` | Validity/safety | Value-producing callees need valid return shape; arity must match; vararg required indexes must be provided | safety | Covered by existing positive/negative inline tests and the 10 current BIF argument-binding rejects. | These are language/semantic validity checks, not optimisation opportunities. | Keep. Improve diagnostics only if needed. |

### Plain-English gate guide

The remaining gates can be understood as a small set of ordinary situations:

- The candidate is too large. A big BIF such as a parser, formatter, HTTP
  helper, or JSON helper may be valid to inline, but exporting the whole body
  would create too much metadata and too much duplicated caller code.

  ```rexx
  jsonget: procedure = .string
    /* many branches and helper calls */
  ```

- The helper is private. This is usually fine. The private helper should inline
  into an exposed wrapper in the same source file; it should not be advertised
  as a standalone cross-file inline target unless we deliberately change the
  visibility policy.

  ```rexx
  namespace sample expose public

  private_helper: procedure = .int
    return 1

  public: procedure = .int
    return private_helper()
  ```

- The procedure changes its binding environment with procedure-level `expose`.
  That is not the same as an exposed argument. The inliner must understand the
  environment binding before it can replace the call.

  ```rexx
  state = .int

  next: procedure = .int expose state
    state = state + 1
    return state
  ```

- The body uses assembler aliasing. Instructions such as `link`, `linkattr`,
  `linktoattr`, or `unlink` can make two registers refer to the same storage.
  That cannot be cloned as ordinary variables until the inliner has an explicit
  operand-effect model.

  ```rexx
  assembler link target, source
  ```

- The class method depends on non-scalar object layout. Simple scalar
  getter/setter-style methods are now the intended open path. Methods that
  read or write arrays, stems, object-valued attributes, or binary values still
  need class-layout and copy/reference proof.

  ```rexx
  _items = .string[]

  items: method = .string[]
    return _items
  ```

- The method or exported body needs factory or member-call proof. Classlib
  iterators are the obvious example: the transport can carry plenty of
  metadata, but constructing and returning another object crosses into factory
  selection, `match`, and lifetime semantics.

  ```rexx
  iterator: method = .iterator
    return .arrayListIterator()
  ```

- The receiver is not a direct local variable. Rewriting `items[i].set("x")`
  as a temporary receiver plus copyback would require evaluating the receiver
  exactly once and then writing it back safely. That is a reference/move or
  materialisation design, not a simple inliner tweak.

  ```rexx
  items[i].set("x")
  ```

- The call is in an expression parent that has not been proved. Many expression
  contexts are open, but each new parent shape must prove where the inlined
  value lands and whether any side effects happen in the same order as a real
  call.

  ```rexx
  say prefix() || suffix()
  ```

- The helper uses dynamic vararg access. Fixed-index varargs are test-covered,
  but dynamic indexing needs a safe locator/capture design, especially for
  reference varargs.

  ```rexx
  pick: procedure = .string
    arg which = .int, ... = .string
    return arg(which)
  ```

- The body carries loop/control-flow or numeric-control nodes that the inline
  metadata codec does not yet transport. These are likely incremental codec
  slices, but each one still needs source, RXAS, RXDAS, binary-import, and
  runtime tests.

  ```rexx
  numeric digits 20
  ```

Recommended order for future review tasks:

1. Treat the wider `W2/C4` class-layout/factory/receiver family as a design
   item, not a quick inliner patch. It moves the classlib and Level G needle
   most, but it depends on object layout, factory selection, copyback, and
   reference/move semantics.
2. Review `C1` residual expression contexts by parent shape, opening only
   shapes that can prove a single destination and call-equivalent side-effect
   order.
3. Review `W4` body-size/profitability after semantic gates settle. This may
   improve BIF metadata coverage, but it should not hide correctness work.
4. `W5` procedure-level expose, as a separate environment-binding design.
5. `W6` assembler aliasing effects, tied to the broader
   register/reference alias and operand-effect design.
6. `W7` decimal/numeric-control nodes, as a small codec expansion if it blocks
   a useful BIF.

### Local follow-up: assembler effect register mapping

The local inliner should eventually support a safe subset of assembler-backed
helpers, but only after the register, lifetime, and operand-effect model is
explicit.

The current unsafe classes are:

- instructions that make one register an alias of storage owned by another
  register or object, especially `link`, `linkattr*`, `linktoattr*`, and
  `unlink`
- instructions with hidden state effects whose operands are not ordinary
  formals/locals protected by copy semantics

The issue is register/effect mapping, not assembler text in general. The
clone/remap core must be able to answer:

- what storage the linked destination register refers to after callee symbols
  have been remapped into the caller's inline scope
- whether the target storage outlives the inlined block
- whether an `unlink` must be preserved, moved, or paired with copyback
- whether the alias can observe mutations from both the caller and callee in
  the same order as a real call
- whether the linked register can be represented as an ordinary symbol/locator
  in the current inline machinery
- for stateful instructions, whether the destination is caller-owned,
  callee-local, or an isolated by-value copy, and whether hidden state can leak
  across sibling expressions or repeated helper calls

The string cursor failure mode is broader than `setstrpos` itself. The VM
stores cursor state on the string value, `substring` reads from that cursor,
and helpers such as `substcut` / `dropchar` can mutate string contents. Real
calls get a frame boundary and argument binding semantics; inlining must
preserve that boundary either by proving the affected register is callee-local
or by explicitly materialising/restoring the caller-visible state. The current
proof for ordinary by-value string formals is the same as the normal call
prologue: assembler operands are marked read/write, writable formals are copied,
and `SCOPY` resets the destination cursor so the copied formal starts in normal
string state. Return handling is state-preserving: the VM moves true local
return values and copies potentially aliased return values, but both paths carry
string cursor metadata along with the string contents.

Two future implementation paths are worth keeping open:

- assembler assists: add pure or effect-explicit VM instructions for common
  helper patterns, for example substring-at-position or copy/truncate forms
  that do not mutate the source cursor/content
- post-inline cleanup/restoration: allow the inliner to attach a cleanup node
  to symbols whose eventual register should have hidden state restored or
  normalised by the emitter after register allocation

The cleanup route is only valid for hidden state where restore/normalise is
semantics-preserving. It does not by itself make destructive content mutation
safe; that still needs an isolated copy, copyback semantics, or a pure
assembler assist.

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

- broader cross-file coverage for remaining local-safe bodies
- broader receiver copyback for mutating methods whose receiver is not a direct symbol
- any later reconsideration of dynamic vararg indexing

Recommended order from the April 2026 review:

1. Harden the existing local model with better fail-closed diagnostics and
   tests around the known excluded buckets. This keeps future inliner work
   auditable.
2. Implement one remaining local fail-closed capability gap, chosen by semantic
   risk rather than diagnostic frequency.
3. Extend cross-file inline-body export/import only for cases already proved
   locally. Do not thread special imported-only cases through the current
   selector.

Outside milestone 2 and 3, the remaining exclusions now split cleanly into:

- post-milestone enhancements that may or may not be worth the complexity, such as dynamic vararg indexing
- safety and cost controls that should remain in some form: recursion-cycle blocking and body-size gating
- non-goals or structural policy exclusions: `main`
- language-validity constraints that are not extra inline restrictions: arity mismatches and invalid non-void fallthrough/value-return shapes

## Verification

The local inlining design should be considered established when:

- the compiler rewrites `inline_test1`, `inline_test_call`, `inline_test_expr`, `inline_test_concat_expr`, `inline_test_say_expr`, `inline_test_return_expr`, `inline_test_unary_expr`, `inline_test_compare_expr`, `inline_test_call_arg_expr`, `inline_test_call_like_arg_expr`, `inline_test_nested_call_expr`, `inline_test_ref_opt`, `inline_test_ref_indexed`, `inline_test_ref_stem`, and `inline_test_class_methods` using the supported strategies
- excluded cases such as large procedures, imported callees without compatible
  payloads, and unsupported mutating method expression calls remain uninlined
  under optimisation
- unsupported by-reference/short-circuit combinations such as those in `inline_test_ref_negative` remain uninlined under optimisation
- stateful string assembler helpers such as `inline_test_stateful_assembler`
  inline only when call-prologue-equivalent copy isolation is visible in the
  generated code
- the resulting AST is structurally valid under `-dp`
- optimised codegen succeeds
- positive and negative compiler tests lock down the selector behaviour

## Non-Goals For The Current Design Boundary

- arbitrary imported callees without compatible inline-body metadata
- imported member bodies from binary-only class metadata
- mutating methods in expression position until receiver copyback can be emitted before `LEAVE_WITH`
- aggressive pruning of fully inlined procedures
- claiming fully generic scope/symbol cloning before it is proven
