# Shared Lotto application

`lotto_surface.crexx` owns the feature, commands, logical view and Save workflow.
`../crexx/lotto_logic.crexx` supplies the unchanged game model. The launchers only
choose a host: `lotto_console.crexx` uses ANSI, `lotto_gtk.crexx` uses GTK, and
`lotto_line.crexx` uses the scripted line host used by focused tests.

Read [the illustrated guide](../SHARED-UI-GUIDE.md) for the OO separation and
native packaging. The original procedural example in `../crexx` remains the
released Beta 3 compatibility example. These shared controls require the newer
UI libraries, optimiser correction and native-provider packaging corrections;
do not mix their archives with a Beta 3 installation.

## Windows build

Use a Visual Studio x64 developer PowerShell, the matched cREXX development
core/UI prefix, and GTK 3. The prefix must contain both UI bytecode modules,
canonical console/GTK static archives, the dependency helper and the GTK native
manifest/data. `BUILD-WINDOWS.ps1` checks these inputs before compiling.

```powershell
& .\BUILD-WINDOWS.ps1 -CorePrefix 'C:\Tools\Matched cREXX UI' `
    -OutputDirectory 'C:\Demos\Franz Lotto native'
```

A local GTK SDK data override is available with `-GtkDataPrefix
'C:\msys64\ucrt64'`. It copies only schemas, icons, MIME/GTK data and pixbuf
modules; it never copies the entire development SDK. The native dependency
manifest supplies the DLL closure. Keep the resulting EXEs, adjacent DLLs,
`share` and `lib` together when relocating the application. Retain dependency
licenses and package provenance when distributing it.

For a source checkout configured with `ENABLE_GTK=ON`, finish `stage-product`
before preparing `ui_functional_tests`, `example_franz_lotto_artifacts` and
`example_text_inspector_artifacts`. Run builds and tests sequentially in a
Windows build tree. A generated-library publication lock should be retried on
its exact target with `--parallel 1` after other build/test processes have exited.

The GTK launcher uses the Windows subsystem, so double-clicking it opens only
the desktop window. The console launcher runs in an ordinary terminal. Both
embed the cREXX VM and application; running them requires neither a cREXX
installation nor the development GTK directory on `PATH`.

## Controls and Save

Choose Austria, Germany or Euro; enter 1–50; activate Generate. Previous/Next
show eight tips per page. Save writes all generated tips, with their generated
game identity, after choosing a path and confirming replacement. Cancelling
keeps results and the existing file. Failed writes report failure and retain
results; the bounded local executor does not promise atomic replacement.

In the console, Tab/Shift-Tab move focus, arrows change an exclusive choice,
typing edits the count, and Enter commits/activates. `g`, `s`, `p`, `n` and `q`
invoke commands outside numeric editing. Escape cancels a dialog and Ctrl-C
requests close. In GTK, use radio buttons, the spin control and named buttons.

`lotto_surface_tests.crexx` covers the shared Save effect chain in both
optimisation modes. `lotto_gtk_tests.crexx` drives the real GTK feature through a
test-only provider, including paging, invalid input, cancellation, replacement
and driver reuse. That test provider and its EXE are excluded from the recipient
application folder.
