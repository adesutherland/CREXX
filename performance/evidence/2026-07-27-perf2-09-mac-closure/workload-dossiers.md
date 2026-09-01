# PERF2-09 Apple workload dossiers

These dossiers join the formal same-session outcome to the latest accepted
mechanism evidence. Exact formal sources, images, products and comparator
hashes are in `artifacts.csv`; exact samples are under `timing/`, `rss/` and
`lifecycle/`. Mechanism counts are not remeasured from the profiling-off
timing product: unchanged rows refer to the checksum-retained PERF2-06/07
schema-5 capture, while Richards and Permute use the accepted V1R01-R1 exact
post-placement proof.

## Sieve

- **Contract:** common equivalent port, equal work 5,500, deterministic count;
  source/RXAS/RXBIN hashes are the Sieve entries in `artifacts.csv`.
- **Outcome:** 5,078.931/3,758.560 work/s; 7.214291x/5.338790x ooRexx and
  1.892228x/1.400305x NetRexx. The threaded VM clears both strong bands; the
  bytecode VM misses the NetRexx 1.50 band by 1.071x.
- **Current work:** 6,825,375 instructions, zero calls, 52 copy operations/10
  bytes, 506,584 clear/reset/destroy operations/120,209 bytes, 8,319
  allocations/4,231,352 bytes, frame high-water one. No selected hot
  value/reference mechanism has meaningful leverage.
- **Other dimensions:** peak RSS 16.59/16.61 MiB; source/RXAS/RXBIN
  983/9,774/4,864 bytes.
- **Disposition:** accepted generic loop/layout guard. Reject candidate timing
  without a new nonzero current mechanism count.

## Permute

- **Contract:** common equivalent recursive/object port, equal work 5,000;
  exact hashes are the Permute entries in `artifacts.csv`.
- **Outcome:** 2,455.036/2,151.502 work/s; 8.005043x/7.015322x ooRexx but
  0.536613x/0.470268x NetRexx. NetRexx parity needs another 1.864x/2.126x.
- **Current work:** accepted V1R01-R1 proof at work 50 has 11,898,926
  instructions, 181,202 copies/1,448,810 bytes, three static copy sites and
  866,199 allocation requests/124,072 bytes. That is the proof-wide direct
  `§this` placement result, down 98.23% in copies from the pre-candidate
  product. The prior top owner was recursive receiver `copy_value` work.
- **Other dimensions:** peak RSS 16.81/16.69 MiB; source/RXAS/RXBIN
  1,313/29,684/11,773 bytes.
- **Disposition:** preserve the accepted compiler-owned win. No residual
  candidate is selected by this Mac scorecard; NetRexx is an outcome target,
  not proof that a specific VM mechanism is safe.

## Bounce

- **Contract:** common equivalent object/reference port, equal work 4,200;
  exact hashes are the Bounce entries in `artifacts.csv`.
- **Outcome:** 3,592.595/2,727.942 work/s; 3.902513x/2.963270x ooRexx and
  1.707562x/1.296592x NetRexx. Only bytecode-VM versus NetRexx misses the
  strong band, by 1.157x.
- **Current work:** 21,139,726 instructions, 10,100 calls, 1,203,002
  copies/9,542,411 bytes, 10,100 moves/400,000 bytes, 920,170
  clear/reset/destroy operations/1,206,764 bytes and 260,520
  allocations/83,328,648 bytes. Residual optimized reference traffic includes
  510,000 `MKREF`, 4,222,600 `LINKATTR1`, 3,065,000 `UNLINK`, 1,000,000
  `UNLINKN` and 510,000 `MINLINK*`; current evidence does not prove one
  redundant ownership operation.
- **Other dimensions:** peak RSS 17.05/17.06 MiB; source/RXAS/RXBIN
  2,230/45,362/17,175 bytes.
- **Disposition:** guard. V1R02 remains unselected until exact owner/root/tree
  proof identifies a safe nonzero reduction; no mapping ledger is permitted.

## Richards

- **Contract:** common state-machine adaptation, equal work 20; exact hashes
  are the Richards entries in `artifacts.csv`.
- **Outcome:** 2.868/2.835 work/s; 0.267262x/0.264171x ooRexx and
  0.157815x/0.155990x NetRexx. It needs 3.742x/3.785x to ooRexx parity and is
  the largest qualified common deficit.
- **Current work and owner:** accepted V1R01-R1 proof at work one has
  8,615,245 instructions, 56,902,732 copies/451,730,841 bytes, 71 static copy
  sites and 735,010 allocation requests/172,258,936 bytes. The installed
  compiler placement removed 16,404,842 copies and 130,345,888 copied bytes,
  delivered about 21% in its ordinary Release verdict, and exhausted that
  proved receiver-materialization slice. Retained native samples and
  allocation high-water evidence place remaining cost in/under recursive
  `copy_value`, but do not by themselves prove another eliminable copy.
- **Other dimensions:** peak RSS 18.20/18.09 MiB versus ooRexx 17.09 MiB;
  source/RXAS/RXBIN 9,403/255,035/78,742 bytes.
- **Disposition and ceiling:** first re-attribute the accepted product's
  residual static/dynamic copy shapes and caller paths. An ideal parity ceiling
  is 3.74–3.79x, but no exact candidate owns that entire ceiling. Rejected
  frame reset, hot ledger, pooling/slab and broad-layout designs remain closed.

## Base64

- **Contract:** common RFC 4648 arithmetic codec and exact 1,024-byte
  round-trip/length/checksum, equal work 2,500; exact hashes are the Base64
  entries in `artifacts.csv`.
- **Outcome:** 1,499.625/1,510.261 work/s; 0.719817x/0.724922x ooRexx and
  0.827447x/0.833316x NetRexx. Both cREXX series remain noisy at `n=20` after
  the governed append; all observations remain included.
- **Current work:** 46,724,369 instructions, 500 calls, 2,221,507
  copies/18,789,128 bytes, 500 moves/512,000 bytes, 1,382
  clear/reset/destroy operations/1,075,088 bytes and 1,700
  allocations/608,160 bytes. The optimized image executes 2,907,503 `SCOPY`
  and 1,370,502 `STRLEN` operations. There is no current retained-
  representation hit/miss counter or proof that broad cache retention is
  valid.
- **Other dimensions:** peak RSS 16.88/16.89 MiB; source/RXAS/RXBIN
  4,437/114,978/38,233 bytes.
- **Disposition and ceiling:** needs 1.379–1.389x to ooRexx parity. Reconfirm
  the noisy lane before using a small delta and require exact reduced
  materialization work. CAP-03's future library API is separate and no
  native/SIMD or broad representation change is authorized.

## Towers

- **Contract:** separate qualified object/allocation lane. Each repetition
  creates one benchmark object and 14 disk objects and performs 8,191 recursive
  moves. Formal ooRexx source SHA-256 is
  `bb081b76306ce1d360f4e739e480e3e89ebceb31028326bc93910c8daa0267b9`.
- **Outcome:** elapsed medians for 100 repetitions are 3,631.729/3,707.652 ms
  versus ooRexx 1,191.427 ms: 0.328060x/0.321343x, requiring
  3.048x/3.112x to parity. The 35.204 ms NetRexx binary/JVM control is
  startup-dominated and receives no Rexx ratio.
- **Ownership/allocation:** the qualified ports preserve per-repetition object
  construction, mutable object links and method dispatch. The former ooRexx
  numeric-node/stem diagnostic is excluded. No current schema-5 payload-shape
  or allocation-lifetime capture was added for Towers, so pooling, slab or
  value-layout attribution would be speculative.
- **Other dimensions:** peak RSS 17.97/18.00 MiB versus ooRexx 17.08 MiB;
  source/RXAS/RXBIN 3,529/81,065/28,647 bytes.
- **Disposition:** qualified outcome target, outside the common aggregate.
  Obtain current shape/lifetime/high-water proof before selecting any V2/V5
  value or allocation owner; C2R03/V6R01 still require architecture approval.

## RexxCPS

- **Contract:** separate native-rate lane: cREXX disclosed 2.2d versus
  canonical ooRexx/Regina 2.2; NetRexx 2.2n is a labelled adaptation control.
  Exact source/image hashes are the RexxCPS entries in `artifacts.csv`.
- **Outcome:** 37.929/35.546 MCPS versus ooRexx 38.091: 0.995754x/0.933193x.
  Parity needs 1.004x/1.072x; the 1.50 band needs 1.506x/1.607x. Regina is
  32.158 MCPS and the NetRexx adapted control is 46.030 MCPS.
- **Current work:** optimized `rxvm` records 27,841,418 instructions, 143,642
  calls, 1,109,124 copies/48,425,897 bytes, 1,641,006
  clear/reset/destroy operations/71,902,353 bytes and 1,515,811
  allocations/274,924,728 bytes. The conversion stream contains 4,258,578
  `SCOPY`, 1,054,500 each `DCOPY`/`DTOS`, 788,500 `STOD`, 282,105 `STRLEN`
  and 1,197,006 `ITOS`. `rxbvm`'s calibrated count is lower and remains
  separately retained rather than normalized away.
- **Other dimensions:** peak RSS 17.98/18.00 MiB versus ooRexx 17.25 MiB;
  source/RXAS/RXBIN 12,180/195,576/68,446 bytes.
- **Disposition:** near-parity outcome guard, not the largest current deficit.
  Selective representation retention needs an exact validity contract and
  nonzero hit/miss ceiling; the V3-R01 correctness fix is not performance
  authorization.

## Approved no-ratio dossiers

- **Mandelbrot:** ooRexx ordinary decimal arithmetic gives 255/128 rather than
  required binary64 checksums 191/50 at sizes 500/750. Excluded, with no
  imported binary helper.
- **Storage:** correct but cREXX necessarily creates a `StorageNode` plus
  `.object[]` for every logical node. Diagnostic exclusion; CAP-02 is deferred
  to an explicit post-Release 1 Level G ownership/container decision.
- **List:** correct but cREXX's weak references require an owning arena absent
  from the comparator object graphs. Diagnostic exclusion.
- **JSON:** correct result but cREXX reparses string/path queries while the
  comparators construct distinct DOMs. Diagnostic exclusion; CAP-01 is
  deferred to an independently approved parse/result/access design.
- **Lifecycle:** exact phases are reported separately. CAP-04's unavailable
  public pure-load boundary is an approved exclusion, not an imputed phase.
