# PERF3-02-R1 infrastructure-enabled option repanel

Status: **complete evidence; production-ladder decision required**

## Outcome

The locked C2E2-P1 storage/CFG infrastructure is preserved exactly, and the
full C0-C4 copy/ownership investigation is replayable from one compiler source
state. Correctness rejects both forms containing the broad C1a-R1 no-write
rule. The independently proved C1a-R2, C1b-R1 and C1c-R1 facts compose without
interference.

On the same-session ordinary Release panel, lower elapsed is better:

| Target | Candidate | `rxvm` paired median | `rxbvm` paired median |
| --- | --- | ---: | ---: |
| Richards | C1a-R2 | -9.04% | -9.11% |
| Richards | C1b-R1 | -44.28% | -44.01% |
| Richards | C1a+C1b | -53.55% | -52.57% |
| Towers | C1c-R1 | -18.92% | -18.97% |

Every row has 12/12 favorable pairs in both VMs and a wholly favorable 95%
mean interval. C1a and C1c reproduce the retained 2026-07-31 authoritative
results within 0.68 percentage points. C1abc emits exactly the C1ab Richards
RXAS and the C1c Towers RXAS, so those measurements are its workload-specific
result; running separately assembled duplicate images would add no mechanism.

The fresh C0 controls are faster than the retained consolidated session by
4.91% to 6.72%. They are therefore used only inside same-round pairs. The old
C1a/C1c timings remain historical authority as instructed.

## Correctness and option disposition

- Masks 0, 1, 2, 3, 4 and 7 pass every applicable benchmark and focused
  receiver/reference test.
- Mask 8 repeats the original correctness failure: optimized Richards returns
  `0/1` instead of `23246/9297` on both VMs.
- Mask 14 passes the small benchmark screen but fails the structural gate by
  crossing a call-bearing receiver guard. It is rejected before timing.
- All variant no-opt Richards/Towers images and all optimized Permute, Bounce
  and Sieve images are identical, confirming the intended narrow scope.
- The formal timing block itself passed 156/156 executions.

## Infrastructure-enabled C2/C3/C4 result

The locked storage service exposes more point identity but does not by itself
prove a full-copy rewrite. After C1a+C1b, Richards still has 59 full copies
with exact base storage; after C1c, Towers has 18. Every one still lacks an
independent ownership/destruction, value-availability, source-lifetime, TRACE
and all-continuation proof. C2 therefore remains analysis-only and untimed.
There are zero current alias-self-copy or residual swap-roundtrip success
paths.

C3 remains the retained 32,557 scalar operations with zero logical payload
bytes and is immaterial. C4 is met exactly by C1a/C1b/C1c: the proved sites
perform no full receiver copy, allocation or recursive traversal.

## Decision

The evidence supports selecting C1abc as the production ladder: retain the
exact C1b guard proof and add the independent C1a unused-receiver and C1c
isolated-formal proofs. That gives the measured C1ab Richards result and C1c
Towers result without a cross-workload regression in this bounded panel.

Selection is not made by this bundle. On approval, the next step is to remove
the disposable build mask and the broad negative branch, make C1a+C1b+C1c the
single production logic, retain the replay diff here, rebuild ordinary Release
and perform the mandatory focused closeout path. C2 ownership work and
assembler-time rework remain separate roadmap items.

## Evidence map

- `variants.csv`, `correctness.csv` and `raw/` retain every positive and
  rejected option outcome.
- `storage-identity.csv` and `c2-c4-disposition.csv` record the new
  infrastructure-enabled analysis.
- `manifest-v1.txt`, `run-v1/`, `paired-effects.csv`, `pre-run.txt` and
  `post-run.txt` retain the formal timing campaign.
- `historical-reconciliation.csv` and `baseline-drift.csv` preserve the old
  authority and label new C0 use.
- `compiler-option-source.diff`, `input-hashes.csv` and `provenance.md` make
  every mask rebuildable after production cleanup.
