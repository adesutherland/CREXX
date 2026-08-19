# POSTPERF-05 closeout validation

Date: 2026-08-18

Status: **complete**.

## Final product identity

The final ordinary Release cache remained `CMAKE_BUILD_TYPE=Release` with
`CREXX_VM_PROFILING=OFF`. After the closeout NR-26 fixed-point correction, the
compiler and complete `linked_opt_runtime_artifacts` target were rebuilt.

All 17 frozen H1 files match byte-for-byte:

- optimized RXAS and RXBIN for DeltaBlue, CD, Richards, Havlak, JSON, List,
  Sieve and RexxCPS: 16/16 match;
- compiler-produced `library.rxbin`: 1/1 match.

This proves that the closeout correctness repair did not change any accepted
timing/static product. `release-artifact-identity.log` is the retained exact
comparison.

## Correctness and build results

| Check | Result | Retained log |
| --- | --- | --- |
| Combined post-defect focused Debug contracts | 36/36 pass, 6.76 s | `postflow-focused-debug-ctest.log` |
| Full ordinary Debug build | pass | `closeout-debug-build.log` |
| Full Debug CTest, `--parallel 30` | 2,251/2,251 pass, 298.29 s | `closeout-debug-ctest.log` |
| Full ordinary Release build, `--parallel 10` | pass | `closeout-release-build.log` |
| Focused Release compiler/shape/runtime/benchmark set | 56/56 pass, 46.50 s | `closeout-release-ctest.log` |

The focused set includes both concrete VMs for the optimized and no-opt HTTP
policy scenarios. It also includes the new structural assertion that scenario
2's `read_request` consumes the register produced by the reaching second
`sockaccept`.

## Closeout defects and contract transitions

Two independent compiler defects have focused regression coverage:

1. retained-call binary and decimal constants now materialise into a register
   before `ret`, while RXAS-supported boolean/integer/float/string constants
   retain their direct control-operand path;
2. NR-26 fixed-point copy processing preserves a deeper read substitution when
   it is the source of a reaching skipped copy definition, preventing an old
   physical target equality from reclaiming the use on the next pass.

The selected profitability policy intentionally retains ordinary calls at
large losing sites. Existing structural tests were therefore adapted to prove
the supported call fallback, receiver/result lifetime and decimal integrity
rather than requiring the former expansion everywhere. All corresponding
runtime/golden results pass in the full Debug suite.

No syntax, opcode, RXBIN format, ABI or VM semantic change is included.
