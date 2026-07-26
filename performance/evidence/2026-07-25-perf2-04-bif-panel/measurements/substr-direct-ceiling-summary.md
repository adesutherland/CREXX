# PERF2-04 scratch-only RexxCPS SUBSTR hand ceiling

This is a non-production control built from detached commit
`6567f0ba23f20623e01322f5a62323b2347ab09d` in
`/private/tmp/crexx-perf2-04.3k3Dfw/substr-direct-src`. The exact B0-R
profiling-off Release compiler and assembler produced the control. No
production source was changed and no formal wall-clock comparison was run.

## Exact timed sites and proof boundary

The live source has exactly two `SUBSTR` sites, both inside the timed kernel:

| ID | source expression | executions per outer iteration | evaluated source | public slice | exact result | proved case |
|---|---|---:|---|---|---|---|
| SLC-S1 | `substr(1234 "5678", 6, 2)` | 14 | `"1234 5678"`, 9 ASCII/codepoints | start 6, length 2 -> zero cursor 5 | `"56"` | positive start, positive supplied length, fully available, no padding |
| SLC-S2 | `substr(1234, 1, 1)` | 28 | `"1234"`, 4 ASCII/codepoints | start 1, length 1 -> zero cursor 0 | `"1"` | positive start, positive supplied length, fully available, no padding |

The B0-R compiler output itself proves the evaluated source values by loading
`"1234 5678"` and `"1234"`. ASCII means byte and codepoint positions coincide
for these exact operands, while the retained primitives still implement
codepoint cursors. Neither site can reach the empty, out-of-range, omitted
length, zero-length, padding, or validation-signal paths.

The complete fallback contract remains in `lib/rxfnsb/rexx/substr.crexx` and
`substr.md`: positive 1-based start, optional non-negative exact length,
one-codepoint padding, Unicode/codepoint slicing, `INVALID_ARGUMENTS`, and a
read-only source. `tests_functional/tsubstr.crexx` covers Unicode, embedded
U+0000, empty/boundary/padding, invalid arguments, and source nonmutation.

## Control shape

SLC-S1 evaluates its already-folded source into a private register, executes
`SETSTRPOS source,5`, then `SUBSTRING result,source,2`. SLC-S2 evaluates its
source into a private register and executes only `SUBSTRING result,source,1`.
This is valid because `LOAD_REG_STRING` calls `set_const_string`, which resets
both the byte and codepoint cursors to zero. `SUBSTRING` copies into separate
result storage and does not advance the source cursor.

Post-timer assertions prove results `"56"`/`"1"`, unchanged source text, and
source cursors 5/0. Splitting SLC-S1 moves the implementation's compound-LHS
key calculation after the slice, but `key1` and `lvar` are stable and
side-effect-free across those statements, so the exact workload values and
observable result are preserved. A production compiler lowering must preserve
the language's original evaluation order rather than rely on that workload
fact.

## B0-R and artifact identity

- `rxc`: `900c2ba2229632c74da2a00cc313efa517beeb13ed8ab58f7e0e1afb41bad857`
- `rxas`: `80d3ff3e5b28e7132158c1b186513755457baf1472c2edd653874005a2648fc4`
- `rxvm`: `aab099d2f1e52f09976002935b21b189c104200f7a4b4155c65eef6eb21ac1d4`
- `rxbvm`: `a4a61df9cceac8a0178ef5583835953f55f882f7b4bab29c754d3c41aed87b5f`
- `library.rxbin`: `d4b35ddefa1b7d6711788b38dc33d0b66ff8a1af43690f2367c98a8ee5f7fcf1`
- B0-R configuration: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`
- scratch source: `830e47a9052f06fbc1e27ab2fc440130431527542792244be76755f23997be76`
- patch: `ddf081cc457edd4e1a7f31787cc6b572746445dc297e349b976e1688f44a0dec`
- RXAS: `963debd84ddc3bc9ec6cac61baf68d6b35b08eb9fa31c047dadfee0bdcb3c1ae`
- RXBIN: `a6f9d6c9fe3092b57be22eb636f929b7e2298e1eff01fb116c92502314937fe6`
- RXBIN disassembly: `0643553f54590b8e852ffe6169434115ac6ac87d430c5b67059f8ef0826c1f16`

## Static and hot-path comparison

Both current `SUBSTR` bodies are already inlined: neither artifact contains a
`rxfnsb.substr` call, and the total call-prefix count remains 27.

| cell | total executable RXAS | main executable RXAS | main locals | RXAS bytes | RXBIN bytes | SLC-S1 static / executed | SLC-S2 static / executed |
|---|---:|---:|---:|---:|---:|---:|---:|
| B0-R current inline | 1,498 | 573 | 105 | 220,731 | 77,438 | 46 / 29 | 46 / 30 |
| direct existing-primitives control | 1,449 | 524 | 109 | 214,819 | 76,046 | 3 / 3 | 2 / 2 |

The whole scratch module is 49 executable RXAS instructions, 5,912 RXAS bytes,
and 1,392 RXBIN bytes smaller despite its predeclarations and post-timer
guards. The exact timed paths remove
`14 * (29 - 3) + 28 * (30 - 2) = 1,148` instructions per outer iteration.
All 42 required slices remain; the removed work is default/pad setup,
validation, length/availability arithmetic, branch scaffolding, and final
result copies. SLC-S2 also proves that an explicit zero-position operation is
unnecessary after the source load.

## Dual-VM profiling-off smoke

Both ordinary Release VMs exited 0, emitted no stderr, passed all post-timer
guards, and printed `PASS: RexxCPS 2.2d cREXX port`.

- `rxvm`: effective smoke count 320
- `rxbvm`: effective smoke count 300

The adaptive diagnostic rates in those logs are not a formal timing verdict.

## Normalized B0-P counts

Counts use the B0-P profiling runtime only for attribution. The baseline and
control images are B0-R products. The denominator is
`(initial_count + effective_count) * 100`, covering both measured trials:
4,900/6,800 baseline/control iterations for `rxvm` and 4,800/6,600 for
`rxbvm`.

| VM/cell | instructions | LOADINT | LOADSTR | ICOPY | SCOPY | ISUB | IGT | IEQ | INE | BR | BRF | STRLEN | SETSTRPOS | SUBSTRING |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| rxvm current | 5,262.067143 | 412.297755 | 438.315918 | 29.627551 | 519.466327 | 140.461020 | 58.566531 | 128.312041 | 43.939796 | 169.214286 | 639.773673 | 143.287959 | 70.254490 | 70.254490 |
| rxvm direct | 4,112.959412 | 243.097353 | 353.389853 | 1.172794 | 476.776765 | 56.331765 | 16.128676 | 57.666029 | 1.397941 | 126.595294 | 427.322059 | 58.368971 | 42.183235 | 70.183235 |
| rxbvm current | 5,263.193542 | 412.387292 | 438.385000 | 29.661458 | 519.517708 | 140.470625 | 58.599167 | 128.360208 | 43.980208 | 169.260417 | 639.956458 | 143.356458 | 70.259792 | 70.259792 |
| rxbvm direct | 4,114.144242 | 243.191515 | 353.462424 | 1.208333 | 476.831061 | 56.342273 | 16.163030 | 57.716515 | 1.440455 | 126.643939 | 427.514242 | 58.441061 | 42.188939 | 70.188939 |

Normalized retired instructions fall by 1,149.107731 (-21.837573%) on `rxvm`
and 1,149.049299 (-21.831789%) on `rxbvm`, within about 1.11 instructions of
the exact 1,148 hot-path prediction after fixed startup and guard normalization.
`SUBSTRING` is unchanged within fixed-run noise. `SETSTRPOS` falls by about 28,
matching removal of the redundant start-1 position operation. `STRLEN` falls
by about 84.9, and conditional/unconditional branches fall by about 255 per
iteration.

Normalized string-to-string copy operations fall from 478.342449 to 365.687500
(`rxvm`) and 478.391250 to 365.739091 (`rxbvm`), with about 259 copied bytes per
iteration removed. Standalone-value and string-buffer allocation counts are
effectively unchanged (about 55.01 and 14.00 per iteration), so this ceiling is
primarily instruction/scan/copy removal rather than an allocation-count win.

## Semantic gaps and placement implication

- The current inline preserves `substr.crexx` body `.srcstep` records and
  operand trace events but has no residual `SUBSTR` function-call event. The
  scratch source instead exposes separate benchmark assignment/assembler
  steps. It is therefore not TRACE/source equivalent. A production compiler
  lowering or constant fold must synthesize the required original call-site
  identity/body stepping contract, or fail closed when that observability
  cannot be preserved.
- Mutating the cursor is safe here only because both sources are private
  literal temporaries. A general lowering of a caller-owned source must not
  expose cursor mutation: it needs a proven private temporary/copy, restoration
  if that is semantically sufficient, or a narrow non-mutating explicit-start
  assist. This control does not justify a public RXAS opcode.
- Both complete expressions are compile-time constants. A companion
  `SLC-CF1` compiler-owned constant fold to `"56"`/`"1"` is the absolute
  exact-site ceiling and would remove the remaining 42 runtime slices. It is
  plausibly stronger than this direct-composition control, but it must use the
  canonical Unicode implementation, preserve validation/evaluation and TRACE
  semantics, and was not installed or timed here.

For these RexxCPS sites, the efficient owner is compiler proof/composition (or
the stronger exact constant fold), with the complete Level B implementation as
fallback and semantic documentation. The evidence does not support native
ownership or a general/public opcode.

## Retained paths

- scratch worktree: `/private/tmp/crexx-perf2-04.3k3Dfw/substr-direct-src`
- retained patch: `pocs/substr-direct-ceiling.patch`
- retained RXAS/RXBIN/disassembly and compile logs:
  `pocs/substr-direct-ceiling-artifacts/`
- retained smoke: `measurements/substr-direct-ceiling-smoke/{rxvm,rxbvm}/`
- retained B0-P counts:
  `measurements/substr-direct-ceiling-counts/{rxvm,rxbvm}/{baseline,direct}/`

All relative retained paths are beneath
`performance/evidence/2026-07-25-perf2-04-bif-panel/`.
