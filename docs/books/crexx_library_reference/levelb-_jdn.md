## DATE calendar core

`_rxsysb` exposes the typed proleptic-Gregorian primitives used by the Level B
DATE cluster and the standalone Classic DATE adapter:

```rexx
_jdn(day=.int, month=.int, year=.int) = .int
_datefromjdn(jdn=.int, expose year=.int, expose month=.int,
             expose day=.int, expose day_of_year=.int,
             expose weekday=.int, expose base_day=.int)
_datevalid(day=.int, month=.int, year=.int) = .boolean
_leapyear(year=.int) = .boolean
_daysinmonth(month=.int, year=.int) = .int
```

The supported calendar is Gregorian year 1 through 9999. JDN 1721426 is
0001-01-01 and JDN 5373484 is 9999-12-31. Weekdays use Monday=1 through
Sunday=7; `base_day` is zero on 0001-01-01.

`_jdn` and `_datefromjdn` signal `INVALID_ARGUMENTS` outside this domain.
`_datefromjdn` computes into locals and does not change its exposed outputs
when validation fails. `_daysinmonth` returns zero for an invalid year/month.

The conversion is constant-time integer arithmetic. The focused
`ts_date_core` harness covers known JDN values, Gregorian century/leap rules,
round trips, both boundaries, signals, and exposed-output preservation.
