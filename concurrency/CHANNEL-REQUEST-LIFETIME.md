# Completed channel request lifetime

Status: local implementation and qualification accepted; later-platform QA
assigned to Hotfix release QA (Adrian/Codex). Adrian approved develop publication
and local installation on 2026-09-09 after accepting the first Release verdict
and memory tradeoff. Worktree: `hotfix`, diagnostic baseline
`d840dc1b0730b793b145b767979b3905e6f4266d`.

## Selected contract

Add `chanrelease status,channel,ticket` and `.channelrequest.release()`.
Release requires successful terminal observation; otherwise it returns
`WOULD_BLOCK` without cancellation or consumption. Successful release ends
request authority, including copied capabilities/wrappers, while saved immutable
completions and the channel remain valid. Repeated release is stale. The typed
method raises `CHANNEL_ERROR` for unsuccessful release. Generation wrap retires
slots; authority never aliases an old request.

Preserve consuming channel waits, repeated request-specific observation before
release, terminal cancellation and structured task scope accounting. Release
must not require a new ticket or falsely succeed after provider cleanup fails.
Join private request threads or establish worker detachment before destruction.

## Design selection

The affected installed commit is
`7de12145a0695a81b345eeff8405203c23586e8c`; its relevant channel sources match
the unmodified hotfix baseline above and current clean `develop`
`0ffc500e67daf24e9a2ffc5343aebd8822382321`. Fresh baseline compilation and a
replay against the installed archives both reproduce the retention mechanism.
The historical contract made close the owner-reclamation boundary. Callers had
not omitted an existing per-request release operation; the repair adds that
missing lifecycle capability and corrects cleanup/bookkeeping scaling.

The status quo retains all requests until close. The observed 65,535-ticket
capacity and newest-first provider lists make sustained use exhaust tickets and
make oldest-first byte cleanup quadratic. A fresh Release probe took 14.004 CPU
seconds to close 65,535 synchronous, observed requests (1.203 CPU seconds to
submit/observe them). Full diagnosis and raw evidence are retained in
`/tmp/crexx-channel-investigation.NMIYvV/`.

The live worker's exact failed allocation and ticket count were not captured.
The mechanism fits its status-8 failures and cleanup sample, but status 8 can
also mean allocation or thread-creation failure. The small synchronous probe
does not explain the live process's 1.7 GB footprint. The HTTP 429 and lease
release failure have no demonstrated common cause. Read-only RAG inspection
confirms its cleanup starts with an stdin half-close, which needs another
request and can fail before later cleanup calls at capacity; the historical
path was not traced. RAG adoption and cleanup recovery remain separate work.

Alternatives considered before implementation:

- Close-only ownership plus channel rotation preserves the API but cannot
  sustain persistent streams without application lifecycle changes.
- Automatic release at observation changes raw terminal-cancellation semantics
  and needs a different typed receipt/cache ownership model.
- Early heavy-resource reaping plus retained tickets requires additional states
  and still needs an authority-release operation.
- Explicit release is selected: existing semantics remain until callers opt in,
  and ownership has one concrete, testable reclamation boundary.

Tracking alternatives: an eagerly allocated full-capacity index wastes space
for small channels; rescanning lazily retained history repeats the diagnosed
cost. Select narrowly scoped, growing/reusable slot storage: a core free list,
per-channel live and unobserved lists, and a typed full-ticket hash index with
reusable entries. Provider request lists gain direct unlink links. This is
execution-owned state, not a cross-worker cache. Full identity comparison and
generation validation remain mandatory. Allocation failure leaves existing
ownership intact; close clears the complete index and logical value ownership.
Ordinary VM value buffers retain reusable capacity while their containing
wrapper storage remains alive; neither release nor close promises immediate RSS
shrinkage. Provider request payloads and joined native-thread resources are
physically destroyed by successful release. Retained completion copies remain
application-owned values.

## Approved sequence and boundaries

Implement the operation, safe provider destruction, reusable typed tracking and
CREXX-owned helper/consumer adoption. Add focused lifecycle and toolchain
regressions. Freeze after minimum focused correctness, build the ordinary
profiling-off Release product, reuse the retained baseline and report the first
Release result. Stop for Adrian's verdict before broad QA, sanitizer work,
documentation polish, install/package proof or publication. The implementation
remains provisional until that verdict is accepted. RAG and corpus work remain
outside this change.

## First Release gate — accepted

On macOS ARM64, the frozen profiling-off Release candidate passes the focused
lifecycle and toolchain checks. Structured task copies now use their scope's
retained join results after close; clearing channel tracking must not invalidate
DO PARALLEL result copyback. The focused set includes this compatibility case.

Two serial paired diagnostic rounds (AB/BA) confirm 65,535 retained-request
cleanup falling from 13.920–14.311 CPU seconds to 0.001 seconds at the probe's
millisecond reporting resolution. Peak RSS rises from about 18.07 MiB to
20.58 MiB, about 2.51 MiB / 13.9%, crossing the memory guard. Extra ticket/provider
links explain an allocation cost; allocator rounding also affects process RSS.
This is a bounded mechanism diagnostic, not a formal portfolio or release claim.

The explicit-release native probe completes 200,000 operations with zero live
tickets at every checkpoint. The ordinary optimized public-API image completes
200,000 release cycles in 2.93 seconds with 14.00 MiB peak RSS, versus 13.94 MiB
at 1,000 cycles. No source/build/test inputs changed during measurement.

Adrian approved accepting the retained-request memory tradeoff and proceeding
with the remaining agreed QA/documentation. The observations remain bounded
mechanism evidence; this approval does not supply unrun platform qualification.
Raw logs, commands, source freeze, build provenance and interpretation are in
the [retained evidence bundle](../performance/evidence/2026-09-09-channel-request-lifetime/README.md).
Scratch builds and the full source freeze remain in
`/tmp/crexx-channel-release.1v4pFt/`.

## Subsequent qualification

The full ordinary Debug correctness suite passes 2,298/2,298 with only the
separately classified performance-measurement lane excluded. The maintained
Apple-ASan focused panel passes 12/12, including byte/child/process providers,
200,000 native release cycles, generation retirement, capacity recovery, public
request copies and structured task results after scope close. The fresh full
Apple-ASan build, preparation and correctness run also pass **2,298/2,298** in
1,293.05 seconds of CTest. No sanitizer diagnostic was reported. A separate
bounded native diagnostic counts 10,000 asynchronous 4 KiB requests: each
successful release joins its one private thread, leaves zero live tickets and
preserves the saved read result.

The installed old reader rejects the release image's new required feature bit
`0x80`. Existing callers must explicitly release observed requests to benefit;
updated library images require the matching runtime. All existing RexxDoc tags
are preserved, and the maintained coverage test passes.

Linux ASan/LSan and Windows thread/process cleanup evidence remain outstanding.
This work does not close existing sanitizer-register entries or replace their
hosted gates. After reviewing the local results, Adrian explicitly approved
publication to `origin/develop`, updating local `develop`, local installation,
and an update to the cREXX-RAG agent. The unrelated UI documentation commit
`0ffc500e67daf24e9a2ffc5343aebd8822382321` is incorporated before publication;
its sole file, `lib/ui/contracts/README.md`, does not change the qualified runtime
or test inputs. RAG source and operational work remain outside this change.

### Accepted later qualification handoff

Owner: **Hotfix release QA (Adrian/Codex)**, accepted by Adrian on 2026-09-09
with the development publication/install approval. Qualify the final
code revision with `Sanitizer QA` (`Linux x64 ASan/LSan` and `macOS arm64 ASan`),
`Build CREXX` and `Deep Build QA`, including `Comprehensive Windows x64`.
The current hosted Windows lane uses MSYS2/MinGW; separate MSVC request-thread
handle, join, cancellation and teardown evidence is still needed for an MSVC
qualification claim.

The sanitizer register still records outstanding closure qualifications for
**SAN-006, SAN-007, SAN-QA-008, SAN-QA-009, SAN-QA-010, SAN-QA-011,
SAN-QA-012, SAN-QA-014 and SAN-QA-015**. Their existing repairs and permanent
regressions are present in the baseline and covered by these full ordinary Debug
and Apple-ASan runs; their individual historical evidence and closure conditions remain in
[`docs/SANITIZER-WORKLIST.md`](../docs/SANITIZER-WORKLIST.md). This handoff keeps
them release-blocking and does not mark them closed or waive any gate. Approved
development-branch publication and local installation are not release/tag or
cross-platform sanitizer-clean claims.
