# Franz's Lotto teaching examples

These examples implement the game choices from Franz Marx's LottoV41:
Austrian 6/45, German 6/49 and Euro-Lotto 5/50 plus 2/12. Each accepts 1–50
tips, draws without duplicate balls within each pool, sorts the results and
writes a file only on explicit Save. Distinct tips may repeat by chance; the
two Euro pools are independent. These are recreational programming examples.

The procedural ANSI application is also available as one source-only RXPIP
package, `crexx-lotto` 0.1.1. It includes the application, model, checks,
Markdown instructions and MIT licence, and uses the complete Beta 3 core's
`rxconsole` provider. See the [package guide](rxpip/lotto-README.md). Generate
a new archive with `python3 examples/franz-lotto/package-rxpip.py OUTPUT.rxpkg`
from the repository root. This prepares a local artifact; it does not publish
it. RXPIP installation is supported, with foreground execution through the
ordinary cREXX tools because RXPIP's current `run` captures child output.
The package prominently credits Franz Marx for the original LottoV41 example;
Adrian Sutherland provided the cREXX console adaptation and package preparation.

- [cREXX ANSI](crexx/lotto.crexx): procedural Level B screen using the shipped
  `rxconsole` primitives, compatible with the released **1.0.0 Beta 3** tools.
- [NetRexx Swing](netrexx/LottoWindow.nrx): desktop window with three exclusive
  radio buttons, numeric entry and a scrollable result pane.
- [Optional cREXX GTK](crexx/lotto_gui.crexx): desktop alternative using the
  existing `gui` plugin and a single-choice combo. The [provider build kit](crexx/gui-provider)
  contains unchanged Beta 3 source/headers; a native plugin and GTK 3 runtime
  closure must be built separately for Windows.
- [Shared cREXX UI](surface/README.md): one Level G component, model, session
  and Save workflow projected through ANSI and GTK. Three exclusive choices
  become GTK radio buttons; a bounded integer becomes a spin control. This
  extension uses the current development libraries and optimiser repair.
- [Illustrated OO/native guide](SHARED-UI-GUIDE.md): real Windows screenshots,
  console capture and a comparison with the NetRexx Swing example.

Git carries the source and documentation, rather than release binaries.
Generated `.rxbin`, `.rxas`, `.class`, native plugins and dependency bundles are
not committed. Start with [WINDOWS-HANDOVER.md](WINDOWS-HANDOVER.md), build the
sources, and perform the [native checklist](WINDOWS-CHECKLIST.txt).

Detailed controls, build/test commands and troubleshooting are in
[cREXX Windows instructions](crexx/WINDOWS.txt),
[NetRexx Windows instructions](netrexx/WINDOWS.txt) and
[GTK provider notes](crexx/gui-provider/WINDOWS-BUILD.txt).
Do not assume the current development compiler establishes Beta 3 compatibility;
that check uses the exact released tools and libraries.

The [retained Mac receipt](TEST-RECEIPT.txt) records previous model, ANSI, Swing,
GTK widget and clean-package checks. [SOURCE-SHA256.txt](SOURCE-SHA256.txt)
identifies the unchanged code/build inputs copied from that tested preparation.
The current [Windows native receipt](WINDOWS-NATIVE-RECEIPT.md) records the
shared/native qualification separately from that historical Mac baseline.

The shared examples are registered in CMake/CTest with opt/noopt Save and GTK
tests. The native build scripts produce `Lotto-Console.exe`, `Lotto-GTK.exe` and
a self-contained NetRexx Java app image; see `surface/BUILD-WINDOWS.ps1` and
`netrexx/BUILD-NATIVE-WINDOWS.ps1`. The original procedural sources remain
unchanged. The shared UI surface is an approved library extension; this branch
does not introduce language syntax. MIT license: [LICENSE.txt](LICENSE.txt).
