# CRI-10 closeout

Status: **documented/package-closed**

Date: 2026-07-30

## Reproducer and diagnosis

The minimized Level B reproducer is
`/tmp/crexx-cri10-repro.cIcU6I/cri10_address_capture.crexx`, SHA-256
`7594a7d16ba4cfa8ae0ddacc1b3d3ba19ef0b66971cfc8cc8f9a300376e5ec08`.
It was compiled optimized and non-optimized, then executed with `rxvm` and
`rxbvm` from the dedicated candidate Debug product:

```sh
rxc -i "$candidate/bin" -n -o cri10_noopt cri10_address_capture.crexx
rxas -n -o cri10_noopt cri10_noopt
rxc -i "$candidate/bin" -o cri10_opt cri10_address_capture.crexx
rxas -o cri10_opt cri10_opt
rxvm  cri10_noopt "$candidate/bin/library"
rxbvm cri10_noopt "$candidate/bin/library"
rxvm  cri10_opt   "$candidate/bin/library"
rxbvm cri10_opt   "$candidate/bin/library"
```

All four executions printed `PASS: ADDRESS capture contract`. The raw combined
log is `/tmp/crexx-cri10-repro.cIcU6I/commands-and-results.log`, SHA-256
`244450ce64f2d6d67d44385b33e4053f588f2426539a305ea8cff657b294cceb`.

The existing implementation is complete:

- the certified ADDRESS exit selects `_redir2string` for a `.string` and
  `_redir2array` for a `.string[]`;
- scalar capture retains stream text and line terminators;
- array capture splits newline-delimited records and removes the terminator;
- stdout and stderr use independent endpoints;
- the environment response writes the command status to `rc`; and
- valid Level B Unicode and emitted punctuation remain data rather than being
  reinterpreted by capture.

The gap was public documentation and exact combined regression coverage, not a
missing facade or interpreter semantic.

## Maintained closure

`lib/rxfnsb/tests_functional/ts_address_capture.crexx` is the maintained Level
B contract. It covers multiline stdout in scalar and array forms, multiline
stderr, separate stdout/stderr, success and failure status, fresh empty output,
Unicode, quotes, brackets, pipes, semicolons and other embedded delimiters. It
uses only deterministic built-in CREXX commands and a process-unique temporary
directory.

The language reference now defines scalar versus array capture, independent
streams, `rc`, fresh empty destinations, Unicode/delimiter behavior and the
boundary between capture representation and array reuse.

| Maintained artifact | SHA-256 |
| --- | --- |
| `ts_address_capture.crexx` | `ffc40f8041cac4e4934120a2ac07a3d3b788c68bb45ff394e8f67df3ef4ffa97` |
| functional CMake registration | `4731b80d86d07f471fce27e5ec0dbd649b25e1696d91672f8c76b6df5bf355ed` |
| `statements.md` | `4ca8391847a0ff5a664f8033283f0d5b921c878d2883f3402bb9379f09410bc1` |

Focused commands:

```sh
cmake --build /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  --target ts_address_capture_noopt_artifact \
           ts_address_capture_opt_artifact --parallel 10
ctest --test-dir /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  -R '^ts_address_capture' --parallel 10 --output-on-failure
```

The exact inventory is five tests: the linked-runtime fixture plus optimized
and non-optimized execution on each VM. All 5/5 pass, zero failed or skipped.
Raw logs and hashes:

- `/tmp/cri10-focused-build.log`:
  `1f6e429a5de2412cea49ca663950ad28ef7ae561f429bf97cd0925c3901c499a`;
- `/tmp/cri10-focused-list.log`:
  `0f06908225ddcc1ae622fa200a53d16f8fc12aaaeaa0078bfa8de581d243c776`;
- `/tmp/cri10-focused-ctest.log`:
  `7de2b3df54a9c23bfc0fd67a0d4a9516e09219e4685ce13bdc0e502b2776fd0b`.

The immediately preceding accepted production state passed complete Debug
CTest 1,943/1,943. CRI-10 changes only documentation and adds a Level B test;
the final programme-wide full Debug suite will include this new matrix.
`git diff --check` passes.

## Compatibility and remaining boundary

No production code, syntax, facade, native ABI, RXAS/RXBIN or serialized format
changed. Captured text remains environment-defined text; platform/environment
line-ending differences remain visible in scalar capture. Invalid UTF-8 is not
Level B text and belongs on a binary/native payload path.

This item deliberately uses fresh capture destinations. Reusing a populated
array and whether `arraydrop` is required is CRI-12. Argument-vector command
dispatch is CRI-11. Neither is silently decided by this closeout.
