# Closeout commands

All commands ran from `/Users/adrian/CLionProjects/CREXX`. Verbose output was
redirected to `logs/`; the command forms below omit only that redirection.

```sh
cmake --build cmake-build-debug \
  --target rxc rxas rxlink rxvm rxbvm linked_opt_runtime_artifacts
ctest --test-dir cmake-build-debug --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|perf2_07_v3_representation|inline_local_member_scalar_opt|inline_receiver_nested_this_direct_placement_opt|inline_receiver_reference_alias_opt|inline_test_computed_receiver_copyback_opt_rewrites|inline_test_computed_receiver_copyback_run_(noopt|opt)|inline_test_mutating_method_scalar_attr_return_run_(noopt|opt))$'
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|perf2_07_v3_representation|inline_local_member_scalar_opt|inline_receiver_nested_this_direct_placement_opt|inline_receiver_reference_alias_opt|inline_test_computed_receiver_copyback_opt_rewrites|inline_test_computed_receiver_copyback_run_(noopt|opt)|inline_test_mutating_method_scalar_attr_return_run_(noopt|opt))$'
ctest --test-dir cmake-build-release --parallel 30 --output-on-failure
```

Apple's sanitizer runtime aborted the first runner build because
`detect_leaks=1` is unsupported. The retained successful ASan-only rerun was:

```sh
tools/asan-run.sh --phase build --build-leaks off \
  --build-target rxc --build-target rxas --build-target rxlink \
  --build-target rxvm --build-target rxbvm \
  --build-target linked_opt_runtime_artifacts
tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --fixture-exclude-setup linked_opt_runtime_artifacts \
  --regex '^(linked_opt_runtime_artifacts_build|perf2_07_v3_representation|inline_local_member_scalar_opt|inline_receiver_nested_this_direct_placement_opt|inline_receiver_reference_alias_opt|inline_test_computed_receiver_copyback_opt_rewrites|inline_test_computed_receiver_copyback_run_(noopt|opt)|inline_test_mutating_method_scalar_attr_return_run_(noopt|opt))$'
```

Lifecycle used the maintained Level B tool twice, with `--build-dir` set first
to retained current product `/private/tmp/crexx-perf2-0607.HOrlKC/build-v3-fixed`
and then to the accepted `cmake-build-release`; each received five initial and
five appended observations:

```sh
cmake-build-release/bin/crexx performance/tools/run_lifecycle.crexx \
  --nokeep --args --runtime crexx --crexx-vm both --runs 5 \
  --repo-root /Users/adrian/CLionProjects/CREXX \
  --build-dir BUILD --output-dir OUTPUT
cmake-build-release/bin/crexx performance/tools/run_lifecycle.crexx \
  --nokeep --args --runtime crexx --crexx-vm both --runs 5 --append \
  --repo-root /Users/adrian/CLionProjects/CREXX \
  --build-dir BUILD --output-dir OUTPUT
```

RSS reused the exact accepted first-verdict manifest and retained products:

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-27-perf2-06-07-v1r01-r1-first-release-verdict/manifests/first-verdict.txt \
  --output-dir performance/evidence/2026-07-27-perf2-06-07-v1r01-r1-closeout/rss \
  --measurement rss --warmups 0 --runs 3
```

The configured Ninja graph has no `package` target. The supported isolated
install check was:

```sh
install_root=$(mktemp -d /tmp/crexx-perf2-0607-install.XXXXXX)
cmake --install cmake-build-release --prefix "$install_root"
"$install_root/bin/rxvm" \
  cmake-build-release/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  "$install_root/bin/library.rxbin" -a 1
"$install_root/bin/rxbvm" \
  cmake-build-release/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  "$install_root/bin/library.rxbin" -a 1
```
