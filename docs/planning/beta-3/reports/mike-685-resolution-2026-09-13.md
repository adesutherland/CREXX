# Issue 685: explicit source inputs and optional project builds

Resolution: **working as designed; documentation clarified**. For the reported
program, list both source files:

```sh
crexx usemod.crexx libmod.crexx
```

The ordinary driver compiles the listed sources and runs their bytecode
together. An import supplies compile-time declarations; it does not add another
source file to the build. The omitted runtime module explains the original
`FUNCTION_NOT_FOUND` result. The original fixture and investigation remain in
[the 11 September report](mike-reports-2026-09-11.md).

`--program output sources...` and `--library output sources...` are existing,
optional workflows for larger or repeatedly built projects. They track
dependencies, reuse unchanged work and publish a named linked output. Both
build without executing and retain explicit source membership. The ordinary
compile-and-run path recompiles its listed sources unless `--nocompile` is
selected; that option reuses bytecode without checking source freshness.

Automatic binary-library discovery serves separately built dependencies. The
library must be visible through a binary import root, and runtime autoload uses
the exact package hint recorded when the compiler selects that binary. `-l`
can supply an existing runtime library explicitly. Neither path builds missing
source modules automatically.

## Focused command validation

On 13 September, nine cases ran in separate scratch directories in both
optimized and unoptimized modes: 18 executions with matching outcomes. The
fixture preserves the original nested-call shape.

`a.crexx`:

```rexx
options levelb
import b
say b..greet()
exit 0
```

`b.crexx`:

```rexx
options levelb
namespace b expose greet
greet: procedure = .string
  return helper("fixed")
helper: procedure = .string
  arg y = .string
  return "hello from B with " || y
```

Success requires exact program output `hello from B with fixed` and exit zero.
Prebuilt cases first compile `b` with `crexx -noexec b`; binary-only cases then
remove its source from that private fixture directory.

| Command and setup | Result in both modes |
| --- | --- |
| `crexx a`, sources only | Exit 11, `FUNCTION_NOT_FOUND` for `b.greet`; no `b.rxbin` built. |
| `crexx a b`, sources only | Success. |
| `crexx b a`, sources only | Success; `b` is a library and `a` is the main program. |
| `crexx a`, with prebuilt sibling `b.rxbin` | Exit 11, `FUNCTION_NOT_FOUND`. |
| `crexx -i . a`, with `b.crexx` and prebuilt `b.rxbin` | Success; exact `b` autoload hint emitted. |
| `crexx -l ./b.rxbin a`, with prebuilt `b.rxbin` | Success; compiles `a` and supplies the existing library. |
| `crexx -l b a`, with only a local `b.rxbin` | Exit 255; the bare name resolves under `CREXX_HOME/bin`. |
| `crexx a`, with only `a.crexx` and sibling `b.rxbin` | Exit 2; compilation cannot resolve `b..greet`. |
| `crexx -i . a`, with only `a.crexx` and sibling `b.rxbin` | Success; exact `b` autoload hint emitted. |

In the explicitly configured binary-root case, `b.rxbin` is selected even
while `b.crexx` remains present. Its consumer assembly contains
`.meta "b.greet"=".autoload" "b"`. Source presence alone therefore does not
establish which artifact supplied the imported callable.

The local toolchain was the retained macOS Debug build used during the preceding
QA. Its `rxc -v` stamp is `crexx-1.0.0-beta.3+local.gfd866d9ef770.dirty`;
the binaries below identify this focused execution, rather than an inferred
exact-source-SHA or new cross-platform qualification claim.

| Tool | SHA-256 |
| --- | --- |
| `crexx` | `a7767658485a0a37a948d83aa4d213c0bebe4d1b4d703a99a8dd0304b24b6be4` |
| `rxc` | `99a58e3f0f488b17a5cb4906db1355cb1a52497f6ba1aaf32cca7d2a362fd16a` |
| `rxas` | `0301efdd5e726e2e0eb1ceeb08a8e9c0e0559cc7e03e851827f3e84ef63b0d4f` |
| `rxvme` | `56a08268d36f38ea9e0e18d62800fe77f925bf8035e9e77f62cc842afcb5b6b3` |

Full argv, output logs, fixtures and `results.json` are retained locally under
`/private/tmp/crexx685-command-validation-fosyyiub/`. The existing
`rxbin_autoload_contract` and `crexx_project_build_contract` retain permanent
coverage of the package and incremental-build contracts.

## Documentation disposition

The driver guide, toolchain overview and worked import example now lead with
the direct multi-source command. Project and library modes follow as optional
concepts for larger projects. The roadmap records completed discovery/build
capabilities and treats possible future incremental defaults, automatic source
membership and improved diagnostics as separate, unselected considerations.
This resolution changes documentation only; no code, test/build input or
current command behavior changes.
