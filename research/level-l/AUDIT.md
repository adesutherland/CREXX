# Expanded research audit

This is an evidence index for the approved PLAN.md, not a replacement scope
record. All engines are research libraries. A mathematical argument about their
model is not machine verification of the cREXX compiler/runtime or a stable
public plugin ABI. Frozen-tool findings remain open and are documented below.

## Required portfolio and independent evidence

| Entry | Executed native code | Independent / decisive evidence | Disposition |
|---|---|---|---|
| ALG-01 | lllex, lldfabuild, lldfa, llmunch; native lexgen; unchanged authored surface adapter | re2c C and direct cREXX controls; byte/priority/span cases; triangular rollback versus q*n reverse work; tranches1/10 | Required experiment complete; Unicode breadth, tagged captures, refill and state minimization are product work. |
| ALG-02 | llpeg ll1 mode; llpratt | FIRST/FOLLOW rejection; 781 generated expression cases and7 value/tree cases against canonical LR; tranches1/3 | LL/Pratt are explicit specializations. |
| ALG-03 | lllr canonical LR(1), LALR merging | Bison3.8.2 canonical/IELR acceptance and LALR-only conflicts; native tree/invalid/resource cases; tranche2 | Native IELR not required; recommend it as later table-size work. |
| ALG-04 | llpeg packrat; lltabpeg settled reverse evaluator | priority/predicate counterexamples, independent consumer oracles,154 shared tree cases and all256 complemented-set bytes+EOF; tranches1/3/7/12–15 | Ordinary non-LR PEG control complete on its declared normalized domain. |
| ALG-05 | llpika, llpikalower, llpikatree |508 explicit-ranked Java cases and977 native-lowering Java comparisons; raw trees/counts/zero flags;707 independent language cases; tranches2/7/10 | Faithful port complete on byte profile; reachable-root pruning separately proved. Longest-choice experiment rejected for PEG preservation. |
| ALG-06 | llsquirrel, llc14, llc14cost |486 Dart core cases;237 full c14 trees/costs;2016 dominance checks; upstream ranking reversal; tranches1–4/11/16 | Separate recursive/recovery semantics; whole-search optimality and universal linearity unproved. |
| ALG-07 | llearley, llforest, llrepair | Bison GLR set-of-tree identities1,1,2,5,14; nullable/cyclic forests;480 weighted-distance controls against finite-language Wagner–Fischer; tranches2/4 | Required general CFG/repair-oracle control complete. Dense representation costs remain explicit. |
| ALG-08 | llstage, llregional, llgroup, llschedule, llproject |1555 token-pass cases;789 regional cases plus multiple regions;4096 attribute graphs; independent compiler tree adapters; tranches5/6/15 | Required composite mechanisms complete. Arbitrary fixups are not inferred from incomplete syntax. |

The GLR control initially using scalar additive merges counted16 rather than14;
its retained failure demonstrates why an independent oracle also needs review.
The final control deduplicates actual tree identities. It is a small-input
reference, not an efficient implementation claim. No unchanged valid panel is
rerun merely to accumulate green outputs.

## Mathematics M-01–M-08

| Obligation | Argument / checker / challenge | Scope of result |
|---|---|---|
| M-01 meanings | SEMANTICS; common IR; explicit ordered/contextfree/pika/squirrel profiles; SURF-01; priority and recursive counterexamples | Recognition, named tree, source and diagnostics are separate observations. No universal semantics conversion. |
| M-02 analysis | llcheck shared facts; llcap coordinator; backend-owned table/admission checks; mutation and same-revision/different-grammar negatives | Sound sufficient checks with unsupported/not-proved/invalid/construction outcomes. Plugin assertions require trust/review. |
| M-03 manipulation | REWRITE-01, SURF-01, OP-01; native EBNF/Pratt lowering and consuming-tree reconstruction; rejected longest-choice | Selected rewrites proved under purity/profile preconditions. DFA state minimization investigated mathematically, not implemented or measured. |
| M-04 composites | STAGE-01/02, TREE-02, COMP-01, ATTR-01; explicit boundary rejection and generated dependency-cycle witness | Finite modes, nesting counter, ordered grouping, DAG and finite-height join schedules. Arbitrary exits/type resolution remain services with separate contracts. |
| M-05 lex first | Initial boundary theorem, LEX-04 checker,46 Pika char/token cases; keyword/priority/cross-boundary negatives | Sufficient immediate-stop CFG substitution proof; no automatic preservation of unrestricted scannerless predicates or Unicode semantics. |
| M-06 bounds | LEX-02/03, PEG-02/03, CFG-02, LR-02, OP-01, PIKA-03; fuel/cell failures; adversarial rollback and recursive growth | Fixed-grammar input bounds plus explicit grammar factors, output and dense-storage costs. Fuel bounds completed work, not successful parsing. |
| M-07 recursive/recovery claims | REC-02–11, PRUNE-01/02, PIKA-02–04; reference counterexamples and held-out damage | Conditional event bound and local dominance proof; global optimality unknown. Hardness encoding, fixed-grammar and frontier-information qualifications are explicit. |
| M-08 tree/proof ownership | SEMANTICS; llcore contracts, llproject/llgroup; TREE-02; mutation/revalidation and B/G adapter | General source-tree infrastructure works for the migration slice. Typed semantic IR, symbols/types/control ownership and compiler exits remain later migration work. |

## Certificates and trust

Regular lexing emits LL-LEX-CERT-1 from the actual DFA, input mode and backend
version; it proves reverse q*n work or a sufficient bounded-backup property.
LL-CERT-2 includes exact grammar serialization, root, revision, profile, engine
version, bound and assumptions. Native LL/LR/Pratt emit conditional linear-input
claims; normalized packrat/tabulation emit their stated linear-in-input bounds;
Earley emits a conservative cubic-input work bound with explicit grammar factors.
Its dense storage is not quadratic. Pika's separate certificate requires an
acyclic unconditional event graph and bounded negative-lookup dependencies;
its grammar path factor may be exponential in grammar size.

`facts.matches` rejects a modified grammar and a different grammar with the same
revision. Boundary proofs bind both DFA and CFG. Tree certificates are invalidated
by mutation and reissued only after structural validation. Strict requests for
an unproved bound fail at admission; construction or runtime exhaustion never
becomes syntax rejection. Existing tests retain all these paths.

The serialized strings are explanatory receipts of trusted native checker
results; there is no independent proof-object kernel or signature-verification
protocol for untrusted external plugins. Do not transport a string supplied by
an arbitrary plugin as proof. The selected implementation, exact inputs and
cREXX value/lifetime behavior remain in the trusted base. Native APIs use owned
snapshots, so a caller's later mutation cannot change an admitted request.

## Remaining product and tool findings

The two frozen optimized-tool observations are preserved under
`evidence/c14-first-opt-failure/` and
`evidence/recovery-diagnostic-opt-failure/`. Split evaluation and stable diagnostic
categories allow the research comparisons; these are not compiler fixes or
proof of current-develop behavior. The lifecycle harness's early copy/select/
identity-stage errors are retained separately as harness failures. Neither
research test success nor this audit closes those frozen-tool investigations.
No sanitizer-clean, cross-platform, release or production-readiness claim is made.

AC-22/23 and the final AC-24/25 decision matrix remain open until their own
retained evidence is complete. This audit does not close them by describing
work still to be done.
