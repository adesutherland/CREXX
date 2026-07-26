# PERF2-04 semantic/machine-ceiling and placement panel

This panel selects an owner by end-to-end machine work, semantic completeness
and the earliest layer that owns the required facts. The native control is a
bound, not a default answer. The current Level B body remains the complete
fallback and behavior documentation in every selected design.

No panel entry is installed in production by this package.

## Placement summary

| Family | Clean inline result | Strongest proved ceiling | New assist/native result | Efficient correct owner | Disposition |
| --- | --- | --- | --- | --- | --- |
| `UPPER` | direct symbols inline; four computed/literal RexxCPS actuals do not | direct `strupper` result placement; stronger exact constant fold | existing `strupper` already is the semantic runtime control | general classified assembler-effect proof, read-only exposed binding, F03 result cleanup and instruction-level constant evaluation | revised first ladder owner; decision required |
| `LENGTH` | timed body already inline | 9-to-6 selected instructions, no scan/conversion reduction | existing `strlen` is already the ceiling primitive | bounded PERF2-03-F03 compiler result/block-exit cleanup | reject now: dual-VM wall verdict is neutral |
| `SUBSTR` | both timed bodies already inline | exact valid-site composition removes validation/default/result work; constant fold is stronger | existing cursor/slice primitives suffice for proved private constants | compiler proof/composition and trace-safe constant evaluation | selected family; no opcode |
| `WORD` | timed body already inline | exact consumer predicate avoids result slice/materialization; constant propagation removes all exact-site work | one benchmark site fails the general-assist gate | compiler consumer/value composition | select compiler case; no public helper/opcode |
| `POS`/search | current relevant body reaches one `strpos` | direct existing primitive | no multi-site deficit | current Level B inline/compiler composition | clean at ceiling |
| `LEFT`/`RIGHT` | bodies eligible/current forms retained | no current timed cell | no evidence | Level B fallback/current inliner | not material |
| `WORDS`/`WORDPOS`/`LASTPOS` | mixed I6 availability | no current timed cell | no evidence | Level B fallback | not material |
| `DATATYPE`/typed BIF conversions | size-significant bodies; no current optimized Tier A calls | language conversion opcodes are a different owner | native control would answer the wrong question | PERF2-07 value/representation programme | not material here |
| Base64 position/copy path | all BIF bodies already inline | Level B codepoint arithmetic removes repeated slice/search/copy work | native is far higher but semantically incomplete; one site cannot adopt an assist | separate maintainable Level B Base64 API under CAP-03 | algorithm winner, no PERF2-04 production slice |

## CAS - `UPPER`/`LOWER`

### Controls

| ID | Form | Machine shape | Result |
| --- | --- | --- | --- |
| CAS-C0 | current Level B `UPPER` call | exposed input, result initialization, one `strupper`, return; normal call for literal/computed actuals | current product |
| CAS-L0 | current body after caller materializes each actual as a symbol | current inliner accepts all four; no UPPER calls, four `strupper` scans | proves `UPPER` can inline today for the bindable call shape |
| CAS-H1 | hand-equivalent dynamic-input ceiling | evaluate once, direct `strupper` into final result | removes call and inline result scaffold |
| CAS-A1 | best Level B algorithm | unchanged current source: it is already one semantic primitive | no source algorithm gap |
| CAS-E1 | classified assembler-effect proof | select the exact opcode variant from validated operand shapes and consume the central RXAS read/write/kill/semantic effects; conservative, opaque, aliasing or contradictory evidence fails closed | recommended proof owner; no handwritten safe-opcode list |
| CAS-B1 | general read-only exposed-formal binding | retain the zero-copy Level B call fallback; when E1 proves a scalar exposed formal read-only and non-escaping, evaluate a non-locator actual once in private storage and use the normal inline binder | recommended call-boundary owner; applicable beyond `UPPER` |
| CAS-F1 | bounded PERF2-03-F03 result cleanup | forward the final scalar result and remove overwritten initialization/block-exit copies when source/TRACE and ownership proof permits | recommended dynamic-input placement owner |
| CAS-L2 | exact constant fold | canonical results `WITH`, `2`, `ARGS`, `(THIS IS THE SECOND)11` | strongest exact RexxCPS ceiling |
| CAS-V1 | general assist control | existing public `strupper` | a second assist duplicates the primitive and is rejected |
| CAS-N1 | native/intrinsic control | the canonical VM `strupper` handler and Unicode mapping | bounds the scan only; native BIF ownership adds no useful mechanism |

The focused call-shape probe retains a normal call for a literal and computed
expression but emits `strupper` for a symbol actual. Exact `rxc -d2` diagnostics
show all four RexxCPS failures at `inline.bind.actuals`, not at profitability or
body selection.

Post-selection review found that the proposed exact-body recognizer encoded
the symptom instead of the missing proof. `UPPER` uniquely retains a scalar
`arg expose` among the simple case/length seed bodies because the prior Level B
review measured the ordinary typed-string defensive link/copy sequence and
explicitly selected the zero-copy fallback. `LOWER` uses an ordinary typed
argument and already follows the general inliner. A scratch non-exposed UPPER
body inlines literal, symbol and computed actuals, but emits a private formal,
result initialization, one `strupper` and a return `copy`; no-opt restores the
`scopy/swap` prologue. Removing `expose` alone is therefore below the current
fallback and machine ceiling.

The central RXAS table already classifies `STRUPPER_REG_REG` as operand 2 read,
operand 1 write/kill with `MAY_THROW`. Compiler assembler-symbol validation
currently assigns read/write to every register operand, so the callable summary
cannot prove the exposed source read-only; the binder consequently accepts only
locator-shaped actuals. The revised owner consumes that central classification,
distinguishes binding mode from real write/escape effects and fails closed for
unclassified, opaque, aliasing, indirect-write or contradictory evidence. No
parallel safe-instruction list is proposed.

### Machine and end-to-end result

The isolated 100,000-iteration control medians are:

| VM | CAS-C0 literal | CAS-L0 symbol inline | CAS-H1 direct | direct/current |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 41.078 ms | 29.393 ms | 27.257 ms | 0.664x elapsed |
| `rxbvm` | 43.671 ms | 30.393 ms | 28.426 ms | 0.651x elapsed |

On canonical adaptive RexxCPS, the retained seven-sample ordinary Release
rates are:

| VM | CAS-C0 current | CAS-L0 symbol | CAS-H1 direct | CAS-L2 constant | constant/current |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 29,015,504 | 31,128,284 | 31,560,526 | 32,707,396 clauses/s | +12.724% |
| `rxbvm` | 27,416,214 | 29,408,680 | 29,657,075 | 30,566,014 clauses/s | +11.489% |

CAS-H1 beats CAS-L0 by 1.389%/0.845%. CAS-L2 then beats CAS-H1 by
3.634%/3.065%. Normalized retired instructions fall from
5,260.985800/5,263.193542 per iteration to 4,921.057963/4,921.983585,
6.461%/6.483%. CAS-L2 removes about 56 runtime `strupper` scans per timed
iteration; CAS-H1 retains the scans but removes call/result scaffolding.

The exact RXAS hot block is 17 instructions current, 25 in the raw symbol-body
inline control, 13 with direct result placement, and 9 for the scratch
constant ceiling. Whole-module scratch sizes include declarations and
post-timer guards and are not production size estimates.

### Placement

Changing the public Level B signature from exposed to by-value would add
fallback copying/metadata work and change an API contract merely to fit the
current binder. A generic new transported "read-only reference" fact would
invoke PERF2-03-F05. Neither is necessary.

The selected owner is an exact-body compiler composition using Architecture
H's already versioned body evidence: single evaluation/materialization for a
non-symbol actual, direct destination placement, current source/TRACE events,
and ordinary call fallback on any proof or profitability failure. A second
constant case may fold only through the authoritative simple Unicode mapping
and must synthesize/preserve the same observation. No public RXAS/RXBIN/ABI,
VM assist or native BIF is justified.

`LOWER` has the same one-primitive character but no current timed product
site. It remains fallback/guard breadth, not an attributed PERF2-04 gain.

## LEN - `LENGTH`

### Controls and machine result

| ID | Form | Result |
| --- | --- | --- |
| LEN-C0 | current Level B body through PERF2-03 | already inline; selected path `dcopy,dtos,load,strlen,icopy,br,isub,ilt,brf` |
| LEN-H1 | direct `strlen` result | selected path `dcopy,dtos,strlen,isub,ilt,brf` |
| LEN-A1 | best Level B algorithm | current source is already one `strlen`; source restructuring cannot improve the scan |
| LEN-L1 | compiler result/block-exit cleanup | destination forwarding across exact inline result and exit |
| LEN-V1 | assist control | existing `strlen`; no missing semantic unit |
| LEN-N1 | native control | existing VM `strlen` handler | same scan ceiling, no native-BIF benefit |

LEN-H1 removes exactly three instructions at 28 executions: 84 instructions
per top-level iteration. B0-P observes -83.9994/-84.0050, or
-1.596649%/-1.596754%, with `dcopy`, `dtos`, `strlen`, frames, standalone
values and string buffers unchanged. The scratch whole module grows only
because it contains two declarations and post-timer guards; it is not a
production estimate.

The first empty-string-initialized scratch exposed a real representation
hazard: stale codepoint-count metadata survived `dcopy; dtos` and made
`strlen` return zero. LEN-H1 uses typed-null state and proves the result `3`.
Any production cleanup must preserve value metadata as well as text.

The serial 2-warmup/7-sample ordinary Release matrix is neutral:

| VM | LEN-C0 median | LEN-H1 median | rate ratio | verdict |
| --- | ---: | ---: | ---: | --- |
| `rxvm` | 29,363,287 | 29,362,309 clauses/s | 0.999966693 (-0.003331%) | neutral |
| `rxbvm` | 27,533,292 | 27,739,431 clauses/s | 1.007486900 (+0.748690%) | neutral |

Paired seven-sample confidence intervals cross zero on both VMs. The exact
84-instruction reduction therefore does not satisfy the decisive end-to-end
gate and does not advance to production.

### Placement

This is exact static reopen evidence for PERF2-03-F03, not a reason to restart
general inliner cleanup. Its ordinary wall result is neutral, so F03 remains
closed for this case. If a future multi-site current profile changes the
end-to-end verdict, the correct owner remains compiler formal/result/block-exit
cleanup over existing `strlen`. A new opcode, VM fast path or native LENGTH
body cannot remove the remaining decimal-to-string conversion or Unicode scan
and is rejected.

## SLC - `SUBSTR`/`LEFT`/`RIGHT`

### Controls and exact selected cases

| ID | Form | Machine shape/result |
| --- | --- | --- |
| SLC-C0 | current complete Level B `SUBSTR` inline | validation, optional/pad setup, lengths/availability, cursor/slice, result copies/exits |
| SLC-H1 | hand-equivalent existing-primitives composition | S1 `setstrpos 5; substring 2`; S2 `substring 1` after a zero-cursor literal load |
| SLC-A1 | best general Level B algorithm | current safe taken path already uses one slice; no general source rewrite beats proved compiler specialization |
| SLC-CF1 | exact constant fold | precompute `"56"` and `"1"`; absolute selected-site ceiling |
| SLC-L1 | compiler proof/composition | admit only constant/private, positive, in-range, supplied-length cases; retain complete body otherwise |
| SLC-V1 | non-mutating span/slice assist | not needed by the two private literal sites and lacks multiple current product sites |
| SLC-N1 | native semantic body | bounds a full implementation but cannot beat no-runtime-work constant cases | rejected as owner |

Both current timed bodies are already inlined. SLC-H1 reduces their executed
paths from 29 to 3 and 30 to 2 instructions. At 14/28 executions it predicts
1,148 instructions removed per iteration; B0-P observes
1,149.107731/1,149.049299, -21.838%/-21.832%. All 42 substring operations
remain, while about 85 `strlen`, 28 redundant `setstrpos`, 255 branches and
112.65 string copies carrying about 259 bytes per iteration disappear.
Allocation counts do not materially move.

The current/direct scratch module comparison is 1,498/1,449 executable RXAS,
573/524 main instructions, 105/109 main locals, 220,731/214,819 RXAS bytes and
77,438/76,046 RXBIN bytes. Unlike other guarded ceilings, SLC-H1 is smaller as
a whole despite its scratch declarations and checks.

The serial 2-warmup/7-sample ordinary Release matrix confirms that the machine
reduction matters end to end:

| VM | SLC-C0 current | SLC-H1 direct | SLC-CF1 constant | H1/current | CF1/current | CF1/H1 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 28,818,112 | 31,318,043 | 32,145,168 clauses/s | +8.674860% | +11.545017% | +2.641049% |
| `rxbvm` | 27,540,875 | 29,437,133 | 29,765,360 clauses/s | +6.885250% | +8.077031% | +1.115010% |

All 42 recorded samples are correctness-qualified; the maintained runner marks
no cell for rerun. CF1 is the strict machine-work and median winner. Its small
`rxbvm` advantage over H1 is overlapping/close, so the production claim is the
proved 140-instruction reduction plus positive medians on both VMs, subject to
the mandatory first ordinary Release verdict—not an overstated rxbvm effect.

### Placement

SLC-H1 is valid only for private literal temporaries; exposing `setstrpos`
mutation on a caller-owned source is not generally correct. The current facts
are compile-time constants, so the earliest and fastest owner is a general
compiler certified-call constant evaluator, with direct existing-primitive
composition as the dynamic safe-case fallback and the complete Level B body
for every unproved case. The approved certification universe is deterministic,
non-I/O, non-random core Level B, but each active certificate still requires a
callable/signature/body identity proof and explicit context, effects, signal,
ownership and canonical-evaluator contract. P04-SLC1 initially admits separate
`UPPER` and proved-domain `SUBSTR` certificates; it does not infer purity from
the universe or introduce general loop-invariant motion. One benchmark does
not meet the multi-site public-assist gate.

`LEFT`/`RIGHT` keep their current complete padding/width bodies. Their only
current RexxCPS appearances are reporting/no-opt controls, so no production
mechanism is attributed to them.

## WRD - `WORD`/`WORDS`/`WORDPOS`

### Controls

| ID | Form | Intended distinction |
| --- | --- | --- |
| WRD-C0 | current `WORD` body inline | blank scans, selected substring/result, comparison |
| WRD-H1 | exact first-word equality predicate | blank scans, one-codepoint span check and `strchar`; no result string |
| WRD-A1 | best general Level B algorithm | current forward scan is already appropriate for general extraction |
| WRD-CF1 | propagated exact value | `key1` is assigned `"Key Bee"`; exact predicate is false if observation/evaluation proof permits |
| WRD-L1 | compiler consumer/value composition | lower an exact single-use predicate or constant fact; retain `WORD` otherwise |
| WRD-V1 | general word-span assist | one current site is insufficient; must beat cleaned Level B across multiple real sites |
| WRD-N1 | native family control | bound only; no evidence it is a better owner than no-materialization composition |

The current/H1/CF1 ordinary Release matrix is positive and well separated:

| VM | WRD-C0 current | WRD-H1 direct predicate | WRD-CF1 constant false | H1/current | CF1/current |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 29,310,271 | 30,257,667 | 31,277,784 clauses/s | +3.2323% | +6.7127% |
| `rxbvm` | 27,801,847 | 28,724,104 | 29,696,325 clauses/s | +3.3173% | +6.8142% |

All 42 recorded samples and all warmup/recorded semantic guards pass; maximum
cell span is 4.068%. WRD-H1 proves that consumer-aware composition matters,
while CF1 is the strict exact-site ceiling because the compiler already emits
literal `"Key Bee"` and the comparison target is literal `"?"`.

At fixed 100x100 B0-P counts, WRD-H1 reduces normalized work from
5,224.4121 to 4,706.4122 instructions per outer iteration (-517.9999,
-9.914989%). WRD-CF1 reaches 4,412.4091/4,412.4093 (-812.0030/-812.0028,
-15.5425%) and removes all 28 timed blank scans, slices and result copies per
iteration. Standalone-value and string-buffer allocation counts remain
unchanged; the gain is scan/instruction/materialization removal.

The setup `word(version_info,1)` call is outside the timer and remains a normal
call. `WORDS`/`WORDPOS` have no optimized Tier A dynamic site. No helper/API or
public word opcode is selected from the one predicate use.

## SRC - `POS` and related search

`POS` validates a positive one-based start, handles empty operands, initializes
the start/result register and invokes one existing `strpos`. The relevant
optimized uses already inline. RexxCPS's 400 residual POS calls are in fixed
TRACE setup/control before clause timing; Base64's search calls are already
inlined and their cost disappears only when its algorithm stops searching a
fixed alphabet.

The hand, compiler, assist and native controls therefore converge on the
existing `strpos` primitive. Current Level B inline/composition is selected;
no new search instruction or native BIF is warranted. `LASTPOS` is not current
material and retains its complete source loop.

## DAT/CNV - typed conversions

The current optimized portfolio has no dynamic Level B `DATATYPE`, binary/hex/
character/decimal conversion call among the seed family. The `datatype` module
is statically large (620 optimized instructions, 30,271 RXBIN bytes) but size
alone does not authorize a mechanism.

RexxCPS's timing profile attributes 25.444/25.125 ms to `ITOS`,
16.551/16.047 ms to `DTOS`, and 5.533/5.549 ms to `STOD` on `rxvm`/`rxbvm`.
Those are language/value representation conversions, not BIF call bodies.
Compiler/value representation work belongs to PERF2-07; a PERF2-04 BIF native
control would answer the wrong ownership question and is rejected.

## B64 - Base64 position/copy path

### Controls

| ID | Form | Decoder instructions/peak locals/code bytes | Dynamic result at 500 repetitions |
| --- | --- | --- | --- |
| B64-C0 | current Level B slice/search decoder | 404 / 40 / 1,349 | 46,726,464 instructions; 2,907,504 string copies; 1,877,756,578 copied bytes |
| B64-A1 | codepoint extraction plus existing `poschar` | 90 / 28 / 322 | 16,787,964 instructions; 1,004 copies; 2,058,598 bytes; 683,003 hidden alphabet searches |
| B64-A2 | codepoint extraction plus ASCII range arithmetic | 209 / 36 / 691 | 21,431,964 instructions; 1,004 copies; 2,058,604 bytes; three fixed `poschar` checks only |
| B64-L1 | compiler fusion of current calls | can reach A1, but cannot infer fixed-alphabet arithmetic without benchmark-shaped semantics | rejected as owner |
| B64-V1 | reusable assist | existing `strchar`/`poschar` already form the control; one site fails adoption gate | rejected |
| B64-N1 | fixed-valid native C ceiling | 33,824-byte diagnostic binary | 4.480 ms median at 2,500 repetitions; malformed/API semantics absent |

A2 retires more VM instructions than A1 because its range classification is
visible bytecode, yet it does far less hidden native scan work and is much
faster. This is why placement is based on total machine work, not opcode count
alone.

The VM implementations of `strchar` and `poschar` call
`string_set_byte_pos()` on their string operand, so they mutate cached cursor
state even though the current RXAS effect row describes operand 2/3 as value
reads. The Base64 controls use private strings and remain correct, but a general
compiler/RXAS fusion cannot assume cursor purity. Correcting/generalizing that
effect belongs to the routed assembler-effect work; it is another reason not
to adopt a one-site assist here.

Ordinary Release medians at 2,500 repetitions:

| VM | B64-C0 | B64-A1 | B64-A2 | A2/C0 speedup | A2 elapsed reduction |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 1,775.334 ms | 580.242 ms | 145.622 ms | 12.191x | 91.797% |
| `rxbvm` | 1,812.428 ms | 611.462 ms | 171.654 ms | 10.559x | 90.529% |

The native fixed-valid control is 396.28x/404.56x faster than current and
32.50x/38.32x faster than A2, but it has no malformed-input, signal, TRACE,
startup, RSS or API proof. It remains `native upper bound only`.

CAP-03 explicitly keeps the common benchmark unchanged and requests a
separate pure Level B Base64 API. B64-A2 is the selected algorithm and Level B
is the selected owner for that future API, subject to its own full semantic
matrix. It is not a PERF2-04 compiler/opcode/native production slice.

## Ordered production ladder

### Cumulative bound

`CF-COMB1` combines only the three selected exact ceilings—CAS-L2, SLC-CF1
and WRD-CF1—against the exact same B0-R product. It excludes neutral LEN-H1
and makes no Base64 source change. Normalized B0-P work falls as follows:

| VM | current instructions/outer | CF-COMB1 | reduction |
| --- | ---: | ---: | ---: |
| `rxvm` | 5,259.941373 | 2,812.821111 | 46.523717% |
| `rxbvm` | 5,262.067143 | 2,813.374124 | 46.534811% |

The bounded serial 2-warmup/7-recorded ordinary Release matrix gives:

| VM | current median | CF-COMB1 median | cumulative change |
| --- | ---: | ---: | ---: |
| `rxvm` | 29,293,128 | 38,924,099 clauses/s | +32.877919% |
| `rxbvm` | 27,502,423 | 36,963,282 clauses/s | +34.400093% |

All 28 recorded executions pass, the candidate wins all seven paired rounds on
each VM, and no cell requests a rerun. The combined scratch module is larger
than current (2,172 versus 1,402 executable instructions; 113,938 versus
77,438 RXBIN bytes) because it carries all family predeclarations and
post-timer semantic guards. Those bytes are deliberately not a production
size estimate. CF-COMB1 is a cumulative TRACE-off machine ceiling, not an
implementation or a claim that the isolated percentages add.

### Recommended three-slice ladder

The smallest ladder justified by the complete panel is all three positive
compiler families below, kept as independent slices. Selecting only the first
slice is a valid prefix, but the evidence does not justify closing the two
later positive families merely to keep the initial change small.

| Order / stable ID | Production owner and bounded scope | Mandatory distinguishing proof | First ordinary Release verdict | Revert boundary |
| --- | --- | --- | --- | --- |
| 1 — P04-CAS1 | General compiler support for classified assembler operand effects and read-only exposed scalar formals. Preserve the current zero-copy Level B call body; evaluate a non-locator actual exactly once in private storage, apply bounded F03 result placement, and use an instruction-level case-transform evaluator for a constant result only when observation can be preserved. Conservative/opaque/aliasing/contradictory metadata retains the call. No public opcode, RXBIN or native BIF. | Generated proof that source opcode variants consume the central RXAS effect record; contradictory summary/body CTest; symbol/literal/computed/repeated/overlapping actuals; empty, multibyte and embedded U+0000 mapping; source unchanged and alias lifetime; local/source/binary imports; missing/old/malformed evidence; optimized/no-opt and exact TRACE/source event order on both VMs. Retain the PERF2-03 getter/setter guards and prove non-UPPER qualifying bodies use the same mechanism. | Isolated UPPER control plus canonical RexxCPS. CAS-H1 is +8.771%/+8.173%; the exact constant ceiling is +12.724%/+11.489%. | One general proof/binding/cleanup family and tests; `upper.crexx`/`upper.md` stay unchanged as fallback/documentation. Architecture selection is required before implementation. |
| 2 — P04-SLC1 | General compiler certified-call constant evaluation over individually proved deterministic core Level B callables. Initial separable entries are constant `UPPER` and the two proved `SUBSTR` cases; existing `setstrpos`/`substring` composition remains the dynamic positive/in-range fallback. No public/private assist or serialized metadata change. | Exact callable/signature/body certificate; constant actuals and evaluation order; UPPER empty/ASCII/Unicode/embedded-U+0000; SUBSTR positive one-based/in-range/supplied-positive-length plus explicit fallbacks for empty, omitted versus zero length, end/out-of-range, default/explicit one-codepoint pad, invalid pad, signals, aliases and TRACE/source; local/source/binary imports, shadowing, no-opt and both VMs. | Accepted CAS1 predecessor versus the combined but separately counted initial certificates on canonical RexxCPS; retain per-entry generated-work attribution. | One certificate-registry/fold family and two independently disableable entries; all other eligible core BIFs remain uncertified, Level B sources remain unchanged and no general loop-invariant motion enters this slice. |
| 3 — P04-CEX1 | Extend the same exact certificate registry with `LOWER`, `LENGTH`, `LEFT` and `RIGHT`. Admit only proved constant, non-signalling cells; bound compile-time result construction at 1,048,576 codepoints and retain the complete Level B call otherwise. | Exact I6/body fingerprints; source/binary/no-opt; nested folding; Unicode/combining/U+0000; empty/zero/equal/truncate/pad; exactly-one-codepoint validation and signal order; dynamic and oversized fallback; same-summary contradictory LOWER body; both VMs. | Focused constant-use Release cell: 286 to 20 executable RXAS, 16,336 to 2,400 RXBIN bytes; repeated cell 21.094049x/21.934257x faster. Current RexxCPS/library/rxpp artifacts remain byte-identical. | Four separable descriptors/evaluators in the existing registry; no Level B, RXAS, RXBIN, ABI, VM or native change. |
| 4 — P04-WRD1 | Extend the exact certified-call registry with constant `WORD(source,wordnum)`, and run the ordinary constant fold/propagate fixed point before and after certified evaluation. This lets flow-proved actuals and ordinary consumers compose without a WORD-specific rule. Keep general/dynamic `WORD` materialization in the Level B fallback. | Empty/blank-only, ASCII and Unicode whitespace, multibyte and missing words, strict equality/prefix/fullwidth mismatch, invalid word-number signals, result ownership, TRACE/source, source import, same-summary/opposite-body contradiction, no-opt and both VMs. The constant timed path remains distinguishable from the dynamic setup path. | Accepted CEX1 to WRD1 canonical RexxCPS: +7.650234%/+8.812155%; 68 executable instructions and the complete timed word scan/slice/false branch disappear. | One exact descriptor/evaluator plus general fold scheduling and tests; `word.crexx` remains the full materializing fallback; no helper API or word opcode. |

P04-SLC1 has now completed its provisional first-verdict gate. The general
registry initially enables two independently removable certificates:
`rxfnsb.upper` for all constant strings and `rxfnsb.substr` only for constant,
positive, supplied-length, fully in-range cells with a valid one-codepoint pad.
The resolved provider must match the exact typed I6 summary and normalized body
fingerprint; any mismatch or unproved domain retains the complete Level B call.
The current evaluators use the VM's canonical Unicode/value primitives. A
future registry backend may execute the certified Level B body through the
compiler's VM bridge without changing certificate selection or call-site proof.

Against accepted P04-CAS1, the exact Release product moves from 1,489 to 1,387
executable RXAS instructions, 572 to 470 in `main`, 105 to 100 main locals,
223,849 to 204,309 RXAS bytes and 78,162 to 72,106 RXBIN bytes. All four
constant `UPPER` scans and both selected `SUBSTR` bodies disappear; unrelated
dynamic slices remain. The linked library and both VMs are byte-identical.
The serial 2-warmup/7-recorded verdict reaches 36,229,324/33,904,454 clauses/s,
an incremental +14.436115%/+14.060120% on `rxvm`/`rxbvm`. All 18 executions
pass and neither cell requests a rerun.

P04-CEX1 then proves the four adjacent core-string certificates. In its exact
semantic cell, two `LOWER`, three `LENGTH`, three `LEFT` and three `RIGHT`
constant uses fall from 286 to 20 executable instructions and 16,336 to 2,400
RXBIN bytes. The repeated ordinary Release cell is 21.094049x/21.934257x
faster on `rxvm`/`rxbvm`; all 68 recorded runs pass. The required ten-sample
append is retained, as are the candidate's noisy short-process spans. Every
candidate maximum nevertheless remains more than 18x below the matching SLC1
minimum. Current RexxCPS, linked library and rxpp artifacts are byte-identical,
so CEX1 is a general constant-use result rather than a Tier A movement.

This facility consumes constants after the ordinary flow-aware expression
folder has reduced each actual. Flow-proved constants therefore benefit
without a BIF-specific data-flow rule, and nested certified calls compose to a
constant. It does not hoist calls: a loop-invariant but non-constant actual
retains the normal Level B path. General loop-invariant motion remains a
separate future analysis decision with alias, signal, TRACE and lifetime proof.
Adrian accepted P04-CEX1's favorable first verdict on 2026-07-25. P04-WRD1's
frozen provisional successor now also has a favorable first verdict: exact
constant `WORD` evaluation plus general consumer folding removes 68 executable
RXAS instructions, and RexxCPS reaches 39,000,952/36,892,167 clauses/s on
`rxvm`/`rxbvm`, +7.650234%/+8.812155% over accepted CEX1. This is the mandatory
stop for Adrian's acceptance before broad QA or closeout.

P04-CAS1 comes first because it removes the only selected residual hot BIF call
boundary, proves the requested literal/computed-actual `UPPER` case and has the
strongest isolated dual-VM result. P04-SLC1 then removes the largest already-
inlined validation/slice scaffold. P04-CEX1 extends the same registry to four
generally useful constant string functions. P04-WRD1 follows because its
benefit requires constant propagation before certified evaluation and consumer
folding afterwards. The combined ceiling shows that completing all selected
timed families is materially worthwhile.

Dynamic LEN-H1 remains outside the ladder: its exact 84-instruction RexxCPS
reduction is neutral in the deciding wall cell. That does not reject constant
`LENGTH`, which advances through CEX1. Base64 A2 is routed to the separate
CAP-03 Level B API track. `POS`, remaining word siblings and typed BIFs retain
their recorded clean/not-material dispositions. No public RXAS, RXBIN, ABI,
VM-assist or native production change is recommended by PERF2-04.

Every recommended slice must be independently guarded, measured and
revertable. Adrian must select the next slice or ladder before implementation.
The first approved production edit then receives only the mandatory focused
correctness and ordinary Release decisive-target verdict before another stop.
