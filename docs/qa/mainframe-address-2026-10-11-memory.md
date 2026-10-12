# Mainframe ADDRESS memory assessment

12 October 2026. The repaired four-cell campaign passes. Native application
heap high-water remains unmeasured and the earlier host s3270 growth remains
unexplained. This distinguishes faults from capacity and qualification limits.

The current assessment is that cREXX had real pointer/allocation defects and
the SDK application profile was undersized. We have no evidence of a CMS
allocator defect. PDOS has separate file-capacity, delivery and native PDS
member-identity defects. Host s3270 memory growth is a separate unresolved
symptom. Both real OS profiles now pass two B and two Classic executions;
the corrected PDOS cells also pass twice in each language mode. Final normal host QA
passes 3,338/3,338, Apple ASan passes 3,140/3,140 and full Linux ASan/LSan passes
3,134/3,134. Native heap high-water usage remains unmeasured.

| Finding | Owner / classification | Current evidence |
| --- | --- | --- |
| Short command read past allocation | cREXX's new first-party native-boundary test mock | SAN-013: seven-byte comparison against four-byte TIME, fixed by checking length first. Focused Debug, Apple ASan and Linux ASan/LSan pass on recorded inputs. No production native adapter is implicated by that diagnostic. |
| Pointer stored in a 64-bit Rexx integer on a 31-bit target | cREXX portability | Explicit uintptr_t conversions preserve pointer bits and the 64-bit language integer. Both CMS/TSO native RXVM packages compile and pass encoded/export checks. Level B now preserves the open caller file and its 64 KiB tail, and a Rexx value above 32-bit range, in two real executions on each OS. |
| Current G and Classic output exceed ordinary-file caps | PDOS file/STORE capacity | 3 MiB CMS input and 2 MiB output/journal objects cannot deliver the matching current libraries. Fresh optional profile admits 16 MiB objects with checked layout/geometry. This was not exhaustion of 512 MiB physical RAM. |
| Native RXC nearly fills executable image cap | CMS/TSO packaging and PDOS loader boundary | Final CMS 7,753,032 of 8,388,608 bytes; TSO 4,583,376 of 5,242,880. Both verified. Image capacity is separate from dynamic heap and CMS virtual storage. |
| Full compiler import exhausts the CMS31 64 MiB heap | SDK application-profile capacity exposed by cREXX | Native CMS RXC fails expanding current G's RXBIN section 3 at 256M virtual storage, then PANIC symbol allocation, 136-byte request, errno 12. A subsequent RXC -v returns RC0, QUERY DISK succeeds, and failed assembly remains the placeholder. A B-only import collection also succeeds; it is not the selected full Classic qualification. |
| Revised heap peak and child-return recovery | All four named workflows pass; peaks remain distinct | Failed CMS256 and preliminary128 candidates are retained. The selected CMS/TSO candidates use 192 MiB; both real OS profiles and their PDOS emulations complete the compiler chain and two B/two Classic executions. Native application heap high-water remains unmeasured. PDOS real memory is 512 MiB, with observed full CMS/TSO peaks 340,123,648/328,568,832 bytes. Default/released SDK heaps stay separate. |
| Native contiguous capacity, without cREXX | CMS USER-subpool capacity in the recorded guest | At 512M, the native oracle obtains, checks both endpoints and releases 64/128/192 MiB serially. 256 MiB is unavailable, final RC 4, followed by successful QUERY DISK. This is not a largest-block measurement or proof of 192 MiB availability beside RXC. |
| Revised CMS heap rejected during RXC -v | CMS capacity followed by a cREXX failure-path defect | Native CMS512M could not satisfy the 256 MiB obtain, followed by a protection exception before ADDRESS/import execution. The actual CMS20 macro contract matches the SDK, and the independent native capacity oracle also rejects 256 MiB. The selected 192 MiB candidate now starts and completes Level B compilation and two real-script executions with the full import collection. |
| Compiler RXPA metadata dereferences failed allocation | cREXX production failure-path defect | Source confirms unchecked metadata and static function allocations. The DMSFRO caller and map imply live MODULE base 0x1F50F000 and protection offset 0x0016320E inside rxpa_addclass. That mapping is derived, pending direct trace; source repair uses the existing allocation-free exhaustion diagnostic instead of dereferencing NULL. Dynamic callback registration also fails explicitly rather than silently dropping a function. |
| Native work disks cannot hold the qualification inputs | Guest disk capacity, separate from application memory | CMS account disk had only 13 MiB free; the operator used a fresh leased scratch disk. TSO RXAS allocation reports IKJ56245I and RXC transfer B37 on OS39M1 before any candidate executes; isolated scratch DASD staging now restores the corrected RXC member and executes -v. Existing datasets are preserved. |
| Host terminal controller stalls above 500 MiB RSS | s3270 host-controller symptom; cause pending | PID71478, s3270 4.5ga6, had one RSS sample of 535,808 KiB during console setup/CP storage definition, before compilation. It exited after the operator stopped it; CMS remained running. There is no same-PID baseline or leak rate. Replacement PID73607 was independently observed at 8,016 KiB RSS. An unsupported connectWait setting was also reported; it is not established as the cause. |
| Fresh PDOS stops before PCOMM, kernel RC 12 | Campaign bootstrap delivery defect | The inherited U.COMMAND input is the 133,302-byte raw PCOMM native record stream, whereas the production loader requires a PDTSO31 fixed-block envelope. The stopped loader receipt is zero, before publication of the application. The failed candidate, core and STORE receipt are preserved. Corrected envelopes and fresh media pass independent readback; boot/runtime qualification remains open. No cREXX program ran in the failed attempt. |
| PDOS TSO cannot open its staged source member | Campaign dataset block geometry | The constructor used VB BLKSIZE 27,998, while frozen TSKFIND rejects DSCB block lengths above 18,452. Corrected fresh packing preserves logical payloads and now admits RXC/RXAS at RC 0. The later output-member failure is a separate K defect; neither is a heap diagnosis. |
| Native Classic stops before script execution | cREXX native runtime provider packaging | Initial CMS/TSO RXVM loads rejected required `rxcstream`, three `rxfs` bindings and classlib autoload delivery after native compilation/linking succeeded. The separately reviewed 32-record RXVM/library amendment fixes that bounded closure. Both real OS runtime replays now pass twice in both language modes. |
| PDOS PDS output written under the wrong member | PDOS K native-file ABI defect | Actual MVSSUPA symbols give ZMEM-OPENCLOS=148, whereas K used 144. The stopped DD-tail/member-prefix keys exactly match that error. The repaired core passes native binary write/reopen/replace and the full B/Classic CMS/TSO replays, with exact stopped output/transcript/disk-guard and resource checks. |

There is currently no evidence of a CMS operating-system allocator defect.
The unchanged native storage fixture was assembled by real CMS20's
ASMAHL/DMSGPI/DMSOM macros. Independent review of its private listing confirms
R0 in bytes, R15=00E00000, identical obtain/release flags, and exactly 32-byte
parameter lists without the later documented fence. The listing's SHA-256 is
`7154eca4946132c26d5574b7bbadf112759cf65d264253759c93f9d308b7df3f`.
This rules out the suspected unit/list-layout mismatch for this guest; it does
not establish that a large contiguous obtain will succeed. No fixture was
executed during this assembly-only comparison. The TSO export
failure is separate: the build recipe passed an absolute output pathname to
classic-ld, which used its first eight characters as the load-member name
`/USERS/A`. No native TSO candidate executed. The repaired recipe supplies a
basename and independently checks member, AMODE and RMODE before freezing.
The final replacement RXC now executes -v on native TSO; the old malformed
package never executed there.
If an allocation fails, record whether the application heap reached its own
cap, the configured guest virtual storage could not satisfy GETMAIN, or PDOS
could not supply owned frames. Those failures require different remedies.
The CMS image cap must not be confused with any of them. Post-return CP DISPLAY
did not establish accessibility of the CMS observed-array storage; real-storage
zeros must not be reported as measured zero heap use.

A second s3270 controller, PID 87364, had one 909,616 KiB RSS sample before
logon, with no cREXX program executing. That invocation omitted connectWait.
The login-readiness gate was missing and is corrected, but no causal link
to either large RSS sample is established. The native capacity-oracle
controller remained responsive at an observed 8,016 KiB. These are host
automation/emulator symptoms, not evidence of a guest or cREXX memory leak.
During final Classic investigation a third controller, PID 12233, stalled
after PA1/DISPLAY PSW/QUERY TIME, with one 595,232 KiB RSS sample. The compiler
was paused at CP READ without a native completion code. Its invalid import
collection and the host-controller stall remain separately recorded; neither
sample establishes the compiler's heap high-water or an emulator leak rate.
Another controller stalled during paste with a single 1,005,680 KiB RSS
sample. The operator terminated that controller, reconnected and gated native
login readiness. This again lacks a same-PID baseline or leak-rate measurement.
During the clean CMS run, an optional CP diagnostic stalled controller PID
42282. Two independent host samples of that same PID showed 1,110,352 then
1,436,192 KiB RSS; the operator subsequently reported about 1.7 GiB before
closing only that owned controller and resuming the guest. These observations
establish growth in this host process during the episode, but not its allocation
cause or a precise leak rate. Further optional debugger sampling is suspended
for this run. The candidate live MODULE base and heap counters were never
confirmed, so no native heap measurement follows from the unsuccessful probe.

A separate host snapshot showed 24 GiB physical storage, 10,552.31 MiB swap
in use and a memory-pressure query reporting 35 percent free. That single
system-wide sample cannot establish current paging cost, historic peak demand
or the cause of native compiler duration. It is separate from CMS virtual
storage, the cREXX application heap and PDOS's 512 MiB guest.

The B-only import diagnostic also identifies a cREXX memory-demand question:
scanning the complete import collection brings unrelated G metadata into this
small Level B compile. Import selection and measured expansion cost merit a
later performance review. The selected qualification keeps full, current
Classic dependencies; reducing the collection would hide its capacity need.

The first final native CMS Classic compile also saw blank RXBIN output
placeholders in its A1 import collection, reporting a truncated header. That
attempt is not valid qualification. It exceeded the 900-second watchdog without
native Ready/RC. Paused native bytes confirm MODULE base `0x1F50F000` and the
first PC in `bits_or`; R4=58 and R12=82 show a normal index/count at that instant.
A second PC maps to newlib `memcmp`, demonstrating instruction progress between
the samples, not a proven fixed-PC hang. The operator preserved the original
attempt and abandoned it through native HX and normal owned-session recovery;
it has no fabricated completion code. A clean Classic replay is running after
native LISTFILE confirmed exactly the four intended RXBIN libraries. We need
that result before drawing a capacity conclusion. Delivery now separates the four matching library RXBIN
inputs from output placeholders and previously linked programs. This avoids
both malformed imports and unnecessary repeated metadata expansion; the
malformed-import diagnostic remains intact. PDOS's current CMS emulation
admits only A1, so its stopped candidate uses a different physical filetype
for the four blank output placeholders. Actual exact-name outputs arise in
STORE and are checked separately.

The PDOS prerequisite retains whole-file buffering. Several open 16 MiB files,
caller/child heaps and loader workspace can consume substantial storage even
when one file fits. The later bounded-streaming/extent-allocation work remains
necessary. No unlimited-depth/all-files-open capacity claim follows from the
disposable profile.

PDOS's CMS heap starts at virtual address `0x08000000`; that does not reserve
the first 128 MiB of physical memory. `TSMALLOC` selects physical frames
separately, after the 16 MiB core reservation. The selected 192 MiB CMS caller
plus the retained 128 MiB TSO child leaves 176 MiB before other images,
stacks, tables and whole-file buffers. This source-level budget supports
testing the 512 MiB machine, but does not substitute for measured peak and
resource-return evidence.

The caller fixture holds a file open across ADDRESS, keeps a value above 32-bit
range intact, then reads a 64 KiB tail and closes successfully. This prevents
stdio prefetch from disguising a broken native file cursor. Real positive RC
and contained-fault checks must then be followed by successful commands and
owned-resource release. Native CMS arbitrary child-ABEND containment is not
inferred from PDOS's protection; its safe command controls stay separate.

SAN-013 remains open until full supported-platform closure. A stale generated
Linux fixture was separately diagnosed after preserved source timestamps
prevented bytecode regeneration; it returned the fixture's file-data mismatch
code without a sanitizer memory diagnostic. After explicit regeneration, all
eight final Linux ASan/LSan checks passed in `20261011-205056-ctest`.
The final focused macOS Debug and Linux Debug checks also pass. The full normal
macOS gate passes all 3,151 tests. Full macOS ASan completed 3,137/3,139: two
Level B fixtures returned 19 without sanitizer diagnostics. Their generated
bytecode at 21:07 predated source at 21:21. The new directory had omitted shared
QA tier finalization, so qa-prep did not rebuild its declared prerequisites.
That preparation wiring is repaired. Fresh macOS ASan run
`20261011-234830-ctest` passes all 11 allocation/ADDRESS checks after completed
preparation. Its import collection matches the four libraries selected for
the guest campaign, and its -n compilation matches the reviewed native recipe.
The earlier full Linux ASan/LSan run `20261011-205105-full` passes 3,133/3,133.
The allocation/preparation delta passes all 11 checks with leak detection in
`20261011-232037-ctest`; its 10,484-file merged source manifest has no remote
mismatches and SHA-256
`4dd9c46480685dbbedc6ae88e91c248db9a67a81de2a94276e9c42ed0872de3f`.
Final full normal macOS passes all 3,338 tests on the corrected inputs
(`/tmp/crexx-mainframe-normal-final-full-tests.log`, 3,786.04 seconds), including
all eight ADDRESS and the compiler allocation-failure check. macOS ASan is
still running. Linux ASan/LSan `20261011-232133-full` stopped after 51 executed tests
(50 pass, one diagnostic-catalogue failure), not after all 3,134 selected tests.
The source glob had admitted macOS AppleDouble metadata sidecars named
`._rxcpfunc.c` and `._test_rxpa_allocation.c`; reading those binary metadata
files correctly raised UNICODE_ERROR without a sanitizer memory diagnostic.
Eight transferred sidecars were individually magic/hash-verified and removed
from the owned VM. Intended source is unchanged: all 10,484 manifest hashes
still match. Reconfiguration removes the metadata from the source list, and
the targeted catalogue control passes with leak checks in
`20261012-001728-ctest`. Full replay `20261012-002225-full` is running with
failure reporting retained for the entire matrix. Future macOS tar transfers
must disable AppleDouble emission. These fixture failures are not evidence of a
CMS or PDOS memory defect. The compiler RXPA exhaustion repair independently
passes deterministic failure injection and all eight Debug ADDRESS checks.

The host matching-import Level B compile used 233,570,304 bytes maximum RSS;
Classic used 316,063,744, with 309,691,040 peak memory footprint. These macOS
64-bit measurements show different demand but do not establish a 31-bit heap
size. The 128 MiB tool builds remain preliminary/superseded. The native serial
64/128/192/256 MiB oracle establishes three success/one-unavailable outcomes;
the 192 MiB CMS candidate has now compiled Level B with the full libraries,
assembled and linked fresh outputs, and run the real script twice at RC 0.
Both runs contain captured QUERY DISK records, nonzero/recovery checks and
caller-file/storage pass markers, followed by native Ready. Native TSO Level B
subsequently also passes twice, including selected IKJEFTSR fault containment,
successful TIME recovery, caller-file/storage checks and explicit native RC 0
and caller continuation. The earlier TSO CLIST status 868 was a pre-CALL
harness failure; its command-string assignment is repaired with &STR and a
small -v control precedes the successful real-script stages. The clean native
Classic compiler now also returns RC 0 on both CMS and TSO; native assembly,
linking and runtime qualification continue. This establishes completion with
the selected 192 MiB profile for those compiler runs, not a measured heap peak.
The corrected PDOS CMS image also completes the full-library B compiler,
assembler and linker chain at RC 0, then rejects the fixture's unsupported
QUERY DISK command and exits normally. PDOS TSO starts RXC successfully but
fails to open its source member. Neither emulated result is a memory-failure
diagnostic; affected delivery/fixture replay remains open.

The stopped corrected PDOS receipts now provide real kernel allocator
observations: CMS reached 283,168,768 bytes (270.05 MiB) and TSO reached
256,565,248 bytes (244.68 MiB) against the 512 MiB machine. Both record zero
final invocation depth and normal owned-resource shutdown. These are the
kernel's observed real-allocation peaks for the recorded partial workloads,
not host RSS, native CMS heap use, or complete Classic/nested-child capacity
qualification. Actual native application heap high-water remains unmeasured.
The first capacity-oracle assembly failed because our comment reached column
72 and LINEWRT operands were literals. It was not executed; both fixture
construction errors are corrected before replay. Neither is a storage fault.

The first final macOS ASan build (`20261012-002133-full`) aborted because our
launch enabled build-phase leak detection, which Apple ASan does not support.
It is preserved as a QA setup failure. The replay explicitly disables leak
detection for both build and test phases; Linux retains supported leak checks.

Native receipt reliability is also under review. The explanation that
ARRAYDROP removes one element is not established: direct
CALL ARRAYDROP followed by three-record ADDRESS OUTPUT, repeated three times,
passes with both the installed `g36bced241809` tools and current `g54f736804de0`
tools. The proposed typed-array resets introduced shadowing declarations in
Level B and were withdrawn. The retained CMS helper repair accepts completion
only from the current snapshot after the new command's echo, with a bounded
optional timeout. The delayed CP SLEEP 5 SEC control records two busy snapshots
then fresh READY; the operator measured 6.356 host seconds. Source, compiled
helper and runner identities must be frozen before counting long stages.
These observations do not prove a production ADDRESS driver defect.

Both real CMS and TSO now compile, assemble and link the clean Classic fixture
at RC 0 with the selected 192 MiB profile. Their first RXVM runs then fail at
the same required-rxcstream check, before script execution. The owning rxlink
projection and static canonical-library smoke also identify missing rxfs
bindings and classlib autoload delivery. This is native cREXX packaging, not
heap exhaustion. The separately reviewed RXVM/library amendment passes native
cross checks and focused normal/Apple-ASan static-port checks; actual guest
replay remains required. See the
[runtime checkpoint](mainframe-address-2026-10-12-runtime-qualification.md).

That native replay is now complete: real CMS31 and real TSO31 each pass two
Level B and two Classic executions with the amended RXVM/library closure.
Native Classic compiler/assembler objects were preserved and reused for
relinking. This confirms the named capacity and packaging repairs; it does
not measure native heap high-water usage or qualify arbitrary child ABENDs.

The latest stopped PDOS CMS run reaches the child and contained fault before
the fixture's unsupported VER spelling returns RC 14; VERSION is the native
router vocabulary. The PDOS TSO run has a separately proven K file bug:
output-member work-area displacement 144 instead of the actual MVSSUPA
symbol-derived 148. It journals DD-name fragments as member names, then
reopens the blank physical placeholder. The assembler can accept that blank
input and linking fails. Neither failure is evidence of heap exhaustion.

The latest partial PDOS allocator observations are 340,078,592 bytes for CMS
and 274,391,040 for TSO. Normal shutdown reports invocation depth zero and
released owners. These are observed real-frame peaks of partial workflows,
not native application heap measurements, complete Classic-chain peaks or
host s3270 RSS. The exact input/fixture/kernel repairs must pass before a
complete-chain storage observation can be claimed.
