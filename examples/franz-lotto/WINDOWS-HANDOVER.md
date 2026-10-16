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

### Accepted Windows extension (12 October 2026)

Adrian requested an enhanced GTK library, a running native Windows executable,
and a demonstration of the same app in console and GTK. He selected three radio
buttons and a numeric spin control. Keep `lotto_logic.crexx` as the shared model,
retain the original Beta 3 ANSI compatibility baseline and the NetRexx example,
and align the new demonstration with the emerging Level G UI surface. One Lotto
component owns model/update/view and Save/close workflows; ANSI and GTK run the
same session through their existing drivers. The original procedural examples
remain compatibility baselines. New widgets belong in the logical view and
driver projections. Native GTK remains mechanism behind its owned mailbox.

### Approved shared control contract — Adrian, 12 October 2026

1. Add one logical `choice` node: stable node ID, parallel option ID/label
   arrays, selected stable option ID and existing relative placement. The
   builder operation is `add_choice(id, option_ids, option_labels, selected,
   relation, anchor)`. GTK projects three radio buttons for Lotto; ANSI
   projects the same exclusive choice with keyboard and pointer operation.
   Selection posts the existing `widget.selection.changed` event with `node`
   and a singleton `items` array, targeted to the owning component.
2. Add one logical bounded `integer` node with current value text, minimum,
   maximum, positive step and existing placement. The builder operation is
   `add_integer(id, value, minimum, maximum, step, relation, anchor)`. Preserve
   text while editing and post the existing `widget.value.changed` and
   `widget.value.committed` events. The feature validates before conversion;
   GTK supplies spin buttons, and terminal hosts supply equivalent editing
   and increment/decrement actions. Lotto uses 1–50, step 1. Reject unsupported
   driver bounds explicitly rather than truncating or silently substituting.
3. Keep Generate, Save, paging and Quit as registered application commands.
   Reuse the existing chooser/confirmation and `io.text.write` catalogue
   contracts for Save. Extend the shared local-resource executor with bounded
   writes and session-owned write grants, preserving existing read grants.
   Cancellation/failure return correlated events; the component performs no
   physical file I/O. Rendering model values must not generate new user events.
4. Keep the current flat view, placement rules, bounded owner queues, fixed
   authored topology, deferred dialogs and close lifecycle. Update all affected
   RexxDoc/API tags, catalogue conformance and real-driver tests together. No
   new reserved event names, native application callbacks, generic property bag
   or driver-specific controls are proposed in application code.

Adrian explicitly approved this shared UI design on 12 October 2026. This is
the implementation contract for the extension; preserve the owner/session,
event/effect and resource-grant boundaries described above.

### Retained Windows baseline evidence (12 October 2026)

- Source transport: remote commit `4bb54b783aa9661a1bc13d99a356a69ac3d11b48`,
  base `b1b2a742315738acfdf894ed0d08e93060c6ed8c`. Original ZIP SHA-256:
  `fb8fd73b84a56ce00ea19de203e96eb36d96ea171d390e8ff1deda81e0ada741`.
  All 30 packaged hashes passed. All 13 Git source/build inputs match after
  LF normalization; `.gitattributes` now preserves their original LF bytes.
- Installed core: `C:\Program Files\CREXX`, `rxc -v` reports Beta 3;
  `BUILDINFO` commit `ae1607b8e145174422cee7f3e73fbcc37a65226c` matches the
  fetched `v1.0.0-beta.3` tag. Main Windows `develop` checkout remains unchanged.
- Test root: `C:\Users\adria\Documents\Franz Lotto Windows 20261012`.
  Original ZIP was unpacked there; sources rebuilt independently in the
  `Rebuilt cREXX` and `Rebuilt NetRexx` folders, both containing spaces.
- Packaged and rebuilt cREXX model checks passed: 3,825 tips, all games/counts,
  validation, Save, replacement and failure. Builds used the installed exact
  Beta 3 `rxc`, `rxas`, `rxlink` and `rxvm`.
- Packaged NetRexx model and Swing-control checks passed with JDK 26. Source
  rebuilding with NetRexx under JDK 26 failed resolving `System.out` to the
  inaccessible `java.lang.System.Out`. Portable Temurin 21.0.12.1+1 rebuilt
  all sources and passed both checks. Keep JDK 21 as the source-build baseline.
- Physical Swing checks: selected Euro, entered/generated 50, used the real
  file chooser, saved 51 records with tip 50, rejected 51 while retaining
  results, and used Quit. Original Java 8-targeted classes ran without a
  NetRexx runtime JAR.
- Original GTK provider built with MSYS2 UCRT64 GCC 16.1.0 / GTK 3.24.52.
  Packaged and freshly compiled/assembled/linked widget smokes passed.
  Physical GTK checks selected Euro, generated/saved 50, scrolled to tip 50,
  and used Quit; VM exited zero. A dependency-isolated native bundle is still open.
- Native Windows pseudo-console: original ANSI form selected Euro, generated
  50, paged to tips 49–50, required a second Save, wrote all 51 records and
  retained the generated Euro identity after selecting Germany. Empty, 0 and
  51 preserved results. Quit and a separate Ctrl-C run exited zero with cursor
  restoration and alternate-screen exit. Capture/transcript logs are retained
  at the test root. Physical mouse and viewport-resize checks remain open.
- Native packaging baseline: scoped `CREXX_HOME` must name the Beta 3 prefix
  rather than the pre-existing `.local` setting. The correct-prefix
  `crexx --program Lotto-Console lotto.crexx lotto_logic.crexx --native` build
  compiled/assembled/linked successfully, then failed because
  `bin/providers/rxconsole.lib` (or `rxconsole_static.lib`) is absent from the
  installed core. Its dynamic `rxconsole.rxplugin` is present. Investigate the
  exact-tag static provider/SDK closure; do not replace installed runtime files.

Adrian authorized a temporary GitHub branch to transfer the examples after app
handoff failed. The original transport commit added only examples. The approved
shared UI extension now also changes library/driver code and its tests, while
the optimiser repair is supplied by current `develop`. Original private email,
compiled artifacts and third-party bundles remain excluded from Git. Source
transport must not be described as completed Windows qualification.

### Accepted continuation — 13 October 2026

Continue with the shared GTK tests and native console/GTK executables using
current development code. Coordinate any required static-provider distribution
correction with the Mac agent. Prepare the NetRexx demonstration and a readable
side-by-side guide to the OO component/session/driver separation and native
packaging. After Windows qualification, prepare a matched interim distribution
with signed Windows assets for Franz; neither local engineering binaries nor an
unsigned snapshot satisfy that final delivery gate.

The topic branch now includes `fcb361b6e` (local merge `88a2c060`), preserving
the approved UI overlay. This includes optimiser fix `a415090dc`, console
archive staging, the llama-free core dependency helper and corrected partial
DLL collection. Windows tools are built locally with MSVC; the installed
released Beta 3 prefix remains unchanged.

## Acceptance and current status

### Windows continuation evidence — 13 October 2026

- Isolated branch includes console/static-package corrections `e56c90ddd` and
  `1031ef530` (merge `776be1e4`). The normal Windows product built with MSVC
  14.44. Current `rxc -v` identifies the local dirty development build, distinct
  from the unchanged installed Beta 3 product.
- The optimiser's permanent four-way smoke and Lotto Save chain passed on
  Windows. All 27 focused gates passed by retaining 21 unaffected results and
  rerunning six corrected host tests. Old probes assumed Save was unsupported;
  they now test closing/cancelling an active Save chooser. The native test
  injector compares file identities across Windows path separators.
- The matched cohort initially passed 26/27 gates and exposed a real GTK
  admission/projection race (`edit:17 commit:10`). A deterministic selection/edit
  burst with an owned idle commit reproduces it. Choice/integer projection now
  waits for captured native facts to drain. Six affected GTK opt/noopt gates
  passed after the fix; 21 unaffected current results are retained.
- GTK controls passed both modes: exclusive radios, spin/edit/commit, silent
  model projection and driver reuse. Full shared Lotto GTK tests passed paging
  to 49–50, invalid 51 preserving results, cancellation preserving a sentinel,
  replacement and generated-game identity.
- `Lotto-Console.exe`, built through the native project driver with explicit
  UI link modules, passed ConPTY checks with empty `CREXX_HOME` and system-only
  `PATH`: Euro/50, paging, invalid 51, cancellation without a new file, Save and
  replacement of 51 records, Euro identity after selecting Germany, write
  failure recovery, Quit zero and terminal restoration. Selected capture is
  `native-console-conpty-capture.log` at the test root.
- `Lotto-GTK.exe` embeds the static provider and uses adjacent DLLs. GTK's
  conservative V2 query publishes its stable ID without promising reentrancy.
  A native integration EXE using the same feature passed the complete GTK
  workflow with system-only `PATH` and default data lookup. Relative pixbuf
  loaders, SVG dependencies, MIME data and icon themes removed the initially
  observed resource warnings. The app-root set is 57 DLLs, including ten CRT
  redistributables; nested pixbuf modules and data remain part of the bundle.
- The generic dependency helper is now a normal llama-free core target. The
  current Windows build compiled it from RXPA sources, replacing the temporary
  engineering helper. The corrected collector generated a fresh GTK provider
  package in an empty directory: all 77 declared link/runtime hashes passed,
  including 61 DLLs and the native provider. No manual DLL pre-staging was used.
- Live native GTK checks after unlocking Windows passed Euro radio selection,
  numeric entry of 50, generation, all seven pages through tips 49–50, real
  chooser cancellation with results retained and clean Quit. The actual app
  screenshot is in `screenshots/crexx-gtk-native-windows.jpg`. The UI automation
  helper could click the chooser but timed out typing into its owned modal;
  full Save/confirmation/overwrite remains covered by native integration tests.
- `Lotto-NetRexx.exe` was created with JDK 21 `jpackage` and its bundled runtime.
  It opened with system-only `PATH`, generated 50 Euro tips, used the real
  chooser to save 51 records and closed through Quit. Original source/model and
  Swing checks remain valid. Its guide screenshot is from this native launcher.
- The NetRexx native build script passed fresh source compilation, both model
  and Swing tests, and jpackage generation under a path containing spaces.
- Both cREXX EXEs were rebuilt through `surface/BUILD-WINDOWS.ps1` into a fresh
  space-containing directory with the normal core helper. The native GTK feature
  integration passed warning-free with system-only PATH/default data lookup.
  The rebuilt console generated Euro/50, confirmed Save, wrote all 51 records,
  exited zero and restored the terminal. See `WINDOWS-NATIVE-RECEIPT.md`.
- All 13 legacy source hashes now match exactly after restoring LF. RexxDoc
  block/parameter/return coverage is preserved or increased in the changed UI
  sources covered by the retained baseline.
- UI source `a8be8ca89` is published and remote-verified. Core helper fixes
  `b112fe05a`/`ba62fbe7f` are merged, and normal MSVC/MinGW targets pass the
  unchanged Unicode/negative-control harness with system-only PATH. MinGW's
  three staged runtime DLL hashes match the selected compiler. MSVC uses a
  target-local C++ `/EHsc` option for exception-path ownership.
- The unsigned review ZIP is privately uploaded: 4,246 files, 90,999,512 bytes,
  SHA-256 `36e063506519a4530cdb7a4eca3c1674a2913f3071a78acbad63e20e98afd27f`.
  All clean-extraction hashes/PE subsystem checks passed. Final signed recipient
  delivery and matching successful CI/signing inputs remain open. No signed assets,
  new release tag, final recipient Drive package or email have been produced.

### Mac diagnosis dispatch — 12 October 2026

Adrian requested a separate agent on the faster Mac using `gpt-6.1-sol` with
`xhigh` reasoning. The new chat **Test shared Lotto UI optimiser on Mac** is
running on the Mac host (chat `01a130b1-c73c-7491-8423-52f3b97657bb`). It received
the private Drive engineering snapshot `Mac-UI-Investigation-20261012.zip`
(109,403 bytes; SHA-256
`461503d840c7f7849b6d1c2646f5e6b1ea3de3d4159ff7aa9250fe6fa41df48f`). All 32
manifest entries passed after Windows archive extraction. This contains the
unfinished source overlay and opt/noopt logs; it is not a recipient release.

The focused `surface/lotto_surface_tests.crexx` reproducer establishes that
the direct chooser/confirmation/write session chain passes under Beta 3, while
the optimised line host loses its confirmation modal. Rebuilding only
`ui_tui.crexx` with `rxc -n` and `rxas -n`, retaining other optimised modules,
makes the entire test pass. The first Mac gate is to test the exact snapshot
with verified current development tools, before declaring this an outstanding
compiler defect. Mac testing/diagnosis is authorized; this dispatch does not
authorize production compiler edits or release promotion. Enhanced shared GTK
runtime validation is still open, distinct from the proven legacy GTK baseline.

1. **AC-01 — Lotto behavior:** all three games and every count 1–50 produce the
   correct count/range/order with no duplicate balls within a pool. Invalid
   numeric entry fails before conversion. **Mac and Windows baseline passed.** Run
   `lotto_checks` and `LottoTests`, then inspect representative generated output.
2. **AC-02 — Interaction/lifecycle:** exclusive choices, numeric entry,
   Generate, complete result viewing, Save confirmation/failure recovery and
   Quit work; startup/Generate do not write the tips file. **Mac ANSI/Swing
   checks passed; Windows legacy manual and shared/native functional checks
   passed. Shared GTK live checks passed; Windows resize/mouse remains unclaimed.**
   Use `WINDOWS-CHECKLIST.txt`.
3. **AC-03 — Beta 3 compatibility:** compile, assemble, link and run cREXX with
   the unchanged released Beta 3 product. `rxc -v` must report
   `crexx-1.0.0-beta.3`. **Mac and Windows baseline passed.** The
   transport branch's development toolchain is not a substitute for this gate.
4. **AC-04 — Windows delivery:** quoted-path build/run/test commands, dependencies,
   controls, output locations and troubleshooting are provided. **Instructions
   prepared; native build scripts/execution and receipt passed.**
5. **AC-05 — Final package:** package only intended source, application artifacts,
   instructions, license and accurate receipt, then unpack/rebuild/run in a clean
   directory with spaces. **Original Mac package passed; Windows package open.**
   This Git branch intentionally carries no generated binaries or email text.
6. **AC-06 — Optional GTK GUI:** preserve the existing Beta 3 plugin/API, build
   the matching native provider, pass widget smoke and interactive checks, then
   test its dependency bundle without a development GTK directory on PATH.
   **Windows provider build, shared/native interaction and runtime closure
   passed. Final extracted recipient/signing gate open.**
7. **AC-07 — Source transport:** preserve tested source and API tags, publish only
   example-related files on `codex/franz-lotto-windows`, and identify the exact
   commit/base for the Windows agent. Source identity and content audit are the
   transfer checks; the remote commit is verified after push.
8. **AC-08 — Shared UI controls:** one Level G Lotto component/view/session runs
   in ANSI and GTK. Its logical choice becomes three mutually exclusive radio
   buttons in GTK, and its integer input becomes a spin control bounded to
   1–50. Existing UI examples and contracts continue to pass. Tests cover
   invalid views/values, selection, edit/commit events, rendering without input
   echo, Save grant/correlation/failure and cleanup. Exercise `rxc`, `rxas`,
   `rxlink` and `rxvm` with the drivers. **Design approved; implemented and focused
   Windows regression/driver gates passed.**
9. **AC-09 — Native demonstration:** provide `Lotto-Console.exe` and
   `Lotto-GTK.exe` using the same Lotto model. Both build and run on Windows;
   the GTK EXE opens directly with its adjacent dependencies. Prove generation,
   Save and clean exit, including a GTK run without the MSYS2 development path.
   **Both native builds and execution passed; final extracted delivery open.**
10. **AC-10 — Recipient package:** publish a verified Drive download containing
    the native demonstrations, source/build instructions, accurate Windows
    receipt, dependency provenance/licenses and checksums. Extract into a fresh
    folder with spaces and verify the packaged controls and native lifecycle.
    Preserve the existing private Drive sharing. **Open.**
11. **AC-11 — Teaching and matched distribution:** retain the validated NetRexx
    example, demonstrate cREXX's two hosts side by side, explain feature/driver
    decoupling and actual native packaging, and obtain/verify the matched signed
    interim Windows distribution after local qualification. **Open.**

## Steps and dependencies

1. **STEP-01 (AC-01, AC-03):** inspect game rules, shipped APIs and exact release
   tools. Complete locally; Beta 3 tag is
   `ae1607b8e145174422cee7f3e73fbcc37a65226c`.
2. **STEP-02 (AC-01, AC-02):** implement separate model/screen layers. Complete;
   the code here is unchanged from the tested preparation.
3. **STEP-03 (AC-01–AC-04):** Windows agent builds/runs focused model, file and
   interactive checks with the released tools. Open. Record commands, versions,
   outcomes and any failure before changing code; avoid overlapping build/test
   commands in one Windows build tree. Baseline and shared/native tests passed;
   physical resize/mouse remains unclaimed.
4. **STEP-04 (AC-04, AC-05):** after native checks, update the recipient receipt,
   rebuild a clean delivery package and verify it. Open on Windows.
5. **STEP-05 (AC-06):** build the GTK provider separately and qualify its complete
   Windows dependency add-on. Open on Windows; no new widget/library API is
   authorized by this transfer.
6. **STEP-06 (AC-07):** copy the existing sources into an isolated managed
   worktree, adapt source-only documentation, audit and push the transport
   branch. Current Mac transfer phase. Expected publication is one source-only
   commit on the branch below; no release promotion or tag is part of this task.
7. **STEP-07 (AC-07):** preserve exact source bytes on Windows checkouts.
   Windows `core.autocrlf=true` converted all 13 manifested code/build inputs
   to CRLF. Their LF-normalized hashes match the qualified delivery. Add an
   example-local `.gitattributes` rule for LF, restore the unchanged LF bytes,
   and rerun the manifest hash audit before publishing the Windows findings.
   This changes checkout policy only; it does not change program logic or APIs.
8. **STEP-08 (AC-08):** agree the shared choice/integer view and Save executor
   contract above before implementation. Extend logical views, host projections
   and focused conformance/driver tests, then author the shared Level G Lotto
   component and tiny driver launchers. Preserve all existing documentation tags.
9. **STEP-09 (AC-08, AC-09):** build the Windows provider, compile/assemble/link
   and run focused regressions, then use the released native packaging route
   to build the console and GTK executables. Reuse unchanged model evidence.
10. **STEP-10 (AC-06, AC-09, AC-10):** collect the complete GTK runtime closure
    and notices, test the native package without a development GTK path, verify
    a clean extracted copy and upload the final private Drive deliverable.
11. **STEP-11 (AC-08, AC-09, AC-11):** verify the current optimiser repair on
    Windows, coordinate the static-library packaging correction with the Mac
    agent, prepare the side-by-side guide and NetRexx app image, then qualify
    the matched signed interim distribution before recipient delivery.

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
