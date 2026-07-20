# NR-09 corrected-pruning ordinary-Release verdict

Status: **complete; mandatory stop for Adrian before broad QA or commit**.

## Product boundary

This is the first ordinary profiling-off Release verdict after the approved
26-form pruning and four mapping-order corrections. All products use the
canonical RexxCPS 2.2d source at SHA-256
`2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`.

- **A:** retained accepted VM, accepted RXBIN and accepted library.
- **B:** corrected candidate VM, accepted RXBIN and accepted library. This is
  the old-bytecode infrastructure and compatibility control.
- **C:** corrected candidate VM, corrected candidate RXBIN and corrected
  candidate library.

Both products are ordinary `CMAKE_BUILD_TYPE=Release`, `-O3 -DNDEBUG`,
`CREXX_VM_PROFILING=OFF` builds. The corrected RXAS hash is
`9715bbd495821cd80fc12b99beecc8fb05adada691c50b0e87d5a7026bf37b6b`;
the final-path RXBIN hash is
`942339800cad54f7594bfdee029329754b775e0695c8e49f1ea74b6001d4fe4f`.
Exact candidate VMs and library are retained under `artifacts/candidate/`.

The accepted and corrected RXAS absolute paths are both 147 characters, so
RXBIN description metadata cannot introduce the path-length imbalance caught
in the first pilot. That non-decisive pilot is preserved under
`pilot-long-path/` and is not used below.

## Schedule and correctness

The campaign uses the same drift-resistant schedule as the accepted
rebaseline: six warmups followed by 12 recorded rounds per VM, with all six
A/B/C orders occurring twice in reverse-symmetric order and VM starting order
alternating by round. All **78/78** executions pass the canonical output,
provenance and `effective_count=100` checks; the recorded set is 72 samples.

The host slowed in the later rounds. Those samples are retained. A/B/C moved
within the same rounds, and the reverse-symmetric paired analysis is the
decision statistic rather than independent absolute medians.

## Result

Positive CPS is faster; negative elapsed time is faster. Intervals are the
approximate two-sided 95% Student-t interval around the mean of the 12 paired
percentage deltas.

| Comparison | `rxvm` paired CPS median (mean 95% interval) | `rxbvm` paired CPS median (mean 95% interval) | Paired elapsed median `rxvm` / `rxbvm` |
| --- | ---: | ---: | ---: |
| B versus A: VM/old-RXBIN infrastructure | -0.182% (-1.264% to +1.113%) | +0.239% (-0.821% to +1.716%) | +0.222% / -0.239% |
| C versus B: corrected generated product | +0.376% (-0.633% to +1.179%) | **+0.836%** (+0.198% to +1.494%) | -0.359% / **-0.807%** |
| C versus A: complete corrected batch | **+0.262%** (-0.489% to +0.843%) | **+0.937%** (-0.097% to +2.689%) | **-0.295% / -0.893%** |

The corrected product beats B in 8/12 paired CPS and elapsed comparisons on
both VMs. Complete C beats A in 7/12 comparisons on both VMs. The `rxbvm`
C-versus-B interval is wholly positive; both complete-product intervals cross
zero.

The verdict is therefore **no regression and neutral-to-slightly-positive**,
not a proved material overall speedup. In the canonical RXAS, the corrected
batch has 1,718 executable instruction lines versus 1,837 accepted (-6.478%)
and 1,702 in the provisional all-enabled product (+16). The pruning restores
some dispatch in exchange for removing 26 low-value/ugly forms while retaining
a positive product direction.

## Decision boundary

This completes the mandatory first Release verdict only. Broad Debug CTest,
sanitizer, install/package proof, audited golden refresh and the production
batch commit remain deferred until Adrian accepts this verdict.

Rerun from the repository root with the already-built exact Release product:

```sh
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/prunedrun1/run_corrected_rebaseline.zsh
```

The runner refuses to overwrite `samples.csv`. It retains raw stdout/stderr,
exact product hashes, provenance, cell/position summaries, every paired delta,
the paired summary, timed candidate artifacts and the timed candidate image.
