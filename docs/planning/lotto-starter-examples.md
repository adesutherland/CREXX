# Lotto starter examples

## Vision and intended outcomes

Provide readable Lotto teaching examples based on Franz Marx's LottoV41:
a procedural cREXX ANSI terminal form compatible with released
`v1.0.0-beta.3`, and a NetRexx Swing desktop window. Both support Austrian
6/45, German 6/49 and Euro-Lotto 5/50 + 2/12, accept 1–50 tips, sort unique
numbers within each pool, display every result and save only on explicit
request. Source and documented build/run commands remain the maintained inputs.

The optional cREXX GTK example uses the existing provider and a build kit
pinned to Beta 3. Its Windows provider/dependency-bundle gate is distinct from
the later shared UI/native examples documented in
[the Windows handover](../../examples/franz-lotto/WINDOWS-HANDOVER.md).
Do not use those later results to infer a fresh Windows RXPIP installation pass.

The console package is **crexx-lotto 0.1.1**, published as `SOURCE/crexx` on
[rexxpository](https://www.rexxpository.org/cgi-bin/rxpip_server.rexx?action=PKG&name=crexx-lotto).
It contains the procedural application, model, model checks, guide and MIT
licence. Franz Marx owns the original LottoV41 attribution; Adrian Sutherland
provided the cREXX console adaptation and packaging. The private original
ooRexx attachment is not distributed. Version 0.1.1 corrected attribution
without changing executable source.

RXPIP installs the source; the ordinary cREXX tools build and launch it in a
foreground terminal. RXPIP's captured `run` path cannot provide this screen's
terminal ownership. No GTK, ncurses, extra native provider or core change is
required beyond the complete Beta 3 core and its `rxconsole` provider.

Console artifact publication was authorized and completed. Announcement drafts
were prepared for Adrian to send; preparing or integrating these sources does
not authorize sending mail, posting announcements or uploading another version.

## Acceptance criteria

Statuses preserve the scope of the completed preparation. Native Windows
qualification of the installed RXPIP console package remains open; it is not
implied by macOS checks or the separate shared UI/native Windows examples.

| ID | Observable contract and verification | Status |
| --- | --- | --- |
| AC-01 | Both original examples implement all three games and integer tip counts 1–50; deterministic model checks cover bounds, invalid inputs, sorted unique pools and independent inspection. | Done |
| AC-02 | Real ANSI/Swing controls support exclusive selection, editing, repeated generation, viewing all tips, explicit Save and clean Quit. Invalid input and Save failure preserve usability; startup/generation never save implicitly. Verify terminal restoration and actual UI controls. | Done at the original local scope |
| AC-03 | Compile, assemble, link and execute the procedural ANSI example with unchanged released Beta 3 tools/libraries, including the shipped console provider. Development-tool success is separate. | Done |
| AC-04 | Windows recipes document prerequisites, quoted paths, commands, controls, output and troubleshooting. Native execution must be checked on Windows; unavailable proof stays explicit. | Recipes done; installed RXPIP package's Windows terminal gate open |
| AC-05 | Prepare and independently unpack the original email ZIP with intended sources, application bytecode/classes, recipes, licence and instructions; rebuild/run from that copy and provide a covering note. | Original delivery preparation done |
| AC-06 | Prepare the optional GTK Lotto example and Beta 3 provider source kit; compile and exercise real widgets locally. A qualified Windows GUI add-on additionally needs a native build and clean dependency closure. | Local preparation done; original Windows provider-kit gate open |
| AC-07 | A reproducible flat RXPIP archive contains exactly the maintained application, model, model checks, guide and licence. Inspect regular members, permissions and source/tag preservation; require only the complete installed core. | Done |
| AC-08 | Qualify temporary-server installation into a path with spaces and released-tool compilation/execution. Exercise foreground keyboard/mouse input, validation, paging, resize, explicit Save/failure recovery and terminal restoration. | Done on macOS; Windows limit retained |
| AC-09 | Supply a reviewable local archive and exact install/build/run instructions before separately authorized publication. Do not infer permission to publish or send a message from preparation. | Done |
| AC-10 | Publish the authorized console artifact and attribution correction under `adriansutherland`; verify public metadata and unchanged executable inputs. Credit Franz first, retain Adrian's adaptation credit and preserve MIT terms. | Done: public 0.1.1 |
| AC-11 | Independently download/install the public artifact with anonymous client state in a fresh directory with spaces; run released-tool model and foreground lifecycle checks. | Done on macOS; native Windows package proof open |
| AC-12 | Prepare and reread separate Mike and new-topic RexxLA drafts with attribution, version, public link and commands; leave delivery to Adrian. | Draft preparation done; no sending authority |

## Implementation steps

| ID | Work, dependencies and criteria | Status |
| --- | --- | --- |
| STEP-01 | Inspect original game rules, released Beta 3 APIs and NetRexx/JDK tools (AC-01, AC-03). | Done |
| STEP-02 | Implement separate generation logic and small screen layers over existing APIs; retain teaching-example scope (AC-01, AC-02); depends on STEP-01. | Done |
| STEP-03 | Exercise models, files and real input/UI; inspect Windows recipes with released tools (AC-01–AC-04); depends on STEP-02. | Local checks done; Windows package gate open |
| STEP-04 | Prepare original ZIP, Windows instructions and covering note; verify clean extraction (AC-04, AC-05); depends on STEP-03's local checks. | Done at original delivery scope |
| STEP-05 | Prepare optional GTK example and pinned provider kit, local widget smoke and Windows handover (AC-06); depends on STEP-02. | Local preparation done; original Windows provider/dependency gate open |
| STEP-06 | Add deterministic source-only package builder and guide without changing game/UI behavior (AC-07); depends on the maintained procedural sources. | Done |
| STEP-07 | Qualify disposable installation, full toolchain and foreground interaction using the shared console PTY harness (AC-08); depends on STEP-06. | Done on macOS |
| STEP-08 | Provide artifact and instructions with publication/platform boundaries (AC-09); depends on STEP-07. | Done |
| STEP-09 | Publish separately authorized console package and attribution correction with accurate metadata (AC-10); depends on STEP-08 and Adrian's publication authority. | Done: 0.1.1 |
| STEP-10 | Verify anonymous live installation and released-tool model/terminal behavior (AC-11); depends on STEP-09. | Done on macOS; Windows package qualification remains open |
| STEP-11 | Prepare attribution-first Mike and new-topic RexxLA drafts for Adrian (AC-12); depends on STEP-10. | Done at draft scope |

## Maintained packaging and checks

From the repository root, create a new archive with:

```sh
python3 examples/franz-lotto/package-rxpip.py OUTPUT.rxpkg
```

The builder never overwrites an existing destination or publishes it. The five
members are `lotto.crexx`, `lotto_logic.crexx`, `lotto_tests.crexx`,
`lotto-README.md` and `lotto-LICENSE.txt`. Only the source comment's relative
licence link changes for the flat layout; executable code and RexxDoc tags stay
with their maintained source owners. Archive ordering, permissions and timestamps
are deterministic.

Inspect/extract the archive into a fresh disposable installation directory, then
run the POSIX foreground-terminal checks with an explicit complete core:

```sh
python3 examples/franz-lotto/tests/ansi_package_checks.py \
  --install "FRESH_INSTALL_DIRECTORY" --bin "COMPLETE_CORE_BIN"
```

This exercises individual `rxc`/`rxas`/`rxlink`/`rxvm` commands, 3,825 generated
tips and model/file failures, redirected-terminal rejection, real keyboard and
mouse control, paging, resize, Save confirmation/failure/recovery, game identity,
Ctrl-C and terminal restoration. It also checks the documented `crexx -program`
build followed by a separate foreground `rxvm` launch. The shared console PTY
harness owns terminal-session mechanics; the Lotto check owns application
expectations. Use disposable directories because the model and terminal checks
create output files there.

See the [package guide](../../examples/franz-lotto/rxpip/lotto-README.md) for
install/build/run commands, controls and platform boundaries. Beta 3 compatibility
uses the released product; integration checks may additionally select a current
Debug core. Keep the claims separate. Broader compiler, sanitizer and GUI/platform
qualification are not implied by this packaging increment.
