## raise

`raise` is the internal `_rxsysb` helper used by Level B runtime code to raise
one of the VM's canonical signals with a diagnostic message.

```rexx
call raise "SYNTAX", "Error 40.28: invalid DATE argument 3", format
```

All three arguments are `.string`. `type` normally names a canonical VM signal
such as `INVALID_ARGUMENTS` or `NOTREADY`. The compatibility spellings
`syntax`, `SYNTAX`, and `error` are normalized to the VM's canonical `ERROR`
signal. `code` is the primary diagnostic text and `parm1` supplies contextual
text. When `parm1` is not blank, the delivered signal message is `code`, one
blank, then `parm1`; a blank `parm1` delivers `code` unchanged.

The procedure retains its internal compatibility result `.int`. Normally the
signal transfers control and no result is observed. A signal handler may
explicitly skip the signal, in which case the helper returns zero to its caller.
An unknown `type` is rejected by the VM as `INVALID_SIGNAL_CODE`.

## Coverage and performance

`lib/rxfnsb/tests_functional/ts_raise.crexx` verifies the delivered signal name
and message, the blank-context case, and VM validation of an unknown signal
name in optimized and unoptimized selector overlays.

The helper performs no output and no scan. It builds at most one diagnostic
string and executes one dynamic-name `signal` instruction.
