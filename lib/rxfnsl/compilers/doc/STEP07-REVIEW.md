# STEP07 proposal: expand call and module contracts

**STEP07A and its exact API approved.** Adrian approved the bounded increment
on 15 October and its [public API](STEP07A-CONTRACTS.md) on 16 October 2026.
Implementation uses Level B and the Level L grammar, AST, IR, analysis and
writer libraries. Existing production syntax remains the reference. Develop
publication was authorised on 17 October; full STEP07/STEP07B obligations remain in
[PLAN.md](../PLAN.md).

**Local completion, 16 October 2026:** F7A-AC01–06 and F7A-STEP01–04 are
verified for this bounded increment. The [validation summary](../PLAN.md#step07a-validation)
maps the criteria to independent Level L, C/G, installed and sanitizer evidence.
The implementation is included in the authorised develop delivery; CI monitoring
remains paused and hosted outcomes are separate.

## Vision and intended outcome

Move beyond fixed scalar signatures and narrow library-call routes. Typed G/B
and Classic callers need explicit, verified argument/result contracts that
survive separate compilation, optimisation and RXAS emission. Level L supplies
reusable structural checks and views; compiler services own types, defaults,
presence, numeric context and activation rules.

Start with **scalar calls, optional literal defaults and presence**, then carry
those contracts through a small separately compiled G library and admitted
Classic runtime-call routes. Inventory current metadata before extending it.
General Classic external-program search/loading, arbitrary providers, source
imports, references, class defaults, variadic tails, arbitrary default
expressions and INTERPRET remain later named coverage. Unsupported forms must
not fall back to the native compiler.

## Simple paired source example

Implemented G coverage, using existing typed syntax:

```rexx
options levelg
say scale(7)
say scale(7,3)
exit 0

scale: procedure = .int
  arg value = .int, factor = 2
  return value * factor
```

The Classic control expresses its default explicitly:

```rexx
options levelc
say scale(7)
say scale(7,3)
exit 0

scale: procedure
  arg value, factor
  if \arg(2,'E') then factor = 2
  return value * factor
```

Both should print `14`, then `21`. G's declaration supplies a typed default
when the second argument is absent. Classic ARG retains its parsing and
presence rules: `ARG(2,'E')` tests the original call even after a local variable
is assigned. An explicitly supplied empty string is present and must not select
the missing-argument default. G's existing `?factor` presence query should also
distinguish omission from a supplied value. Conversion/error behaviour comes
from each language contract, not matching output in this one example.

The [procedure reference](../../../../docs/books/crexx_language_reference/procedures_and_arguments.md)
defines typed defaults and presence; current Classic
[argument tests](../tests/backend/arguments-c.crexx) distinguish omitted/empty
values. G optional/default source forms extend the original PoC in STEP07A. Both
examples now compile through the new tools; their executed outputs and RXAS
are retained with the qualification receipt.

## Tree, IR and emission responsibilities

| Stage | G | C |
|---|---|---|
| Source AST | Required integer `value`, optional `factor` with literal default `2`; calls retain actual positions | PROCEDURE/ARG nodes and authored presence/default logic; calls retain actual positions |
| Checked call contract | Target identity, formal/result types, required/optional positions, default and original presence | Target/activation, original presence and ordered Classic values; assigning locals never overwrites call presence |
| Level L IR | Checked numeric references, ordered operands, result/control/effect information and source origins | Same structural services; Classic conversions/context/pools retain their registered semantics |
| Lowering/RXAS | Existing typed call/default/presence ABI | Classic argument packets, activation, numeric context and result behaviour |

Inserting a default must not erase original presence. Effects must not be
reordered or duplicated; unknown external effects prevent unjustified
propagation. Function inlining and register packing are not part of STEP07A.
Any generic descriptor/verifier added to Level L needs an independent non-C/G
fixture. Extend existing schema/service APIs where possible and review exact
new public fields before implementation. Readable views project checked numeric
records; the IR remains compact rather than text-based.

## Checkable acceptance criteria

1. **F7A-AC01 — scope:** inventory current signature/import contracts and mark
   each form admitted or refused. Retain independent expected outputs/failures
   and native controls; review public API additions before editing them.
2. **F7A-AC02 — presence/defaults:** required, omitted optional, explicit value,
   empty value and presence queries preserve their meanings. Evaluate admitted
   operands once in source order. Test both languages with IR on/off, errors
   and recursion. Scalar literal admission does not admit arbitrary defaults.
3. **F7A-AC03 — modules:** a separately compiled G scalar routine exports and
   imports cREXX-read/written metadata; admitted C runtime routes use checked
   contracts. Wrong/missing signatures, required actuals and malformed imports
   fail with locations and no partial output. Old compiler services remain
   unavailable to new compilation requests.
4. **F7A-AC04 — reusable Level L:** shared layout/descriptor validation and
   transformation obligations belong in Level L, with a non-C/G regression.
   Defaults, conversions, activation and effect rules remain language-owned;
   unknown semantics remain conservative.
5. **F7A-AC05 — emission/ownership:** Level B selection uses Level L writer
   services. Retain complete RXAS examples and tests for necessary call/default
   work, presence, lifetimes, origins and file-preserving failures. Native
   B/G/C ABI controls do not make their incidental excess the quality target.
6. **F7A-AC06 — delivery/docs:** focused end-to-end and installed CLI/service
   checks, unchanged evidence reuse, updated architecture/trees/algorithms and
   guide. Keep VM/host/assembler modes and local/hosted boundaries explicit.
   Formal research performance tests are not required. Proper B and full
   replacement remain open.

## Approved sequence and gates

| Step | Work | Gate / acceptance |
|---|---|---|
| **F7A-STEP01 — complete** | Pin signatures/presence/default/export metadata and paired corpus; show exact tree/IR/API deltas | Exact API approved 16 October; AC01 verified |
| **F7A-STEP02 — complete** | Add required shared checks/views, G admission and C contract tests | AC02/04 verified, including independent Level L regression |
| **F7A-STEP03 — complete** | Carry contracts through separate G modules and C runtime imports to the existing backend | AC03/05 verified, no old-compiler delegation |
| **F7A-STEP04 — locally complete** | Qualify focused semantic/error/shape/installed paths, document remaining gaps | AC01–06 verified locally; develop publication authorised 17 October |

**Next delivery priority, approved 16 October:** complete Level C before
STEP07B. The [C-first milestone](../PLAN.md#c-first-delivery-milestone) supersedes
the earlier balanced C/G expansion sequence. Adrian's subsequent direction is
to review instructions one at a time, starting with SAY, and leave STEP08
INTERPRET and its mechanism review until later. Each instruction records its
semantics, errors, RXAS and shared dependencies; eventual complete C still
requires Classic host/condition/TRACE coverage and INTERPRET. G remains
maintained at current coverage; proper B/self-hosting and further G expansion
are deferred with their criteria intact. Actual RXAS quality and independent
semantic expectations are required throughout; native C output may be faulty
or inefficient. Exact public API and architectural decision gates remain.

## F7A-STEP01 checkpoint

The [contract and tree proposal](STEP07A-CONTRACTS.md) records the source-form
inventory, minimal public additions, numeric format transition, call matching,
metadata and emission obligations. The [native controls](../PLAN.md#step07a-validation)
ran 38 focused commands, including 14 executions on both VMs. B/G/C optional
defaults and presence, separately compiled G metadata and a missing-required
argument diagnostic are pinned. They validate the reference behaviour only.
That design checkpoint was reference evidence. The separate implementation
receipt now closes the approved increment; STEP07B remains separately gated.
