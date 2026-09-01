# HIGHLIGHT-01 parser-mode projection worklist

Status: complete; first profiling-off Release verdict accepted and
cross-project closeout validated

Last updated: 2026-07-28

## Scope and baseline

This is a bounded correctness and performance defect across cREXX parser mode,
DSLSH source-position conversion and THE's batch syntax-highlighting consumer.
It is not a VM, language, RXAS, RXBIN or public protocol redesign.

Exact source baseline before the first production edit:

- cREXX `develop` at `d5f0827ca2708eae9d9be182c6d0d53bd6229b74`, clean;
- DSLSH `develop` at `a64c99385aa48022087d7749dbd7f15d6548d3e0`, clean;
- THE `main` at `a42e977eb95ba38ef319c345eafcc1d2c224b90e`, with the
  pre-existing stabilization, syntax-highlighting, documentation and test
  work retained as a dirty worktree;
- ordinary profiling-off cREXX Release `rxc` SHA-256
  `32879e5e97a9feb6540ca304abf115d58ee434beb78d68e27129ce4ee4a7f8a8`;
- linked Release DSLSH parser archive SHA-256
  `cf963351eb18ac8e53a5e5ce606cefaba8be1fd73402abd2e3cfdb16c8fd1c0f`;
  and
- 1,939-line, 66,604-byte THE `render-html.crexx` input SHA-256
  `d6f797c5f8e66b51c10e4d72787fd25ed72b499b986e4c8ebe91ca25236a752b`.

The host is Darwin 25.5.0 arm64 with 10 logical CPUs, Apple clang 21.0.0 and
CMake 4.3.2. Builds use the repository-required parallel policy. Benchmark
samples remain serial.

## Falsifiable hypothesis and mechanism evidence

For a small eight-line input, cached THE plus `rxc`/DSLSH parsing takes about
0.17-0.19 seconds, so the multi-process protocol is not intrinsically a
multi-second path. The large 1,939-line input does not complete within THE's
5,000 ms wait and currently yields zero style spans.

A four-second native sample of the ordinary Release `rxc --syntaxhighlight`
child captured 3,429 stacks. `utf8nlen` was the top frame in 3,257 samples
(about 95%). About 78.6% of all stacks were under
`cb_add_missing_tokens -> compiler_get_token_callback`, which restarts at
`context->token_head` and converts token spans from the start of the source for
each missing-token callback. Another approximately 16.6% was under source-tree
projection using the same stateless conversion.

Hypothesis: building source-position facts once and looking up projected tokens
by position will remove the repeated source and token-list scans while
preserving byte-boundary rejection and DSLSH's zero-based Unicode-codepoint
contract. The expected shape is O(source bytes) preparation plus O(1) ASCII or
O(log codepoints) Unicode span lookup, instead of repeated O(source prefix) and
O(token count) scans.

## Candidate and ownership selection

### Status quo

`cb_utf8_byte_span_to_codepoint_span()` calls `utf8nlen(source, byte_pos)` for
every span. CREXX's missing-token callback begins again at the token-list head
for every requested gap. It has no retained allocation but is demonstrably
pathological for a whole source file.

### Candidate A: cursor-only conversion and token traversal

Retain one source pointer/codepoint cursor and one token cursor. This uses O(1)
memory and is the narrowest implementation. It is rejected as the production
choice because DSLSH's bottom-up missing-token walk and AST/source-map
projection do not promise monotonic query order. Backward queries would either
be wrong or fall back to the status-quo rescan.

### Candidate B: eager source position index and projected-token lookup

DSLSH owns an additive UTF-8 position-index API. ASCII source is detected once
and uses byte offsets directly without a per-codepoint allocation. Unicode
source owns a boundary-offset vector and uses exact binary-search lookup.
CREXX owns a per-parse projected-token vector, built from the authoritative
token list and searched by codepoint position. Both objects are local to one
parse and freed before the parse context is released.

This is the selected production candidate for the first Release verdict. It is
order-independent, keeps the existing stateless DSLSH API for compatibility,
does not alter the wire protocol, and gives the ASCII workload the direct
machine-level operation that its byte/codepoint identity permits. Unicode
memory is `(codepoint_count + 1) * sizeof(size_t)` plus the projected-token
vector; no state survives a parse request.

### Candidate C: sparse checkpoints

Build a small checkpoint table and rescan at most one fixed byte window per
query. This lowers Unicode memory but retains per-query scanning and adds
boundary/checkpoint complexity. It remains a fallback if Candidate B's measured
memory or lifecycle cost is unacceptable; it is not selected before evidence
shows that tradeoff is needed.

### Prototype ceiling

A disposable `clang -O3` primitive comparison used all 7,828 word-start byte
positions in the exact 66,604-byte THE source. Repeated `utf8nlen` took
0.182551 seconds. One ASCII classification/index pass took 0.000048 seconds and
all direct indexed lookups took 0.000003 seconds, with identical accumulated
positions. This is a mechanism ceiling, not an end-to-end product claim.

## Semantic risks and focused correctness matrix

- ASCII, multibyte UTF-8, zero-length spans, end-of-buffer spans and rejection
  of positions inside a multibyte codepoint must match the existing stateless
  helper.
- Queries must remain correct when requested in non-monotonic order.
- Source-map projection must preserve raw editor positions and suppression of
  generated tokens.
- Missing comments, punctuation and unknown gaps must retain their current
  token types and lengths.
- Level B and Level C parser-mode fixtures, diagnostics overlays and retained
  highlighting cache behavior must remain unchanged.
- Allocation failure must fail safely; it must not publish a partial parse tree
  or leak an index/token vector.

The minimum pre-verdict checks are the DSLSH codebuffer unit test plus focused
cREXX syntax-highlighting, source-semantics/editor-diagnostics and highlighting
cache tests. A large end-to-end THE source result must contain nonzero style
spans before it is considered timing-eligible.

## First profiling-off Release verdict

After the focused correctness checks pass, freeze implementation and build the
ordinary profiling-off cREXX Release product with parallel compilation. Compare
the exact 1,939-line input through the same THE/DSLSH path against the retained
pre-edit observation:

- the 5,000 ms wait failed to produce any spans;
- the outer run nevertheless exited successfully after about 6.57 seconds;
- a 20,000 ms wait also produced zero spans and took 25.00 seconds; and
- native sampling attributed about 95% of `rxc` stacks to the conversion path.

The candidate passes its first verdict only if the exact source produces
nonzero, structurally valid style spans within 5,000 ms using ordinary Release
binaries, small-source output remains equivalent, and no focused correctness
test fails. Report raw wall samples and the before/after boundary. Stop for
Adrian's direction at that point; do not proceed to broad CTest, sanitizer,
install/package, THE renderer caching/precompilation, documentation polish or
commit/publish work before the verdict is accepted.

Adrian accepted the first verdict on 2026-07-28. The formerly timing-ineligible
large input produced 11,030 structurally valid spans. Five ordinary
profiling-off Release parser-path samples ranged from 0.33 to 0.49 seconds,
with a median of about 0.36 seconds, versus failure at both the 5-second and
20-second waits before the candidate. The focused DSLSH unit test and seven
focused cREXX highlighting/cache/diagnostic tests passed before acceptance.

## Accepted-closeout path after direction

If the first verdict is accepted:

1. fix THE's generated profile so a nonzero `SDSLHWAIT` result fails closed;
2. give that profile a stable/content-addressed path so CREXXSAA can reuse its
   path-scoped build cache;
3. precompile or reliably cache the Level B renderer instead of compiling it on
   every request;
4. run the proportional DSLSH, CREXX and THE focused plus broad Release/Debug
   regression checks, using the documented parallel policy;
5. remeasure small, medium and large source cells, separately reporting process
   lifecycle, compilation/cache and parser projection costs;
6. update DSLSH parser integration and THE end-user documentation; and
7. retain the accepted evidence and reconcile the roadmap status.

No commit or push is authorized by this worklist.

## Accepted closeout evidence

The accepted closeout retained the public DSLSH protocol and existing
`STYLESPANS` array contract. THE added a separate `STYLESPANSTEXT` scalar for
batch transfer, a stable hosted-profile path, fail-closed `SDSLHWAIT` handling,
precompiled Release renderer images, and a single advancing render cursor.

Mechanism isolation on the current large THE renderer showed why the remaining
latency was not inherent process overhead:

- Release parser projection alone was about 0.35 seconds;
- exporting 11,000-plus records through the legacy array took about 3.79
  seconds because it performed one hosted variable-pool update per record;
- the bulk scalar export took 0.34, 0.34, 0.36 and 0.36 seconds on warm runs;
  and
- two traced runs of the stable packaged profile were both CREXXSAA cache hits.

After one unrecorded warmup per source, five serial profiling-off Release
end-user wrapper samples were:

| Input | Lines | Bytes | Raw wall seconds | Median |
| --- | ---: | ---: | --- | ---: |
| small `hello.crexx` | 8 | 191 | 0.38, 0.36, 0.36, 0.36, 0.35 | 0.36 |
| medium `batch-md-rexx.crexx` | 437 | 15,322 | 0.44, 0.41, 0.40, 0.43, 0.43 | 0.43 |
| large `render-html.crexx` | 1,942 | 66,939 | 0.59, 0.61, 0.61, 0.62, 0.59 | 0.61 |

The large HTML result contained 11,079 styled spans. A forced one-millisecond
wait returned infrastructure status 2 with empty standard output, so a parser
timeout can no longer masquerade as successful unhighlighted output.

Final local regression evidence:

- DSLSH Release: 13/13; Debug: 13/13;
- cREXX Release: 1,925/1,925; Debug: 1,925/1,925, including 70 syntax
  highlighting tests in each configuration;
- THE Release: 55/55; Debug: 55/55; and
- final focused THE source highlighter: 1/1, including the timeout and
  precompiled-release paths.

The installed user-prefix cREXX was deliberately not replaced: no install,
commit or push was authorized. End-to-end measurements therefore selected the
validated sibling Release build explicitly.
