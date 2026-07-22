# NR-09 final corrected-product Release refresh

Status: **78/78 executions pass; the final corrected product shows no
regression and is positive in both VM modes**.

This is the post-QA ordinary profiling-off Release refresh of the accepted
26-form-pruned NR-09 product. It uses the same drift-controlled three-cell
design as the earlier corrected rebaseline:

- A: reconstructed accepted VM, accepted RXBIN and accepted library;
- B: final candidate VM, accepted RXBIN and accepted library; and
- C: final candidate VM, final candidate RXBIN and final candidate library.

There are six warmups and 72 recorded executions. The 12 recorded rounds use
all six A/B/C permutations twice for each VM, with alternating VM order. Every
execution passes the canonical RexxCPS contract.

## Result

| VM | B/A infrastructure paired median CPS | C/B corrected-product paired median CPS | C/A complete-product paired median CPS | C/A 95% interval |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | -0.846% | +1.345% | **+1.385%** | -0.748% to +3.582% |
| `rxbvm` | +0.934% | +2.106% | **+2.868%** | +0.162% to +3.667% |

Paired process elapsed medians agree with the complete-product direction:
-1.311% for `rxvm` and -2.828% for `rxbvm`. The `rxvm` interval crosses zero;
the `rxbvm` complete-product interval is wholly positive. This is evidence of
no regression and a positive final sample, not grounds to generalize the
larger point estimates beyond this noisy campaign.

Cell B also supplies 26 successful executions of the final VMs against the
exact accepted old RXBIN/library (one warmup and 12 recorded samples for each
VM). The independent installed-tree compatibility smoke in `../qa-closeout/`
adds one further canonical pass per VM.

## Provenance and raw evidence

- `provenance.txt` records source state, host/toolchain, build options, exact
  artifact hashes and the schedule.
- `samples.csv` and `raw/` retain all 78 observations and raw outputs.
- `cell-summary.csv`, `paired-deltas.csv`, `paired-summary.csv` and
  `position-summary.csv` retain the derived statistics.
- `sha256sums.txt` binds the runner, source, binaries, images, samples and
  summaries.
- `run_final_rebaseline.zsh` and `summarize_corrected.py` rerun the campaign.

The final candidate RexxCPS RXAS and library hashes are unchanged from the
accepted pruned implementation (`9715bbd495821cd80fc12b99beecc8fb05adada691c50b0e87d5a7026bf37b6b`
and `8a658a63019aedb99244ad71206882ef8e488aa4f79fbab64a77ed84654b0049`).
The final candidate RXBIN hash is
`7a36cb4dee471daab0b406dc3788058746ab9f55bf3e21b431eda27d36250f9e`.
