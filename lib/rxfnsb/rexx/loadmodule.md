# Level B `loadmodule`

`loadmodule` is the explicit Level B runtime loader for one RXBIN or RXPLUGIN
artifact.

```rexx
module_number = loadmodule(module_path)
```

`module_path` is a `.string`. The return value is an `.int`: a positive value
is the one-based number of the last loaded module, while zero or a negative
value is the VM loader's failure status.

On success, the VM immediately relinks the runtime so unresolved imports can
bind to procedures, classes, and interfaces in the new module. The function
loads only the explicit path supplied by the caller; it does not search or
sweep a directory. Callers should therefore pass an intended trusted artifact,
as described in `docs/books/crexx_programming_guide/running.md`.

## Failure contract

`loadmodule` deliberately preserves the `METALOADMODULE` status protocol. A
missing, invalid, or blank path returns a non-positive integer rather than
raising a signal. This is an explicit VM contract, not silent error handling:
the opcode documentation requires callers to inspect the result, and only a
fatal dispatch-table allocation failure terminates execution.

```rexx
provider = loadmodule("./providers/example.rxbin")
if provider <= 0 then return 1
```

## Coverage and performance

`lib/rxfnsb/tests_functional/ts_loadmodule.crexx` loads a focused provider and
checks missing and blank-path statuses in optimized and unoptimized modes.
Existing dynamic interface and host regressions cover late binding to real
provider exports.

The wrapper is constant-size: one typed string argument, one integer result,
one `metaloadmodule` instruction, and no wrapper-side scan, copy, conversion,
or allocation. Module loading and relinking costs belong to the VM operation
itself and occur only on an explicit call.
