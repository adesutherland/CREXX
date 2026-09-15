# S4-D01 — Preserve complete RXPA text at the inference boundary

Status: approved and implemented; focused correctness passes. Adrian accepted
the STEP-04 Release verdict and subsequent replay on 2026-09-14. The measured
Metal overhead remains an accepted observation with unresolved cause; functional
STEP-04 closeout is active. Sanitizer/platform qualification remains STEP-06.
Parent scope: [STEP-04](native-inference-step-04.md). The bounded negotiated host-service correction is authorized. It is not a sanitizer finding or llama.cpp defect.

## Vision and intended outcome

Pass the complete cREXX UTF-8 string to the embedding tokenizer, with its actual
byte length. Valid text containing U+0000 must never be silently shortened by the
C adapter. Exact option strings and filesystem paths must reject embedded NUL
where their target contract cannot represent it. Preserve the approved `.string`
public API, plugin isolation and the existing legacy initializer layout.

## Reproduction and cause

The permanent [ordinary cREXX reproducer](../../tests/native-inference/embedding_text_boundary.crexx)
constructs a 21-codepoint string `cpu`, U+0000, `unexpected suffix` using the
existing explicit UTF-8 codec. `rxllama.configtext` incorrectly accepts it as
the exact option `cpu`. The [retained failure](../qa/native-inference-step04/text-boundary-red.log)
reports `FAIL: RXPA silently discarded text after embedded U+0000` with exit 1.
The fixture checks its full length before invoking the plugin.

`rxvm_getstring()` copies all `value.string_length` bytes and adds a terminator,
but the RXPA `GETSTRING()` interface returns only a `char *`. The caller cannot
distinguish an embedded U+0000 from that final terminator. STEP-03 uses C-string
option construction; the initial STEP-04 adapter likewise passes a C string to
request admission, so the same mechanism affects input text. Reading VM internals
from the plugin, scanning past the first NUL, or assuming `strlen()` is the real
length are not valid repairs. `GETNATIVEPAYLOAD()` exposes `.binary`, not the
string byte length. The interpreter itself retains the complete valid string.

## Alternatives and recommendation

1. Leave the C-string boundary in place: rejected, because it silently changes
   valid input and cannot implement the approved no-truncation contract.
2. Change native text parameters to explicitly encoded `.binary`, with a Rexx
   text wrapper: technically viable using existing payload helpers, but changes
   the approved native signatures and adds conversion/copying and two surfaces.
3. **Recommended:** provide a length-aware, call-scoped read-only text view through
   a negotiated RXPA host service. It returns pointer plus byte length; the
   provider copies admitted request input once into its bounded owned storage.
   Preserve the unversioned legacy initializer layout. A size/version-negotiated
   session-factory extension can receive the immutable service table without
   changing existing factory callbacks or global helper state. Old plugins on
   new hosts retain existing behavior; this new provider must fail explicitly on
   an old host lacking the required service instead of reverting to truncation.

The host-service lifetime and negotiation are an RXPA architecture change. This
proposal requests approval for that bounded change, not a general plugin-ABI
redesign. Exact registration/static-catalogue paths must receive the same checks.

## Numbered checkable acceptance criteria

1. **S4-D01-AC-01:** The ordinary cREXX fixture rejects the full invalid option
   and accepts its ordinary `cpu` positive control. Retain the pre-repair failure.
2. **S4-D01-AC-02:** Embedding inputs preserve exact UTF-8 byte length, including
   U+0000. Compare token counts and output with a direct length-aware pinned
   tokenizer control; byte limits count the complete input.
3. **S4-D01-AC-03:** Empty, ASCII, multibyte Unicode and embedded-NUL inputs have
   a valid borrowed view only for the native call. Request ownership copies before
   return; no VM pointer survives in a request or crosses workers.
4. **S4-D01-AC-04:** Dynamic and static session-aware loading receives the service
   safely. Existing plugin initializer offsets and old-plugin behavior remain
   compatible; an old host produces a bounded unsupported-service failure.
5. **S4-D01-AC-05:** Focused normal Debug checks pass through the four tools and
   applicable VMs. Include first-party overhead in STEP-04's bounded first Release
   verdict. Maintained sanitizer/platform proof remains assigned to STEP-06.

## Numbered implementation steps

1. **S4-D01-01:** Preserve the reproducer and source trace, then obtain Adrian's
   selection of the bounded interface change before editing host/RXPA contracts.
2. **S4-D01-02:** Add ordinary service-negotiation, old-host/old-plugin and text-view
   regressions; implement the selected call-scoped service and guarded session
   factory negotiation for both dynamic and static provider paths.
3. **S4-D01-03:** Consume explicit lengths in rxllama, preserve input bytes and
   reject NUL for C-string-only selectors/paths. Run matching input/ownership
   controls; do not silently weaken the public `.string` contract.
4. **S4-D01-04:** Rejoin STEP-04 at focused correctness and its first Release glue
   verdict. Preserve all original request, GPU, batching and qualification scope.

## Approval boundary

[AGENTS.md](../../AGENTS.md) requires: "Pause for user approval before making
language-design decisions, syntax changes, or architectural shifts."
Treating the new negotiated RXPA service as an architectural shift is the
implementation agent's interpretation. The provider implementation approval
does not itself select an expansion of the host/plugin contract.

Adrian approved the recommendation after reviewing the reproducer and compatibility
boundary. S4-D01-01 is complete; proceed with S4-D01-02–04 within this scope.

## Implementation and acceptance handoff

S4-D01-01–03 are implemented. AC-01–04 have ordinary local evidence: complete
text/owned requests, strict selectors, direct tokenizer/vector controls, sized
dynamic/static manifest negotiation and actual rxllama old-host rejection.
The unversioned initializer declaration remains byte-identical. AC-05 focused
Debug and ordinary Release correctness pass; S4-D01-04 rejoins STEP-04 at its
[first Release decision](../../performance/evidence/2026-09-14-ni-s4-first-release/README.md).
CPU figures are within variation; Adrian accepted NI-S4-P01's indicative Metal
overhead and variation after the later replay. Its cause remains unresolved. The
[evidence bundle](../qa/native-inference-s4d01/README.md) and STEP-04 handoff retain
all remaining work. Normal functional closeout resumed after that acceptance;
maintained sanitizer/platform proof remains STEP-06.
