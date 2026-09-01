# Initial concurrency Mac performance diagnostic

Date: 2026-08-16  
Branch: `develop`  
Frozen control: `b6a16dc3ae235eb959e926326fff2be8eb8e0ead`  
QA candidate: `5bede182efa93dfd8ee79680f8b1232dcdf39f5e`

Status: **complete diagnostic; not a formal baseline because the host was on
battery throughout the campaign.**

## Result

The comparison used ordinary profiling-off Release `profile-20` VMs, one
shared current RXBIN/library set and balanced same-session control/candidate
pairs. Correctness passed for every process and no sample was removed.

The task-launch panel reached the 36-pair noise ceiling. All four cells remain
noisy/inconclusive and none has an adverse guard hit. Median changes, oriented
so positive is faster, are:

| Cell | `rxbvml` | `rxtvml` |
| --- | ---: | ---: |
| tiny-task latency | +0.874% | +1.575% |
| task throughput | +0.717% | +0.331% |

The ordinary single-thread panel also reached 36 pairs. Bounce on `rxtvm` is
clearly favourable; all other cells remain noisy/inconclusive. The 36-pair
Richards `rxtvm` mean crossed the adverse 3% guard at +3.389%, while its median
was 0.613% faster and its mean interval spanned -3.628% to +10.407%. The exact
unchanged 12-pair confirmation did **not** reproduce a guard hit: mean -0.984%,
median -1.156%, interval -3.652% to +1.684%.

Because the power log records battery use from before the first timing block
through the final confirmation, these numbers are diagnostic observations,
not a governed performance verdict. The earlier accepted AC-power task and
single-thread baselines remain authoritative.

**2026-08-18 disposition:** Adrian waived an unchanged quiet-AC replay as
unnecessary. Mac correctness, sanitizer, stress, Release, install and package
proof pass; this diagnostic found no confirmed adverse guard; and later
concurrency production changes carried clean single-thread Release guards. The
battery samples remain diagnostic and are not promoted. Reopen timing only for
a relevant source change, a concrete evidence inconsistency or a new governed
performance question.

## Product shape

Relative to the frozen feature baseline, the QA candidate is 96 bytes larger
for `rxbvm` and 112 bytes larger for `rxtvm`. The intervening production edits
are the provider completion-depth correction and a private executor cache-count
test accessor; neither changes the dispatch hot loop. The diagnostic timings
cannot attribute any cell movement to either edit.

## Evidence map

- `manifest.txt` and `richards-rxtvm-confirmation-manifest.txt`: exact balanced
  schedules;
- `timing/`: raw task, single-thread and confirmation samples plus captured
  output and correctness status;
- `task-summary-36.csv`, `paired-summary-36.csv` and
  `richards-rxtvm-confirmation.csv`: final decision tables;
- `summarize_*.crexx`: Level B reducers used for the retained summaries;
- `provenance.txt`, `power-log-extract.txt` and `power-settings.txt`: host,
  build, power and interpretation boundary;
- `artifacts.sha256` and `artifact-sizes.txt`: exact product/workload identity;
  and
- `qa/`: build, runner and reducer logs.

Exact replay commands are in [`COMMANDS.md`](COMMANDS.md).
