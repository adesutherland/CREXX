# Date and Time

The `time` package provides classes for representing dates and times and for
reading and writing ISO 8601 representations. The package separates a local
date and time from a date and time that has an explicit offset from UTC.

The principal classes are:

* `DateTime`, representing a calendar date and a time of day;
* `OffsetDateTime`, representing a `DateTime` together with a UTC offset; and
* `ISO8601`, providing ISO 8601 parsing and formatting.

A `DateTime` does not imply a time zone. For example:

    2026-08-24T20:00:00

describes a local date and time, but does not identify where in the world that
time occurs.

An `OffsetDateTime` adds an offset from UTC:

    2026-08-24T20:00:00Z
    2026-08-24T22:00:00+02:00

These two values represent the same instant. The first is expressed in UTC and
the second at an offset of two hours ahead of UTC.

The package deliberately distinguishes UTC offsets from time zones. An offset
such as `+02:00` is sufficient to identify an instant, but it does not identify
a geographical time zone or describe daylight-saving rules.

## DateTime

`DateTime` represents a calendar date together with a time of day. Time is
represented with microsecond precision.

The class uses the Rexx base date internally for the calendar component. The
time component is the number of microseconds elapsed since midnight.

A `DateTime` can be created without arguments:

    now = .DateTime()

This creates a value containing the current local date and time.

The constructor can also be supplied with a Rexx base date and the number of
microseconds since midnight:

    base = date("BASE", "2026-08-24", "INTERNATIONAL") as .int
    value = .DateTime(base, 43200000000)

This represents noon on 24 August 2026.

The two components can be obtained with:

    baseDate()
    dayMicroseconds()

`daySeconds()` returns the number of complete seconds elapsed since midnight.

### Calendar Fields

The individual calendar fields are available through:

    year()
    month()
    day()

The time fields are available through:

    hour()
    minute()
    second()
    microsecond()

For example:

    value.year()
    value.month()
    value.day()
    value.hour()

return the corresponding fields as integers.

The ordinal day within the year is returned by:

    yearDay()

January 1 has ordinal day 1. December 31 therefore has ordinal day 365 in an
ordinary year and 366 in a leap year.

The textual names of the weekday and month are available through:

    dayName()
    monthName()

These methods use the underlying Rexx `DATE()` facilities.

### Calendar Properties

`isLeapYear()` tests whether the year containing the `DateTime` is a Gregorian
leap year:

    if value.isLeapYear() then
        say "leap year"

`daysInYear()` consequently returns either 365 or 366.

`daysInMonth()` returns the number of days in the month containing the value.
The calculation takes both the month and leap years into account.

For example, a `DateTime` in February 2024 reports 29 days, while one in
February 2025 reports 28.

### Date Representations

The methods:

    standardDate()
    internationalDate()

return representations produced using the corresponding Rexx `DATE()` formats.

`internationalDate()` is particularly useful when working with ISO 8601
calendar dates.

### Arithmetic

A `DateTime` can be changed by adding units of time:

    addMicroseconds(n)
    addSeconds(n)
    addMinutes(n)
    addHours(n)
    addDays(n)
    addWeeks(n)
    addYears(n)

The arithmetic methods modify the `DateTime`.

Negative arguments subtract time. Thus:

    value.addHours(-3)

moves the value three hours earlier.

Time arithmetic automatically crosses calendar-day boundaries. Adding one
microsecond to:

    23:59:59.999999

produces midnight on the following day. Subtracting one microsecond from
midnight produces:

    23:59:59.999999

on the preceding day.

`addYears()` performs calendar arithmetic rather than simply adding a fixed
number of days. In particular, adding one year to February 29 in a leap year
produces February 28 if the resulting year is not a leap year.

### Comparison

Two `DateTime` values can be compared with:

    compareTo(other)
    isBefore(other)
    isAfter(other)
    equals(other)

`compareTo()` returns a negative value, zero, or a positive value according to
whether the receiver is earlier than, equal to, or later than the supplied
value.

`equals()` tests both the calendar date and the time of day.

Because `DateTime` has no UTC offset, these comparisons compare local date and
time values. They should not be used to decide whether two values expressed
with different UTC offsets identify the same instant.

### ISO Week Dates

`DateTime` provides the ISO week calendar independently of the ordinary
calendar year.

The ISO weekday is returned by:

    weekDay()

Monday is weekday 1 and Sunday is weekday 7.

The ISO week number and ISO week-numbering year are returned by:

    weekNumber()
    weekNumberYear()

These values are deliberately separate. The ISO week year can differ from the
calendar year near New Year.

For example, 1 January 2016 belongs to ISO week 53 of 2015:

    Calendar date:    2016-01-01
    ISO week year:    2015
    ISO week number:  53

Similarly, 31 December 2018 belongs to ISO week 1 of 2019.

`weekNumberDate()` returns the complete ISO week-date representation, such as:

    2015-W53-5

`weeksInYear()` returns either 52 or 53 according to the number of ISO weeks in
the applicable ISO week year.

### String Representation

`toString()` returns the date and time in the form:

    YYYY-MM-DDThh:mm:ss.ffffff

For example:

    2026-08-24T20:57:57.123456

All six microsecond digits are included. This gives `DateTime` a predictable
string representation independent of choices made by more specialized ISO
8601 formatting.

## OffsetDateTime

`OffsetDateTime` combines a local `DateTime` with an explicit offset from UTC.

It is constructed from these two components:

    value = .OffsetDateTime(datetime, offset)

The offset is expressed in minutes. For example:

    .OffsetDateTime(datetime, 120)

represents the supplied local date and time at UTC+02:00, while:

    .OffsetDateTime(datetime, -330)

uses UTC-05:30.

The components are returned by:

    dateTime()
    offsetMinutes()

An offset of zero denotes UTC.

An `OffsetDateTime` stores the local representation as supplied. Parsing:

    2026-08-24T22:00:00+02:00

therefore stores 22:00 as its `DateTime` and 120 as its offset. It is not
silently converted to UTC during parsing.

### Conversion to UTC

`toUTC()` returns an equivalent `OffsetDateTime` with an offset of zero.

For example:

    2026-08-24T22:00:00+02:00

is converted to:

    2026-08-24T20:00:00Z

Similarly:

    2026-08-24T14:30:00-05:30

also converts to:

    2026-08-24T20:00:00Z

UTC conversion can cross a calendar-day boundary. For example:

    2026-08-24T01:00:00+02:00

becomes:

    2026-08-23T23:00:00Z

The original `OffsetDateTime` is not modified by `toUTC()`.

## ISO8601

`ISO8601` provides parsing and formatting of ISO 8601 representations.

There are separate parsing operations for values without an offset and values
with an explicit UTC offset. This distinction prevents offset information from
being silently discarded.

### Parsing Local Date and Time Values

`parse()` returns a `DateTime`.

Supported representations include:

    YYYY-MM-DD
    YYYY-MM-DDThh:mm:ss
    YYYY-MM-DDThh:mm:ss.f
    YYYY-MM-DDThh:mm:ss.ff
    ...
    YYYY-MM-DDThh:mm:ss.ffffff

A date without a time is interpreted as midnight:

    iso = .ISO8601()
    value = iso.parse("2026-08-24")

The resulting `DateTime` represents:

    2026-08-24T00:00:00.000000

Fractional seconds are converted to microseconds by padding on the right. Thus:

    .1       -> 100000 microseconds
    .123     -> 123000 microseconds
    .123456  -> 123456 microseconds

The current implementation supports at most six fractional digits, matching
the precision of `DateTime`.

Malformed input raises the `INVALID_ARGUMENTS` signal. This includes malformed
separators, incomplete times, values outside the supported clock ranges, and
fractional seconds exceeding the supported precision.

### Validating Input

`isValid()` tests whether a string can be parsed as a supported local ISO 8601
value.

For example:

    iso.isValid("2026-08-24")
    iso.isValid("2026-08-24T12:34:56")
    iso.isValid("2026-08-24T25:00:00")

return 1, 1, and 0 respectively.

`isValid()` uses the same parser as `parse()`. It catches the
`INVALID_ARGUMENTS` signal locally rather than maintaining a separate set of
validation rules.

### Parsing Values with UTC Offsets

`parseOffset()` parses ISO 8601 values that contain an explicit UTC offset and
returns an `OffsetDateTime`.

UTC can be written using `Z`:

    2026-08-24T20:00:00Z

or using a numeric offset:

    2026-08-24T22:00:00+02:00
    2026-08-24T14:30:00-05:30

Fractional seconds can also be used:

    2026-08-24T22:00:00.123456+02:00

The numeric offset is converted to minutes. Thus:

    Z       ->    0
    +02:00  ->  120
    -05:30  -> -330

The local date and time are preserved in the returned `OffsetDateTime`. The
offset is not applied until an explicit operation such as `toUTC()` requests
conversion.

This separation is important. An ISO 8601 value without an offset:

    2026-08-24T20:00:00

does not contain enough information to identify an instant in UTC. The library
therefore does not invent an offset for such a value.

Conversely, an explicit offset is never silently discarded.

### Formatting

`format()` formats a `DateTime` with microsecond precision:

    YYYY-MM-DDThh:mm:ss.ffffff

`formatDate()` formats only the calendar date:

    YYYY-MM-DD

`formatSeconds()` formats a date and time without fractional seconds:

    YYYY-MM-DDThh:mm:ss

`formatOffset()` formats an `OffsetDateTime`, including its UTC offset.

For example:

    2026-08-24T22:00:00.123456+02:00

An offset of zero is canonically formatted using `Z`:

    2026-08-24T20:00:00.000000Z

Non-zero offsets use signed hours and minutes:

    +02:00
    -05:30

Parsing and formatting can therefore be used for round trips:

    value = iso.parseOffset(
        "2026-08-24T22:00:00.123456+02:00")

    text = iso.formatOffset(value)

The resulting text is:

    2026-08-24T22:00:00.123456+02:00

## DateTime and OffsetDateTime

The distinction between `DateTime` and `OffsetDateTime` is fundamental to the
package.

A `DateTime` answers the question:

    What calendar date and clock time is this?

An `OffsetDateTime` additionally answers:

    What is its offset from UTC?

Consequently:

    2026-08-24T20:00:00

is a valid `DateTime`, but does not by itself identify a unique instant.

By contrast:

    2026-08-24T20:00:00Z

does identify an instant.

Different `OffsetDateTime` representations can identify that same instant:

    2026-08-24T20:00:00Z
    2026-08-24T22:00:00+02:00
    2026-08-24T14:30:00-05:30

Converting each to UTC produces the same value.

A UTC offset is not the same as a geographical time zone. The offset
`+02:00`, for example, contains no information about daylight-saving rules or
the geographical region in which it applies. Time-zone databases and named
time zones are therefore outside the responsibilities of these classes.
