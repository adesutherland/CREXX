# Date and Time

Programs have several different reasons for working with dates and times. A
program may need to record a calendar date, calculate the number of days
between two events, determine the ISO week containing a date, or identify an
instant that can be compared with an event recorded in another part of the
world. Although these requirements are related, they do not all require the
same representation.

The cRexx `time` package therefore distinguishes between a date and time as it
appears on a clock and a date and time that also carries an offset from UTC.
It also provides support for converting these values to and from ISO 8601
representations.

The package is built around three classes:

* `DateTime` represents a calendar date and a time of day;
* `OffsetDateTime` adds an offset from UTC to a `DateTime`; and
* `ISO8601` converts these values to and from ISO 8601 text.

This separation is useful even in programs that never deal with different
time zones. It makes explicit whether a value such as 20:00 means simply
"eight o'clock" or identifies an instant relative to UTC.

## Representing a Date and Time

A `DateTime` combines a Gregorian calendar date with a time of day. It has
microsecond precision but contains no UTC offset.

The simplest way to create one is:

    now = .DateTime()

which represents the current local date and time.

Internally, `DateTime` builds on the date facilities already present in Rexx.
The calendar part is stored as a Rexx base date: the number of days from the
Rexx base date. The time is represented independently as the number of
microseconds since midnight.

This representation has a useful property. Calendar arithmetic can be
performed on the base date while arithmetic within a day can be performed on
a single integer. Crossing midnight then consists of carrying whole days
between the two values.

A particular date and time can consequently be constructed directly when
these values are known:

    base = date("BASE", "2026-08-24", "INTERNATIONAL") as .int
    noon = .DateTime(base, 43200000000)

The resulting object represents noon on 24 August 2026.

Normally an application does not need to work with these internal values.
`DateTime` provides access to the familiar year, month, day, hour, minute,
second and microsecond fields, as well as useful calendar properties such as
the ordinal day within the year.

For example:

    say noon.year()
    say noon.month()
    say noon.day()
    say noon.hour()

produces the calendar and clock components of the value.

The underlying base-date representation nevertheless remains available. This
is particularly useful when interfacing with existing Rexx programs and
functions that already use `DATE("BASE")`.

## Calendar Arithmetic

Date and time arithmetic contains a number of boundary cases that are easy to
miss when dates are represented merely as formatted strings. `DateTime`
performs arithmetic on its internal date and time representation instead.

For example, adding a microsecond to:

    2026-08-24T23:59:59.999999

produces:

    2026-08-25T00:00:00.000000

The reverse operation crosses the boundary in the other direction. This same
mechanism allows seconds, minutes and hours to cross one or several calendar
days without special treatment by the application.

Arithmetic can be performed in units ranging from microseconds to weeks. A
negative amount moves backwards in time:

    value.addHours(-3)

Calendar arithmetic requires slightly different rules. A year is not a fixed
number of days, and a month is not a fixed number of seconds. Leap years in
particular make an operation such as "one year later" different from adding
365 days.

`addYears()` therefore performs calendar arithmetic. If a date is 29 February
and the destination year is not a leap year, the resulting date is 28
February.

The class also exposes the calendar properties needed for this sort of work.
It can determine whether its year is a leap year, the number of days in its
month or year, and its position within the year.

## Comparing DateTime Values

Two `DateTime` objects can be ordered chronologically because their calendar
and clock components form a single local timeline.

For example:

    if first.isBefore(second) then
        say "first comes before second"

Values can also be compared for equality or using a three-way comparison.
Equality requires both the date and the time of day to be equal.

These operations compare `DateTime` values as represented. When an
application needs to compare values carrying UTC offsets, `OffsetDateTime`
provides the necessary additional information.

## ISO Week Dates

The ordinary calendar year is not the only useful way of locating a day in a
year. Business and administrative applications often use ISO week dates.

ISO weeks begin on Monday. Week 1 is the week containing 4 January, which is
equivalent to saying that it is the week containing the first Thursday of the
year.

An important consequence is that the ISO week-numbering year does not always
start on 1 January.

For example, 1 January 2016 was a Friday and belongs to:

    2015-W53-5

It is calendar year 2016, but ISO week 53 of week-numbering year 2015.

The reverse can also occur. 31 December 2018 was a Monday and belongs to:

    2019-W01-1

`DateTime` provides the ISO weekday, week number and week-numbering year, and
can produce the complete ISO week-date representation. It can also determine
whether an ISO week year contains 52 or 53 weeks.

Keeping the calendar year and week-numbering year as separate concepts avoids
a common source of errors around New Year.

## Textual Representation

The normal string representation of a `DateTime` is deliberately predictable:

    2026-08-24T20:57:57.123456

The fractional part always contains six digits, corresponding to the
microsecond precision of the class.

This is a useful representation for display and diagnostics, but parsing and
formatting ISO 8601 is kept in a separate class. This prevents the basic
`DateTime` class from accumulating rules concerned only with textual
representations.

## Adding a UTC Offset

A local date and time is sufficient for many applications. In other cases the
value must identify an instant relative to UTC.

`OffsetDateTime` adds this information without changing the meaning or
implementation of `DateTime`. It consists simply of a `DateTime` and an offset
from UTC expressed in minutes.

For example, suppose `dt` represents 22:00 on 24 August 2026:

    value = .OffsetDateTime(dt, 120)

The offset of 120 minutes means that the local value is two hours ahead of
UTC. The complete value can therefore be written as:

    2026-08-24T22:00:00+02:00

Negative offsets work in the same way. An offset of -330 represents
UTC-05:30.

The local representation is retained. Constructing or parsing the value above
does not silently turn 22:00 into 20:00. This is useful because the object
continues to represent exactly the date, time and offset supplied by the
application.

When UTC is required, the conversion is explicit:

    utc = value.toUTC()

For the preceding example this produces the equivalent value:

    2026-08-24T20:00:00Z

UTC conversion also handles changes of calendar day. Thus:

    2026-08-24T01:00:00+02:00

becomes:

    2026-08-23T23:00:00Z

The original value is left unchanged.

This gives applications two useful views of the same information: the local
date and time at the specified offset, and the corresponding UTC value.

## ISO 8601

ISO 8601 provides a standard textual notation for dates and times. The
`ISO8601` class performs the conversion between those representations and the
date and time classes.

The distinction between `DateTime` and `OffsetDateTime` is reflected in the
parser. A representation without an offset is parsed as a `DateTime`; a
representation containing `Z` or a numeric UTC offset is parsed as an
`OffsetDateTime`.

### Local Date and Time

A calendar date can be parsed on its own:

    iso = .ISO8601()
    value = iso.parse("2026-08-24")

Because no time was specified, the result represents midnight:

    2026-08-24T00:00:00.000000

A complete local date and time can be written as:

    2026-08-24T12:34:56

Fractional seconds are optional:

    2026-08-24T12:34:56.1
    2026-08-24T12:34:56.123
    2026-08-24T12:34:56.123456

`DateTime` has microsecond precision, so fractional values are normalized to
six digits. The examples above consequently represent 100000, 123000 and
123456 microseconds respectively.

Input is checked while it is parsed. Invalid separators, incomplete times,
clock fields outside their permitted ranges and fractions beyond the
supported precision cause the `INVALID_ARGUMENTS` signal.

Applications that merely need to test input can use `isValid()`. It uses the
same parser and handles `INVALID_ARGUMENTS` internally, so validation and
parsing cannot gradually acquire different interpretations of the accepted
syntax.

### UTC and Numeric Offsets

An ISO 8601 date and time can identify UTC directly with `Z`:

    2026-08-24T20:00:00Z

or specify a numeric offset:

    2026-08-24T22:00:00+02:00
    2026-08-24T14:30:00-05:30

These forms are parsed with `parseOffset()` and produce an
`OffsetDateTime`.

The numeric representation is converted internally to an offset in minutes,
so the examples correspond to offsets of 120 and -330 minutes. `Z` corresponds
to an offset of zero.

Fractional seconds and offsets can be combined:

    2026-08-24T22:00:00.123456+02:00

The parser preserves both the local time and the offset. Conversion to UTC,
when wanted, remains an explicit operation on the resulting
`OffsetDateTime`.

### Formatting and Round Trips

The formatter performs the reverse conversion. A `DateTime` can be formatted
as a calendar date, a date and time to second precision, or a date and time
including microseconds.

An `OffsetDateTime` additionally includes its UTC offset. Zero is written in
the conventional `Z` form, while other offsets are written numerically:

    2026-08-24T20:00:00.000000Z
    2026-08-24T22:00:00.123456+02:00

This allows parsed values to make a straightforward round trip:

    text = "2026-08-24T22:00:00.123456+02:00"

    value = iso.parseOffset(text)
    result = iso.formatOffset(value)

`result` contains:

    2026-08-24T22:00:00.123456+02:00

The parser and formatter therefore form the boundary between the textual
ISO 8601 representation used for interchange and the `DateTime` and
`OffsetDateTime` objects used by an application.

## Choosing a Representation

The choice between the two date-time classes follows from the information
available to the application.

Use a `DateTime` when the value consists of a calendar date and clock time:

    2026-08-24T20:00:00

Use an `OffsetDateTime` when an explicit relationship with UTC is part of the
value:

    2026-08-24T20:00:00Z
    2026-08-24T22:00:00+02:00

The `ISO8601` class preserves this distinction when values cross a textual
interface. It does not invent an offset when none was supplied, and it does
not discard one that was present.

This separation keeps the individual classes small while providing the
building blocks needed for calendar calculations, ISO week dates, interchange
formats and UTC-based comparisons.
