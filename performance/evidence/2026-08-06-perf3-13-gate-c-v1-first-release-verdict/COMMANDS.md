# Reproduction commands

The scratch source was at commit `0d1fe884782ff369960b1c67c38127407ce54588`
with the uncommitted Gate C allocator/value-layout diff described in
`source-identities.csv`.

Configure and build the two ordinary profiling-off S0 Release products:

```sh
cmake -S SOURCE -B V0_BUILD -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=ON -DCREXX_VM_PROFILING=OFF \
  -DCREXX_RXVM_MEMORY_GEOMETRY=S0 -DCREXX_RXVM_VALUE_LAYOUT=V0
cmake -S SOURCE -B V1_BUILD -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=ON -DCREXX_VM_PROFILING=OFF \
  -DCREXX_RXVM_MEMORY_GEOMETRY=S0 -DCREXX_RXVM_VALUE_LAYOUT=V1
cmake --build V0_BUILD --target ts_regvalue_tester rxbvm rxvm --parallel 10
cmake --build V1_BUILD --target ts_regvalue_tester rxbvm rxvm --parallel 10
```

Run the focused correctness checks in each build:

```sh
ctest --test-dir BUILD \
  -R '^(rxvmmemory_allocator|ts_regvalue_tester|rxvm_product_entry_point)$' \
  --output-on-failure
```

Prove V0 emitted-text identity against retained S0:

```sh
otool -tvV RETAINED_S0/bin/rxbvm > retained.text
otool -tvV V0_BUILD/bin/rxbvm > fresh-v0.text
sed '1d' retained.text > retained.body
sed '1d' fresh-v0.text > fresh-v0.body
cmp retained.body fresh-v0.body
shasum -a 256 retained.body fresh-v0.body
```

Run the balanced first verdict with the retained Gate B `crexx` control:

```sh
caffeinate -i GATE_B_BUILD/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest manifests/value-layout-v0-v1.txt \
  --output-dir timing --measurement timing --warmups 1 --runs 4
```

The paired summary uses recorded passing samples only. For elapsed workloads,
the favorable per-round delta is `(V0 - V1) / V0`; for RexxCPS it is
`(V1 - V0) / V0`. The mean 95% interval uses Student's t with three degrees of
freedom (`3.182446305`). The stable-six result is the geometric mean of median
V1/V0 throughput ratios for Sieve, Permute, Bounce, Richards, Towers and
RexxCPS. Base64 is deliberately excluded.

