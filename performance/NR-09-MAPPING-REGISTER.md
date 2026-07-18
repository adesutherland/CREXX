# NR-09 exact mapping register

Status: **approved implementation register; no batch production edit started**

Recorded: 2026-07-18

Source ledger: `performance/evidence/2026-07-17-nr-09-sequence-ledger-poc/retained-rxvm/sequence-ledger.csv`

This register resolves all 76 selected stable identities from the retained
RXSEQ decision view. Counts are bounded observations from revision-labelled
N=2/3/4 windows. Windows overlap, so counts must not be added to estimate a
dispatch reduction. Exact current-image occurrences and operands must be
re-mined before instruction forms are frozen.

## Ownership

- **Class 1 / RXAS-backed:** RXAS must recognize the mapping. rxc may also
  emit the large instruction directly when one AST-node emission naturally
  owns the complete unit. If two AST nodes merely happen to emit adjacent
  instructions, keep rxc simple and let RXAS collect the sequence.
- **Class 2 / rxc-owned:** rxc must prove compiler-temporary registers,
  aliases or otherwise irrelevant intermediate effects. RXAS must not infer
  those source facts from adjacency alone.

## Approved batch order

1. Define the canonical large instructions, operand forms, effects metadata,
   VM handlers and direct instruction tests for all selected families.
2. Add RXAS recognition and positive/negative peephole tests for every Class
   1 mapping. This remains the backstop even when rxc can emit directly.
3. Add simple single-AST direct rxc emission for Class 1 where natural, then
   the rxc-owned Class 2 lowerings and compiler structural tests.
4. Run minimum focused correctness, retain per-mapping static/dynamic deltas,
   freeze the whole batch, and run one ordinary profiling-off Release verdict
   against the accepted NR-09 Rule 1 baseline.
5. Stop for Adrian. After the batch verdict is accepted, run the proportional
   full QA closeout, refresh only audited goldens, and commit when requested.

## Active mappings

Active total: **67** (Class 1: **16**; Class 2: **51**) across **12** families.


### Numeric-context prologue (9)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-3-2040868729` | 2 | 16434838 | `SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) \| SETNUMCAS_INT(c2)` |
| `seq-4-2765130742` | 2 | 16434838 | `SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) \| SETNUMFRM_INT(c3) \| SETNUMCAS_INT(c3)` |
| `seq-2-3448147170` | 2 | 16434838 | `SETNUMFRM_INT(c1) \| SETNUMCAS_INT(c1)` |
| `seq-2-2063277697` | 2 | 16434838 | `SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2)` |
| `seq-3-505676515` | 2 | 16434838 | `SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2) \| SETNUMFRM_INT(c3)` |
| `seq-2-1555722586` | 2 | 16434838 | `SETNUMDGTS_INT(c1) \| SETNUMFUZ_INT(c2)` |
| `seq-4-1598304623` | 2 | 16290679 | `SETNUMFUZ_INT(c1) \| SETNUMFRM_INT(c2) \| SETNUMCAS_INT(c2) \| SETNUMSTD_INT(c2)` |
| `seq-3-3769039113` | 2 | 16290679 | `SETNUMFRM_INT(c1) \| SETNUMCAS_INT(c1) \| SETNUMSTD_INT(c1)` |
| `seq-2-2810718276` | 2 | 16290679 | `SETNUMCAS_INT(c1) \| SETNUMSTD_INT(c1)` |

### Multi-swap (3)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-4228974504` | 1 | 40867553 | `SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4)` |
| `seq-3-1898281124` | 1 | 28654326 | `SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) \| SWAP_REG_REG(r5,r6)` |
| `seq-4-1694203526` | 1 | 21208320 | `SWAP_REG_REG(r1,r2) \| SWAP_REG_REG(r3,r4) \| SWAP_REG_REG(r5,r6) \| SWAP_REG_REG(r7,r8)` |

### Call-window preparation (7)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-3148268850` | 1 | 26311205 | `SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1)` |
| `seq-2-367699156` | 1 | 16037923 | `LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2)` |
| `seq-3-922997700` | 1 | 15670658 | `LOAD_REG_INT(r1,c1) \| SETTP_REG_INT(r2,c2) \| SWAP_REG_REG(r3,r2)` |
| `seq-2-3078002600` | 1 | 11864272 | `SWAP_REG_REG(r1,r2) \| SETTP_REG_INT(r3,c1)` |
| `seq-3-374026400` | 1 | 10298519 | `SWAP_REG_REG(r1,r2) \| SETTP_REG_INT(r3,c1) \| SWAP_REG_REG(r4,r3)` |
| `seq-3-332906936` | 1 | 9260019 | `SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) \| SETTP_REG_INT(r3,c1)` |
| `seq-4-3990362176` | 1 | 8746515 | `SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) \| SETTP_REG_INT(r3,c1) \| SWAP_REG_REG(r4,r3)` |

### Call-window through call (3)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-2398252370` | 2 | 14787936 | `SWAP_REG_REG(r1,r2) \| CALL_REG_FUNC_REG(r3,c1,r4)` |
| `seq-3-474703047` | 2 | 10764409 | `SETTP_REG_INT(r1,c1) \| SWAP_REG_REG(r2,r1) \| CALL_REG_FUNC_REG(r3,c2,r4)` |
| `seq-2-4103262805` | 2 | 1590962 | `SETTP_REG_INT(r1,c1) \| CALL_REG_FUNC_REG(r2,c2,r3)` |

### Multi-null (3)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-2174211815` | 1 | 12521800 | `NULL_REG(r1) \| NULL_REG(r2)` |
| `seq-3-1529237914` | 1 | 9857187 | `NULL_REG(r1) \| NULL_REG(r2) \| NULL_REG(r3)` |
| `seq-4-2233651730` | 1 | 7761578 | `NULL_REG(r1) \| NULL_REG(r2) \| NULL_REG(r3) \| NULL_REG(r4)` |

### Constant load and direct destination (3)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-401038279` | 1 | 3380680 | `LOAD_REG_INT(r1,c1) \| ICOPY_REG_REG(r2,r1)` |
| `seq-2-2151719822` | 1 | 1948338 | `LOAD_REG_INT(r1,c1) \| LOAD_REG_INT(r2,c2)` |
| `seq-2-1749204061` | 1 | 1349155 | `LOAD_REG_INT(r1,c1) \| LOAD_REG_INT(r2,c1)` |

### Unlink/cleanup chains (7)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-793582563` | 2 | 10866847 | `UNLINK_REG(r1) \| UNLINK_REG(r2)` |
| `seq-2-3635382226` | 2 | 8212066 | `ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r1)` |
| `seq-2-3501308369` | 2 | 3060430 | `ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r2)` |
| `seq-3-3245642035` | 2 | 1991103 | `LOAD_REG_INT(r1,c1) \| ICOPY_REG_REG(r2,r1) \| UNLINK_REG(r2)` |
| `seq-3-3179044455` | 2 | 1517894 | `ICOPY_REG_REG(r1,r2) \| UNLINK_REG(r1) \| UNLINK_REG(r3)` |
| `seq-2-1721009397` | 2 | 860448 | `UNLINK_REG(r1) \| BR_ID(c1)` |
| `seq-4-2010490934` | 2 | 364203 | `LOAD_REG_INT(r1,c1) \| ICOPY_REG_REG(r2,r1) \| UNLINK_REG(r2) \| UNLINK_REG(r3)` |

### Attribute/index/link scaffolding (23)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-450164226` | 2 | 7113611 | `UNLINK_REG(r1) \| LINKATTR1_REG_REG_INT(r2,r3,c1)` |
| `seq-2-2413302004` | 2 | 5754920 | `IADD_REG_REG_INT(r1,r2,c1) \| LINKATTR1_REG_REG_REG(r3,r4,r1)` |
| `seq-2-249145866` | 2 | 5754920 | `SETATTRS_REG_INT(r1,c1) \| IADD_REG_REG_INT(r2,r3,c2)` |
| `seq-3-4129899544` | 2 | 5754920 | `SETATTRS_REG_INT(r1,c1) \| IADD_REG_REG_INT(r2,r3,c2) \| LINKATTR1_REG_REG_REG(r4,r1,r2)` |
| `seq-2-2073917396` | 2 | 5571246 | `LINKATTR1_REG_REG_INT(r1,r2,c1) \| SETATTRS_REG_INT(r1,c2)` |
| `seq-3-3409788275` | 2 | 5569610 | `LINKATTR1_REG_REG_INT(r1,r2,c1) \| SETATTRS_REG_INT(r1,c2) \| IADD_REG_REG_INT(r3,r4,c3)` |
| `seq-4-4021698051` | 2 | 5569610 | `LINKATTR1_REG_REG_INT(r1,r2,c1) \| SETATTRS_REG_INT(r1,c2) \| IADD_REG_REG_INT(r3,r4,c3) \| LINKATTR1_REG_REG_REG(r5,r1,r3)` |
| `seq-2-1606333325` | 2 | 5067078 | `LINKATTR1_REG_REG_INT(r1,r2,c1) \| ICOPY_REG_REG(r1,r3)` |
| `seq-3-2231782718` | 2 | 5067078 | `LINKATTR1_REG_REG_INT(r1,r2,c1) \| ICOPY_REG_REG(r1,r3) \| UNLINK_REG(r1)` |
| `seq-2-3056034726` | 2 | 2132602 | `MINATTRS_REG_REG_INT(r1,r2,c1) \| LINKATTR1_REG_REG_REG(r3,r1,r2)` |
| `seq-2-1590753747` | 2 | 2107218 | `SETATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_REG(r2,r1,r3)` |
| `seq-2-4110644254` | 2 | 1989352 | `LINKATTR1_REG_REG_REG(r1,r2,r3) \| LOAD_REG_INT(r4,c1)` |
| `seq-3-3212259269` | 2 | 1925878 | `LINKATTR1_REG_REG_REG(r1,r2,r3) \| LOAD_REG_INT(r4,c1) \| ICOPY_REG_REG(r1,r4)` |
| `seq-4-3622087100` | 2 | 1925878 | `LINKATTR1_REG_REG_REG(r1,r2,r3) \| LOAD_REG_INT(r4,c1) \| ICOPY_REG_REG(r1,r4) \| UNLINK_REG(r1)` |
| `seq-2-1126144698` | 2 | 1860398 | `UNLINK_REG(r1) \| LINKATTR1_REG_REG_INT(r1,r2,c1)` |
| `seq-3-635106007` | 2 | 1606900 | `SETATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_REG(r2,r1,r3) \| LOAD_REG_INT(r4,c2)` |
| `seq-3-2064625188` | 2 | 803504 | `UNLINK_REG(r1) \| UNLINK_REG(r2) \| LINKATTR1_REG_REG_INT(r2,r3,c1)` |
| `seq-2-3405515001` | 2 | 56006 | `MINATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_INT(r2,r1,c1)` |
| `seq-3-1769762332` | 2 | 24 | `LINKATTR1_REG_REG_INT(r1,r2,c1) \| STOI_REG(r1) \| ICOPY_REG_REG(r3,r1)` |
| `seq-4-186090217` | 2 | 24 | `LINKATTR1_REG_REG_INT(r1,r2,c1) \| STOI_REG(r1) \| ICOPY_REG_REG(r3,r1) \| UNLINK_REG(r1)` |
| `seq-4-448602274` | 2 | 24 | `MINATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_INT(r2,r1,c1) \| STOI_REG(r2) \| ICOPY_REG_REG(r3,r2)` |
| `seq-2-4234099218` | 2 | 24 | `LINKATTR1_REG_REG_INT(r1,r2,c1) \| STOI_REG(r1)` |
| `seq-3-311949687` | 2 | 24 | `MINATTRS_REG_INT(r1,c1) \| LINKATTR1_REG_REG_INT(r2,r1,c1) \| STOI_REG(r2)` |

### Typed conversion/copy (3)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-99899793` | 2 | 1002000 | `ICOPY_REG_REG(r1,r2) \| ITOF_REG(r1)` |
| `seq-2-18336876` | 2 | 22402 | `STOI_REG(r1) \| ICOPY_REG_REG(r2,r1)` |
| `seq-3-3596904615` | 2 | 24 | `STOI_REG(r1) \| ICOPY_REG_REG(r2,r1) \| UNLINK_REG(r1)` |

### Arithmetic chains (3)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-1242494231` | 2 | 501000 | `FDIV_REG_REG_REG(r1,r2,r1) \| FSUB_REG_REG_FLOAT(r3,r1,c1)` |
| `seq-2-4185535788` | 2 | 501000 | `FMULT_REG_REG_FLOAT(r1,r1,c1) \| ICOPY_REG_REG(r2,r3)` |
| `seq-2-329632883` | 2 | 501000 | `FSUB_REG_REG_FLOAT(r1,r2,c1) \| LOAD_REG_INT(r3,c2)` |

### String conversion/concatenation (2)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-2591213324` | 2 | 14000 | `ITOS_REG(r1) \| CONCAT_REG_STRING_REG(r2,c1,r1)` |
| `seq-2-3536275467` | 2 | 24 | `SCONCAT_REG_REG_STRING(r1,r1,c1) \| ITOS_REG(r2)` |

### Load/get-attributes (1)

| Stable ID | Class | Leading observation | Normalized mapping |
| --- | ---: | ---: | --- |
| `seq-2-1613512688` | 2 | 14012 | `LOAD_REG_INT(r1,c1) \| GETATTRS_REG_REG_INT(r2,r3,c2)` |

## Retained prior rejection

These four Class 1 mappings are semantically eligible for an RXAS backstop,
but NR-07 already measured and rejected their product benefit. They remain
out of the active 67 and must not be reintroduced under a new name.

| Stable ID | Leading observation | Normalized mapping |
| --- | ---: | --- |
| `seq-2-3164625435` | 40527479 | `IEQ_REG_REG_INT(r1,r2,c1) \| BRF_ID_REG(c2,r1)` |
| `seq-2-1096152183` | 19623744 | `IGT_REG_REG_REG(r1,r2,r3) \| BRT_ID_REG(c1,r1)` |
| `seq-2-1873236026` | 17018464 | `ILT_REG_REG_REG(r1,r2,r3) \| BRF_ID_REG(c1,r1)` |
| `seq-2-1921843141` | 9961515 | `ILT_REG_REG_INT(r1,r2,c1) \| BRF_ID_REG(c2,r1)` |

## Deferred controls

These five selected patterns are not in the low-risk batch.

| Stable ID | Leading observation | Normalized mapping | Reason |
| --- | ---: | --- | --- |
| `seq-2-1195624459` | 17305244 | `ENDLIFE_REG(r1) \| ENDLIFE_REG(r2)` | NR-08-subsumed lifecycle family; reference and exceptional state remain observable |
| `seq-3-2282414376` | 12106588 | `ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3)` | NR-08-subsumed lifecycle family; reference and exceptional state remain observable |
| `seq-4-4083690158` | 9924504 | `ENDLIFE_REG(r1) \| ENDLIFE_REG(r2) \| ENDLIFE_REG(r3) \| ENDLIFE_REG(r4)` | NR-08-subsumed lifecycle family; reference and exceptional state remain observable |
| `seq-2-1213998682` | 973976 | `BRF_ID_REG(c1,r1) \| LINKATTR1_REG_REG_INT(r2,r3,c2)` | branch into alias/capacity work retains observable control and throw boundaries |
| `seq-2-3089514224` | 23 | `BRF_ID_REG(c1,r1) \| MINATTRS_REG_INT(r2,c2)` | branch into alias/capacity work retains observable control and throw boundaries |
