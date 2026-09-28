# CodeQL #220: RXBIN graph-operand diagnostic format

Focused repair receipt, 28 September 2026. Work began at clean local
`87fcdd49e8954babf6c048c2b88722207b2f7d88`. During the focused checks,
the coordinator committed documentation-only `d40a626e0953aced0328a5a8aa5c40b3b83fc8b8`;
its two changed paths are the authoritative plan and the #226 review receipt.
It changed no code, test, or build input. The only product edit in this repair is
the format specifier in `binutils/rxbin007.c:1059`.

## Type and behavior inspection

`rxbin007_encode_instructions` declares `opcode` as `int` and `operand_index`
as `size_t` (`binutils/rxbin007.c:992-994`). The diagnostic reaches
`rxbin007_set_error(const char *format, ...)`, which forwards arguments to
`vsnprintf` (`:49-56`). The old `"%d:%d"` expected an `int` for the second
argument. The corrected `"%d:%zu"` matches both actual argument types and the
adjacent graph-operand diagnostics. The call to `rx_graph_resolve_operand`,
original cause text (`graph_error`), `free(graph_error)`, and failure return
are unchanged. No RXBIN data format, graph semantics, or success path changed.

The changed branch requires a semantic graph operand whose string cannot be
resolved against its graph. Existing graph tests do not assert this exact
writer-side diagnostic. There is no useful reachable large-index value to
force a visibly different `%d` result; this repair rests on the C variadic
type inspection plus the relevant existing graph and RXBIN behavior checks,
without adding a test that merely mirrors the new specifier.

## Focused qualification

Host: macOS ARM64. Both build trees are Debug; `cmake-build-debugasan` uses
`-fsanitize=address -fno-omit-frame-pointer`. Apple LeakSanitizer is not
supported, so the maintained runner used `--build-leaks off --leaks off`.

| Command | Result and log |
| --- | --- |
| `cmake --build cmake-build-debug --target test_rxgraph test_rxas_flow_graph rxdas_graph_error_artifact --parallel 8` | PASS; `/tmp/crexx-codeql220-debug-build.4sbetS`, SHA-256 `9158fda8566e015beba3f8d20e53b37fd4e4aa873fc03c5ea1bca22d63c5d945`. The graph-error artifact target prepares the compact roundtrip fixture. |
| `ctest --test-dir cmake-build-debug -R '^(rxgraph_unit\|rxas_flow_graph_contract\|rxdas_graph_error\|rxdas_roundtrip_compact_format)$' --parallel 1 --output-on-failure` | PASS 4/4; `/tmp/crexx-codeql220-debug-ctest.mKYx5k`, SHA-256 `292691f35191029839f34e56e88a01473aeb0f0f6e2c0f046f0b48ec57c5d44d`. |
| `tools/asan-run.sh --phase build --build-target test_rxgraph --build-target test_rxas_flow_graph --build-target rxdas_graph_error_artifact --build-leaks off --no-live-tail` | PASS; `cmake-build-debugasan/asan-logs/20260928-183457-build/build.log`, SHA-256 `2a172aecf4a4d96b5b45ac4a8429447ebb5724ec7bf45d707e0f0f96487d84bd`. |
| `tools/asan-run.sh --phase ctest --regex '^(rxgraph_unit\|rxas_flow_graph_contract\|rxdas_graph_error\|rxdas_roundtrip_compact_format)$' --leaks off --test-jobs 1 --no-live-tail` | PASS 4/4, no sanitizer diagnostics; `cmake-build-debugasan/asan-logs/20260928-183509-ctest/ctest.log`, SHA-256 `bafa146607fce7501ac66203a3a380f37ad1a73d240ad3747a6a34b7f7c70da7`. |

`rxgraph_unit` checks graph operand resolution; the compact roundtrip checks
RXBIN writing and reading; `rxdas_graph_error` checks malformed-graph
rejection in the reader, not the changed writer diagnostic; and
`rxas_flow_graph_contract` checks related graph behavior. These are
proportional regressions, not a claim of direct dynamic coverage of the rare
diagnostic branch. `git diff --check` passed. No broad test was repeated.

## Publication boundary

The already-running hosted sanitizer matrix targets published
`9f2f44cfd888d324858769809b0381e524850b4c`; it does not test this
subsequent product edit. Its result remains valid only for that named input.
The local focused Debug/Apple ASan checks qualify this one-line change.
After coordinator review and publication, automatic normal Build and CodeQL
on the resulting exact head should verify platform compilation and whether
alert #220 is resolved. If AC-08 requires an exact-final-source combined
Linux ASan/LSan plus macOS ASan verdict, that remains an open coordinator
gate; this receipt neither relabels the running matrix nor dispatches another.
