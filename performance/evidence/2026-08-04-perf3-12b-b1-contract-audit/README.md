# PERF3-12B B1 compound-tail contract audit

Date: 2026-08-04

Status: complete and accepted; exact native-stem metadata correction retained

## Outcome

The retained PERF3-12 and accepted X1 evidence is valid. The canonical source,
no-opt RXAS, no-opt RXBIN and accepted optimized RXBIN are byte-identical to
the retained artifacts. The optimized RXAS differs from the pre-X1 PERF3-12
capture exactly because X1 is present. The current library has bounded declared
drift from the X1 capture because the later queue module is now included; the
same current library must be used for every PERF3-12B control and candidate.

Five generated static sites construct `"Key Bee." || lvar` before native stem
access. Their exact fixed-work weight is 2,240,000 concatenations. The current
lowering has no live register for the unpunctuated left segment `"Key Bee"`, so
a segmented candidate must account for stable-left provisioning rather than
claiming the hand-equivalent ceiling for free.

The native stem handler audit closes a precondition for the comparison:

- `STEMGET`, `STEMSET`, `STEMGET2` and `STEMSET2` can signal
  `UNICODE_ERROR|FAILURE`, before logical writes;
- `INVALID_ARGUMENTS` is not reachable because none of the four calls an
  indexed native-stem operation;
- failures preserve the get destination and logical stem contents, although
  a failed or successful lazy initialization/growth may retain private backing
  capacity;
- the stem is external mutable state, so these operations are not
  success-stable; and
- one-part validation of a joined key is not generally equivalent to
  validating both segments independently. The measured sites are eligible
  only because the left literal and the `ITOS`-derived right segment are each
  valid UTF-8.

The metadata correction and new allocation-failure fixtures are intentionally
small and independent of either S1 segmented selection or H1 loop reuse. No
compound-tail rewrite is installed by this audit.

## Authority and scope

- Published starting product: `965b461d813f6042063ee786d8d00cea870da096`.
- PERF3-12B control commit: `6033cc2a7bed8801716a760d7de7298341efee59`.
- Branch: `codex/perf3-12b-compound-tail`.
- Canonical source: `tests/benchmarks/rexxcps_levelb.crexx`, SHA-256
  `2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`.
- Host: Apple M5, Darwin 25.5.0 arm64, macOS 26.5.2, 10 logical CPUs.
- Toolchain: CMake 4.3.2 and Ninja 1.13.2.
- Product: ordinary profiling-off Release artifacts. The scale observations
  are single diagnostic samples, not benchmark timing claims.

## Evidence map

- `artifacts.csv`: retained and current product identities.
- `analysis/site-map.csv`: exact generated sites, provenance and fixed-work
  weights.
- `analysis/native-stem-contract.md`: handler-level signal, failure, UTF-8,
  storage and alias findings.
- `validation/baseline-and-scale.txt`: checksum audit, focused pre-edit test
  result and unchanged-input assembler scale.
- `validation/first-release-verdict.txt`: added after the provisional metadata
  correction is rebuilt and compared.
- `checksums.sha256`: recursive closure excluding itself.

## Interpretation boundary

The 2,240,000 concatenations are a dynamic upper bound for segmented selection,
not a wall-clock claim. B1 and B2 remove overlapping work. This audit proves
the instruction contracts needed to compare them; it does not select a route,
authorize a new opcode, or make the `C` TRACE event disposable.
