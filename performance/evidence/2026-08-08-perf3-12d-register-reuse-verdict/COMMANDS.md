# PERF3-12D register-reuse command record

All commands ran from
`/Users/adrian/CLionProjects/CREXX-parse-planning-poc` on branch
`codex/peter-parse-planning-poc`, based on
`06fe132872b1ef51409cb071cb907da9f3714632` from `origin/develop`.
The worktree contained the uncommitted approved PERF3-12D implementation.

Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs. Low-power mode was off
for battery and AC profiles. Product builds used CMake 4.3.2, Apple clang
21.0.0, `CMAKE_BUILD_TYPE=Release`, and `CREXX_VM_PROFILING=OFF`.

## Controlled rerun requested on 2026-08-08

The rerun used the preserved RXC outputs from the two exact implementation
stages rather than recompiling them against a later exit library:

- `rexxcps-metadata.rxas`: optimized compiled PARSE after the initial fix and
  before internal-binding metadata suppression;
- `rexxcps-internal.rxas`: the same optimized compiled PARSE after generated
  bindings were classified as internal.

The executable products and inputs were copied to
`/private/tmp/crexx-parse-matrix.VUZVL3` before the first warmup.  The frozen
VM and library hashes are recorded in `README.md` and were rechecked after both
panels.

Five images were assembled:

```sh
rxas-old -o images/A-pre-meta_old-rxas input/pre-metadata.rxas
rxas-old -o images/B-meta-fixed_old-rxas input/metadata-fixed.rxas
rxas-new-link-direct -o images/C-pre-meta_new-link input/pre-metadata.rxas
rxas-new-link-direct -o images/D-meta-fixed_new-direct input/metadata-fixed.rxas
rxas-new-scopy-direct -o images/E-pre-meta_new-scopy input/pre-metadata.rxas
```

`rxas-old` is the preserved ordinary Release product with H02 disabled.
`rxas-new-link-direct` uses `link` when register metadata blocks redirection and
direct register reuse when no metadata observation remains.
`rxas-new-scopy-direct` is the matched `scopy` control.

`rxdas` verified four loads in A/B, one load plus three links in C, one load
with all four `strpos` consumers redirected to that register in D, and one load
plus three string copies in E.  A/B have identical executable instruction
streams after metadata records are excluded.

The main panel discarded two warmups per variant and VM.  Forty recorded
rounds used five cyclic orders followed by their five reverse orders, repeated
four times.  Each variant occupied every position eight times; VM order
alternated each round.  The execution command was:

```sh
FROZEN_VM FROZEN_LIBRARY.rxbin images/VARIANT.rxbin -a
```

The dedicated `link`/`scopy` panel discarded two warmups per form and VM, then
recorded 80 paired rounds per VM, alternating both form order and VM order.
No sample was removed or replaced.  Raw normalized native CPS, effective count,
calibration and pass fields are retained in:

- `raw/rxc-rxas-matrix-40.csv` (400 recorded processes plus 20 warmups);
- `raw/link-scopy-dedicated-80.csv` (320 recorded processes plus 8 warmups).

All 720 recorded processes reported the canonical-default contract,
`effective_count=500`, `calibrated=1`, and a RexxCPS pass.

## Compile and assemble

The S and L images used the same RXC-produced metadata-retaining RXAS. D was
recompiled after the exit/internal-binding metadata change.

```sh
cmake-build-release/bin/rxc -i cmake-build-release/bin \
  -o SCRATCH/rexxcps-VARIANT tests/benchmarks/rexxcps_levelb.crexx
RXAS-VARIANT -o SCRATCH/rexxcps-VARIANT SCRATCH/rexxcps-VARIANT
```

`RXAS-VARIANT` was a saved ordinary Release `rxas` binary for S and the
temporary diagnostic L edit; D and the final structural check used the current
ordinary Release product.

## Timing

The original N control and first H02 S verdict were recovered from their
retained canonical logs and normalized into
`raw/rxas-no-h02-control.csv` and
`raw/rxas-h02-scopy-first-verdict.csv`. Their source artifacts both contain the
same four PARSE sites; N has four `load *,"b"` operations and S has one load
plus three `scopy` operations. Each verdict discarded its warm-up and retained
eight runs per VM. They were consecutive verdicts, not a paired panel.

For each VM and panel, one warm-up per variant was discarded. Twelve recorded
rounds alternated `control,candidate` and `candidate,control` order:

```sh
cmake-build-release/bin/VM cmake-build-release/bin/library.rxbin \
  SCRATCH/rexxcps-VARIANT.rxbin -a
```

Panel 1 compared S with L. Panel 2 compared L with D. Raw native CPS and pass
fields are retained in `raw/*-scopy-link.csv` and `raw/*-link-direct.csv`.

The normalized N and first-S CSVs were checked against these retained sorted
sample sets:

```text
N rxtvm: 42958833 43008977 43074781 43236064 43504780 43712063 43870232 43888716
N rxbvm: 45019129 45364518 45711235 46123376 46258818 46307187 46695369 46864133
S rxtvm: 42030016 42217713 42266963 42356657 42477599 42749914 42890401 43252258
S rxbvm: 44471879 44553511 44718279 44824115 45139268 45139513 45210777 45635011
```

## Diagnostic counts

```sh
cmake-build-profile-parse/bin/rxtvm \
  --profile=counts --profile-output=COUNTS.csv \
  cmake-build-release/bin/library.rxbin \
  SCRATCH/rexxcps-VARIANT.rxbin -a
```

The profiling VM supplied counts only; all timing used the profiling-off
Release VMs.

## Focused qualification

```sh
cmake --build cmake-build-release --parallel 10 \
  --target rxas rxc exit_parse_bin test_parse_direct_bin \
           test_parse_compiled_patterns_bin

ctest --test-dir cmake-build-release --parallel 10 --output-on-failure -R \
  '^(nr14_frozen_parse_contract|rxas_optimizer_string_literal_reuse|rxas_optimizer_string_literal_reuse_noopt|test_parse_noopt|test_parse_opt|test_parse_direct_noopt|test_parse_direct_opt|test_parse_compiled_patterns_noopt|test_parse_compiled_patterns_opt)$'
```

CTest also ran the three required generated-binary fixtures, producing the
retained 12/12 result in `raw/test-final-focused.log`.

## Approved closeout QA

After Adrian selected `link` over `scopy` for the metadata-visible fallback,
the isolated Debug tree was configured and both build types were rebuilt with
the lifetime proof and negative cases:

```sh
cmake -S . -B cmake-build-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DCREXX_VM_PROFILING=OFF

cmake --build cmake-build-debug --parallel 10

cmake --build cmake-build-release --parallel 10 --target \
  rxas rxdas rxlink rxc rxtvm rxbvm test_rxas_flow_graph \
  test_rxop_metadata mutate_rxbin_feature exit_parse_bin test_parse_bin \
  test_parse_direct_bin test_parse_compiled_patterns_bin
```

The same 14-test affected matrix ran in Debug and Release:

```sh
ctest --test-dir BUILD --parallel 10 --output-on-failure -R \
  '^(nr14_frozen_parse_contract|test_parse_noopt|test_parse_opt|test_parse_bin_build|test_parse_direct_noopt|test_parse_direct_opt|test_parse_direct_bin_build|test_parse_compiled_patterns_noopt|test_parse_compiled_patterns_opt|test_parse_compiled_patterns_bin_build|rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_string_literal_reuse|rxas_optimizer_string_literal_reuse_noopt)$'
```

Full Debug qualification used the repository-prescribed parallelism:

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

It passed 2,000/2,000 tests in 216.15 seconds. The retained logs are:

- `qa-closeout/debug-build.log`;
- `qa-closeout/debug-focused.log`;
- `qa-closeout/debug-ctest.log`;
- `qa-closeout/release-build.log`; and
- `qa-closeout/release-focused.log`.

The final canonical structure check compiled, assembled and disassembled
`tests/benchmarks/rexxcps_levelb.crexx` with the ordinary profiling-off Release
tools.  The source RXAS hash reproduced the retained metadata-fixed input; the
disassembly contains one `load r68,"b"` and four `strpos` consumers of `r68`.

## Develop integration qualification

After merging the newer local and remote `develop` histories, the overlapping
queue-snapshot fixes were resolved to the existing inline/re-pin contract. The
exact optimized assembly reproducer and focused RXAS checks were:

```sh
cmake-build-debug/bin/rxas \
  -o cmake-build-debug/lib/rxfnsb/rexx/rxjson-repin-fixed \
  cmake-build-debug/lib/rxfnsb/rexx/rxjson.rxas

ctest --test-dir cmake-build-debug --parallel 10 \
  --output-on-failure -R \
  '^(rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_string_literal_reuse|rxas_optimizer_string_literal_reuse_noopt)$'
```

Both complete profiling-off build trees were rebuilt:

```sh
cmake --build cmake-build-debug --parallel 10
cmake --build cmake-build-release --parallel 10
```

The 20-test Release integration matrix added both RXQUEUE functional families
to the prior PARSE/RXAS set, and full merged Debug qualification used:

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

The retained integration logs are:

- `qa-integration/debug-rxas-focused.log`;
- `qa-integration/debug-build.log`;
- `qa-integration/release-build.log`;
- `qa-integration/release-focused.log`; and
- `qa-integration/debug-ctest.log`.
