# Optimization-boundary audit evidence — 18 September 2026

Authority: [audit, findings and proposed decisions](../../planning/release-1/optimization-boundary-audit-2026-09-18.md).
Source: `15c8a3ba42009ab8b5a9b447aa8c06ce86b9b392`, macOS ARM64 Debug.
Tool hashes and runtime selection are in [baseline.json](baseline.json).
No production optimizer changes were made.

This directory's original results describe the pre-repair audit. The
[subsequent local correction](repair/README.md) preserves that evidence and
records 102 passing affected tests plus the corrected original output.
Adrian accepted the RXC-only inline transport and RXAS status-bit boundaries
later on 18 September; those are no longer pending design decisions.

## Results

**OPT-BOUNDARY-01 was an open correctness defect at this baseline.** A valid assembly
procedure receives two aliased argument slots, writes through the second and
then converts the first to a string. Optimized RXAS removes the necessary
second `itos`; optimized execution prints `41`, versus `42` with `rxas -n`.
The fixture first exercises a separate global-mutation control, which prints
`42` in both modes. Thus its complete output is `42\n41\n` optimized and
`42\n42\n` unoptimized.

The exact inputs and disassemblies are retained in:

- [entry-alias.rxas](artifacts/entry-alias.rxas)
- [optimized disassembly](artifacts/entry-alias-opt.disassembly)
- [unoptimized disassembly](artifacts/entry-alias-noopt.disassembly)
- [RXAS proof diagnostic](artifacts/entry-alias-rxas-debug.log)

**OPT-BOUNDARY-02 documents the now-accepted RXC coherence obligation.** A deliberately
edited library contains executable `ret 99` but an unchanged inline AST for
`return 42`. Binary-only imports produce `42` in the optimized caller and `99`
in the `rxc -n` caller. The consistent-library control prints `42` in both
modes; removing the stale template prints `99` in both. This is not a claim
that normal compilation generates inconsistent libraries.

See [original library](artifacts/library-consistent.rxas),
[edited library](artifacts/library-stale.rxas),
[optimized caller](artifacts/caller-stale-opt.rxas), and
[unoptimized caller](artifacts/caller-stale-noopt.rxas).

The metadata-free existing handwritten fixture
`tests/rxas_optimizer/redundant_itos_runtime.rxas` passed in both modes, with
14 `itos` instructions unoptimized and 9 optimized. Final RXLink output
contained no inline template; `rxlink -i` preserved it in the checked library.

All **23 selected existing CTests passed**, including shared opcode-table and
flow-graph consistency, call/signal barriers, storage identities, relevant and
unrelated metadata/TRACE observations, repeated conversions, successful guards,
string-literal reuse, and malformed inline-summary fallbacks. The new alias
probe exposes a gap in that retained coverage; the green CTest result does not
close it. Direct `test_rxop_metadata` output was:

```text
opcode effects: total=660 source=590 classified=590 conservative=0 reserved=67 internal=3
```

## Commands and reproduction

The focused build succeeded:

```sh
cmake --build cmake-build-debug --target rxc rxas rxdas rxlink rxbvm test_rxop_metadata test_rxas_flow_graph --parallel 8
```

[build.log](build.log) retains the output. Existing tests used:

```sh
ctest --test-dir cmake-build-debug --output-on-failure --parallel 4 -R '^(rxas_optimizer_(metadata|whole_procedure_flow(_noopt)?|storage_identity_flow|redundant_itos_flow(_noopt)?|duplicate_link_read_(meta_relevant|meta_unrelated|trace_relevant|trace_unrelated|call_alias)|barrier_(call|dcall|signal)|successful_guard_flow(_noopt)?|string_literal_reuse(_noopt)?)|rxas_flow_graph_contract|inline_(summary_(version|shape|formal)|receiver_summary)_fallback_binary_opt)$'
```

Output: [focused-ctest.log](focused-ctest.log). No source/test/build input was
changed by a repair and no broad suite or sanitizer gate was run.

Run the bounded reproduction from the repository root after building those
tools:

```sh
python3 docs/qa/optimization-boundary-audit-2026-09-18/probe.py
```

The harness creates a temporary directory and prints its location. It uses the
current Debug tool binaries, excludes library source/assembly from binary-import
lookup and records each invocation's arguments, working directory, exit status,
stdout and stderr. It reports entry-alias semantic equivalence as a boolean:
**a zero harness exit is completion of the audit probes, not a passing verdict
for that defective behavior**. The expected correct output remains `42\n42`.

The completed run is retained as [summary.json](summary.json) and
[commands.json](commands.json). Temporary paths in the latter identify the
actual run; the script recreates new paths on replay. Text fixtures are retained
under `artifacts/`; generated executables are not checked in. These are dated
audit probes, not substitutes for a permanent product regression when the
defect is repaired.

Scope: local Debug source audit and focused execution on the selected switch
VM. No release, performance, sanitizer or cross-platform qualification is
claimed, and no architecture proposal has been approved by these results.
