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

### Current Tracer Status

As of the inline rule catalog split, steps 2 and 3 are implemented and the four
current call-site inline buckets are represented as selector-backed rules in
`compiler/rxcp_inline_rules.c`. The shared clone, actual-binding, return
rewrite, receiver-copyback, and imported-template machinery is still represented
as inline service boundaries rather than fully generic remap services. That is
the next extraction point before Level C lowering should depend on these
helpers.

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
- The first approved Level C lowering subset that changes
  `levelc_compile_unsupported`.
- Which runtime helper calls should be treated as ordinary inline candidates
  and which should be protected as semantic boundaries.

## Working Position

The inlining-first tracer is the right next move. It is difficult, but it makes
the difficulty visible while the compiler still has strong tests. Level C
lowering then becomes an application of a proven remapping substrate rather
than the first place where the substrate is asked to handle scopes, aliases,
temporaries, source anchors, and fixed-point optimisation all at once.
