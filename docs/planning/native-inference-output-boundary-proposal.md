# S5-D01 — Publish complete generated UTF-8 through RXPA

Status: approved with simplification guidance, 15 September 2026; implemented and focused checks passed; first Release verdict accepted. Parent:
[STEP-05](native-inference-step-05.md), S5-AC-02/05. This is the output counterpart
to the already approved S4-D01 borrowed input view.

## Vision and selected scope

Publish generated text with its exact byte length, including valid U+0000,
without changing the approved `.string` API or adding a Rexx conversion shim.
The current RXPA `SETSTRING` calls `set_null_string`: it has no length parameter
and truncates at the first NUL. `GETNATIVEPAYLOAD` is binary storage, not string
storage. The VM's `set_string` already supports explicit lengths internally.

Approved addition: append an optional `string_set(value, data, byte_length)`
callback to `rxpa_host_services_v1`, guarded by complete struct size, ABI version
and non-NULL function pointer. Add a checked C helper. It copies into VM-owned
string storage, accepts empty text and embedded NUL, and rejects NULL with nonzero length,
oversized lengths and invalid UTF-8 without modifying the destination. Borrowed
input is needed only for the call. Do not change the legacy RXPA initializer.
Generation construction on an older host without this service fails explicitly;
existing embedding behavior remains available when its original services exist.

Keeping the C-string setter would silently truncate valid output. Returning
binary and converting in Rexx would change the approved surface and revive the
wrapper pattern Adrian has just removed. Neither is selected.

## Numbered acceptance criteria

1. [x] **S5-D01-AC-01:** Retain the current setter's ordinary failing NUL-output
   control; new output preserves empty/ASCII/Unicode/NUL bytes exactly and owns
   its copy independently of the source buffer.
2. [x] **S5-D01-AC-02:** Reject null/invalid/truncated/oversized inputs before
   mutating output; retain normal positive controls and destination preservation.
3. [x] **S5-D01-AC-03:** Old-sized/poisoned/absent host tables fail safely;
   legacy initializer layout and existing plugin behavior remain unchanged.
4. [x] **S5-D01-AC-04:** Typed and low-level generation outputs use the checked
   service. Focused normal Debug checks pass; performance is included in the
   first Step 5 Release verdict. Sanitizer/platform checks remain Step 6.

## Numbered implementation steps

1. [x] **S5-D01-01:** Retain the minimal failed output-copy control and obtain
   approval for the concrete optional host-table extension.
2. [x] **S5-D01-02:** Add the bounded callback/helper and permanent negotiation,
   validation and ownership regressions in the existing host-service fixture.
3. [x] **S5-D01-03:** Wire generation chunk publication, run focused controls,
   and rejoin Step 5's first Release verdict.

## Approval boundary

[AGENTS.md](../../AGENTS.md) says: "Pause for user approval before making
language-design decisions, syntax changes, or architectural shifts."
I treat expanding the host/plugin service contract as an architectural decision,
as in S4-D01. This asks only for the described optional output callback;
no language syntax, initializer-layout change or general ABI redesign is proposed.
This was the approval boundary before implementation; the approval and completed
implementation criteria are recorded above.

## Approval clarification and storage contract

Adrian authorized making this complete and as simple as possible, with callers
shielded from string-storage complexity, and asked about five trailing NULs and
byte versus codepoint lengths. The selected contract uses UTF-8 **byte lengths**
at the C boundary; the VM validates and maintains its codepoint count. Ordinary
Rexx positions remain codepoint based. No caller-provided terminator or padding
is required. Empty NULL input is accepted only with length zero. The setter owns
its copy, including when the source is a borrowed view of its own destination.

`prep_string_buffer_metric`/`set_null_string` do not promise terminators.
`null_terminate_string_buffer` and the legacy RXPA `GETSTRING` copy-out provide
one C terminator where needed. Five NUL bytes have no additional UTF-8 meaning
and would not make embedded U+0000 safe for C-string functions. No global padding
change is needed or selected. Consumers using the new counted interface only
handle a pointer and byte length; they do not manage codepoint counting or padding.

Baseline failure: `/tmp/ni-s5/output-boundary-before.log`, ordinary
`rxpa_host_text_services`: `RXPA output truncated embedded U+0000: 1 of 5 bytes`.
The permanent control now uses the counted service and adds ownership, alias,
invalid/truncated input and sized-tail negotiation controls. Sanitizers stay held
until STEP-06 under the existing authorization.

Focused host-service validation passed in `/tmp/ni-s5/host-after.log` (0.81 s);
16 ordinary typed/low-level generation consumers passed in
`/tmp/ni-s5/public-debug-01`. The first Release verdict is accepted; Step 6 proof
remains pending.

The [first Release verdict](../../performance/evidence/2026-09-15-ni-s5-first-release/README.md)
now includes the output service and both generation surfaces. S5-D01 implementation
criteria are checked; Adrian accepted the Step 5 performance disposition on
15 September. [Ordinary closeout evidence](../qa/native-inference-step05/README.md)
retains the further checks; Step 6 sanitizer/platform proof remains open. No global
padding or legacy initializer change was made.
