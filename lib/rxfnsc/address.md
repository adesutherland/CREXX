# ADDRESS — Level C Classic instruction and BIF

Compiled Level C keeps the selected environment, its alternate, and the three
connection descriptions in `RexxActivationArguments` for each Classic
invocation. `RexxClassicBifAddress.crexx` reads that same state for `ADDRESS()`.
The direct library BIF retains its pool-based entry for existing callers.

## Contract

```text
ADDRESS([option])
CheckArgs: oEINO
```

The optional, case-insensitive option has these results:

| Option | Result |
|---|---|
| omitted or `N` | active command-environment name |
| `I` | input `position type resource` |
| `O` | output `position type resource` |
| `E` | error `position type resource` |

The three connection fields are separated by one blank. A null resource is
still represented, so a default connection result has a trailing blank.

The default activation state is:

| State | Default |
|---|---|
| environment | `CREXX` |
| input | `INPUT NORMAL ` |
| output | `REPLACE NORMAL ` |
| error | `REPLACE NORMAL ` |

This follows the Classic contract described by the
[ANSI X3J18 draft, section 9.5.1](https://www.rexxla.org/rexxlang/standards/j18pub.pdf).

## Direct Level C API

```rexx
result = rexxclassicbif_address(reference context)
```

`context` is a `RexxBifCallContext`. Arguments and provided/omitted positions
are carried as `RexxValue` and presence arrays. The direct entry reads the
caller's `RexxVariablePool` state. The compiler calls
`rexxclassicbif_address_frame(context, activation)` so the query sees the
current invocation's selection and its snapshotted connection resources.
Child Classic activations copy both the active and alternate settings and
restore the caller's state on return.

Both entries clear the context error, validate the call and return a new
`RexxValue`. Neither performs command dispatch.

## Compiled instruction

`ADDRESS` selects an environment, `ADDRESS VALUE expression` evaluates and
selects one, and bare `ADDRESS` exchanges the active and alternate settings.
`WITH` sets INPUT, OUTPUT and ERROR connections. A STREAM variable is evaluated
when its ADDRESS clause runs and its filename is retained in the frame; a
later assignment to that variable does not redirect the existing connection.

An explicit command uses `RexxClassicAddressCommand`: it captures the command
text once, applies transient `WITH` connections, and dispatches one
`addressrequest` through the shared Level B environment registry. INPUT STEM
uses its counted lines; INPUT STREAM reads exact bytes through
`addressredirect.input_binary` and preserves a missing final newline. OUTPUT
and ERROR STEM or STREAM receive captured output, with REPLACE or APPEND as
selected. INPUT STREAM uses the shared RXCV channel's 16 MiB input limit;
an unreadable or larger input is a resource failure, reported through NOTREADY
when that policy is enabled or that NOTREADY is delayed in a normalized
external CALL handler. Command completion sets `RC`, `.RC` and `.RS`; eligible ERROR,
FAILURE and NOTREADY events reach the active SIGNAL or CALL policy at the
causing ADDRESS clause. Invalid STEM counts signal SYNTAX 54.1 there. A native
callback's condition and diagnostic are copied by the approved `rxvml`
response bridge without changing its callback ABI.

### Direct command binding plan

`RexxClassicAddressCommand.withBinding(kind:string,name:string)->void` appends
one ordered `VAR` or `STEM` descriptor. Names use the existing binding
convention: a scalar name or a stem basename. The compiler supplies checked
lexical names. Adding a descriptor reads no pool values and does not scan the
command text or dispatch it.
An unsupported normalized kind signals `INVALID_ARGUMENTS` without adding a
descriptor or dispatching the command.

`run()` honors trace inhibition before binding reads and preserves WITH
validation precedence. It then snapshots descriptors once, in order, into a
run-local request. VAR reads the raw symbol value. STEM reads literal `0..count`
through `stemSymbolValue`; count must be whole, nonnegative and representable,
otherwise `syntaxError()` reports 54.1. Strong run-local stem owners remain live
through dispatch, including its existing by-value request wrappers. The request
is initialized even when a built-in empty command skips dispatch. A repeated
run takes fresh snapshots.

After dispatch and WITH output/error application, descriptors copy back in
their original order, before RC/.RC/.RS finalization. VAR writes only when a
VAR response update matched, so a previously unset scalar stays unset without
an update; an explicit empty update remains a write. STEM merges response
updates, validates final literal `0` before writing that stem, then always
copies the final counted range `0..count` through `setStemValue`. This includes
direct provider mutations without response records and may materialize numeric
cells in the bound range. Stem defaults, nonnumeric and out-of-count tails,
and EXPOSE aliases survive. No arbitrary-tail enumeration or mutation tracking
is performed. Bindings and borrowed workers retain the shared protocol's
existing per-binding response scan and alias/last-update rules.

`testRexxClassicAddressCommand.crexx` uses an in-process B environment to check
these owners, projection order, no-response growth/shrink, count failures,
repeat snapshots and completion state. This shared B bridge does not close the
research C frontend's command-anchor, warning, handler or host/platform work.

An expression-only Classic clause captures the command text once, then copies
the frame's active environment and lasting connections into the same command
adapter. It uses the same request, RC/.RC/.RS update, SYNTAX and condition
delivery as an explicit command. The compiler retains its authored
`IMPLICIT_CMD` source node and warning before lowering; Level B/G compiler
exits retain their own path.

An ADDRESS command scalar can contain embedded NUL, but the current shared
process channel rejects it and the native callback request exposes only a C
string. The host-command boundary is tracked as `LC-HOST-ADDRESS-NUL` in the
Level C worklist. Adrian approved closing the ADDRESS instruction with this
host-interface obligation still open; no length-aware native callback ABI or
VM change was made for that gap.

## External condition scope API

`RexxActivationArguments.beginExternalConditionScope()` starts a provider-owned
condition scope. It clears all seven foreign handler identities, pending and
dispatch records, and delayed-dispatch errors. Only a valid one-shot explicit
`beginCallTrap(2..5,description)` entry retains its delivered CONDITION C/D/E/I
and DELAY state. An ordinary or stale entry starts with empty condition data;
normalizing twice cannot deliver the same marker again.

`delayedSuppressionConditionId()` returns a suppression ID only when it is in
2..5 and still equals the delayed condition. It can remain active after another
trap changes the current CONDITION data. Internal children inherit validated
suppression but never the explicit-entry marker. `beginExternalProgram()`
clears inherited suppression while preserving an explicitly prepared marker;
matching ON/OFF replacement cancels both. Numeric, ADDRESS/TRACE, arguments,
results and program-root behavior otherwise retain their existing contracts.

The command adapter preserves NOTREADY at both input and output/error stream
gates for a local policy or delayed suppression ID5. Without either, existing
input fallback and normal completion remain. `queueCallCondition()` still
returns whether an event was queued or buffered, not whether it was handled:
ERROR/FAILURE/NOTREADY can be suppressed without a queued event. Delayed HALT
reuses the existing parent buffer and one-pending-event rules.

These shared B methods are a prerequisite for later research C provider and
handler integration. They do not add C ON/OFF syntax, checkpoints, event relay,
resumption or a new host interface; existing B/G/native entry points are not
automatically normalized.

## Errors

Level C errors are recorded on the call context and the function returns a
blank `RexxValue`:

| Case | Error |
|---|---|
| a provided option is empty | `RXC-LC-40.21` |
| option is not `E`, `I`, `N`, or `O` | `RXC-LC-40.28` |
| more than one argument | `RXC-LC-40.4` |

## Coverage

`lib/rxfnsc/tests_functional/testRexxClassicBifAddress.crexx` calls the direct
and frame BIFs in optimized and unoptimized modes. The linked Level C ADDRESS
fixtures cover valid forms, invocation state, external signed calls, exact
stream input, output/error capture, condition delivery, source trees, and
source-anchored errors. `levelc_address_host_callback` exercises the native
environment response and Unicode streams. The Level B protocol test checks
that binary input can preserve embedded NUL and non-text bytes.
