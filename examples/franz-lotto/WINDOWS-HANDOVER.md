# Franz Lotto: Windows continuation

This is the durable Windows continuation of the Lotto preparation plan. The
acceptance and step IDs below retain the original intended outcome and local
evidence. Update this file with native Windows results; source transport alone
does not close the Windows gates.

## Vision and scope

Give Franz two readable Lotto teaching examples: a procedural cREXX ANSI form
compatible with the **released `v1.0.0-beta.3` tools**, and a NetRexx Swing desktop
window. Both support Austrian 6/45, German 6/49 and Euro-Lotto 5/50 + 2/12,
integer counts 1–50, sorted unique balls within each pool, all-result viewing,
explicit Save, and clean Quit. An optional cREXX GTK window reuses the same
model through the existing `gui` plugin, with a standalone provider build kit.
The intended delivery is a tested Windows example package with source and clear
instructions; the GTK add-on needs its native plugin and full runtime closure.

Adrian authorized a temporary GitHub branch to transfer the examples after app
handoff failed. This branch adds source/docs/tests only under `examples/`; it
does not change production compiler, VM or UI code. The original private email,
compiled artifacts and third-party dependency bundles are excluded. The current
transport phase must not be described as a completed Windows qualification.

## Acceptance and current status

1. **AC-01 — Lotto behavior:** all three games and every count 1–50 produce the
   correct count/range/order with no duplicate balls within a pool. Invalid
   numeric entry fails before conversion. **Mac passed; Windows open.** Run
   `lotto_checks` and `LottoTests`, then inspect representative generated output.
2. **AC-02 — Interaction/lifecycle:** exclusive choices, numeric entry,
   Generate, complete result viewing, Save confirmation/failure recovery and
   Quit work; startup/Generate do not write the tips file. **Mac ANSI/Swing
   checks passed; Windows manual checks open.** Use `WINDOWS-CHECKLIST.txt`.
3. **AC-03 — Beta 3 compatibility:** compile, assemble, link and run cREXX with
   the unchanged released Beta 3 product. `rxc -v` must report
   `crexx-1.0.0-beta.3`. **Mac passed; Windows exact-version proof open.** The
   transport branch's development toolchain is not a substitute for this gate.
4. **AC-04 — Windows delivery:** quoted-path build/run/test commands, dependencies,
   controls, output locations and troubleshooting are provided. **Instructions
   prepared; native execution and resulting receipt open.**
5. **AC-05 — Final package:** package only intended source, application artifacts,
   instructions, license and accurate receipt, then unpack/rebuild/run in a clean
   directory with spaces. **Original Mac package passed; Windows package open.**
   This Git branch intentionally carries no generated binaries or email text.
6. **AC-06 — Optional GTK GUI:** preserve the existing Beta 3 plugin/API, build
   the matching native provider, pass widget smoke and interactive checks, then
   test its dependency bundle without a development GTK directory on PATH.
   **Mac provider/widget smoke passed; Windows build, interaction and bundle
   closure open.** Never call the source kit a qualified Windows GUI add-on.
7. **AC-07 — Source transport:** preserve tested source and API tags, publish only
   example-related files on `codex/franz-lotto-windows`, and identify the exact
   commit/base for the Windows agent. Source identity and content audit are the
   transfer checks; the remote commit is verified after push.

## Steps and dependencies

1. **STEP-01 (AC-01, AC-03):** inspect game rules, shipped APIs and exact release
   tools. Complete locally; Beta 3 tag is
   `ae1607b8e145174422cee7f3e73fbcc37a65226c`.
2. **STEP-02 (AC-01, AC-02):** implement separate model/screen layers. Complete;
   the code here is unchanged from the tested preparation.
3. **STEP-03 (AC-01–AC-04):** Windows agent builds/runs focused model, file and
   interactive checks with the released tools. Open. Record commands, versions,
   outcomes and any failure before changing code; avoid overlapping build/test
   commands in one Windows build tree.
4. **STEP-04 (AC-04, AC-05):** after native checks, update the recipient receipt,
   rebuild a clean delivery package and verify it. Open on Windows.
5. **STEP-05 (AC-06):** build the GTK provider separately and qualify its complete
   Windows dependency add-on. Open on Windows; no new widget/library API is
   authorized by this transfer.
6. **STEP-06 (AC-07):** copy the existing sources into an isolated managed
   worktree, adapt source-only documentation, audit and push the transport
   branch. Current Mac transfer phase. Expected publication is one source-only
   commit on the branch below; no release promotion or tag is part of this task.

## Transport and previous validation

- Branch: `codex/franz-lotto-windows`.
- Base: `origin/develop` at `b1b2a742315738acfdf894ed0d08e93060c6ed8c`.
- Examples: `examples/franz-lotto/crexx/` and `examples/franz-lotto/netrexx/`.
- Provider kit: `examples/franz-lotto/crexx/gui-provider/`.
- Source identities: `SOURCE-SHA256.txt`; prior validation: `TEST-RECEIPT.txt`.
- macOS ARM64: both models checked 3,825 tips each; invalid counts, Save,
  replacement and failure recovery passed. cREXX also passed `-n` model checks.
- Real ANSI PTYs covered paging through 50 tips, mouse, numeric paste, resize,
  Save failure/recovery, Quit/Ctrl-C and terminal/cursor restoration.
- Swing controls were exercised on the event-dispatch thread with JDK 21 and
  NetRexx 5.10-GA; compiled execution needed no NetRexx runtime JAR.
- The unchanged Beta 3 GUI provider built with GTK 3.24.52; released Beta 3
  tools/VM passed the actual GTK widget smoke. Clean extracted source rebuilds
  also passed under paths containing spaces. Native Windows remains unverified.

## Fetch and build on Windows

Use a clean checkout or an appropriately isolated Windows worktree. Preserve
unrelated local work rather than resetting it to this branch.

```text
git fetch origin codex/franz-lotto-windows
git switch --track origin/codex/franz-lotto-windows
```

The following are **Command Prompt (`cmd.exe`) commands**. `CREXX_BIN` names an
installed/extracted **official Beta 3 Windows x64 core** bin directory. No llama
plugin/model is needed. Start in `examples\franz-lotto\crexx`:

```text
set "CREXX_BIN=C:\Program Files\CREXX\bin"
"%CREXX_BIN%\rxc.exe" -v
"%CREXX_BIN%\rxc.exe" -o lotto_logic lotto_logic.crexx
"%CREXX_BIN%\rxas.exe" -o lotto_logic lotto_logic
"%CREXX_BIN%\rxc.exe" -o lotto lotto.crexx
"%CREXX_BIN%\rxas.exe" -o lotto lotto
"%CREXX_BIN%\rxlink.exe" -s -o lotto_app lotto lotto_logic "%CREXX_BIN%\library"
"%CREXX_BIN%\rxc.exe" -o lotto_tests lotto_tests.crexx
"%CREXX_BIN%\rxas.exe" -o lotto_tests lotto_tests
"%CREXX_BIN%\rxlink.exe" -s -o lotto_checks lotto_tests lotto_logic "%CREXX_BIN%\library"
"%CREXX_BIN%\rxvm.exe" lotto_checks.rxbin
"%CREXX_BIN%\rxvm.exe" lotto_app.rxbin
```

Keep the whole Beta 3 core installation, including the adjacent `rxconsole`
provider. Use a foreground terminal at least 80x24. Stop on any failed build
command; do not run stale artifacts. `crexx/WINDOWS.txt` documents controls and
output. The model checks write only their own `lotto-test-output.txt`.

In `examples\franz-lotto\netrexx`, the retained baseline is JDK 21 and NetRexx
5.10-GA. The Windows continuation already has JDK 26 and NetRexx: use those
existing tools initially and record their exact versions/results. Do not install
a second JDK merely for this transport. Adjust `NETREXX_JAR` to the actual
installation (the path below illustrates the retained NetRexx version):

```text
set "NETREXX_JAR=C:\Tools\NetRexx-5.10-GA\lib\NetRexxC.jar"
java -version
java -cp "%NETREXX_JAR%" org.netrexx.process.NetRexxC -noverbose LottoModel.nrx LottoWindow.nrx LottoTests.nrx LottoGuiTests.nrx
java -cp . LottoTests
java -cp . LottoGuiTests
java -cp . LottoWindow
```

The optional GTK kit requires a matching Windows C compiler, CMake >=3.24,
pkg-config and GTK **3**, plus the complete GTK runtime closure. Follow
`crexx/gui-provider/WINDOWS-BUILD.txt` for MSYS2 UCRT64 build, explicit plugin
loading, smoke invocation and the clean dependency-bundle gate. It builds only
the extra provider; the core compiler/VM need no production changes.

Finish the manual cases in `WINDOWS-CHECKLIST.txt`, especially the file chooser,
overwrite cancellation, numeric paste/resize, last-result paging/scrolling,
saved-game identity, normal terminal restoration and native GUI close. Report
Windows results separately from the retained Mac evidence. No full/deep QA
dispatch is required merely to transport these unchanged standalone examples.
