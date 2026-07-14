# Classic TRACE built-in function

## Signature

```text
TRACE([option]) -> string
```

This is the standalone Level C `RexxValue` BIF implemented by
`RexxClassicBifTrace.crexx`. It is separate from the Level B trace/debugger
runtime and from the compiler's TRACE statement exit.

## Result and state

`TRACE()` returns the setting that was in effect before the call. With no
argument, or with an omitted optional argument, it is a pure query. The result
is the current single option letter, prefixed with `?` when interactive tracing
is enabled.

When an argument is present:

1. each leading `?` toggles interactive tracing;
2. an empty remainder selects Normal (`N`);
3. otherwise the uppercased first remainder character becomes the setting;
4. `O` also clears interactive tracing;
5. the result is still the previous setting.

For example, starting from Normal:

```rexx
previous = trace("results")  /* N; current setting becomes R */
current  = trace()           /* R */
previous = trace("?i")       /* R; current setting becomes ?I */
```

The caller's `RexxVariablePool` owns the per-activation state. A child pool may
copy it with `inheritTraceState`; subsequent changes remain activation-local.

## Arguments and errors

The CheckArgs contract is `oACEFILNOR`. The optional value may contain leading
question marks followed by a value whose first effective character is one of
`A C E F I L N O R`. Empty text and question marks with no remaining letter
are valid and select Normal. Invalid options set `RXC-LC-40.28`; more than one
argument sets `RXC-LC-40.4`.

Classic `TRACE()` does not expose the cREXX statement extensions `ASM`, `LLM`,
`ENV`, output targets, or namespace controls. Those belong to the separate
Level B/compiler-exit runtime.

`testRexxClassicBifTrace.crexx` calls `rexxclassicbif_trace` directly in
optimized and unoptimized harnesses. Compiler lowering remains on the
deprecated compatibility path until the later bulk Level C lowering change.
