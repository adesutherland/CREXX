# PERF3-11 legacy-proof baseline commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`.

## Freeze the accepted assembler

```sh
cp cmake-build-release/bin/rxas BASELINE_DIR/rxas-stage6
shasum -a 256 BASELINE_DIR/rxas-stage6
git rev-parse HEAD
```

## Representative diagnostics

All verbose assembler diagnostics were redirected to temporary logs.

```sh
cmake-build-release/bin/rxas -d -o OUTPUT \
  cmake-build-release/tests/benchmarks/benchmark_awfy_richards_opt \
  >STDOUT_LOG 2>STDERR_LOG

cmake-build-release/bin/rxas -d -o OUTPUT \
  cmake-build-release/tests/benchmarks/benchmark_awfy_towers_opt \
  >STDOUT_LOG 2>STDERR_LOG

cmake-build-release/bin/rxas -d -o OUTPUT \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt \
  >STDOUT_LOG 2>STDERR_LOG
```

`^NR27 accept`, `^NR27 flow` and `^PERF3 legacy-rule` records supplied the
retained decision tables.  RXBIN hashes include the input module/description
path; comparisons used the same exact input path and filename.

## Focused legacy oracles

```sh
for fixture in whole_procedure_flow whole_procedure_panel nr18_flow_harvest
do
  cmake-build-release/bin/rxas -d -o OUTPUT \
    tests/rxas_optimizer/$fixture >STDOUT_LOG 2>STDERR_LOG
done

ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure \
  -R '^rxas_optimizer_'
```

The focused matrix passed 49/49.

## Diagnostic implementation checks

```sh
/usr/bin/cc \
  -Icmake-build-debug/generated -Iassembler -Icmake-build-debug/assembler \
  -Iplatform -Iavl_tree -Iutf8 -Ibinutils/include \
  -std=gnu90 -Wall -Wextra -Wconversion -Wsign-conversion \
  -fsyntax-only assembler/rxas_opt.c

cmake --build cmake-build-debug --target rxas --parallel 10
cmake --build cmake-build-release --target rxas --parallel 10
ctest --test-dir cmake-build-debug --output-on-failure \
  -R '^rxas_optimizer_copy_acopy_opt$'
```

Existing aggregate-initializer warnings remain present; the strict syntax
command and both builds returned zero.
