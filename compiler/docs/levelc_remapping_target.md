# Level C Remapping And Tree Surgery Target

Status: working design note. This document defines the intended remapping
target and the first tracer-bullet path. It does not introduce approved source
syntax or change compiler behaviour by itself.

## Purpose

Level C lowering needs a controlled way to transform a parsed source tree into
the canonical AST shape already accepted by validation, optimisation, and
emission. The same mechanism is also useful for inlining and other optimiser
rewrites. The common problem is tree surgery:

- match an existing AST shape;
- prove the rewrite is legal at this exact site;
- build a replacement tree, often with new scopes, symbols, temporaries, and
  helper calls;
- preserve evaluation order, aliasing semantics, source anchors, diagnostics,
  and debug metadata;
- leave downstream compiler phases with ordinary validated AST nodes.

The recommended tracer bullet is to express the existing inlining rewrite
rules in the new remapping model before enabling Level C compilation. Inlining
is already production compiler tree surgery with broad tests. If the remapping
model cannot describe the current inliner, it is not strong enough for Level C.

## Current Situation

Level C currently has a Classic REXX scanner/glue/grammar path used for syntax
highlighting and parser diagnostics. Normal `rxc` compilation of
`OPTIONS LEVELC` stops before code generation. The Level C parser builds AST
nodes for Classic-only constructs such as `LEVELC_*` instructions and also
uses canonical nodes where the Classic and Level B forms are already close.

The Level C runtime foundation exists in `lib/rxfnsc`:

- `RexxValue` represents Classic scalar values with lazy string, binary,
  integer, float, and decimal views.
- `RexxStem` represents Classic stem bindings.
- `RexxVariablePool` maps normalized names to scalar/stem bindings and
  models dropped state and exposure/aliasing.
- `RexxClassicBifs` starts the shared Classic-compatible BIF surface.

The key Level C lowering rule from
`compiler/docs/levelc_working_architecture.md` still stands: ordinary Classic
variables are variable-pool entries, not Level B typed locals, unless a later
optimisation proves that a local replacement preserves Classic semantics.

The current generic rewrite helper in `rxcp_ast_rewrite.[ch]` can build and
reuse nodes while keeping source anchors. It is useful, but it is not yet a
remapping system. It does not model rule guards, scopes, symbol remaps,
effect order, sidecar state, multi-node statement replacement, or fixed-point
strategy.

The inliner in `rxcp_inline.c` already contains those stronger concepts in
hand-written form:

- structural callee eligibility gates;
- call-site capability buckets;
- scope and symbol cloning maps;
- argument capture and binding;
- by-reference and vararg remapping;
- `BLOCK_EXPR` expression rewrites with `LEAVE_WITH`;
- statement-block rewrites for whole statements;
- method receiver binding and copyback;
- source/provenance/debug preservation;
- cross-file inline-body import/export through `META_INLINE`.

That makes inlining the best first remapping specimen.

## Target Shape

The remapping target is not source text. It is the canonical compiler work AST:

- node types already understood by normal validation, optimiser passes, and
  emitter;
- normal `Scope` and `Symbol` ownership;
- source anchors attached to generated nodes as exact, inherited, synthetic,
  composite, or stripped provenance as appropriate;
- compiler-added nodes marked as compiler generated where that matters for
  diagnostics and pruning;
- no source-dialect-only nodes remaining after the pass boundary that claims
  to lower that dialect.

For Level C, the conceptual target can be described as a Level B-like program
that imports the Classic runtime and manipulates a hidden variable pool. The
compiler should build the AST directly; the source spelling below is only a
human-readable contract.

```rexx
options levelb
import rexxvalue
import rexxpool

__lc_pool = .RexxVariablePool()

call __lc_pool.setValue("A", .RexxValue("1"))
__lc_tmp = __lc_pool.value("A").add(.RexxValue("2"))
call __lc_pool.setValue("B", __lc_tmp)
say __lc_pool.value("B").asString()
```

Important consequences:

- Classic reads lower to pool reads that yield `RexxValue` objects.
- Classic writes lower to pool setter calls, not mutation of an arbitrary
  returned object.
- Classic operators lower to `RexxValue` helper methods/functions until a
  later optimiser proves a narrower typed path.
- Classic stem, compound, indirect, `DROP`, `PROCEDURE EXPOSE`, `PARSE`,
  `ADDRESS`, and `INTERPRET` semantics remain pool/sidecar concerns.
- The target is still ordinary AST. It should be optimisable and inlineable by
  the same machinery as Level B helper code.

## Rule Model

A remapping rule should be a structured compiler object, even if the first
implementation is a C API rather than an external DSL. An external rule syntax
can come later once the semantics are proven.

Conceptual shape:

```text
rule <id>
phase <phase-name>
priority <number>
root <node-type-or-family>

match
  <tree pattern with named captures>

bind
  <derived values such as symbols, scopes, actual lists, source anchors>

guard
  <fail-closed predicates with named rejection reasons>

build
  <replacement AST, scopes, symbols, sidecar updates, metadata>

effects
  reads <abstract state>
  writes <abstract state>
  preserves-evaluation-order <proof note>

provenance
  anchor <source node>
  generated <synthetic|inherited|composite>
  diagnostic-owner <node>

legality
  target-dialect <canonical-levelb|inline-normalized|levelc-lowered>
  no-surviving <node families>

tests
  <focused tests that prove this rule>
```

The implementation form should probably start as C descriptors with callback
hooks, for example:

```c
typedef struct RemapRule {
    const char *id;
    RemapPhase phase;
    int priority;
    int (*match)(RemapContext *, ASTNode *, RemapMatch *);
    int (*guard)(RemapContext *, const RemapMatch *, RemapReject *);
    ASTNode *(*build)(RemapContext *, const RemapMatch *);
} RemapRule;
```

The rule language must make the hard parts explicit:

- replacement may be one node or a sibling list;
- a rule may replace an expression child or a whole enclosing statement;
- generated scopes must have clear parents and defining nodes;
- cloned symbols must be mapped, not accidentally shared;
- reused nodes must be detached and symbol-table references decoupled;
- source anchors must be chosen deliberately;
- rule failures need stable rejection reasons for debug output;
- rules must advertise which target shapes they leave behind;
- traversal strategy and fixed-point behaviour must be deterministic.

## Strategy Model

Remapping should use explicit strategy rather than ad hoc recursive mutation:

1. Run source-dialect validation that must happen before lowering.
2. Run one named remapping phase.
3. Validate the phase target shape.
4. Repeat named optimisation/remapping phases to a bounded fixed point where
   appropriate.
5. Emit only after the AST is canonical for the emitter.

Rules should be fail-closed. A missed optimisation is acceptable. A rewrite
that changes Classic semantics, value liveness, aliasing, or receiver state is
not.

For Level C, the likely phase split is:

1. `levelc.pre-lower.validate`: Classic-only structural diagnostics.
2. `levelc.pool-lower`: variables, literals, basic expressions, `SAY`, simple
   assignment, and hidden pool setup.
3. `levelc.control-lower`: `IF`, `SELECT`, simple `DO`, `LEAVE`, `ITERATE`.
4. `levelc.runtime-lower`: BIFs, `CALL`, `PARSE`, `ADDRESS`, `INTERPRET`,
   `PROCEDURE`, and exposure.
5. normal validation, optimiser, inliner, and emitter.

This split is a planning device, not an approved implementation boundary.

## Inlining As The First Tracer Bullet

The current inliner proves what the remapping language must express.
`compiler/docs/inlining_design.md` remains the detailed inlining design; this
section recasts it as remapping rules.

### Existing Inliner Phases

The current pass effectively does this:

1. Identify structurally inlineable procedures, methods, and factories.
2. Walk call sites after children have been visited.
3. Classify each call site by parent context.
4. Pick a narrow rewrite bucket.
5. Build isolated inline scopes and symbol maps.
6. Bind actuals into cloned formals.
7. Clone callee instructions.
8. Rewrite returns for the chosen bucket.
9. Replace the call site or enclosing statement.
10. Repeat in optimiser fixed-point passes.

The structural gates include body availability, arity, return shape, node
count cutoff, recursion blocking, unsupported assembler alias/effect checks,
unsupported reference-bearing shapes, procedure-level `EXPOSE`, unsupported
vararg access, and unportable class attribute shapes.

The call-site gates include assignment shape, statement-call shape, direct
single-value expression consumers, eager operator safety, short-circuit
contexts, receiver-position exclusions, imported method receiver restrictions,
and mutating-method receiver copyback restrictions.

### Rule Families To Capture

#### `inline.rhs-eager-operator.capture-left`

When the RHS of an eager binary operator is an inlineable call and the left
operand is not already a constant or plain stable value, rewrite the whole
operator to a `BLOCK_EXPR`:

```text
left <op> call(...)
```

becomes conceptually:

```text
BLOCK_EXPR {
  __inline_lhs = left
  LEAVE_WITH __inline_lhs <op> call(...)
}
```

The next fixed-point pass can then inline the call in a safe expression
context. This is a compact first implementation candidate because it exercises
expression replacement, temp creation, source anchors, `BLOCK_EXPR`, and
`LEAVE_WITH` without cloning a callee body.

#### `inline.call.statement`

For:

```rexx
call func(a)
```

replace the whole `CALL` statement with a compiler-generated `INSTRUCTIONS`
block. The block binds actuals, clones the callee body, and handles `RETURN`
according to whether the caller ignores the value. Return expressions must
still be evaluated because they may have side effects.

#### `inline.assignment.whole-rhs`

For:

```rexx
x = func(a)
```

replace the whole assignment when the RHS is the call and the LHS shape is
supported. Binding and cloned body instructions are inserted before the return
assignment. A final `RETURN expr` becomes conceptually:

```rexx
x = expr
```

More complex assignment LHS or aggregate return cases may fall back to a
`BLOCK_EXPR` result path when that preserves copy semantics.

#### `inline.expression.block`

For direct value-consuming expression positions:

```rexx
say func(a)
return func(a)
if func(a) then ...
outer(func(a))
```

replace the call expression with:

```text
BLOCK_EXPR {
  <argument binding>
  <cloned body>
  LEAVE_WITH <return expression>
}
```

The rule must prove that the parent consumes the block expression as one value
and that sibling liveness/evaluation order is preserved.

#### `inline.bind.actuals`

Formal binding is itself a family of subrules:

- ordinary by-value actuals become assignments into cloned formal symbols;
- omitted optional actuals read from the formal default expression;
- writable aggregate/binary/object values may require isolated copy temps;
- by-reference actuals map formal references to caller-visible locator shapes;
- nontrivial reference locator children are captured once into temps;
- by-value varargs are captured and exposed through a generated vararg array;
- `.ref`/`EXPOSE` varargs map constant-index accesses back to captured
  aliasable actuals;
- method receivers bind once into the cloned `§this`;
- factory bodies initialise cloned `§factory` using `setattrs` and
  `setobjtype`.

The remapping API therefore needs reusable clone/bind services, not only
pattern-to-template replacement.

#### `inline.return.rewrite`

Return handling depends on the bucket:

- statement assignment: `RETURN expr` becomes assignment to the caller target;
- ignored call value: `RETURN expr` is evaluated into a sink;
- expression block: `RETURN expr` becomes `LEAVE_WITH expr`;
- void fallthrough in statement-call sites may synthesize a dummy value only
  inside a block expression that needs one;
- mutating method returns may need receiver copyback before `LEAVE_WITH`.

This is a good example of why the rule language needs context and effect
contracts, not just tree patterns.

#### `inline.receiver.copyback`

A mutating method inline may write class attributes through the cloned
receiver. If the original receiver is a direct non-attribute symbol, the
inliner can copy the updated `§this` value back after the body. Other receiver
shapes remain fail-closed until materialisation and copyback are explicitly
modelled.

#### `inline.imported-template`

Cross-file inlining imports compiler-owned `META_INLINE` payloads as read-only
templates and feeds them through the same local clone/bind/rewrite machinery.
The remapping system should preserve this principle: serialized AST payloads
are transport for canonical compiler trees, not source text and not raw C
struct memory.

## What The Inlining Tracer Proves

Converting the inliner first proves the remapping substrate can handle:

- ordinary expression and statement replacement;
- generated sibling lists;
- generated local scopes;
- source and diagnostic anchors;
- symbol and scope remapping;
- temporary symbols;
- cloned body templates;
- reusable subrules for argument binding;
- fail-closed guards with debug-visible reasons;
- fixed-point interaction with existing optimiser passes;
- imported canonical AST payloads;
- negative tests that prove unsupported shapes stay untouched.

That is almost the same machinery Level C needs, except Level C adds a larger
source dialect and Classic runtime semantics.

## Level C Rule Sketches

These are conceptual examples, not approved syntax.

### Pool Setup

```text
rule levelc.routine.pool-setup
phase levelc.pool-lower
match routine-scope without LEVELC_POOL sidecar
build
  hidden local __lc_pool : RexxVariablePool
  prologue assignment __lc_pool = .RexxVariablePool()
effects writes variable-pool-sidecar, instruction-list
```

### Scalar Literal

```text
rule levelc.literal.string
phase levelc.pool-lower
match LEVELC_STRING or Classic literal token
build .RexxValue(<source text>)
effects none
```

### Variable Read

```text
rule levelc.variable.read
phase levelc.pool-lower
match Classic direct variable reference $name
guard not assignment target
build __lc_pool.value(normalizeName($name))
effects reads variable-pool
```

### Assignment

```text
rule levelc.assignment.scalar
phase levelc.pool-lower
match ASSIGN(Classic target $name, $expr)
build call __lc_pool.setValue(normalizeName($name), lowerValue($expr))
effects writes variable-pool
```

### Binary Add

```text
rule levelc.expr.add
phase levelc.pool-lower
match OP_ADD($left, $right)
build lowerValue($left).add(lowerValue($right))
effects preserves left-to-right operand order
```

### SAY

```text
rule levelc.say
phase levelc.pool-lower
match SAY($expr)
build SAY(lowerValue($expr).asString())
effects output
```

These sketches deliberately target runtime helper calls first. Later
optimisation may inline helpers, fold constants, or replace proven local values
with typed Level B locals, but that must be a separate proof after Classic
semantics are represented correctly.

## Optimisation And Inlining Scope

Inlining and optimisation are in scope from the beginning, but they should be
layered:

1. Lower Level C to correct canonical AST using the runtime helper surface.
2. Run ordinary validation to prove the generated tree is a real Level B-style
   program.
3. Let existing inlining inline runtime helper methods where safe.
4. Add targeted remapping optimisations only when the guards can prove Classic
   semantics are preserved.

Examples of future optimisation rules:

- constant `RexxValue("1").add(RexxValue("2"))` can fold to `RexxValue("3")`
  when the active numeric settings are known;
- repeated direct reads of an unmodified Classic variable may share a temp
  inside one expression;
- a private helper call can inline through the normal inlining rules;
- a Classic variable can become a typed Level B local only inside a region
  where no `DROP`, exposure, `ADDRESS`, `PARSE`, `INTERPRET`, stem access, or
  derived-name operation can observe the pool identity.

The optimiser should not erase the variable-pool model just because a local
example looks scalar. Classic observability is the guard.

## Proposed Implementation Path

1. Keep this document and `compiler/docs/inlining_design.md` as the design
   pair: the inlining document remains the exhaustive current behaviour; this
   document defines the general remapping vocabulary.
2. Add a small remapping descriptor layer that can wrap existing inliner
   functions without changing behaviour. The first win is traceability:
   stable rule ids, guard ids, and phase names.
3. Convert `inline.rhs-eager-operator.capture-left` first. It is compact and
   exercises `BLOCK_EXPR`, temp creation, source anchors, and expression
   replacement.
4. Convert one statement-position inline bucket next, preferably the simplest
   final-return whole-RHS assignment slice.
5. Convert the shared clone/bind/return services under the remapping API,
   keeping existing tests green after each step.
6. Once inlining is represented by the remapping substrate, add the smallest
   Level C lowering tracer: pool setup, scalar literal, variable read/write,
   `SAY`, and one binary operator.
7. Only then open broader Level C control flow and BIF lowering.

## First Level C Lowering Slice

Status: first executable tracer slice implemented. This is intentionally small
and rollback-friendly. It proves that Level C can enter the normal compiler
pipeline through remapping without pretending that Classic REXX compilation is
generally supported.

### User-Visible Slice

Accept only top-level programs whose non-`OPTIONS` clauses are built from:

- direct scalar assignments: `name = expression`;
- direct scalar variable reads: `name`;
- string and integer/numeric literal tokens, materialised as Classic
  `RexxValue` objects from their source text;
- parenthesised expressions already collapsed by the Level C grammar;
- binary `+`, lowered through `RexxValue.add()`;
- `SAY expression`, lowered through `RexxValue.asString()`.

Representative first fixture:

```rexx
options levelc
a = 1
b = a + 2
say b
```

Expected output:

```text
3
```

The existing `levelc_compile_unsupported` test should remain, but its fixture
must move to a construct outside this slice, such as `PARSE ARG`, `IF`, `DO`,
`ADDRESS`, `DROP`, a label, or an implicit command. The first implementation
must not remove the unsupported diagnostic as the default for unsupported
Level C programs.

### Explicit Non-Goals

Do not include these in slice 1:

- procedures, labels, `CALL`, `RETURN`, `SIGNAL`, or `EXIT`;
- `IF`, `SELECT`, `DO`, `LEAVE`, `ITERATE`, or any control-flow lowering;
- `PARSE`, `ARG`, `PULL`, `PUSH`, `QUEUE`, `ADDRESS`, `INTERPRET`, `TRACE`,
  `NUMERIC`, or `DROP`;
- implicit command clauses;
- BIF calls, function calls, method calls authored by the user, or host
  variable anchors;
- stems, compound variables, indirect variables, or exposure/aliasing;
- optimiser proofs that replace the variable pool with Level B locals.

Those exclusions are not philosophical limits. They keep the first slice small
enough that every accepted tree shape can be audited and tested.

### Conceptual Target

The compiler should build canonical AST directly. The following Level B-like
text is only the human-readable contract:

```rexx
options levelb
import rexxvalue
import rexxpool

__lc_pool = .RexxVariablePool()
call __lc_pool.setValue("A", .RexxValue("1"))
call __lc_pool.setValue("B", __lc_pool.value("A").add(.RexxValue("2")))
say __lc_pool.value("B").asString()
```

Important target rules:

- Classic variable names are normalized once, using the Level C symbol rules
  already used by the parser and diagnostics.
- Reads always go through the visible `RexxVariablePool`.
- Writes always call the pool setter; they do not mutate a returned value
  object in place.
- Literal lowering should preserve the Classic source value through
  `RexxValue` construction rather than prematurely choosing a Level B type.
- The generated tree must be canonical enough for normal validation,
  optimisation, assembly, linking, and VM execution.

### Rule Table

| Rule id | Selector | Guard | Build |
| --- | --- | --- | --- |
| `levelc.slice1.program.accept` | `REXX_UNIVERSE > PROGRAM_FILE > INSTRUCTIONS` | Every child is `REXX_OPTIONS`, supported `ASSIGN`, or supported `SAY`; no `ERROR`/`WARNING` severity that should block compile | Establish a Level C lowering context and hidden pool symbol |
| `levelc.routine.pool-setup` | accepted top-level instruction list | No existing hidden pool sidecar | Add runtime imports and a generated pool initialization before lowered user clauses |
| `levelc.literal.value` | `STRING` or `INTEGER` in value position | Token value is valid under existing Level C lexical diagnostics | Build `.RexxValue(<classic-source-text>)` |
| `levelc.variable.read` | `VAR_SYMBOL` in expression position | Direct scalar name only; not a target, not stem/compound/indirect | Build `__lc_pool.value(<normalized-name>)` |
| `levelc.assignment.scalar` | `ASSIGN(VAR_TARGET, expr)` | Direct scalar target; RHS recursively supported | Build statement call `__lc_pool.setValue(<normalized-name>, lower(expr))` |
| `levelc.operator.add` | `OP_ADD(left, right)` | Both operands recursively supported | Build `lower(left).add(lower(right))`, preserving left-to-right evaluation |
| `levelc.say.value` | `SAY(expr)` | Expression recursively supported | Build canonical `SAY(lower(expr).asString())` |

The first implementation can express these as C descriptors and helper
functions, following the inline selector-table style. The selector array does
not need to become an external DSL for this slice.

### New Shared Builder Commands Added

The inlining tracer has already extracted generated scopes, temps, anchors,
assignments, assembler instructions, capture-once patterns, and safe
replacement helpers. Slice 1 adds these neutral remap-builder commands:

- create an import/runtime dependency node;
- build a factory-style object construction expression such as
  `.RexxVariablePool()` or `.RexxValue(text)`;
- build a method-call expression with an explicit receiver and arguments;
- build a statement call when the result is ignored, for example
  `pool.setValue(name, value)`;
- build a member-call statement directly when a rewrite wants the side effect
  rather than the return value;
- build a generated named-target assignment, for example
  `temp = materialise(expr)`;
- build a generated indexed-target assignment, for example
  `frame[1] = value`;
- build an anchored instruction-list builder and append its children into the
  real instruction stream;
- build generated name strings from a prefix plus a source-node id, index, or
  source name;
- build generated import/runtime dependency nodes that own their diagnostics
  as compiler-generated code;
- build literal and `NOVAL` helper nodes used by those shapes.

The procedure/expose slice adds more generated-tree building blocks in the
same spirit. They are deliberately named as operations that could later become
DSL commands:

- build unary keyword expressions such as `reference value` and
  `dereference value`;
- build a generated local procedure/function call with explicit argument
  materialisation;
- build canonical procedure headers, `ARGS` blocks, `ARG` nodes, void return
  types, and reference types for generated helper routines;
- begin an argument frame and append value/provided-mask slots while preserving
  caller-controlled expression evaluation order;
- materialise parent/child `RexxVariablePool` scopes and direct scalar
  `exposeValue` aliases.

Classic variable-pool policy remains in the Level C lowerer:

- create the hidden pool variable name used by the generated top-level routine;
- normalize a Level C symbol token to the runtime pool key while preserving the
  source anchor used for diagnostics;
- reject unsupported residual Level C nodes with a stable reason.

This split keeps generic AST construction under the remap builder while
leaving Classic semantics in the Level C policy layer.

### Implementation Order

0. Keep a checked-in Level B baseline program that mimics the exact target
   shape before enabling the Level C source that should lower to it. The
   baseline is
   `compiler/tests/rexx_src/levelc_slice1_target_shape.crexx`, and its CTest
   smoke test compiles, assembles, and runs the runtime-backed target program.
   Treat this as step zero for each later slice: write the target-shape
   Level B-like program first, prove it runs, then make Level C materialise the
   same conceptual shape.
   When a future canonical target shape is not reachable through authored
   Level B syntax, keep the target contract anyway as pseudo-code and make the
   generated-tree debug probe the executable shape test.
1. Add a Level C lowering entry point, `rxcp_levelc_lower.[ch]`, that runs
   after `rexcpars(context)` and before normal validation/emission.
2. Change the `LEVELC` branch in `rxcpmain.c` to parse with `rexcpars()`,
   then attempt the slice-1 lowering. If the accept rule rejects the tree,
   emit the existing unsupported diagnostic and fail exactly as today.
3. Add the missing remap-builder commands for imports, runtime construction,
   method calls, and statement calls. Keep hidden pool naming in the Level C
   lowerer until another frontend needs the same policy.
4. Implement the accept/reject walker first. It should return one stable
   rejection reason per unsupported node family.
5. Implement expression lowering recursively for literals, direct variables,
   and `OP_ADD`.
6. Implement statement lowering for scalar assignment and `SAY expression`.
7. Mark generated nodes with deliberate source anchors: generated runtime
   setup should be synthetic from the file/options anchor, and lowered user
   clauses should inherit the source clause anchor.
8. Run normal validation and optimiser passes on the lowered canonical tree.
9. Keep all other Level C compile inputs fail-closed until their rule family is
   added deliberately.

### Acceptance Checks

Minimum QA for this slice:

- the Level B target-shape baseline remains green, proving the runtime helper
  program that Level C intends to materialise is itself valid;
- existing Level C DSLSH/syntax-highlighting tests remain green;
- `levelc_compile_unsupported` still passes using an unsupported fixture;
- a new `levelc_slice1_pool_say` compile/run fixture emits `3`;
- a generated-tree debug probe shows no residual `LEVELC_*` nodes for the
  accepted fixture;
- the Level C accepted fixture can pass through `rxc`, `rxas`, `rxlink` where
  applicable, and `rxvm`;
- the same fixture output is compared with Regina REXX as a local development
  sanity check where Regina is available, but CTest should not depend on
  Regina being installed;
- `RXCP_INLINE_RULE_SUMMARY=1` remains unaffected, proving the inlining rule
  catalog and Level C lowering catalog do not collide.

This slice is useful even before optimisation. It proves the end-to-end path:
Level C parser AST to remap rule selection, runtime-backed canonical AST,
normal compiler validation, normal optimiser/emitter, and executable VM output.

## Second Level C Lowering Slice: PROCEDURE EXPOSE

Status: implemented as the first variable-pool scope and aliasing tracer. This
is the minimum slice that proves Classic `PROCEDURE EXPOSE` can be represented
as generated canonical code rather than by mutating Level C nodes in place.

### User-Visible Slice

Accepted source shape:

```rexx
options levelc

a = 1
call change
say a
exit

change:
procedure expose a
a = a + 2
return
```

Expected output:

```text
3
```

Accepted restrictions:

- local routines must be a `LABEL` immediately followed by `PROCEDURE`;
- the top-level main segment must `EXIT` before the first local routine so
  fall-through into generated helper procedures is impossible;
- direct local `CALL` targets are supported only when they name one of the
  planned local routines, and no call arguments are supported yet;
- plain `PROCEDURE` and argument-bearing procedures remain outside this slice;
- `PROCEDURE EXPOSE` supports only direct scalar names; stems, compound names
  and indirect names remain outside this slice;
- the procedure body may contain the same scalar assignment/read/add/SAY forms
  as slice 1 and must end with a bare `RETURN`.

### Conceptual Target

The checked-in baseline program is
`compiler/tests/rexx_src/levelc_slice2_procedure_expose_target_shape.crexx`.
Its human-readable target contract is:

```rexx
options levelb comments_dash numeric_classic
import rexxvalue
import rexxpool

__rxcp_levelc_pool = .RexxVariablePool()
call __rxcp_levelc_pool.setValue("A", .RexxValue("1"))
call __rxcp_levelc_proc_CHANGE(reference __rxcp_levelc_pool)
say __rxcp_levelc_pool.value("A").asString()
return

__rxcp_levelc_proc_CHANGE: procedure
  arg __rxcp_levelc_parent_pool_ref = reference .RexxVariablePool
  __rxcp_levelc_parent_pool = dereference __rxcp_levelc_parent_pool_ref
  __rxcp_levelc_pool = .RexxVariablePool()
  call __rxcp_levelc_pool.exposeValue("A", reference __rxcp_levelc_parent_pool, "A")
  call __rxcp_levelc_pool.setValue("A", __rxcp_levelc_pool.value("A").add(.RexxValue("2")))
  return
```

Important target rules:

- the caller passes the active variable pool by reference;
- the generated procedure dereferences that parent pool, creates its own local
  pool, and uses `exposeValue` to alias selected names back to the parent;
- procedure body reads and writes continue to use the active generated
  `__rxcp_levelc_pool`, so later body rules do not need to know whether a name
  is local or exposed;
- the generated helper procedure is canonical Level B shape after lowering,
  even though the compiler currently builds that tree directly rather than
  round-tripping through this text.

### Rule Table

| Rule id | Selector | Guard | Build |
| --- | --- | --- | --- |
| `levelc.procedure-plan.accept` | top-level `LABEL`, following `LEVELC_PROCEDURE`, and body segment | `PROCEDURE` has only supported direct scalar `EXPOSE` arguments; body ends in bare `RETURN`; duplicate labels rejected | Record a procedure slice with label, procedure node, body range, and normalized name |
| `levelc.main.call-local` | `CALL target` in the main segment | target normalizes to a recorded procedure and has no arguments | Build ignored-result call to generated helper, passing `reference __rxcp_levelc_pool` |
| `levelc.main.exit-before-routines` | main segment before first local routine | at least one bare `EXIT` exists when procedures are present | Lower `EXIT` to canonical `RETURN` so generated helper routines are not executed by fall-through |
| `levelc.procedure.shell` | recorded procedure slice | generated name is unique and parent pool arg is available | Build canonical procedure header, typed `ARGS`, parent-pool dereference, and child-pool setup |
| `levelc.procedure.expose.scalar` | each direct scalar expose target | name is not a stem/compound/indirect form | Build `__rxcp_levelc_pool.exposeValue(name, reference __rxcp_levelc_parent_pool, name)` |
| `levelc.procedure.body` | supported body statements | same expression/statement guards as slice 1, plus final bare `RETURN` | Lower body statements against the generated child pool |

### Acceptance Checks

Minimum QA for this slice:

- the Level B target-shape fixture
  `levelc_slice2_procedure_expose_target_shape` emits `3`;
- the Level C source fixture `levelc_slice2_procedure_expose` emits `3`;
- `levelc_slice2_procedure_expose_unsupported` proves a near-miss procedure
  layout without the main `EXIT` still fails closed;
- `levelc_slice2_plain_procedure_unsupported` proves plain `PROCEDURE` is not
  accidentally admitted by the expose-specific guard;
- `levelc_slice2_procedure_expose_tree_shape` inspects the `-d1`
  `STAGE_LEVELC_LOWERED` tree and checks for `PROCEDURE`, `ARGS`,
  `OP_REFERENCE`, `OP_DEREFERENCE`, `RexxVariablePool`, `exposeValue`,
  `setValue`, `value`, `add`, and `asString`, while rejecting residual
  `LEVELC_*` nodes and the temporary builder-only `INSTRUCTIONS` wrapper.

## Remapper Debugging Contract

Tree-shape debugging is a first-class part of each slice, not an optional
manual check. The preferred sequence is:

1. Write the conceptual target as Level B-like code when that shape is
   reachable and executable.
2. If the canonical tree is not reachable through authored Level B syntax,
   document the pseudo-target anyway and test the generated tree directly.
3. Keep builder functions small and named by semantic action, so a later DSL
   can map rules onto commands such as `materialise-parent-pool`,
   `capture-locator-once`, `writeback-through-captured-locator`, or
   `expose-scalar-alias`.
4. Add a lowered-tree debug probe for every new accepted family. Runtime output
   proves semantics; debug shape proves the remapper is creating the intended
   tree.
5. Keep unsupported near-misses as negative tests so the accepted language does
   not widen accidentally.

This means the target-shape fixture is a scaffold, not a constraint. The real
contract is the canonical AST that the remapper builds and the downstream
compiler accepts.

## Current Tracer Status

As of the inline rule catalog and service-runner split, the four current
call-site inline buckets are represented as
selector-backed rules in `compiler/rxcp_inline_rules.c`. Structural eligibility,
actual binding, callee-body cloning, return rewriting, and receiver copyback
are now explicit service boundaries executed through `rxcp_remap_run_service`.
Their internals have also been split into private implementation fragments for
binding, cloning, rewriting, analysis, and payload import/export. Those
fragments are still included by `rxcp_inline.c` rather than compiled
independently, because the static helper dependency graph is intentionally being
thinned in stages.

The first Level C-neutral builder/materialisation layer now lives in
`compiler/rxcp_remap_build.c` and `compiler/rxcp_remap_build.h`. It provides
shared building blocks for generated scopes, numeric-context copying, source
anchors, generated blocks, temp symbols, symbol refs/targets, integer constants,
string constants, instruction-list builders, builder-child append operations,
generated name strings, generated imports, factory/function/member calls,
member-call statements, assembler instructions, register-copy instructions,
assignments, generated named-target and indexed-target assignments, generated
procedure headers, generated `ARGS`/`ARG` nodes, generated void/reference type
nodes, argument-frame begin/slot materialisation, sink targets,
assignment/`LEAVE WITH` append operations,
replacement-with-symbol-disconnect, capture-once rewrites, and
captured-locator proof/materialisation patterns. The capture primitives now
separate reusable assignment construction and captured-value references from
the higher-level append-and-readback convenience calls. The named locator
patterns are
`rxcp_remap_capture_locator_once()`,
`rxcp_remap_materialise_selected_value()`, and
`rxcp_remap_writeback_through_captured_locator()`. They let a rule prove that a
variable-like locator's child expressions have been evaluated once, rebuild a
read or write view of the selected value from those captures, and write a
changed value back through the same locator. The inliner is the first client,
but the API is intentionally under the remap layer so Level C lowering, other
Rexx front ends, and future optimisation rewrites can use the same mechanics
without depending on inline-specific policy.

The active Level C lowering tracer now lives in `compiler/rxcp_levelc_lower.c`
and `compiler/rxcp_levelc_lower.h`. The normal compiler `LEVELC` branch parses
with `rexcpars(context)`, prepares the Level C source tree and diagnostics,
then attempts the fail-closed remap. Accepted programs are rewritten to a
Level B-shaped work tree that imports `rexxvalue` and `rexxpool`, creates a
hidden `RexxVariablePool`, lowers scalar assignments to `setValue`, lowers
scalar reads to `value`, lowers literals to `RexxValue`, lowers proven Classic
expression operators through named `RexxValue` helpers, lowers `SAY` through
`asString`, and lowers the first local `CALL` plus `PROCEDURE EXPOSE` shape by
passing a parent pool reference into a generated helper procedure. Arithmetic,
concatenation, comparison, prefix, and XOR expressions are ordinary method-call
materialisations. Short-circuit `&` and `|` use a generated outer-scope result
anchor plus an `IF` over one-shot generated `DO 1` blocks so the right-hand
expression remains lazy. The BIF slices import `rexxclassicbifs`, parse
function-call expressions, lower proven `LENGTH(value)` calls to the direct
`rexxclassicbif_length(RexxValue)` helper, and lower proven `SUBSTR` calls
through a generated dispatcher frame with `.RexxValue[]` slots, `.int[]`
provided flags, `RexxBifCallContext`, `setArguments`, `setCallerPool`, and
`rexxclassicbif_call(reference ctx)`.

The stem slice lowers simple compound variables through the same active pool:
`stem.1` and `stem.i` become `stemValue("STEM.", tail)` reads and
`setStemValue("STEM.", tail, value)` writes. Dynamic assignment tails are
materialised into generated string temporaries before the right-hand expression
is lowered, preserving the locator-once rewrite pattern needed by later
optimisation. `PROCEDURE EXPOSE stem.` lowers to `exposeStem`, so a generated
procedure aliases the caller's stem object rather than copying a scalar slot.
Bare stem default-value assignment/read and exposing a single compound variable
remain outside this first slice.

The procedure slice now admits plain internal `PROCEDURE` when the body remains
inside the proven tree shape. It records fixed direct-scalar `ARG` arity,
generates `.RexxValue` hidden actual parameters, binds them into the generated
procedure pool, and uses `.RexxValue` return types only for routines with
`RETURN expr`. Statement `CALL p a,b` is supported for direct variable/integer
tails; expression-position local calls such as `x = f(a,b)` are supported when
the callee has a value return.
Rejected programs keep the existing unsupported Level C compilation diagnostic.

The BIF proof layer is now `RexxValue`-native. `RexxBifCallContext` carries
`RexxValue` argument slots plus a separate provided-argument mask, and
`rexxclassicbif_call()` returns a `RexxValue` while errors stay on the context.
This keeps fixed-arity direct helpers optimisable without losing the dynamic
dispatcher path used by RexxScript. Optional-arity BIFs should stay on the
dispatcher path until a direct helper is worth specialising. The Level C lowerer
uses neutral argument-frame commands for the generated value array and
provided-mask array: `rxcp_remap_begin_argument_frame()` creates the frame
storage, and `rxcp_remap_append_argument_frame_slot()` appends each value slot
and provided flag after the caller has lowered that argument expression. This
keeps expression-evaluation policy in the caller while moving the frame tree
shape into the remap builder. Source forms with omitted argument positions, for
example `xxx(,a,,b)`, are admitted through a call-list-specific parser shape.
The list materialises `NOVAL` placeholders only inside function arguments,
keeping ordinary comma recovery outside primary expressions and preserving a
conflict-free Lemon grammar.

The expression slice adds shared branch builders to the neutral remap layer:
`rxcp_remap_create_if_statement()` and `rxcp_remap_create_do_block()`. The block
builder deliberately creates `DO -> REPEAT(FOR 1), INSTRUCTIONS`; this encodes a
one-shot generated block and avoids relying on parser-only normalization for
plain grouped `DO`.

## Mapper Consolidation Review 2026-06-24

The current selector tables are not the main source of duplication. They are
already compact and useful for explaining what a rule selects. The repeated
work is in materialisation: creating anchored temporary instruction lists,
moving generated prelude statements into the real stream, assigning generated
named values, and turning receiver/method/argument triples into side-effecting
statements.

Those mechanics are now shared as remap-builder commands:
`rxcp_remap_create_instruction_builder()`,
`rxcp_remap_append_builder_children()`,
`rxcp_remap_create_named_assignment()`, and
`rxcp_remap_create_member_call_statement()`. The Level C mapper uses them for
short-circuit preludes, BIF dispatcher setup, compound-tail temporaries, pool
writes, procedure exposes, ARG binding, and program/statement prelude shells.
These names are intentionally command-like so a later table-driven or DSL-like
mapper can call the same operations without learning the raw AST shape.

The final consolidation sweep moved the remaining obvious framework mechanics
out of the Level C mapper:

- `rxcp_remap_begin_argument_frame()` and
  `rxcp_remap_append_argument_frame_slot()` own value-array/provided-mask
  frame shape;
- `rxcp_remap_create_procedure_header()`,
  `rxcp_remap_create_args_builder()`, `rxcp_remap_create_arg()`,
  `rxcp_remap_create_void_type()`, and `rxcp_remap_create_reference_type()`
  own generated procedure shell shape;
- `rxcp_remap_create_generated_import()` marks compiler-generated runtime
  imports so validation does not report them as user-authored unused imports;
- `rxcp_remap_create_generated_node_name()`,
  `rxcp_remap_create_generated_indexed_name()`, and
  `rxcp_remap_create_prefixed_name()` centralise generated naming mechanics.

The remaining non-extractions are intentional policy boundaries rather than
obvious framework gaps. Level C still owns Classic variable-pool naming,
uppercase symbol-key normalisation, BIF selection policy, direct helper versus
dispatcher choice, and procedure/expose eligibility. Pool-like locator
materialisation should only move into the neutral layer when another Rexx
front end needs the same reference/copy/writeback policy.

At this point the remapping infrastructure is complete enough for the current
feasibility phase: selectors/rule summaries describe selection, service
boundaries own guards/proofs, and the remap builder owns reusable tree
materialisation commands. Further extraction should be demand-led by Classic
REXX coverage or optimiser rules, not by speculative abstraction.

This path gives a real safety net. The inliner has existing positive and
negative tests, source/import cases, and opt/noopt runtime comparisons. Passing
those tests after each migration is stronger evidence than building a new Level
C lowering engine in isolation.

## Acceptance Checks

For the inlining-remap tracer:

- existing inline tests continue to pass;
- no-opt and opt runtime outputs stay identical for composed inline fixtures;
- negative inline tests continue to show calls that should not be rewritten;
- debug output can identify the rule id and guard reason for accepted and
  rejected rewrite candidates;
- imported `META_INLINE` payload tests continue to round-trip.

Useful focused command:

```sh
ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure
```

For Level C lowering once enabled:

- `levelc_compile_unsupported` changes only when the tracer compile path is
  intentionally opened;
- syntax-highlighting/parser tests still pass;
- the first lowered programs should compare against Regina or another Classic
  reference where practical;
- generated AST after lowering contains no unsupported `LEVELC_*` nodes for the
  accepted tracer subset;
- lowered-tree debug probes assert the expected canonical shapes, especially
  for generated procedures or other shapes that may not be easy to author
  directly as Level B;
- runtime tests cover `RexxValue`, `RexxStem`, `RexxVariablePool`, and
  `RexxClassicBifs` interactions used by the lowered code.

## Open Decisions

- Whether the durable rule notation remains C descriptors plus callbacks or
  grows an external `.rxrw`-style declarative format.
- The exact API boundary between generic remapping services and inliner-owned
  services such as call-site classification and eligibility analysis.
- How much rule tracing should appear under existing `-d2`/`DEBUG_INLINE`
  output versus a new remapping debug channel.
- The exact hidden-symbol naming and sidecar representation for Level C
  variable pools, loop state, and procedure exposure.
- The runtime/helper contract for full Classic stem default values, stem
  `DROP`, and exposing a single resolved compound variable rather than a whole
  stem.
- Which runtime helper calls should be treated as ordinary inline candidates
  and which should be protected as semantic boundaries.
- The point where a dispatcher-backed BIF should gain a specialised direct
  helper.

## Working Position

The inlining-first tracer is the right next move. It is difficult, but it makes
the difficulty visible while the compiler still has strong tests. Level C
lowering then becomes an application of a proven remapping substrate rather
than the first place where the substrate is asked to handle scopes, aliases,
temporaries, source anchors, and fixed-point optimisation all at once.
