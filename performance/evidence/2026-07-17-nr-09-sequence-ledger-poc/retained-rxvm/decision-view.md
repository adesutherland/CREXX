# NR-09 RXSEQ candidate decision view

Deterministic post-processing of revision-labelled retained RXSEQ evidence. Dynamic counts are bounded observations from each exact cell; site/module columns are per-entry observations and maxima, not a guessed cross-image union. No row is an accepted fusion.

## Evidence revisions

| Revision | Role | Input rows |
|---|---|---:|
| nr05-6a064499f327-rxseq-schema4 | historical | 14009 |
| rexxcps-2.2d-baseline-e748621d1 | current | 5306 |
| rexxcps-2.2d-nr08-63743eb90e97 | candidate | 5005 |

Unique revision-pattern ledger rows: 11332. Stable identities selected by global/per-workload deduplication: 76.

| Reviewed | Unreviewed | Unsafe | Subsumed | Candidate |
|---:|---:|---:|---:|---:|
| 9928 | 1404 | 9558 | 322 | 48 |

`unsafe` is a mechanical fail-closed classification (unknown/conservative effects or control/alias/reference/throw/opaque risk), not proof that a future transform is impossible. `candidate` still requires the recorded liveness and interrupt-equivalence proof.

## Global top 25

Ordering is workload count, entry/mode breadth, maximum per-entry sites/modules, dynamic count, shorter window, then normalized pattern.

| Rank | ID | Workloads | Modes | Max sites | Max modules | Count | Status | Pattern |
|---:|---|---:|---:|---:|---:|---:|---|---|
| 1 | seq-2-1555722586 | 11 | 2 | 52 | 14 | 16434838 | unsafe | SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) |
| 2 | seq-2-3448147170 | 11 | 2 | 52 | 14 | 16434838 | unsafe | SETNUMFRM_INT(c1) \| SETNUMCAS_INT(c1) |
| 3 | seq-2-2063277697 | 11 | 2 | 52 | 14 | 16434838 | unsafe | SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) |
| 4 | seq-3-505676515 | 11 | 2 | 52 | 14 | 16434838 | unsafe | SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) \| SETNUMFRM_INT(c3) |
| 5 | seq-3-2040868729 | 11 | 2 | 52 | 14 | 16434838 | unsafe | SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) \| SETNUMCAS_INT(c2) |
| 6 | seq-4-2765130742 | 11 | 2 | 52 | 14 | 16434838 | unsafe | SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) \| SETNUMFRM_INT(c3) \| SETNUMCAS_INT(c3) |
| 7 | seq-2-2810718276 | 11 | 2 | 48 | 12 | 16290679 | unsafe | SETNUMCAS_INT(c1) \| SETNUMSTD_INT(c1) |
| 8 | seq-3-3769039113 | 11 | 2 | 48 | 12 | 16290679 | unsafe | SETNUMFRM_INT(c1) \| SETNUMCAS_INT(c1) \| SETNUMSTD_INT(c1) |
| 9 | seq-4-1598304623 | 11 | 2 | 48 | 12 | 16290679 | unsafe | SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) \| SETNUMCAS_INT(c2) \| SETNUMSTD_INT(c2) |
| 10 | seq-2-2151719822 | 11 | 2 | 45 | 4 | 1948338 | candidate | LOAD_REG_INT(r1,c1) \| LOAD_REG_INT(r2,c2) |
| 11 | seq-2-1921843141 | 11 | 2 | 35 | 9 | 9961515 | unsafe | ILT_REG_REG_INT(r1,r2,c1) \| BRF_ID_REG(c2,r1) |
| 12 | seq-2-3501308369 | 11 | 2 | 35 | 5 | 3060430 | unsafe | ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r2) |
| 13 | seq-2-3405515001 | 11 | 2 | 22 | 1 | 584 | unsafe | MINATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_INT(r2,r1,c1) |
| 14 | seq-2-1096152183 | 11 | 2 | 18 | 6 | 19623744 | unsafe | IGT_REG_REG_REG(r1,r2,r3) \| BRT_ID_REG(c1,r1) |
| 15 | seq-2-2591213324 | 11 | 2 | 6 | 1 | 160 | unsafe | ITOS_REG(r1) \| CONCAT_REG_STRING_REG(r2,c1,r1) |
| 16 | seq-2-1613512688 | 11 | 2 | 5 | 2 | 10174 | unsafe | LOAD_REG_INT(r1,c1) \| GETATTRS_REG_REG_INT(r2,r3,c2) |
| 17 | seq-2-18336876 | 11 | 2 | 4 | 2 | 248 | unsafe | STOI_REG(r1) \| ICOPY_REG_REG(r2,r1) |
| 18 | seq-2-4234099218 | 11 | 2 | 2 | 1 | 24 | unsafe | LINKATTR1_REG_REG_INT(r1,r2,c1) \| STOI_REG(r1) |
| 19 | seq-2-3536275467 | 11 | 2 | 2 | 1 | 24 | unsafe | SCONCAT_REG_REG_STRING(r1,r1,c1) \| ITOS_REG(r2) |
| 20 | seq-3-1769762332 | 11 | 2 | 2 | 1 | 24 | unsafe | LINKATTR1_REG_REG_INT(r1,r2,c1) \| STOI_REG(r1) \| ICOPY_REG_REG(r3,r1) |
| 21 | seq-3-311949687 | 11 | 2 | 2 | 1 | 24 | unsafe | MINATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_INT(r2,r1,c1) \| STOI_REG(r2) |
| 22 | seq-3-3596904615 | 11 | 2 | 2 | 1 | 24 | unsafe | STOI_REG(r1) \| ICOPY_REG_REG(r2,r1) \| UNLINK_REG(r1) |
| 23 | seq-4-186090217 | 11 | 2 | 2 | 1 | 24 | unsafe | LINKATTR1_REG_REG_INT(r1,r2,c1) \| STOI_REG(r1) \| ICOPY_REG_REG(r3,r1) \| UNLINK_REG(r1) |
| 24 | seq-4-448602274 | 11 | 2 | 2 | 1 | 24 | unsafe | MINATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_INT(r2,r1,c1) \| STOI_REG(r2) \| ICOPY_REG_REG(r3,r2) |
| 25 | seq-2-3089514224 | 11 | 2 | 2 | 1 | 23 | unsafe | BRF_ID_REG(c1,r1) \| MINATTRS_REG_INT(r2,c2) |

## Top 10 per workload after stable-ID dedup

| Workload | Rank | ID | Count | Site observations | Module observations | Pattern |
|---|---:|---|---:|---:|---:|---|
| sieve | 1 | seq-2-2151719822 | 302 | 8 | 2 | LOAD_REG_INT(r1,c1) \| LOAD_REG_INT(r2,c2) |
| sieve | 2 | seq-2-1590753747 | 2106800 | 6 | 2 | SETATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_REG(r2,r1,r3) |
| sieve | 3 | seq-2-1096152183 | 1000202 | 6 | 2 | IGT_REG_REG_REG(r1,r2,r3) \| BRT_ID_REG(c1,r1) |
| sieve | 4 | seq-2-3635382226 | 1606900 | 4 | 2 | ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r1) |
| sieve | 5 | seq-2-4110644254 | 1606900 | 4 | 2 | LINKATTR1_REG_REG_REG(r1,r2,r3) \| LOAD_REG_INT(r4,c1) |
| sieve | 6 | seq-2-401038279 | 1606900 | 4 | 2 | LOAD_REG_INT(r1,c1) \| ICOPY_REG_REG(r2,r1) |
| sieve | 7 | seq-3-3212259269 | 1606900 | 4 | 2 | LINKATTR1_REG_REG_REG(r1,r2,r3) \| LOAD_REG_INT(r4,c1) \| ICOPY_REG_REG(r1,r4) |
| sieve | 8 | seq-3-3245642035 | 1606900 | 4 | 2 | LOAD_REG_INT(r1,c1) \| ICOPY_REG_REG(r2,r1) \| UNLINK_REG(r2) |
| sieve | 9 | seq-3-635106007 | 1606900 | 4 | 2 | SETATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_REG(r2,r1,r3) \| LOAD_REG_INT(r4,c2) |
| sieve | 10 | seq-4-3622087100 | 1606900 | 4 | 2 | LINKATTR1_REG_REG_REG(r1,r2,r3) \| LOAD_REG_INT(r4,c1) \| ICOPY_REG_REG(r1,r4) \| UNLINK_REG(r1) |
| permute | 1 | seq-2-793582563 | 5039000 | 25 | 2 | UNLINK_REG(r1) \| UNLINK_REG(r2) |
| permute | 2 | seq-2-2073917396 | 4031200 | 20 | 2 | LINKATTR1_REG_REG_INT(r1,r2,c1) \| SETATTRS_REG_INT(r1,c2) |
| permute | 3 | seq-2-2413302004 | 4030000 | 16 | 2 | IADD_REG_REG_INT(r1,r2,c1) \| LINKATTR1_REG_REG_REG(r3,r4,r1) |
| permute | 4 | seq-2-249145866 | 4030000 | 16 | 2 | SETATTRS_REG_INT(r1,c1) \| IADD_REG_REG_INT(r2,r3,c2) |
| permute | 5 | seq-3-3409788275 | 4030000 | 16 | 2 | LINKATTR1_REG_REG_INT(r1,r2,c1) \| SETATTRS_REG_INT(r1,c2) \| IADD_REG_REG_INT(r3,r4,c3) |
| permute | 6 | seq-3-4129899544 | 4030000 | 16 | 2 | SETATTRS_REG_INT(r1,c1) \| IADD_REG_REG_INT(r2,r3,c2) \| LINKATTR1_REG_REG_REG(r4,r1,r2) |
| permute | 7 | seq-4-4021698051 | 4030000 | 16 | 2 | LINKATTR1_REG_REG_INT(r1,r2,c1) \| SETATTRS_REG_INT(r1,c2) \| IADD_REG_REG_INT(r3,r4,c3) \| LINKATTR1_REG_REG_REG(r5,r1,r3) |
| permute | 8 | seq-2-4228974504 | 3746700 | 16 | 2 | SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) |
| permute | 9 | seq-2-3501308369 | 2015652 | 13 | 2 | ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r2) |
| permute | 10 | seq-2-450164226 | 2629650 | 11 | 2 | UNLINK_REG(r1) \| LINKATTR1_REG_REG_INT(r2,r3,c1) |
| mandelbrot | 1 | seq-2-1195624459 | 505002 | 14 | 2 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) |
| mandelbrot | 2 | seq-2-1749204061 | 563006 | 12 | 2 | LOAD_REG_INT(r1,c1) \| LOAD_REG_INT(r2,c1) |
| mandelbrot | 3 | seq-2-3164625435 | 17089756 | 8 | 2 | IEQ_REG_REG_INT(r1,r2,c1) \| BRF_ID_REG(c2,r1) |
| mandelbrot | 4 | seq-2-99899793 | 1002000 | 8 | 2 | ICOPY_REG_REG(r1,r2) \| ITOF_REG(r1) |
| mandelbrot | 5 | seq-3-2282414376 | 4000 | 8 | 2 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) |
| mandelbrot | 6 | seq-2-1096152183 | 17392096 | 6 | 2 | IGT_REG_REG_REG(r1,r2,r3) \| BRT_ID_REG(c1,r1) |
| mandelbrot | 7 | seq-4-4083690158 | 3000 | 6 | 2 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) \| ENDLIFE_REG(r4) |
| mandelbrot | 8 | seq-2-1242494231 | 501000 | 4 | 2 | FDIV_REG_REG_REG(r1,r2,r1) \| FSUB_REG_REG_FLOAT(r3,r1,c1) |
| mandelbrot | 9 | seq-2-4185535788 | 501000 | 4 | 2 | FMULT_REG_REG_FLOAT(r1,r1,c1) \| ICOPY_REG_REG(r2,r3) |
| mandelbrot | 10 | seq-2-329632883 | 501000 | 4 | 2 | FSUB_REG_REG_FLOAT(r1,r2,c1) \| LOAD_REG_INT(r3,c2) |
| towers | 1 | seq-2-4228974504 | 1720380 | 41 | 2 | SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) |
| towers | 2 | seq-2-3148268850 | 1147310 | 30 | 2 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) |
| towers | 3 | seq-2-2398252370 | 901550 | 24 | 2 | SWAP_REG_REG(r1,r2) \| CALL_REG_FUNC_REG(r3,c1,r4) |
| towers | 4 | seq-2-367699156 | 901410 | 23 | 2 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) |
| towers | 5 | seq-3-922997700 | 901410 | 23 | 2 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) \| SWAP_REG_REG(r3,r2) |
| towers | 6 | seq-2-1213998682 | 601220 | 26 | 2 | BRF_ID_REG(c1,r1) \| LINKATTR1_REG_REG_INT(r2,r3,c2) |
| towers | 7 | seq-2-1721009397 | 601180 | 22 | 2 | UNLINK_REG(r1) \| BR_ID(c1) |
| towers | 8 | seq-2-3635382226 | 491740 | 26 | 2 | ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r1) |
| towers | 9 | seq-2-1606333325 | 491740 | 26 | 2 | LINKATTR1_REG_REG_INT(r1,r2,c1) \| ICOPY_REG_REG(r1,r3) |
| towers | 10 | seq-3-2231782718 | 491740 | 26 | 2 | LINKATTR1_REG_REG_INT(r1,r2,c1) \| ICOPY_REG_REG(r1,r3) \| UNLINK_REG(r1) |
| bounce | 1 | seq-2-3635382226 | 2730600 | 35 | 2 | ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r1) |
| bounce | 2 | seq-2-1606333325 | 2730600 | 35 | 2 | LINKATTR1_REG_REG_INT(r1,r2,c1) \| ICOPY_REG_REG(r1,r3) |
| bounce | 3 | seq-3-2231782718 | 2730600 | 35 | 2 | LINKATTR1_REG_REG_INT(r1,r2,c1) \| ICOPY_REG_REG(r1,r3) \| UNLINK_REG(r1) |
| bounce | 4 | seq-2-2398252370 | 682800 | 11 | 2 | SWAP_REG_REG(r1,r2) \| CALL_REG_FUNC_REG(r3,c1,r4) |
| bounce | 5 | seq-2-1555722586 | 703002 | 11 | 3 | SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) |
| bounce | 6 | seq-2-3448147170 | 703002 | 11 | 3 | SETNUMFRM_INT(c1) \| SETNUMCAS_INT(c1) |
| bounce | 7 | seq-2-2063277697 | 703002 | 11 | 3 | SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) |
| bounce | 8 | seq-3-505676515 | 703002 | 11 | 3 | SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) \| SETNUMFRM_INT(c3) |
| bounce | 9 | seq-3-2040868729 | 703002 | 11 | 3 | SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) \| SETNUMCAS_INT(c2) |
| bounce | 10 | seq-4-2765130742 | 703002 | 11 | 3 | SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) \| SETNUMFRM_INT(c3) \| SETNUMCAS_INT(c3) |
| storage | 1 | seq-2-3148268850 | 614370 | 20 | 2 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) |
| storage | 2 | seq-2-2810718276 | 532472 | 16 | 4 | SETNUMCAS_INT(c1) \| SETNUMSTD_INT(c1) |
| storage | 3 | seq-2-1555722586 | 532472 | 16 | 4 | SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) |
| storage | 4 | seq-2-3448147170 | 532472 | 16 | 4 | SETNUMFRM_INT(c1) \| SETNUMCAS_INT(c1) |
| storage | 5 | seq-2-2063277697 | 532472 | 16 | 4 | SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) |
| storage | 6 | seq-3-505676515 | 532472 | 16 | 4 | SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) \| SETNUMFRM_INT(c3) |
| storage | 7 | seq-3-3769039113 | 532472 | 16 | 4 | SETNUMFRM_INT(c1) \| SETNUMCAS_INT(c1) \| SETNUMSTD_INT(c1) |
| storage | 8 | seq-3-2040868729 | 532472 | 16 | 4 | SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) \| SETNUMCAS_INT(c2) |
| storage | 9 | seq-4-2765130742 | 532472 | 16 | 4 | SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) \| SETNUMFRM_INT(c3) \| SETNUMCAS_INT(c3) |
| storage | 10 | seq-4-1598304623 | 532472 | 16 | 4 | SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) \| SETNUMCAS_INT(c2) \| SETNUMSTD_INT(c2) |
| list | 1 | seq-2-3148268850 | 11606400 | 78 | 2 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) |
| list | 2 | seq-2-3078002600 | 2821400 | 44 | 2 | SWAP_REG_REG(r1,r2) \| SETTP_REG_INT(r3,c1) |
| list | 3 | seq-2-4228974504 | 2833200 | 42 | 2 | SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) |
| list | 4 | seq-2-367699156 | 8785600 | 40 | 2 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) |
| list | 5 | seq-3-922997700 | 8785600 | 40 | 2 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) \| SWAP_REG_REG(r3,r2) |
| list | 6 | seq-3-332906936 | 2820800 | 38 | 2 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) \| SETTP_REG_INT(r3,c1) |
| list | 7 | seq-3-374026400 | 2820800 | 38 | 2 | SWAP_REG_REG(r1,r2) \| SETTP_REG_INT(r3,c1) \| SWAP_REG_REG(r4,r3) |
| list | 8 | seq-4-3990362176 | 2820800 | 38 | 2 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) \| SETTP_REG_INT(r3,c1) \| SWAP_REG_REG(r4,r3) |
| list | 9 | seq-2-2398252370 | 8791200 | 36 | 2 | SWAP_REG_REG(r1,r2) \| CALL_REG_FUNC_REG(r3,c1,r4) |
| list | 10 | seq-3-474703047 | 8778800 | 32 | 2 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) \| CALL_REG_FUNC_REG(r3,c2,r4) |
| richards | 1 | seq-2-793582563 | 2392289 | 257 | 2 | UNLINK_REG(r1) \| UNLINK_REG(r2) |
| richards | 2 | seq-2-3635382226 | 983970 | 194 | 2 | ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r1) |
| richards | 3 | seq-3-3179044455 | 510064 | 135 | 2 | ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r1) \| UNLINK_REG(r3) |
| richards | 4 | seq-2-401038279 | 536567 | 99 | 2 | LOAD_REG_INT(r1,c1) \| ICOPY_REG_REG(r2,r1) |
| richards | 5 | seq-2-3056034726 | 438368 | 110 | 2 | MINATTRS_REG_REG_INT(r1,r2,c1) \| LINKATTR1_REG_REG_REG(r3,r1,r2) |
| richards | 6 | seq-2-1126144698 | 571948 | 97 | 2 | UNLINK_REG(r1) \| LINKATTR1_REG_REG_INT(r1,r2,c1) |
| richards | 7 | seq-3-3245642035 | 364173 | 92 | 2 | LOAD_REG_INT(r1,c1) \| ICOPY_REG_REG(r2,r1) \| UNLINK_REG(r2) |
| richards | 8 | seq-4-2010490934 | 364173 | 92 | 2 | LOAD_REG_INT(r1,c1) \| ICOPY_REG_REG(r2,r1) \| UNLINK_REG(r2) \| UNLINK_REG(r3) |
| richards | 9 | seq-3-2064625188 | 551464 | 90 | 2 | UNLINK_REG(r1) \| UNLINK_REG(r2) \| LINKATTR1_REG_REG_INT(r2,r3,c1) |
| richards | 10 | seq-2-4228974504 | 1328184 | 94 | 2 | SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) |
| json | 1 | seq-2-1195624459 | 6180000 | 212 | 2 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) |
| json | 2 | seq-2-4228974504 | 29460000 | 208 | 2 | SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) |
| json | 3 | seq-3-1898281124 | 25030000 | 174 | 2 | SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) \| SWAP_REG_REG(r5,r6) |
| json | 4 | seq-3-2282414376 | 4440000 | 174 | 2 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) |
| json | 5 | seq-4-4083690158 | 3690000 | 152 | 2 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) \| ENDLIFE_REG(r4) |
| json | 6 | seq-4-1694203526 | 20600000 | 140 | 2 | SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) \| SWAP_REG_REG(r5,r6) \| SWAP_REG_REG(r7,r8) |
| json | 7 | seq-2-2174211815 | 900000 | 72 | 2 | NULL_REG(r1) \| NULL_REG(r2) |
| json | 8 | seq-2-1873236026 | 16670000 | 62 | 2 | ILT_REG_REG_REG(r1,r2,r3) \| BRF_ID_REG(c1,r1) |
| json | 9 | seq-3-1529237914 | 740000 | 58 | 2 | NULL_REG(r1) \| NULL_REG(r2) \| NULL_REG(r3) |
| json | 10 | seq-2-3148268850 | 7365000 | 53 | 3 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) |
| base64 | 1 | seq-2-1195624459 | 9596007 | 109 | 2 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) |
| base64 | 2 | seq-3-2282414376 | 7543003 | 87 | 2 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) |
| base64 | 3 | seq-4-4083690158 | 6173002 | 72 | 2 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) \| ENDLIFE_REG(r4) |
| base64 | 4 | seq-2-2174211815 | 8220000 | 42 | 2 | NULL_REG(r1) \| NULL_REG(r2) |
| base64 | 5 | seq-3-1529237914 | 6850000 | 35 | 2 | NULL_REG(r1) \| NULL_REG(r2) \| NULL_REG(r3) |
| base64 | 6 | seq-4-2233651730 | 5480000 | 28 | 2 | NULL_REG(r1) \| NULL_REG(r2) \| NULL_REG(r3) \| NULL_REG(r4) |
| base64 | 7 | seq-2-3148268850 | 3077506 | 25 | 2 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) |
| base64 | 8 | seq-2-367699156 | 2053505 | 21 | 2 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) |
| base64 | 9 | seq-3-922997700 | 1711505 | 19 | 2 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) \| SWAP_REG_REG(r3,r2) |
| base64 | 10 | seq-2-2398252370 | 686506 | 17 | 2 | SWAP_REG_REG(r1,r2) \| CALL_REG_FUNC_REG(r3,c1,r4) |
| rexxcps | 1 | seq-2-1195624459 | 21513 | 487 | 10 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) |
| rexxcps | 2 | seq-3-2282414376 | 16581 | 371 | 9 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) |
| rexxcps | 3 | seq-2-2174211815 | 23600 | 395 | 16 | NULL_REG(r1) \| NULL_REG(r2) |
| rexxcps | 4 | seq-4-4083690158 | 11998 | 283 | 7 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) \| ENDLIFE_REG(r4) |
| rexxcps | 5 | seq-3-1529237914 | 19587 | 299 | 14 | NULL_REG(r1) \| NULL_REG(r2) \| NULL_REG(r3) |
| rexxcps | 6 | seq-4-2233651730 | 15978 | 211 | 14 | NULL_REG(r1) \| NULL_REG(r2) \| NULL_REG(r3) \| NULL_REG(r4) |
| rexxcps | 7 | seq-2-367699156 | 1794 | 187 | 8 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) |
| rexxcps | 8 | seq-2-3148268850 | 1725 | 161 | 8 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) |
| rexxcps | 9 | seq-2-4103262805 | 1242 | 154 | 6 | SETTP_REG_INT(r1,c1) \| CALL_REG_FUNC_REG(r2,c2,r3) |
| rexxcps | 10 | seq-3-922997700 | 1529 | 145 | 8 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) \| SWAP_REG_REG(r3,r2) |

## Strongest bounded next candidates

| ID | Revision | Count | Pattern | Required proof |
|---|---|---:|---|---|
| seq-2-2151719822 | nr05-6a064499f327-rxseq-schema4 | 1948338 | LOAD_REG_INT(r1,c1) \| LOAD_REG_INT(r2,c2) | liveness, destination, and interrupt-equivalence proof |
| seq-2-367699156 | nr05-6a064499f327-rxseq-schema4 | 16037923 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) | liveness, destination, and interrupt-equivalence proof |
| seq-2-3148268850 | nr05-6a064499f327-rxseq-schema4 | 26311205 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) | liveness, destination, and interrupt-equivalence proof |
| seq-3-922997700 | nr05-6a064499f327-rxseq-schema4 | 15670658 | LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) \| SWAP_REG_REG(r3,r2) | liveness, destination, and interrupt-equivalence proof |
| seq-2-4228974504 | nr05-6a064499f327-rxseq-schema4 | 40867553 | SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) | liveness, destination, and interrupt-equivalence proof |
| seq-3-1898281124 | nr05-6a064499f327-rxseq-schema4 | 28654326 | SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) \| SWAP_REG_REG(r5,r6) | liveness, destination, and interrupt-equivalence proof |
| seq-2-3078002600 | nr05-6a064499f327-rxseq-schema4 | 11864272 | SWAP_REG_REG(r1,r2) \| SETTP_REG_INT(r3,c1) | liveness, destination, and interrupt-equivalence proof |
| seq-3-374026400 | nr05-6a064499f327-rxseq-schema4 | 10298519 | SWAP_REG_REG(r1,r2) \| SETTP_REG_INT(r3,c1) \| SWAP_REG_REG(r4,r3) | liveness, destination, and interrupt-equivalence proof |
| seq-3-332906936 | nr05-6a064499f327-rxseq-schema4 | 9260019 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) \| SETTP_REG_INT(r3,c1) | liveness, destination, and interrupt-equivalence proof |
| seq-4-3990362176 | nr05-6a064499f327-rxseq-schema4 | 8746515 | SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) \| SETTP_REG_INT(r3,c1) \| SWAP_REG_REG(r4,r3) | liveness, destination, and interrupt-equivalence proof |
| seq-2-401038279 | nr05-6a064499f327-rxseq-schema4 | 3380680 | LOAD_REG_INT(r1,c1) \| ICOPY_REG_REG(r2,r1) | liveness, destination, and interrupt-equivalence proof |
| seq-2-1749204061 | nr05-6a064499f327-rxseq-schema4 | 1349155 | LOAD_REG_INT(r1,c1) \| LOAD_REG_INT(r2,c1) | liveness, destination, and interrupt-equivalence proof |

## NR-08-subsumed sequence families

Patterns containing `ENDLIFE_REG` are marked subsumed only when the exact aggregated current 2.2d count is lower in the frozen NR-08 candidate. Remaining candidate occurrences are retained as conservative/reference/generated behavior, not promoted.

| ID | Pattern | Measured disposition |
|---|---|---|
| seq-2-1195624459 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) | NR-08 current=2072983;candidate=0;delta=-2072983 |
| seq-3-2282414376 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) | NR-08 current=1595814;candidate=0;delta=-1595814 |
| seq-4-4083690158 | ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) \| ENDLIFE_REG(r4) | NR-08 current=1147312;candidate=0;delta=-1147312 |

## Production follow-up

Use the selected mechanically complete candidates as a review queue only. For one candidate at a time, prove operand liveness/aliasing, exception and VM interrupt-boundary equivalence, decide compiler versus RXAS ownership, add generated structural tests, then run the performance programme's formal Release verdict. No opcode or fusion is authorized by this ledger.
