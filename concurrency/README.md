# cREXX concurrency workspace

This directory is the control plane for cREXX concurrency work. Concurrency is
a language, library, toolchain and VM capability; it is not a subdivision of
the performance programme. Performance evidence may inform a concurrency
decision, but `performance/` does not own concurrency scope or publication.

## Authorities

Use these sources in order:

1. current code and executable tests;
2. the enduring maintainer reference in
   `docs/ai-context/CREXX_CONCURRENCY.md`;
3. the language, programming and library books under `docs/books/`;
4. [`WORKLIST.md`](WORKLIST.md) for current status and remaining work;
5. [`TEST-MANIFEST.md`](TEST-MANIFEST.md) for the maintained executable matrix
   and deterministic entry point;
6. [`QA-CLOSEOUT.md`](QA-CLOSEOUT.md) for the frozen Release 1 review and
   platform-validation ledger;
7. [`IMPLEMENTATION-STATUS.md`](IMPLEMENTATION-STATUS.md) for the current
   source-to-test truth matrix;
8. [`DECISIONS.md`](DECISIONS.md) for accepted design boundaries; and
9. [`history/`](history/) and `performance/evidence/` for dated provenance.

The historical records contain the former Gate E/F names. Those names identify
the development sequence only; they are not user-facing feature names or
current worklist states.

## Status vocabulary

- **implemented** means the current source contains the capability and focused
  executable tests cover its intended contract;
- **locally qualified** means the recorded Mac Debug, Release and sanitizer
  closeout passed;
- **portable** means the required Linux, Windows and Mac provider behavior has
  passed the current conformance matrix;
- **published initial** means release documentation and packages expose the
  capability with an explicit initial compatibility boundary; and
- **stable** requires a separate compatibility and release decision.

An implemented capability is not automatically portable, published or stable.

The project-wide ordering authority is `docs/ROADMAP.md`. Files in this
directory are detailed status, decision and evidence ledgers, not a competing
area roadmap.

## Document destinations

| Audience | Enduring document |
| --- | --- |
| Rexx programmer learning the model | `docs/books/crexx_programming_guide/concurrency.md` |
| Language implementer or precise syntax reader | `docs/books/crexx_language_reference/concurrency.md` |
| Level B class-library user | `docs/books/crexx_library_reference/concurrency.md` |
| Concurrent HTTP client/server user | `docs/books/crexx_library_reference/concurrent_http.md` |
| Maintainer or AI agent | `docs/ai-context/CREXX_CONCURRENCY.md` |
| RXAS author | `docs/reference/rxas/instructions/09-io-sockets-processes-and-time.md` |
| Current implementation and publication status | [`WORKLIST.md`](WORKLIST.md) |
| Executable test manifest and entry point | [`TEST-MANIFEST.md`](TEST-MANIFEST.md) |
| Release 1 solution-point and platform QA | [`QA-CLOSEOUT.md`](QA-CLOSEOUT.md) |
| Source-to-test capability truth | [`IMPLEMENTATION-STATUS.md`](IMPLEMENTATION-STATUS.md) |

Release-specific availability belongs in `docs/releases/`. Benchmark results
and regression guards remain under `performance/` and link back here when they
affect concurrency work.

## Working rules

- Preserve structured task lifetime, execution-local mutable VM state and
  receiver-owned transfer as coupled invariants.
- Do not introduce a public RXPA task path, raw thread identity, worker
  affinity, live cross-worker VM objects or provider-specific opcode families.
- Treat public syntax, RXAS/RXBIN, provider ABI and wire-protocol changes as
  separately approved architecture decisions.
- Exercise `rxc`, `rxas`, `rxlink` and both VMs when concurrency libraries or
  compiler lowering change.
- Keep dated timings and raw logs in their evidence bundles. Summarize only the
  resulting status here.
