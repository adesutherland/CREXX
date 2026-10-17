# New Level C instruction review

The [C-first plan](../PLAN.md#c-first-delivery-milestone) owns scope, acceptance
and implementation decisions. Adrian directed instruction-by-instruction review
on 16 October 2026, leaving INTERPRET until later. This ledger follows the
existing Classic LC-I order. Native instruction closures do not establish new
compiler coverage. Shared services are reviewed when an instruction needs them;
their unresolved portions stay visible rather than being silently inherited.

## Delivery order

| Existing contract | New compiler review |
|---|---|
| LC-I-01 SAY | First review and CFC-SAY-DEF01 repair qualified locally; full host/TRACE/shared dependencies remain open. |
| LC-I-02 DROP | Next review. |
| LC-I-03 assignment | Not yet reviewed for full scope. |
| LC-I-04 NOP | Not yet reviewed for full scope. |
| LC-I-05 OPTIONS | Not yet reviewed for full scope. |
| LC-I-06 IF | Not yet reviewed for full scope. |
| LC-I-07 SELECT | Not yet reviewed for full scope. |
| LC-I-08 DO | Not yet reviewed for full scope; inspect context-specific logical error identity. |
| LC-I-09 LEAVE | Not yet reviewed for full scope. |
| LC-I-10 ITERATE | Not yet reviewed for full scope. |
| LC-I-11 ARG | Not yet reviewed for full scope. |
| LC-I-12 PROCEDURE | Not yet reviewed for full scope. |
| LC-I-13 CALL | Not yet reviewed for full scope. |
| LC-I-14 RETURN | Not yet reviewed for full scope. |
| LC-I-15 EXIT | Not yet reviewed for full scope. |
| LC-I-16 PULL | Not yet reviewed for full scope. |
| LC-I-17 PUSH | Not yet reviewed for full scope. |
| LC-I-18 QUEUE | Not yet reviewed for full scope. |
| LC-I-19 PARSE | Not yet reviewed for full scope. |
| LC-I-20 ADDRESS | Not yet reviewed for full scope. |
| LC-I-21 implicit command | Not yet reviewed for full scope. |
| LC-I-22 NUMERIC | Not yet reviewed for full scope. |
| LC-I-23 SIGNAL | Not yet reviewed for full scope. |
| LC-I-24 TRACE | Not yet reviewed for full scope. |
| LC-I-25 INTERPRET | Deferred by Adrian, including mechanism review; required for eventual complete C. |

The full BIF/LC-reference crosswalk remains CFC-AC01 work. Each instruction adds
its relevant rows; this table is not a declaration that other features work or
that unreviewed instructions have no implementation.

## SAY: contract and paths

[LC-AC-57 / LC-I-01](../../../../docs/planning/release-1/levelc-compatibility-worklist.md)
and LC-REF-068 require an optional expression evaluated once, followed by one
line through configured output, including an empty line for bare SAY. Shared
expression rules apply without an extra SAY-specific limit. LC-REF-028 and the
accepted LC-UNICODE model govern radix literals and fixed U+0000–U+00FF ordinals.

| Obligation | New path and evidence | Remaining boundary |
|---|---|---|
| Bare and expression forms | `levelc.rxl` creates `Say(value?)`; `llfrontend.statement` lowers the expression once, then one `crexx.c.say`. Existing frontend grammar/syntax tests plus `tests/say/forms-c.crexx`. | Full expression/BIF coverage belongs to shared semantics and CFC-AC02. |
| Evaluation/output order | Ordinary numeric/string expressions, IF/group and private-procedure call with inner output before outer output. `tests/say/forms-c.crexx` has independent expected bytes. | EXPOSE, shared-frame labels and general calls remain PROCEDURE/CALL dependencies. |
| Text and ordinals | `llfrontendparse` joins only an adjacent single X/B symbol to a C String token; `llfrontend.ordinalliteral` validates and uses B `b2x`/`x2c` at compilation. Existing `core.constant` preserves exact UTF-8 text. | No G radix-literal expansion. No general BIF admission is implied by compiler-side decoding. |
| Output route | `llrxas.classic` calls checked `llctarget.sayvalue`; its B implementation calls `RexxValue.asString(1)` and the existing VM SAY instruction. | New-image configured callback/context isolation and output-failure proof remain open. Existing native callback tests qualify the unchanged platform only. |
| Syntax and expression failure | Focused malformed literal/expression tests preserve existing output files. Runtime division aborts before the failing SAY or any later output, with authored location. | Classic numeric diagnostic identities for all source errors, caught conditions and full TRACE/clause observations remain open. |
| Source and RXAS | Complete literal/error RXAS and shape assertions in the SAY receipt; stable logical basename, full literal span, one output call and no runtime radix conversion. | Full source clause IDs and TRACE remain CFC-AC03 / LC-I-24; generated clause ID zero does not claim full trace support. |

### CFC-SAY-DEF01: radix suffix treated as a variable

The pre-repair command `say '410042'x` succeeded but printed `410042X`, in
both compiler IR modes. The token-layout adapter inserted abutment before X;
its value came from the variable pool. The expected result is A, NUL, B, LF.
This is a new compiler defect, not a missing optimisation.

The repair combines adjacent string-plus-single-symbol X/B tokens while keeping
the authored bytes. A space, comment or longer identifier prevents that merge.
Semantic lowering validates the digits and grouping and constructs the existing
Classic constant. No new generic Level L API/schema, AST kind, runtime helper
or emitter operation is needed. In particular, the emitted program does not
call X2C/B2X or read a variable named X/B to execute the literal.

The grouping expectation follows IBM's [Classic token reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=syntax-tokens):
hex groups have byte boundaries, binary groups have nibble boundaries, the
leading group can be short, and boundary blanks are invalid. cREXX's separately
approved ordinal model maps decoded byte values to Unicode scalars rather than
the reference host's EBCDIC page. Invalid grouping is diagnosed before emission,
even inside an unreachable arm. A fresh native `rxc-c -n` control rejects `'4 1'x` with RXC-LC-15.1,
agreeing with the independent expectation. Inspecting the common literal decoder
alone would miss that earlier native validation; no native defect is claimed.

### RXAS review and outstanding cost

The full literal-only baseline still constructs an argument/context wrapper,
a private pool, return packet and fatal-error handler. The pool and copied
argument/context slots are unused in that example. These costs belong to the
existing context/pool specialisation and slot/register cleanup owners in
[poc-coverage](poc-coverage.md#code-quality-and-remaining-work), not to the SAY
output contract. Record their removal criteria: prove no pool, argument or
numeric-context observer, preserve call/return/condition semantics, and assert
absence of the corresponding constructions in constant-only programs. This
review does not accept the baseline as optimal or close CFC-AC06.

The output helper itself performs one Classic string conversion followed by
one VM SAY. Its checked call has a typed argument window and cleanup. Inspect
that body as well as the caller: one helper call is not one unit of total work.
The radix repair removes the erroneous variable lookup/concatenation and their
runtime helper work; it does not hide decoding in a target helper. Source
records for literal evaluation and output describe different authored ranges;
they must not be deleted just to reduce metadata counts.

The [SAY validation summary](../PLAN.md#say-validation) records local results and limits.
SAY stays open for the explicit shared/host/TRACE obligations above. Review and
closure are separate states; the next instruction is DROP after SAY disposition.
