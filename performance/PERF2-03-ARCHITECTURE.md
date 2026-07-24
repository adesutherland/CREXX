# PERF2-03 flow-aware inlining 2.0 architecture decision

Status: **approved**; production slices 1-4 authorized, pause before slice 5

Date: 2026-07-24

Baseline: `develop` at
`086138f1e93da8e84d45f4cd3ba9b6620f792a14`

## Decision

Recommend **H: a versioned pre-inline callable summary plus an explicit
per-site expansion plan, a boundary-aware candidate-local flow overlay and a
bounded cleanup/profitability fixed point**.

This is one architecture, not a request to merge the P1-P4 prototype patches.
All expansion remains speculative: the original call stays owned and untouched
until the cleaned candidate wins the final cost gate. Missing proof rejects the
affected fact or site only. The architecture adds no public RXAS instruction,
RXBIN form, ABI surface or VM behavior.

## Why H is selected

The exact-HEAD panel rules out each smaller component as a complete design:

- P1 confirms that a precise call-site equivalence fact can be highly valuable:
  direct receiver placement removes 11 Richards copies/instructions without a
  local-register regression, and its retained accepted Release verdict was
  about 24% faster on both VMs. It does not describe the broader formal,
  result, ownership or profitability problem.
- P2 confirms that the existing read-only-formal fact can remove 21 Richards
  copies/instructions, but it raises Permute's peak locals from 30 to 32 and is
  not a speed win in an exact-HEAD AC run: +0.052% on `rxvm`, -1.152% on
  `rxbvm`. A pre-inline summary is necessary but cannot decide alone.
- P3 confirms that candidate-local result placement is semantically viable and
  removes some Richards scaffolding, but it adds an instruction in RexxCPS. A
  post-clone rewrite is necessary but cannot be unconditional.
- P4 proves that the detached-candidate/fallback boundary works and can cut
  Richards from 1,897 to 1,385 static instructions. Its 100-node-only gate also
  makes the failure of a one-dimensional policy explicit: RexxCPS call
  instructions rise from 27 to 84 and peak locals from 105 to 107. A bounded
  fixed point needs a multi-metric, profile-aware final decision.

Design A lacks site-specific post-clone cleanup. Design B lacks early semantic
facts, imported parity and safe speculative ownership. Design C lacks the facts
needed to prove its rewrites. H joins the useful parts at explicit ownership
and invalidation boundaries.

## Fact and ownership model

### `InlineCallableSummary`

A versioned immutable summary is produced after validation, typing and current
formal-effect discovery, before destructive cloning. Local and imported
callables expose the same schema alongside the existing inline body payload.
It records only facts proved at that stage:

| Domain | Required facts |
| --- | --- |
| Formal | type/shape, read/write/escape, by-value or `.ref`, optional presence and default obligations |
| Receiver | direct/computed identity, class/object effects and copyback obligations |
| Result | alternatives, fallthrough, scalar/aggregate/reference shape and ownership |
| Control | signal/unwind, cleanup regions and externally visible exits |
| Context | numeric inheritance, TRACE and source-identity requirements |
| Cost | raw structural nodes and conservative pre-cleanup instruction/copy/branch/local estimates |

The summary is conservative. Absence of a fact means no transformation that
requires it; it is not a whole-procedure ban. Payload schema/version mismatch
falls back to the current call path.

### `InlineExpansionPlan`

Every candidate site owns:

1. the untouched original call;
2. a detached expanded candidate;
3. actual-to-formal and receiver equivalences;
4. capture and evaluation-order obligations;
5. optional/default presence paths;
6. formal, block-result and return-result identities;
7. cleanup/ownership and signal/unwind obligations;
8. inline provenance and a pre-cleanup cost snapshot.

The plan, rather than the AST replacement itself, is the transaction boundary.
If proof fails, cleanup invalidates an obligation, or cost loses, the detached
candidate is discarded and the original call remains unchanged.

### Candidate-local flow overlay

The post-clone overlay extends the NR-26 fact vocabulary across inline-created
`BLOCK_EXPR`, `LEAVE_WITH`, SELECT/SWITCH and joins. It gives cleanup queries
for definitions, uses, values, liveness, last use, receiver/formal/result
identity and cleanup ownership without making mutable CFG internals part of the
general compiler API. Inline provenance scopes invalidation and diagnostics to
one candidate.

## Cleanup and convergence

For one detached candidate, iterate in this order:

1. propagate proved constants and scalar copies;
2. remove dead formal/default/exit/branch/result scaffolding;
3. apply proved last-use moves and result placement;
4. rebuild or invalidate only affected candidate facts;
5. stop at a fixed point or the documented candidate-local bound.

The convergence measure is lexicographic and monotone: executable AST nodes,
stores/copies, branches/exits, then unresolved candidate markers. A rewrite is
accepted only when it reduces that measure and preserves all proof obligations;
no rewrite reintroduces a removed candidate marker. The iteration bound is
derived from initial candidate nodes plus marker count. Hitting the bound is a
site-local non-inline fallback, not a compiler failure.

## Final profitability gate

Compare the cleaned candidate with the retained call using all of:

- profile-weighted dynamic instruction and call savings where a valid profile
  exists, otherwise a conservative static estimate;
- peak locals/register pressure as a hard regression guard unless an explicit
  measured win justifies it;
- static instructions, copies, branches, temporaries and default scaffolding;
- RXAS, RXBIN and linked-image bytes; and
- any remaining cleanup, context, TRACE/source or ownership obligation.

Install only a decisive win. An uncertain, neutral or losing candidate remains
a call. This policy deliberately rejects both P2's Permute local regression and
P4's RexxCPS call/local regression even though another metric improves.

## Initial transformation panel

| Transformation | Required proof | Losing/missing-proof action |
| --- | --- | --- |
| Direct receiver placement | receiver identity, evaluation order, no required copyback | retain receiver capture/call |
| Read-only scalar formal sharing | formal read-only, direct compatible symbol, no escape/ref/default obligation | materialize formal or retain call |
| Default/omitted cleanup | exact presence path and default evaluation/side-effect proof | retain default scaffold |
| Result placement | one compatible destination, ownership and sibling liveness | retain block/result temporary |
| Last-use move | unique last use and cleanup-owner transfer | retain copy |
| Dead exit/branch cleanup | boundary-aware reachability and unwind equivalence | retain control scaffold |

Writable by-value values, `.ref`, aggregates, objects, overlapping/repeated
actuals and computed expressions are transformed only when their own alias,
snapshot, lifetime and cleanup proofs exist. TRACE, calls, references and
handwritten RXAS are never blanket exclusions.

## Smallest proposed production slice

If Adrian approves implementation, take exactly one slice:

> Add the `InlineExpansionPlan` detached transaction/fallback scaffold and use
> it only for the already-proved P1 direct method-receiver equivalence. Do not
> enable P2 formal sharing, P3 general result placement, P4's node threshold or
> the broader cleanup fixed point in that slice.

This seeds H at its ownership boundary while reusing the strongest measured
fact: current exact-HEAD focused correctness is 33/33, Richards removes 11
copies/instructions with unchanged peak locals, and the retained accepted
profiling-off Release evidence shows the large both-VM benefit. The slice is a
narrow architecture seed, not the completed flow-aware inliner.

After the minimum focused correctness checks pass, freeze implementation, build
the ordinary profiling-off Release product, run the smallest decisive Richards
comparison against retained valid evidence, report the first Release verdict
to Adrian and stop. No broad closeout precedes that verdict.

## Decision

Adrian approved H on 2026-07-24, then authorized production slices 1-4 with QA
and an independent commit after every slice. Production execution must pause
before slice 5 reference/object ownership work.
