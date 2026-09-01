## find

`find` is the VM/TSO-compatible argument-order alias of Level B `wordpos`:

```rexx
find(needle=.string, haystack=.string [,start=.int]) = .int
```

It returns the one-based word number at which the exact, case-sensitive word or
phrase first occurs, or zero. `start` is a one-based word number and defaults to
`1`. Runs of Unicode whitespace separate words. Prefixes are not matches.

```rexx
find("the", "Now is the time") == 3
find("be", "To be or not to be", 3) == 6
```

An empty/blank phrase or haystack returns zero. A non-positive `start` signals
`INVALID_ARGUMENTS`. Inputs are not modified.

The implementation deliberately delegates to the reviewed Level B `wordpos`
algorithm so the compatibility alias cannot drift from its word semantics. It
adds one fixed function call and no string copy or scan of its own. There is no
Level C BIF or class method named FIND. The focused harness is
`lib/rxfnsb/tests_functional/ts_find.crexx`.
