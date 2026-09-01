# PERF3 closeout fusion registry evidence

Status: **complete diagnostic census; no production candidate**

This bundle supports `performance/PERF3-FUSION-REGISTRY.md`. The maintained
Level B census tool inspected post-RXAS disassembly from 17 optimized
portfolio-v2 RXBIN images.

Results:

- 1,248 public serialized fusion/superinstruction sites;
- 34 observed public identities;
- 18 exact `PRIVATE_R1_RELINK` sites;
- 16 exact `PRIVATE_R2_COPYATTR1` sites; and
- 1,282 public plus private-eligible static sites in total.

Private eligibility is List 14, Towers 9, NBody 7, Bounce 2 and Richards 2.
These are static sites, not dynamic executions. The current profiler's
canonical attribution is documented in the registry and no private dynamic
count is fabricated.

The census initially inspected compiler-produced RXAS. Review found that this
would omit fusions selected by RXAS itself, so the retained result was rerun on
final optimized RXBIN disassembly. Only the corrected post-RXAS result is
retained here.

Files:

- `fusion-census.csv`: complete per-image/per-identity output;
- `fusion-identities.csv`: summed identity counts;
- `fusion-workloads.csv`: summed workload counts and private subset;
- `PROVENANCE.md`: source, tool, product and host identity; and
- `COMMANDS.md`: reproducible generation and validation commands.
