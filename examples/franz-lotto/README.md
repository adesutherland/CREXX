# Franz's Lotto teaching examples

Two small examples implement the game choices from Franz Marx's LottoV41:
Austrian 6/45, German 6/49 and Euro-Lotto 5/50 plus 2/12. Each accepts 1–50
tips, draws without duplicate balls within each pool, sorts the results and
writes a file only on explicit Save. Distinct tips may repeat by chance; the
two Euro pools are independent. These are recreational programming examples.

- [cREXX ANSI](crexx/lotto.crexx): procedural Level B screen using the shipped
  `rxconsole` primitives, compatible with the released **1.0.0 Beta 3** tools.
- [NetRexx Swing](netrexx/LottoWindow.nrx): desktop window with three exclusive
  radio buttons, numeric entry and a scrollable result pane.
- [Optional cREXX GTK](crexx/lotto_gui.crexx): desktop alternative using the
  existing `gui` plugin and a single-choice combo. The [provider build kit](crexx/gui-provider/)
  contains unchanged Beta 3 source/headers; a native plugin and GTK 3 runtime
  closure must be built separately for Windows.

This is a **source-only Windows transport**, not a release-package payload.
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
Native Windows builds, interaction and the GTK dependency bundle remain open.

These standalone examples are not registered in the aggregate CMake/CTest
product targets on this transport branch. No production language/API change is
part of the transfer. MIT license: [LICENSE.txt](LICENSE.txt).
