# PERF3-02-C2E2 symbolic storage-identity PoC

Status: **complete — core-infrastructure candidate; production integration
requires a separate architecture gate**

Date: 2026-07-31

## Outcome

The diagnostic PoC proves that RXAS can follow the storage cell addressed by
a register slot through direct `link`, `swap`/`swapn` and
`unlink`/`unlinkn`, rather than treating the register number as the storage
identity. The must analysis converges at joins and loops, fails closed on
disagreement and same-frame signal-handler entry, and does not mutate the
instruction queue.

This is materially useful. On the current compiler-generated Richards RXAS,
all 55 full-copy sites rejected by the existing procedure-wide taint have an
exact base-storage identity at the copy. Towers recovers 13 of 56 such sites.
That is an analysis-coverage result, not proof that any full copy may yet be
removed: ownership, value availability, lifetime, TRACE and source identity
remain separate obligations.

The PoC also found the architectural prerequisite that prevents an unsafe
"across the board" conversion today. `linkattr*` and `linkref` can signal
before changing their destination, while `SIGCALLA skip` resumes after the
instruction. Their current CFG fallthrough therefore represents both the
successful new alias and the unchanged or partially changed skip state. The
PoC correctly reports that merged destination as unknown. Production exact
attribute identities require distinct normal/success and signal/skip edges,
or a local proof that the operation cannot signal.

Disposition: **core-infrastructure candidate** for direct link/swap/unlink and
point identity, with exception-aware mapping edges as the first production
architecture prerequisite. No tactical rule was removed and no emitted RXBIN
changed.

## Representative results

| Input | Procedures | Exact state cells | Unknown | Globally tainted full-copy sites | Exact/base at point | Swap round trips |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards current compiler RXAS | 24 | 171,203 / 176,414 (97.046%) | 5,211 | 55 | 55 / 55 | 0 |
| Towers current compiler RXAS | 13 | 39,040 / 39,858 (97.948%) | 818 | 56 | 13 / 13 | 0 |
| retained CRI-13 hot-shape fixture | 1 | 262 / 264 | 2 | not comparable | one of two copies exact/base | 0 |

The representative compiler RXAS contains only throwing attribute-link forms
in the counted link population: Richards reports 649 success-only/unknown
links and Towers 64. Direct-link transfer is proved by the focused fixture.
The existing tactical optimizer already removes the residual adjacent swap
round trips in these two optimized workload inputs, so both report zero.
The focused fixture proves one non-adjacent, unobserved `swapn` round trip and
rejects the corresponding case when an affected mapping is written.

The exact frozen CRI-13 input was previously retained only by hash and a
focused trace; its old temporary source is unavailable. The PoC therefore
replays the exact relevant instruction shape from the retained trace. It
keeps `linkattr1 r39,a1,8; copy r31,r39` unknown because of signal/skip, while
proving `copy r40,r31; blen r41,r40` exact and base-local. This is a focused
mechanism proof, not a fresh live replay of frozen R1.

## Safety and product identity

- Focused transfer/join/loop/handler/reference diagnostics: pass.
- Both VMs, optimized and no-opt runtime fixture: 4/4 pass.
- Complete selected RXAS optimizer/runtime set: 52/52 pass.
- Ordinary Release `rxas` build: pass.
- Richards debug-versus-ordinary RXBIN SHA-256:
  `6659a2495fb14cedbd0a448e554266a0e77db23e947b7c63ac3392d2df0a9be8`.
- Towers debug-versus-ordinary RXBIN SHA-256:
  `d5f2c79b19b15e58c2626a087080e850e80156d2d2ead93836c3d8aad5f0c342`.

Those hash pairs compare fresh assemblies with the same input and output stem.
They do not claim identity with a CMake-produced RXBIN whose embedded output
identity may differ.

## RXAS simplification boundary

The current tactical rules remain the emitted-image oracle. Their migration
map is in `summary/tactical-rule-map.csv`.

- The two swap-cancellation rules are direct future consumers of one shared
  permutation-round-trip proof.
- The six duplicate direct-link read rules are candidates for replacement by
  shared storage identity plus value/ownership/TRACE facts.
- The six duplicate `linkattr1` read rules cannot yet be replaced safely;
  they need exception-aware edges and canonical attribute access paths.
- The three adjacent swap collectors and six call-window preparation rules are
  instruction selection. They may consume a future normalized permutation,
  but are not made obsolete merely by knowing storage identity.

This separates reusable semantic analysis from local superinstruction
selection and provides a controlled route to simplifying `rxas_opt.c` without
discarding working coverage.

## Production integration plan for approval

1. Move the bounded analysis from debug reporting to a reusable graph-owned
   query with no rewrite consumer and add explicit normal versus signal/skip
   mapping edges.
2. Make existing available/may-reach value facts query storage identities at
   each point while retaining ownership, metadata, TRACE and effect barriers.
3. Add a shared swap-permutation consumer and replace the two cancellation
   rules after bytecode and runtime equivalence proof.
4. Replace the six direct-link duplicate-read rules through the shared
   identity/value consumer, one typed/full-value family at a time.
5. Add canonical attribute access paths only after the exceptional-edge proof,
   then evaluate the six `linkattr1` rules. Retain the local rules until their
   replacements have exact negative coverage.
6. Keep swap collection and call-window fusion as an instruction-selection
   layer, potentially simplified to consume a normalized sequence.

The first production rewrite would immediately enter the mandatory ordinary
Release verdict and stop for Adrian's decision before broad closeout.

## Evidence map

- `summary/identity-replay.csv` — aggregate exact/unknown transfer and copy
  coverage.
- `summary/tactical-rule-map.csv` — rule-family migration disposition.
- `raw/focused-diagnostics.txt` — exact focused proof diagnostics.
- `provenance.md` — host, inputs, hashes, commands and validation scope.
- `checksums.sha256` — recursive bundle integrity excluding itself.

