# Mainframe upstream integration review

30 September 2026. Base `e7ac9edacd1ec053c4df2b4c082961bae084ff69`.
Execution plan: [mainframe integration](../../planning/beta-3/mainframe-integration-2026-09-30.md).

## Scope and source identity

The supplied Lab tree has HEAD `db87e9ac3b329dbc5d6a20d93da442bdcca9601a`
plus its uncommitted repair. All 44 changed files were compared byte-for-byte
with the imported patch before review follow-ups; all matched. No Lab source
or SDK/runtime file was edited. The current develop base was exactly the Lab
commit's parent, so the full delta applied without conflict.

The review inspected the production patch, all permanent fixtures and the
existing approved architecture/text plans. Compiler/assembler edits outside
entry points change diagnostic characters only; no optimizer rule, language
syntax or RXBIN format is changed. The record-table and AST guards are shared
repairs for reproduced CMS memory exhaustion. SDK WAIT and increased native
heaps/stacks remain Lab runtime inputs and are not copied into cREXX.

## Findings and limits

1. **Console failure propagation, reproduced:** `platform_console_text_write`
   rejects the UTF-8 euro sign because IBM1047 cannot represent it. The actual
   UTF FWRITE handler discards the return, reaches normal dispatch, produces
   zero bytes, leaves `ferror(stdout) == 0` and sets only `errno == EILSEQ`.
   SAY's default callback likewise ignores the helper's status. The public
   callback is void and the existing write handlers already ignore libc write
   return values. Extending this to a Rexx signal is a behavior decision; the
   source integration does not silently invent that policy. The maintained
   stdio fixture tests the helper's negative return, not Rexx propagation.
   [console-error-probe.c](console-error-probe.c) selects the real FWRITE body.
   This is an ordinary error-reporting issue, not a sanitizer finding. Adrian
   approved "Raise a signal" on 30 September. The follow-up uses the existing
   execution-local pending signal API in the default SAY callback and existing
   SET_SIGNAL_MSG in the UTF console handlers. Conversion errors raise
   UNICODE_ERROR; output/flush errors raise NOTREADY. The callback ABI is unchanged.
   The permanent `mainframe_console_signals` test executes the real switch VM
   for both terminal and caught failures, native accent bytes, custom callbacks
   and actual read-only output-descriptor failures.
2. **HIGH/PDOS guest chain, later native closure:** the initial Lab wider-profile report
   records native RXC RC0 followed by RXAS rejection of fresh assembly. It also
   records a PDOS console-field overflow during diagnostics. Initial member readback identified two lost output blocks at track
   transitions; the user-requested repeat reproduced it. During final upstream
   CI, the Lab traced WRBLOCK accepting unit check as a successful full write
   and completed its separate PDOS kernel repair. Its updated repair report and
   WIDE-06 record native RXC/RXAS/fresh RXVM RC0/0/0, a complete 3,108-record
   assembly matching the host byte-for-byte, interactive and ordered byte proof,
   with independent coordinator review. That closes the frozen-package PDOS
   blocker; it does not qualify this new upstream signal follow-up or modern
   z/OS HIGH. Some summary text still retained the earlier partial status at
   readback and must be reconciled in the Lab. The cREXX baseline stayed fixed.
3. Converted text streams are sequential and single-direction. Text `+` modes
   fail before opening/truncating; binary modes retain libc behavior. The
   frozen native SDK lacks append despite host codec append proof. Arbitrary
   converted seeking and every native selector/storage layout are not qualified.
4. The real-handler UTF/BYTE and console fixtures are Apple host components.
   The Linux funopen adapter is test-only and requires hosted compilation.
   The CMS frontend fixture deliberately mixes desktop frontend diagnostics
   and native entry-point diagnostics and does not establish full guest stdout.
5. Exact formal beta packages and manual installation/execution remain open.
   Prior guest builds and the older final sanitizer matrix have their own
   source/runtime/artifact identities and are not relabelled for this candidate.

## Qualification

The final product/test commit is `6a09f7786b79c025981bda5d7a891982f8304dc0`. Documentation-only
preparation follows it; [product-inputs.json](product-inputs.json) pins all 42
changed product/test/build files and the exact qualified tree. No product input
changed after final qualification. [results.json](results.json) pins commands,
capability limits and log digests.

| Check | Result | Retained receipt |
| --- | --- | --- |
| Core, optional ordinary tools, comprehensive prerequisites and native host fixtures | Build passes | [Debug build](debug-build.log) |
| Normal correctness, parallel 30 | 2,293/2,293; 766.24 s | [Debug CTest](debug-correctness.log) |
| Focused maintained Apple ASan, serial | 8/8; 23.26 s; no sanitizer finding | [ASan build](asan-build.log), [focused CTest](asan-focused.log) |
| Actual console signal paths | 27 assemble/real-VM checks, including terminal/caught conversion errors and write/flush errors | [command receipt](console-signals.log) |
| Release packaging guard units | 13/13 | [guard receipt](release-guards.log) |

The nested CMS matrix was measured alone in Debug (0.95 s) and Apple ASan
(19.06 s); the console matrix measured 0.37 s / 4.09 s. Both retain serial
scheduling and a generous 300 s hang backstop. No syntax, optimizer policy,
bytecode format or callback ABI changed. Earlier pre-signal 2,292/2,292 and
focused 24/24 / Apple ASan 7/7 receipts are historical and are superseded by
these final product-input results.

Apple provides no LSan; Linux ASan/LSan and broad platform assurance remain
separate formal gates. Automatic development
[Build CREXX 36749864309](https://github.com/adesutherland/CREXX/actions/runs/36749864309)
and [CodeQL 36749863689](https://github.com/adesutherland/CREXX/actions/runs/36749863689)
pass on `e15392da705c5e9e0e1e57b70586b53fc8cb64ed`. All four core Release/package
lanes, MinGW, optimizer parity and four configured base plugin lanes pass. The
published dev-snapshot tag points at that exact commit; 30 assets are present.
All four actual core-package smoke summaries and the Mac/Windows SDK receipts
identify it. CodeQL analysis 1868703980 reports nine existing open alerts
(216–218, 222–227); no new alert, analysis error or warning. Existing alert
triage remains separate from a successful analysis workflow. Nonblocking CI
warnings identify the existing MSYS2 MINGW64 environment and Node 20 action
runtime deprecations. [publication.json](publication.json) retains terminal
runs, package digests and the late native boundary update. The final child is
documentation/evidence only; unchanged valid product-input proof is reused
without another local suite or duplicate CI matrix. The [formal candidate handoff](../../planning/beta-3/formal-candidate-2026-09-30.md)
records the remaining release work. Beta 3 is prepared, not tagged/released.
