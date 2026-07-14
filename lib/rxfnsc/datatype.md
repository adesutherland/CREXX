# DATATYPE — Classic Level C

```text
.rexxclassicbifdatatype..rexxclassicbif_datatype(
    context = reference .RexxBifCallContext) = .RexxValue
```

The call context represents `DATATYPE(string [,type])` with checklist
`rANY oABLMNSUWX`. With no option, the BIF returns `NUM` or `CHAR`; with an
option it returns `1` or `0`. Only the first option character is significant
and is normalized through normal Level C option handling. `D` is not a Classic
option; it remains a Level B and `.Rexx` extension.

The shared `RexxClassicDatatype` engine also backs CheckArgs `NUM`, `WHOLE`,
`BIN`, `HEX`, and `SYM`, plus the direct `SYMBOL` BIF. A spelling accepted by
one of those rules therefore cannot drift from DATATYPE's corresponding
classification.

Character classification comes from the active `RexxClassicConfig` profile:

- `BYTE` scans exact bytes and accepts arbitrary byte values as inputs;
- `UTF8` scans Unicode codepoints and rejects binary-only invalid UTF-8 with
  `23.1`;
- paired extra lowercase/uppercase tables extend `A`, `L`, `M`, `S`, and `U`;
- extra digit tables contain one or more consecutive ten-character families in
  `0` through `9` order and extend `A`, `B`, `N`, `S`, `W`, and `X`;
- B/X grouping uses space plus the profile's configured blank characters, with
  no leading/trailing blank and right-counted groups of four or two digits;
- the configurable exponent-digit limit defaults to nine. DATATYPE `N`/`W`
  return `0` beyond it; CheckArgs `NUM` reports `40.9`.

Number parsing normalizes configured digits to ASCII without float conversion.
`W` is exact and does not use `NUMERIC FUZZ` or a tolerance. CheckArgs `NUM` and
`WHOLENUM` retain the caller numeric context; the strict `WHOLE` rules normalize
only signed whole-digit spellings for BIF integer arguments.

Argument count, omission, empty option, and invalid option errors use the
standard `40.3`, `40.4`, `40.5`, `40.21`, and `40.28` identities. The function
is directly callable and does not enter the deprecated name dispatcher.
