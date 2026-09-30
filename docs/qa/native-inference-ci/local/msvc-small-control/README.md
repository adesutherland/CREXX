# CI-F11: Windows SDK macro collision in generation

MSVC job `104481333824` in run `34998653921` reaches `bridge.cpp` after compiling
CUDA and fails at `generation.h:39` with C2632 (`char` followed by `char`).
The Windows SDK's `rpcndr.h` defines `small` as `char`, colliding with our local
detokenizer buffer. Microsoft retains the definition in its
[SDK header source](https://github.com/microsoft/win32metadata/blob/main/generation/WinSDK/RecompiledIdlHeaders/shared/rpcndr.h).

The identical actual bridge translation unit fails with `-Dsmall=char` before
renaming the buffer to `token_piece`, and passes afterwards. `command.json`,
`before.log`, `after.log` and `result.json` retain this causal local control.
The actual bridge target also builds in normal Debug and maintained Apple ASan;
their runner logs and environment are retained here. The rename changes no
generation behavior, buffer size, API or model setting. The ordinary MSVC
provider build is the permanent target-platform regression/closure check.

This repair serves CI-AC-03/07 in the existing pipeline plan. Targeted MSVC
object compilation is the next check; GPU delivery and exact-candidate gates
remain open. This does not authorize CI-D01's proposed packaging/toolchain
changes or a new broad QA cycle.
