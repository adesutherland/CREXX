# PERF2-04 RexxCPS WORD exact-use ceiling

This is a scratch-only decision package built from detached commit
`6567f0ba23f20623e01322f5a62323b2347ab09d`. It compares the current inlined
Level B `WORD` implementation, a safe predicate-first Level B algorithm, a
hand-equivalent exact-use composition, and the exact constant-result ceiling.
No production source was changed.

The result is not evidence for replacing general `WORD` with a native BIF or a
new public instruction. The selected timed expression is already inlined. Its
remaining cost is materializing a general `WORD` result before comparing that
result with a one-codepoint literal, even though the exact source and result
are provable at compile time.

## Exact sites and workload attribution

The canonical current product has two `WORD` sites:

| ID | source site | region | current form | exact current attribution |
|---|---|---|---|---|
| WORD-S0 | `system = word(version_info, 1)`, `tests/benchmarks/rexxcps_levelb.crexx:107` | setup/reporting, before all benchmark timers | residual imported call | one `rxfnsb.word` call in the fixed B0-P run; not a timed-kernel cause |
| WORD-S1 | `if word(key1, 1) = "?" then say "Failed6"`, line 166 | timed kernel | imported Level B body fully inlined by PERF2-03 | 28 evaluations per outer iteration; all cost is charged to `main`, not `rxfnsb.word` |

For WORD-S1, `key1` is assigned the literal `"Key Bee"` once per `lvar` and is
not changed across the two `j` iterations. There are 14 `lvar` iterations and
two `j` iterations, hence 28 timed evaluations per outer iteration. The fixed
B0-P control uses 100 iterations and averaging 100 without calibration, so the
exact denominator is 280,000 timed `WORD` evaluations. `wordnum` is the
constant positive integer 1, the comparison RHS is the literal `"?"`, both
operands are side-effect-free, and the predicate is always false.

The separate opaque diagnostic has a setup site at line 131 and a timed site at
line 186. Its A/B inputs use `"Key Bee"`/`"?"` and `"Key Dee"`/`"!"`. That
diagnostic is not part of the accepted current product profile and was neither
rewritten nor timed in this control.

## Semantic proof boundary

The complete contract remains in `lib/rxfnsb/rexx/word.crexx`, its RexxDoc/API
companion, and the focused `WORD` tests. The current implementation is the
complete fallback for Unicode/codepoint indexing, whitespace recognition,
positive 1-based `wordnum`, empty/not-found behavior, validation signals,
result ownership, and ordinary evaluation/TRACE behavior.

The scratch predicate-first helper was compared with `word(source, 1) = "?"`
over ten post-timer cases on both VMs:

- empty input;
- ASCII and Unicode blank-only input;
- `"Key Bee"`;
- the one-codepoint word `"?"`;
- `"? tail"` and leading/mixed-Unicode-blank forms;
- the multi-codepoint first word `"?x"`;
- a non-ASCII first word; and
- the fullwidth question mark, which must not equal ASCII `"?"`.

The helper performs a strict empty-string rejection, finds the first nonblank
codepoint, reads only that codepoint, rejects it unless it is U+003F, and only
then finds the first word boundary to prove the word has length one. The empty
check is required: forward `FNDNBLNK` reports not-found as negative length, so
empty input otherwise produces the ambiguous value zero. The hand control uses
the same predicate-first order directly at WORD-S1, but omits the empty check
only because the exact site proves `key1 == "Key Bee"` before every use.

## Candidate cells

| stable ID | purpose | timed WORD-S1 shape | production status |
|---|---|---|---|
| WORD-C0 | current clean source inline | general Level B `WORD` body, then loose string comparison | current product baseline |
| WORD-L1 | best safe Level B algorithm control | strict empty check; first-nonblank scan; first codepoint test; word-end proof only on U+003F | scratch-only semantic/algorithm control |
| WORD-H1 | hand-equivalent exact-use control | first-nonblank scan and first codepoint test; later work is unreachable for exact `"Key Bee"` | scratch-only machine ceiling |
| WORD-CF1 | absolute exact-site ceiling | compiler proof establishes the predicate is false; no runtime WORD-S1 path | scratch-only constant-result ceiling |

WORD-L1 and WORD-H1 use only existing semantic primitives. No public RXAS,
RXBIN, VM, ABI, or native surface was added.

## Tool and artifact identity

- B0-R configuration: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`
- B0-P configuration: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=ON`
- B0-R `rxc`: `900c2ba2229632c74da2a00cc313efa517beeb13ed8ab58f7e0e1afb41bad857`
- B0-R `rxas`: `80d3ff3e5b28e7132158c1b186513755457baf1472c2edd653874005a2648fc4`
- B0-R `rxvm`: `aab099d2f1e52f09976002935b21b189c104200f7a4b4155c65eef6eb21ac1d4`
- B0-R `rxbvm`: `a4a61df9cceac8a0178ef5583835953f55f882f7b4bab29c754d3c41aed87b5f`
- B0-P `rxvm`: `44705d0c9d39a659f80350fbefb7d3f51019a7cf1ae4dc72f453c88e7468da93`
- B0-P `rxbvm`: `9ae6721988a5605d29433129e9bcdad95f9d1c4ca9c366d5dcf237c892484795`
- `library.rxbin`: `d4b35ddefa1b7d6711788b38dc33d0b66ff8a1af43690f2367c98a8ee5f7fcf1`
- canonical optimized product RXBIN: 77,438 bytes,
  `9b535403baddc5b7076d5eb23027a296d982660546bcecc74d3784bb2fa23741`

The guarded scratch RXBIN identities are:

| cell | scratch source | patch | RXAS | RXBIN | disassembly |
|---|---|---|---|---|---|
| WORD-C0 | `3295d8af976b8cdf6ee6bc98dc08810f41b18a53ffc721334f9cb025bcf81d77` | `7fe47c85184bb57de554015c0dc9a59d2582b8a6f0228aed4818453fa7c87901` | `5c4f6fe07966e81ed37b900fe8f43522f6cea4e93586277f283b2856ffe87bad` | `be94a967aa5dad0b596bc33da7f5071a736601e489453edaf4b1dcc86c18d306` | `897e9b1332929ebe39da8913f3fd6bf1cd14ee35d24d01a499708b0626756457` |
| WORD-L1 | `7526a54d08f51d44c6e30e6184fcaed7bf6c4d437a5b6b3957b19bf0d8d05b5a` | `453d3617e640588e4a7517ef7bf8a6ec1faeb80005f78488e51811aa9460dcef` | `52422c6914877a2ac4009839aa075827c6fd28bfb15b97cd6d99c3dfd725cec4` | `59f21cbaec3f3ba2192a5dd195f6d921a0896d2f994a62cc1f04c4236cd73440` | `69d4ddb53d1bde236a60e564d9c0c1893b51afe8e0aa7e7df224d5ef1b4c3619` |
| WORD-H1 | `b485c75528056fec16dc80c2da872e0a26641d1989fbb55b7ffddc141846a10e` | `818cedb60e5782e2180f67ca07576d820add63340c0ba99d9d569bfcf53c9339` | `54afa5a910ccebbd1fc98237f3425cc14b9f15a3fba88485c52c5591f1da26fd` | `982f693cbe84ebe4491c5d133f67516e42fccf0729e01c21d9c7d745e51ab8b2` | `0ad28da229eabd184da059e65f8a1b5976393d0b2fd505a0318da4f70658594d` |
| WORD-CF1 | `1d13ab98a791d299cc3c58010221d3deb8c0dc24267179a7500adabb8d5f25e6` | `686ac9a202925993893362ca4cbf01b83a322153cb27048fa6727941838d219e` | `4bb832ce71547a3c4e8afba170f677e66769b023121dabeb7025ca976ba99eed` | `4b0e81a81227ae293e39196971337aca91e9b85b45c74cac863cedd9fb1b42b5` | `39f7ad398073c26a4b180c5cd079c721c5312e4965d15aed91c3bc34b8d3a5ef` |

## Static and exact hot-path comparison

All four cells contain the same post-timer semantic guard. The guarded current
cell is therefore the static/count comparator; its larger image is not the
77,438-byte canonical product image.

| cell | total executable RXAS | main executable RXAS | main locals | RXAS bytes | RXBIN bytes | WORD-S1 static / executed | WORD-S1 code bytes |
|---|---:|---:|---:|---:|---:|---:|---:|
| WORD-C0 | 2,254 | 541 | 105 | 363,990 | 118,058 | 46 / 29 | 142 |
| WORD-L1 | 2,238 | 525 | 105 | 363,261 | 117,682 | 30 / 14 | 99 |
| WORD-H1 | 2,228 | 515 | 110 | 361,844 | 117,802 | 13 / 6 | 46 |
| WORD-CF1 | 2,208 | 495 | 105 | 357,336 | 116,242 | 0 / 0 | 0 |

The current exact path performs length, first-nonblank and word-end scans,
positioning, substring materialization, one three-byte result copy, and the
final loose comparison. WORD-L1 removes 15 executed instructions per timed
evaluation while preserving peak locals and the surrounding generated code.
It removes one `FNDBLNK`, timed `STRLEN`, `SETSTRPOS`, `SUBSTRING`, result
`SCOPY`, and loose `REQ` per evaluation, replacing them with a strict empty
test and `STRCHAR`. String-buffer and standalone-value allocation counts are
unchanged.

WORD-H1 reduces the exact block from 29 to six executed instructions: on
`"Key Bee"`, the first codepoint is `K`, so it never performs the word-end
scan. Its source-level predeclarations raise peak main locals from 105 to 110
and perturb an unrelated compound-concatenation path. The exact block removes
23 instructions, but whole-image B0-P counts improve by only 18.5 instructions
per timed evaluation after that unrelated extra work. It is therefore a valid
exact-use ceiling, not the proposed production source rewrite.

WORD-CF1 removes all 29 executed WORD-S1 instructions per timed evaluation.

## Dual-VM correctness smoke

Each of the four cells ran on the B0-R `rxvm` and `rxbvm` with
`--smoke-count 100`. All eight executions exited zero, emitted empty stderr,
printed `PERF2-04-WORD-GUARD PASS`, printed
`PASS: RexxCPS 2.2d cREXX port`, and emitted no failure or panic marker. The
adaptive smoke count was 300 except for one WORD-CF1 `rxvm` run at 400; these
smoke timings are not used for a performance claim.

## Normalized B0-P attribution

The profiling builds are used only for deterministic attribution. Every cell
ran both VMs at fixed count 100, averaging 100, with calibration disabled.

| VM | cell | total instructions | per outer iteration | delta vs WORD-C0 | delta per timed WORD | delta percent |
|---|---|---:|---:|---:|---:|---:|
| `rxvm` | WORD-C0 | 52,244,121 | 5,224.4121 | 0 | 0.000000 | 0.000000% |
| `rxvm` | WORD-L1 | 48,044,093 | 4,804.4093 | -4,200,028 | -15.000100 | -8.039236% |
| `rxvm` | WORD-H1 | 47,064,122 | 4,706.4122 | -5,179,999 | -18.499996 | -9.914989% |
| `rxvm` | WORD-CF1 | 44,124,091 | 4,412.4091 | -8,120,030 | -29.000107 | -15.542476% |
| `rxbvm` | WORD-C0 | 52,244,121 | 5,224.4121 | 0 | 0.000000 | 0.000000% |
| `rxbvm` | WORD-L1 | 48,044,154 | 4,804.4154 | -4,199,967 | -14.999882 | -8.039119% |
| `rxbvm` | WORD-H1 | 47,064,122 | 4,706.4122 | -5,179,999 | -18.499996 | -9.914989% |
| `rxbvm` | WORD-CF1 | 44,124,093 | 4,412.4093 | -8,120,028 | -29.000100 | -15.542472% |

WORD-L1 removes about 280,000 timed `FNDBLNK`, `STRLEN`, `SETSTRPOS`,
`SUBSTRING`, `SCOPY`, and loose-`REQ` executions and adds about 280,000
`STRCHAR` plus strict-empty comparisons. String-to-string copy operations fall
by about 280,000 and copied bytes by about 840,000, exactly one three-byte
materialized result per timed WORD. Standalone values remain exactly 550,039
and string buffers exactly 140,023 in all cells. The retained normalized TSV
contains the exact raw opcode, copy, byte, and allocation counters.

## Ordinary profiling-off Release wall matrix

The maintained Level B matrix driver ran the exact canonical product WORD-C0
RXBIN plus WORD-H1 and WORD-CF1 scratch RXBINs, serially rotated by workload,
on both ordinary profiling-off Release VMs. The run used two warmups and seven
recorded samples per cell. All 54 executions exited zero and passed workload
correctness; every H1/CF1 warmup and recorded execution also passed the
post-timer semantic guard. The host remained on AC power at 80% battery with no
reported thermal or performance warning. Capture ran from 16:26:25Z to
16:27:44Z.

The comparison metric is the benchmark-native canonical CPS calculated from
the benchmark's internal timed kernel. Raw process elapsed is retained but is
not compared because calibration can select different effective counts.

| VM | WORD-C0 median CPS | WORD-H1 median CPS | H1 vs C0 | WORD-CF1 median CPS | CF1 vs C0 | CF1 vs H1 |
|---|---:|---:|---:|---:|---:|---:|
| `rxvm` | 29,310,271 | 30,257,667 | +3.232300% | 31,277,784 | +6.712708% | +3.371433% |
| `rxbvm` | 27,801,847 | 28,724,104 | +3.317251% | 29,696,325 | +6.814216% | +3.384687% |

Recorded spans were 1.141611% to 4.067810%; relative MAD was 0.256138% to
1.044964%. The maintained driver recommended no rerun for any cell.

## Observability, ownership, and transport gaps

- The current inline retains `.srcstep` records from `word.crexx` plus the
  caller site. WORD-L1/H1/CF1 source rewrites do not reproduce the original
  body steps, call-site event order, or source identity. TRACE mode and no-opt
  were not exercised. A production fold or fusion must synthesize the correct
  observability or fail closed to the ordinary call/fallback.
- `FNDNBLNK`, `FNDBLNK`, and `STRCHAR` are Unicode/codepoint operations. The
  VM's Unicode scan/`STRCHAR` paths can update a string cursor. The current
  inline operates on a private formal temporary. WORD-H1 acts directly on
  caller `key1`; its exact ASCII/start-zero use leaves the same observed
  cursor, but a general leading-Unicode-blank case could expose mutation. A
  production composition needs a private ephemeral value, proof that avoids
  observable mutation, restoration when sufficient, or a narrowly justified
  nonmutating private assist.
- This exact `wordnum == 1` proof cannot produce `INVALID_ARGUMENTS`. Any
  generalized transform must preserve source, index, and RHS evaluation order,
  signal identity/order, alias lifetime, and result ownership.
- The measured canonical timed site arrives through an imported body summary.
  Local, source-import, binary-import, no-opt, and contradictory-proof forms
  were not generalized or regression-tested by this scratch control.
- Only one real canonical timed site is selected. The opaque form is a
  diagnostic, not independent adoption evidence. A general consumer fusion
  still needs proof at multiple real sites, or a compiler fold whose general
  correctness and profitability do not depend on this benchmark.

## Placement recommendation

1. For exact WORD-S1, the most efficient correct owner is **WORD-CF1,
   compiler-owned constant/proof folding**. It is the mathematical and machine
   ceiling: zero runtime scans, copies, allocations, or comparison work, with a
   measured +6.712708% `rxvm` and +6.814216% `rxbvm` canonical CPS effect. It
   must use canonical Unicode/WORD semantics and preserve evaluation, signals,
   source identity, and TRACE behavior.
2. For nonconstant-source `word(source, 1) = "?"` consumers, or a broader
   compile-time RHS class whose loose-comparison equivalence is separately
   proved, the next owner to investigate is **compiler-owned predicate
   fusion/composition** using the existing scan/codepoint primitives in private
   ephemeral registers. WORD-H1 proves the end-to-end opportunity
   (+3.232300%/+3.317251%) while WORD-L1 supplies the safer algorithm shape.
   The production compiler form should not inherit WORD-H1's five persistent
   source locals or unrelated codegen perturbation.
3. Keep `lib/rxfnsb/rexx/word.crexx` unchanged as the complete, maintainable
   fallback and documentation for materializing general `WORD` results.
4. Do not add a native owner, public RXAS opcode, serialized RXBIN change, ABI
   change, or VM assist from this evidence. Existing composition reaches the
   selected use's hand ceiling, and the constant fold is stronger.

This supplies bounded PERF2-03-F03 reopen evidence for formal/result/block-exit
and temporary cleanup at a proved hot BIF consumer. It does not supply an I6
fact and does not reopen PERF2-03-F05.

## Retained paths

All relative paths below are under
`performance/evidence/2026-07-25-perf2-04-bif-panel/`:

- scratch patches: `pocs/word-current-baseline-guard.patch`,
  `pocs/word-levelb-predicate-first.patch`,
  `pocs/word-direct-predicate-first-ceiling.patch`, and
  `pocs/word-constant-false-ceiling.patch`;
- RXAS/RXBIN/disassembly and compile logs:
  `pocs/word-ceiling-artifacts/{current,helper,direct,constant}/`;
- dual-VM smokes:
  `measurements/word-ceiling-smoke/{rxvm,rxbvm}/{current,helper,direct,constant}/`;
- B0-P raw profiles:
  `measurements/word-ceiling-counts/{rxvm,rxbvm}/{current,helper,direct,constant}/`;
- exact normalized counts: `measurements/word-ceiling-normalized-counts.tsv`;
- ordinary Release matrix manifest:
  `controls/word-rexxcps-poc-matrix-v1.txt`;
- retained ordinary Release raw evidence:
  `measurements/word-rexxcps-timing/`; and
- closure manifest: `measurements/word-ceiling-checksums.sha256`.

The four detached source worktrees remain under
`/private/tmp/crexx-perf2-04.3k3Dfw/word-{baseline-src,ceiling-src,predicate-first-src,constant-false-src}`.
