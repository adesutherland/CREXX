# Mike's CREXX report review — 11 September 2026

The supplied examples establish four documentation/usage traps, rather than a
demonstrated runtime or compiler implementation defect. Two larger-program
claims need their original source, and the numeric-period observation remains
unreproduced. No language or runtime behavior was changed in this review.

## Revisions and evidence

Mike's email, `CREXX bug reports` (11 September), reports Linux build
`48ebc1f610a948a39972348379fd02fff4b156ad`. It contains seven findings and three
historical findings described as already fixed. It has no source attachment.
The related `PROLOG in/with CREXX` and 8 September `datatype..` emails were also
reviewed; they do not supply the missing Prolog source.

Independent validation used a freshly rebuilt macOS ARM64 Debug toolchain at
`beee17322956a86cbdb3fcbf8fc0db6d560fc02b`, in the `CREXX-hotfix` worktree.
The reported Linux revision was not rebuilt or qualified in this review.

[Retained evidence](mike-reports-2026-09-11-evidence.json) contains the runnable
source fixtures, command arguments, exit statuses, output, tool hashes and
issue mapping. Original verbose logs and scripts are in the local scratch
directory `/tmp/crexx-mike-reports.n746j3`; that temporary directory is not a
permanent evidence store. The checked-in JSON retains the relevant evidence.

## Disposition of every finding

| Report | GitHub issue | Validated result and action |
| --- | --- | --- |
| 1: imported nested call panics | [#685](https://github.com/adesutherland/CREXX/issues/685) | Caller omits the runtime library; panic resolves `libmod.a` before its body runs. Explicit loading, packaged autoload and explicit linking pass. Worked documentation added. Automatic source-dependency builds remain a design decision. |
| 2: filename changes compilation | [#686](https://github.com/adesutherland/CREXX/issues/686) | Not independently reproduced: original 2,900-line source is missing. Requested exact source/imports/commands. Filename-derived namespace identity is a diagnostic avenue, not an established cause. |
| 3: `rc` rejects string results | [#687](https://github.com/adesutherland/CREXX/issues/687) | `rc` is `.int`; ordinary `.int` variables show the same failures. Separate string result variables pass. Language-reference documentation added. |
| 4: array declaration does not clear | [#688](https://github.com/adesutherland/CREXX/issues/688) | AST contains `DEFINE`, binding exposed storage. Explicit `arraydrop` produces the expected second token set. Declaration/clearing documentation added. |
| 5: stale value across branches | [#689](https://github.com/adesutherland/CREXX/issues/689) | Not independently reproduced: email supplies an incomplete paraphrase. Requires original source, failing query and before/after split. No speculative optimizer change. |
| 6: sibling block bindings | [#690](https://github.com/adesutherland/CREXX/issues/690) | Current block-scope behavior and warnings reproduced. Declaring the result before the conditional works. Migration example added. |
| 7: numeric period | [#691](https://github.com/adesutherland/CREXX/issues/691) | Literal and substring/loop cases consistently return 0. Closed as not planned, with the unconfirmed observation retained for reopening if evidence arrives; no datatype patch. |

The three historical checks also pass with and without optimization:
`datatype("-2","N")` and `datatype("-2","W")` return 1;
`subword("a b c d",1,2)` returns `a b`; and `charout` returns 0, writes exactly
`hello world` to a fresh file and closes successfully. These are current
validation results, not an attribution of the original repair commits.

## Important reproduction details

For report 1, source-only imports show this matrix:

| Library body | Optimized | Unoptimized |
| --- | --- | --- |
| A returns a literal | succeeds | missing `libmod.a` |
| A calls zero-argument B | succeeds | missing `libmod.a` |
| A calls B with a string argument | missing `libmod.a` | missing `libmod.a` |

Optimized assembly for the first two cases contains no retained call to A.
Inlining hides the absent runtime input; argument passing is not the failing
operation. Explicit `crexx -l ./libmod.rxbin usemod.crexx` succeeds. In a
binary-only package directory, `crexx -i . usemod.crexx` emits the exact
`libmod` autoload hint and runs successfully with both optimization settings
under both `rxbvm` and `rxtvm`. Explicit `rxlink` succeeds under both VMs too.

A clean two-source `crexx --program combined usemod.crexx libmod.crexx`
build succeeds, an unchanged second build reports `SKIP: project current`,
and `rxvm combined.rxbin` prints the expected greeting. A preliminary attempt
in the mixed diagnostic scratch directory rejected changing project inputs and
published no image; that attempt is retained in the evidence, not presented as
a passing build or diagnosed as another defect.

For report 3, the first example fails at compile time when optimization exposes
the constant invalid conversion, but at runtime without optimization. The
second fails at runtime in both modes. Replacing `rc` with a normal explicit
`.int` reproduces those categories; replacing it with an inferred string
variable succeeds in both modes. Declaring `rc = .string` does not override
the system type.

For report 4, both original and corrected programs were compiled, assembled,
linked and run under both concrete VMs. Their outputs were checked exactly.
The corrected program also passes both driver optimization modes. For report
6, the corrected output and absence of `NOT_IN_SAME_SCOPE` were checked in
both modes.

## Design and follow-up boundaries

Automatically building/loading source imports would change dependency
selection, project membership and runtime loading policy; #685 records that
decision for Adrian. Generalizing `rc` beyond integers, making its ordinary
integer assignments illegal, changing bare declarations into runtime resets,
or changing Level B block scope would also require explicit language approval.
None is necessary to correct these examples under the existing contracts.

#686 and #689 need evidence before either an implementation plan or a design
proposal can be justified. The supplied smaller attempts reportedly do not
reproduce those failures. The review does not infer correctness of the missing
program from passing smaller cases.

Changes are documentation-only. The focused reproductions support the
classifications and examples; they are not full Debug, sanitizer, Linux or
release qualification. No full QA sweep or hosted workflow was requested or
represented as complete for these documentation edits.
