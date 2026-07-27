# Reproduction commands

The retained manifests intentionally contain the executed absolute scratch
paths. A replay may replace paths only; keep variants, work counts, lifecycle
and common input hashes unchanged.

The retained implementation patch is sufficient to reconstruct the candidate
source without depending on the dirty PoC worktree:

```sh
evidence_root=/private/tmp/crexx-perf2-06-reset.0ozeyr/worktree/performance/evidence/2026-07-27-perf2-06-vm-c2-reset-poc
replay_root=$(mktemp -d)
r0_src="$replay_root/r0-src"
candidate_src="$replay_root/candidate-src"

git worktree add --detach "$r0_src" \
  ab9eef54576388121cdb02e285bd99981e68d8b3
git worktree add --detach "$candidate_src" \
  ab9eef54576388121cdb02e285bd99981e68d8b3
git -C "$candidate_src" apply "$evidence_root/poc/vm-c2-reset-poc.patch"
```

## Ordinary Release products

```sh
r0_release="$replay_root/r0-release"
r1_release="$replay_root/r1-release"
r2_release="$replay_root/r2-release"

cmake -S "$r0_src" -B "$r0_release" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$r0_release" \
  --target rxvm rxbvm crexx language_benchmarks --parallel 10

cmake -S "$candidate_src" -B "$r1_release" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF -DRXVM_VM_C2_RESET_MODE=1 \
  -DRXVM_VM_C2_RESET_DIAGNOSTICS=OFF \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$r1_release" \
  --target rxvm rxbvm --parallel 10

cmake -S "$candidate_src" -B "$r2_release" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF -DRXVM_VM_C2_RESET_MODE=2 \
  -DRXVM_VM_C2_RESET_DIAGNOSTICS=OFF \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$r2_release" \
  --target rxvm rxbvm --parallel 10
```

## Focused correctness

The same commands ran once in each independent diagnostics-on, profiling-off
R1/R2 Debug build. The first selected CTest fixture built the required linked
runtime artifacts. Set `mode=1` and then `mode=2`:

```sh
mode=1
check_build="$replay_root/r${mode}-debug"
cmake -S "$candidate_src" -B "$check_build" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF \
  -DRXVM_VM_C2_RESET_MODE="$mode" \
  -DRXVM_VM_C2_RESET_DIAGNOSTICS=ON

frame_ref_arg_re='^(rxas(stress|interface|instructiongap|nr09superinstruction|reference(catch)?|perf2(referenceguard|partialreference|exactrelink))tests(-rxbvm)?|nr21_fixed_call_contract|repro_(array_arg|args_alias|duplicate_call_argument)_run_(noopt|opt)|arg_semantics_(scalar|array|object)_run_(noopt|opt)|optional_arg_presence_copy_run_(noopt|opt)|nr06_call_window_scalar_run_(noopt|opt)|inline_test_(ref_vararg|ref_vararg_runtime|vararg|vararg_dynamic|vararg_dynamic_loop|multi_return_call|object_writable_arg|binary_arg_return)_run_(noopt|opt)|attr_array_call_result_lifetime_run_(noopt|opt)|reference_source_(explicit|contract|iterator|dereference_live|inline_lifetime|block_lifetime)_(rxvm|rxbvm)_(noopt|opt))$'
signal_trace_re='^(test_signal_mask|rxassignal.*|rxasbreakpoint.*|rxvminstrumented(signal|breakpoint)tests|rxbvminstrumented(signal|breakpoint)tests)$'
lifecycle_re='^(reentrancy_check|dynamic_load|dynamic_interface_load_(rxvm|rxbvm)|dynamic_interface_load_driver|ts_loadmodule_(noopt|opt))$'

ctest --test-dir "$check_build" --parallel 10 --output-on-failure \
  -R "$frame_ref_arg_re"
ctest --test-dir "$check_build" --parallel 10 --output-on-failure \
  -R "$signal_trace_re"
ctest --test-dir "$check_build" --parallel 10 --output-on-failure \
  -R "$lifecycle_re"
```

## Formal timing

```sh
runner=/Users/adrian/CLionProjects/CREXX/cmake-build-release/bin/crexx
manifest=performance/evidence/2026-07-27-perf2-06-vm-c2-reset-poc/timing/manifest.txt

caffeinate -i "$runner" performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args --manifest "$manifest" \
  --output-dir performance/evidence/2026-07-27-perf2-06-vm-c2-reset-poc/timing/formal-12 \
  --measurement timing --warmups 1 --runs 12

caffeinate -i "$runner" performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args --manifest "$manifest" \
  --output-dir performance/evidence/2026-07-27-perf2-06-vm-c2-reset-poc/timing/extension-10 \
  --measurement timing --warmups 0 --runs 10

caffeinate -i "$runner" performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args --manifest "$manifest" \
  --output-dir performance/evidence/2026-07-27-perf2-06-vm-c2-reset-poc/timing/extension-12 \
  --measurement timing --warmups 0 --runs 12
```

The intermediate 22-pair and final 34-pair absolute summaries, candidate
filters and paired summaries were generated exactly as follows:

```sh
timing_root="$evidence_root/timing"

"$runner" performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest "$manifest" --output-dir "$timing_root/formal-22-summary" \
  --measurement timing --summary-only \
  --samples "$timing_root/formal-12/samples.csv" \
  --samples "$timing_root/extension-10/samples.csv"

"$runner" performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest "$manifest" --output-dir "$timing_root/formal-34-summary" \
  --measurement timing --summary-only \
  --samples "$timing_root/formal-12/samples.csv" \
  --samples "$timing_root/extension-10/samples.csv" \
  --samples "$timing_root/extension-12/samples.csv"

for capture in formal-12 extension-10 extension-12; do
  "$runner" "$evidence_root/tools/filter_vm_c2_pairs.crexx" \
    --nokeep --args "$timing_root/$capture/samples.csv" \
    "$timing_root/$capture/r1-paired-samples.csv" fixed-core-copy
  "$runner" "$evidence_root/tools/filter_vm_c2_pairs.crexx" \
    --nokeep --args "$timing_root/$capture/samples.csv" \
    "$timing_root/$capture/r2-paired-samples.csv" static-guarded-copy
done

paired_tool=performance/evidence/2026-07-26-perf2-06-vm-audit/summarize_paired.crexx
"$runner" "$paired_tool" --nokeep --args \
  "$timing_root/r1-vs-r0-paired-summary.csv" \
  "$timing_root/formal-12/r1-paired-samples.csv" \
  "$timing_root/extension-10/r1-paired-samples.csv" \
  "$timing_root/extension-12/r1-paired-samples.csv"
"$runner" "$paired_tool" --nokeep --args \
  "$timing_root/r2-vs-r0-paired-summary.csv" \
  "$timing_root/formal-12/r2-paired-samples.csv" \
  "$timing_root/extension-10/r2-paired-samples.csv" \
  "$timing_root/extension-12/r2-paired-samples.csv"
```

## Reset diagnostics

The diagnostics-on Debug products from the correctness section generated the
raw rows. This shell function states the exact 16 invocations without hiding
workload substitutions:

```sh
machine_root="$replay_root/machine-stats"
mkdir -p "$machine_root"

capture_stats() {
  mode="$1"
  vm="$2"
  name="$3"
  rxbin="$4"
  work="$5"
  "$replay_root/r${mode}-debug/bin/$vm" \
    "$r0_release/tests/benchmarks/$rxbin" \
    "$r0_release/bin/library.rxbin" -a "$work" \
    > "$machine_root/r${mode}-${vm}-${name}.out" 2>&1
}

for mode in 1 2; do
  for vm in rxvm rxbvm; do
    capture_stats "$mode" "$vm" permute benchmark_awfy_permute_opt.rxbin 50
    capture_stats "$mode" "$vm" list benchmark_awfy_list_opt.rxbin 100
    capture_stats "$mode" "$vm" base64 benchmark_base64_roundtrip_opt.rxbin 500
    capture_stats "$mode" "$vm" sieve benchmark_awfy_sieve_opt.rxbin 50
  done
done

reset_args=()
for mode in 1 2; do
  for workload in permute list base64 sieve; do
    reset_args+=("r$mode" "$workload" \
      "$machine_root/r${mode}-rxvm-${workload}.out" \
      "$machine_root/r${mode}-rxbvm-${workload}.out")
  done
done
"$runner" "$evidence_root/tools/summarize_vm_c2_reset.crexx" \
  --nokeep --args "$machine_root/reset-summary.csv" "${reset_args[@]}"
```

The retained Level B summarizer rejects VM-parity differences and carries
`core_reset_bytes` directly into the summary.

## Count profiles

```sh
r0_profile="$replay_root/profile-r0-debug"
r1_profile="$replay_root/profile-r1-debug"
r2_profile="$replay_root/profile-r2-debug"

cmake -S "$r0_src" -B "$r0_profile" -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF \
  -DCREXX_VM_PROFILING=ON
cmake --build "$r0_profile" --target rxvm rxbvm --parallel 10

for mode in 1 2; do
  profile_build="$replay_root/profile-r${mode}-debug"
  cmake -S "$candidate_src" -B "$profile_build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF \
    -DCREXX_VM_PROFILING=ON \
    -DRXVM_VM_C2_RESET_MODE="$mode" \
    -DRXVM_VM_C2_RESET_DIAGNOSTICS=OFF
  cmake --build "$profile_build" --target rxvm rxbvm --parallel 10
done

profile_root="$replay_root/profiles"
mkdir -p "$profile_root"
capture_profile() {
  variant="$1"
  profile_build="$2"
  vm="$3"
  name="$4"
  rxbin="$5"
  work="$6"
  "$profile_build/bin/$vm" --profile=counts \
    --profile-output "$profile_root/$variant-$vm-$name.csv" \
    "$r0_release/tests/benchmarks/$rxbin" \
    "$r0_release/bin/library.rxbin" -a "$work" \
    > "$profile_root/$variant-$vm-$name.out" 2>&1
}

for vm in rxvm rxbvm; do
  for variant in r0 r1 r2; do
    profile_build="$replay_root/profile-${variant}-debug"
    capture_profile "$variant" "$profile_build" "$vm" permute \
      benchmark_awfy_permute_opt.rxbin 50
    capture_profile "$variant" "$profile_build" "$vm" list \
      benchmark_awfy_list_opt.rxbin 100
    capture_profile "$variant" "$profile_build" "$vm" sieve \
      benchmark_awfy_sieve_opt.rxbin 50
  done
done

profile_args=()
for workload in permute list sieve; do
  for vm in rxvm rxbvm; do
    profile_args+=("$vm-$workload" \
      "$profile_root/r0-$vm-$workload.csv" \
      "$profile_root/r1-$vm-$workload.csv" \
      "$profile_root/r2-$vm-$workload.csv")
  done
done
"$runner" "$evidence_root/tools/compare_vm_c2_profiles.crexx" \
  --nokeep --args "$profile_root/operation-identity.csv" \
  "${profile_args[@]}"
```

The retained run reused exact-base R0 profiles from the immediately preceding
VM-C2 bundle after input-hash verification. The fresh R0 commands above are the
equivalent independent replay path.

## Code size

For each profiling-off, diagnostics-off Release binary:

```sh
stat -f '%N,%z' "$binary"
otool -l "$binary" > "$variant-$vm-otool.txt"
nm -n "$binary" > "$variant-$vm-nm.txt"
```

Mach-O `__text` is the `__TEXT,__text` section size. `run()` extends from
`_run` to the first strictly greater sorted `T`/`t` symbol address; aliases at
the same address are skipped.

## Integrity

From the PoC worktree root, validate the frozen measured products/inputs and
then every retained evidence file:

```sh
shasum -a 256 -c "$evidence_root/product-hashes.sha256"
shasum -a 256 -c "$evidence_root/checksums.sha256"
```
