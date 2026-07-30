# crexx-rag integration ledger evidence

Status: closed; all CRI-01 through CRI-14 items have accepted dispositions

This bundle retains CREXX-side evidence for `CRI-01` through `CRI-14`. The
read-only `/Users/adrian/CLionProjects/crexx-rag` checkout is evidence only and
must remain unchanged.

## Initial identities

| Surface | Identity |
| --- | --- |
| CREXX source | `develop` at `d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf` |
| crexx-rag evidence | `main` at `97cd87e91344d6ac1773a054bd38df23eb128ed2` |
| Installed CREXX | `crexx-1.0.0-beta.3+local.g057592681c0c`, build `20260728` |
| Host | Darwin 25.5.0, Apple arm64, 10 logical CPUs |
| CMake | 4.3.2 |
| C/C++ compiler | Apple clang 21.0.0 (`clang-2100.1.1.101`) |

Installed executable SHA-256:

```text
72ad5a744e941123cc8ea8c2b8e410531b0bc780c5937a9c877e003c24d09c9d  crexx
a7ad25ac18350937fddaee91402c053f092b5affda0fa42068389b663d3c1d34  rxc
7192de0e5f1b273546d875138c40e0f1591b285bbc60f61d9b5a73d3ebdd86fc  rxvm
496c381ef114a83d6940b5fc2eb38b89a9aadb8fe70b603fe2f280dd317bafc0  rxbvm
083ce3a9b5e0e1e16af870b6ad73bad289b8293e0d07fb7cf83aecd44da6ad6b  rxvme
f91c978315cc2780436b838242b7472ba3d49420851752dccc0fba18b3e4d967  rxbvme
```

The exact initial terminal capture is retained outside both repositories at
`/tmp/crexx-integration-ledger.mZ5jyp/`. Later evidence added here must keep
raw commands, stdout/stderr, exit codes, version/build fingerprints, counts,
timings, and interpretation boundaries distinct.

## Clean Debug baseline

Dedicated build directory:
`/tmp/crexx-integration-ledger.mZ5jyp/baseline-debug`.

The source baseline reports
`crexx-1.0.0-beta.3+local.gd78c6fcfa81e` built `20260729`. Its driver SHA-256
is `fc4a24323300f01320462be22bb9a8f8a24387043647823190674a22ccf3d617`;
the exact six executable hashes, generated version/build files, CMake cache
identity, configure log, and clean build log are retained under the temporary
evidence root.

Results:

- focused import, PARSE, interface-import, and RXPA baseline: 72/72 passed,
  zero failed or skipped, 8.10 seconds;
- full Debug CTest: 1925/1925 passed, zero failed or skipped, 205.06 seconds,
  using `--parallel 30 --output-on-failure`;
- CRI-01, CRI-04, CRI-05, and CRI-06 exact source-baseline reproducers all
  fail as reported in both optimized and non-optimized compilation;
- CRI-06's minimized malformed RXPA plugin and importer are fingerprinted in
  the raw temporary evidence and produce
  `#INTERNAL_ERROR_PARSING_IMPORT_AST` deterministically.

Raw paths:

- `baseline-configure.log`, `baseline-build.log`, `baseline-fingerprint.txt`,
  `baseline-ctest-list.txt`, `baseline-focused-ctest.log`, and
  `baseline-full-ctest.log` under the temporary evidence root;
- `source-baseline-repros/cri01` through `cri06` (only concrete defects 01,
  04, 05, and 06 are currently populated).

Immediately before the first production edit, CREXX remained at the recorded
HEAD with only the five pre-existing lifecycle artifacts plus this programme's
two new evidence paths. The read-only crexx-rag branch, HEAD, and complete
short-status listing exactly matched the initial audit. Its tracked worktree
diff SHA-256 was
`97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9`,
its empty index diff SHA-256 was
`30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d`,
and its porcelain-v2 status SHA-256 was
`badeaa8b316bb0e41c75cd2086dae5ad417257fcd0273b360e562ef2f87f9360`.

## CRI-01 — imported Level B record returned through Level G

Disposition: **fixed**.

The unchanged source baseline compiles and assembles the provider, then rejects
the imported Level G consumer with `#TYPE_MISMATCH` in both optimized and
non-optimized modes. Compiler debug evidence showed that the same exposed
procedure was discovered twice with return-type text
`.repro_record..reprorecord` from qualified source metadata and `.reprorecord`
from the binary/import stub path. Both names resolved to the same loaded class
symbol, but duplicate-import consistency compared the raw strings first.

`compiler/rxcpfunc.c` now normalizes source-qualified contract type names before
class-symbol lookup and uses resolved nominal class identity when duplicate
object metadata uses different spellings. The equivalence helper preserves
strict scalar identity, reference/value form, and complete array shape. A
negative regression returning `wrongrecord` where `reprorecord` is required
still fails with `#TYPE_MISMATCH`. There is no syntax, runtime, public ABI, or
RXAS/RXBIN serialization change.

Maintained coverage is registered as `imported_record_return_contract`. Its
matrix includes source and binary provider imports; Level B and Level G
facades; direct imported-procedure assignment; method return, invocation, and
assignment; optimized and non-optimized compilation; both `rxvm` and `rxbvm`;
and the wrong-record negative case.

Candidate identity:

- build: `crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty`, Debug, Apple arm64,
  build date `20260729`;
- `crexx` SHA-256:
  `48738276fecd6b92fa6dbd42333950776247ae4334f0c74104033da81c2958c3`;
- `rxc` SHA-256:
  `89d57c404e47d6ae1d2025484db359a7c16d5fe288a1af3aafcfe7055a17fe2b`;
- `rxvm` SHA-256:
  `c4355743570253dc8bfe950ba817385e87f484d452b408ecdb4810c54f30ddfe`;
- `rxbvm` SHA-256:
  `b0bf3ced46d3573c2bf6268b3c9d086ca2e48412d528fd11c347f93bff191fbe`.

Results:

- exact original Gate-1A reproducer: provider and consumer compile/assemble in
  optimized and non-optimized modes;
- maintained regression: 1/1 passed;
- broader import/interface slice: 31/31 passed;
- clean full candidate build: passed;
- complete Debug CTest: 1926/1926 passed, zero failed or skipped, 190.29
  seconds with `--parallel 30 --output-on-failure`;
- focused ASan build and regression: passed, regression 1/1 in 23.17 seconds;
- Apple ASan does not support leak detection. The first runner invocation with
  `detect_leaks=1` aborted at a sanitized build-time tool with the explicit
  platform diagnostic. The rerun kept ASan enabled and set
  `detect_leaks=0`; both commands and raw outputs are retained;
- `git diff --check`: passed.

Raw paths under `/tmp/crexx-integration-ledger.mZ5jyp/`:

- `source-baseline-repros/cri01/source-baseline.log`;
- `cri01-contract-names.log`;
- `cri01-candidate-test.log`;
- `cri01-original-candidate/candidate.log`;
- `cri01-candidate-import-slice.log`;
- `cri01-candidate-full-build.log`;
- `cri01-candidate-full-ctest.log`;
- `cri01-candidate-fingerprint.txt`;
- `cri01-candidate-asan-configure.log`;
- `cri01-asan-logs/20260729-214125-build/build.log` (retained unsupported-LSan
  attempt);
- `cri01-asan-logs/20260729-214138-build/build.log`;
- `cri01-asan-logs/20260729-214828-ctest/ctest.log` (retained missing focused
  prerequisite attempt);
- `cri01-asan-logs/20260729-214846-build/build.log`;
- `cri01-asan-logs/20260729-214906-ctest/ctest.log`.

Remaining risk: Windows and non-Apple compiler validation are deferred to the
programme's final cross-platform/CI gate. The current host proves both VM
engines. A scratch install/package identity is deliberately separate and
remains pending for CRI-07 and final downstream proof.

## CRI-04 — terminal `do forever` and missing return

Disposition: **fixed**.

The unchanged source baseline reports `#RETVAL_MISSING` in optimized and
non-optimized compilation. Its statement after the loop is, correctly, still
inside the named procedure under the documented callable-boundary rule, but it
is unreachable. The compiler's early structure pass appended a bare return by
inspecting the lexical final statement rather than whole-body reachability. The
later typed flow builder compounded that model by adding a possible normal-exit
edge to every loop, including unconditional `do forever`.

Callable structure now computes a bounded statement-flow summary before adding
an implicit return. An unconditional forever loop has no fall-through unless a
reachable `leave` exits that loop. Conditional exits count; a `leave` after
`return` or `iterate` is unreachable; an unqualified inner-loop `leave` belongs
to the inner loop; simple `do` groups inherit their body's fall-through; and
conditional/bounded loops remain conservative. The typed flow graph now omits
only the unconditional forever loop's false normal-exit edge. Explicit `leave`
still creates the real edge after the loop.

Maintained coverage is registered as `do_forever_return_contract`. It checks
the retained unreachable-tail form, a returning terminal loop, a genuinely
nonterminating typed routine, unreachable `leave`, reachable and conditional
exits, nested-loop ownership, explicit return after a loop exit, and ordinary
fall-through. Both compiler modes are covered; the terminating positive matrix
runs on `rxvm` and `rxbvm`.

Results:

- exact retained source: compile/assemble passed optimized and non-optimized;
- maintained regression: 1/1 passed in 5.66 seconds;
- broader loop/return/control-flow slice: 41/41 passed;
- focused ASan regression: 1/1 passed in 8.45 seconds with
  `detect_leaks=0` on Apple ASan;
- clean complete candidate build: passed;
- complete Debug CTest: 1927/1927 passed, zero failed or skipped, 194.72
  seconds with `--parallel 30 --output-on-failure`;
- `git diff --check`: passed.

Post-CRI-04 candidate identity remains
`crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty`, Debug, Apple arm64, build date
`20260729`. The `rxc` SHA-256 is
`5180a78d0a979b5128be356e72d63bf91599c0b2d5746bfe892a6c69dabe8a56`;
both current VM hashes and all changed-source/test hashes are in the raw
fingerprint.

Raw paths under `/tmp/crexx-integration-ledger.mZ5jyp/`:

- `source-baseline-repros/cri04/source-baseline.log`;
- `cri04-debug-ast.log`;
- `cri04-exact-candidate.log`;
- `cri04-focused-build.log` and `cri04-focused-ctest.log`;
- `cri04-control-flow-slice.log`;
- `cri04-asan-logs/20260729-220208-build/build.log`;
- `cri04-asan-logs/20260729-220216-ctest/ctest.log`;
- `cri04-candidate-full-build.log`;
- `cri04-candidate-full-ctest.log`;
- `cri04-candidate-fingerprint.txt`.

The two accidentally overlapping early focused CTest invocations and their
programme-owned child processes were terminated after they collided in the
same dedicated work directory. The corrected registered test has a 120-second
timeout and was subsequently run serially through every accepted gate above.
No user process or user build tree was touched.

## CRI-05 — Level G PARSE and certified Level B lowering

Disposition: **fixed**.

The unchanged source baseline lowers an ordinary Level G `PARSE VAR` to the
certified exit's `assembler parseplan` fragment, then reports
`#ASSEMBLER_ONLY_LEVELB` in both compiler modes. Instrumented scratch evidence
showed two validation moments: the temporary replacement fragment and the
grafted caller tree. On the attached-object invocation path, the bridge passed
no registered `ExitEntry` into response handling, so certified identity was
lost and the fragment inherited Level G.

The bridge now resolves an attached exit object's class back to its registered
entry. Replacement fragments are treated as compiler-owned Level B only when
that entry is in the hard-coded certified allowlist. Their graft wrapper carries
an internal certified-fragment semantic context so later validation permits the
generated assembler node. The generic graft path, uncertified exits, and
authored Level G `ASSEMBLER` receive no exception.

Maintained coverage is registered as `parse_levelg_contract`. It checks Level G
and Level B forms for whitespace-separated fields, literal delimiters,
multiple and empty fields, a missing delimiter, positional templates, an empty
source, Unicode, and repeated source/target aliasing. Both optimization modes
compile and retain the frozen `parseplan` lowering, and both `rxvm` and `rxbvm`
execute each positive. A negative authored Level G `assembler` case must fail
with `#ASSEMBLER_ONLY_LEVELB`.

Results:

- exact Gate-1A reproducer: compile, assemble, and execute passed in optimized
  and non-optimized modes on both VMs; output preserves the intentional empty
  field;
- maintained regression: 1/1 passed in 4.56 seconds;
- broader PARSE/certified-exit slice: 29/29 passed in 141.31 seconds, including
  its required linked-runtime fixture build;
- focused ASan regression: 1/1 passed in 7.57 seconds with
  `ASAN_OPTIONS=detect_leaks=0`, the supported Apple-host setting;
- fresh dedicated configure and complete build: passed;
- initial clean full suite: 1927/1928 passed and correctly failed the optimized
  `address_expose` golden because three compiler-private branch labels moved by
  one ordinal after attached certified identity was preserved;
- the generated output differed only in those three label definitions/uses.
  The optimized golden was updated, the focused ADDRESS check passed 2/2, and
  the complete clean-tree Debug suite then passed 1928/1928 with zero failures
  or skips in 190.82 seconds using `--parallel 30 --output-on-failure`;
- `git diff --check`: passed.

Candidate identity is
`crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty`, Debug, Apple arm64. SHA-256:

```text
f33003508dd33fc89830541e5e1881a9f91d47df97ab3eb056c4b71aebc00cb6  crexx
2fe0732ecd1baa5df24483d9f1eedd3415c281159ab11297c018177eae603c1b  rxc
8291b54072bd63091d73b2e1fbda3c504c7ea8a4f4ead6e797d9f2d3327c92fb  rxvm
b3d37ed02a64d71c2556d3d702f9f740dbe7521ed0b20a15d131d22fedc27b1e  rxbvm
```

Raw paths under `/tmp/crexx-integration-ledger.mZ5jyp/`:

- `source-baseline-repros/cri05/source-baseline.log`;
- `cri05-debug-fragment.log`, `cri05-debug-fragment-2.log`, and
  `cri05-debug-fragment-3.log`;
- `cri05-exact-candidate-compile.log` and `cri05-exact-runtime.log`;
- `cri05-contract-ctest.log` and `cri05-broader-parse-exit-ctest.log`;
- `cri05-asan-logs/20260729-222616-build/build.log` and
  `cri05-asan-logs/20260729-222844-ctest/ctest.log`;
- `cri05-clean-configure.log`, `cri05-clean-build.log`,
  `cri05-clean-full-ctest.log`, and
  `cri05-clean-full-ctest-after-golden.log`;
- `cri05-address-expose-rerun.log` and
  `cri05-address-expose-after-golden.log`;
- `cri05-candidate-fingerprint.txt` and
  `crexx-rag-post-cri05-audit.txt`.

The post-CRI-05 read-only audit retains the same crexx-rag branch, HEAD,
tracked-diff SHA-256, empty index, and a byte-identical 72-line short-status
listing. No read-only repository file was changed. No language syntax, public
ABI, serialized RXAS/RXBIN contract, or runtime value semantics changed.

## CRI-06 — malformed RXPA signature diagnostics

Disposition: **fixed**.

The minimized dynamic plugin registers `sdk_bad.addints` with argument metadata
`.int,.int`. The unchanged source baseline rejects the importing call at line
5, column 5 as `#INTERNAL_ERROR_PARSING_IMPORT_AST` in both modes. The RXPA
metadata path converted argument text into a synthetic Level B procedure and
classified every failed parse as an internal compiler failure. The conversion
also had two adjacent gaps: a semicolon could become an extra statement in an
otherwise parseable stub, and a trailing comma disappeared without error.

RXPA import validation now rejects empty components and statement separators
outside quoted defaults before building an AST. If the synthetic declaration
still fails, a return-only probe distinguishes a bad return from bad argument
metadata. The imported-function record carries a stable code plus field/reason,
and the normal consumer diagnostic path emits five structured parameters:
`name`, `import_file`, `field`, `declaration`, and `detail`. The source location
remains the importing cREXX call because a compiled native plugin does not carry
a source location for its C macro argument.

The exact original now reports, in both modes:

```text
Error in rxpa_bad_signature.crexx @ 5:5 -
#RXPA_IMPORT_SIGNATURE_INVALID name="addints"
import_file="rx_sdk_bad.rxplugin" field="arguments"
declaration=".int,.int" detail="invalid Level B declaration"
```

Maintained `rxpa_signature_diagnostics` coverage builds one native plugin with
valid zero-, one-, and multi-argument declarations plus six malformed cases:
unnamed type lists, a semicolon separator, malformed argument type syntax,
malformed return type syntax, an empty middle component, and a trailing empty
component. Positives compile/assemble in both modes and execute on `rxvm` and
`rxbvm`; negatives assert the code, plugin, field, consumer location, and
absence of `INTERNAL_ERROR`.

Results:

- exact original and six-case raw matrix: expected structured failures in both
  modes; localized en_GB rendering also verified;
- maintained regression: 1/1 passed in 7.86 seconds;
- RXPA plus diagnostic-catalog slice: 46/46 passed in 172.63 seconds, including
  the required 164.09-second linked-runtime fixture rebuild;
- focused ASan regression: 1/1 passed in 12.01 seconds with
  `ASAN_OPTIONS=detect_leaks=0`;
- fresh dedicated configure and complete build: passed;
- complete clean-tree Debug CTest: 1929/1929 passed, zero failed or skipped, in
  222.45 seconds with `--parallel 30 --output-on-failure`;
- `git diff --check`: passed.

Candidate identity is
`crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty`, Debug, Apple arm64. SHA-256:

```text
1926bc376e1d3e49a840d63fd8ad7e37af33083f4eda9a4bd348aeb0730e50de  crexx
e91057ddfb1b8cdc1385b31d9fefef5266e0bfa1a4a20ac015c64cf26e817e50  rxc
80ab1e0a797da6ba94450988e4dd646538cf768883ebb32781acb496ae311d40  rxvm
b61eeb0e648888d500c01d19e6a09766aa2d0df9ef4040cf0ef6210c482f330d  rxbvm
f48fc8ecc1d4d1b966f3cb79a4366bca232581d8f416899bc267d5362e822cb9  rx_rxpa_bad_signatures.rxplugin
```

Raw paths under `/tmp/crexx-integration-ledger.mZ5jyp/`:

- `source-baseline-repros/cri06/source-baseline.log`;
- `cri06-red-build.log` and `cri06-red-ctest.log`;
- `cri06-candidate-build.log`, `cri06-candidate-build-2.log`,
  `cri06-candidate-ctest.log`, and `cri06-candidate-ctest-2.log`;
- `cri06-candidate-diagnostics.log`;
- `cri06-rxpa-diagnostic-slice.log`;
- `cri06-asan-logs/20260729-225733-build/build.log` and
  `cri06-asan-logs/20260729-225948-ctest/ctest.log`;
- `cri06-clean-configure.log`, `cri06-clean-build.log`, and
  `cri06-clean-full-ctest.log`;
- `cri06-candidate-fingerprint.txt`.

The documented RXPA `ADDPROC` declaration syntax is unchanged. No C callback
signature, macro expansion, binary ABI, runtime call path, language syntax, or
serialized format changed. The new diagnostic is cataloged in default English,
German, and Dutch; en_US inherits the default English template.

## CRI-02 — optimized `.binary` by-value hot-loop regression

Disposition: **fixed**. Stable performance ID: `PERF2-07-B01`. Adrian accepted
the frozen V1 candidate on 2026-07-30. The complete
frozen acceptance rule, V0/V1/V2/V3 comparison, raw paths, exact medians,
consequences, and continuation prompt are in
[`performance/CRI02-BINARY-BYVALUE-WORKLIST.md`](../../CRI02-BINARY-BYVALUE-WORKLIST.md).

Fresh baseline profiling proves the optimized compiler inlined the helper but
materialized 614,400 full 12,288-byte binary copies: 7,549,747,200 logical
bytes plus 614,400 `endlife` operations. Non-optimized execution retained the
call boundary and copied no payload. The exact checksums matched on both VMs.
An isolated machine control and compiler PoC both reached the exposed/direct
ceiling, so the compiler's existing read-only inline proof was selected over a
post-RXAS peephole or runtime copy-on-write architecture.

The production candidate first fixes typed binary-memory write-use tracking,
closing a minimized writable-formal isolation leak in the unoptimized path.
It then permits only validated read-only, exact, non-escaping `.binary`
formals to share a direct caller-local register and prevents the generated
formal from ending caller storage. Focused correctness passed 6/6 registered
tests across both compiler modes and VMs before the state froze.

The clean ordinary Release candidate reports
`crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty`; `rxc` SHA-256 is
`8c6aa738f1841d4e0f565903e469a7639e9b2d6dbe9c512e379fda15e641a498`.
Its optimized exact-probe RXAS/RXBIN hashes are respectively
`c59f5d12fda891acc3a5c8531b1e72a7eac6d786ed93d5dc8603a3d197d420bf`
and `9b8d132f2ca1cefbf0fd23b355053848d92026b550c3741924cb6ef858ba11fa`.
The RXBIN remains 10,365 bytes, the same size as baseline.

The mandatory first verdict used one warmup and twelve balanced/interleaved
rounds over baseline/candidate, opt/noopt, and `rxvm`/`rxbvm`: all 104 child
executions passed with checksum `944025600`. Optimized by-value medians fell
from 72,564.0 to 5,284.5 us on `rxvm` and from 77,261.5 to 5,677.5 us on
`rxbvm`, improving 92.72%/92.65% and removing 99.61%/99.52% of the inversion
gap. The predeclared adjacent-control guard initially stopped the candidate:
`rxvm` exposed and direct medians were +14.54%/+5.17%, while `rxbvm` stayed
within 1%. No sample was removed.

Adrian then authorized only the bounded `rxvm` order/per-variant and peak-RSS
adjudication. It retained the exact kernels, dimension 3072, 200 iterations,
work 614400, by-value boundary, and checksum. After the governed append rules,
34 balanced pairs per timing cell show that the apparent slowdown follows
phase position: both controls are slow only after the baseline's approximately
73 ms defensive-copy loop. With both controls before that loop, medians are
+2.45% exposed and -1.55% direct; with direct before it and exposed after it,
they are -1.31% and +10.70%; in isolated processes they are -2.93% and -1.15%.
The isolated RXAS files are byte-identical baseline/candidate and disassembly
differs only in module/description path metadata. All 350 timing executions
(10 warmup and 340 recorded) passed; the short neutral series remain honestly
labelled noisy/inconclusive at the governed cap.

Peak-RSS medians changed from 17,678,336 to 17,580,032 bytes on `rxvm`
(-0.56%) and from 17,612,800 to 17,547,264 bytes on `rxbvm` (-0.37%). All 12
RSS executions passed on AC power with low-power mode off and no thermal or
performance warning. The prior guard hit is therefore classified as a
fixed-order warm-state measurement artefact, not an exposed/direct compiler
regression. The measurement countermeasure is separate balanced processes or
full phase-order rotation when an earlier phase changes by an order of
magnitude; no production countermeasure is warranted.

Raw evidence under `/tmp/crexx-integration-ledger.mZ5jyp/` includes
`cri02-baseline-*`, `cri02-v1-*`, `cri02-v2-*`, the focused build/CTest logs,
`cri02-release-candidate/`, candidate RXAS/RXBIN and hashes,
`cri02-first-verdict-manifest.txt`, the complete `cri02-first-verdict/` sample/
output bundle, host pre/post state, and acceptance calculations.
The adjudication adds `cri02-adjudication/` and
`cri02-adjudication-control-src/`, including exact commands, manifests, three
timing blocks, merged summaries, paired analysis, RSS, host state, hashes, and
temporary Level B sources. Its 58-entry checksum manifest verifies and hashes
to `9c7307fe825228ef466e2c390dd2dc93354cc8e5b35c932435a246c851f28c81`.
The frozen production-source hashes still match and `git diff --check` passes.

The proportional B4 closeout initially passed 1932/1934 complete Debug tests.
Both failures were expected compiler goldens for the same
`select_dispatch_strings.binary_short` I6 callable summary. After normalizing
its formal flags from `464` to `400`, each generated file was byte-for-byte
identical to its golden. The cleared `64` bit is
`RXCP_INLINE_FORMAL_ESCAPES`: V1 intentionally removes it only when body
analysis proves the by-value `.binary` formal read-only and exact. This changes
private optimization evidence, not the metadata schema, public RXAS/RXBIN
format, VM instruction set, language contract, or ABI.

The maintained contract now checks mask `400` for read-only binary formals and
mask `416` for typed-write formals. A new imported dependency is assembled,
disassembled, reassembled, and consumed in both compiler modes. Optimized code
must bind the imported reader directly, isolate the writer with a copy, and
never end caller storage; non-optimized code must retain both call boundaries.
Both artifacts execute on both VMs and prove that the writer cannot mutate the
caller. Only after this matrix passed were the two exact flag fields updated.

Final B4 results are focused normal 8/8, affected Apple ASan 3/3 with the
platform-supported `detect_leaks=0`, and complete Debug CTest 1934/1934 with
zero failures or skips in 213.67 seconds. `git diff --check` passes and the four
production-source hashes are unchanged from the frozen candidate. Raw closeout
logs, exact commands, source/RXAS/RXBIN round-trip artifacts, diffs,
fingerprints, and audits are under `/tmp/crexx-integration-ledger.mZ5jyp/` with
the `cri02-b4-*` prefix.

The item-boundary read-only crexx-rag audit preserves branch `main`, HEAD
`97cd87e91344d6ac1773a054bd38df23eb128ed2`, tracked diff SHA-256
`97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9`,
empty index diff SHA-256
`30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d`,
and no-branch porcelain-v2 status SHA-256
`02a6e4216ff47be3ce7252be2ccb39157bc45fda316c254f7198648109df14f1`.

## CRI-03 — historical hosted `rxhttp` timeout classification

Disposition: **no-CREXX-change**.

The CREXX-side audit separates HTTP helper coverage from the socket-boundary
coverage supplied by the completed Gate-1A experiment:

- `lib/rxfnsb/tests_functional/ts_rxhttp.crexx` covers request construction,
  custom headers, UTF-8 byte `Content-Length`, content-length and chunked body
  extraction, blank and Unicode bodies, non-2xx preservation, a truncated
  body, and an invalid status line;
- the unchanged read-only Gate-1A fixture covers deterministic HTTP success,
  malformed JSON after a valid HTTP response, a server that exceeds a 50 ms
  receive timeout, a refused loopback connection, and a structured provider
  error;
- the historical evidence records that generation had completed before the old
  adapter repeatedly reparsed the full 3,072-value embedding response. The
  parse-once adapter with the effective bounded-dimension field later completed
  generation in 1,446,777 us and an eight-value embedding in 364,282 us.

No hosted endpoint was contacted during this CREXX replay. The fixture bound
only `127.0.0.1`, used test key text, and reported:

```text
P1A_LLM_DIAG case=malformed code=-102 category=malformed_response message=provider response is not valid JSON: unterminated array
P1A_LLM_DIAG case=timeout code=-5 category=timeout message=socket receive timed out
P1A_LLM_OK generation=1 embedding=1 packed_f32=1 dimension_request=1 malformed=1 timeout=1 connection=1 structured_errors=1 hosted=0
```

The unchanged Gate-1A sources were compiled outside the read-only checkout with
the post-CRI-06 clean candidate. The optimized build used the exact commands
encoded by the read-only `cmake/P1AProviderBoundary.cmake`, with these paths:

```text
CPRAG_RXC=/tmp/crexx-integration-ledger.mZ5jyp/cri06-clean-debug/bin/rxc
CPRAG_RXAS=/tmp/crexx-integration-ledger.mZ5jyp/cri06-clean-debug/bin/rxas
CPRAG_CREXX_BIN_DIR=/tmp/crexx-integration-ledger.mZ5jyp/cri06-clean-debug/bin
CPRAG_JSON_MODULE=/Users/adrian/CLionProjects/crexx-rag/incubator/p1a/json_document/json_document.crexx
CPRAG_MODULE=/Users/adrian/CLionProjects/crexx-rag/incubator/p1a/provider_boundary/provider_boundary.crexx
CPRAG_SOURCE=/Users/adrian/CLionProjects/crexx-rag/incubator/p1a/provider_boundary/provider_boundary_test.crexx
CPRAG_LOOPBACK=/tmp/crexx-integration-ledger.mZ5jyp/cri03-replay/p1a_provider_loopback
CPRAG_WORK_DIR=/tmp/crexx-integration-ledger.mZ5jyp/cri03-replay/work-rxvme
```

The non-optimized build repeated each of the three source compile/assemble
pairs with `rxc -n ... --import-rxas` and `rxas -n ...`, using only the
temporary work directory and candidate `bin` as import paths. Each matrix then
started the local fixture with five requests and ran:

```text
rxvm  -l <temporary-work>;<candidate-bin> <test> provider_boundary json_document library -a <loopback-port>
rxbvm -l <temporary-work>;<candidate-bin> <test> provider_boundary json_document library -a <loopback-port>
```

Results:

- CREXX `ts_rxhttp`: optimized and non-optimized passed on `rxvm`; the same two
  bytecode artifacts passed on `rxbvm`;
- Gate-1A provider loopback: optimized and non-optimized passed on both `rxvm`
  and `rxbvm`, four of four, with fixture exit status zero in every run;
- malformed response, timeout, and refused connection all produced their
  expected structured categories; no independent transport failure occurred;
- `rxhttp.crexx`, `ts_rxhttp.crexx`, and all runtime transport semantics remain
  unchanged.

An initial attempt deliberately retained in `rxvme.log` used the installed-
shape downstream command with the build-tree `rxvme` and requested the legacy
external module `rx_socket`. It failed before the test with
`ERROR reading module file rx_socket` because the clean build tree exposes the
current core socket environment through `rxvm`/`rxbvm`, not an installed legacy
plugin artifact. Replaying the same bytecode on the two current core VM
variants, without that obsolete explicit module argument, is the applicable
candidate proof. This is not an `rxhttp` result and was not counted as one.

Relevant SHA-256:

```text
4af648d22994c75a7e6182769479de28890d519c515d5ef3429e4a7b3a292693  rxhttp.crexx
a973bb837a679ec32192885064181bad346344057e4815d7477d76c0c885d9c5  ts_rxhttp.crexx
a5266e25d752fe5ad438c2992d4aa6f3a3769229f7f1de362d2fc49ae4d73f17  provider_boundary.crexx
eb5090f456675bb99f8224fdf4bb03674554c5c740df891825e5e99f6e75ad9f  provider_boundary_test.crexx
c662b1de10de63bc2395395cf3906d3d6fe7034d80c2bd18274ed23aa99dc92d  p1a_provider_loopback.cpp
37e228966502920f55a562bd366dc2245231dc7ed8a1644b1dd479b52d367449  temporary p1a_provider_loopback executable
92305d7841f5b18eccc4534db7dd6e2f78451b7df9b69d1390700ff1df85eb19  optimized provider-boundary-test.rxbin
bae61ed7a1a7da69ee9232073cf700e0e1a6bbbe912c022cd67caa8a7e631311  non-optimized provider-boundary-test.rxbin
```

Raw paths under `/tmp/crexx-integration-ledger.mZ5jyp/cri03-replay/`:

- `rxvme.log` and `work-rxvme/commands-and-output.txt` (retained non-result);
- `rxvm-opt.out`, `rxvm-opt.err`, `rxbvm-opt.out`, and `rxbvm-opt.err`;
- `noopt-build.log`, `rxvm-noopt.out`, `rxvm-noopt.err`,
  `rxbvm-noopt.out`, and `rxbvm-noopt.err`;
- the four `loopback-*.out`/`.err` pairs inside the optimized and non-optimized
  work directories;
- `ts-rxhttp-ctest.log`, `ts-rxhttp-rxbvm-noopt.log`, and
  `ts-rxhttp-rxbvm-opt.log`;
- `hashes.txt` and the fingerprinted temporary artifacts.

The old hosted symptom is therefore not an `rxhttp` defect. It was downstream
work after a completed transport call plus an application/provider request-
shape error. Changing timeout semantics would be unjustified and would erase
the useful deterministic `-5` contract. Hosted reliability, retry,
cancellation, privacy routing, streaming, and SLA evidence remain explicitly
outside this disposition.

## CRI-07 — installed RXPA SDK and external consumer

Disposition: **documented/package-closed**.

The baseline scratch prefix `/tmp/crexx-cri07-baseline.sG1DNC` contained 131
files. Its only development header was `include/crexx_version.h`: there was no
installed `rxpa/crexxpa.h`, transitive `rxinteger.h`, CMake package, imported
target, or supported plugin helper. This reproduces the Gate-1A SDK probe's
need to copy private source/build artifacts.

The additive installed surface now provides:

- `include/rxpa/crexxpa.h`, its generated version dependency, and
  `include/platform/rxinteger.h`;
- a header-only `CREXX::RXPA` target with install-relative includes;
- `CREXXConfig.cmake`, `CREXXConfigVersion.cmake`, `CREXXTargets.cmake`, and
  the supported `RXPluginFunction.cmake` under `lib/cmake/CREXX`;
- imported executable targets and path variables for `rxc`, `rxas`, `rxlink`,
  `rxdas`, both ordinary VMs, both exit VMs, and the `crexx` driver;
- exact `CREXX_VERSION_STRING`/display/build-channel identity, the package
  prefix and BUILDINFO path, plus explicit compiler import, plugin, and runtime
  directories; and
- maintained documentation for `find_package(CREXX CONFIG)`, exact-version
  checking, dynamic plugin output, compiler import paths, and full-path VM
  module loading.

The maintained `rxpa_external_sdk_consumer` test starts from a fresh install,
copies its CMake, C, and Level B sources to an external work directory, and
forbids any installed include beneath the CREXX source root. It then:

1. proves a deliberately wrong full version fails with `CREXX SDK version
   mismatch`;
2. configures and verbosely builds `rx_cri07_sdk_probe.rxplugin` using only the
   installed package and public headers;
3. proves compilation fails without the external plugin import path;
4. compiles and assembles the importer in optimized and non-optimized modes;
5. proves runtime load fails without the external plugin module path; and
6. executes both artifacts on `rxvm` and `rxbvm`, with all four runs reporting
   the exact candidate version and `SDK_ADD_RESULT=42`.

The first completely clean Unix Makefiles build exposed a separate but real
install-path prerequisite regression at 58%: `test_rexxscript_bin` and
`test_rexxscript_direct_bin` named `bin/rexxscript.rxbin` as a file dependency
before the producing subdirectory was visible, so Make reported no rule for
the file. Ninja's cross-directory graph had hidden the missing edge. The
countermeasure adds explicit target-level dependencies from the two consumers
to `rexxscript`; from the exact missing-image state, both targets then built,
their six build/runtime tests passed, and the resumed clean parallel Makefiles
build reached 100%. No source, runtime, language, ABI, or serialized behavior
changed.

The final package proof uses separate `mktemp -d` roots:

```text
clean build:     /tmp/crexx-cri07-clean.bZW30U/build
scratch prefix:  /tmp/crexx-cri07-prefix.s2mTWY
consumer:        /tmp/crexx-cri07-consumer.SPIBqo
candidate:       crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty
installed files: 138
```

Key clean-candidate SHA-256:

```text
98884f301c01b59f1658793a065e361514903a1c067ee624501d1ebdf32c30b3  rxc
c8bcff86ec3f73d9551b92f65e1a5054a391ed947474a28a7c19e30076b3a4a1  rxas
f69086e3d8bf8dc2b11ca0c1daa5775e0c8e62f507cfab2273364f80ed409a0f  rxvm
2ec8797c8c6c8bcd529b91f946f851da255d5d42cf1f5f937f911e694f455d98  rxbvm
30926b1527a19b21dc0cfe28f8ec7bccc4a47444af9f0287c7de042e471a40f0  installed crexxpa.h
569c66c60599e2416d4cd21e4ef186e0392394ce2024726cbeaf746e8a33cba3  external rxplugin
```

Validation results:

- installed consumer in the established Ninja candidate: 1/1 passed;
- broader RXPA/compiler diagnostics slice: 46/46 passed;
- clean Unix Makefiles consumer: 1/1 passed in 5.33 seconds;
- Makefiles RexxScript regression: 6/6 passed;
- copied manual external consumer: optimized/non-optimized passed on both
  VMs, four of four, with exact version and result checks;
- complete clean Debug CTest with `CTEST_PARALLEL_LEVEL=30`: 1935/1935 passed,
  zero failures, in 223.96 seconds. `CREXX_TLS_LIVE_SMOKE` was explicitly
  unset, so the nominal live TLS test remained a local skip path and no hosted
  call occurred; and
- `git diff --check`: passed. No production C/C++ source changed, so an RXPA
  sanitizer run is not applicable to this packaging/documentation-only
  surface.

Raw evidence under `/tmp/crexx-integration-ledger.mZ5jyp/`:

- `cri07-baseline-install.log` and `cri07-baseline-manifest.txt`;
- `cri07-focused-ctest-r3.log`, `cri07-focused-ctest-r4.log`, and
  `cri07-rxpa-focused-suite.log`;
- `cri07-clean-paths.txt`, `cri07-clean-configure.log`, the retained failing
  `cri07-clean-build.log`, `cri07-make-rexxscript-target-proof.log`, and the
  successful `cri07-clean-build-r2.log`;
- `cri07-clean-install.log`, `cri07-clean-focused-ctest.log`,
  `cri07-make-rexxscript-ctest.log`, and
  `cri07-clean-full-debug-ctest.log` (SHA-256
  `39f447565666cda446e749c2e64314f8cba9b6b9c97e234f6e9fca6a9e60927f`);
- `cri07-manual-consumer-configure.log`,
  `cri07-manual-consumer-build.log`,
  `cri07-manual-consumer-compile-load.log`, and
  `cri07-manual-artifact-manifest.txt`; and
- `cri07-candidate-fingerprint.txt` and `crexx-rag-post-cri07-audit.txt`.

The post-item read-only audit preserves crexx-rag branch `main`, HEAD
`97cd87e91344d6ac1773a054bd38df23eb128ed2`, tracked diff SHA-256
`97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9`,
empty index diff SHA-256
`30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d`,
and porcelain-v2 status SHA-256
`02a6e4216ff47be3ce7252be2ccb39157bc45fda316c254f7198648109df14f1`.

The public change is additive. Existing RXPA callback signatures, macro
expansions, dynamic plugin naming/loading, language syntax, ABI, and RXAS/RXBIN
formats are unchanged. Public external support is deliberately for dynamic
plugins; static declaration/definition helpers remain an in-tree core-build
facility. Cross-platform package execution remains for CI/final programme
validation.

## CRI-08 — RXPA const-correct status strings

Disposition: **fixed**.

The exact minimized sources are retained under
`/tmp/crexx-cri08-baseline.XDjCQM/`. Against the pre-change installed SDK, the
C source fails four `-Werror -Wwrite-strings` checks: direct
`rxpa_func_setstring`, `SETSTRING`, `RETURNSIGNAL`, and `RESETSIGNAL` all
discard `const`. The C++17 source fails those conversions without requiring
`-Wwrite-strings` and also exposes a tag/typedef collision at
`rxpa_initctxptr` that prevented the public header from being consumed as C++.
The unchanged sources compile with zero diagnostics against the candidate.

The ownership trace reaches one implementation:

```text
SETSTRING / RETURNSIGNAL
  -> rxpa_func_setstring or rxpa_setstring
  -> rxvm_setstring
  -> set_null_string(value *, const char *)
  -> prep_string_buffer + memcpy into VM-owned storage
```

The input is neither mutated nor retained, so const qualification does not
select a new ownership model. The null-terminated bytes are copied before the
call returns; the caller retains the original buffer and may reuse or release
it immediately. Null is not a supported text value; `""` represents empty
text.

The callback typedef, static shim declaration/definition, compiler guard
function, VM shim, and VM implementation now accept `const char *`. Mutable
callers continue to convert safely. Code that implements a setter and assigns
it through `rxpa_func_setstring` must add `const` to that parameter on rebuild;
that is the intended source-level tightening. The pointer representation and
calling convention do not change.

The header preserves the historical C tag `struct rxpa_initctxptr`, while C++
uses a non-colliding tag behind the same `rxpa_initctxptr` pointer typedef.
Dynamic C++ `LOADFUNCS` now emits the loader-required C-linkage `_initfuncs`
symbol. The static C++ path had two pre-existing macro defects exposed by the
same consumer: the plugin-ID expression was suppressed by token pasting, and
the generated initializer definition did not open its function body. A
two-stage expansion helper and the missing body opener are the bounded
countermeasures. The same C++17 source now compiles with `LOADFUNCS` in both
dynamic and static modes under `-Werror`; the dynamic module also loads and
runs.

The maintained installed consumer now proves:

- C compilation with `-Wall -Wextra -Werror -Wwrite-strings`, using const
  result and diagnostic text through `SETSTRING` and `RETURNSIGNAL`;
- C++17 dynamic and static registration compilation with
  `-Wall -Wextra -Werror`;
- an unmangled exported `_initfuncs` from the C++ `.rxplugin`;
- optimized and non-optimized cREXX imports of the C and C++ namespaces;
- all four `rxvm`/`rxbvm` and mode combinations return exact markers; and
- a mutable C buffer changed from `copy-owned` to `Xopy-owned` immediately
  after `SETSTRING` still yields `SDK_COPY_RESULT=copy-owned`, proving the
  documented immediate copy rather than borrowing/retention.

Binary ABI proof is explicit. The old and candidate C headers report identical
Apple-arm64 table layout:

```text
size=168 setstring=48 setsayexit=152 resetsayexit=160
```

Both archives expose the same C `rxpa_setstring` symbol at the same object
offset in this build. More importantly, the cross-built matrix passes: the
old-header C plugin runs on candidate `rxvm` and `rxbvm`, while candidate-
header C and C++ plugins run on both pre-change VMs. This is four of four with
exact version/result/copy/C++ markers. Callback-table order, size, pointer
representation, loader contract, symbol names, and calling convention are
therefore preserved. Macro expansion changes only C/C++ compilation; it emits
no cREXX, RXAS, or RXBIN contract change and adds no runtime copy or allocation.

Validation results:

- final installed consumer: 1/1 passed in 51.84 seconds after the static macro
  countermeasure and its required rebuild;
- focused RXPA slice: 46/46 passed; the final complete suite re-exercised that
  slice after the last header change;
- final focused Apple ASan: 1/1 passed in 139.35 seconds with
  `ASAN_OPTIONS=detect_leaks=0`;
- final cross-built binary matrix: 4/4 passed across both VMs and both
  header/VM directions;
- authoritative complete Debug CTest: 1935/1935 passed, zero failures or CTest
  skips, in 383.38 seconds with `CTEST_PARALLEL_LEVEL=30`; the live TLS
  environment remained explicitly unset and no hosted call occurred; and
- `git diff --check`: passed.

Raw evidence under `/tmp/crexx-integration-ledger.mZ5jyp/` and the dedicated
baseline directory includes:

- `/tmp/crexx-cri08-baseline.XDjCQM/{const_consumer.c,const_consumer.cpp}` and
  their failing baseline/clean candidate compiler logs;
- `/tmp/crexx-cri08-baseline.XDjCQM/abi-baseline.txt` and
  `abi-candidate-final.txt`;
- `cri08-focused-external-consumer-r2.log` through
  `cri08-focused-after-static-macro-r2.log`, preserving the unrelated metadata
  warning isolation and the diagnosed static macro failures;
- `cri08-rxpa-focused-suite.log`;
- `cri08-asan-logs/20260730-093307-ctest/ctest.log` (SHA-256
  `6b05f1966fad602cdd2ab6af351708df41c7513051a480adcc9bccc294b7470f`);
- `cri08-abi-manifest-final.txt` (SHA-256
  `6f008288299c35ac9330bbf5f2f945363a32644cf8a8352088a716650b4bb108`)
  and `cri08-cross-built-abi-final.log`;
- `cri08-full-debug-ctest-final.log` (SHA-256
  `7ea36e75222cb431296bd7aa0dd595b63db5ab7fd95728c3018e086ef5b1a4c6`);
  and
- `cri08-candidate-fingerprint.txt` and `crexx-rag-post-cri08-audit.txt`.

The item-boundary read-only audit again preserves crexx-rag branch `main`, HEAD
`97cd87e91344d6ac1773a054bd38df23eb128ed2`, tracked diff SHA-256
`97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9`,
empty index diff SHA-256
`30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d`,
and porcelain-v2 status SHA-256
`02a6e4216ff47be3ce7252be2ccb39157bc45fda316c254f7198648109df14f1`.

No ABI or ownership decision is required. The change preserves the public
binary ABI, language design, serialized formats, and existing copy semantics;
cross-platform compiler/package execution remains for CI/final programme
validation.

## CRI-09 — final JSON surface decision

Status: **fixed**. Option B is the implemented public surface; V1 and V2 remain
rejected comparators, and accepted A2 plus B1 are closed by
[`CRI09-CLOSEOUT.md`](CRI09-CLOSEOUT.md).

The maintained generic Level-B reproducer is
`lib/rxfnsb/tests_functional/ts_rxjson_noisy_contract.crexx`. It proves the
current strict `rxjson` functions can implement typed required/missing/null/
extra-field policy, but noisy extraction has no public parser boundary or
retained document. On its 488-character adversarial input, the application-only
workaround performs 4,161 complete parser calls over 1,110,760 copied candidate
characters. Optimized and non-optimized images pass on `rxvm` and `rxbvm`.

Adrian then directed that CRI-09 select the final production JSON surface rather
than land a narrow helper that would be revisited in CRI-13. The complete
architecture packet is
[`CRI09-JSON-SURFACE-DECISION.md`](CRI09-JSON-SURFACE-DECISION.md). It compares
retaining the current surface, the recommended single-module immutable indexed
document, promoting the read-only incubation unchanged, and a native opaque
handle. The recommendation preserves all functional selectors and adds a
single Level-B `.jsondocument`, document-local node traversal, strict typed
getters, and `jsonscancontainer`, while explicitly excluding repair/schema
policy and packed numeric serialization from this decision.

A narrow `jsonspan` prototype was not accepted as production. Its first
four-cell run exposed the UTF-8 byte-offset versus public character-position
hazard for `Gràdh 中`; the code was removed and the failing log retained. No
unapproved production JSON API or serialized format remains in the worktree.

The implementation is governed by `CAP-01-J01`. Before its first production
edit, the worklist predeclares the Release acceptance rule and preserves a
dedicated pre-edit executable. Existing selector medians are guarded within
25%; parse-once construction plus 30 indexed accesses must be at most 50% of
the corresponding retained repeated-parse workload on both optimized VMs; and
the noisy-container path must use a structural scan plus one strict parse of
the returned document.

Frozen V1 passes focused correctness 11/11 and reduces retained parse plus 30
indexed path gets by 79.14%/79.67% on the two optimized VMs. Its mandatory first
Release verdict nevertheless fails: eight of ten existing-selector cells are
36.96--53.39% slower than the paired baseline, and the adversarial scanner
takes 639,531/688,135.5 us because it restarts at 64 invalid openers. Broad
closeout has stopped. The full evidence, cause, countermeasures, and exact
decision are in
[`CRI09-FIRST-RELEASE-VERDICT.md`](CRI09-FIRST-RELEASE-VERDICT.md).

Adrian approved the V2 shared-parser/result-sink countermeasure and requested a
hand-written table-driven tokenizer. Frozen V2 uses a 256-byte class table and
10-state number DFA as a streaming lexer with no token-list allocation. Focused
correctness passes 11/11. The balanced ordinary Release verdict is again mixed:
all legacy cells improve 8.65--18.19% versus V1, retained access improves
75.56%/78.64% versus repeated pre-edit access, and scanner time falls
99.92%/99.90% to 413/523 us. Seven of ten compatibility cells nevertheless
remain 26.92--40.23% slower than pre-edit. The successful scanner candidate is
also boundary-validated then indexed, so it uses two grammar passes while
allocating only one owned slice/index. Exact products, samples, hashes, cause,
independent parser and scanner decisions, and the continuation prompt are in
[`CRI09-V2-RELEASE-VERDICT.md`](CRI09-V2-RELEASE-VERDICT.md).

Adrian approved the lean hybrid A2 and required its benchmark before B1
closure. B1 accepts one allocation-free boundary-validation pass plus one
indexing pass for the successful slice; it is a documentation/closure step,
not another tokenizer implementation. Frozen A2 passes focused correctness
11/11 and every unchanged
Release rule. All ten legacy cells are within the 25% guard and improve
10.41--14.96% versus V2; retained path access improves 76.25%/79.05%; scanner
medians are 394.5/480.5 us; and there is no material optimizer inversion.
Adrian accepted frozen A2 on 2026-07-30. Exact Release evidence is in
[`CRI09-A2-RELEASE-VERDICT.md`](CRI09-A2-RELEASE-VERDICT.md).

B1 documents and accepts one allocation-free boundary-validation pass plus one
indexing pass for the successful slice; it is not another tokenizer phase.
Focused Debug and macOS ASan pass 11/11 across both VMs and opt/no-opt, and
complete Debug passes 1,943/1,943 with zero failures or skips. Exact maintained
artifacts, commands, raw hashes, sanitizer platform limitation, compatibility
statement and remaining risks are retained in
[`CRI09-CLOSEOUT.md`](CRI09-CLOSEOUT.md).

Raw evidence under `/tmp/crexx-integration-ledger.mZ5jyp/`:

- `cri09-baseline-probe-build-final.log`;
- `cri09-baseline-probe-crossvm-final.log`;
- `cri09-baseline-probe-raw-final.log`; and
- `cri09-jsonspan-crossvm-focused.log` (discarded prototype failure).

## CRI-10 — multiline ADDRESS output capture

Disposition: **documented/package-closed**.

The existing ADDRESS redirect machinery already supports scalar stream-text
capture and line-oriented `.string[]` capture for stdout and stderr, with the
command status in `rc`. The new maintained Level B contract covers multiline
output/errors, empty output, Unicode and delimiters across opt/no-opt and both
VMs; focused CTest passes 5/5. No production, syntax, facade, ABI or format
change was needed. Exact proof and the CRI-12 reuse boundary are in
[`CRI10-CLOSEOUT.md`](CRI10-CLOSEOUT.md).

## CRI-11 — argv-preserving ADDRESS execution

Disposition: **documented/package-closed**.

The existing `ADDRESS CREXX "run :argv[]"` surface launches a program directly
from an exposed `.string[]`, preserving whitespace, empty arguments, quotes,
Unicode and shell metacharacters without shell interpretation. A harmless
external fixture and maintained Level B test pass 5/5 across optimized and
non-optimized compilation and both VMs. `COMMAND`, `SYSTEM`, and `CMD` remain
command-string environments. No new syntax or runtime change was needed. Exact
proof is in [`CRI11-CLOSEOUT.md`](CRI11-CLOSEOUT.md).

## CRI-12 — redirect array lifecycle

Disposition: **documented/package-closed**.

Redirect arrays intentionally append captured records to the caller-owned
mutable array. Empty streams preserve earlier elements and command failures use
the same independent `OUTPUT`/`ERROR` rule. Callers use `arraydrop` before a
command when replacement-style reuse is required. The maintained Level B test
passes 5/5 across opt/no-opt and both VMs. No production change was needed.
Exact proof is in [`CRI12-CLOSEOUT.md`](CRI12-CLOSEOUT.md).

## CRI-13 — parse-once JSON and packed numeric entities

Disposition: **fixed**.

The accepted CRI-09 document is the final parse-once surface. The unchanged
existing JSON benchmark shows 76.25%/79.05% faster retained path use and
93.87%/94.11% faster resolved-node use. The new current-API numeric diagnostic
passes opt/no-opt and both VMs and separately records parse, traversal, typed
materialization/copy, raw f32/i64 conversion, direct scans, JSON encoding, total
and peak RSS. Current public composition takes 27.14--29.16 ms; a renamed
out-of-tree integrated-projection prototype shows a 13.19x--19.77x plausible
mechanism gain.

The recommendation is explicit bulk projection to existing headerless
canonical-little-endian `.binary`, without adopting wrapper classes,
normalization metadata or the historical `F32V`/`I64V` envelope. Exact options,
raw evidence and the smallest decision are in
[`CRI13-PACKED-NUMERIC-DECISION.md`](CRI13-PACKED-NUMERIC-DECISION.md).

Adrian approved B as the primitive and directed a later by-value C class
comparison. Frozen B passes focused correctness and six of seven first Release
rules, but fails the prototype ceiling: f32 projection is 11.28x--13.05x the
prototype versus the allowed 2x. The exact verdict and recommended private
allocation-free source-span countermeasure are in
[`CRI13-B-RELEASE-VERDICT.md`](CRI13-B-RELEASE-VERDICT.md).

The approved R1 countermeasure also passes correctness, exact output,
optimizer and unchanged-JSON guards, but its repeated Release verdict is
adverse: optimized f32 projection is 5.4295/5.863 ms, 20.99x/22.01x the
prototype, and optimized `rxbvm` total reaches 25.38% of retained current. RXAS
proves the classifier is fully inlined. Two hidden whole-source copies per
element move 359,294,976 logical bytes; a no-loop-copy scratch control improves
to 1.311/1.755 ms but still misses the ceiling. Exact results and the
recommended parse-time private-node flags are in
[`CRI13-B-R1-RELEASE-VERDICT.md`](CRI13-B-R1-RELEASE-VERDICT.md).

The machine trace also establishes a separate systemic follow-on. RXAS already
has CFG, liveness, effects and copy-flow analysis but deliberately rejects
generic copies as `full-value-ownership-unproved`. A bounded register-local
full-copy proof, followed by a separate `linkattr/copy/unlink` lifetime panel,
is queued as `PERF2-07-B02`; it cannot waive CRI-13's failed ceiling. Exact
static/dynamic counts, byte volumes, controls and RXAS diagnostics are in
[`CRI13-R1-RXAS-TRACE.md`](CRI13-R1-RXAS-TRACE.md). No item is active, and C
and CRI-14 were deferred at that R1 stop.

Adrian then approved the parse-time flag countermeasure and observed that the
existing conversion signal can be translated at the public method boundary.
Frozen R2 passes every rule: optimized f32 projection is 295/326 us,
1.14x/1.22x the prototype, and production total is 9.60%/11.03% of retained
current. Adrian accepted it; complete Debug passes 1,963/1,963 and affected
ASan passes 17/17. Evidence:
[`CRI13-R2-RELEASE-VERDICT.md`](CRI13-R2-RELEASE-VERDICT.md) and
[`CRI13-R2-CLOSEOUT.md`](CRI13-R2-CLOSEOUT.md).

The approved benchmark-local C comparison then tested headerless owning f32
and i64 wrappers with typed by-value `read` and `write`. Correctness passes 5/5
and formal Release 40/40, but optimized reads are 5.01x/4.71x raw B and writes
are 3.67x/3.19x on `rxvm`/`rxbvm`. RXAS shows resolved but uninlined method
calls with initialization and attribute link/unlink, not per-element binary
copies. CRI-13 is now decision-blocked with no active item. The recommendation
is B only for Release 1 and a later generic class-method-access countermeasure
before reconsidering public C wrappers. Evidence:
[`CRI13-C-CLASS-COMPARISON-DESIGN.md`](CRI13-C-CLASS-COMPARISON-DESIGN.md) and
[`CRI13-C-CLASS-RELEASE-VERDICT.md`](CRI13-C-CLASS-RELEASE-VERDICT.md).

Adrian accepted that disposition. The probe classes were removed; the
maintained benchmark and its opt/no-opt Release images exactly match frozen R2,
and the restored dual-VM matrix passes 5/5. Final evidence:
[`CRI13-CLOSEOUT.md`](CRI13-CLOSEOUT.md).

## CRI-14 — generic operation contract for external consumers

Disposition: **fixed** under approved Option B.

The retained Level B probe uses typed operation/request/result/error interfaces,
concrete record-like classes, arrays, an optional argument and RexxDoc. Its
optimized and non-optimized RXAS are byte-identical and the resulting graph
views are byte-identical: 19 types, 10 members, 12 callables, 3 relationships
and 10 declarations.

The graph is a useful static exporter seed but not a complete public contract.
It omits all nine raw class attributes and all RexxDoc, carries no version,
nullability, error linkage or evolution policy, exposes no parameter names in
its structured view, and retains several relative object spellings as separate
opaque types. Level B runtime reflection is value-only. The pre-decision fresh
141-file scratch install contained no graph header or CMake target; its copied
static library was not a supported SDK, and the minimized external configure
failed exactly on missing `CREXX::RXBIN`.

The recommendation is a build-time `crexx-contract` tool and CMake helper that
derive a closed contract from existing Level B interfaces and emit the new
deterministic `crexx.operation-contract/1` JSON artifact. This avoids language
syntax, runtime reflection, a public C ABI and an RXBIN 007 change, but the new
CLI/CMake and JSON format require Adrian's approval. Exact source, commands,
hashes, format-1 mapping/evolution rules, alternatives and continuation prompt
are in
[`CRI14-CONTRACT-SURFACE-DECISION.md`](CRI14-CONTRACT-SURFACE-DECISION.md).

Adrian approved Option B on 2026-07-30 and clarified that the emitted
`crexx.operation-contract/1` artifact is the durable long-term surface even if
metadata acquisition later evolves. The implementation therefore separates a
metadata-independent contract model/writer from its initial private RXBIN 007
adapter.

The installed CLI/CMake surface, strict type/evolution matrix, global-selector
regression countermeasure, focused/ASan/clean-full validation, 141-file scratch
install, external consumer hashes and read-only downstream replay are retained
in [`CRI14-CLOSEOUT.md`](CRI14-CLOSEOUT.md). Final clean Debug is
1,965/1,965 with zero failures/skips. The no-fallback downstream build succeeds;
23/26 tests pass and the three failures are exact downstream removal seams for
the retired `rx_socket` argument and superseded incubator JSON wrapper.

The complete one-to-one disposition table, compatibility statement, rejected
alternatives, unblock matrix, suggested downstream ledger closure text and
Phase-1B stop prompt are in
[`CREXX-RAG-INTEGRATION-CLOSEOUT.md`](CREXX-RAG-INTEGRATION-CLOSEOUT.md).
