# Prototype provenance

## Common baseline

- Branch/source: detached `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`.
- Accepted Q0 `rxvm` SHA-256:
  `2bb27f8a31298e20a67b6a986040580f83e5bcd7261b1795333e3d36762ca871`.
- Accepted Q0 `rxbvm` SHA-256:
  `a738271972e52ea8a0cf0981eb86d58179c6f715824595eb8ca576737e360e15`.
- Accepted `library.rxbin` SHA-256:
  `a9b660f6a67fd57fa35ae180a6b3c0f2764d44241fc0272bd6c6b843ce5d8e10`.
- Accepted bundle manifest SHA-256:
  `fde9aa923fc451ea16edc0c4f09bf532faa2e024aa22fbfc72e35a965f78eacc`;
  independently verified `1948/1948` rows before the PoC.
- Configuration: CMake `4.3.2`, Ninja `1.13.2`, Apple clang `21.0.0`,
  `Release`, `CREXX_VM_PROFILING=OFF`, `BUILD_TESTING=ON`.

## Isolated variants

| Variant | Scratch source | Tracked patch SHA-256 at build | `rxvm` SHA-256 | `rxbvm` SHA-256 |
| --- | --- | --- | --- | --- |
| Q3 direct | `/private/tmp/crexx-perf2-02.kJ5sZT/q3-direct` | `b4782927bada7fca11d6ca32db740eab1ed7bf2af504de86a3cec00e5d6f0c16` | `3afb0d87e4d2fefa1e30452254806bdd81e56001991f97538f14d5427301c1c7` | `583e85ce831fdb0e47cf49866de1fff0017921dc83ee2ea164e6f8fae2c9e2ae` |
| Q3b exact direct | `/private/tmp/crexx-perf2-02.kJ5sZT/q3b-exact` | `e673e61e34c248a5e9002974edc4df8599bee03a3e0dcc5f558b5c083a552150` | `382f2262e638c61135269d65551a60fe2867253d188e6478442dea260b6f4b0f` | `2a8ed23cd9e857b9e0bb85fe8dae0a9f312295226b4a9b412afc30c63b4a6858` |
| Q4 eager | `/private/tmp/crexx-perf2-02.kJ5sZT/q4-eager` | `5f9d81ce35c05dec104c8464f0e823e0caf4ba1ddf7a9d1d3b060fb76a84b164` | `c81271321c7a94b4b32b2e433015d84f3747bf6c0c8087e20949f39ea364e0ed` | `f15817dfaa5746c8997b5877a1f21576bfa831a838a493d5afc017b158c60c94` |
| Q7 core | `/private/tmp/crexx-perf2-02.kJ5sZT/q7-core` | `87ca836eeedf743df65b990a759d735a617e4cac1dd992435120a3fe3b8bed93` | `fdeafe3c2c6519cb09a05d3a12ce57ec1dd83c6f6815c669322ae4c128aa52c0` | `0daaeef9cdf3c8c4a216b386851536d7065e04d48023d94c071c93cf9918d2fb` |
| Q1 Richards | `/private/tmp/crexx-perf2-02.kJ5sZT/q1-richards` | `5522ded06f2cca8d564fe0f674e187fed39753a2358b484dadcc5ab72ecd8bd6` | accepted Q0 runtime used for timing | accepted Q0 runtime used for timing |

Q1's generated Richards image SHA-256 is
`57d803889249c2c22bc7aba4812bdf916aa16661e739fc052cb21cd024e552a3`.
Its Q1-built VM hashes are retained only as build proof and were not timing
inputs.

Q3b, like every runtime prototype, was timed with the accepted Q0 benchmark
image and accepted Q0 library above. Its separately built library is build
proof only and is not a product-timing input.

## Build-private diagnostic overlays

The Q3b and Q4 counter overlays hash to
`6124071290f4f8fd01f6d3222068fe1e6dd009f7e21006b6df5c5f9525a1fd64`
and
`be4b7f48d03f3a6d1dde61316124a517be882de188e826cfcc0d75a67aca9c1d`.
After their eight diagnostic runs, both overlays were reversed and ordinary
Release targets rebuilt; the original four Q3b/Q4 binary hashes in the table
above were reproduced exactly. The isolated Q7 diagnostic overlay hashes to
`ab2657a617cdf9a16058656d36c6f7ce636d9b7f9277c4a25a397bffc7b56ac9`
over the unchanged Q7 base patch. These builds add counters/timers and were
never product-timing inputs.

## Host state

Mac17,3 Apple M5 arm64, 10 logical CPUs, 24 GiB RAM, Darwin `25.5.0`,
macOS `26.5.2` build `25F84`. Timing blocks ran serially under `caffeinate -i`
on attached AC power with battery 80% and AC low-power mode `0`; no other build,
test or benchmark campaign overlapped them. The decisive Q0/Q3b/Q4 block began
at load `1.13 1.48 1.83` and ended at `1.15 1.37 1.68`. The Sieve guard ended
at `1.48 1.42 1.66`; the final balanced RSS block ended at
`1.69 1.51 1.64`. AC and low-power state were reread unchanged around each
block. `kern.thermal_state` returned no value on this host.
