# Native mainframe ADDRESS

This static provider connects the existing Level B and Classic ADDRESS
protocol to the maintained Mainframe ELF SDK command ABI. CMS builds keep
the CMS personality on native CMS and PDOS; TSO builds keep TSO. A separately
queried PDOS environment is optional. Application registration and a later
PDOS-specific combined provider remain outside this increment.

`platform/mainframe_command*` owns names, command policy, capabilities and
result mapping. This plugin owns VM values, redirect callbacks and reference
counts. Native parameter lists, linkage and text conversion remain in the SDK.
The shared VM initializer registers the static catalogue; the native compiler
registers declaration metadata before imports. No constructor or dynamic
library loader is required on the mainframe.

| Request | CMS31 | TSO31 |
| --- | --- | --- |
| Native host command | CMSCALL, synchronous | IKJEFTSR, synchronous |
| SYSTEM / COMMAND / CMD | Selected CMS driver | Selected TSO driver |
| Other names | Re-query native SUBCOM | Borrow/query/release current ENVBLOCK; generic invocation unsupported |
| PDOS | Only if queried present | Only if queried present |
| OUTPUT capture | Reviewed native CMS QUERY DISK subset | Unsupported |
| Separate ERROR or INPUT | Unsupported | Unsupported |

The native TSO host-command service needs no IBM Rexx processor. Discovery
borrows an existing context for one call and never creates, terminates or
caches its pointer. A cached Rexx environment object rechecks native presence
on execution, without timers. Registered embedding-host callbacks retain
their existing higher priority.

Names are trimmed/uppercased and checked against the eight-byte native name
alphabet. Command text stays intact: valid UTF-8 representable in IBM1047,
1–32767 input bytes, no embedded NUL. Invalid text and unsupported redirection
are rejected before command effects. A valid signed command RC is distinct
from a facility, loader or sink failure. The common Classic layer owns
RC/.RC/.RS and ERROR/FAILURE handling.

Native CMS capture uses synchronous Pipelines and a newly reserved private
file, preserving collisions and cleaning up on every exit. The command subset
is QUERY DISK with an optional single-letter mode; QUERY TIME is rejected
because our CMS20 probe showed console output bypassing the pipeline. Records
are explicitly decoded into the existing receiver. Trailing spaces survive.
CMS's file sink represents an empty physical record as one space; we retain
that native representation and make no lossless empty-record claim. This is
one combined console response, not separate stdout/stderr. PDOS has no qualified
Pipelines capture route. The operator's OS probes establish facility behavior;
real cREXX receiver qualification is tracked separately in the plan.

Focused host checks compile the actual SDK adapters over a deterministic
native boundary. They cover policy, exact bytes, cleanup faults, two live VM
contexts, callback precedence, Classic conditions and a caller file held open
across commands. They are not an emulator or native ABI proof.

```sh
cmake -S . -B cmake-build-debug \
  -DCREXX_MAINFRAME_COMMAND_ADAPTERS=/absolute/sdk-source/libc/src/adapters
cmake --build cmake-build-debug --target mainframe_address_checks --parallel 4
ctest --test-dir cmake-build-debug -R '^mainframe_address_' --output-on-failure
```

Native RXC/RXAS/RXLINK/RXVM packages use
[`mainframe-build.crexx`](../../../ports/single-threaded/mainframe-build.crexx),
matching source manifests, canonical newlib overlays and the selected profile.
Before/after source/include/tool hashes detect concurrent input changes.
The [implementation plan](../../../docs/planning/mainframe-address-2026-10-11.md)
owns acceptance, actual guest evidence and remaining qualification. No released
mainframe support follows from a host build alone.
