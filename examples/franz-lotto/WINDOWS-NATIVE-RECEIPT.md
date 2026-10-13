# Windows native Lotto qualification — 13 October 2026

The shared cREXX console and GTK applications build and run as Windows EXEs.
The NetRexx Swing application also has a Windows launcher and bundled Java
runtime. These are unsigned engineering images. Signed interim distribution
qualification and recipient publication remain separate, open delivery gates.

## Source and tools

- Isolated branch: `codex/franz-lotto-windows`. Engineering producer BUILDINFO:
  `88a2c060972a47959e4d825a21cb4ec7b0a61096`, local channel, dirty UI overlay.
  It includes `fcb361b6e`, optimiser repair `a415090dc`, console static staging,
  the normal core dependency helper and partial DLL collection correction.
- MSVC x64 14.44, Ninja, CMake 4.3.3; GTK 3.24.52 from MSYS2 UCRT64.
  The installed released Beta 3 product was preserved. Its unchanged procedural
  example compatibility evidence remains in `WINDOWS-HANDOVER.md`.
- NetRexx 5.10-GA source compiler, portable Temurin JDK 21.0.12.1+1, `jpackage`.
  Compiled application execution needs no NetRexx runtime JAR or separate JDK.
- Both native build scripts ran successfully into fresh directories containing
  spaces. The cREXX recipe uses the normal source-project compiler/assembler/
  linker/packer path and explicit UI module closure. No temporary helper or
  manual DLL pre-staging was used for this build.

## Results

| Gate | Result |
| --- | --- |
| Current compiler structured-exit regression | All four compiler/assembler optimisation combinations passed |
| Shared views, grants/writes, dialogs, catalogue, line host, Lotto and Text Inspector | All 27 focused gates passed after the GTK correction below; 21 unaffected current results retained, six GTK gates rerun |
| Queued GTK input regression | Old implementation reproduced `edit:17 commit:10`; corrected implementation passes opt/noopt, silent projection and two driver lifetimes |
| Fresh provider collector | Empty output directory, manifest version 1, all 77 declared hashes verified; 61 DLLs, provider and 15 import archives |
| Console static closure | Canonical and historical archive SHA-256 match: `8a660be1bec8c1befc6228c83876ad4c38197dd0df3a179976688f29b2226f93` |
| Fresh native console | System-only PATH, empty CREXX_HOME; Euro/50, chooser/confirmation, 51 saved records including tip 50, Quit exit 0 and terminal restoration |
| Prior full native console interaction | Paging to 49–50, invalid 51, Save cancellation, replacement, generated-game identity, write failure recovery and Ctrl-C/Quit passed |
| Fresh native GTK feature integration | System-only PATH, empty CREXX_HOME/default data lookup; real controls, paging, invalid input, Save/cancel/replacement, generated identity and reuse passed without warnings |
| Live GTK desktop | Radio selection, count 50, generation, all seven pages, real chooser cancellation/results retained and Quit passed; actual screenshot retained |
| NetRexx recipe | Fresh source build, 3,825-tip model checks, Swing controls/Save/recovery/Quit tests and app-image generation passed |
| Native NetRexx live desktop | System-only PATH; Euro/50, real chooser, 51 saved records and Quit passed |
| Original source identity | All 13 exact manifested hashes passed after restoring LF; original program logic unchanged |
| RexxDoc coverage | Existing UI blocks/parameter/return tags preserved or increased |

The GTK fix defers choice/integer projection while native mailbox facts remain.
This prevents an earlier model view from overwriting a later captured user edit.
The deterministic test uses actual GTK signals and an owned, same-priority idle
activation between owner turns. It changes neither the session nor event API.

The portable GTK folder contains adjacent runtime DLLs and separate schemas,
icons, MIME/GTK data, relative pixbuf loader cache/modules and notices. The new
manifest includes the loader modules and SVG/XML dependencies. Build metadata,
test-only providers/EXEs and generated tips are excluded from recipient apps.

## Engineering EXE identities

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| Lotto-Console.exe | 8,501,248 | `4ff052b3c70773e7a5c165f90d22d287a44cf511a928d063a090a910f4cb204e` |
| Lotto-GTK.exe | 7,781,376 | `b5fecbfd62ece44038074ce814838b4001f043ae330d514a86c7446e6ae99a1f` |

## Remaining distribution gates and limits

The core packaging CI exposed two helper problems on Windows: Unicode narrow
arguments fail the unchanged acceptance test, and MinGW needs its C++ runtime
closure. A same-source probe confirmed missing-DLL exit `0xC0000135`; supplying
the three matched GNU runtime DLLs made ASCII fingerprinting pass, then exposed
the independent Unicode failure. The Mac agent is preparing the narrow fixes.
No successful matched core/plugin/manager signing input exists for this UI
overlay yet. Do not sign an older snapshot to substitute for it.

Physical Windows terminal mouse/viewport resizing remains an unclaimed manual
check. The GTK UI automation helper could click the chooser but timed out typing
into its owned modal; full native Save behavior is proven by real-widget
integration rather than that physical keyboard path. The local write executor
reports failures and closes streams, but does not promise atomic replacement.

Evidence logs are retained under `Documents/Franz Lotto Windows 20261012`,
including `matched-product-final.log`, `matched-shared-ui-ctest.log`,
`gtk-queued-edit-before.log`, `gtk-queued-edit-after.log`,
`native-build-script-test.log`, `native-matched-console-capture.log`,
`native-matched-gtk-test.log` and `netrexx-build-script-test.log`.
