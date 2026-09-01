# Replay commands

Representative Clang configuration:

```sh
cmake -S . -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_HANDLER_PANEL=profile-20
cmake --build "$BUILD" --target rxtvm rxbvm --parallel 2
```

Real GCC uses:

```sh
cmake -S . -B "$GCC_BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/opt/homebrew/opt/gcc/bin/gcc-16 \
  -DCREXX_ENABLE_TLS=OFF -DCREXX_VM_HANDLER_PANEL=profile-20
cmake --build "$GCC_BUILD" --target rxtvm rxbvm --parallel 2
```

The pilot and formal matrices use the unchanged Level B runner:

```sh
caffeinate -i "$CONTROL_BUILD/bin/crexx" \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest "$MANIFEST" --output-dir "$OUTPUT" \
  --measurement timing --warmups 2 --runs 12
```

Pilot invocations use `--warmups 1 --runs 4`. Inputs and `library.rxbin` are
frozen from the accepted R3 control. Every manifest retains the absolute paths
used on the measurement host.

The focused suite is:

```sh
ctest --test-dir "$BUILD" --parallel 1 --output-on-failure -R \
'^(linked_opt_runtime_artifacts_build|dynamic_interface_load_driver|rxvmworker_lifecycle|test_signal_mask|rxassignalstests|rxvminstrumentedsignaltests|rxbvminstrumentedsignaltests|rxvminstrumentedbreakpointtests|rxbvminstrumentedbreakpointtests|rxasdispatchcontract-rxbvm|rxasdispatchcontract-rxtvm|reentrancy_check|ts_loadmodule_noopt|ts_loadmodule_opt)$'
```

For the all-inline expansion proof, the R3 compile commands were changed from
`-c` to `-E -P`, once against a `git archive` of starting commit
`3ad9890c139c29e2df44c72d720bec127c3eb65e` and once against R5. Absolute
source roots were normalized to `/SOURCE`, then `cmp` passed. SHA-256 was:

```text
rxbvm 8780e80c37f12ea72149e727e86bf2dd3b0bf006ae3ce02cc23d946b4462e16a
rxtvm 5f9fb6f8afeeadf4c6c20390b5c9be70170c98c1804c9e3f04573d9ccdaf4a1b
```

Derived comparison tables use `control / candidate` for elapsed time and
`candidate / control` for RexxCPS `benchmark_rate`. This direction distinction
is mandatory when replaying the aggregate calculations.
