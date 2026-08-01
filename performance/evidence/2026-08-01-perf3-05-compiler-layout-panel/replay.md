# PERF3-05 replay routes

Run from the repository root. The retained logs and capture manifests are the
exact command authority; this file identifies the shortest route without
pretending that old disposable `/tmp` products remain durable.

## Exact current baseline

1. Configure a new Release Ninja tree with `CREXX_VM_PROFILING=OFF` and
   `CMAKE_BUILD_TYPE=Release`.
2. Build the product with 10 jobs.
3. Recreate the five-workload matrix named in
   `baseline/baseline-five-workload-proof.log` and prove the expected output in
   both `rxvm` and `rxbvm`.
4. Hash binaries and record Mach-O `__text` and `run()` extents. Repeat in an
   independent build directory for the unchanged-source drift control.

The accepted uncommitted source input is frozen in
`baseline/accepted-source.diff`.

## Semantic ceiling

Use the retained `semantic-ceiling/perf3_05_base64_controls.rxbin`; rebuild it
from the adjacent RXAS only when testing assembler reproducibility. Run the six
cells in `semantic-ceiling/matrix-v1.txt` through
`performance/tools/run_cross_runtime_matrix.rxas` with two warmups and 12
recorded observations. This diagnostic must not replace the canonical Base64
workload.

## ThinLTO

Use the effective Apple-Clang ThinLTO flags recorded by `lto/ipo-proof.txt` for
both VM core object libraries and final links. Rebuild in a separate directory,
prove the five-workload matrix, then capture the 20 L0/LTO cells from
`lto/five-workload-matrix-v1.txt`. The retained formal pre/post state and
capture manifest define the exact run.

## PGO

Do not use only `CMAKE_C_FLAGS_RELEASE`; that retained route did not instrument
the VM object libraries. Apply `-fprofile-instr-generate` through global C
flags, run all ten training workload/VM cells in `pgo/training-matrix-v1.txt`,
merge the raw profiles, and rebuild with the profile-use flag recorded by
`pgo/use-proof.txt`. The raw profiles and merged `portfolio.profdata` are
retained. `rxvm.profdata` and `rxbvm.profdata` replay the separate-profile
pilot.

## L3 no-flatten

Apply `hotcold/noflatten.patch` only to a detached worktree. Configure a fresh
ordinary Release build, then use `hotcold/five-workload-matrix-v1.txt` for the
balanced timing screen. The `timing/`, `rss/` and `lifecycle/` manifests retain
the three independent comparisons. Do not apply this patch to production: it
is a rejected layout control.

## VM library linking

`link-diagnostic/cmake-link-interface.txt` contains the exact baseline CMake
lines and linker commands. Compile `link-diagnostic/minimal_rxvm.c` against
`inc/rxvm.h`, then link it against the archives in the recorded command with
Apple ld `-why_load`; `static-why-load.log` records the resulting object chain.
Relink `libcrexxsaa` with and without
`link-diagnostic/crexxsaa_exports.txt`, then repeat the downstream tool link
with the dylib alone and with CMake's expanded public archive list.
`link-timings.tsv` contains 15 monotonic wall-clock samples per form.

