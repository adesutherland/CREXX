# `arrayfind` (Level B)

```rexx
arrayfind(needle = .string,
          array = .string[],
          from = .int optional,
          case_sensitive = .int optional) = .int
```

`arrayfind` returns the first one-based index at or after `from` whose element
contains `needle`, or zero when no element matches. The string-array convention
uses `array[0]` as the high-water mark. `from` defaults to one; a value below
one clamps to one, while a value above the high-water mark returns zero.

`case_sensitive` defaults to `1`. Supply `0` for Unicode uppercase-folded
matching. Any other flag raises `INVALID_ARGUMENTS`. An empty needle returns
zero, matching the Level B `pos` contract.

The implementation reads but does not mutate the array. It uses direct VM
substring search, folds the needle once for insensitive searches, and makes no
other selector calls. The focused harness is
`lib/rxfnsb/tests_functional/ts_arrayfind.crexx`.
