# DECIMAL-01 decNumber tuning candidates

This opt-in directory compiles the existing `mc_decimal` adapter and decNumber
3.68 source as an isolated 48-build tuning grid:

- `DECDPUN`: 3, 4, 8 and 9;
- `DECNUMDIGITS`: 18, 34 and 64; and
- `DECBUFFER`: 20, 36, 64 and 128.

The four buffer sizes are the multiples-of-four nearest the predeclared
18/34-digit boundaries plus the current 64 and extended 128 controls. Each
loadable plugin is one composed build; results from different builds are not
combined into an unmeasured synthetic candidate.

The grid is excluded from ordinary builds. Enable it in a separate Release
tree:

```text
cmake -S . -B cmake-build-release-decimal-stage3 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DCREXX_BUILD_DECNUMBER_TUNING_CANDIDATES=ON
cmake --build cmake-build-release-decimal-stage3 \
  --target decnumber_tuning_candidates --parallel 10
```

Plugin names encode the complete configuration. For example,
`rxvm_mc_decimal_d2_p3_n18_b20` means `DECDPUN=3`, `DECNUMDIGITS=18` and
`DECBUFFER=20`. The unchanged ordinary provider remains
`rxvm_mc_decimal` with its default 8/64/64 configuration.
