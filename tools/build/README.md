# rxBuild Level L reader

This example demonstrates a small build-plan language described by
[`grammar/build.levell`](grammar/build.levell).

The user-oriented quick guide is in [RXBUILD.md](RXBUILD.md).

The complete lifecycle is documented in
[`docs/ai-context/RXBUILD_BUILD_PROCESS.md`](../../docs/ai-context/RXBUILD_BUILD_PROCESS.md).

The CMake build packages [`build_reader.crexx`](build_reader.crexx) as the
`rxbuild` command. `rxbuild` loads the generated `rxbuildgrammar` module,
reads one `.build` file, reports parser diagnostics, prints each recognized
top-level operation, and performs the declared grammar compilation, reader
compilation, and program execution. The `.build` file is a runtime input, not
an `.rxpp` wrapper.

[`example.build`](example.build) exercises `COMPILE GRAMMAR`, `COMPILE READER`,
and `RUN`, including their supported attributes. The final `RUN` invokes the
small generated demo program in [`build_demo.crexx`](build_demo.crexx).
[`simple.build`](simple.build) additionally demonstrates `COMPILE PROGRAM`,
which creates the program it later runs.
It also demonstrates string variables and `${NAME}` substitution.

[`selection.build`](selection.build) demonstrates directory selection and
iteration. `FILES IN ... EXTENSION ... BASENAMES` returns sorted
extensionless names. Adding `RECURSIVE` preserves subdirectory paths relative
to the selected directory. `DO OVER ... AS ... END;` expands its body once per
selected item and mirrors nested output directories.

## Build and run

Configure/build the command from the repository root:

```powershell
cmake --build cmake-build-release --target rxbuild --parallel 1

& .\cmake-build-release\bin\rxbuild.exe `
  tools\build\example.build
```

The reader prints one `RXBUILD` line for each top-level statement. Generated
files stay under the CMake build tree.

To exercise diagnostics, run `rxbuild` against a malformed `.build` file. The
command returns a non-zero status and reports `RXBUILD_PARSE`.

Undefined substitutions are rejected before compilation; see
[`variables-invalid.build`](variables-invalid.build).
