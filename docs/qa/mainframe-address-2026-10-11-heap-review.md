# CMS heap candidate amendment

11 October 2026. This amends the initial candidate's CMS runtime profile after
the real CMS failure, without changing ADDRESS semantics or the frozen scripts.
The original [review](mainframe-address-2026-10-11-review.md) and failed candidate
remain evidence. This amendment requires affected CMS replay; it is not success
or release qualification.

On native CMS with 256M virtual storage and a fresh scratch disk, RXC
`-n -E UTF8 -l A1 -i A1 -o MFADDB MFADDB` failed after about 44 seconds while
expanding the current G library's RXBIN section 3. The following symbol
allocation requested 136 bytes and reported errno 12. The initial runtime had
a 64 MiB application heap. RXC -v subsequently returned RC0 and QUERY DISK
succeeded. The failed assembly remained its one-record placeholder.
Post-return CP DISPLAY did not establish accessibility of the observation
array; those zeros are not measured zero heap use.

A separate B-only import-collection diagnostic compiled successfully. It
confirms the cost of scanning unrelated G for Level B, but does not qualify
the full Classic path. The campaign restores unified A1 imports and retains
the complete current G and exits. Import filtering and collection policy are
separate performance work; PDOS currently admits CMS A1 only.

The explicit local CMS candidate selects 268,435,456 heap bytes. The SDK
consumer replaces the known `cms-c2.o` syscall archive member, compiled from
maintained `newlib_syscalls.c` with that flag. It does not append a competing
allocator or change the allocation/return ABI. The producer and candidate
result record the flag and source/library hashes. The maintained CMS adapter
builder also accepts a bounded, doubleword-aligned CMS31 override. The existing
default and 24-bit profiles remain separate. Invalid alignment is rejected
before the consumer creates output.

Fresh CMS RXC/RXAS/RXLINK/RXVM packages pass source/tool before-and-after checks,
encoded instruction checks and MODULE write/verification. Their image sizes
are unchanged: 7,752,776, 4,165,456, 3,560,752 and 4,262,952 bytes respectively.
The heap is obtained dynamically; increasing it does not consume the 8 MiB
MODULE image budget. TSO binaries and 128 MiB profile are unchanged and reusable.

The operator tested reversible CP DEFINE STORAGE 512M and fresh CMS IPL. Restore
the original 64M before final handback. The selected PDOS CMS obtain path has
no 64 MiB cap, checks aligned lengths and starts allocation at or above
0x08000000; TSMALLOC excludes existing mappings. A CMS caller's 256 MiB heap
plus the retained TSO child's 128 MiB is 384 MiB before core, images, file
buffers and tables. Keep serial compiler/tool stages, the 512 MiB machine,
actual high-water and resource-return checks. This is not arbitrary-depth
capacity qualification. A larger physical profile requires its own selection
if the measured campaign cannot fit.

The replacement pack uses `mainframe-qa-pack.crexx`'s explicit fifth `heap256`
argument. It keeps the exact scripts, KEEPIN and current libraries, selects
the four new CMS packages, and reuses the unchanged TSO packages. Strict source,
package and supplemental hashes precede staging. Native CMS sources are staged
as MFADDB/MFADDC CREXX and invoked by bare stems because the selected ordinary
CMS PLIST carries eight-byte tokens; source bytes and language OPTIONS are
unchanged. The same delivery alias applies on PDOS CMS. Replay both CMS native
and PDOS CMS cells after replacing tools, preserving the original failure and
the final caller/file/capture/condition/recovery evidence.
