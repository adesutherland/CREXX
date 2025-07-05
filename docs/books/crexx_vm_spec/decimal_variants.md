# Decimal instruction implementation variants

The preceding set of decimal instructions is implemented twice, in VM Plugins which can be switched out at runtime to determine the requested behaviour. One set implements the classic \rexx{} ANSI X3.274-1996 unlimited precision decimal floating point, which corresponds to the IEEE 754-2008 revision. The other set of instruction implementations is new for \crexx{} and contains the implementation of the rxvmplugin plugin using (platform-instruction set architecture specific) long double precision floating point numbers.

## md decimal




## db decimal

| Arch    | OS # | `long double` Size | Format Used            | Precision (bits) | Decimal Precision |
|---------|------|--------------------|-------------------------|------------------|-------------------|
| x86_64  | 1    | 16 bytes           | x87 80-bit extended     | ~64              | ~19–20 digits     |
| x86_64  | 2    | 16 bytes           | IEEE 128-bit            | 113              | ~33–34 digits     |
| x86_64  | 3    | 16 bytes           | IEEE 128-bit (emulated) | 106–113          | ~31–33 digits     |
| x86_64  | 4    | 8 bytes            | IEEE 64-bit (double)    | 53               | ~15–17 digits     |
| ARM64   | 5    | 16 bytes           | IEEE 128-bit            | 113              | ~33–34 digits     |
| ARM64   | 6    | 8 bytes            | IEEE 64-bit (double)    | 53               | ~15–17 digits     |
| s390x   | 5    | 16 bytes           | IEEE 128-bit            | 113              | ~33–34 digits     |

Table: db decimal precision. {#tbl:id}

| OS # | OS Description              |
|------|-----------------------------|
| 1    | Linux (glibc, e.g. Ubuntu, Debian, Fedora) |
| 2    | Linux (musl, e.g. Alpine)   |
| 3    | macOS (x86_64)              |
| 4    | Windows (x86_64, MSVC)      |
| 5    | Linux (glibc, including s390x and ARM64) |
| 6    | macOS (ARM64)               |

Table: db decimal precision: OS legenda. {#tbl:id}

| Arch/OS# | Notes                                                                 |
|----------|-----------------------------------------------------------------------|
| x86_64/1 | True x87 80-bit precision; ABI aligns to 16 bytes; used by GCC        |
| x86_64/2 | `long double` uses IEEE binary128 via software (libquadmath)          |
| x86_64/3 | Clang supports IEEE 128-bit emulation; not hardware backed            |
| x86_64/4 | `long double` is just an alias for `double` in MSVC                   |
| ARM64/5  | GCC maps `long double` to binary128 if supported                      |
| ARM64/6  | No `__float128` support in Clang; `long double` equals `double`       |
| s390x/5  | Hardware-backed IEEE 128-bit (z13 and newer)                          |

Table: db decimal precision: architecture notes. {#tbl:id}
