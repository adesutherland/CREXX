## Level B signal objects

`signal.crexx` defines the typed Level B condition objects used by signal
handlers and the action values returned by action-aware handlers. It is runtime
support for Level B; it is not a Level C Classic BIF.

## Public condition contract

`.signal(name, message="")` creates a message-only `.standard_signal`. Names are
normalized to uppercase. `code()` maps every VM signal name to its integer
runtime code; `SYNTAX` is an alias for `ERROR` (code 3), and an unknown name maps
to `INVALID_SIGNAL_CODE` (code 13). Raising an unknown dynamic name causes the
VM to deliver `INVALID_SIGNAL_CODE`.

The `.signal` methods are:

| Method | Type | Meaning |
| --- | --- | --- |
| `name()` | `.string` | Canonical condition name |
| `code()` | `.int` | VM signal code |
| `module()` | `.int` | One-based originating module, or zero when absent |
| `address()` | `.int` | Originating RXAS address, or zero when absent |
| `message()` | `.string` | String message view |
| `payload()` | `.object` | Object view of the payload slot |
| `file()` | `.string` | Closest preceding source file, or blank |
| `line()` | `.int` | Source line, or zero |
| `column()` | `.int` | Active source column, or zero |
| `source()` | `.string` | Whole authored source line, or blank |

Message-only `.standard_signal` values have no runtime origin, so their
module/address/line/column methods return zero and file/source return blank.

## Runtime transport

The VM passes a five-slot interrupt object to a procedure handler: code, module,
address, name, and message/payload. `.runtime_signal_raw` maps those physical
slots with explicit register views. `.runtime_signal` wraps the raw object behind
the public `.signal` interface and resolves source metadata only when the
file/line/column/source methods are requested.

The raw class and `runtime_signal.set_raw()` are runtime/compiler transport, not
ordinary application construction APIs. Their attribute order and register
views must remain aligned with `rxsignal_populate_raw_interrupt()` in the VM.

## Handler actions

Action-aware handlers return one of:

```rexx
return .signalaction.skip()
return .signalaction.fail()
```

`kind()` returns `skip` or `fail`. Skip resumes after the faulting instruction
and fail propagates the condition. Instruction-level retry is deliberately not
provided; use an explicit source-level loop for retryable work.

## Coverage and performance

`lib/rxfnsb/tests_functional/ts_signal.crexx` covers the public factory provider,
the complete name/code table, all action factories, raw transport views, the
runtime wrapper, populated source metadata in the unoptimized image, and the
documented blank/zero metadata results in the source-stripped optimized image.
Compiler-exit and VM RXAS tests separately cover syntax lowering, handler
diagnostics, scope, and dispatch modes.

The name/code mapping uses a static string SELECT so optimized RXAS can use one
packed dispatch table instead of repeating a long comparison ladder. Ordinary
field access is constant-time. Source metadata lookup is a closest-preceding
linear scan and runs only when a handler requests source information.
