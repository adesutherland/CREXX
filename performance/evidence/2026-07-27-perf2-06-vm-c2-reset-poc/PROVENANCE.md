# Provenance

## Source

- Accepted/base commit: `ab9eef54576388121cdb02e285bd99981e68d8b3`
  (`evidence: retain PERF2-06 sieve outputs`).
- Accepted VM-C1b implementation commit:
  `a39608426e2c1bb84d5fc0c4f767f4c9492339a9`.
- Exact R0 source: detached worktree at the base commit.
- R1/R2 source: dirty isolated branch
  `codex/perf2-06-vm-c2-reset-poc`, based on the same commit.
- The candidate patch changes private VM/CMake/profiler code and live
  performance documentation only. It changes no public RXAS, RXBIN, ABI or
  language surface.
- The earlier rejected C2-A/C2-B worktree was not modified.

## Host and toolchain

- Host: Apple M5, 10 logical CPUs.
- OS: Darwin 25.5.0, arm64.
- Compiler: Apple clang 21.0.0 (`clang-2100.1.1.101`).
- CMake: 4.3.2.
- Ninja: 1.13.2.
- Generator: Ninja.
- Formal capture remained on AC power with low-power mode 0 and no recorded
  thermal or performance warning. Raw pre/post/final state is under `timing/`.

## Products and inputs

- R0 Release: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`, exact base
  source.
- R1 Release: the same, plus `RXVM_VM_C2_RESET_MODE=1` and
  `RXVM_VM_C2_RESET_DIAGNOSTICS=OFF`.
- R2 Release: the same, plus `RXVM_VM_C2_RESET_MODE=2` and diagnostics off.
- Each product built independent `rxvm` and `rxbvm` VM cores.
- All 24 timing cells used R0-built optimized Permute, List, Base64 and Sieve
  RXBINs plus the same R0-built `library.rxbin`; their hashes are retained.
- Lifecycle is process-startup-inclusive. Source/TRACE metadata was retained
  identically in the common inputs.

The R0-built `crexx` target did not include its generated
`classlib_native` image, so an initial launcher attempt failed before creating
any timing sample. The formal runs used the pre-existing ordinary
profiling-off `crexx` driver at
`/Users/adrian/CLionProjects/CREXX/cmake-build-release/bin/crexx` solely to
execute the Level B matrix runner. Its embedded version reports
`crexx-1.0.0-beta.3+local.ge9ecd880364f.dirty` with build date 20260726; its
exact hash is retained. It is not a product under comparison and does not run
inside any timed child process. The six measured VM binaries and all
workload/library inputs remained the frozen products above.

## Measurement

- Runner: checked-in Level B
  `performance/tools/run_cross_runtime_matrix.crexx`.
- One balanced 24-cell manifest: four workloads x two VM modes x three
  variants.
- Initial capture: one warmup and 12 recorded rotated rounds.
- Noise extension: 10 recorded rotated rounds, no warmup.
- Final interval/guard extension: 12 recorded rotated rounds, no warmup.
- Final paired count: 34 per workload/VM/variant comparison.
- Samples were serial within each capture. No sample or outlier was removed.
- Base64 remains beyond the formal raw-cell noise thresholds at the cap and is
  reported as such. The absolute-cell gate also flags Sieve R0 `rxvm` and R1
  `rxbvm`; the paired R1 `rxbvm` interval remains wholly adverse and is
  reported with that caveat.

## Diagnostics and correctness

- Machine/reset counters used independent Debug products with diagnostics on
  and the exact formal work counts.
- Deterministic profile identity used independent Debug products with
  `CREXX_VM_PROFILING=ON`, diagnostics off and profile mode `counts`.
- The copied R0 count CSVs are the retained exact-base profiles from the
  immediately preceding VM-C2 PoC. The optimized benchmark RXBIN hashes match
  the current R0 inputs; full selected operation rows compare exactly.
- Focused correctness and exact reset counters used the same independent Debug
  R1/R2 products with reset diagnostics on and VM profiling off. Each passed
  78 frame/reference/argument, 57 signal/instrumentation and 8
  lifecycle/late-load tests. Diagnostics were off in all profile and Release
  timing products.

Raw samples, outputs, count profiles, diagnostic rows, CTest logs, Mach-O load
commands, symbol maps and file-size records are retained in this directory.
