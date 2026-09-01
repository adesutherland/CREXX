# RCC-5C caller-relative decimal precision: first Release verdict

Status: accepted by Adrian on 2026-08-20 and proportionally qualified. The
independent pre-existing RXAS SSA sanitizer failure still prevents a clean
repository-wide Apple-ASan build.

## Change and boundary

The control is detached commit `298f412dc0e40ef12b4957df4b5f8b57a8a14d9f`,
the completed fixed-tier RCC-5C implementation. The candidate is the named
dirty working-tree change to `rxdecimal` and its tests/docs. It keeps the
18/32/64/96 work tiers through caller digits 64 and uses caller precision plus
32 guard digits above that point. `pi` and `euler` use precision-aware
per-mutable-module caches. The public surface is qualified through caller
digits 128; this is an assurance boundary, not a runtime ceiling.

No language syntax, opcode, RXBIN layout, provider ABI, public namespace, or
procedure signature changes.

## Host and build

- Apple M5 MacBook Air, 10 logical CPUs, Darwin 25.5.0 arm64;
- Apple Clang 21.0.0, CMake 4.3.2, Ninja 1.13.2;
- AC power, with no recorded thermal, performance, or CPU-power warning;
- ordinary `Release`, `CREXX_VM_PROFILING=OFF`; and
- serial balanced/interleaved sampling, one warmup and 12 initial pairs.

The control and candidate benchmark outputs match for every recorded process.
No sample was removed.

## Accepted performance verdict

The four steady 100,000-iteration cells are clearly favorable, with paired
mean elapsed changes from -1.314789% to -0.553696%. The four one-iteration
cold-start cells reached the 36-pair ceiling and remain noisy/inconclusive.
Their medians are -0.174966% to +0.046527%, and their paired means are
-2.071495% to +1.819533%. No cell hits the 3% workload guard.

`paired-summary.csv` is the final combined decision table. The initial and two
unchanged-condition append directories retain every raw sample and output.

## Post-acceptance qualification

- complete Debug build and 2,302/2,302 CTests passed in 192.95 seconds;
- after the final cache down-round test-only addition, focused Debug passed
  5/5 and complete profiling-off Release passed 2,302/2,302 in 78.37 seconds;
- the full Apple-ASan build reproduced the already-recorded unrelated
  `rxas_flow_value_node()` heap-use-after-free while assembling optimized AWFY
  Towers, before CTest could start;
- a targeted Apple-ASan decimal build passed, followed by all 5/5 decimal
  surface and optimized/no-opt concrete-VM cells; Apple leak detection remains
  unsupported and was disabled as required by repository policy; and
- a fresh 169-file scratch install ran the 128-digit bytecode smoke and built
  and ran an arm64 native executable from the same source.

The installed driver uses the documented `-lrxfnsg` application-library
boundary for both bytecode and native packaging. No native provider/plugin list
is supplied; the decimal core is selected by the installed product packaging.

The sanitizer blocker is not caused by this change: the same optimized Towers
reproducer and `rxas_flow_ssa.c:3422` signature were retained during RCC-5B.
It remains a separately scoped RXAS correction rather than being hidden as a
decimal failure.
