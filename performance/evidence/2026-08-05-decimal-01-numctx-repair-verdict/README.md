# DECIMAL-01 numeric-context repair verdict

Date: 2026-08-05

Status: **focused correctness repair accepted by Adrian on 2026-08-05**

## Scope and boundary

This bundle records the user-authorized repair of the three failures retained
in the opening evidence:

1. `NUMERIC FUZZ` was stored but ignored by decimal comparisons in both
   providers;
2. `db_decimal` bypassed scientific/engineering form and exponent case; and
3. `db_decimal` always rounded halfway values away from zero instead of
   selecting Common half-even versus Classic half-up.

The repair preserves the decimal plugin ABI, RXAS encoding and stored numeric
context. It changes provider consumers and focused tests only. No candidate,
default-provider change, performance measurement, broad CTest, sanitizer run
or Gate 0 closeout is included.

## Source and host

- worktree: `/Users/adrian/CLionProjects/CREXX-decimal-performance`
- branch: `codex/decimal-performance`
- baseline HEAD: `4813e98d1dca1ac77d5899dd6c5787e4b83f4772`
- source scope: the DECIMAL-01 control, tests, evidence and repair introduced
  from the named baseline and committed together on the private branch
- host: Apple M5, 10 logical CPUs
- OS: macOS 26.5.2 build 25F84, Darwin 25.5.0 arm64
- compiler: Apple clang 21.0.0 (`clang-2100.1.1.101`)
- build system: CMake 4.3.2, Ninja 1.13.2
- Debug and Release: single-config Ninja, `CREXX_VM_PROFILING=OFF`

These are correctness runs. Their elapsed times are not evidence and are not
interpreted.

## Selected implementation

- Both providers round temporary comparison operands to effective
  `DIGITS - FUZZ`; stored operands and ordinary arithmetic precision are not
  changed.
- `mc_decimal` uses temporary arbitrary-precision `decNumber` values under a
  temporary comparison precision and restores the arithmetic context.
- `db_decimal` explicitly implements Common half-even and retains Classic
  half-up/away rounding, including the former `exponent_shift == 0` gap.
- `db_decimal` sends normalized coefficient/exponent components through the
  framework-owned REXX formatter rather than `%LG`.

## Verdict

| Build | Focused CTest | Observable matrix | Result |
| --- | ---: | ---: | --- |
| Debug, profiling off | 9/9 | 6/6 | pass |
| Release, profiling off | 9/9 | 6/6 | pass |

The focused CTest selection covers the two provider suites, optimized/no-opt
compiler context lowering and all default/explicit provider combinations on
both VMs. The separately named observable program covers FUZZ equal/distinct
boundaries, register/register and register/literal comparison, scientific/
upper and engineering/lower output, Common/Classic halfway behavior, and
caller/child context isolation.

The confirmed defects are repaired for the focused contract. Adrian accepted
this verdict on 2026-08-05 and directed the programme to its second stage,
Gate 1 current-provider baseline preparation. Broader official vectors,
signal/status boundaries, Classic quotient rules, high precision, sanitizers
and supported platforms remain explicit correctness work; no timing may start
without the shared host being cleared and reserved.

## Identities

```text
e18b7d450e63ad6d49a825072fa18dead04fcabf0b1e29900878a68de5e1ed84  mc_decimal.c
b2df69da1b2dc55ab9df9156386afc4ba729850f224e49199cb045720b7eb29d  db_decimal.c
51d0da222d666e9c5b40de63b72fa3d9f45ee92eb1c0e441211ee7215f9fe839  db_decimal_tests.c
9f79f17cb60c0a101aaf97f734d3478903760e920f279bd074a6a1af4486feb9  tests_decimal.rxas
047abc4c1315eb52873f2be24c95e644ed709f554a4094910cfdc5acdec01aac  rxas_numeric_context_observable.rxas
a5aae20d1e3f8d5af8f3bbe2f2a66f87febcae6d47c1fc8617f6ffcb1e7dd3a9  Release rxvm
baa436468be9b66bf8165aacb2f324c0563825e0945a4105cd1eb53b18f70ed7  Release rxbvm
467ec67cff807facb8f6fccec4db862448642acdeb166ba5e4045e2c02515d79  Release mc_decimal plugin
7d071124fc181e1af496304f1ab76eb9ea8b88f7030369c5f6d696d364beabae  Release db_decimal plugin
```
