# Replay commands

Representative final configurations:

```sh
cmake -S . -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_HANDLER_PANEL=profile-30

cmake -S . -B "$GCC_BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/opt/homebrew/opt/gcc/bin/gcc-16 \
  -DCREXX_ENABLE_TLS=OFF -DCREXX_VM_HANDLER_PANEL=profile-30

cmake --build "$BUILD" --parallel 2
ctest --test-dir "$BUILD" --parallel 30 --output-on-failure
```

The all-inline controls use the same commands with
`-DCREXX_VM_HANDLER_PANEL=all-inline`.

The retained Level B runner command was:

```sh
caffeinate -i /private/tmp/crexx-perf3-05-r3.IvLL06/gcc-pre/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest /private/tmp/crexx-perf3-05-r3.IvLL06/r3-final-verdict-manifest.txt \
  --output-dir /private/tmp/crexx-perf3-05-r3.IvLL06/r3-final2-timing \
  --measurement timing --warmups 2 --runs 12
```

Before and after the run, SHA-256 covered both edited C sources, the manifest
and all eight rebuilt Clang/GCC all-inline/profile-30 VM binaries. The two hash
files are identical.
