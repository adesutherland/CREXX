# Mainframe ADDRESS qualification checkpoint

12 October 2026. Stage 1 is implemented and locally qualified on `develop`.
The broad host gates and all four selected real/emulated OS cells pass, with
independent output/lifecycle validation and normal fleet handback. This records
observed
results without treating an incomplete compiler invocation as a pass or a
memory failure. Adrian separately approved Stage 1 source publication across
the four owning repositories on 12 October. Managed-image adoption remains
a separate decision.

The development publication includes the implementation, permanent fixtures,
reproducible recipes and these qualification receipts. Its normal automatic
product/smoke, optimizer-parity and CodeQL workflows are expected to succeed;
the actual runs remain GitHub evidence. Publication reuses the completed
qualification under the exact source identities recorded below. The independent
RXPP/RexxScript commits through `18d13e5c8504f9790c8f004dec5c1fc33cc58c81`
were fast-forwarded before publication without changing any Stage 1 file or
unrelated local file. This does not claim a repeated broad suite for those
upstream changes. No release tag, combined PDOS build or application environment
registration is part of this checkpoint.

| Runtime | Level B | Classic | Boundary |
| --- | --- | --- | --- |
| Native CMS31 | Fresh native compile/assemble/link; two amended-RXVM executions at RC 0 | Fresh native compile/assemble; original object relinked with complete libraries; two executions at RC 0 | Capture, nonzero/recovery and caller-file/storage checks pass. Replacement MODULE and classlib have exact native export/readback. |
| Native TSO31 | Fresh native compile/assemble/link; two amended-RXVM executions at RC 0 | Fresh native compile/assemble; original object relinked with complete libraries; two executions at RC 0 | Selected IKJEFTSR fault recovery and caller continuation pass in both language modes. Native member and library delivery are verified. |
| PDOS CMS31 personality | Fresh compile/assemble/link; two runtime executions at RC 0 | Fresh compile/assemble/link with full library closure; two runtime executions at RC 0 | Coordinator confirms all eleven command results, scoped PASS/recovery/caller assertions, six outputs, exact transcript, baseline disk guard and normal release. |
| PDOS TSO31 personality | Fresh compile/assemble/link; two corrected-argument runtime executions at RC 0 | Fresh compile/assemble/link with full library closure; two runtime executions at RC 0 | Coordinator verifies six full PDS/member outputs, byte-identical reference assemblies, scoped PASS/recovery/caller assertions, exact transcript, baseline disk guard and normal release. Initial bad spelling RC16 is retained separately. |

The native CMS Classic attempt with blank RXBIN placeholders in the import
collection was preserved and controlled-abandoned through HX and normal
owned-session recovery. Its two paused PCs show instruction progress, not a
fixed-PC hang; it has no invented completion code. The corresponding native
TSO invalid-import attempt was cancelled through its own native operator
command without FORCE. Both attempts remain distinct from clean replay.

The current campaign's raw guest evidence is private at
`/Users/adrian/MainframeLab/private/mainframe-address-final-20261012`.
Independent review has read native CMS B-RXC/B-RXAS/B-RXLINK and both
B-RXVM-args screens, and native TSO BCOMP/BASS/BLINK/BVM1/BVM2-str screens.
The [memory assessment](mainframe-address-2026-10-11-memory.md) owns the
allocation, capacity and controller findings; full host QA passes and SAN-013
is closed for its host mock finding.

## Corrected disposable PDOS delivery

The failed image supplied the raw 133,302-byte PCOMM record stream in
U.COMMAND. The production loader requires a PDTSO31 fixed-block transport
envelope. Its stopped application-loader receipt was zero and the kernel
reported RC 12 before PCOMM or cREXX execution. The original image, core,
console, STORE and shutdown receipts are retained.

The correction uses the unchanged product `tso_pack` classic31 route. Each
147,616-byte stage contains eight F/18452 blocks and exactly 133,318 raw
bytes: the frozen PCOMM records followed by the documented 16-byte transport
EOF. Independent review recomputes the envelope FNV and compares every raw
byte and zero padding. Stage SHA-256 is
`1681ed4dbecbe75fef6f691c6050110fbf3f82027eb45be10d5b722966d8521a`.

Fresh CKD construction also exposed arbitrary unused bytes after validated
end-of-track sentinels. The recipe normalizes only those tails on both source
and decoded readback, preserving record headers, keys, data and EOT bytes.
Changed spans and before/after hashes are retained. The existing installer
then compares complete normalized tracks and independently reads back the
bootstrap/core source bytes and padding. This is normalized track equality,
not a claim that the original converter preserves unused padding literally.

Corrected CMS/TSO media live in z-pdos's ignored build directories
`address-final-cms-corrected-20261012` and
`address-final-tso-corrected-20261012`. All 20 CMS and 24 TSO physical input
checks pass. Both have an empty layout-2 STORE with 16 MiB object limits,
911 blocks per bank and explicit 512-cylinder geometry. The extra TSO
MFA12LIB.RXBIN PDS contains exactly the four matching import libraries.
The 28-record delivery delta is independently exact, manifest SHA-256
`f95cba388bd0c29f69a6c72cea75aaf37bf6017d70bf70c435333f545847c843`.
No kernel, native cREXX tool or managed image changed for this correction.

## Independent Classic runtime control

The retained
[`mainframe-qa-bytecode.crexx`](../../ports/single-threaded/mainframe-qa-bytecode.crexx)
produces a separate host-compiled control from the frozen MFADDC source and
four matching libraries. Input/tool hashes match before and after compile,
assemble and link. It leaves the original native campaign pack unchanged.

`output/mainframe-address-host-classic-20261012/MFADHC.rxbin` is 8,632,133
bytes, SHA-256
`6867b575052ab0f91278db515d9e6a16612dfa1a243c40a4333877c9e422fe69`.
Its 14-record frozen-bytecode manifest has SHA-256
`3ede27422005a46f7c3342bdfc954201e6438de7db381b3bd903526675096191`.
Both host runtimes over the actual SDK mock boundaries pass the same Classic
script at RC 0. This optional control remains host-only; the completed actual
native compiler chains and real script runs below supersede its proposed
fallback role. It is not counted as native RXC/RXAS/RXLINK or guest capture,
condition or fault-recovery proof.

## Bounded CMS emulation fixture amendment

The production PDOS command router supplies CMS QUERY TIME and explicitly
rejects QUERY DISK with command RC 24. The shared native fixture's expectation
that QUERY DISK succeeds was unsuitable for this bounded personality. Its
failed RXVM invocation returned fixture RC 5 and then shut down normally;
there was no memory exhaustion, owner leak or driver freeze.

Two separate guest fixtures preserve the original 47-record native pack and
25-record supplement. They use QUERY TIME for success, explicitly require
QUERY DISK's RC 24/ERROR result, and check successful recovery. They also
check unavailable capture, PDOS child RC 7, contained fault and subsequent
VER/CMS command execution. The B fixture retains an open caller file across
commands/children, reads the 64 KiB tail and preserves a value above 32 bits;
the Classic fixture checks RC/.RC/.RS and CALL ON ERROR/FAILURE.

Both compile and assemble with the matching host metadata compiler and frozen
four-library inputs. This is syntax/bytecode evidence, not native runtime
qualification. Fresh CMS delivery and native compile/assemble/link/execute
remain required. The 10-record fixture-review manifest has SHA-256
`88addc9708d4b18eb8177335e8b46fe1ac07a633807261970a00c1b73a84b337`.
No production kernel or driver is changed to fabricate a disk response.

## Runtime-only dependency amendment

Both native systems reach the same required-provider loader failure after
successful Classic compilation, assembly and linking. Independent reading of
CMS C3-RXVM-1 and TSO CVM1-native-final confirms failure before script execution.
The [dependency review](mainframe-address-2026-10-12-classic-provider-review.md)
records the missing rxcstream/rxfs bindings and classlib autoload dependency.

The separately frozen amendment is
`output/mainframe-address-classic-runtime-final-20261012`. Its 32-record
classic-runtime-delta.sha256 has SHA-256
`df72ccd337b66ecb9467fd9af641bff6a2331abf3601366e446482847c69f886`.
Both replacement native RXVM builds pass object/image checks with matching
before/after hashes. The four previous libraries are byte-identical; matching
classlib is 1,100,706 bytes, SHA-256
`1c7ff05accbb5502f35f5d03e7b7d40e68d0be7788c17d06de9f41bee05ea5fb`.
The original pack and native C objects are retained. Native replay replaces
only RXVM, stages classlib and relinks the same C object with the complete
library closure. Both real OS cells now pass two B and two Classic executions.
The coordinator independently read CMS D1-LevelB-VM1/2 and D1-Classic-VM1/2,
and TSO CLINKN, BVM1N/2N and CVM1N/2N. CMS capture reports five records;
TSO's selected fault leaf is contained and subsequent commands return normally.
CMS exported MODULE and classlib hashes match the frozen replacement inputs.
This closes the native replay for the named CMS31/TSO31 profiles, with existing
scope limits retained. The separately amended PDOS cells also pass, as recorded
below.

The static canonical-library smoke/state panel passes 2/2 normal and 2/2 Apple
ASan (leak checking off), including direct execution of the frozen SMOKEL
bytecode. The normal desktop ADDRESS/stream-lifetime panel passes 9/9 in
11.05 seconds. These are focused host results, separate from guest proof and
the full gates on the preceding candidate. Final normal macOS passes
3,338/3,338; final Apple ASan `20261012-002213-full` passes 3,140/3,140 in
6,593.15 seconds with build/test leak detection off and driver RC 0. The
corresponding full Linux ASan/LSan `20261012-002225-full` passes 3,134/3,134 in
6,563.10 seconds with driver RC 0. All 10,484 allocation-final source identities
match afterward. The owned Linux VM then shuts down normally; its PID,
port-22023 listener and private disk handles are absent. The native-provider
amendment has separate focused static-port proof and actual native replay.

The broad Linux run uses the retained allocation-final source snapshot,
10,484 files, rather than an assertion that every later working-tree file is
identical. Seven listed host files have since changed: the sanitizer worklist,
plan, rxvmload, rxcstream, and the static-port CMake/README/native-build recipe.
The later runtime code is selected by explicit static/native provider flags;
the native amendment's source and packaging are separately frozen and checked.
Documentation/recipe changes do not replace the broad-run input identity.
No claim of a newly rebuilt full Linux native-provider matrix follows from
that run. The allocator and original short-command trigger inputs are unchanged.

After the broad gates, the latest host provider code is rebuilt under Apple
ASan in `20261012-031335-full`; all eight ADDRESS checks pass in 2.24 seconds.
The stream resource lifecycle check then passes 1/1 in 2.28 seconds under the
same rebuilt artifacts (`20261012-033802-ctest`). Build/test leak checking is
off as required on Apple. Together with the separate static native-port
2/2 normal and 2/2 ASan panel, this is focused evidence for the later guarded
provider amendment, alongside actual native CMS/TSO execution.

## Resumed guest qualification

Both corrected disposable PDOS media siblings are prepared and physically
verified in the Lab's `docs/updates/2026-10-12-address-member-media.md`. The
operator task briefly stopped making operational progress after native input
export. The operator has now resumed and completed normal native handback,
including CMS DEFINE/QUERY STORAGE64M and LOGOFF, CP/z/OS shutdown, actual
emulator/controller exit and exact lease release. PDOS maintenance was then
reacquired for the disposable campaign and released after final handback.

The focused IOFLT31 run passes at actual END1 RC0 on core f71139b0...d0b26.
The coordinator independently parses its stopped CKD: IOFLA001/IOFLA002 have
the full requested keys and exact logical 700-byte patterns, tag177 after
replacement and tag93 respectively. Their logical hashes are
`98c082bfed658d923a1b8df123a4e71029cbf0615a0af6f720ade4f7cd0ce63d`
and `6376c359a76b7983836206110bbfa2e7a7f9d61ceb4d0a6c5e4835854f2fb38e`.
Outside-STORE bytes match the frozen baseline; all named shutdown/depth/lease
checks pass. The third journal object is the expected PDOS.TRANSCRIPT.
Observed real-frame peak is 134,627,328 bytes; it is not a full Classic peak.

The full corrected PDOS B/C replay is admitted on untouched sibling input
clones. Carrying the two focused-test objects into that run would exceed the
eight-slot journal when six script outputs and the transcript are present.
Preserve the focused stopped copy separately. Both language modes recompile
from the 23-record sources and execute twice; full output/resource readback
and final PDOS handback now pass.

The first full corrected TSO attempt produces fresh RXC/RXAS/RXLINK output
at actual END2/3/4 RC0 with correct MFADDB/MFADLB journal keys. Its first
runtime invocation returns valid application RC16 before fixture output.
The B source returns 16 at the initial caller-file fopen. The operator supplied
bare `MFA12.INPUT`, which the maintained cREXX logical NAME.TYPE interface
cannot resolve without roots; explicit native names require a colon,
parenthesis or leading quote. `DSN:MFA12.INPUT` selects the intended dataset
and the SDK strips that prefix. This is an invocation error, not a new
allocator diagnosis. The failed receipt stays retained; runtime replay uses
the unchanged compiled output and corrected argument. TSO B subsequently passes
twice, and the fresh Classic compiler completes at RC0 and advances to assembly.

## PDOS CMS full result

`pdos-member-full/CMS/run/receipt.json` passes with every named actor check
true, raw core f71139b0...d0b26 and all PCOMM END1 through END11 at RC0. The
coordinator independently scopes two B and two Classic PASS/recovery markers;
both B lifetimes preserve the caller file/storage assertions. Stopped CKD
inspection finds exactly the six nonempty A1 file/type keys and the expected
transcript, with no deleted objects and unchanged bytes outside STORE.
Every decoded logical output matches the private export. The transcript's
stored hash matches its independently captured event stream.

Both native assemblies equal the frozen CMS references after only replacing
the compiler's exact source filename MFADDB.crexx/MFADDC.crexx by the host
reference's .REXX spelling (81/3,615 occurrences). The raw artifacts remain
unchanged. Logical linked B is 220,945 bytes; Classic is 6,527,858 bytes. The
host RXBIN reader accepts all four object/linked outputs; linked closures
contain 19 and 163 modules. Observed real-frame peak is 340,123,648 bytes,
with final invocation depth zero and normal kernel/emulator/controller exit.
This is the full named PDOS CMS workflow, not native CMS heap high-water or
general 0.2.2 storage/STREAM qualification.

## PDOS TSO full result and closeout

The first TSO run retains the fresh B compiler/assembler/linker at END2/3/4
RC0 and the invalid bare-dataset invocation at END5 RC16. The corrected
continuation uses `DSN:MFA12.INPUT`, the unchanged B outputs, fresh Classic
compiler/assembler/linker and two lifetimes of each language. Its END1 through
END7 are all RC0. The coordinator independently scopes every PASS/recovery
marker and both B caller-file/storage checks.

Independent stopped CKD inspection verifies exactly six full 52-byte
dataset/member keys and one transcript, no erased objects, all logical outputs
equal to their exports and unchanged bytes outside STORE. Both assemblies
are byte-identical to the frozen TSO references: B 62,934 bytes, SHA-256
`fb8ab7e95407e6857dd21525d991f557a971eb85b7cb6bc8f2e9d0380d872f95`;
Classic 1,253,965 bytes, SHA-256
`212518fd7e251b637f5b6fe39a4e821f0ceaf1a4b96187487e2c768a24bd080b`.
Linked B is 224,154 bytes/20 modules; Classic is 6,574,080 bytes/165 modules.
The RXBIN reader accepts all four objects/linked files. Peak observed real
frames is 328,568,832 bytes, with final depth zero and all cleanup checks true.

The final host-handback receipt has no owned emulators/controllers/actors,
closed ports and no open work-disk handles. Exact maintenance release precedes
the fresh six-guest fleet receipt: all stopped, all lease sets empty. Canonical
compressed stopped media, exact outputs, small receipts and all failed
attempts remain private; accepted reproducible conversions/disassembly dumps
may be retired. The 13/32/23 source/input freezes still strictly verify.

This closes local Stage 1 for CMS31/TSO31 personalities on the named real
systems and their PDOS emulations. It includes native CMS's selected QUERY
DISK capture; other unsupported redirection/capture fails explicitly. Native
heap high-water, arbitrary CMS child ABEND, complete native filesystem/STREAM
semantics and other addressing profiles are not inferred. Application
registration and the combined inbuilt PDOS personality remain Stage 2/later
work. No commit, publication, release or managed-image adoption follows.

## Remaining PDOS replay

`output/mainframe-address-pdos-fixtures-final-20261012` freezes the four
personality-specific source copies and matching bytecode. The 23-record
pdos-fixture-delta.sha256 has SHA-256
`cf0f7f2b3067d48438def18bfa5cec0b0f80fbc07b13e4ce05df32fb0599e016`.
It uses VERSION for the actual native PDOS environment and preserves all
child, fault, capture and caller assertions. Original native fixtures and
the failed disposable media remain retained.

The PDOS native PDS repair is owned by z-pdos PD-032 and its
`pdos/doc/qualification/ADDRESS-PDS-MEMBER-REVIEW-2026-10-12.md`.
The symbol-derived member-from-OPEN displacement is 148 bytes, replacing 144.
The old value exactly explains the stopped wrong journal keys and empty
physical placeholder selected on subsequent input. This is a PDOS file-identity
defect, not a cREXX allocation failure. Fresh reviewed core, focused native
binary write/reopen/replace, stopped member/payload readback and both affected
PDOS script cells have now passed. Failed candidates remain separately retained.
