# Classic TRACE built-in function

## Signature

```text
TRACE([option]) -> string
```

`RexxClassicBifTrace.crexx` supplies a direct pool adapter and the compiled
Classic invocation adapter `rexxclassicbif_trace_frame`. Both use the shared
`RexxTraceState` option parser. The compiler connects successful invocation
updates to the existing Level B trace controller; the BIF itself owns no VM
breakpoint handler or source-event formatter.

## Result and state

`TRACE()` returns the setting that was in effect before the call. With no
argument, or with an omitted optional argument, it is a pure query. The result
contains the current mode and active `?`/`!` prefixes, in that order.

When an argument is present:

1. empty text resets Normal (`N`) and clears both prefixes;
2. each leading `?` or `!` toggles the corresponding flag;
3. prefixes alone preserve the current mode;
4. alphabetic options use their first letter; exact `AS`/`ASM` and `LL`/`LLM`
   retain the existing extensions;
5. signed ASCII integer text is validated without narrowing: zero selects Off,
   while a nonzero count preserves the mode without enacting skip/suppression;
6. `O` clears both prefixes; invalid options leave all state unchanged.

The result remains the setting before the call. The approved practical scope
does not prompt for interactive input, enact numeric counts or accept SCAN.
`ENV` selects Classic Error by its first letter. See
[TRACE requirements](../../docs/ai-context/CREXX_TRACE_REQUIREMENTS.md).

For example, starting from Normal:

```rexx
previous = trace("results")  /* N; current setting becomes R */
current  = trace()           /* R */
previous = trace("?i")       /* R; current setting becomes ?I */
```

Compiled Classic uses `RexxActivationArguments` state, even when caller and child
share a variable pool. Children inherit their activation state and changes remain
local. The direct compatibility adapter uses its caller's `RexxVariablePool`;
a child pool may copy that state with `inheritTraceState`.

## Arguments and errors

The CheckArgs contract is `oANY`; the shared state parser validates the full
option text after normal argument checking. Invalid options set
`RXC-LC-40.28`; more than one argument sets `RXC-LC-40.4`.

Output targets, environment rereading and namespace controls belong to the
Level B/compiler-exit runtime, rather than this Classic BIF.

`testRexxClassicBifTrace.crexx` calls `rexxclassicbif_trace` directly in
optimized and unoptimized harnesses. Compiled Classic selects the frame adapter;
the research compiler's independent fixtures live in
`research/level-l/compilers/tests/trace`.
