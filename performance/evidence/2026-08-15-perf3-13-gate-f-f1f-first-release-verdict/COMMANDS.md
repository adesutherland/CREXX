# F1f qualification commands

The retained logs are the command-output authority. Principal commands were
run from the repository root on `develop`.

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release -L gate_f \
  --output-on-failure --parallel 10

tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off
tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^(gate_f_.*|rxvmchannel_.*|rxvmbyteendpoint_substrate|program_generation_control-.*|testConcurrency_.*|address_.*|test_address_direct_.*|ts_address_.*|inline_nested_binary_block_owner_.*|rxvmworker_lifecycle|rxvmactive_isolation|persistent_worker_executor-.*)$'

cmake --build cmake-build-debug \
  --target test_gate_f_channel_benchmark_bin --parallel 10
cmake --build cmake-build-release \
  --target test_gate_f_channel_benchmark_bin --parallel 10

# Generate the format-compatible accepted image from the F1e source, then use
# the current sealed image for the F1f candidate. The evidence records hashes.
gf_image_dir=$(mktemp -d /tmp/crexx-f1f-images.XXXXXX)
git archive 3d9f484e2 \
  interpreter/tests/test_gate_f_channel_benchmark.rxas | tar -x -C "$gf_image_dir"
cmake-build-release/bin/rxas -o "$gf_image_dir/accepted-channel.rxbin" \
  "$gf_image_dir/interpreter/tests/test_gate_f_channel_benchmark"
cp cmake-build-release/interpreter/test_gate_f_channel_benchmark.rxbin \
  "$gf_image_dir/candidate-channel.rxbin"

GF_CONTROL_DIR=/tmp/crexx-f1f-control.h6JpKq \
GF_ACCEPTED_IMAGE=/tmp/crexx-f1f-images.63dREB/accepted-channel.rxbin \
GF_CANDIDATE_IMAGE=/tmp/crexx-f1f-images.63dREB/candidate-channel.rxbin \
performance/evidence/2026-08-15-perf3-13-gate-f-f1f-first-release-verdict/capture-channel-paired.zsh \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1f-first-release-verdict/timing/initial

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1f-first-release-verdict/timing/initial/summary.csv \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1f-first-release-verdict/timing/initial/samples.csv
```

The same frozen binaries, images, commands and 12-pair balanced ordering
produced `timing/confirmation` without changing the candidate or harness.
