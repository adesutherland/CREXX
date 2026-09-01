# Provenance

## Repository and host

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- Source HEAD: `e38e514bf611ae3873513368c44742e2ae7332d1`
- Upstream: `origin/develop` at
  `21fdcf529d0e51ea264bf0c92ccfbdc06dea8200`
- Worktree: existing PERF3-02/C1b documentation and evidence plus this PoC;
  five pre-existing untracked lifecycle RXBIN files remained untouched.
- Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs.
- CMake: 4.3.2; Ninja: 1.13.2; Apple Clang: 21.0.0
- Analysis mode: Debug `rxas -d`; diagnostic only, no timing claim.
- Debug `rxas` SHA-256:
  `3c09d8236b29d1989bbeb3784509394004ff9bf118b8e84249a254b1a552960c`.

## Inputs

- Richards current compiler RXAS:
  `cmake-build-debug/tests/benchmarks/benchmark_awfy_richards_opt.rxas`,
  SHA-256
  `ec61aa8cb044312675502c0ce2a46d4f131e0640bae6188d5db007bfb8731673`.
- Towers current compiler RXAS:
  `cmake-build-debug/tests/benchmarks/benchmark_awfy_towers_opt.rxas`,
  SHA-256
  `e78bc406672af9c3fc7f9434ad9a4790d9fd12fe0df2852d05696fd2fffcadba`.
- Focused analysis fixture:
  `tests/rxas_optimizer/storage_identity_flow.rxas`, SHA-256
  `81f14ab80b614fe287a2690d4c70d17882a013c85808872e34b7db8e6bf4d0fb`.
- Runtime fixture: `tests/rxas_optimizer/storage_identity_runtime.rxas`,
  SHA-256
  `9529de36b0bc5628849ebb12bda0aec76bad2270875d24f9f90960ddc487beb5`.
- CRI-13 retained trace:
  `performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-R1-RXAS-TRACE.md`,
  SHA-256
  `d76e7f70de6b78be5d4f85b64fad48d3e0393c526110f464f9973b55e356e4f4`.

The exact frozen CRI-13 source named by that trace was under its historical
temporary evidence root and is not present now. No current-live replay is
claimed for it.

## Representative assembly commands

The same source and output stem were used for each debug/ordinary pair:

```sh
cmake-build-debug/bin/rxas -d \
  -o /tmp/crexx-c2e2-replay.I0kCT3/richards-compiler-debug \
  cmake-build-debug/tests/benchmarks/benchmark_awfy_richards_opt.rxas
cmake-build-debug/bin/rxas \
  -o /tmp/crexx-c2e2-replay.I0kCT3/richards-compiler-normal \
  cmake-build-debug/tests/benchmarks/benchmark_awfy_richards_opt.rxas

cmake-build-debug/bin/rxas -d \
  -o /tmp/crexx-c2e2-replay.I0kCT3/towers-compiler-debug \
  cmake-build-debug/tests/benchmarks/benchmark_awfy_towers_opt.rxas
cmake-build-debug/bin/rxas \
  -o /tmp/crexx-c2e2-replay.I0kCT3/towers-compiler-normal \
  cmake-build-debug/tests/benchmarks/benchmark_awfy_towers_opt.rxas
```

Debug mode emits verbose assembly diagnostics to stdout; only the compact
`NR27 identity` stderr summaries are retained here. Ordinary/debug outputs are
byte-identical within each same-input pair.

## Validation commands and outcomes

```sh
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(rxas_optimizer_|storage_identity_runtime_)'
```

Result: 52/52 passed. This includes 51 optimizer-labelled tests and four
optimized/no-opt runtime cells across `rxvm` and `rxbvm`; labels overlap.
The required shared linked-runtime artifact fixture also passed.

```sh
cmake --build cmake-build-release --target rxas --parallel 10
```

Result: passed, including compilation of `assembler/rxas_flow.c` and linking
of ordinary Release `rxas`.

## Bound and interpretation

The diagnostic analysis retains one `size_t` storage identity per reachable
program point and register slot and skips a procedure above 2,000,000 cells.
On this 64-bit host the primary matrix is therefore bounded at approximately
16 MB, with separate linear queue/bitmap/scratch overhead. The must lattice is
monotone: an exact identity can degrade to unknown but cannot later be guessed
back into existence at a join. This PoC records coverage and equivalence, not
compile-time cost or end-to-end speedup.
