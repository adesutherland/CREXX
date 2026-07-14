# Level B `time`

`time` exposes the Level B VM clocks through a text-returning API:

```rexx
time([option=.string]) = .string
```

The case-insensitive options are:

| Option | Result |
|---|---|
| `N` or omitted | local `hh:mm:ss` |
| `L` | local `hh:mm:ss.ffffff` |
| `C` | local 12-hour `hh:mmam` or `hh:mmpm` |
| `H`, `M`, `S`, `US` | hour, minutes, seconds, or microseconds since local midnight |
| `E`, `R` | elapsed seconds, or elapsed seconds followed by reset |
| `UTC` | UTC `hh:mm:ss` |
| `ZD` | UTC offset in seconds |
| `T`, `TS` | process clock ticks, or ticks per second |
| `ZN` | standard and daylight timezone names separated by `;` |

Unsupported options signal `INVALID_ARGUMENTS`. Platform clock failures are not
reported by the current VM instructions; allocation failure is fatal.

The `US`, offset, tick, and timezone paths return immediately after one clock
instruction. Numeric decomposition occurs only for formats that require it;
normal, long, and civil text is appended directly without general string helper
calls. The elapsed timer is process-global through `_rxsysb._elapsed`, handles
local-midnight rollover, and `R` returns the prior elapsed value before reset.

This typed Level B extension is distinct from Classic Level C `TIME`, whose
three-argument conversion and frozen-clause-time contract is documented in
`lib/rxfnsc/time.md`.
