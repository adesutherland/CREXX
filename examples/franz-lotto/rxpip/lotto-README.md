# Franz Marx's Lotto — cREXX console package 0.1.1

Original LottoV41 example: **Franz Marx**. This package is the cREXX ANSI
console adaptation and packaging by **Adrian Sutherland**, based on Franz's
game choices. Franz's original ooRexx source is not included here.

One source package provides an ANSI screen with keyboard and optional mouse
controls. Choose Austrian 6/45, German 6/49 or Euro-Lotto 5/50 plus 2/12,
generate 1–50 tips, page through them and explicitly save the complete result.
This is a recreational programming example; it does not predict lottery draws.

The package contains `lotto.crexx`, `lotto_logic.crexx`, `lotto_tests.crexx`,
this guide and `lotto-LICENSE.txt`. It reuses the complete cREXX **1.0.0 Beta 3**
core, including its `rxconsole` provider. Keep the installed core together.
No GTK, ncurses, C compiler or additional RXPIP dependency is needed. Package
version 0.1.1 is independent of the required cREXX product version.

## Install

Install from rexxpository into a new directory:

```sh
rxpip-crexx install --target="Lotto console" crexx-lotto
cd "Lotto console"
```

Alternatively, unpack a downloaded archive:

```sh
mkdir "Lotto console"
tar -xzf crexx-lotto-0.1.1.rxpkg -C "Lotto console"
cd "Lotto console"
```

Use a foreground terminal, at least 80 columns by 24 rows. RXPIP's current
`run` command captures child output, which prevents this full-screen program
from acquiring its terminal. Install with RXPIP, then build and launch with the
ordinary cREXX tools below. RXPIP's global store is not needed.

## Build and run

With the complete cREXX core's `bin` on PATH:

```sh
crexx -program lotto-app lotto.crexx lotto_logic.crexx
rxvm lotto-app.rxbin
```

The first command builds the application; the second opens its screen. To
exercise the individual tools instead, run each command below in order,
stopping if a command fails:

```sh
rxc -o lotto_logic lotto_logic.crexx
rxas lotto_logic
rxc -o lotto lotto.crexx
rxas lotto
rxlink -o lotto-app lotto lotto_logic
rxvm lotto-app.rxbin
```

The linked application uses the installed core's library and console provider.
The source files remain readable alongside it; no precompiled platform-specific
plugin is included in this package.

On Windows, use Command Prompt or Windows Terminal with `rxc.exe`, `rxas.exe`,
`rxlink.exe` and `rxvm.exe` on PATH. The commands and source names above are
otherwise the same. Quote the full tool path if the core is under Program Files.
Keep the current directory writable for compilation and explicit Save.

## Controls

- Tab / Shift-Tab moves focus; arrows change the focused game choice.
- A / D / E or 1 / 2 / 3 selects Austria / Germany / Euro-Lotto.
- T focuses the tip count. The first digit replaces the previous value;
  subsequent digits append. Backspace edits and Delete clears it.
- Enter in the numeric field generates; Enter on a button activates it.
- G generates; P / N or PageUp / PageDown changes result page.
- S saves after a second S confirms replacement. Q quits.
- Escape or Ctrl-C quits from anywhere. Letter shortcuts are inactive while
  editing the tip count; Tab leaves that field.
- A left mouse press and release on the same displayed control activates it.
  Clicking the count field focuses it. All operations also work by keyboard.

Startup and Generate write no result file. Invalid input keeps the previous
results. Save replaces `Lotto-Tipps-cREXX.txt` in the current directory only
after confirmation and includes tips on every page. Save failure leaves the
results available. Changing the selected game does not relabel an existing
draw. Normal exit restores terminal modes, cursor and the previous screen.

Mouse reports depend on the terminal and any SSH/multiplexer forwarding.
Resize below 80x24 shows a resize prompt; enlarge it to continue. Redirected
stdin/stdout or an IDE output pane is not a usable console.

## Model and file checks

Run in a disposable writable directory containing these sources. The checks
create/replace `lotto-test-output.txt`; the directory `missing-lotto-folder`
must not exist because it exercises Save failure.

```sh
rxc -o lotto_tests lotto_tests.crexx
rxas lotto_tests
rxlink -o lotto-checks lotto_tests lotto_logic
rxvm lotto-checks.rxbin
```

Expected output:

```text
PASS: cREXX Lotto (3825 tips; validation, save, replace, failure)
```

These checks cover the model and file operations. Real foreground-terminal
keyboard, mouse and restoration checks are maintained in the CREXX repository
at `examples/franz-lotto/tests/ansi_package_checks.py`. The package's own
native Windows terminal behavior remains a separate platform qualification.

MIT licence: `lotto-LICENSE.txt`. Sources and package builder:
[CREXX Lotto example](https://github.com/adesutherland/CREXX/tree/develop/examples/franz-lotto).
