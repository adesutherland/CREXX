# PERF3-12 K04e reassessment validation

## Product and build

The ordinary timing product and profiling build both came from clean detached
commit `c4470635048e497417c4db92c03ecbcd79eaa750`. The ordinary build is
`Release`, `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`; the profiling build uses
the same optimization flags with profiling on. CMake 4.3.2, Ninja 1.13.2 and
AppleClang 21.0.0 were used on Apple M5/macOS 26.5.2.

The fixed manifest binds:

- unchanged no-opt image
  `6a3d8844d60f28c4c60315031b7e8116fb344af10f6ac4422359d508c138d223`;
- current optimized image
  `46c4fb77a567135de1f4221c8b3486ec468c3ada43cb485e7c4cf5b00e1478e0`;
- current library
  `7322d7ef98f30ecfc6ce52ac3e976a26fdeda5e24d0dfc2be4d3e83a2e68971f`;
  and
- unchanged evidence runner
  `2495bc2c630e5862368e16dfebc89f459b4e3b5268357dd4aef8ed2ee9af5713`.

## Commands

```sh
cmake -S <clean-c44706350-source> -B <profile-build> -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=ON
cmake --build <profile-build> --target rxvm rxbvm rxseq --parallel 10

<ordinary-build>/bin/crexx performance/tools/run_evidence_bundle.crexx \
  --nokeep --args --manifest manifests/rexxcps-profile-fixed200-manifest-v1.txt \
  --release-build <ordinary-build> --profile-build <profile-build> \
  --output-dir profiles/<vm> --vm <vm> --profile-mode counts --force
```

The two VM bundles were captured serially. Both commands returned PASS with
zero stderr and retained one correctness-checked recorded profile plus RXSEQ
N=2/3/4 evidence for each optimized/no-opt image.

## Independent count audit

Summing every `instruction` row reproduces:

| VM | Previous no-opt | Current no-opt | Previous opt | Current opt |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 148,701,541 | 148,700,911 | 54,221,210 | 53,660,581 |
| `rxbvm` | 148,701,541 | 148,700,911 | 54,221,182 | 53,660,552 |

The no-opt RXBIN is byte-identical, so its 630-dispatch change is outside the
benchmark image and accompanies the accepted current shared library. The
optimized total falls by 560,629/560,630. The separate K04e site proof accounts
for exactly 560,000 of those dispatches (`ILT+BRF -> BLE`); the remaining
low-frequency movement agrees with the no-opt/library and final-output paths.

The ranked mechanism counts are unchanged in both VMs:

- PARSE instructions: 1,960,000;
- derived PARSE-only copies: 7,280,000;
- derived PARSE grouped nulls: 1,960,000;
- `CONCAT string,reg`: 2,520,002, of which 2,240,000 are exact Key Bee tails;
- `DCOPY=DTOS=2,220,000`;
- decimal copy value operations: 2,220,000 and 97,680,000 bytes;
- hot `CALL4`: 280,000; and
- stem-get/set/reset counts: 2,500,000/1,380,000/280,000.

All schema-5 status domains are complete. Both profiles report result zero,
the expected PASS marker, zero invalid events and zero overflow. Each nested
bundle checksum file verifies completely.

## Host and claim boundary

The host stayed on AC with low-power mode off and no thermal/performance
warning. Counts are deterministic attribution, not elapsed-time performance.
The refreshed profiling-off Mac scorecard and accepted paired K04e verdict
remain the runtime authorities.
