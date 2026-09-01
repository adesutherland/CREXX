# PERF3-12B B5 first ordinary Release verdict

Date: 2026-08-05

Status: accepted by Adrian; B6 proportional closeout authorized

## Verdict

The clean production reimplementation of H1 is favorable and accepted.  RXAS
keeps the first conditional joined-key construction as the lazy seed, assigns
one new private local to that seed generation, and redirects four later proved
equivalent uses.  No benchmark-specific rule, new opcode, public RXAS/RXBIN
form, RXC optimization or VM representation is introduced.

The implementation emits `main` as `380 -> 365` instructions with
`.locals=104`.  Five generated `"Key Bee." || lvar` constructions become one
cached construction consumed by the initial `STEMSET`, three `STEMGET`s and the
later `STEMSET`.  The proof service makes exactly four relevant queries and
accepts all four; the semantic batch allocates one private local atomically.

## First Release comparison

The ordinary profiling-off Release product was built with
`CREXX_VM_PROFILING=OFF`.  The retained B4 S0 RXAS was assembled once with the
production RXAS tool so the comparison changes only the accepted optimization.
The same current Release VM and library executed S0 and production.

The host was on AC power with low-power mode off.  One warmup and six serial,
balanced and rotating recorded rounds were run across S0/production and both
VMs.  All 28 processes passed; no sample was removed.

| VM | S0 median CPS | Production median CPS | Paired median | Favorable | Mean 95% interval |
|---|---:|---:|---:|---:|---:|
| `rxvm` | 46,288,139 | 47,199,707 | +2.557920% | 6/6 | +1.208636%..+3.320166% |
| `rxbvm` | 45,806,432 | 47,316,788 | +3.169497% | 5/6 | -0.540467%..+5.171145% |

The small `rxbvm` panel retains one -3.006146% observation and is therefore
inconclusive by the mean-interval rule.  It remains directionally consistent
with the checksum-closed B4 36-pair result, where H1 was clear favorable at
+4.274944%.  The new production `rxvm` panel is independently clear favorable,
and the production structure exactly reproduces the selected H1 mechanism.
Adrian accepted the first verdict and authorized B6.

## Correctness and fail-closed coverage

- the final focused graph/metadata/semantic-batch/optimized/no-opt panel passes
  5/5 in both Debug and Release;
- the native-stem/runtime selector panel passes 19/19;
- ordinary and retained-input production images pass under both `rxvm` and
  `rxbvm` with zero stderr;
- changed right values, reference exposure, counted-call observation,
  signal/extra TRACE, and absence of a valid loop fail closed;
- direct later reuse of the compiler temporary remains valid because the cache
  owns a distinct fresh local; and
- the Sieve zero-candidate control remains byte-identical to the pre-production
  control.

## Evidence map

- `timing/` retains the capture manifest, all warmup/recorded samples, process
  outputs, absolute summary and the exact paired rows;
- `structure/` retains S0/production disassemblies and the final proof-service
  diagnostic summary;
- `artifacts/rexxcps-production.rxbin` is the exact retained-input production
  image used in the comparison;
- `logs/` retains driver and final focused Debug/Release results;
- `identities.sha256` binds the source input, S0/production products, assembler,
  both VMs and library; and
- `checksums.sha256` recursively closes this directory except itself.

Broad QA, the fresh current-product Mac scorecard, documentation closeout and
publication belong to B6 and are intentionally not claimed by this bundle.
