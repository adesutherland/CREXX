# CREXX Smoke Tests

The local smoke suite is selected with:

```bash
ctest --test-dir cmake-build-debug -L smoke --output-on-failure --parallel 10 --timeout 600
```

As of the 2026-07-09 review, the smoke label selects 108 tests. The set is
intentionally broad but not exhaustive: it should catch broken developer builds
quickly without replacing the full CTest suite.

## Selection Rules

- Cover every shipped toolchain product: `rxc`, `rxas`, `rxlink`, `rxdas`,
  `rxvm`/`rxbvm`, `rxpp`, `rexxscript`, and `crexx`.
- Cover the main runtime surfaces: base BIFs, classlib, `rxfnsc`, `rxfnsg`,
  `rxfnsl`,
  native-backed adapters, RXPA plugins, SAA entry points, ADDRESS, source maps,
  diagnostics, signals, and native packaging.
- Prefer one representative per matrix axis. Do not label both `noopt` and
  `opt`, both `rxvm` and `rxbvm`, or every source/binary import variant unless
  that axis is the concern being tested.
- Avoid smoke tests that require the global `linked_opt_runtime_artifacts`
  fixture. That fixture can build a large part of the suite and defeats quick
  turnaround.
- Keep performance tests, deprecated slow plugins, broad syntax-highlighting
  matrices, and exhaustive optimizer cases in the full suite.

## Coverage Review

| Product or concern | Representative smoke coverage |
| --- | --- |
| `rxc` compiler | help, diagnostics, representative noopt/opt goldens, errors, imports, interfaces, optional args, Level C, source maps |
| `rxas` assembler | parser init/token/error tests, invalid mnemonic diagnostic, optimizer copy/acopy case |
| `rxlink` linker | success, control-file linking, interface linking, format checks, signature mismatch diagnostics |
| `rxdas` disassembler | basic/interface roundtrip and dump support |
| `rxvm` / `rxbvm` | basic RXAS execution, string/reference/decimal/interface instructions, compact format, signal and UTF checks |
| Libraries | rxfnsb ADDRESS/parse/stem/JSON/socket, rxfnsc runtime pools/classic BIFs/value, rxfnsg provider helpers, rxfnsl generated-output tinyexpr demo |
| Classlib | list/hash map/object collections, JSON, native OS adapter |
| Plugins and RXPA | math/stack/string/keyaccess/map/system plugins, dynamic/static RXPA link, class declarations, callbacks, native payload, multi-plugin loading |
| Host integration | SAA cache, `crexxsaa` variables/status, ADDRESS bridge/callback, dynamic loading |
| Front-end tools | `rxpp` smoke/srcmap/tokens/diagnostics, RexxScript runtime/compat/CLI, `crexx` driver/native packaging |

## Maintenance

When adding a high-value regression test, add `smoke` only if it covers a new
product path or a historically fragile concern. If it is one more member of an
existing matrix, keep it out of smoke and leave it in the full suite.
