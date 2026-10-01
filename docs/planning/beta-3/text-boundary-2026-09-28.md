# Beta 3 text encoding boundary

28 September 2026. Contract clarified during the approved
[core-baseline work](core-baseline-2026-09-28.md). Implementation and
cross-project qualification are still in progress.

## Ownership

**cREXX owns conversion between its internal UTF-8 text and external code
pages.** The shared codec implementation belongs in common cREXX code; the
platform layer selects the encoding and applies the codec at each external
text boundary. The parser, resolver, assembler, linker and VM must not acquire
separate EBCDIC or Windows-code-page implementations.

| Layer | Contract |
| --- | --- |
| Shared cREXX codec code/data | Strict, reusable UTF-8 and named single-byte conversions, streaming state, validation and errors. Use the existing authoritative mappings. No OS, filename or namespace policy. |
| cREXX platform layer | Select and apply conversion exactly once for source/imports, textual tool input/output, application text files, console/diagnostics, arguments and native names. Own logical-to-native filenames and logical line adaptation. |
| newlib and native OS adapter | Allocation, C stream machinery, raw byte/record I/O, native services, ABI and low-address buffers, resource lifetime, error and close/flush reporting. The cREXX path must not implicitly transcode its payload. |
| Binary paths | RXBIN, executable/package bytes, `.binary` and explicit binary file I/O stay byte-exact. Physical native record framing is separate from character conversion. |

Input is native/external bytes → cREXX platform decode → internal UTF-8.
Output is internal UTF-8 → cREXX platform encode → native/external bytes.
The raw runtime service between platform code and the OS must not repeat either
conversion. Record APIs preserve payload, length and record boundaries; cREXX
turns logical text-record boundaries into lines, and reverses that adaptation
on output. Binary record handling never invokes a character codec.

## Encoding policy

The baseline set is UTF-8, US-ASCII, ISO-8859-1, Windows-1252, IBM437, IBM850
and IBM1047, using cREXX's existing mapping policy. ASCII is not a name for all
8-bit encodings. UTF-8 is not interchangeable with a Windows ANSI/OEM page.
Other EBCDIC pages and Windows pages are not implied by the family names.

File selection and host-native encoding are separate. Selecting Windows-1252
or UTF-8 for an input file must not change an IBM1047 console or native-name
service. Existing open streams retain the encoding with which they were
opened. Malformed input, incomplete sequences and unrepresentable output
produce errors; no detection guesses or silent replacement. Explicit existing
library replacement APIs retain their documented opt-in semantics.

This changes external text handling, not Classic BYTE/UTF8 language modes,
byte-oriented BIF behavior, Rexx integer widths or RXBIN representation.

## Required mainframe runtime handoff

The existing CMS and TSO adapters already convert some streams. For the cREXX
build they must expose or select a raw route that bypasses those conversions.
Existing ordinary C clients may retain their separately selected conversion
route; it must not be stacked underneath the cREXX codec. A process-wide
double-conversion workaround or passing cREXX codec tables to newlib is not
the selected architecture.

The current version-1 [raw-service handoff](native-raw-services.md) and
[header](../../../platform/native_raw.h) specify the implementation target.
The raw contract covers console reads/writes, file byte and record I/O,
arguments and native-name/member services. Each interface declares whether its
input/output is native encoded bytes, raw binary, or an explicit encoding-neutral
value. An already-decoded compatibility interface must be identified as such
until migrated; cREXX must not decode it again or call that migration complete.
EOF, short I/O, read/write/flush/close failure and ownership are explicit.

Fatal diagnostics need an allocation-free raw output service, including its
first call. cREXX performs bounded formatting and conversion without requiring
a running VM, loaded Rexx libraries or fresh heap allocation. Runtime emergency
messages use their own declared native-service contract.

The generic PDS iterator returns actual members, without cREXX suffixes,
namespace assumptions or root selection. Any native-name decoding used by
cREXX belongs in its platform adapter. Service signatures and package versions
must be agreed in the implementation handoff; this document does not assert
that old runtime packages already supply them.

On 28 September Adrian explicitly accepted minor subsequent changes to
`platform_fopen()` by the mainframe agent as native details are settled. The
current backend signatures and open mechanics may therefore be reconciled in
the platform implementation and its supporting headers. Keep native flags
behind the platform interface, retain cREXX codec ownership, and preserve the
distinction between binary data, text encoding and byte/record representation.
Return such adjustments upstream with focused regression evidence. This
allowance does not require compiler/VM callers to learn native open flags and
does not qualify an untested combined runtime/package; those gates remain open.

## Acceptance and next steps

This is the boundary specification for AC-04/AC-11 of the core-baseline plan,
not a separate release plan.

1. **TEXT-01:** audit every supported component's text and binary routes;
   identify exactly one codec owner at each crossing.
2. **TEXT-02:** use the shared mapping authority and test selected pages,
   streaming splits, malformed/unmappable data and byte-exact binary controls.
3. **TEXT-03:** identify/version the mainframe raw services, remove or bypass
   runtime conversion in the cREXX route, and test error/cleanup behavior.
4. **TEXT-04:** qualify the combined cREXX/runtime inputs; retain native CMS31,
   TSO31 and TSO64 package gates in the lab's beta 3 plan until those exact
   artifacts pass their required native checks.

Tentative local runtime-map-setter work is superseded by this clarification
and is not published or accepted baseline evidence. The unrelated shared
compiler/binutils/allocation repairs continue under the approved plan.

On 29 September the Mainframe Lab integration branch connected the existing
CMS/TSO `mainframe_set_text_conversion(0)` runtime switch to cREXX's shared
codec at `platform_openfile` and the RXC/RXAS `-E` selectors. The code is
guarded for mainframe builds; binary opens remain byte oriented. The local
CMS31 check compiled a source member to readable native RXAS, assembled it,
and ran its RXBIN on z/VM 4.4. A separate macOS rebuild and focused platform
tests passed. These results do not qualify the other guest profiles or the
formal beta 3 revision.
