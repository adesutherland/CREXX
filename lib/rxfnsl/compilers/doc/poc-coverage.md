# C/G PoC coverage and boundaries

This is the Phase A coverage map for the [approved POC-01–10 profile](poc-scope.md).
[PLAN.md](../PLAN.md) owns acceptance; the [STEP06 receipt](../evidence/step06-20261015/README.md)
records actual runs and input identities. A row establishes the admitted slice,
not full C/G language compatibility. The compiler and reusable services are
Level B; grammars use Level L and RXAS output uses `llir.writer`.

## Source to execution coverage

Paths in the test columns are relative to `compilers/tests`. Each executable
case has an independent expected stdout/error in `backend/cases.json`, or in
`check-poc.py` for the new paired cases. Existing compiler controls supplement
those expectations. They are not consulted by either new compiler.

| Profile | Source and intended behaviour | AST and IR evidence | Emission/execution and failures |
|---|---|---|---|
| POC-01: source | `backend/spelling-{c,g}.crexx`, `provenance/unicode-{c,g}.crexx`, new `poc/control-{c,g}.crexx`: quoted Unicode, doubled quotes, nested comments, clauses and CRLF | `frontend-tests` grammar/syntax/treechecks preserve exact spelling and byte extents; independent `ts_llsource` checks Unicode columns and line endings | Spelling/Unicode backend and provenance cases execute on both VMs; malformed UTF-8/NUL, unclosed quotes/comments and missing operands are located failures. Walkthrough `.ast` files retain real nonterminal trivia ranges. |
| POC-02: scalar values | `backend/scalars-g.crexx`, `spelling-*`, `conversions-g`, `negative-zero-g`, numbers pair: C `01` stays `01`; G prints `1`; C unset reads return the uppercase name; typed defaults retain their values | Mapping assertions distinguish C pool access from G slots/types; original and selected IR are visible in STEP06 | Existing scalar/conversion/default/exact-byte backend tests, integer-boundary evaluator tests and new walkthrough on/off execution. G `.decimal` starts absent (`nan`); no full numeric-language claim. |
| POC-03: expressions | `backend/operators-g`, `text-comparisons-*`, evaluation pair and new `poc/control-*`: arithmetic, comparisons, concatenation, unary operators and nested logic | G short-circuit blocks versus eager C operations; independent non-C/G optimiser oracle and C/G branch oracle | Existing operator/cast/error cases; new nested RHS prints twice in C and never in G. Unknown semantics, possible failures and mutable state retain conservative behaviour. Overflow/division are not presumed foldable. |
| POC-04: statements | Pipeline, spelling, loops and `examples/step06/walk-*`: assignments, SAY, NOP, groups, IF/ELSE, keyword names | Named `condition`, `yes`, `no` children and nearest-IF assertions; core branch edges versus source-tree ownership | Existing control/default/emitter tests; new walkthrough retains ELSE in AST/frontend IR and removes its division only in selected IR. Missing THEN fails compilation and preserves the prior destination. |
| POC-05: loops | `backend/loops-*`, `calls-g`, new `poc/control-*`: zero/several iterations, nested loops, LEAVE/ITERATE and one-iteration early return | `while.test/body/after` blocks, explicit jumps; syntax/mapping checks reject LEAVE/ITERATE outside a loop | Existing loops plus new paired return from a loop, IR on/off. Post-test/counted/named loops remain outside this profile. |
| POC-06: routines | `backend/calls-*`: factorial returns `720`, operand calls print in source order; new controls return from a loop | Typed G signatures; separate C label/PROCEDURE nodes and explicit activation/argument/result packets | Recursive calls, early return, G void/discard and C private-pool isolation; wrong arity, missing typed actuals, void-as-value, missing return and Classic fallthrough/label-only profiles fail explicitly. |
| POC-07: Classic arguments | `backend/arguments-c`, `argv-c`, `calls-c`: omitted versus empty arguments, uppercase ARG, presence, RESULT clearing | Ordered explicit missing argument nodes and packet append/setresult operations | Existing expected output includes ARG count/presence and unset RESULT after valueless RETURN. No-value expression call is a runtime error. Optional/default/vararg G and EXPOSE source forms remain later. |
| POC-08: numeric context | `backend/numeric-c`, numbers pair, new `poc/control-c`: inherited/restored context, form/reset and expression-valued DIGITS/FUZZ | Numeric snapshot operands and named settings; FORM/comment mapping regression | New setters each print once; DIGITS 4/FUZZ 1 makes `1.234 = 1.233` true, resetting FUZZ makes it false, resetting DIGITS restores 9. Existing cases cover call restoration, forms and invalid DIGITS/FUZZ. |
| POC-09: imports | Pipeline/evaluation and `frontend-tests` calls/contracts: G imported LENGTH, C LENGTH/ARG/DIGITS | Level B RXBIN reader checks actual type/member/callable/autoload data; no native metadata reader | Missing/foreign/truncated contracts and malformed signatures fail; installed commands use the explicit artifact directory. Generic imports/providers/classes are not implied by this narrow contract subset. |
| POC-10: failures | Syntax/type/profile groups, `backend-tests`, `check-backend`, `check-provenance`, new walkthrough variants | Failed parse or recovered tree never yields a successful graph; invalid/foreign graphs and unknown target contracts are refused | Located source errors, reached/skipped division, non-Boolean Classic truth, conversion errors, budgets, output-write/rename failures and destination preservation. Full authored TRACE/condition handlers and recovery parity remain later. |

## Ownership, limits and independent Level L proof

Existing Level B regressions are reused with their qualified product/source
identities, rather than replacing them with a compiler-specific test:

- `frontend-tests` lifecycle checks repeated/idempotent requests, earlier owned
  trees/graphs surviving another failure, missing imports, source/work/nesting
  limits and no ambient import fallback.
- `backend-tests` checks output surviving its request, output/register/work
  refusal, empty/unterminated/foreign/forged graphs, exact byte quoting, source
  ranges, logical identities and selection-based frames.
- Independent `ts_llir` / `ts_llirservices` cover schemas, codecs, stale/foreign
  handles, unknown contracts, checked lowerings, writer sections/chunks/sinks
  and bounded atomic output. `ts_lloptimise` uses its own non-C/G semantics and
  bounded interpreter; it checks source immutability, maps, all function roots,
  effects, state barriers, divergence, refusal, convergence and work/index/byte
  limits. C/G modules are not imported by those generic consumers.
- `backend-optimise` checks C/G admitted evaluators, an independent branch
  oracle, original/selected graphs and remapped writer origins/sink failures.
- `check-backend` preserves a prior destination on syntax/type/profile/UTF-8,
  missing input/import, staging collision and rename failure. New missing-THEN
  checks exercise the same contract in both compiler IR modes and languages.

Concurrent compilation/reentrant VM sessions are not inferred from sequential
owned requests. The later compile-service/INTERPRET gate owns those requirements.
Resource counters are deterministic safety limits; they are not wall-clock
performance promises. Formal performance testing is not a research requirement.

## Dependency and mode inventory

| Role | Actual dependency | Boundary |
|---|---|---|
| Initial seed/build | Native B `rxc`, `rxas`, `rxlink`, concrete VM, Level B `library.rxbin`; Level L grammar generator | Builds B compiler libraries and generates C/G grammar bytecode. No G compiler source import/host prerequisite. Proper B and B2/B3 self-hosting remain STEP07B. |
| New compilation request | B/L compiler bytecode: frontend, grammar, contract reader, IR/services and RXAS selector/writer | No calls or links to the old source compilers or their C backends. Empty-PATH command checks and source/build import inventories establish this boundary. |
| Command packaging | Generated native VM launchers; existing filesystem provider | Packaging/runtime facilities, not native compiler algorithms. The reusable `llcompilers.rxbin` service omits CLI filesystem dependencies. |
| Target contracts | `-i` explicitly supplies RXBIN data | Reading Classic metadata does not load Classic semantics into the compiler host. |
| G execution | Emitted RXBIN plus `library.rxbin` | Existing assembler/linker/VM remain platform tools. |
| C execution | Above plus Level B `llctarget.rxbin`, existing `rxfnsc.rxbin` and its class/runtime dependencies | Pool, argument-presence, numeric-context and fatal-condition semantics belong here. These target dependencies are separate from B/L compiler-host closure. |

Three independent controls must remain visible:

| Axis | Control | Evidence boundary |
|---|---|---|
| Build the B compiler host | Native seed `rxc -n` and matching host assembly control versus normal host build | STEP05 both host variants, standalone B-only bootstrap opt/no-opt; STEP06 new checks in both variants. This does not mean every native prerequisite was rebuilt with optimisation disabled. |
| Transform C/G IR | New `llrxc-c/g -n` or request `optimise=0` versus default | STEP05 paired checks and STEP06 walkthrough/control/error variants. Baseline emitter cleanup applies in both. |
| Optimise target assembly | `rxas -n` versus normal assembler | Existing backend/provenance suites cover both; new STEP06 checks fix `-n` to attribute changes to shared IR optimisation. |
| Execute target | `rxbvm` versus `rxtvm`, where built | Existing and new checks name the concrete VM; product alias `rxvm` is not a third independent implementation. |

Local macOS ARM64 evidence covers these paths and fresh installed products.
Normal hosted checks for STEP05 commit `5f9feff27eaf114bd57f17e162d956270ff8fc54`
were still running at the STEP06 audit: [Build](https://github.com/adesutherland/CREXX/actions/runs/38797175765)
and [CodeQL](https://github.com/adesutherland/CREXX/actions/runs/38797175713), attempt 1.
That is not a hosted pass for new STEP06 tests. Windows/MSVC supplies the portable
VM only; `if(TARGET ...)` registers the threaded path only where available.
Linux, Windows, Intel macOS and MinGW STEP06 execution are unverified here.
Constrained/mainframe bootstrap, installation and resource applicability remain
open: no local desktop run closes them. Full LLCOMP-AC12 remains open.

## Code quality and remaining work

The [STEP05 full-module counts](../examples/step05/README.md) and
[STEP06 walkthrough](../examples/step06/README.md) separate instructions,
calls, frame registers, metadata and bytes. The earlier native B/G/C comparison
is a dated control, not a current new-backend snapshot or an optimality target.
Existing structural tests reject unused defaults, gratuitous call windows,
entry fallthrough jumps, false authored provenance and physical source paths.

| Remaining cost/limitation | Reason it is retained | Owner and later closure |
|---|---|---|
| Distinct value registers and retained slots/pools | Simple stable allocation; STEP05 removes operations/values/blocks, not every storage table | Shared liveness/packing and backend allocation proposal, with call-window/lifetime/error proof before enabling reuse. |
| Classic call flag/count/copy/null groups | Required argument presence, borrowing and cleanup under the current ABI | Compiler RXAS target adapter; later specialised call lowering must preserve ownership and error behaviour. |
| Classic numeric/context/pool helpers | Context can change at runtime; generic passes have no licence to guess Classic arithmetic or pool facts | Compiler semantic services and target adapter; later guarded facts/specialisation, with context mutation/recursion/error tests. |
| Clause ID zero, full TRACE and conditions | Truthful absence of authored grouping; current essential range/origin and fatal-message relay only | Source-event/condition gate before broad TRACE, handler or INTERPRET claims. |
| Narrow imports and scalar types | Deliberate Phase A contracts | Balanced STEP07 coverage; no arbitrary module/class/ref/source-import support claimed. |

The PoC demonstrates that Level L can host both deliberately different
frontends, generic checked transformations and real target emission. Its useful
shared outputs are the numeric IR/contracts, reconstruction and proof-aware
passes, source index, owned failures and writer. The next review should start
with **typed/local call and module contracts for G/B alongside Classic
argument/activation contracts**, using independent Level L tests wherever a
reusable contract is added. Inventory optional arguments, explicit presence,
return values and import errors before choosing a surface. Follow with
reference/alias effects and Classic pool exposure, then structured loop forms.
Proper B's source/ABI inventory can proceed alongside this work; implementation
and INTERPRET mechanism changes need their own review. No STEP07 surface is
approved by completion of this bounded STEP06 qualification.
