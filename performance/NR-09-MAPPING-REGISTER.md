# NR-09 exact mapping register

Status: **corrected pruning implemented and broad QA complete; final ordinary
Release refresh shows no regression; production-batch commit pending**

Recorded: 2026-07-18
Updated: 2026-07-20

Source ledger: `performance/evidence/2026-07-17-nr-09-sequence-ledger-poc/retained-rxvm/sequence-ledger.csv`

This register resolves all 76 selected stable identities from the retained
RXSEQ decision view. Counts are bounded observations from revision-labelled
N=2/3/4 windows. Windows overlap, so counts must not be added to estimate a
dispatch reduction. Exact current-image occurrences and operands must be
re-mined before instruction forms are frozen.

## Approved corrected production disposition

Adrian approved the overlap-corrected disposition on 2026-07-19. The
production edit removes these 26 forms while preserving each numeric opcode as
a fail-safe reserved slot, so existing RXBIN numbering does not move:

- `NUMCTX_INT_INT_INT_INT_INT`
- `SETTPSWAPSETTP_REG_INT_REG_REG`
- `ILOADCOPY_REG_REG_INT`
- `ILOADN_REG_INT_REG_INT`
- `ILOADN_REG_REG_INT`
- `FSUBILOAD_REG_REG_FLOAT_REG_INT`
- `ITOSCONCAT_REG_STRING_REG`
- `SCONCATITOS_REG_STRING_REG`
- `ILOADGETATTRS_REG_INT_REG_REG_INT`
- `IGETATTR1_REG_REG_INT`
- `MINIGETATTR1_REG_REG_INT`
- `ISETATTR1_REG_INT_INT`
- `RELINKATTR1_REG_REG_INT`
- `UNLINKRELINKATTR1_REG_REG_REG_INT`
- `LINKSETATTRS_REG_REG_INT_INT`
- `LINKSETATTRSADD_REG_REG_INT_INT_REG_REG_INT`
- `SETATTRSADD_REG_INT_REG_REG_INT`
- `LINKILOAD_REG_REG_REG_REG_INT`
- `UNLINKLINKATTR1_REG_REG_REG_INT`
- `LINKATTR1ADD_REG_REG_REG_INT`
- `ISETATTR1_REG_REG_INT`
- `STOIATTR1_REG_REG_INT`
- `MINSTOIATTR1_REG_REG_INT`
- `ILOADSETUNLINK_REG_REG_INT`
- `LINKILOADSETUNLINK_REG_REG_REG_REG_INT`
- `SETLINKATTR1_REG_REG_INT_INT`

The corrected mapping order retains four forms that the first all-enabled
census had masked:

- narrow `ITOF_REG_REG` selection to the measured arithmetic chain;
- select `FMULTICOPY_REG_FLOAT_REG_REG` for that chain;
- promote `LINK + SETLINKATTR1` to
  `LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT`;
- promote `SETLINKATTR1 + LOAD` to
  `SETLINKILOAD_REG_REG_INT_REG_REG_INT`.

The two ugly caller-temporary forms remain present unchanged in this edit, but
only as design candidates for a later separately measured replacement. They
are not approval to preserve their side effects indefinitely.

## Selected clean replacement designs (not implemented)

### Result-only `FDIVSUB`

The current form implements `fdiv quotient,numerator,quotient` followed by
`fsub result,quotient,constant` and therefore asks generated code to expose the
quotient register. The preferred replacement keeps the four logical inputs
but changes the contract to:

```text
quotient = numerator.float / divisor.float
result.float = quotient - constant
```

Only `result` is written; `numerator` and `divisor` are unchanged. The VM
handler should hold `quotient` in a C local so no bytecode register exists for
the intermediate. rxc may select the form only when compiler liveness proves
the original quotient result is dead after the subtraction and no generated
TRACE event requires that intermediate write. Otherwise it must leave the two
instructions expanded. This is Class 2 compiler knowledge and must not become
an RXAS adjacency inference. The evidence justifying a replacement PoC is
501,000 bounded executions and +34.379%/+23.874% isolated cell speedup on
`rxvm`/`rxbvm`.

### Compact TRACE-correct `ILOADSETUNLINKN`

The current wide form represents `LOAD temporary,constant; ICOPY
alias,temporary; UNLINK alias; UNLINK other` and exposes `temporary` only to
preserve a generated TRACE write. The preferred lowering uses the existing
compact `ILOADSETUNLINKN_REG_INT_REG` form (`alias,constant,other`) when rxc
proves that the temporary has no later semantic read and can retarget any
generated TRACE observation to the equal value stored through `alias` at the
same source step. If that TRACE proof is unavailable, the sequence remains
expanded; it must not retain or invent a pointless caller operand. RXAS must
not infer this compiler-owned fact. A replacement PoC must include positive
TRACE retargeting plus negative later-read, relevant-TRACE and alias/cleanup
cases before retiring the wide form. The evidence justifying the PoC is
364,203 bounded executions and +11.793%/+18.185% isolated cell speedup on
`rxvm`/`rxbvm`.

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

## Selected mapping input

Selected input total: **67** (Class 1: **16**; Class 2: **51**) across **12**
families. This is the approved mapping queue recorded below, not the final
source-visible opcode-form count after evidence-led pruning.

## Implemented batch shape

The first complete candidate represented the selected mappings with 60
canonical instruction forms: 57 mapping forms plus three wider forms that
preserved TRACE-visible intermediate writes. The approved balanced review then
withdrew 26 source-visible forms and kept their numeric opcode slots reserved.
The final public addition is **34 forms**: 32 forms under the 25 mnemonics in
`docs/reference/rxas/instructions/12-large-instructions.md`, plus the
two-register `ITOF` and `STOI` forms in their existing reference chapters.

All retained Class 1 mappings have RXAS backstops. rxc emits a large
instruction directly only when one emission site owns and fixes the complete
semantic unit. Alias/copy/cleanup Class 2 combinations are selected from the
actual final typed instruction stream, not AST provenance; an actual `ICOPY`
must be present before an integer cleanup form can be selected. Source steps
and labels remain barriers. TRACE references are retargeted only to a
proved-equal retained register, and authored RXAS never gains compiler-only
temporary/alias assumptions.

The operand-transport prerequisite is isolated in commit `32bf7e76f`. **38 of
the 67 mappings require more than three operands:** 11/16 Class 1 and 27/51
Class 2. Without arbitrary operand transport, all 11 wide Class 1 backstops and
up to 38 exact selected mappings would be blocked or require descriptor/partial
alternatives. The three trace-preserving production forms also use wide
transport. The prerequisite extends the instruction table, assembler,
disassembler, linker and relocation, VM decode, rxc `ASSEMBLE`, metadata,
instruction database and their tests without changing RXBIN 007.

The original 60-form focused gate passed 9/9 selected CTests and reported
600 forms/388 unique mnemonics. Those figures describe the retained
pre-pruning candidate evidence, not the final product. The final assembler and
human reference agree on **574 forms/367 unique mnemonics**, and the final
instruction source/database addition is exactly **34 rows**.

The initial unmatched-session Release comparison appeared negative at
-2.233%/-2.289% median CPS (`rxvm`/`rxbvm`). A same-session rebaseline proved
that sign was baseline drift, not a reproduced VM or fused-product regression.
After the approved 26-form pruning, the first equal-path verdict was
+0.262%/+0.937%. The post-QA 78/78 refresh records complete-product paired
median CPS of **+1.385%/+2.868%**; the `rxvm` interval crosses zero and the
`rxbvm` interval is wholly positive.

Broad closeout passes full Debug CTest 1,864/1,864, an audited 110-golden
refresh, supported Apple ASan with no sanitizer diagnostic, an isolated
112-file install, and both installed VMs against the exact accepted old RXBIN
and library. Apple ASan does not support leak detection on this host. The QA
also added the three retained fused call forms to native cold signal-window
restore. Evidence is under
`performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/qa-closeout/`
and `finalrun01/`.


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
