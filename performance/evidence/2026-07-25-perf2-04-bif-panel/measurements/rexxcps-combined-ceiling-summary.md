# PERF2-04 cumulative RexxCPS exact-site ceiling

CF-COMB1 is a scratch-only composition of the independently proved exact-site
ceilings CAS-L2 (`UPPER`), SLC-CF1 (`SUBSTR`) and WRD-CF1 (`WORD`). It was
built from detached commit
`6567f0ba23f20623e01322f5a62323b2347ab09d` in
`/private/tmp/crexx-perf2-04.3k3Dfw/rexxcps-combined-src` with the exact B0-R
profiling-off Release compiler and assembler. No production source, BIF body,
compiler, RXAS/RXBIN contract, VM or ABI was changed.

## Composition and semantic audit

The three controls touch disjoint timed expressions in the same RexxCPS outer
kernel:

| family | exact selected sites per outer iteration | combined replacement | cross-family guard |
|---|---:|---|---|
| `UPPER` CAS-L2 | four calls per `lvar`, 56 calls total | four canonical constant results passed in original argument order | final values for all four result variables |
| `SUBSTR` SLC-CF1 | 14 compound assignments plus 28 false predicates, 42 calls total | `"56"` assignment and removal of the provably false `"1" = "9"` branch | canonical fallback results plus final compound value `"58"` after both downstream increments |
| `WORD` WRD-CF1 | 28 `word(key1,1) = "?"` predicates | removal after proving `key1 = "Key Bee"` | ten-case original-WORD versus predicate-first Unicode/blank guard |

`SUBSTR` owns the compound value but does not alter `key1`; `WORD` consumes
that stable `key1`; the UPPER arguments/results are independent of both. The
combined dual-VM guard therefore checks both each family's semantics and the
only relevant cross-family data/order interaction. The patch applies cleanly
to exact HEAD and `git diff --check` passes.

This proves only the selected constant, signal-free, TRACE-off cells. The
scratch source cannot reproduce the original imported inlined bodies'
`.srcstep`, operand TRACE, function-event or source identities. A production
compiler fold must preserve/synthesize evaluation, signal and observation
order or fail closed. The complete Level B sources remain the fallback and
behavioral documentation.

## Artifact identity and static cost

- scratch source: `b735ae409e90f232b4c14f6f3cb364e84ca22bd944c6feb40e45a8338d485dc9`
- patch: `a0b6ce10f0a73d93b35e3284ce9c4afa96e04fe7813e0e58ec9224def95fffe2`
- RXAS: `a74150aecfced3c003f5be9d9bc9651a22c4036d92c7e9bcbb9112b886c69904`
- RXBIN: `209cefdb8ca88768ffe46e38a0d7c7b646182db0afdf1991e7a4f1395caeec15`
- disassembly: `103bb22a0788c340d752f227ad7adf9d58d464966714b0283d4cbefe2dff2dc4`
- B0-R current RXBIN: `9b535403baddc5b7076d5eb23027a296d982660546bcecc74d3784bb2fa23741`

The maintained Level B artifact summarizer reports:

| cell | program bytecode instructions | maximum locals | RXBIN bytes | module-set instructions | module-set bytes |
|---|---:|---:|---:|---:|---:|
| CF-C0 current | 1,402 | 105 | 77,438 | 55,626 | 937,407 |
| CF-COMB1 | 2,172 | 109 | 113,938 | 56,396 | 973,907 |

The apparent +770 instructions and +36,500 RXBIN bytes are scratch guard cost,
chiefly ten post-timer WORD equivalence cases that retain both the original
Level B call and the hand-equivalent predicate. They are not a production-fold
size estimate. The exact timed counts and wall result below exclude their
placement outside the benchmark timer while still retaining their fixed
startup/exit cost in raw profiles and process executions.

## Dual-VM correctness and normalized machine work

Both ordinary Release VMs passed the noncanonical smoke, all combined guards
and canonical RexxCPS correctness with zero stderr. The maintained Level B
profile summarizer reports all four schema-5 count profiles complete. Counts
are normalized over both calibrated trials using
`(initial_count + effective_count) * 100` outer iterations.

| VM | cell | instructions/outer | calls | frames | value copies | branches | allocations |
|---|---|---:|---:|---:|---:|---:|---:|
| `rxvm` | current | 5,259.941373 | 70.988824 | 70.989020 | 141.083922 | 1,066.541569 | 266.163137 |
| `rxvm` | CF-COMB1 | 2,812.821111 | 14.509697 | 14.509798 | 112.075960 | 457.509091 | 153.118283 |
| `rxbvm` | current | 5,262.067143 | 71.029184 | 71.029388 | 141.168980 | 1,067.136531 | 266.251429 |
| `rxbvm` | CF-COMB1 | 2,813.374124 | 14.520206 | 14.520309 | 112.098144 | 457.663918 | 153.141340 |

CF-COMB1 removes 2,447.120262 normalized instructions per outer iteration on
`rxvm` (-46.523717%) and 2,448.693019 on `rxbvm` (-46.534811%). Calls/frames
fall by about 56.5 and allocation requests by about 113 per iteration; this is
not merely a branch-count effect.

| VM/cell | STRUPPER | SUBSTRING | SETSTRPOS | FNDNBLNK | FNDBLNK | STRLEN |
|---|---:|---:|---:|---:|---:|---:|
| rxvm current | 56.277647 | 70.244314 | 70.244314 | 28.163333 | 28.000196 | 143.158627 |
| rxvm CF-COMB1 | 0.143030 | 0.126869 | 0.126869 | 0.085960 | 0.001414 | 29.628687 |
| rxbvm current | 56.288980 | 70.254490 | 70.254490 | 28.170000 | 28.000204 | 143.287959 |
| rxbvm CF-COMB1 | 0.145979 | 0.129485 | 0.129485 | 0.087732 | 0.001443 | 29.662268 |

The near-zero residuals are fixed setup/post-timer guard work after
normalization. The combined control removes all timed UPPER conversions, both
selected SUBSTR slices and both WORD scans/slice for every selected execution.
This confirms that the three independently selected ceilings do not consume
the same machine work: their reductions survive composition. The residual
STRLEN is chiefly the separate timed LENGTH control and unrelated fixed work.

## Ordinary profiling-off Release cumulative wall result

The maintained Level B matrix driver ran the current and combined exact B0-R
images serially, with two warmups and seven recorded samples per VM. All 36
invocations exited zero; all 28 recorded samples passed; every candidate run
passed the combined guard; stderr is empty. The host remained on AC power at
80% battery with no thermal/performance warning.

| VM | current median clauses/s | CF-COMB1 median | combined/current | favorable recorded rounds | relative MAD current/combined | span current/combined |
|---|---:|---:|---:|---:|---:|---:|
| `rxvm` | 29,293,128 | 38,924,099 | +32.877919% | 7/7 | 0.347054% / 1.666307% | 3.675497% / 6.714041% |
| `rxbvm` | 27,502,423 | 36,963,282 | +34.400093% | 7/7 | 0.458192% / 0.577616% | 2.287744% / 2.227408% |

The maintained variability rule recommends no rerun. This direct combined
matrix is the cumulative ceiling result; percentages from the three separate
family matrices should not be arithmetically summed because they were captured
in different serial batches with different scratch guard/image shapes.

## Placement implication and stop

The cumulative result strengthens, but does not broaden, the three individual
placement decisions: exact constant/proof folding belongs in the compiler,
with the complete Level B BIF bodies retained as the universal fallback and
semantic documentation. The disjoint machine-work retirement and +32.9% to
+34.4% end-to-end ceiling justify an ordered compiler-fold ladder if Adrian
selects it. They do not justify native ownership, a public RXAS opcode, a VM
assist, a production edit, or a formal portfolio run.

## Retained paths

All relative paths are beneath
`performance/evidence/2026-07-25-perf2-04-bif-panel/`.

- patch: `pocs/rexxcps-combined-ceiling.patch`
- RXAS/RXBIN/disassembly and compile logs:
  `pocs/rexxcps-combined-ceiling-artifacts/`
- dual-VM smoke: `measurements/rexxcps-combined-smoke/`
- B0-P raw counts: `measurements/rexxcps-combined-counts/`
- maintained profile summaries: `measurements/rexxcps-combined-profile-summary/`
- normalized count table: `measurements/rexxcps-combined-normalized-counts.csv`
- maintained static inventory: `measurements/rexxcps-combined-static.csv`
- serial Release matrix/raw outputs/state: `measurements/rexxcps-combined-timing/`
- exact controls: `controls/rexxcps-combined-{artifacts,profiles,poc-matrix}-v1.txt`
