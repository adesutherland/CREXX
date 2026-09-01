# PERF2-06/07 successor hardware handover

## Frozen input and order

Transfer the local closeout commit that contains this file and require a clean
tree before building. Record `git rev-parse HEAD`, its parent, branch/upstream,
the recursive `SHA256SUMS` verification and compiler/CMake/Ninja versions in
each target bundle. The Apple source base was
`b08611179db5ff4257c3be3103f3aeab55ea5b50`; the accepted compiler/runtime and
test file hashes are in `source-product-workload.sha256`.

The required successor order remains:

1. PERF2-08 equivalence/capability gate on the Mac;
2. PERF2-09 closure, bounded PERF2-10 Apple controls and PERF2-11 Apple
   pre-handover scorecard;
3. Intel x86-64 Linux GCC and Clang decision-critical tuning/counters;
4. supported Linux ARM64 validation; and
5. supported Windows on the same physical Intel x86-64 machine.

Do not infer cross-platform superiority from this Apple result. Do not reopen
C2-A/B, C2R01, exact reset lists, reset-needed/quickened clearing, C3R01,
changed-only numeric synchronization or cleanup-only flattened-interpreter
reshaping. C2R03 and V6R01 still require Adrian's architecture decision.

## Linux build and correctness

Use independent clean products; never share mutable artifacts between
compilers or variants:

```sh
handover_root=$(mktemp -d /tmp/crexx-perf2-0607-handover.XXXXXX)
git worktree add --detach "$handover_root/source" CLOSEOUT_COMMIT

cmake -S "$handover_root/source" -B "$handover_root/gcc-release" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake -S "$handover_root/source" -B "$handover_root/clang-release" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build "$handover_root/gcc-release" --parallel "$(nproc)"
cmake --build "$handover_root/clang-release" --parallel "$(nproc)"
ctest --test-dir "$handover_root/gcc-release" --parallel "$(nproc)" --output-on-failure
ctest --test-dir "$handover_root/clang-release" --parallel "$(nproc)" --output-on-failure
```

Before the broad suites, run the exact 10-test regex in `COMMANDS.md` against
each product. Build a profiling/count product separately with
`-DCREXX_VM_PROFILING=ON`; never time it as the ordinary product.

## Linux timing, RSS and native counters

Recreate the accepted first-verdict manifest with target-local absolute paths
and freshly verified hashes. Preserve the six workloads, both VMs, current and
accepted linked products, arguments, expectations, warmups and 12 balanced
pairs. Use the maintained Level B matrix for timing and a separate zero-warmup,
three-observation RSS capture. Do not copy the absolute Apple paths from the
retained manifest.

For Sieve, Permute, Richards and Bounce at minimum, retain both VM modes under
GCC and Clang:

```sh
perf stat -r 3 \
  -e cycles,instructions,branches,branch-misses,cache-references,cache-misses \
  -- VM IMAGE LIBRARY -a WORK
perf record -o perf.data -- VM IMAGE LIBRARY -a WORK
perf report --stdio -i perf.data
```

Add supported L1 instruction-cache and iTLB events reported by `perf list`.
Record unavailable events rather than inventing substitutes. Retain product
hashes, ELF text size, `run()` disassembly/symbol extent, top helper/caller
stacks and exact operation counts. Intel Linux is the principal tuning and
selection environment.

`PERF2-06-D01` is a separate exact-parent-versus-accepted VM-C1b comparison,
not a reason to modify V1R01-R1. Reopen D01 when a cell regresses by more than
3%, a compiler reverses the result, or code-layout variance reverses the
accepted portfolio gain. Do not select a final stream/default until Apple,
Linux x86-64, Linux ARM64 and Windows evidence reconciles.

## Linux ARM64

Repeat the clean profiling-off Release, focused/broad correctness, both-VM
timing, RSS, artifact and available native-counter lanes on supported Linux
ARM64. Record actual GCC/Clang availability and versions. Apple ARM64 does not
satisfy this Gate E lane; an unavailable compiler or counter is an explicit
blocker, not permission to substitute Apple evidence.

## Supported Windows on the same Intel host

Use the repository-supported MSYS2 GCC/Ninja environment and the Linux-selected
frozen source:

```sh
cmake -S . -B cmake-build-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF
cmake --build cmake-build-release --parallel
ctest --test-dir cmake-build-release --parallel 10 --output-on-failure
cmake --install cmake-build-release --prefix /tmp/crexx-perf2-0607-install
```

Run the same paired timing cells and both VMs. Retain PE/image and RXBIN sizes,
the isolated install smoke and peak working set using a trusted Windows process
measurement. If supported tooling cannot provide hardware counters or a
reliable peak working set, record the exact blocker; do not publish a zero or
invented value. A Windows correctness or regression-guard failure reopens the
cross-platform decision and forbids a Windows-only shortcut.

## Universal stop rules

- zero correctness failures;
- stop for Adrian on any language, public RXAS/RXBIN, ABI or architecture
  decision;
- stop for an adverse common aggregate worse than 1% or comparable Tier A cell
  worse than 3%;
- stop when a named lifecycle phase regresses by both more than 5% and 1 ms;
- stop when peak RSS regresses by both more than 5% and 1 MiB;
- stop when an artifact regresses by both more than 5% and 4 KiB;
- use same-session balanced controls before attributing drift; and
- do not proceed to a final VM/default or external-superiority claim before
  Gates E and F are satisfied on the full required platform matrix.
