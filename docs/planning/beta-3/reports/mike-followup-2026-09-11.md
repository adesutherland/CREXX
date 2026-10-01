# Mike's follow-up sources and process question — 11 September 2026

The later emails supplied `crexx_bug_report_claude.zip` and
`crexx_prolog.zip`, and asked about the disabled `process` plugin as a route
to Python integration. The source archives change the status of two findings
from the [initial review](mike-reports-2026-09-11.md): both now have independent
reproductions. The process question concerns selection of the current API.

Validation used the same macOS ARM64 Debug product code at
`beee17322956a86cbdb3fcbf8fc0db6d560fc02b`; intervening local commits contain
documentation only. The [evidence JSON](mike-followup-2026-09-11-evidence.json)
retains commands, outputs, source/archive hashes, the reduced namespace fixture
and the tested Python example. Original archives are retained outside the
repository under
`/Users/adrian/Documents/Codex/email-drafts/mike-crexx-2026-09-11/`.

## Revised report 2: same-namespace source interference

[#686](https://github.com/adesutherland/CREXX/issues/686) now records a
reproduced failure, not a missing-source investigation. The large supplied
file compiles in an isolated directory, but adding its byte-identical sibling
with the same namespace produces 47 type/conversion/object/array errors.

The reporter's proposed tiny example has a separate missing import: its
`.stem()` call fails both alone and with the sibling. Adding `import rxfnsb`
makes both versions pass. That tiny example does not demonstrate the claimed
namespace failure.

A different nine-line example using an exposed `.stem` global does reproduce
the interference. It compiles alone; adding an identical second source file
causes `TYPE_MISMATCH` at `.stem()` and `BAD_CONVERSION` at the indexed return.
The fixture is retained in #686 and the evidence JSON. The exact resolver/type
mechanism and appropriate duplicate-provider handling remain to be diagnosed.
This does not support a ban on all same-namespace source fragments.

## Report 5: compiler optimization divergence

[#689](https://github.com/adesutherland/CREXX/issues/689) now records a
reproduced compiler optimization defect. The supplied query is
`functor(foo(a,b,c),F,A).`, followed by `true.` and `halt.` as in the archive.

| Source | Compiler optimization | Assembler optimization | Result |
| --- | --- | --- | --- |
| Original | on | on | `F = foo, A = 0` |
| Original | off | off | `F = foo, A = 3` |
| Original | on | off | `F = foo, A = 0` |
| Original | off | on | `F = foo, A = 3` |
| Split workaround | on | on | `F = foo, A = 3` |
| Split workaround | off | off | `F = foo, A = 3` |

All runs exit zero, so process status alone would miss the defect. The stage
comparison localizes the divergence to compiler optimization, without proving
which transform is responsible. The original/split source diff changes the
namespace, preinitializations and Arity/Functor ordering as well as extracting
helpers. Reduction must control those differences. No compiler patch or
optimizer exclusion was made during this follow-up review.

## Process plugin, tasks and Python

[#692](https://github.com/adesutherland/CREXX/issues/692) records this routing
question, the complete Python/CREXX source pair and the wayfinding guidance.

The `process` entry in `lib/plugins/CMakeLists.txt` is commented out; it is
not selected on Windows either. The source's POSIX branches do not establish
current build selection, support or product qualification.

The active runtime includes `interpreter/rxspawn.c` and
`interpreter/rxvmchannel_child.c`. `ADDRESS` adapts input/output/error capture
onto those maintained providers. The tested example uses
`address crexx "run :argv[]" input request output reply error errors` to run
Python directly, preserving argument boundaries. With the attached worker it
prints `status: 0` and `python:hello from CREXX`, without a process plugin.
The argv form is also documented in the reporter's `48ebc1f610a9` source tree;
this review did not execute that build on Linux.

Tasks are another current surface: `.taskpool.local(...)` and
`.taskpool.process(...)` execute CREXX work on threads or isolated worker VMs.
Level G supplies task declarations and `DO PARALLEL`; Level B exposes the
underlying classes. Running an external Python executable and selecting a
CREXX task pool are distinct operations. The old handle-based plugin is not
needed for the demonstrated integration, but full one-to-one equivalence with
every historical `process*` function has not been established.

## Distributed Prolog example

The supplied three-source program builds through the existing explicit project
path:

```sh
crexx --program combined caller_example.crexx prolog.crexx crexxcallback.crexx
rxvm combined.rxbin
```

The run gives `factorial(6) = 720`, `X = 3628800`, two grandchildren, the
expected failing/reset queries, the greeting callback and the doubling callback
result 42. This validates the supplied demonstration, not all Prolog semantics.
Its build notes should describe explicit dependency loading as normal packaging
and distinguish older unverified/fixed comments from current findings.

## Delivery and scope

A detailed email and a standalone Claude wayfinding guide are prepared for
Mike, with the Python source pair. They direct Claude to `AGENTS.md`, current
architecture/language references, active CMake targets, real examples and
focused output-checked tests. They explicitly preserve the two reproduced
compiler investigations alongside the usage/documentation corrections.

The guide maps all eight issues (#685–#692) and asks Claude to append Linux
retests, reductions and proposed fixes directly to the existing issues when
Mike has authorized GitHub access. It supplies an evidence template and keeps
observations separate from hypotheses, workarounds and approval-dependent
design proposals. Without access, Claude can prepare the same comment for
Mike to post.

This is a follow-up investigation and draft preparation. Neither compiler
defect is repaired or release-qualified here. No Linux, sanitizer, full Debug
or hosted QA pass is claimed, and the email has not been sent.
