# RXFS pathname operations: bounded repair proposal

28 September 2026. **Copy repair approved after Adrian's clarification;
hardlink/move and legacy semantic changes remain proposals.** Adrian asked
whether the change belonged to the `crexx` wrapper; after clarification that
it is the native `rxfs` provider's copy operation, he replied “Approved”.
This authorizes RXFS-STEP-02 and its relevant characterization, focused QA,
independent review and ordinary publication. It does not approve D-01's
hardlink/move contract qualification or changing legacy broken-link behavior.
This is subordinate to the authoritative
[beta 3 core-baseline plan](core-baseline-2026-09-28.md), not a new baseline
scope record. Source reviewed: published `develop`
`9f2f44cfd888d324858769809b0381e524850b4c`, plus local documentation
through `925f6261460102a5581298f64f236123fcda8c6e`. The current local
changes after the published source do not modify `lib/plugins/fs/rxfs.c` or
`rxfs_ops.h`. No code, test, workflow, alert, or lab state was changed for
this proposal.

## Vision and intended outcome

Keep useful desktop `rxfs` file operations with their published status and
destination behavior while closing avoidable check/use gaps. The newer
`copy`, `hardlink`, and `move` operations promise that they reject a final
symlink/reparse point and do not overwrite an existing destination. The
[reference](../../books/crexx_library_reference/rxfs.md) assigns parent-path
traversal, ownership, permissions and transactional publication to callers;
it does **not** promise a general immutable source pathname or an atomic
source-identity check. The [controlled macOS #226 probe](../../qa/beta3-core-baseline/codeql-226-path-swap-probe-2026-09-28.md)
nevertheless shows that a substituted final symlink is followed into a victim
by `hardlink` and transferred by `move`. Preserve the explicit final-component
promise; do not declare a second `lstat`, a cooperative `fileguard`, or a
post-success rollback an atomic fix. Keep #222–224 and #226 open until
implemented behavior, status compatibility and supported-platform evidence
support a disposition.

This proposal covers [#222](https://github.com/adesutherland/CREXX/security/code-scanning/222),
[#223](https://github.com/adesutherland/CREXX/security/code-scanning/223),
[#224](https://github.com/adesutherland/CREXX/security/code-scanning/224),
and [#226](https://github.com/adesutherland/CREXX/security/code-scanning/226)
in the desktop/default `rxfs` provider. It does not qualify mainframe native
filesystem services or change the open AC-04/11 raw-backend work.

## Current calls and compatibility that a repair must retain

| Surface | Current path and stable-path result | Repair constraint |
| --- | --- | --- |
| `copy` | Unix `lstat` then `open(O_NOFOLLOW)`, `open` target with `O_EXCL`; Windows `GetFileAttributesW` then `CopyFileW(..., TRUE)`. New operation returns `0/-8`. | Read and classify the *opened* source object, keep byte-copy and destination exclusion, reject final symlink/reparse point. A substituted FIFO must not make an unchecked open block. Windows `CopyFileW` follows a symlink supplied as source, per Microsoft. |
| `hardlink` | Unix `lstat` then `link`; Windows attributes then `CreateHardLinkW`; returns `0/-8`. | Keep regular-file and same-filesystem behavior, no destination replacement. On macOS the probe shows `link` followed the substituted symlink into the victim. Microsoft says `CreateHardLinkW` follows a source symlink too. |
| `move` | Unix `lstat` then `renamex_np(RENAME_EXCL)` on macOS or `renameat2(RENAME_NOREPLACE)` on Linux; Windows attributes then `MoveFileExW` without replace. Returns `0/-8`; accepts ordinary files and directories. | Preserve no-overwrite and supported directory moves. The Unix rename primitives operate on the current source *directory entry*: if it is a symlink, they move the link itself. Their exclusion flags guard the **destination**, not the source type. |
| Legacy `rmdir` (#224) | `stat` follows final symlink; missing, regular file, broken symlink and link to a regular file return `-4`; real empty directory succeeds; link to directory reaches `rmdir` and normally returns `-8`. | `rmdir` itself enforces directory type. If eliminating precheck, classify the captured failure without turning stable wrong-type/missing into `-8`; a post-failure, read-only type query may be needed for this historical distinction. |
| Legacy `delete` (#223) | `stat` failure returns `-4`; successful `remove` returns 0; `EACCES` from `remove` gives `-3`, other failure `-8`. On POSIX a broken final symlink is `-4`, whereas a valid symlink is removed as an entry. | Calling `remove` first would delete a broken symlink and return 0: that is a compatibility change. Also preserve the `-3` permission distinction. A successful removal cannot be retroactively classified as a formerly broken link. |
| Legacy `rename` (#222) | `stat(source)` failure returns `-4`; otherwise POSIX `rename` or Windows `MoveFileExA(..., MOVEFILE_REPLACE_EXISTING)` returns `0/-8`. On POSIX, a broken final symlink is `-4` but a valid symlink is renamed as an entry. | Calling rename first would move a broken symlink and return 0. This legacy operation **does replace** an existing target; do not import the newer `move` no-overwrite policy into it. |

These stable-path results follow the current C code and POSIX final-link
behavior; Windows CRT `stat/remove/_rmdir` details and all race/error mappings
still need native Windows characterization. A `stat` after a failed mutation
can help report a stable-path error; it cannot make a preceding pathname
mutation act on the object originally inspected. The original precheck can
also collapse an access-denied `stat` into `-4`; replacing it with an
operation-first implementation may expose `-3`/`-8` instead. Record those
cases before treating a simplified error map as compatible.

## Platform facilities and limits

* **Unix copy: local repair is feasible.** Open the source once with
  `O_RDONLY|O_NOFOLLOW|O_NONBLOCK` (plus applicable close-on-exec), then
  `fstat` that descriptor and require a regular file. Use its mode for the
  exclusive destination open and read from that same descriptor. The
  nonblocking flag prevents a FIFO swapped into the final name from hanging
  before `fstat` rejects it. `O_NOFOLLOW` rejects a final symlink; it says
  nothing about parent components. An ordinary file substituted *before*
  the open becomes the object copied, consistent with a pathname API that
  promises no earlier object identity. Check read/write/fsync/close and
  cleanup as today. [Apple `open(2)`](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/open.2.html)
  and [Linux `open(2)`](https://man7.org/linux/man-pages/man2/open.2.html)
  document the relevant flags and descriptor identity.
* **Windows copy: use an inspected handle, pending compatibility tests.**
  `CreateFileW(..., OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT)` can open the
  final reparse point itself. Inspect that same handle with
  `GetFileInformationByHandleEx(FileAttributeTagInfo)` and require an ordinary
  file. Copy from that handle into a `CREATE_NEW` destination and retain
  error/cleanup behavior. This avoids `GetFileAttributesW` followed by a
  fresh path-based `CopyFileW`; compare the latter's current metadata and
  sharing behavior before implementation. [Microsoft `CreateFileW`](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew),
  [`FILE_ATTRIBUTE_TAG_INFO`](https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-file_attribute_tag_info),
  and [symbolic-link effects](https://learn.microsoft.com/en-us/windows/win32/fileio/symbolic-link-effects-on-file-systems-functions)
  describe these semantics.
* **Linux hardlink: a pinned-FD route exists with qualifications.**
  `linkat(fd, "", ..., AT_EMPTY_PATH)` can link an opened regular file,
  but requires `CAP_DAC_READ_SEARCH`; it is not a general unprivileged path.
  The Linux manual documents an alternative through `/proc/self/fd/<fd>`
  with `AT_SYMLINK_FOLLOW` when procfs is mounted. Both need explicit
  filesystem/permission tests and a no-overwrite target. Plain `linkat`
  without `AT_SYMLINK_FOLLOW` links a substituted symlink *as a symlink*;
  it does not reject it. [Linux `linkat(2)`](https://man7.org/linux/man-pages/man2/link.2.html).
* **macOS hardlink: no equivalent pinned-FD source was established in this
  review.** The documented `link(path, target)` takes a source pathname;
  the actual macOS probe shows following the substituted link. A second
  `lstat`, a pathname reconstructed from an open descriptor, or linking
  then inspecting/removing the new target leaves a race or transient
  forbidden destination. POSIX `linkat` can choose whether to follow a
  symlink, but its normal source is still a pathname; with follow off it
  could create another name for the substituted symlink. [Apple `link(2)`](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/link.2.html)
  and the [POSIX `linkat` semantics](https://man7.org/linux/man-pages/man3/linkat.3p.html)
  define the available pathname behavior.
* **Unix move: no source-type predicate in the reviewed rename call.**
  `RENAME_EXCL` and `RENAME_NOREPLACE` protect the destination, but a
  concurrently substituted final symlink is still a valid source entry
  for rename. This is an inference from the documented primitive semantics,
  confirmed for macOS by the probe; it is not a proof about every filesystem.
  Opened directory handles anchor parent traversal but do not turn the
  source entry into a conditional regular-file/directory rename.
  [Apple `rename(2)`](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/rename.2.html),
  [Apple exclusive-rename capability](https://developer.apple.com/documentation/foundation/urlresourcekey/volumesupportsexclusiverenamingkey),
  and [Linux `renameat2(2)`](https://man7.org/linux/man-pages/man2/rename.2.html)
  support this narrower conclusion.
* **Windows hardlink/move: investigate supported handle/share routes, do not
  assume the Unix limit applies.** `CreateFileW` can inspect the final
  reparse point and exclude `FILE_SHARE_DELETE` while the handle is held;
  Microsoft documents that delete access includes rename. This may make
  the existing pathname `CreateHardLinkW` usable without a final-entry swap
  on filesystems honoring sharing, but it needs adversarial reparse and
  share-mode tests, including ancestor-path behavior. For move,
  `SetFileInformationByHandle(FileRenameInfo)` acts on a validated source
  handle and `ReplaceIfExists=FALSE` rejects an occupied destination; test
  file **and directory** compatibility, sharing, and target volumes before
  selection. The public Win32 `FILE_INFO_BY_HANDLE_CLASS` list includes
  `FileRenameInfo`, but no `FileLinkInfo`; a kernel-driver
  `FILE_LINK_INFORMATION` structure is not an established desktop Win32
  solution. [Microsoft `CreateFileW`](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew),
  [`FILE_RENAME_INFO`](https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-file_rename_info),
  [`SetFileInformationByHandle`](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfileinformationbyhandle),
  and [Win32 information classes](https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ne-minwinbase-file_info_by_handle_class)
  are the candidate API contract.

## Decisions required before hardlink/move or legacy semantic edits

**D-01 — Concurrent untrusted mutation of the final source entry.** The
published words “reject a final symlink/reparse point” can be enforced for
copy at the handle-open point. For Unix pathname hardlink/move, the reviewed
ordinary user-space calls cannot both perform the operation and atomically
condition it on the final source entry's type. Adrian needs to choose whether
these operations are intended only for caller-controlled source parent paths
(with rejection of a stable final link and a documented concurrent-mutation
limit), or whether an adversarial concurrent writer must be excluded. The
latter needs a proven platform-specific primitive or a narrower supported
capability on affected platforms. Until chosen, retain the current published
promise and #226 as open; do not broadly disable working hardlink/move
merely to hide the alert, and do not call a check/recheck sequence a fix.

**D-02 — Broken final symlinks in legacy operations.** Preserve the observed
`-4` for `delete`/`rename` of a broken POSIX symlink, or explicitly approve
entry-oriented behavior (operate on the broken link and return 0). The latter
matches `remove`/`rename` primitives but is a behavior change. Keeping a
precheck retains the check/use window. `rmdir` can likely be repaired
independently with operation-first error classification; its historical
wrong-type and broken-link status still needs tests. Decide Windows CRT
behavior from an actual Windows run before imposing the POSIX result there.

## Acceptance criteria and steps (all verification remains OPEN)

The copy-only slice of RXFS-AC-01, RXFS-AC-02 and RXFS-AC-05, and
RXFS-STEP-02 with its supporting review/QA steps, is now approved.
RXFS-AC-03/04 and their semantic decision gates remain unapproved. The
existing 53-case macOS provider receipt supplies retained characterization;
it does not substitute for new failure regressions or other platform proof.
Copy keeps its ordinary byte-copy and `0/-8` contract, destination exclusion,
regular-source restriction and final-link rejection. It promises no immutable
snapshot of concurrently edited contents. Preserve supported metadata/error
behavior when changing a platform implementation, and report any unavoidable
public behavior change before selecting it.

| ID | Observable pass condition | Evidence |
| --- | --- | --- |
| RXFS-AC-01 | Stable-path status, no-overwrite, accepted file/directory type and broken-link behavior are explicitly characterized on macOS, Linux and Windows; any approved difference is reflected in the reference. | Focused permanent fixtures, including missing, wrong type, valid/broken final symlink, permission, destination occupied and normal success. |
| RXFS-AC-02 | `copy` never follows a final symlink/reparse point and accepts only the opened regular source; a substituted FIFO/special file does not hang or create a target; existing-target exclusion, bytes and cleanup survive errors. | Deterministic substitution at the open/inspection boundary, short/error I/O injection, Debug and matching maintained sanitizer checks plus native Windows tests. |
| RXFS-AC-03 | Hardlink and move obey Adrian's selected D-01 contract without silently narrowing supported use; no occupied destination is replaced and a final link is rejected under the selected threat model. | Platform-specific deterministic final-entry substitution, ordinary file/directory success, same/cross-volume and target-exists cases; evidence for any declared unsupported primitive. |
| RXFS-AC-04 | Legacy #222–224 repairs preserve approved status and overwrite behavior, or document approved semantic changes; no remaining precheck/use assertion is called fixed merely because an extra check was added. | Causal tests for each operation and stable-path return code, with normal Debug/maintained sanitizer and hosted Windows evidence. |
| RXFS-AC-05 | Coordinator reviews exact source, platform results and static-analysis instances before any alert disposition; baseline gates retain their own exact revisions. | Diff, focused logs, automatic Build/CodeQL results on published head, explicit remaining alert/AC status. |

1. **RXFS-STEP-01 (RXFS-AC-01/03/04; decision gate):** freeze current
   published behavior in small platform fixtures, especially broken links,
   permission errors and destination collisions. Obtain Adrian's D-01/D-02
   decisions before altering hardlink/move or legacy status policy.
2. **RXFS-STEP-02 (RXFS-AC-02):** implement the opened-object copy repair
   in the owning `rxfs` provider only, preserving the `0/-8` surface and
   current cleanup. Validate file, link, FIFO/special, short-read/write and
   source-swap cases in focused normal and maintained sanitizer builds.
3. **RXFS-STEP-03 (RXFS-AC-03; depends on D-01):** implement only the
   platform routes actually supported by the selected contract. Prove
   destination non-overwrite and final-link treatment on macOS, Linux,
   MSVC and MinGW; retain useful stable-path operations where supported.
4. **RXFS-STEP-04 (RXFS-AC-04; depends on D-02):** remove avoidable legacy
   prechecks only with exact status/error mapping or an approved semantic
   update. Keep `rename`'s historical overwrite distinct from new `move`.
5. **RXFS-STEP-05 (RXFS-AC-05):** independent coordinator review, focused
   exact-input QA, then ordinary publication and automatic Build/CodeQL.
   Inspect the alert instances; do not infer alert closure from a successful
   workflow. Broad sanitizer assurance remains governed by the authoritative
   plan's AC-08 and source revision, not by this proposal.
