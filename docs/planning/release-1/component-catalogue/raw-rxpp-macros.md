# Raw RXPP macro and source-inclusion library surfaces

Stage 2 discovery data. These rows describe definitions in the four support
files that RXPP stages and installs. They do not imply a language level,
product role, delivery recommendation, or quality judgement.

## Function-like and command macros

| ID | Support file | Public macro | Form | Parameters | Source |
|---|---|---|---|---|---|
| `MACRO-maclib-cube` | `maclib` | `CUBE` | function-like macro | `x` | `preprocessor/maclib.rexx:4` |
| `MACRO-maclib-square` | `maclib` | `SQUARE` | function-like macro | `x` | `preprocessor/maclib.rexx:5` |
| `MACRO-maclib-double` | `maclib` | `double` | function-like macro | `x` | `preprocessor/maclib.rexx:6` |
| `MACRO-maclib-hi` | `maclib` | `hi` | function-like macro | `stem` | `preprocessor/maclib.rexx:9` |
| `MACRO-maclib-mapput` | `maclib` | `mapput` | function-like macro | `map,k,v` | `preprocessor/maclib.rexx:10` |
| `MACRO-maclib-mapget` | `maclib` | `mapget` | function-like macro | `map,k` | `preprocessor/maclib.rexx:11` |
| `MACRO-maclib-repeat` | `maclib` | `repeat` | function-like macro | `n` | `preprocessor/maclib.rexx:14` |
| `MACRO-maclib-guard` | `maclib` | `guard` | function-like macro | `cond,act` | `preprocessor/maclib.rexx:15` |
| `MACRO-maclib-setstem` | `maclib` | `SetStem` | variadic function-like macro | `name,...` | `preprocessor/maclib.rexx:18` |
| `MACRO-maclib-foreach` | `maclib` | `foreach` | function-like macro | `stem,indx` | `preprocessor/maclib.rexx:20` |
| `MACRO-maclib-forpair` | `maclib` | `forpair` | function-like macro | `array,key,val` | `preprocessor/maclib.rexx:21` |
| `MACRO-maclib-swap` | `maclib` | `swap` | function-like macro | `a,b` | `preprocessor/maclib.rexx:26` |
| `MACRO-maclib-readfile` | `maclib` | `readfile` | function-like macro | `stem,file` | `preprocessor/maclib.rexx:29` |
| `MACRO-maclib-writefile` | `maclib` | `writefile` | function-like macro | `file,stem` | `preprocessor/maclib.rexx:30` |
| `MACRO-maclib-ltrim` | `maclib` | `ltrim` | function-like macro | `str` | `preprocessor/maclib.rexx:33` |
| `MACRO-maclib-rtrim` | `maclib` | `rtrim` | function-like macro | `str` | `preprocessor/maclib.rexx:34` |
| `MACRO-maclib-startswith` | `maclib` | `startswith` | function-like macro | `suffix,string` | `preprocessor/maclib.rexx:36` |
| `MACRO-maclib-endswith` | `maclib` | `endswith` | function-like macro | `suffix,string` | `preprocessor/maclib.rexx:37` |
| `MACRO-maclib-notempty` | `maclib` | `notEmpty` | function-like macro | `str` | `preprocessor/maclib.rexx:39` |
| `MACRO-maclib-ispunct` | `maclib` | `isPunct` | function-like macro | `str` | `preprocessor/maclib.rexx:40` |
| `MACRO-maclib-charat` | `maclib` | `charAt` | function-like macro | `str,pos` | `preprocessor/maclib.rexx:41` |
| `MACRO-maclib-islonger` | `maclib` | `isLonger` | function-like macro | `str,len` | `preprocessor/maclib.rexx:43` |
| `MACRO-maclib-isshorter` | `maclib` | `isShorter` | function-like macro | `str,len` | `preprocessor/maclib.rexx:44` |
| `MACRO-maclib-equals` | `maclib` | `equals` | function-like macro | `a,b` | `preprocessor/maclib.rexx:45` |
| `MACRO-maclib-equalsfold` | `maclib` | `equalsFold` | function-like macro | `a,b` | `preprocessor/maclib.rexx:46` |
| `MACRO-maclib-contains` | `maclib` | `contains` | function-like macro | `str,sub` | `preprocessor/maclib.rexx:48` |
| `MACRO-maclib-clamp` | `maclib` | `clamp` | function-like macro | `val,lo,hi` | `preprocessor/maclib.rexx:50` |
| `MACRO-maclib-between` | `maclib` | `between` | function-like macro | `n,a,b` | `preprocessor/maclib.rexx:52` |
| `MACRO-maclib-inrange` | `maclib` | `inRange` | function-like macro | `n,a,b` | `preprocessor/maclib.rexx:53` |
| `MACRO-maclib-isleapyear` | `maclib` | `isLeapYear` | function-like macro | `y` | `preprocessor/maclib.rexx:55` |
| `MACRO-maclib-debug` | `maclib` | `debug` | function-like macro | `expr` | `preprocessor/maclib.rexx:58` |
| `MACRO-maclib-comment` | `maclib` | `comment` | function-like macro | `msg` | `preprocessor/maclib.rexx:59` |
| `MACRO-maclib-traceblock` | `maclib` | `traceblock` | function-like macro | `name` | `preprocessor/maclib.rexx:60` |
| `MACRO-maclib-tracevalue` | `maclib` | `traceValue` | function-like macro | `v` | `preprocessor/maclib.rexx:62` |
| `MACRO-maclib-cparse` | `maclib` | `cparse` | multiline function-like macro | `string,template` | `preprocessor/maclib.rexx:64` |
| `MACRO-macsys-log` | `macsys` | `log` | function-like macro | `msg` | `preprocessor/macsys.rexx:2` |
| `MACRO-macsys-argv` | `macsys` | `argv` | function-like macro | `stem` | `preprocessor/macsys.rexx:4` |
| `MACRO-macsys-strlen` | `macsys` | `strlen` | function-like macro | `len,strg` | `preprocessor/macsys.rexx:5` |
| `MACRO-macsys-fastpos` | `macsys` | `fastpos` | function-like macro | `into,srch,string` | `preprocessor/macsys.rexx:6` |
| `MACRO-macsys-execio` | `macsys` | `execio` | command macro | `num DISKX file keyword stem` | `preprocessor/macsys.rexx:8` |
| `MACRO-macsys-iseven` | `macsys` | `isEven` | function-like macro | `n` | `preprocessor/macsys.rexx:11` |
| `MACRO-macsys-isodd` | `macsys` | `isOdd` | function-like macro | `n` | `preprocessor/macsys.rexx:12` |
| `MACRO-macsys-ispositive` | `macsys` | `isPositive` | function-like macro | `n` | `preprocessor/macsys.rexx:13` |
| `MACRO-macsys-isnegative` | `macsys` | `isNegative` | function-like macro | `n` | `preprocessor/macsys.rexx:14` |
| `MACRO-macsys-iszero` | `macsys` | `isZero` | function-like macro | `n` | `preprocessor/macsys.rexx:15` |
| `MACRO-macsys-sign` | `macsys` | `sign` | function-like macro | `n` | `preprocessor/macsys.rexx:18` |
| `MACRO-macsys-pi` | `macsys` | `pi` | function-like macro | none | `preprocessor/macsys.rexx:19` |
| `MACRO-macsys-euler` | `macsys` | `euler` | function-like macro | none | `preprocessor/macsys.rexx:20` |
| `MACRO-macsys-ifnull` | `macsys` | `ifNull` | function-like macro | `val,fallback` | `preprocessor/macsys.rexx:23` |
| `MACRO-macsys-xor` | `macsys` | `xor` | function-like macro | `a,b` | `preprocessor/macsys.rexx:24` |
| `MACRO-macsys-info` | `macsys` | `info` | function-like macro | `msg` | `preprocessor/macsys.rexx:26` |
| `MACRO-macsys-error` | `macsys` | `error` | function-like macro | `msg` | `preprocessor/macsys.rexx:27` |
| `MACRO-macsys-warn` | `macsys` | `warn` | function-like macro | `msg` | `preprocessor/macsys.rexx:28` |
| `MACRO-macsys-isdigit` | `macsys` | `isDigit` | function-like macro | `string` | `preprocessor/macsys.rexx:30` |
| `MACRO-macsys-isalpha` | `macsys` | `isAlpha` | function-like macro | `string` | `preprocessor/macsys.rexx:31` |
| `MACRO-macsys-isblank` | `macsys` | `isBlank` | function-like macro | `str` | `preprocessor/macsys.rexx:32` |
| `MACRO-macsys-isupper` | `macsys` | `isUpper` | function-like macro | `str` | `preprocessor/macsys.rexx:33` |
| `MACRO-macsys-islower` | `macsys` | `isLower` | function-like macro | `str` | `preprocessor/macsys.rexx:34` |
| `MACRO-macsys-isalnum` | `macsys` | `isAlnum` | function-like macro | `str` | `preprocessor/macsys.rexx:35` |
| `MACRO-macsys-isspace` | `macsys` | `isSpace` | function-like macro | `str` | `preprocessor/macsys.rexx:36` |
| `MACRO-macsys-isempty` | `macsys` | `isEmpty` | function-like macro | `str` | `preprocessor/macsys.rexx:37` |
| `MACRO-macsys-quote` | `macsys` | `quote` | function-like macro | `string2quote` | `preprocessor/macsys.rexx:39` |
| `MACRO-macsys-dquote` | `macsys` | `Dquote` | function-like macro | `string2quote` | `preprocessor/macsys.rexx:40` |
| `MACRO-macsys-clear` | `macsys` | `clear` | command macro | `array` | `preprocessor/macsys.rexx:42` |

Discovered macro definitions: **64**.

## `##USE`-included procedures

| ID | Support file | Procedure | Result | Parameters | Source |
|---|---|---|---|---|---|
| `RXPPPROC-mathlib-gcd` | `mathlib` | `gcd` | `.int` | `a=.int, b=.int` | `preprocessor/mathlib.rexx:10` |
| `RXPPPROC-mathlib-lcm` | `mathlib` | `lcm` | `.int` | `a=.int, b=.int` | `preprocessor/mathlib.rexx:22` |
| `RXPPPROC-mathlib-isprime` | `mathlib` | `isPrime` | `.int` | `n=.int` | `preprocessor/mathlib.rexx:29` |
| `RXPPPROC-mathlib-factorial` | `mathlib` | `factorial` | `.int` | `n=.int` | `preprocessor/mathlib.rexx:42` |
| `RXPPPROC-mathlib-pow` | `mathlib` | `pow` | `.float` | `base=.float, exp=.float` | `preprocessor/mathlib.rexx:54` |
| `RXPPPROC-mathlib-modinv` | `mathlib` | `modinv` | `.int` | `a=.int, m=.int` | `preprocessor/mathlib.rexx:61` |

Discovered included procedures: **6**.

`syslib.rexx` is an installed placeholder with no procedure or macro leaf. It
is retained as a package-level row in `raw-packages-and-demos.md`.

Total public RXPP support-file leaves: **70**.
