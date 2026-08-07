# PERF3-13 Gate C C2 formal allocator-geometry verdict

Date: 2026-08-06

## Verdict

Recommend **S0 (64 KiB slabs, 16 KiB maximum slab class)** as the provisional
Gate C substrate for the later value-shape panel. S1 and S1b remain useful
diagnostic alternatives, but neither displaces S0 as the balanced default:

- S1 (64/8 KiB) reduces allocator-owned retained slabs by exactly 196,608
  bytes on every workload, but its stable-six median aggregate is 0.997964
  versus S0 (-0.2036%), it is clearly adverse on Sieve and Bounce, and its
  measured process RSS is 0.22-1.74% higher.
- S1b (128/16 KiB) has a 1.003752 stable-six median aggregate (+0.3752%) and
  process RSS 1.69-2.49% below S0. The timing result is inside the neutral 1%
  aggregate band, however, while deterministic allocator telemetry shows
  589,824-917,504 additional retained slab bytes per worker and no reduction
  in peak-live allocator capacity.
- S0 avoids both trade-offs. No timing, RSS or artifact guard fires for either
  survivor, so this is a simplicity and worker-scaling selection, not a claim
  that the alternatives are regressions outside the approved budget.

S2 was rejected by the preceding first verdict and did not enter this formal
panel. This verdict stops before `rxtvm`, V1 value layout, reclamation or any
production industrialisation. Adrian remains the authority for accepting S0
and opening the next slice.

## Exact product and scope

- Source commit: `0d1fe884782ff369960b1c67c38127407ce54588`.
- Branch: `codex/rxvm-default-and-base64-review` in the isolated scratch
  worktree `/private/tmp/crexx-rxvm-inline.yvLywZ/source`.
- Dirty scope: the provisional C2 closed geometry selector, allocator/value
  focused tests, live worklist/roadmap updates and retained C2 evidence.
- Host: Apple M5, 10 logical CPUs, macOS 26.5.2 (Darwin 25.5.0), Apple Clang
  21.0.0.
- All cells are ordinary profiling-off Release builds. In every cell the
  compiler-selected `rxvm` product resolves to `rxbvm`.
- V0 remains the unchanged 240-byte `value`; R0 performs no automatic
  reclamation.
- Canonical optimized images and the runtime library are reused from the
  accepted Gate B build. Spawn is deliberately not exercised during this
  transitional allocator/value-shape programme.

## Correctness and sampling

The Level B runner self-test passes, including its exact three-cell schedule
proof. For 12 rounds, each survivor occupies each absolute position four times
and every pair appears six times in each relative order.

All 765 timing executions pass correctness: 21 warmups and 744 recorded
samples. No sample was removed.

1. Initial block: one warmup plus 12 balanced recorded rounds for seven
   workloads and three survivors; 273/273 pass.
2. Absolute-noise append: Sieve and Base64 crossed the declared 10% span rule,
   so the prescribed ten serial paired rounds ran with no warmup; 60/60 pass.
3. Uncertainty append: the first paired intervals crossed zero, so all seven
   workloads received another 12 balanced pairs; 252/252 pass.
4. Terminal append: the five workloads still below the ceiling received the
   last 12 balanced pairs; 180/180 pass.

Sieve and Base64 therefore have 34 recorded pairs per comparison. Permute,
Bounce, Richards, Towers and RexxCPS have the governed maximum of 36. An
interval that still crosses zero is reported as `noisy_inconclusive`; no
favourable subset was selected.

The separate RSS panel uses four serial, position/pairwise-balanced recorded
rounds with no warmup: 84/84 processes pass.

## Timing scorecard

All changes below are candidate versus S0 and oriented so positive is faster.
The stable-six aggregate contains Sieve, Permute, Bounce, Richards, Towers and
RexxCPS. Base64 is retained but excluded as the already-declared noisy CAP-03
library/API lane.

| Candidate | Sieve | Permute | Bounce | Richards | Towers | RexxCPS | Stable-six |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| S1 | -0.154% clear adverse | -0.375% inconclusive | -0.627% clear adverse | -0.135% inconclusive | -0.220% inconclusive | +0.207% inconclusive | -0.204% |
| S1b | -0.051% inconclusive | -0.109% inconclusive | -0.152% inconclusive | +0.371% clear favourable | +0.826% inconclusive | +0.848% clear favourable | +0.375% |

The stable-six ratios are based on final per-cell medians, not the means of
paired percentages. Neither candidate crosses the 1% aggregate guard, and no
workload paired median crosses the 3% individual guard. Base64 paired medians
are +0.291% for S1 and +0.358% for S1b, but both intervals remain extremely
wide and non-selecting.

## Memory and artifact scorecards

Four-round process RSS medians versus S0 are:

- S1: +40,960 to +319,488 bytes (+0.22% to +1.74%);
- S1b: -311,296 to -475,136 bytes (-1.69% to -2.49%).

No RSS row is both more than 5% and more than 1 MiB adverse. Process RSS and
allocator-owned retention measure different things: S1b's lower committed
process pages do not remove the deterministic extra slab capacity that will be
owned by every worker.

The accepted first-verdict telemetry is reused because this formal panel uses
the exact same binaries and inputs. Its 21 S0/S1/S1b cells have zero allocation
failures, invalid frees and wrong-owner frees. S1 saves 196,608 retained bytes
and 41,240-134,232 peak-live capacity bytes per workload. S1b adds
589,824-917,504 retained bytes and changes peak-live capacity by zero.

All three `rxbvm` files are exactly 1,000,936 bytes. The hashes differ as
expected for their compile-time tables, but the artifact-size delta is zero.

## Host-state limitation

The campaign ran on AC with low-power mode off and without concurrent builds,
tests or benchmark campaigns. Pre/post load and power state are retained.
`pmset -g therm` was inadvertently omitted from the initial pre-state capture;
a mid-campaign and the post-campaign capture both report no thermal or
performance warning. This is disclosed rather than backfilled as a historical
pre-state observation.

## Evidence map

- `manifests/`: exact formal and governed-append cell definitions;
- `timing/`: every raw timing sample/output block, final paired uncertainty
  summary and stable-six ratios;
- `rss/`: raw four-round samples/output, absolute summaries and S0 deltas;
- `allocator/`: exact S0/S1/S1b subset of the accepted deterministic telemetry;
- `tools/summarize_geometry_paired.crexx`: evidence-local Level B reducer;
- `artifact-scorecard.csv`: exact product sizes and hashes;
- `pre-state.txt`, `campaign-mid-thermal.txt`, `post-state.txt` and
  `build-configs.txt`: host and build identity;
- `source-identities.csv`: exact source, tool, product and workload hashes;
- `COMMANDS.md`: exact reproduction commands; and
- `checksums.sha256`: recursive bundle integrity.

## Claim boundary

This is an Apple ARM64 product-lane geometry decision for unchanged V0/R0. It
does not claim cross-platform validation, a production allocator policy, an
accepted compact value, reclamation behavior, worker/thread safety, transport
semantics, `rxtvm` parity, spawn compatibility, or a resolution of Base64.
