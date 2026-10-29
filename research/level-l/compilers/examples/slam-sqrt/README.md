# Exploring Classic Rexx with Level L: a SLAM example

This demonstration asks what Level L could contribute to Tom Dyer's SLAM code
auditing. Its cREXX consumer reads real Classic Rexx through the existing
Level L grammar, explores the source tree, compares deliberately normalized
shapes, and queries checked control-flow and effect services. Python supplies
independent exact-arithmetic certificates for the numerical results.

The useful distinction is between **what was written**, **where execution can
go**, **what a bounded analysis has established**, and **what a test observed**.
Each finding below retains that distinction. This is a worked analysis input
for a SLAM-style auditor; weighted search, NEEDS/GIVES composition and an
SQLite evidence store are not implemented here.

## Run the demonstration

With the normal Debug Classic C research products already prepared, run from
the repository root:

```sh
# cREXX tree, shape, CFG and proof exploration only:
python3 research/level-l/compilers/examples/slam-sqrt/compare.py --build cmake-build-debug --analysis-only --check

# Also compile/run the numerical cases and check their certificates:
python3 research/level-l/compilers/examples/slam-sqrt/compare.py --build cmake-build-debug --check
```

The driver builds [analyse.crexx](analyse.crexx) with the native B seed and
prepared frontend libraries. It does not regenerate the grammar. Numerical
programs pass through `llrxc-c`, `rxas`, `rxlink` and one `rxbvm`. Generated
sources, RXAS and images are temporary and are removed after the run. The
report is printed to stdout; no report is committed to the repository.
Static analysis reads each authored fixture directly, so its line numbers refer
to that file. Numerical execution adds a common input/output wrapper around
the same routine body; the checked static graph excludes that wrapper.

`--check` checks the authored positive/negative controls as well as the
published numerical contrast. `--values path` supplies a custom nonnegative
finite decimal input set without that fixed-corpus assertion.

## 1. Ask the grammar for a tree

These are excerpts from the runnable Level B consumer, not a separate parser:

```rexx
import llcfrontend
import llast

front = .llcfrontend..request(source,artifacts,'FDSqrt.rexx')
if front.status() <> 0 then do
  say front.diagnostic()
  return 1
end
tree = front.ast()
body = tree.root().child('body')
```

The consumer finds a `CLabel` whose `name` is `FDSqrt`, then examines the
following top-level statements up to the next label. A string or comment
containing `FDSqrt:` cannot select the routine. Classic labels and PROCEDURE
remain distinct nodes; this bounded example does not invent an ooRexx method
model or a general Classic call graph.

Children have grammar-defined roles. This snippet explores an IF node:

```rexx
condition = node.child('condition')
yes = node.child('yes')
say condition.sourcetext()
say yes.kind()
say node.line()
```

For the revised routine's exit it exposes `next>=guess`, a `Leave` node and
an authored line. `node.each()` traverses all children in declaration order;
`tree.walk('parents first')` is available for whole-tree visitors. Both borrow
the live owning tree. `showtree()` in the consumer uses recursive child cursors
and hides transparent expression wrappers in its human-facing outline.

The revised routine's meaningful outline is:

```text
Assignment next = (guess+(x/guess))/2
IfThen next>=guess
  Leave
Assignment guess = next
Return guess
```

The consumer's `loopfacts()` interprets the flat loop body using Classic
LEAVE semantics: taking that exit bypasses `guess=next`, and the final return
uses the previous estimate. Vision instead has a conditional `return next`
inside a counted loop. These are source/control observations, not proofs that
either stopping rule gives a sufficiently accurate root.

The flow explainer is deliberately limited to the fixtures' flat loops and
unnamed direct conditional LEAVE. It is not a general transfer/handler analysis.
The missing-exit control reports the absent direct exit and is never executed;
this does not establish all-input termination or nontermination in the presence
of errors, signals, callbacks or host interruption.

## 2. Turn a tree observation into a small explicit proof rule

A division appears as a `Chain` with ordered `operators` and `operands`:

```rexx
operator = node.child('operators',i)
if operator.kind() = 'divide' then do
  divisor = node.child('operands',i + 1)
  verdict = literalnonzero(divisor)
  say divisor.sourcetext() verdict
end
```

The runnable `literalnonzero()` helper unwraps transparent chains and groups.
For an unsigned ASCII integer literal it inspects the exact spelling: all-zero
spelling disproves nonzero; a nonzero digit proves it. Every other expression
returns `unknown`. It uses no approximate arithmetic and assumes no variable
range. Thus the demonstration has three useful outcomes:

- `2`: nonzero **proved**, by this consumer's literal rule.
- `0` in the injected divisor fault: nonzero **disproved**, before executing it.
  The error is inevitable if that division is reached; an earlier return can
  still avoid it.
- `guess`: **unknown**; proving positivity/nonzero needs a value analysis that
  accounts for input, initialization, updates and Rexx arithmetic semantics.

Level L provides the checked representation and traversal. The small rule is
our application analysis, and is not a general theorem prover built into L.
It does not establish that the surrounding division completes without error.

## 3. Query the real checked IR and its proof services

AST roles explain source constructs; the Classic frontend's IR describes
actual evaluation and control. Keep the module, index and effect model in
stable storage while querying their borrowed results:

```rexx
import llirgraph
import llireffects
import llceffects

graph = front.graph()
index = .llirgraph..index(reference graph,1000000,67108864)
if index.status() <> 'ok' then do
  say index.message()
  return 1
end
loops = index.loops()
live = index.liveness()
```

The consumer checks each construction status. For an actual `crexx.c.div`
operation it follows an indexed use of its result:

```rexx
value = op.result(1)
useid = index.use(value.id(),1)
consumer = graph.operation(useid)
dominates = index.dominates(index.opblock(op.id()),index.opblock(useid))
```

The fixtures show the `x/guess` result used by `crexx.c.add`; changing the
operator gives `crexx.c.sub`. The producer block dominates the use block.
This is a checked CFG fact. Dominance alone says nothing about convergence,
result accuracy, or whether the producer completed normally.

The next query makes that additional obligation explicit:

```rexx
model = llceffects..model(reference graph)
proof = .llireffects..proof(reference index,reference model,
                          'available',op.id(),useid,'',1)
say proof.status() proof.reason()
```

On this real Classic division the result is `unknown` with
`completion.edge.unknown`: the C effect model retains possible failure and
context boundaries. An indexed use is insufficient authority to move/reuse
an operation's result across those boundaries. `live.complete(fnid)` is also
reported; its conservative storage facts are not proofs about Classic dynamic
variable pools, NOVALUE behaviour or aliases.

`unknown` is a refused conclusion, not proof that the program is wrong. Graph
construction or resource failures likewise publish no usable analysis.

## 4. See a checked proof succeed, and see its premises matter

`proofcontrols()` builds a separate tiny acyclic IR: a constant, a declared
step and a store. It is an executable API teaching control, not a simplified
proof of the original Newton loop. The language adapter supplies explicit
trusted effect premises for the step:

```rexx
summary = .llireffects..summary(1,1)
call summary.operand(1,'value')
call summary.result(1,'normal')
call summary.control('none','none','none')
call summary.write('unrelated')
sealed = summary.seal()
call model.put(step.id(),summary)
sealed = model.seal()
```

The query checks whether the interval preserves a named state domain:

```rexx
proof = .llireffects..proof(reference index,reference model,
                          'preserves',generator,useid,'demo.estimate')
if proof.status() = 'proved' then do
  if proof.matches(reference index,reference model) then say proof.reason()
end
```

An unrelated write permits `proved` with
`state.preserved.in.acyclic.interval`. Declaring a write to `demo.estimate`
returns `unknown` with `state.or.observation.changed`; omitting the step's
premise returns `unknown` with `effect.unknown`. The service checks consequences
of the declared semantics; it does not prove that a caller's declaration is
true. Replacing the owning graph/index/model invalidates dependent proofs.
The `preserves` rule is bounded to an ordered interval in one acyclic block.

See [the L proof contracts](../../../../../langtools/doc/ir-design.md#optional-analysis-and-proof-services)
for exact rule boundaries. The runnable snippets demonstrate both successful
use and principled refusal without inventing a universal numerical proof.

A further value analysis could carry `x>0` along the guard's normal edge,
derive the initial bound on `guess`, and attempt to preserve a nonzero bound
through updates. That would require Rexx decimal/comparison semantics and
invalidation at writes, calls, handlers or dynamic code. A convergence or
termination argument would require additional invariants and a progress
measure. Those analyses are not supplied by this example's literal rule,
dominance query or effect model.

## 5. Compare controlled variations

`normalized()` is a small cREXX AST consumer. Within the admitted fixture
profile it assigns simple local variables stable IDs by first occurrence.
It retains operator kinds, grouping, literal spelling and ABS call identity;
comments and source coordinates do not enter the representation. EXPOSE,
compound names, unknown calls and unsupported node kinds are refused. This is
an explicit normalization policy, not built-in semantic equivalence or a
claim that arbitrary Rexx variable renaming is safe.

The checked controls serve distinct purposes:

| Variation | Intended question and conclusion |
|---|---|
| Consistent local renaming | Same normalized tree; the sample numerical outputs should agree. This does not prove all-input equivalence. |
| Old loop count `14` changed to `1` | Different tree; the independent checker should expose increased numerical failure. |
| Newton `+` changed to `-` | Different operator in the tree and corresponding IR use. Analysis only. |
| Revised exit removed | Different tree; absent direct loop exit is visible. Analysis only. |
| Divisor `2` changed to `0` | Different literal; the bounded literal rule disproves a necessary condition before execution. Analysis only. |

## 6. Keep numerical evidence independent

The original 29 inputs and routines reproduce SLAM's published contrast:
12 old-routine answers fail the exact tolerance certificate, while all revised
answers pass it. Revised and Vision agree on all 29 sample answers; at `0.5`,
`99.5` and `2E9` they agree but fail the nearest-nine-significant-digit check.
The one-step control is expected to fail tolerance on all inputs except `1`.
These are maintained expectations checked by `--check`, not new bug discoveries.

Python converts decimal input/output strings to exact fractions. For a
nonnegative candidate `r` and input `x`, tolerance checks
`(r-r*1e-8)^2 <= x <= (r+r*1e-8)^2`, clamping the lower bound to zero. The
nearest-nine check uses squared midpoints to adjacent nine-digit decimal
values, including their unequal spacing at powers of ten. Midpoint ties are
accepted; no tie-breaking convention is certified. Agreement is reported
separately from either certificate.

Separate runtime probes retain the old/revised FDSqrt policy: `-1` and `0`
both return `0`. A negative input has no real-square-root certificate. Vision's
original ooRexx negative-input exception has not been ported, so the tool
explicitly skips that comparison. A `x<=0` guard is source evidence, not a
mathematical justification for defining a negative input's root as zero.

## What this contributes to SLAM

SLAM can supply candidate relationships, scoring, composition and accumulated
test evidence. This example shows a possible grammar/analysis input: exact
constructs and source origins, a stated shape policy, actual control/data-use
relationships, small static obligations and explicit reasons a bounded proof
was accepted or refused. It gives an auditor evidence it can point to and
limitations it must preserve. It establishes no performance comparison,
general hidden-precondition discovery, ooRexx grammar support or universal
termination/correctness result.
Level L can host other input languages, but ooRexx would need its own grammar
and semantic adapter; the current demonstration uses the Classic frontend.

## Source and attribution

The fixtures adapt [`FDSqrt` before](https://github.com/animatedads/alchemy/blob/main/packages/oorexx_slam_v0.1/tests/fixtures/fdsqrt_fixed14.cls)
and [after](https://github.com/animatedads/alchemy/blob/main/packages/fd_door_micro_motion_v0.2-dev8/src/FDDoorMicroMotion.cls)
the repair, and [Vision's rx_vision_sqrt](https://github.com/animatedads/alchemy/blob/main/packages/oorexx_vision/src/Vision.cls).
The inputs come from [SLAM's test_fdsqrt.rex](https://github.com/animatedads/alchemy/blob/main/packages/oorexx_slam_v0.1/tests/test_fdsqrt.rex).
The original door routine was written by ChatGPT; SLAM's architecture is Tom
Dyer's and its code is Claude's (Opus 5.5). The positive-input calculations are
preserved while ooRexx routine/argument syntax is adapted to Classic labels,
PROCEDURE and PARSE ARG. The additional faulty variations are solely cREXX
teaching controls.
