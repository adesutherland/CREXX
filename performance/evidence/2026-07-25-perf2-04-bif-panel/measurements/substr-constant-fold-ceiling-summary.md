# PERF2-04 RexxCPS SUBSTR constant-fold ceiling

SLC-CF1 is a non-production exact-site control built from detached commit
`6567f0ba23f20623e01322f5a62323b2347ab09d` in
`/private/tmp/crexx-perf2-04.3k3Dfw/substr-const-src`. The exact B0-R
profiling-off Release compiler and assembler produced its RXAS and RXBIN. No
production source, BIF body, compiler, assembler, VM, ABI or serialized format
was changed.

## Selected sites and semantic proof

Both current calls are binary-imported `rxfnsb.substr` bodies already admitted
by PERF2-03 and inlined into the local RexxCPS `main`; neither has a residual
function call opcode.

| ID | exact source | executions per outer iteration | canonical evaluated operands | canonical result | selected semantic cell |
|---|---|---:|---|---|---|
| SLC-S1 | `substr(1234 "5678", 6, 2)` | 14 | source `"1234 5678"`, start 6, length 2 | `"56"` | positive 1-based start, supplied positive length, fully available, no padding |
| SLC-S2 | `substr(1234, 1, 1)` | 28 | source `"1234"`, start 1, length 1 | `"1"` | positive 1-based start, supplied positive length, fully available, no padding |

The current generated RXAS independently loads the two canonical source
strings. These exact operands are ASCII, so byte and codepoint positions
coincide, but a production fold must use the canonical Unicode/codepoint
implementation rather than byte slicing. Empty strings, omitted/default
length, zero length, end/out-of-range positions, padding, invalid padding,
numeric conversion errors and signals are unreachable in this cell and remain
owned by the complete Level B fallback in `lib/rxfnsb/rexx/substr.crexx` and
its RexxDoc/API companion.

SLC-S1 replaces the call result in the original compound assignment with
`"56"`; its downstream two numeric increments and compound-key order are
proved by a post-timer check for final value `"58"`. SLC-S2 removes a predicate
that is canonically false because `"1" <> "9"`. Post-timer calls through the
unaltered Level B implementation recheck both folded values on each VM. There
are no repeated or overlapping actuals, exposed reference results or
caller-owned source lifetime in either selected site.

The source control does not preserve the current imported body's exact
`.srcstep`, operand TRACE or source identity. That is an explicit proof gap,
not permission to weaken observability: production must reconstruct the
required call-site/body observation and signal order or decline the fold.
Optimized/no-opt equivalence and local/source-import/binary-import transport
outside these exact imported optimized sites remain production guard
requirements.

## Required candidate-panel disposition

| panel member | stable ID | result for the selected sites | placement implication |
|---|---|---|---|
| current clean Level B inline | SLC-C0 | Semantically complete; already inline, but retains validation/default/pad/length/branch/copy machinery | fallback and documentation, not the exact-site ceiling |
| hand-equivalent machine ceiling | SLC-H1 | Existing `SETSTRPOS`/`SUBSTRING` composition removes 1,148 timed instructions per outer iteration while retaining all 42 slices | dynamic/proven-range compiler composition control |
| best Level B algorithm | SLC-L1 | There is no runtime algorithm to beat for fully constant operands; literal substitution is the mathematical ceiling but is inappropriate as a benchmark-source production edit | use canonical compile-time evaluation, retain source body |
| compiler lowering/composition | SLC-CF1 | Fully proved constant evaluation removes another 140 timed instructions and all selected slices | recommended exact-site owner |
| general RXAS/VM assist | SLC-A1 | Existing primitives already reach the dynamic ceiling; no residual selected operation exists after CF1 | reject for this cell |
| native/intrinsic control | SLC-N1 | Cannot beat zero runtime SUBSTR work and would add crossing/maintenance cost | reject for this cell |
| placement | SLC-P1 | Compiler constant fold under proof and observability gates; otherwise current imported Level B inline/fallback | recommended, not installed |

## Artifact identity

- B0-R `rxc`: `900c2ba2229632c74da2a00cc313efa517beeb13ed8ab58f7e0e1afb41bad857`
- B0-R `rxas`: `80d3ff3e5b28e7132158c1b186513755457baf1472c2edd653874005a2648fc4`
- B0-R `rxvm`: `aab099d2f1e52f09976002935b21b189c104200f7a4b4155c65eef6eb21ac1d4`
- B0-R `rxbvm`: `a4a61df9cceac8a0178ef5583835953f55f882f7b4bab29c754d3c41aed87b5f`
- B0-R `library.rxbin`: `d4b35ddefa1b7d6711788b38dc33d0b66ff8a1af43690f2367c98a8ee5f7fcf1`
- SLC-CF1 scratch source: `932fca1e94440a8d65a2fa9e31711f4443eff2fe5962a2c4ed2a35250d82f0a7`
- SLC-CF1 patch: `c0d7b0ba19538330b8b176be88d0e38ff775fe26da5f92fbdc437fd9ff32be9f`
- SLC-CF1 RXAS: `59d36b6fe9b7e55b66cad6cc38d11c01ba39a7d44f61bc686d449367b53508de`
- SLC-CF1 RXBIN: `fd214bb36d8964578822355ab046c6aadeb4eaa89f3bca2bc9a1dfe971fd1206`
- SLC-CF1 disassembly: `5155cff7fc2aaf2a0d09919ca6dbcd9e3cda1b06f5e988751baec0aed6d03a73`

## Static and executed-machine comparison

The complete statement-region count below includes source-step and surrounding
assignment/compare instructions. It reconciles with the RHS-only SLC-H1 count
in `substr-direct-ceiling-summary.md`: each current region has three fixed
surrounding instructions, so both views predict the same H1 reduction.

| cell | total executable RXAS | main executable RXAS | main locals | RXAS bytes | RXBIN bytes | SLC-S1 static/executed | SLC-S2 static/executed |
|---|---:|---:|---:|---:|---:|---:|---:|
| SLC-C0 current inline | 1,498 | 573 | 105 | 220,731 | 77,438 | 49 / 32 | 49 / 32 |
| SLC-H1 direct composition | 1,449 | 524 | 109 | 214,819 | 76,046 | 6 / 6 | 5 / 4 |
| SLC-CF1 constant fold | 1,436 | 511 | 102 | 209,153 | 73,922 | 4 / 4 | 0 / 0 |

Against current, CF1 removes 62 whole-module executable RXAS instructions
(-4.138852%), 11,578 RXAS bytes (-5.245299%) and 3,516 RXBIN bytes
(-4.540407%), despite post-timer guards. Against H1 it removes 13 instructions,
5,666 RXAS bytes and 2,124 RXBIN bytes. The timed proof is exact:
`14 * (32 - 4) + 28 * (32 - 0) = 1,288` fewer instructions per outer
iteration than current, and 140 fewer than H1.

## Dual-VM correctness and B0-P attribution

Both ordinary profiling-off Release VMs exited 0, printed the canonical PASS,
emitted no stderr and passed the two canonical-result checks plus the
compound-result/order guard.

| VM/cell | normalized instructions | LOADSTR | SCOPY | SETSTRPOS | SUBSTRING |
|---|---:|---:|---:|---:|---:|
| rxvm current | 5,262.067143 | 438.315918 | 519.466327 | 70.254490 | 70.254490 |
| rxvm SLC-H1 | 4,112.959412 | 353.389853 | 476.776765 | 42.183235 | 70.183235 |
| rxvm SLC-CF1 | 3,972.971471 | 325.390294 | 476.776765 | 28.183676 | 28.183676 |
| rxbvm current | 5,263.193542 | 438.385000 | 519.517708 | 70.259792 | 70.259792 |
| rxbvm SLC-H1 | 4,114.144242 | 353.462424 | 476.831061 | 42.188939 | 70.188939 |
| rxbvm SLC-CF1 | 3,973.557313 | 325.426119 | 476.831061 | 28.186418 | 28.186418 |

CF1 reduces normalized retired instructions by 24.497895% on `rxvm` and
24.502922% on `rxbvm` versus current, and by 3.403582%/3.417161% versus H1.
The approximately 42-SUBSTRING decrement from H1 proves that every selected
runtime slice disappears. H1-to-CF1 string-copy and allocation counts remain
effectively unchanged; the incremental win is exact slice/instruction removal,
not an allocation claim. B0-P is attribution only.

## Profiling-off Release wall result

The maintained Level B matrix driver ran current, H1 and CF1 serially on each
B0-R VM with two warmups and seven recorded samples per cell. All 54 invocations
passed with zero stderr.

| VM | current median clauses/s | SLC-H1 | H1/current | SLC-CF1 | CF1/current | CF1/H1 |
|---|---:|---:|---:|---:|---:|---:|
| `rxvm` | 28,818,112 | 31,318,043 | +8.674860% | 32,145,168 | +11.545017% | +2.641049% |
| `rxbvm` | 27,540,875 | 29,437,133 | +6.885250% | 29,765,360 | +8.077031% | +1.115010% |

CF1 is fastest by median on both VMs. The incremental `rxbvm` result overlaps
H1 and is small, so the exact +1.115010% magnitude is not a formal production
claim. The cross-VM direction plus the strict 140-instruction machine-work
reduction selects CF1 as the bounded panel recommendation, subject to the
mandatory production first-verdict gate if Adrian authorizes implementation.

## Recommended owner and stop

Recommend a compiler-owned canonical constant fold for the two proved sites,
with current Level B SUBSTR retained unchanged as the full semantic fallback
and documentation. Use SLC-H1 only as the fallback compiler composition model
for a future separately proved dynamic/range cell. Do not add a public RXAS
opcode, private VM assist or native BIF for this evidence.

This is the PERF2-04 decision gate only. It does not install the fold, broaden
it to unproved SUBSTR semantics, run the formal portfolio, or authorize a
production slice.

## Retained paths

All relative paths are beneath
`performance/evidence/2026-07-25-perf2-04-bif-panel/`.

- patch: `pocs/substr-constant-fold-ceiling.patch`
- RXAS/disassembly and compiler/assembler logs:
  `pocs/substr-constant-fold-ceiling-artifacts/`
- dual-VM smoke: `measurements/substr-constant-fold-ceiling-smoke/`
- B0-P baseline/H1/CF1 counts:
  `measurements/substr-direct-ceiling-counts/{rxvm,rxbvm}/{baseline,direct,constant}/`
- serial B0-R wall matrix and state:
  `measurements/substr-rexxcps-timing/`
- wall control manifest: `controls/substr-rexxcps-poc-matrix-v1.txt`
