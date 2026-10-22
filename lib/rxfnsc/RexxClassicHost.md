# Classic host runtime

`RexxClassicHost.crexx` owns an invocation and its completion. The generated
Level C `--host namespace.entry` export is an ordinary typed procedure:

```text
entry(request: RexxClassicHostRequest) -> RexxClassicHostResult
```

The compiler, shared runtime and generic VM have separate responsibilities.
The compiler anchors the request, activation, pool, configuration, Context and
Scope in local storage before lending references. The runtime owns Classic
mode, argument presence, services, pool windows and cleanup. The VM owns the
underlying references, typed conditions, channels, platform clock and interrupt
ingress. An ordinary typed call receives its declared values; it receives no
implicit Classic variable-pool capability.

## Requests and completion

`RexxClassicHostRequest(mode, source="")` accepts `COMMAND`, `FUNCTION` or
`SUBROUTINE`. `append(value, supplied=1)` and `appendText(text, supplied=1)`
preserve every supplied position, including trailing omissions and present
empty values. Strings retain their Unicode scalars, embedded NULs and trailing
blanks. Request limits count UTF-8 bytes, raw positions and enabled callback
depth; they do not claim to limit native memory use.

The request supplies these initial settings:

| Method | Contract |
|---|---|
| `setConfiguration(config)` | Copy service configuration; existing supplied stream resources remain borrowed. |
| `setSource(text)` | Exact logical source identity for host observations. |
| `setEnvironment(text)` | Initial nonempty ADDRESS environment. |
| `setTrace(text)` | Initial setting validated by the existing TRACE owner. |
| `setNumeric(digits, fuzz, form)` | Initial numeric state; form 1 is SCIENTIFIC, 2 ENGINEERING. |
| `setQueue(name)` | Select an existing queue; empty requests a fresh private invocation queue. |
| `setDefaultStream(direction, name, encoding="UTF8", replacing=0)` | Select default input 1, output 2 or error 3, opened at Scope entry through the existing Classic stream/codec owner. Empty error name retains inherited stderr. |
| `setClock(provider, trap=0)` | Select a coherent clock sample provider or trap. |
| `setLimits(argument_bytes, positions, callback_depth)` | Refuse an argument mutation atomically when it exceeds its budget. |

`activation()`, `pool()` and `configuration()` create the values anchored by the
generated wrapper. A fresh root has a fresh visible pool, a fresh activation
and a configuration value. The configuration getter snapshots service settings;
Scope enters the protected lifetime before `configureResources(reference Config)`
opens selected streams. Partial initialization therefore closes every resource
already opened and returns the original typed failure. Reusing a request does
not reuse a live pool window.

The result owns its value and typed condition after invocation teardown:

| `outcome()` | `indicator()` | Meaning |
|---|---|---|
| `ABSENT` | `N` | Completed without a returned value. |
| `RESULT` | `R` | Present returned value, including an empty string. |
| `CONDITION` | `E` | Escaping typed condition. |
| `RESOURCE` | `X` | Exact Classic resource error 5.1. |
| `UNABLE` | `S` | Exact Classic unavailable error 48.1 or typed NOT_IMPLEMENTED. |

`hasValue()` and `hasCondition()` are independent of a value's text. `value()`,
`condition()`, `source()` and `commandStatus()` expose the owned observations.
The integer status is a compatibility view and does not replace the result
packet. Classification uses the shared exact Classic SYNTAX-code decoder;
arbitrary FAILURE message text cannot claim a resource outcome. Generic native
allocation FAILURE remains a typed condition unless its owner supplies an
established resource identity. An unhandled POSIX_INT remains its original
typed condition; a Classic HALT handler can complete normally.

Scope consumes a detached completion packet with `finish(completion)`, captures
an escaping bound event with `failed(problem)`, and revokes resources with an
idempotent `close()`. A cleanup condition cannot erase an earlier primary
condition: the result retains it through `hasCleanupCondition()` and
`cleanupCondition()`. TERMINATE executes at most once. The propagating VM CANCEL
event follows the same cleanup path, preserving its original typed condition,
revoking callback windows and restoring nested callback state before an outer
handler resumes. KILL remains terminal and uses native VM teardown. Ignored OS
notifications and ignored arithmetic/breakpoint notifications are excluded from
cleanup selectors; this API does not add language-wide finally semantics to
user OS handlers.

The stable host activation owns terminal failure separately from a Classic
program's normal EXIT state. Scope binds that owner before INITIALIZE; local
calls and selected precompiled Classic routines inherit a checked reference.
An escaping event that Classic has declined retains its exact first typed
condition and unwinds through the existing exit guards. `finish()` reads that
authoritative owner before the detached return packet. A selected external
routine's normal EXIT stays local to its program.

Scope close revokes the completion owner. A checked generation prevents a
retained child from regaining access when the same activation is used by a
later Scope. Normal program EXIT lookup follows its own root without reading
another activation's host-failure state. Nested scopes own distinct failure
state. Without a live explicit host boundary, the existing terminal HALT relay
remains in force.

## Configured hooks and windows

`RexxClassicHostProtocol.crexx` holds dependency-light hook, window and clock
types. A `RexxClassicHostHook` implements:

```text
invoke(operation, values: RexxValue[], presence: int[],
       window: RexxClassicHostPool) -> RexxClassicHostHookResult
```

Register a service with `config.registerHostHook(operation, hook, trap=0)`.
Operation names are uppercased. Passing an uninitialized hook removes that
slot. The trap runs first. An unhandled, absent or present-empty trap falls
through; a nonempty trap replacement, including a single blank or NUL, a typed
condition or a command response wins.
A configuration hook's present empty replacement remains a replacement.

Hook results distinguish the default unhandled factory, `returning(value)`,
`absent()`, `failed(bound_condition)` and `command(addressresponse)`. A failed
response must retain a real bound VM event for the shared SIGNALORIGIN relay;
constructing an unrelated signal object does not fabricate provenance.

The configured operations are INITIALIZE, TERMINATE, SAY, PULL, LINEIN, PUSH,
QUEUE, QUEUED, RANDOMSEED, RANDOM, CHARIN, CHAROUT, LINEOUT, CHARS, LINES, QUALIFY,
STREAM, COMMAND and EXTERNALROUTINE. Stream BIF hooks receive validated source
actuals and presence; a service owns the meaning of its result. COMMAND may
return the existing ADDRESS response with RC, condition and diagnostics.
TERMINATE receives the completion indicator and optional value, tag and message.

EXTERNALROUTINE is a missing-name fallback after internal routines, BIFs and
explicitly selected providers. It does not preempt an admitted callee or search
PATH/source files. `externalRoutine(name, function, values, presence)` preserves
all original actuals and holes. Its window supplies the exact ROUTINE name and
FUNCTION/SUBROUTINE INVOCATION fact separately from the ordinary arguments.
Handled absent and unhandled differ; expression calls still require a value.
Dynamic Classic source loading remains with INTERPRET.

An enabled pool window delegates to the existing visible-pool owner:

| Method family | Meaning |
|---|---|
| `set`, `value`, `drop` | Validated Classic symbols with normal tail substitution. |
| `setDirect`, `valueDirect`, `dropDirect` | Already-resolved uppercase head and exact tail; no extra tail casing or substitution. |
| `valuePool(pool, subject)` | Explicit configured external VALUE-pool lookup. |
| `valueOther(subject, index=0)` | ARG, ARGCOUNT, MODE, SOURCE, SOURCELINE, SOURCELINES, ADDRESS, ROUTINE or INVOCATION facts. |
| `reset`, `next`, `nextVariable` | Snapshot visible names, observe current values, skip dropped names; new names appear after reset. |

Lookup results distinguish missing from present empty values. Nested callbacks
suspend the outer window, then restore it. Disabled, expired, suspended,
foreign-context and teardown access raises REFERENCE_INVALID. A private
VM-module sentinel and window generation are checked before following Context
owner references. A private VM-local active-state reference also suspends an
outer grant during a nested host invocation with a different configuration;
Scope close restores it in last-entered, first-closed order. No process-global
registry or identity token is introduced.

The compiler refreshes `config.bindHostContext(current_context as .object)` at
every callback boundary, including after PROCEDURE. The enabled access provider
checks the concrete Context class before casting, validates live owners and
retains only authoritative activation/pool/config references. Each window owns
that snapshot. Nested dispatch restores the prior binding; close clears it.
The root Context is used for invocation initialization/finalization, while a
procedure callback uses its current private pool and EXPOSE aliases.

## Queues, streams, process bytes and clocks

Selected queue operations borrow the existing queue owner and restore the
outer selection. Private queue selection probes the existing repository with a
finite bound and preserves occupied names and their contents. A private queue
is deleted at scope close. FIFO/LIFO, empty
queue fallback and exact NUL-containing entries remain shared runtime behavior.

Configured default text I/O uses `RexxClassicStream` and its codec. ADDRESS and
explicit process calls obtain defaults through
`config.processEndpoint(direction)`. Explicit ADDRESS connections, including
authored NORMAL, override those defaults. Process CALL adds no new redirection
syntax: its configured/default streams and exit-status result use the same
foundational ADDRESS child/byte-channel implementation.

The generic `addressbytestream` interface supplies `input_snapshot()->binary`,
`append_bytes(binary)` and `finish()`. The adapter borrows Config plus its
stream slot, and links the authoritative stream for every operation. Input
remains bounded by the existing ADDRESS byte limit. Nonseekable pending decoded
lookahead is refused before reading or changing positions. Named input retains
the original remaining bytes; a prior logical prefix is located by its owning
codec; a non-reversible replacement prefix is refused before changing logical
input position. EOF is visible to later reads and explicit repositioning starts a fresh
logical read. Raw output writes and flushes the original bytes without codec
conversion. Later text output re-enters the existing codec; invalid encoded raw
data raises NOTREADY without rewriting it. `finish()` does not close a borrowed
resource. Scope closes streams it opened; supplied configurations retain their
own existing handles.

A clock returns one `RexxClassicHostClockSample(base_day, local_microseconds,
utc_microseconds, offset_microseconds)` or `fallthrough()`. A present trap
sample overrides configuration. An absent trap falls through. Sampling remains
lazy and frozen for one clause; child samples use the root's configured
producer. TIME(E/R) retains the existing root UTC elapsed origin. This contract
does not introduce monotonic semantics or a new clock opcode.

## Example

Given a compiled `--host example.run` export:

```rexx
options levelb
import example
import rexxclassichost
import rexxvalue

main: procedure = .int
  request = .RexxClassicHostRequest("FUNCTION","embedded example")
  call request.appendText("exact" || ('00'x as .string) || "value")
  call request.appendText("",0)
  call request.appendText("")
  outcome = example..run(request)
  if outcome.hasCondition() then return 1
  if outcome.hasValue() then say outcome.value().asString()
  return outcome.commandStatus()
```

Maintained development fixtures are `tests_functional/testRexxClassicHost.crexx`
and the `research/level-l/compilers/tests/group2/host-*` consumers. They cover the
selected local normal Debug/rxbvm boundaries. Platform/delivery and formal full
compiler acceptance remain governed by the authoritative Level L plan; these
fixtures do not establish unrun platform proof.
