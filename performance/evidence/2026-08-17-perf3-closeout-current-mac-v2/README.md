# PERF3 closeout portfolio-v2 Mac baseline

This bundle is the first formal portfolio-v2 baseline and the terminal Mac
scorecard for PERF3. It measures the accepted current working-tree product; it
does not introduce or select another production optimization.

## Outcome

| Dimension | Result |
| --- | --- |
| Source | `c38a5d184dee1ecd659c83ccbc4a18001d46dc77` plus the controlled, uncommitted PERF3-closeout scope; exact product, tool, source and generated-artifact hashes are in `artifacts.csv` |
| Host | MacBook Air `Mac17,3`, Apple M5, 10 logical CPUs, macOS 26.5.2 (25F84), AC power, low-power mode off |
| Product | fresh profiling-off Release (`-O3 -DNDEBUG`); compiler-selected `rxvm` is the bytecode engine (`rxvm -> rxbvm`); `rxtvm` is a separately labelled concrete-engine control |
| Correctness gate | fresh Release pre-timing gate 22/22 and final combined focused replay 26/26; portfolio-v2 qualification 36/36 cREXX process cells plus qualified ooRexx and NetRexx Base64-v2 cells |
| Formal timing | 516/516 initial processes and 150/150 one-append processes; 580 recorded observations, 86 warmups, zero discarded samples |
| Common five | product `rxvm/ooRexx 4.897751`; `rxvm/NetRexx 1.543319`; v2 is a new aggregate identity and is not a v1 trend comparison |
| Peak RSS | 86/86 zero-warmup observations; kept separate from elapsed time |
| Lifecycle | 70/70 phase samples; all seven phase rows remain variability-flagged and are diagnostic only |
| Artifact inventory | 87/87 exact path, size and SHA-256 rows |
| Fusion registry | 1,248 public serialized sites plus 34 exact private-eligible sites across 17 optimized images; adaptive quickening closed |
| Stop | PERF3 retired; no further VM edit, cross-platform performance claim, sanitizer/install claim, direct CPython/Java language score, commit or push |

The v2 aggregate is ahead of both qualified reference aggregates, but that
summary is deliberately not allowed to hide the remaining deficits. Richards
is below ooRexx and NetRexx, and Permute is below decimal NetRexx. The new
idiomatic Base64-v2 cell is materially stronger than both references and is a
new source identity, which is why no numerical comparison to the retained v1
aggregate is valid. See [scorecard.md](scorecard.md).

## Evidence map

- `timing/initial/` retains two warmups and ten serial recorded processes for
  every one of the 43 formal cells, including captured correctness output.
- `timing/append/` retains the one permitted ten-sample append for the exact 15
  initially flagged cells. `timing/final/` is the merged authority.
- `rss/` retains two independent peak-RSS observations for every formal cell.
- `lifecycle/` retains compile/translate, assemble and honestly named
  load-to-first-result phases plus the exact generated probe forms.
- `generated/netrexx-v2/` retains the exact staged source, generated Java and
  class used by the Base64-v2 NetRexx cell.
- `artifacts.csv` inventories the toolchain, VMs, sources, RXAS/RXBIN images,
  external runtimes, tools, manifests and lifecycle products.
- `manifests/` retains the formal, noise-append, artifact and fusion manifests.
- `provenance/` retains configuration/build logs, CMake cache and host/source
  facts. `checksums.sha256` recursively closes this directory.

## Noise and inclusion

No correctness-passing observation was removed. After the single governed
append, 15 timing cells remain flagged: ooRexx and NetRexx Permute, NetRexx
Bounce, ooRexx Richards, product and NetRexx Towers, ooRexx/Regina/NetRexx
RexxCPS, product JSON-legacy and JSON-parse, both JSON-query controls, and both
Queens controls. They remain in the final medians. A second timing append was
not taken.

The RSS run is descriptive and deliberately small (`n=2`). NetRexx Bounce,
Base64-v2 and RexxCPS are variability-flagged. Every lifecycle phase is also
flagged at `n=10`; lifecycle is retained as a boundary diagnostic, not blended
into throughput.

## Claim boundary

This is a same-session Apple ARM64 closeout baseline for an exact uncommitted
working-tree scope. The worktree could not be commit-clean without exceeding
the publication authority, so the bundle uses the base commit plus an
87-artifact hash inventory rather than pretending otherwise. It does not prove
Linux or Windows performance, final release packaging, sanitizer cleanliness,
or superiority of cREXX over Java or Python. NetRexx uses its normal generated
Java/HotSpot substrate; direct Java and CPython AWFY ports remain labelled
post-PERF3 controls if they are added.
