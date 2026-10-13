# Native ADDRESS candidate review and campaign

11 October 2026. Local development candidate, based on `develop`
`54f736804de08684852c63eb7673435157ef1e30`; no commit or release selection.
This review precedes the combined guest campaign. Results belong in a later
qualification receipt; this document does not claim those runs passed.

## Reviewed implementation

The existing ADDRESS request/response and Classic condition machinery remain
authoritative. One shared physical-layout header replaces duplicate indices.
The static provider owns per-VM sessions and reference-counted driver payloads,
checks session identity, copies command text before receiver callbacks, restores
the active VM, and lets the common controller finish redirects. Both standalone
and embedded entry paths register the same catalogue. Host callbacks keep their
higher precedence; the CMS demo is not the native provider.

CMS and TSO stay separate build personalities. Native TSO execution uses the
SDK's processor-independent IKJEFTSR entry. Other TSO environment discovery
borrows/query/releases the current ENVBLOCK for each operation. No borrowed OS
pointer or negative presence assertion survives in a process-global cache.
Synchronous native returns determine completion; no timer establishes readiness.

Driver validation preserves command text while checking UTF-8/IBM1047, native
name bounds, NUL and length. Valid signed command RC is separate from facility,
module and capture errors. Diagnostics retain actual reason/ABEND outputs and
label unavailable sentinels as not reported. Pointer round trips use uintptr_t
without reducing Rexx's 64-bit integer width.

CMS capture is limited to native QUERY DISK, optionally one mode letter.
The bounded grammar prevents pipeline injection; unsupported capture/input is
rejected before effects. A private file is reserved, read as physical records,
decoded and cleaned up through every exit. Collision files are preserved.
Cleanup failure is reported, rather than asserted impossible. Native blank
records become one space in the qualified sink; QUERY TIME, separate ERROR,
TSO capture and PDOS capture remain unsupported. This is no shared-disk atomic
creation or arbitrary CMS child-ABEND containment claim.

## Build and resource boundaries

All eight packages passed encoded instruction/export checks and fresh
verification against their producer's before/after source, include and tool
hashes. RXC/RXAS/RXLINK reuse unchanged final packages. RXVM uses the reviewed
packages that include the diagnostic correction. Canonical newlib overlays
contain the current parser/assembler dependencies; current G and exits are
required. No old beta libraries or abbreviated Classic library is substituted.

CMS RXC occupies 7,752,776 image bytes under its 8 MiB cap, at origin
0x02200000. TSO RXC occupies 4,583,120 under its 5 MiB cap; its entry reserves
stack separately. These are loader bounds, not heap measurements. Selected
heaps are the SDK's 64 MiB CMS31 and explicit 128 MiB TSO31. The campaign must
measure/refuse actual exhaustion and demonstrate caller recovery. Desktop RSS
and the 512 MiB PDOS profile do not prove every nested workload fits.

PDOS's optional fresh-media large-files profile coordinates 16 MiB object/input
caps, checked PDSTLAY2 banks, a 325-cylinder STORE extent, 512-cylinder admission,
installer and independent readback. It rejects incompatible media before
writes. CMS's padded framed-output boundary is 16,772,868 bytes. Whole-file
backing remains; extent allocation and streaming are still PD-030 work.
Each personality gets fresh STORE state because tombstones retain key slots.
Physical placeholders have no previous valid result. Managed images are excluded.

## Frozen campaign inputs and matrix

`ports/single-threaded/mainframe-qa-pack.crexx` copies the exact native packages,
their input receipts, original MFADDB/MFADDC UTF-8 sources, KEEPIN DATA and
matching library/rxfnsc/rxfnsg/rxcexits into the private campaign pack. Its
`frozen-inputs.sha256` is verified before staging. SDK command-adapter overlays,
the PDOS core/image and fault targets are independently pinned by the operator.
Changes require review/refreeze and affected replay.

Run real RXC/RXAS/RXLINK/RXVM for Level B and Classic on CMS31, TSO31,
PDOS CMS31 and PDOS TSO31. Use `-n -E UTF8` and explicit native A1/PDS roots.
These fixtures qualify ADDRESS and native tool/file paths without requiring
compiler optimization; the normal host correctness gate retains optimizer checks.
The B arguments are TARGET CAPTURE-MODE PDOS-OR-DASH KEEPIN-PATH QUALIFY
and, for native TSO, its staged fault LOAD dataset. Classic uses TARGET
CAPTURE-MODE PDOS-OR-DASH QUALIFY and optional native TSO fault LOAD dataset,
as separate native arguments or one Classic argument string.

Native CMS uses verified-absent STATE MFN404 DATA A for a valid positive RC;
native TSO uses verified-absent LISTDS MFADDR.NOEXIST.DATA. PDOS uses the exact
previously qualified ENVCHILD HELLO RC7 and ENVFAULT targets through ADDRESS
PDOS, followed by VER and the selected native command. The fresh TSO MFFLT31
leaf deliberately executes invalid opcode 0000 without child GETMAIN, C stack
or heap. Run it only through the reviewed IKJEFTSR CALL path. Native CMS has
safe nonzero/missing-command controls; arbitrary ABEND recovery remains outside
its qualified facility boundary.

KEEPIN contains ALPHA/LF, 65,536 X bytes/LF and OMEGA/LF in opaque UTF-8 bytes.
It stays open across commands and faults. Post-command reads exceed the selected
SDK newlib 1024-byte stdio buffer, so the check requires fresh native reads.
The 64-bit guard, exact data, close result, RC/.RC/.RS, conditions, output records,
command recovery, repeated lifetimes and resource high-water/release evidence
are retained. The operator must confirm absent targets, placeholders and fresh
outputs, then normally shut down and release leases. Coordinator validation
independently checks results, hashes, stopped readback and handback state.

## Host QA status and remaining risks

The original mock short-command overread is repaired under SAN-013. Focused
Debug, Apple ASan and Linux ASan/LSan panels have passed on their recorded
inputs. Strengthened final fixtures and full supported macOS/Linux sanitizer
gates are running. SAN-013 stays open until its actual closure conditions pass.
No host mock substitutes for native ABI, capture, resource or script evidence.
Remaining risks are native compiler peak memory, actual path/directory behavior,
CMS capture receiver cleanup, child-return state and large-output persistence.
