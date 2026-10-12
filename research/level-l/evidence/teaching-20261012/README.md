# Teaching examples and comment/migration review — 12 October 2026

This receipt qualifies the new examples and documentation against the existing
`13041b644` Level L runtime checkpoint. It does not change the parser/runtime,
add new comment syntax, implement a forest resolver, remove legacy RexxScript,
or close the full research/common-surface programme. The new source/test bytes
and actual native tools/artifacts are identified in `inputs.json` and each panel
receipt. Work remains on `levell`, unmerged for the agreed final review.

| Panel | Complete Level L build | Evidence |
|---|---|---|
| `qualified-opt/receipt.json` | Optimized product | 404 runtime cases + eight expected generator refusals; 562 commands. |
| `qualified-noopt/receipt.json` | Complete unoptimized standalone Level L and L2 | Same 404 runtime cases + eight expected refusals; 562 commands. |
| `guide-receipt.json` | Optimized product | Seven documented wrapper/toolchain commands; both VMs print exactly `RESULT\|11`. |

Each panel includes 176 calculator executions (22 inputs × source/RXAS ×
consumer opt/no-opt × both VMs) and 228 backend-lesson executions. Source-emitted
calculator modules are also compiled in the selected consumer optimizer mode.
Together: **808 runtime checks, 16 expected build refusals**, plus the two
documented calculator executions. All builds are free of warnings. The eight
refusals per panel are left-recursive list LL/packrat, palindrome LL/LR,
ambiguous arithmetic LL/packrat/LR, and common-prefix LL. Each refusal is checked
for its actual admission category and absence of a generated artifact body.

The lessons exercise LL(1), canonical LR(1), ordinary packrat and Earley;
the calculator exercises Pratt, with DFA lexing in all cases. The existing
Pratt-in-LR region/recovery panel remains retained under the preceding
continuation rather than being gratuitously rerun. The new language examples
cover successful versus speculative alternatives, whole-input failures,
left-recursion admission, a unique general-CFG palindrome, an ambiguous forest
count capped at two and no silent tree selection. Located errors and a custom
command message are checked in their applicable backends. A lexical error is
not reported as a grammar ambiguity.

Two regular line-comment forms (`#`, `//`) are tested on all four grammar
engines, including CRLF/EOF, doubled-quote shielding, markers inside strings,
Unicode comments, original token spans and a later lexical failure. The
calculator's different `(* ... *)` nested pair is tested through both emission
formats, with nesting/Unicode/newlines. These are existing capabilities; coherent
multiple block/line forms, configurable shielding and newline retention are
the separate proposal in SURFACE.md. The current one-pair preparation stage
does not compose coherently with block markers inside line-comment DFA rules.
The guide also flags the current frontend's incomplete rejection of duplicate
comment declarations; this remains an open common-model admission obligation.

The calculator checks precedence, left association, grouping, prefix nesting,
decimal conversion/arithmetic, exact decimal `0.1+0.2`, source locations for
bad operators/EOF/lexical damage/unclosed comments, and direct/computed division
by zero. Invalid input emits no `RESULT`. `7/0 +` produces the parse error
before evaluation, so it does not report a division that was never executed.
Decimal semantics belong to this example; they do not change RexxScript's
unsuffixed fractional literal policy.

Intermediate tutorial development exposed two fixture mistakes: the consumer
initially omitted the retained `arithmetic` start wrapper, and its unmatched-byte
assertion named an incorrect lexical code. Both were corrected before the final
panels; no core product repair was needed. Earlier development artifacts remain
outside Git in `research/level-l/build/teaching-development-20261012/`. Only the
final qualified panels are retained here. A preliminary wrapper invocation
omitted the output-directory creation; the actual guide includes it and the
seven-command replay passes.

All 64 relevant core-source/artifact identities checked against the preceding
continuation manifest match. Its core/bootstrap/Unicode/recovery/lifecycle and
consumer evidence therefore remains applicable; no new product rebuild,
aggregate CTest expansion or broad Release/sanitizer/hosted/platform gate was
necessary for this example/documentation change. Local links and fenced blocks
in the edited guides/plan were checked. Source/documentation whitespace checks
pass; raw CR/CRLF inputs, intentionally trailing input spaces and blank diagnostic
excerpt lines are preserved unchanged. All 1,615 staged files were compared
byte-for-byte with the working inputs before commit. This is native macOS ARM64 functional
evidence, not install/package, hosted CI or sanitizer qualification.

The first commit includes the implementation, examples, plan/guide updates and
these receipts. Expected delivery is the existing `origin/levell` safety branch;
actual commit and remote identity are reported in the conversation. No status-only
CI follow-up is planned. EX-AC02–04 are verified for the stated increment;
EX-AC01's concrete proposal is ready for review and EX-STEP05 remains gated on
the maintainer's comment-language decision. All wider LANG obligations remain
open.
