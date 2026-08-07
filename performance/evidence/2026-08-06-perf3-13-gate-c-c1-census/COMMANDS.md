# Gate C C1 command record

All commands ran serially in the private scratch source
`/private/tmp/crexx-rxvm-inline.yvLywZ/source`. Build and profiling output was
redirected to bounded log files.

## Diagnostic build

```sh
cmake -S /private/tmp/crexx-rxvm-inline.yvLywZ/source \
  -B /private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-v0-census \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=ON
cmake --build /private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-v0-census \
  --target rxvm rxbvm rxtvm test_rxvmmemory test_rxvmprofile --parallel 10
```

The final focused check was:

```sh
ctest --test-dir /private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-v0-census \
  --output-on-failure \
  -R '^(rxvmmemory_allocator|rxvmprofileaccounting|rxvmprofiletable|rxbvmprofiletable|rxvmprofilecsv|rxbvmprofilecsv|rxvml_utf_boundaries|rxpa_utf_validation|ts_regvalue_tester)$'
```

## Product-lane census

Every cell used this shape, with `CREXX_RXVM_MEMORY_STATS=1` and `/usr/bin/time
-lp` supplying separate allocator/process diagnostics:

```sh
<diagnostic-build>/bin/rxvm --profile=counts \
  --profile-output profiles/rxvm-<workload>.csv \
  <gate-b-control>/tests/benchmarks/<image>.rxbin \
  <gate-b-control>/bin/library.rxbin <arguments>
```

Exact cells:

| Workload | Image | Arguments |
|---|---|---|
| Sieve | `benchmark_awfy_sieve_opt.rxbin` | `-a 5500` |
| Richards | `benchmark_awfy_richards_opt.rxbin` | `-a 20` |
| Base64 | `benchmark_base64_roundtrip_opt.rxbin` | `-a 2500` |
| JSON | `benchmark_json_parser_opt.rxbin` | `-a 5000` |
| Towers | `benchmark_awfy_towers_opt.rxbin` | `-a 100` |
| RexxCPS | `benchmark_rexxcps_levelb_opt.rxbin` | canonical `-a` with no user arguments |

Sieve, Base64 and Towers were repeated with `<diagnostic-build>/bin/rxtvm`.
The value-census sidecars were compared after replacing only `vm=<mode>` with
`vm=VM`; all three pairs were byte-identical.

## Ordinary product exclusion proof

```sh
cmake -S /private/tmp/crexx-rxvm-inline.yvLywZ/source \
  -B /private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-ordinary-proof \
  -G Ninja -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF
cmake --build /private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-ordinary-proof \
  --target rxvm rxbvm rxtvm test_rxvmmemory --parallel 10
<ordinary-build>/interpreter/test_rxvmmemory
<ordinary-build>/bin/rxvm <Sieve image> <library.rxbin> -a 50
strings <ordinary-build>/bin/rxbvm | \
  rg 'value-census|RXVM_MEMORY_OVERSIZED_STATS'
```

The final search returned no match. This is a build/exclusion and correctness
proof, not a formal timing comparison.
