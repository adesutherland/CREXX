# DECIMAL-01 Gate 1 current-provider cell matrix

Status: **preparation only; no timing captured and host not reserved**

This record fixes the Gate 1 comparison boundary before calibration or formal
sampling. The existing Level B
`performance/tools/run_cross_runtime_matrix.crexx` remains the sampling and
validation tool. It can express the required provider choice directly in each
cell's argv, so no new timing runner is needed.

The maintained kernels are also the arithmetic core of the publishable
[`CREXX Decimal Benchmark (CDB-1)`](CREXX-DECIMAL-BENCHMARK.md). CDB-1 is an
independently authored CREXX benchmark informed by public decimal literature;
it is not described as a standards-body benchmark.

## Provider rows

| Row | Provider selection | Context | Interpretation |
| --- | --- | --- | --- |
| D0s | built-in default `mc_decimal` | Common 18 and Classic 9 | canonical current-product baseline, static/default packaging |
| D0d | `-p rxvm_mc_decimal` | Common 18 and Classic 9 | dynamic-packaging control; excluded from D0s throughput claims |
| D1a | `-p rxvm_db_decimal` | Common 18 | unrestricted diagnostic ceiling; every checksum difference remains visible and excludes the cell from correctness-qualified ranking |
| D1b | `-p rxvm_db_decimal` | Classic 9, FUZZ 0 | speed control admitted only where its exact output/branch result matches D0s and D0d |

`rxvm` and `rxbvm` are separate rows. Optimized compiler output assembled with
RXAS optimization enabled is the primary product result. The identical
optimized compiler output assembled with RXAS `-n` is the assembler-isolation
control. Full no-opt disables both compiler and RXAS optimization and is a
broader diagnostic. The three are reported separately and never averaged.
Static/default and dynamic-provider lifecycle, RSS and artifacts also remain
separate dimensions.

## Maintained fixed-work payloads

The two Level B sources under `tests/performance/decimal/` perform no internal
timing and emit deterministic checksums:

- `decimal_gate1_common18.crexx` owns D0/D1a Common-18 comparisons;
- `decimal_gate1_classic9.crexx` owns D0/D1b Classic-9 qualification.

Each accepts `MODE ITERATIONS OPAQUE_SEED`. Every decimal operand family is
derived from the runtime seed so optimized cells cannot constant-fold away the
provider operation. `all` is a correctness-smoke mode only; formal cells select
one kernel so the operation mix is explicit.

| Layer | Mode | Fixed work per iteration | Primary pressure |
| --- | --- | ---: | --- |
| L2 | `arithmetic` | 4 units | add, subtract, multiply and exact divide over pre-created decimals |
| L2 | `conversion` | 2 units | string-to-decimal and decimal-to-string with short, small-exponent and exponent forms |
| L2 | `compare` | 3 units | equal, early-different and precision-edge late-different operands |
| L2 | `context` | 1 context event | procedure entry/return, provider context sync, add and observed compare |
| L3 | `ledger` | 3 units | parse, signed balance update and formatted posting |
| L3 | `compound` | 60 units | twelve monthly multiply/divide/add/subtract/format steps |

The existing separately named `rexxcps_family_controls.crexx decimal-string`
cell is an attribution-only L3 control. Canonical-default RexxCPS 2.2d is the
mandatory whole-product guard and remains separately reported rather than
entering a decimal aggregate.

## L1 adapter boundary

Level B cannot call the C `decplugin` function table directly. The L1 payload
will therefore be a small fixed-work C ABI exerciser under
`tests/performance/decimal/`; it is a measurement payload, not an orchestration
or statistics tool. The existing Level B matrix runner continues to own argv,
warmups, samples, output retention and summaries.

The payload must:

1. dynamically load exactly one named bundled provider;
2. install an explicit five-field numeric context;
3. pre-create operands outside arithmetic/compare loops;
4. expose separate arithmetic, comparison, parse/format, copy/clear and
   context-sync modes;
5. emit exact operation count and deterministic final text/branch checksum;
6. include no candidate-specific shortcut and no internal sample selection;
7. keep process elapsed, plugin lifecycle and steady adapter work separately
   labelled; and
8. free every value and the provider on every normal exit.

No L1 result will be interpreted as a pure library-core result. L0 native-core
experiments remain disposable scratch diagnostics outside the repository.

## Qualification and sampling gates

Before calibration:

- [x] build both sources as compiler-on/RXAS-on,
  compiler-on/RXAS-off and compiler-off/RXAS-off images;
- [x] run the `all 2 1` smoke across all three optimizer boundaries, both VMs
  and default/explicit `mc_decimal`/`db_decimal` providers;
- [x] retain exact D0s/D0d/D1a/D1b checksum matrix below;
- [x] admit D1b mode by mode only when its checksum matches D0;
- [x] build and qualify the L1 payload; and
- [ ] verify the canonical RexxCPS command still reports
  `contract=canonical-default` under each provider row where applicable.

The 2026-08-05 Release qualification passed all 81 selected CTest fixture,
adapter, workload, RexxCPS-family and optimizer-integrity tests. Maintained
checks confirm that runtime parse/format, arithmetic, `DEQ` and `DLT`
instructions remain after disassembling all six final context/mode RXBIN
images: product optimized, optimized-compiler/RXAS-noopt, and full no-opt.
These are correctness/setup results; CTest elapsed values were discarded.

The exact two-iteration Level B matrix collapses to three checksum rows because
VM and all three optimizer selections did not change any checksum:

| Context/provider | Arithmetic | Conversion | Compare | Context | Ledger | Compound | Admission |
| --- | --- | --- | ---: | --- | --- | --- | --- |
| Common-18 D0s/D0d | `123456789012345.61` | `39:0.0000123456789012341` | 14 | `3.96` | `100113580245802.218:38` | `241445676608.4196:448` | canonical baseline |
| Common-18 D1a | `123456789012346` | `36:0.0000123456789012341` | 6 | `3.96` | `100113580245802:30` | `241445676608.801:376` | diagnostic only; mismatches retained |
| Classic-9 D0s/D0d/D1b | `12345.6781` | `25:0.0000123456781` | 14 | `3.96` | `102469.135:20` | `241.4363:232` | all six D1b modes admitted |

The L1 adapter also passed all 20 provider/context/mode cases. Its differing
copy/clear byte checksums (`88` for `mc_decimal`, `16` for `db_decimal`) expose
representation size rather than a semantic mismatch. Common-18 context-sync
checksums (`36` versus `30`) retain the Apple `db_decimal` 15-digit cap.

After qualification, but before the first calibration or formal timing cell,
Adrian must clear and reserve the host. Calibration selects one common integer
iteration count per like-for-like provider group that makes the fastest cell at
least one second without unequal work. Formal absolute baselines use two
warmups and ten recorded serial observations with rotated row order. RSS uses
zero warmups and three observations. No elapsed value produced during build or
correctness qualification is performance evidence.
