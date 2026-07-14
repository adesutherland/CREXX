# Level B ADDRESS environment protocol

`_address.crexx` is the Level B runtime protocol behind the `ADDRESS`
statement, ADDRESS function calls, host callbacks, redirects, and variable
writeback. It is not the Classic Level C `ADDRESS()` query BIF; that separate
contract is documented in `lib/rxfnsc/address.md`.

## Main surfaces

The module provides four related layers:

1. `addressdriverregistry` normalizes driver names, aliases, and prefixes and
   resolves a requested environment to its registered driver.
2. `addressstem`/`standardaddressstem` and
   `addresssandbox`/`standardaddresssandbox` provide normalized key/value
   stores. Their `next(cursor)` cursor is an `.int`; zero starts iteration.
3. `addressbinding`, `addressrequest`, `addressresponse`,
   `addressfunctionrequest`, and `addressfunctionresponse` carry commands,
   functions, redirects, sandbox data, exposed values/stems, return codes, and
   diagnostics.
4. `_address*`, `_new_address_environment`, `_ensure_address_environment`,
   `_register_address_environment`, and `_set_address_environment` construct,
   dispatch, register, and select environment implementations.

Environment names and sandbox/stem keys are stripped and uppercased for
lookup. Commands, values, diagnostics, redirect resources, and provider data
remain strings. Counts, indexes, return codes, binding slots, and iteration
cursors use `.int` at the Level B boundary.

The request/response, binding, standard sandbox, and standard stem class
attribute order is part of the native VM bridge ABI in `interpreter/rxvml.c`.
Do not reorder or insert attributes without changing and testing that bridge.

## Errors

Invalid programmer arguments are reported with the `INVALID_ARGUMENTS`
signal. In particular, driver and environment names must not be blank, and a
stem binding count passed to the native spawn bridge must not be negative.
Runtime provider failures remain `addressresponse` or
`addressfunctionresponse` values with a return code, condition name, and
optional diagnostics.

An unknown command environment produces a failure response; it is not an
argument-type error.

## Examples

The following operations do not launch an external command:

```rexx
registry = .addressdriverregistry()
call registry.add("openai", "gpt", "gpt_")
say registry.lookup("gpt_4_1")  /* OPENAI */

sandbox = .standardaddresssandbox()
call sandbox.set("Mode", "safe")
say sandbox.get("MODE")         /* safe */
say sandbox.next(0)             /* MODE */
```

To handle a bad name:

```rexx
do
  call _set_address_environment("")
on signal invalid_arguments as problem
  say problem.name()             /* INVALID_ARGUMENTS */
end
```

## Performance notes

Registry entries and stored keys are normalized on insertion. Lookups
normalize the search key once and compare it with the stored normalized keys;
they do not repeatedly strip and uppercase every stored entry. Updating an
existing sandbox or stem entry reuses the normalized key rather than
normalizing it again. The current registries are compact arrays, so lookup is
linear in the number of registered entries and performs no per-entry
normalization or object allocation.

## Coverage

`lib/rxfnsb/tests_functional/ts_address_protocol.crexx` covers the pure Level B
protocol in optimized and unoptimized modes: registry aliases/prefixes,
typed cursors, normalization, drop/iteration, request/response objects,
sandbox/stem/scalar writeback, function requests, environment selection,
unknown-environment responses, and signal errors. The existing
`ts_address.crexx`, `ts_address_crexx.crexx`, compiler ADDRESS cases, and native
host callback tests cover execution integration.
