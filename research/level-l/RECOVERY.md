# Recovery comparison

The algorithm alone does not make a helpful diagnostic. Grammar labels, retained
source, context and explicit repair semantics matter. The native controls retain
byte locations, source/caret rendering, related quote/comment openers, stable
categories, proposed edits or skipped/missing structure, and clean/recovered/
invalid/resource distinctions. No human usability study was performed.

## Frozen comparisons

| Observation | Array panel:33 damaged,24 marked later arrays | Quoted/comment panel:28 damaged,20 marked later records |
|---|---|---|
| Ordinary PEG farthest / LR expected set | syntax rejection with expectations | syntax rejection with expectations |
| Explicit PEG recovery labels |22 complete recovered parses;15/24 later |12 complete recovered parses;8/20 later |
| LR local insertion/deletion search |31 complete edited parses;24/24 later |28 complete edited parses;20/20 later |
| Faithful Pika |root rejects;24/24 clean named islands |root rejects;20/20 clean named islands |
| Squirrel c14 |33 full-span error analyses;24/24 later |28 full-span error analyses;20/20 later |
| Frontier experiment |same40 raw trees as c14 |33/34 raw trees equal;Q020 changes at equal cost |

The recovery counts above exclude clean controls. The all-case summaries also
include five clean later arrays and two clean later records, giving denominators
29 and22 respectively. Earlier prose mixed all-case array counts with
damaged-only lexical counts; this 11 October clarification changes no engine
results. See [the exact reconciliation](evidence/user-guide/recovery-counts.json).

The array panel has40 cases/13 held out; the lexical panel34/13. Clean inputs
remain clean. The quoted/comment lexer stops on24/28 damaged inputs; its four
successes mean tokenization, not a grammatical parse. Later-record retention
is not measured for this admission gate. Recovery engines in that panel read
original bytes; scanner success is not a prerequisite. A token-only pipeline
needs its own lexical error tokens, lexical recovery, or a raw-input fallback.

Labels are economical and can be specific, but misplaced labels alter the clean
language or conceal the actual error. The retained `[x];[a]` example says a
closing bracket is expected where the real issue is the invalid item. Closing
quote/comment labels are useful after an opener has committed; they cannot
repair every damaged opener. No held-out retuning was performed.

The LR control minimizes edits in its bounded local stack search, then ranks
forward progress. It does not minimize the whole-file edit distance: on
`[a],a];[a]` its local cost is3 while inserting one opening bracket is globally
cost1. Equal-cost repairs may also delete useful content. Suggested scripts,
origin maps and any subsequent errors remain available.

Pika's explanation uses a bounded positive-clause chart heuristic. The expected
construct is a hypothesis, not an exhaustive expected-token set or proof of
intent. Whole-chart islands remain useful even when the requested root fails.
A root-reachability pruning policy must keep any additional island roots that
a consumer wants to inspect.

The c14 objective ranks error regions/structural readings; it is not ordinary
byte insertion/deletion distance. In held-out Q020, `x="a"/a*/;x="b"`, faithful
c14 inserts a zero-width missing star and retains the comment's `a`. The
frontier chooses a different cost1 error span and empty comment body. Both
retain the later record. The known mutation supports the former reconstruction
for this fixture; equal cost does not make the frontier result better. Its
previous algebraic ranking-reversal repair remains valid on its stated domain.
Global recovery optimality remains unproved.

## Evidence and units

`evidence/recovery-summary.json` and `evidence/lexrecovery-summary.json` derive
from complete outputs in tranches11/16. The independent recognizers use exhaustive
short-input insertion/deletion BFS. Substitution costs2; native exact-oracle
cross-checks cover selected small cases. Cost beyond the BFS bound is unknown.
The original lexical oracle accidentally treated UTF-8 `é` as one character
edit. The corrected oracle treats its two bytes separately; Q034 is cost2.
`evidence/lexrecovery-oracle-unit-correction/` preserves the error. No apparent
local/global gap from that unit mismatch is used as evidence.

Work counters are engine-specific. c14/frontier array work is42916/46722 and
allocations30307/33747; lexical work24255/24455 and allocations15999/16180.
These are counts, not wall times or live bytes. Frontier retention buys no
later-record improvement on these panels and changes one reconstruction
adversely against the known mutation. Keep both policies experimental.

The useful-consumer findings also matter: the frozen RexxScript reference raises
a host signal on an unclosed quote; the prototype lexer produces a lexical
failure. B/G/C initial-tree controls preserve actual syntax errors separately
from later missing-import failures. These observations motivate an explicit
lexical/parse/semantic diagnostic contract; they do not establish whole-language
compatibility or a current production defect repair.
