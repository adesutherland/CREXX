## Generalized comment diagnostics — 12 October 2026

The shared coordinator now reports named unclosed block/nested forms with the
original opening byte range. Duplicate opener admission includes related
rule locations; invalid/nullable/recursive/overlapping outside patterns and
unsupported qualifiers have explicit build explanations. Strict UTF-8,
resource/contract failure and target recovery remain distinct.

The focused comment panels cover default/custom opacity, kept versus hidden
line endings, original spans and record recovery on LL/LR/PEG/Earley, plus
Pratt, the LR/Pratt region and the standalone DFA route. Three additional
regressions guard custom metadata in RXAS, optional integer argument presence,
and the first exhaustion count; literal delimiter identity has its own numeric
spelling control. Counts and current source/tool identities are in the
[comment receipt](../../../research/level-l/evidence/comments-20261012/README.md).
An unclosed target block is unrecoverable under the current profile. General
nested/token-stage repair, captured-context placeholders and wider display
coordinates retain their open programme obligations.

# Level L diagnostic and recovery assessment

**12 October continuation, bounded native qualification verified:** source excerpts now
use ASCII escapes by default for tabs, controls and non-ASCII scalars, so caret
positions are deterministic. Headers still report original byte offsets and
Unicode scalar columns. `llsource..diagnostic(...,'unicode')` provides an
optional Unicode excerpt; font-dependent cell widths remain outside that view.
Pratt and regional failures include the actual unexpected lexeme. Recovery
messages are presented in source order.

Target recovery can explicitly mask malformed UTF-8 in a working copy, retain
original damaged byte ranges and skip the containing synchronization interval.
It returns a recovered result, never a clean/executable result. Strict parsing
still rejects malformed encoding. Unclosed target comments remain unrecoverable
under the current flat-record contract.

Proper grammar readers inspect isolated declaration candidates after lexical
quote/comment damage, using line-aligned declaration boundaries. Such candidates
are diagnostic information; the original document stays failed and no code is
emitted. Valid quoted/commented text remains opaque. L0 keeps its minimal
first-error behavior. The new backend/coordinate and resync panels pass
under CONT-AC03; historical baseline descriptions below remain dated.

**Common-surface implementation update, 11 October:** the product now has
simple rule messages, unexpected lexemes, stack-checked LR expectations, and
shared message rendering for LR, LL(1), PEG and Earley. Proper specification
readers can collect syntax errors at lexically recognized declaration
boundaries; the seed stays minimal. A separate bounded recovery request can
skip damaged flat records, retain later structure and return a nonzero status.
The growing [settings example](user-guide.md#grow-one-settings-grammar) shows
the concrete syntax and API. Full qualification of the expanded checkpoint is
still open in PLAN.md; these mechanisms do not complete every item below.

The remainder records the **earlier first-error baseline**, followed by the
worklist. Reuse its evidence for the inputs it identifies, not as a current
claim that the new recovery API is absent. In particular, source preparation
still stops at malformed UTF-8 or an unclosed comment; specification lexical
damage, general nested recovery, captured-message fields and display-cell
coordinates remain open. A recovered tree is for inspection, never permission
to execute a script.

**Bootstrap checkpoint update:** the generated specification frontend now
supplies syntax recognition; the seed is absent from the delivered tool.
Named `malformed`/`otherwise` diagnostics and EOF metadata are effective, and
CR/CRLF grammar coordinates are counted consistently. The current observations
below supersede the earlier PoC findings for those features. Tree annotations still do not
customize generated views. Recovery remains absent. See the
[bootstrap contract](bootstrap.md) and its current qualification receipt.

This assessment checks the local `levell` implementation on 11 October 2026.
The [user guide](user-guide.md) describes its supported surface. The
[current tutorial receipt](../../../research/level-l/evidence/bootstrap/user-guide/receipt.json)
checks the delivered generated frontend. The
[earlier audit](../../../research/level-l/evidence/user-guide/receipt.json) remains
historical evidence; research recovery comparisons are retained separately.
No recovery engine was added to the product during this bootstrap increment.

## What grammar authors receive

The frontend identifies ordinary errors by source line and zero-based byte
offset. Duplicate declarations, missing names and incomplete groups have useful
direct messages. For example, the two-undefined-reference probe reports only:

```text
LL_GRAMMAR: line 28, byte 1080: undefined symbol absent_one
```

It does not continue to `absent_two`. The scanner/declaration builder shares a
single stored error; after failure there is no pass that seeks the next safe
declaration and continues collecting independent diagnostics.

Backend refusal is often more explanatory than a raw parser-state report:

```text
LL_LR_ADMISSION: LL_LR_CONFLICT: LR(1) conflict before end of input. Cannot choose between completing left.number (line 9) and completing right.number (line 10). State the intended choice in the rules (for example factor the shared prefix or separate expression precedence). A conflict alone does not prove ambiguity.
```

The nullable-token diagnostic explains that a token must consume input.
Recursive lexical rules are identified as unsupported by the selected backend,
with a separate stage suggested for nesting. Ordered semantics and predicates
are refused as capabilities rather than silently reinterpreted as CFG rules.
Construction limits are distinguished from semantic impossibility.

These messages are a useful starting point. They still lack the shortest
conflicting example, a derivation showing both interpretations, a related-source
range for each side, and a distinction between likely language-preserving and
language-changing suggested edits. “Factor the shared prefix” is generic
guidance, not a proved repair. The research witness tools have some of these
capabilities; the product command does not call them.

Generated-frontend syntax errors and malformed UTF-8 ingress use the source
formatter and include a caret. Semantic errors such as undefined references
still have only a line and byte offset. `levell` does not consistently include
the filename; the CMake wrapper supplies it. LF, CRLF and CR specification
coordinates now agree, as checked across all three bootstrap stages.

### Declared behavior and remaining admission gaps

Lexical actions now deliver their contract; the remaining admission gaps still
need attention:

| Surface | Current observation | Consequence |
|---|---|---|
| A `diagnostic` declaration and `otherwise then error name` | The message is emitted in both source/RXAS definitions and returned with the diagnostic name on unmatched input. | Active; the current tutorial and lexer controls exercise it. |
| `tree rule name.value kind 42 arity 1` | Generation succeeds with byte-for-byte identical output to the same specification without it. | The frontend's schema is not delivered to the generated language. The current named view is a separate fixed API. |
| Special `eof`/`malformed` lexer rules | EOF metadata and malformed-input messages are emitted. An ABI adapter can append the zero-length EOF record; parser streams use implicit EOF. | Active; scalar and historical lexer controls exercise the distinction. |
| Quoted nonempty parser literals with `input tokens` | Source inspection shows byte values become numeric terminal IDs; no lookup maps the spelling to a lexer token. | A plausible rule such as `name '=' value` can describe the wrong token language. Use `name equals value`. |

The first three rows have direct generation/runtime probes; quoted token
literals remain a source-inspected limitation. Before wider adoption, an accepted declaration
must either implement its advertised contract or fail generation with a clear
capability diagnostic. The same rule should apply to every backend plugin.

## What target-language users receive

The tutorial consumer calls `llsource..diagnostic` to add the input filename,
line, column, original source excerpt and caret to the library's raw error.
RexxScript does the equivalent with stable `RS_*` categories.

| Input problem | Actual product behavior |
|---|---|
| Missing settings value | At the separator after `first =`, reports `expected number, string`. |
| Missing equals sign | Reports `expected equals` at the following token. |
| Missing assignment separator | Includes `separator` among the expected candidates. |
| Unknown character | Generic lexical error at the unmatched byte. |
| Unterminated quoted value | Generic unmatched-byte message at the opening quote; it does not explain the missing closing quote. |
| Unterminated nested comment | Specific `LL_COMMENT: unclosed nested comment` at the opener. |
| Invalid UTF-8 | `LL_UTF8`, original byte location and hexadecimal excerpt. |
| Two bad assignments followed by a good one | One diagnostic; no later assignment is returned to the visitor. |
| Earlier syntax mistake followed by a lexical mistake | The lexical mistake is reported first because whole-stream lexing precedes parsing. |

The ten malformed settings inputs were run on both concrete VMs, with the
same outcomes. A separate `rexxscript1` probe containing two bad assignments
and a later `SAY` produces one `RS_PARSE` diagnostic and does not execute the
later statement. This verifies the current rejection boundary; it is not a
successful recovery test.

Parser expectations come from nonzero actions in the current LR table row,
including reductions. They are **candidate tokens**, not a list verified by
simulating the complete parser stack. The parser does not yet name the actual
unexpected lexeme, group a long candidate list into “expression” or “statement”,
show a nesting/context chain, or attach the opening delimiter as a related
location. Those improvements can make a deterministic LR parser substantially
friendlier without changing its recognition algorithm.

Location handling retains exact original byte offsets through comment masking
and token stages. The formatter counts Unicode scalars; it does not calculate
display-cell widths for tabs, wide characters and combining sequences. This
is adequate source indexing but imperfect visual caret alignment. Grammar and
target failures should ultimately use the same structured location service.

## Resynchronization in the product

There is currently no insertion/deletion search, panic-mode skip to a language
boundary, error-token continuation, multi-diagnostic collection or successful
partial-tree result. The parser returns at its first invalid action. A partial
internal arena is not a recovered application tree.

Recovery must span the whole pipeline. A parser that can resume after a missing
semicolon is no help if the lexer has already stopped on a damaged quote.
Conversely, blindly skipping to the next newline can corrupt a language where
strings, comments or statements may span lines. The grammar author needs an
explicit contract for which boundaries are safe and what damaged structure is
preserved. Resuming analysis must remain separate from permission to execute.

## What the research results establish

The retained experiments compare different objectives. LR local repair edits
the input until it can continue; Pika can retain clean named fragments while
the requested root still fails; Squirrel c14 constructs an error-bearing
structural analysis. A full-span c14 result is not an error-free parse, and an
island is not a repaired whole program.

For clarity, the following table counts **damaged inputs only**, including
later-structure denominators. It is derived from the frozen records, not a new
timing or engine run:

| Engine/control | Array panel: 33 damaged; 24 have a marked later array | Quote/comment panel: 28 damaged; 20 have a marked later record |
|---|---|---|
| Explicit PEG recovery labels | 22 complete recovered parses; later array retained in 15/24 | 12 complete recovered parses; later record retained in 8/20 |
| Bounded LR local insertion/deletion | 31 complete edited parses; later array retained in 24/24 | 28 complete edited parses; later record retained in 20/20 |
| Faithful Pika island inspection | Root rejects; clean later array available in 24/24 | Root rejects; clean later record available in 20/20 |
| Squirrel c14 error analysis | 33 full-span error analyses; later array retained in 24/24 | 28 full-span error analyses; later record retained in 20/20 |

The complete panels also include seven/six clean controls, respectively, and
13 held-out cases each. All-case summaries count five additional clean later
arrays and two additional clean later records: hence 29 and 22 in those JSON
denominators. Those extra observations must not be represented as recovery
from damage. The old recovery narrative used the all-case array denominator
and the damaged-only lexical denominator; its units are now made explicit.

The results support further work on ordinary LR recovery. They do not establish
recovery quality for full RexxScript, all languages or real users. In particular:

- The LR method has a retained case with local edit cost 3 where a global
  one-edit repair exists. Equal-cost choices may discard useful structure.
- Misplaced PEG labels can hide the real fault or alter clean acceptance.
  One retained example blames a closing bracket when the item is invalid.
- Pika explanations are bounded chart-based hypotheses about missing
  structure, not an exhaustive expected-token proof.
- The c14 frontier refinement changes one quote/comment reconstruction at
  equal cost without improving later-record retention; the known mutation
  favors the original reconstruction. Global optimality remains unproved.
- The separate lexer rejects 24/28 damaged quote/comment inputs. Recovery
  engines in that comparison consume the original bytes; their success cannot
  be transferred directly to the product's stop-on-error token pipeline.

There was no human usability study. The
[research recovery report](../../../research/level-l/RECOVERY.md),
[frozen array records](../../../research/level-l/evidence/recovery-summary.json)
and [lexical summary](../../../research/level-l/evidence/lexrecovery-summary.json)
retain the counterexamples and exact interpretation. The guide audit includes
a [denominator reconciliation](../../../research/level-l/evidence/user-guide/recovery-counts.json).

## Approved next increment and surface review

Adrian has selected DFA, LL(1), canonical LR(1), ordinary packrat PEG, Pratt and
Earley for the initial supported toolkit, including LL(1) and Earley from the
start. Shared error reporting and resynchronization are authorized work under
[PLAN.md](../../../research/level-l/PLAN.md), LANG-AC01–14. Pika and Squirrel
are not dependencies. The mechanisms below must now be integrated with the
common result/recovery contract across all admitted backend profiles; starting
with LR does not complete that requirement. Exact new syntax and Unicode/
composition policies are the requested
[surface discussion](../../../research/level-l/SURFACE.md#common-surface-unicode-and-composition-review--11-october).
This scope update does not change the measured current behavior above.
Easy authored messages with the relevant context are an explicit beginner
surface requirement (LANG-AC14); they must not require a recovery algorithm,
custom tree schema or host callback. The proposed rule-bound declaration is in
the surface discussion and does not yet change current product messages.

1. **Make admission and reporting trustworthy.** Reject or implement the
   remaining ineffective tree declarations; diagnose quoted token-language
   literals (duplicate token IDs are already rejected); unify source ranges, filenames, stable codes and
   line endings. Preserve the distinction between unsupported, conflict and
   resource exhaustion. These fixes protect the grammar author's understanding
   of what was actually built.
2. **Improve explanation before repair.** Name the unexpected token and its
   text, filter LR candidates with bounded stack simulation, and allow authored
   construct labels such as “expression after `=`”. For grammar conflicts, add
   bounded witness search with both derivations; report “no witness found
   within this bound” rather than claiming unambiguity. Shared analysis should
   provide common facts; the backend should explain its own admission failure.
3. **Recover the Level L specification itself.** Collect independent errors
   and resume at a verified `name: kind` declaration boundary outside strings,
   comments and groups. An unclosed quote/group needs an explicit recovery
   policy, not an arbitrary colon search. Suppress secondary undefined-name
   cascades caused by a declaration already marked invalid. Any error still
   prevents artifact publication.
4. **Add an explicit target-language recovery contract.** Preserve lexical
   damage as located error tokens/regions, support bounded LR local repairs,
   and fall back to grammar-declared statement or block boundaries. Respect
   nested delimiters. Return diagnostics plus an error-bearing analysis tree,
   with original spans and inserted/skipped origin information. Execution
   continues to require a clean result. The exact authoring/API surface needs
   Adrian's language-design review.
5. **Qualify usefulness on real mistakes.** Extend the frozen and held-out
   panels to the settings language and actual RexxScript grammar: independent
   errors, damaged delimiters, keywords used as names, comments, Unicode,
   misleading equal-cost repairs and resource exhaustion. Measure correct
   primary blame, later-error discovery, retained valid structure, spurious
   cascades and bounded work separately. Have cREXX programmers try the
   messages; passing generated fixtures cannot substitute for that feedback.

At each stage, termination should be testable: recovery either consumes input,
makes a bounded insertion, changes to a strictly progressing synchronization
position, or exhausts a declared budget. A finite budget proves an upper bound
on work, not that the chosen repair reflects user intent. We should promise the
former only where accounted for and assess the latter with examples and users.

## Open findings and ownership

These entries belong to the Level L implementation and should be resolved in
the next diagnostic implementation increment. Their status is **open**; this
assessment documents them without silently treating documentation as a repair.

| ID | Finding / next action | Closure evidence |
|---|---|---|
| LL-D01 | Partly repaired: named lexical diagnostics and EOF metadata work. Tree declarations remain ineffective; reconcile frontend/backend admission and emission. | An authored feature changes the delivered behavior as documented, or generation rejects it with a capability explanation. |
| LL-D02 | Partly repaired: CR/CRLF/LF lines agree and syntax errors have carets. Semantic locations still lack consistent carets/filenames and related ranges. | Exact located diagnostics for LF, CRLF, CR, UTF-8 damage and related rule ranges. |
| LL-D03 | Token-ID/literal authoring traps and generic conflict advice need stronger admission/explanation. | Negative fixtures, explicit capability boundaries, and bounded conflict examples where found. |
| LL-D04 | Runtime syntax/lexical explanation lacks construct context and verified candidate filtering. | Actual-token, quote/opener and stack-checked expectation cases without misleading alternatives. |
| LL-D05 | Neither specification nor target pipeline resynchronizes. | Two independent errors plus retained later structure; clean-language invariance; lexical damage; bounded progress; no execution after recovery. |
| LL-D06 | Formatter counts scalars rather than display cells. | Agreed display policy and tab/combining/wide-character rendering examples. |

The next owner is the Level L implementation work under
[PLAN.md](../../../research/level-l/PLAN.md). Scope/design decisions for new
recovery declarations remain Adrian's; none of these items depends on closing
the experimental Pika/Squirrel questions first.
