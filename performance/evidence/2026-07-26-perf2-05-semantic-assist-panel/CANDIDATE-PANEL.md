# PERF2-05 profile-selected semantic-assist candidate panel

Date: 2026-07-26

Status: decision required at the mandatory pre-production selection stop. No
production implementation is selected or authorized by this panel.

## Evidence boundary

- Source product: detached clean `develop` commit
  `537d3b3d276606767535ecb84ad2a3c80073e5dd` after accepted P05-CF1.
- Ordinary product: profiling-off `Release`, `-O3 -DNDEBUG`.
- Diagnostic product: profiling-on `Release`, counts only.
- Portfolio: the governed 11 workloads, optimized and no-opt, with exact
  program and library hashes from `post-p05-profile-manifest-v1.txt`.
- Both `rxvm` and `rxbvm` completed every entry and exact RXSEQ N=2/3/4
  analysis. The two modes agree on the semantic counts used below.
- Counts, static sites and modules describe each row independently. Rows and
  projected savings overlap and must not be summed into a product claim.

## Refreshed ranking

| Rank | ID | Exact optimized footprint | Semantic unit | Disposition before PoC |
| ---: | --- | --- | --- | --- |
| 1 | SA-R1 | `UNLINK; LINKREF`: 4,337,800 executions, 11 sites, 2 modules (`List`, `Bounce`) | replace one local weak-reference alias with another while preserving unlink-first failure state | passes bounded PoC; recommend exact private form after compiler/RXAS eligibility proof |
| 2 | SA-R2 | `LINKATTR1; COPY; UNLINK`: 3,929,948 executions, 14 sites, 4 modules; the exact `List` reference getter owns 3,820,600 executions and 5 sites in one module | materialize a receiver-owned weak-reference descriptor without materializing its target object | passes bounded canonical-copy PoC; recommend as first independent rung |
| 3 | SA-B1 | `ICOPY; BR`: 2,321,562 executions, 59 sites, 5 modules | propagate a scalar `BLOCK_EXPR` result to a common exit | instruction gate passes but timing is noisy/VM-dependent; neutral, compiler-only reopening condition |
| 4 | SA-S1 | `ITOS; CONCAT`: 2,885,410 executions, 16 sites, 11 modules, but 2,885,400 executions and 6 sites are RexxCPS-only | integer representation materialization consumed by concat | retain as a one-workload control; the old exposed-temporary public form was already rejected |
| 5 | SA-D1 | `DCOPY; DTOS`: 2,541,900 executions, 5 sites, 1 module | decimal snapshot followed by representation conversion | defer: one-module breadth and the open PERF2-07 V3 representation-validity case make this a poor P05 owner |
| 6 | SA-T1 | adjacent `SETTP`: 2,506,050 executions, 27 sites, 4 modules | independent public status writes | neutral control: no coherent combined state transition was proved |
| 7 | SA-C1 | adjacent `SCOPY`: 4,178,200 executions, 16 sites, 2 modules | mostly independent PARSE/result copies | reject: overlapping data movement is not one semantic operation |

The larger `IEQ; BRF` family remains a historical NR-07 rejected control:
20,411,477 optimized executions, 87 sites and 9 modules do not constitute new
evidence for rerunning the same direct compare/branch lowering.

## Semantic and effect obligations

### SA-R1 - local weak-reference relink

The source sequence first restores the destination local's base storage and
then validates/links the new reference. If `LINKREF` raises
`REFERENCE_INVALID`, the destination must remain unlinked. A valid assist must
therefore preserve:

1. unlink-before-validation ordering;
2. the destination's base/local mapping on failure;
3. reference-cell validity and target identity without extending target
   lifetime;
4. the existing assignment TRACE value and source location;
5. frame exit, signal/unwind and recycled-frame cleanup.

No TRACE event occurs between the two executable instructions at the selected
generated sites. This makes the pair a coherent compiler-selected or private
execution assist. A public `relinkref` spelling would also be coherent, but it
would add RXAS/RXBIN surface and therefore remains an approval-only control.

### SA-R2 - partial reference-descriptor materialization

The `List` benchmark uses an arena because cREXX references are weak: the
reference descriptor does not keep the target `ListElement` alive. The hot
getter copies `next_reference` out of an object attribute. It does **not** copy
the target object.

The general `COPY` handler copies the complete VM value state and retains the
source reference cell while releasing any destination reference payload.
A narrower reference-descriptor path may be valid only if it preserves:

1. reference-cell retain/release and destination cleanup ordering;
2. destination reference identity, which ordinary value copy deliberately
   preserves;
3. status/object/empty representation invariants of a reference value;
4. invalidation and `refvalid` behavior after target lifetime end;
5. the source and assignment TRACE events currently attached around the
   temporary link/copy/unlink transaction;
6. exact bounds failure before destination mutation.

The first PoC ceiling should directly copy the attribute value with the
canonical `copy_value` behavior while removing the temporary alias. A second
payload-only helper is admissible only after the reference-value shape
invariant is mechanically proved and guarded. Copying a raw target pointer is
not admissible.

### SA-B1 - scalar block result

`ICOPY` is nonthrowing and the following `BR` is unconditional. The preferred
owner is compiler register/result placement: make the scalar producer or
returned local own the common block-result register when liveness, source
TRACE, cleanup and multiple-exit equivalence are proved. This removes an
instruction rather than merely changing dispatch.

An `icopybr` instruction is a useful native ceiling, but it is not the first
production design: it exposes a control-flow fusion in public RXAS/RXBIN and
retains the otherwise unnecessary copy.

## Placement comparison

| Candidate | Existing composition | Compiler result/selection | Public instruction | Private/quickened execution | Native helper/control | Panel verdict |
| --- | --- | --- | --- | --- | --- | --- |
| SA-R1 | correct, two dispatches | exact generated pair selection | coherent but approval-only | strongest no-RXBIN-change placement | direct one-handler ceiling | public control passes; recommend compiler eligibility plus private execution |
| SA-R2 | correct, three dispatches plus temp alias | exact reference-accessor proof already exists | generic `copyattr1` is too broad; typed reference form still approval-only | strong if TRACE retarget and fallback are exact | canonical-copy ceiling first; payload-only guard second | public control passes; recommend compiler proof plus private assist only if composition needs it |
| SA-B1 | correct, copy plus branch | preferred: result-register forwarding | approval-only ceiling | possible but preserves redundant copy | one-handler ceiling | public ceiling is neutral; no production form selected |
| SA-S1 | correct | can prove a dead conversion temp at selected sites | prior exposed-temp form rejected | possible, but one hot workload | old cell showed only small gain | do not reopen before R1/R2/B1 |
| SA-D1 | correct | result forwarding may remove `DCOPY` only with decimal ownership proof | poor breadth | not before V3 fix | conversion helper is not the missing proof | defer to PERF2-07 |

## Bounded PoC result

All candidate images passed both VM modes. The semantic reference guard covers
descriptor `refvalid`/`deref`, out-of-range failure before destination mutation
and relink's unlink-first invalid-reference state.

| Candidate | Static reduction | Dynamic reduction | `rxvm` median | `rxbvm` median | Gate |
| --- | ---: | ---: | ---: | ---: | --- |
| R1 relink | 9 instructions | 3,827,800 instructions | -2.253% | -1.623% | pass |
| R2 direct descriptor materialization | 10 instructions | 7,641,200 instructions | -6.173% | -6.154% | pass |
| R1+R2 compatibility control | 19 instructions | 11,469,000 instructions | -8.721% | -7.603% | pass, but keep production rungs separate |
| B1 copy/branch | 23 instructions | 1,366,001 instructions | +0.318% | -4.070% | neutral: high dispersion and VM disagreement |

The public scratch forms are one-handler/native ceilings. They do not pass the
separate public-surface necessity gate and are not recommended for assignment.

## Recommended separable ladder

1. **R2a - canonical direct reference-attribute materialization:** reuse the
   accepted exact reference-getter proof, remove the temporary link/unlink,
   retain canonical `copy_value`, and verify retargeted TRACE values.
2. **R1a - exact relink:** fuse only generated `UNLINK; LINKREF` sites, preserve
   unlink-first failure state, and use a private execution form after the
   compiler/RXAS layer proves eligibility.
3. **B1a - compiler scalar result forwarding, not yet selected:** reopen only
   where all exits and observations are equivalent and a compiler-owned PoC
   produces a stable multi-workload result. The fused `ICOPY; BR` form remains
   a neutral ceiling.
4. **R2b - guarded partial descriptor copy:** only if R2a proves that generic
   value copying still owns material time and a mechanical reference-shape
   guard can preserve every field and lifetime effect.

Each rung must be independently revertable and measured. No projections from
different rows may be added. SA-S1/SA-D1/SA-T1/SA-C1 remain neutral or deferred
controls unless an earlier rung fails and new exact evidence changes their
breadth or semantic owner.
