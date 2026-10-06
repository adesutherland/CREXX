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
when that policy is enabled. Command completion sets `RC`, `.RC` and `.RS`; eligible ERROR,
FAILURE and NOTREADY events reach the active SIGNAL or CALL policy at the
causing ADDRESS clause. Invalid STEM counts signal SYNTAX 54.1 there. A native
callback's condition and diagnostic are copied by the approved `rxvml`
response bridge without changing its callback ABI.

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
