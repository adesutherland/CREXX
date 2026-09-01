# CRI-12 closeout

Status: **documented/package-closed**

Date: 2026-07-30

## Reproducer and diagnosis

The minimized Level B reproducer is
`/tmp/crexx-cri12-repro.v3xDwq/cri12_array_lifecycle.crexx`, SHA-256
`7cb1a12b836f56c358adb3cf614510ea63e54ebe63f7dc0d71c2478de360d87e`.
It was compiled optimized and non-optimized and executed with both `rxvm` and
`rxbvm` from the dedicated candidate Debug product. All 4/4 executions printed
`PASS: ADDRESS array lifecycle`. The combined raw log is
`/tmp/crexx-cri12-repro.v3xDwq/commands-and-results.log`, SHA-256
`eefce7116de18469fa16e9ba9deb9bf3a2081c92bbf1dea18a63d94eb2fc1efd`.

Observed behavior is consistent and intentional:

- the first redirected record is appended at element 1;
- reuse appends later records after the existing high-water mark;
- an empty stream appends nothing and therefore preserves existing elements;
- a failed command follows the same per-stream rule: its error records append,
  while an empty output stream leaves its output array unchanged; and
- `arraydrop` resets the caller-owned array before replacement-style reuse.

The implementation agrees with the documented Level B array model.
`_redir2array` exposes the caller-owned `.string[]` to the VM redirect endpoint,
and `Output2ArrayThread` uses `add_new_element` for each record; neither layer
performs an implicit clear. `arraydrop` is the public in-place clear operation.
The existing NR-16/17 ADDRESS benchmark already calls `arraydrop` before every
replacement-style capture, providing an independent maintained usage example.

An implicit reset would discard a supported accumulation use and would make
redirection unlike other explicit array mutation. The reported gap was that
this lifecycle was not stated in the ADDRESS reference or locked by a focused
regression, not a contradiction in runtime semantics.

## Maintained closure

`lib/rxfnsb/tests_functional/ts_address_array_lifecycle.crexx` is the maintained
Level B contract. It covers first capture, append reuse, empty output before and
after `arraydrop`, failure status, empty failure output, failure-error append,
and failure reuse after `arraydrop`.

The statements reference now says that array destinations are append targets,
that empty streams and failures do not implicitly clear either stream array,
and that callers must apply `arraydrop` independently to `OUTPUT` and `ERROR`
when they want replacement semantics.

| Maintained artifact | SHA-256 |
| --- | --- |
| `ts_address_array_lifecycle.crexx` | `fea5df83e9548c26583a28ec7d6ebd4af57b65b5f0e7c95f767fe397c539063a` |
| functional CMake registration | `242a966a168da752dfba354dee6e3430c7fea61f71f9adf77a2e85d5aad79ba8` |
| `statements.md` | `2da44c1f07a4869158f1273d0c3c8a00203b8f3d59a1275e4b4251688f5ed6aa` |

Focused commands:

```sh
cmake --build /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  --target ts_address_array_lifecycle_noopt_artifact \
           ts_address_array_lifecycle_opt_artifact --parallel 10
ctest --test-dir /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  -R '^ts_address_array_lifecycle' --parallel 10 --output-on-failure
```

The exact inventory is five tests: the linked-runtime fixture plus optimized
and non-optimized execution on each VM. All 5/5 pass, zero failed or skipped.
Raw logs and hashes:

- `/tmp/cri12-focused-build.log`:
  `eab78ed67292d1df157f3199a75f5e81152aff8d146ff1a7816df3de28d0b528`;
- `/tmp/cri12-focused-list.log`:
  `3b89a95def309466486c8bfd7023d30dbf0a346b70094b1782b133d61bcfa667`;
- `/tmp/cri12-focused-ctest.log`:
  `9cc25dd108ce346c9208bb47ed275a45c2476310766bfc78ebdfcb2f13b9741d`.

No C/C++ runtime or library code changed, so this documentation/test closure
does not create a new affected native surface requiring a sanitizer rerun. The
final programme-wide Debug and sanitizer gates remain pending. `git diff
--check` passes.

## Compatibility and risks

No production behavior, syntax, public API, native ABI, RXAS/RXBIN or serialized
format changed. Existing accumulation users retain their current behavior.
Replacement-style users now have an explicit, tested requirement to call
`arraydrop`.

Redirection is incremental rather than transactional: if an environment emits
records and later fails, the emitted records remain appended and `rc` reports
the failure. Callers that require rollback must capture into a fresh array and
adopt it only after checking `rc`.
