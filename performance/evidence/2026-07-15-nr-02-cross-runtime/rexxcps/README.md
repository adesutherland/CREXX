# RexxCPS NR-02 qualification slice

Status: CREXX, ooRexx, Regina and NetRexx pilot cells retained

These are qualification pilots, not an NR-10 formal baseline and not a
cross-runtime aggregate.

## Provenance and version resolution

| Source | Role | SHA-256 / provenance |
| --- | --- | --- |
| `tests/benchmarks/cross-runtime/rexxcps/rexxcps_2_2.rex` | canonical Classic RexxCPS 2.2 | byte-for-byte official ooRexx sample; `b86b1232a3747bacdeba64da16eb79cb0e0115bc3d91da3c2b0b08a80772c8f4` |
| `tests/benchmarks/cross-runtime/rexxcps/rexxcps_2_1n.nrx` | canonical bundled NetRexx example | byte-for-byte NetRexx 5.10-GA example; `9aa47a25f9aff0085ad8a2600fbf8785b772347b8c9b29427050ae85d93e6dbd` |
| `tests/benchmarks/cross-runtime/rexxcps/rexxcps_2_2n.nrx` | disclosed NetRexx adaptation | preserves the bundled 2.1n timed kernel and adds 2.2 minimum-duration calibration; diff retained as `netrexx-2.1n-to-2.2n.diff` |
| `tests/benchmarks/rexxcps_levelb.crexx` | disclosed cREXX Level B 2.2c port | equivalence/substitution audit remains in the dated programme report and benchmark README |
| files containing `_opaque` | diagnostics only | opaque runtime variant A/B, deterministic state observation and trace controls; diffs retained here |

The official NetRexx 5.10-GA distribution therefore does **not** solve the
2.1n-versus-2.2 issue by itself: it still bundles 2.1n. NR-02 retains that
source unchanged and uses separately named 2.2n for the current-duration
protocol. Neither 2.2n nor 2.2c is relabelled as unchanged Classic 2.2.

RexxCPS uses the original source redistribution terms and CPL 1.0 retained in
`tests/benchmarks/`; the NetRexx distribution also carries its ICU `LICENSE`.

## Timed-kernel and observation ledger

| Runtime/source | Timed-kernel relationship | Observable state / optimizer resistance | Inspection |
| --- | --- | --- | --- |
| Regina / canonical 2.2 | unchanged official source and nominal 1,000-clause unit | canonical rate plus separate A/B challenge; A/B both finish `1|69|1.22694` | count-one challenge `TRACE I`: 948 source records, 2,642 intermediate records, 280 parse-assignment records, 3,870 kernel trace lines |
| NetRexx / 2.2n | exact bundled 2.1n NetRexx kernel; 2.2-style self-calibration outside the kernel | separate A/B opaque challenge; both finish `1|69|1.22694`; runtime argument and final state feed generated Java branches/output | generated Java, Java 8 class files and complete `javap -c -p` retained for 2.1n, 2.2n and opaque |
| CREXX / 2.2c | disclosed Level B substitutions from the existing report | separate A/B opaque challenge; both finish `1|69|1.22694`; focused opt/no-opt tests pass | `TRACE I` count-one attempt retained as a partial negative result and terminated while still in the kernel; build has `CREXX_VM_PROFILING=OFF` |
| ooRexx / canonical 2.2 | unchanged official source and nominal 1,000-clause unit | canonical rate plus separate A/B challenge; A/B both finish `1|69|1.22694` | executable `rexxc` images retained; count-one challenge `TRACE I`: 990 source and 4,012 intermediate trace records inside the timed kernel |

Opaque A/B values preserve the intended taken/not-taken paths while changing
the runtime-selected key, digit prefix, false comparison values and selected
text. They add variable lookup and observation work, so their rates are
diagnostics rather than canonical scores.

### Exact opaque A/B difference

A and B run the same timed clauses, loop counts, call structure and final
correctness checks. The variant is selected from the process argument before
timing; the selected values are then read inside the kernel so a compiler or
JIT cannot safely specialize the run to one source literal pattern.

| Runtime-selected field | Variant A | Variant B | Preserved behavior |
| --- | --- | --- | --- |
| Compound-stem key | `Key Bee` | `Key Dee` | Different stem tail; same reads/writes and final value 69 |
| Digit text | `12345678` | `92345678` | Different first digit; `substr(...,6,2)` remains 67 |
| False string comparison | `foobar` | `foobaz` | `j=value` remains false |
| False numeric comparison | `9` | `8` | First digit comparison remains false |
| False word comparison | `?` | `!` | First word comparison remains false |
| Parsed text | `Foo Bar` | `Zoo Bar` | Same parse shape; different observed text |
| Subroutine words | `with`, `args` | `WITH`, `ARGS` | Same case-insensitive call/parse behavior |

Both variants deliberately keep `opaquezero=0`, `opaqueone=1` and marker `b`,
and both must finish with `variant|1|69|1.22694`. A is not a baseline and B is
not an “optimization disabled” mode: they are paired perturbations. A material
canonical/A/B difference is a reason for investigation, not a score to average
into the canonical result.

## Pilot results

All samples are serial, process-startup-inclusive and metadata/runtime-default
mode. Each Regina/NetRexx series used one process warmup; CREXX used one
recorded full-default run per variant because each run took about 18 seconds.
Raw output, elapsed nanoseconds, exact argv and benchmark-native metrics are in
each `pilot/` directory.

| Runtime / source | Recorded n | Native CPS range | Arithmetic mean |
| --- | ---: | ---: | ---: |
| Regina 2.2 canonical | 3 | 22,336,702–23,424,690 | 22,853,783 |
| Regina 2.2 opaque A | 2 | 16,111,543–21,269,159 | 18,690,351 |
| Regina 2.2 opaque B | 2 | 20,691,953–20,745,267 | 20,718,610 |
| NetRexx 2.2n | 3 | 34,359,379–40,607,000 | 38,038,641 |
| NetRexx 2.2n opaque A | 2 | 37,055,346–38,950,437 | 38,002,892 |
| NetRexx 2.2n opaque B | 2 | 22,325,786–33,814,955 | 28,070,370 |
| CREXX 2.2c | 1 | 527,607 | 527,607 |
| CREXX 2.2c opaque A | 1 | 535,462 | 535,462 |
| CREXX 2.2c opaque B | 1 | 553,429 | 553,429 |
| ooRexx 2.2 canonical | 3 | 12,100,824–21,058,410 | 16,986,023 |
| ooRexx 2.2 opaque A | 2 | 18,379,188–20,173,302 | 19,276,245 |
| ooRexx 2.2 opaque B | 2 | 15,972,927–21,296,609 | 18,634,768 |

The low Regina A and NetRexx B observations are retained, not discarded. The
NetRexx A mean matches 2.2n within 0.1%, while B is noisy; CREXX A/B are within
about 5% of its single canonical pilot. This rules out no specific partial
optimization, but it provides no evidence of wholesale kernel deletion: all
variants execute runtime-dependent paths and preserve the observed state.

The CREXX rates are materially below the earlier seed observations (mean
845,834 CPS). Host/source identity is the same, but the current run environment
was slower across these full-default processes. Do not combine the two bundles
or infer a regression without a controlled NR-10 rerun.

The unchanged NetRexx 2.1n exploratory run reported 24,099,006 CPS but only
about 0.00415 seconds for its internally averaged timed region. It is retained
under `netrexx-2.1n/` as historical/version evidence and excluded from the
current pilot comparison.

The ooRexx count-one `TRACE I` diagnostic retained 990 source records and 4,012
intermediate records inside the explicit kernel markers. Intermediate record
types include 858 variable reads, 522 assignments, 505 operator evaluations,
194 compound-name evaluations and 127 function calls. These are trace-event
counts, not VM opcode counts. The binary `rexxc` translations were executed as
a separate encoded-image proof and are not mixed with the source pilots.

## Commands and runtime settings

The cell manifests retain exact argv. NetRexx compilation used:

```sh
PATH=/Users/adrian/.local/opt/netrexx/5.10-GA/bin:$PATH \
  nrc -nocolor -keepasjava SOURCE.nrx
```

Execution used JDK 26.0.1 mixed mode with default command-line flags recorded
in `netrexx-2.1n/exploratory-pilot.txt`; the JVM reported G1GC, compressed oops
and a segmented code cache. NetRexx emitted class-file version 52.0 (Java 8).
No explicit `-Xint`, tier threshold, heap, GC or JIT override was applied.

ooRexx execution used the absolute user-local 5.1.0 r12973 interpreter path:

```sh
/Users/adrian/.local/opt/oorexx/5.1.0-12973/bin/rexx SOURCE.rex [ARGS]
```

Homebrew's `/opt/homebrew/bin/rexx` remains Regina. ooRexx source pilots used
the runtime's default metadata mode; separately retained `rexxc` encoded images
prove successful translation/execution but are not benchmarked as the same
lifecycle.

## Open qualification work

- the RexxCPS four-runtime NR-02 slice is qualified; transfer controlled
  repeated same-host samples to NR-10 without treating these pilots as a ratio;
- replace the partial CREXX trace attempt with bounded VM profiling or another
  non-interactive dynamic-count path if NR-02 review requires it;
- native-C ceiling controls were not selected for this slice and remain an
  optional labelled control, never a RexxCPS score; and
- promote only controlled repeated same-host samples to NR-10.
