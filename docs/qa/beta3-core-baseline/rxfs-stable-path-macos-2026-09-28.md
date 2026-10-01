# RXFS macOS stable-path behavior before a filesystem repair

28 September 2026. **Review evidence only.** This characterizes the current
desktop/default `rxfs` provider on Darwin 25.6.0 arm64 for the coordinator's
[bounded RXFS proposal](../../planning/beta-3/rxfs-path-operations-proposal-2026-09-28.md),
especially decision D-02. It does not test a concurrent path swap, a native
Windows/Linux implementation, or mainframe services. The separate
[#226 substitution probe](codeql-226-path-swap-probe-2026-09-28.md) remains
the evidence for that macOS race. No repository production/test source,
workflow, alert or lab state was changed.

## Exact source and execution route

Published filesystem source is `develop`
`9f2f44cfd888d324858769809b0381e524850b4c`. The checkout began this
probe at `df72bd984`; subsequent coordinator commits through `ee8ab6c1b`
changed documentation only. Current `lib/plugins/fs/rxfs.c` SHA-256 is
`ef833dd500a47f1ee810466603a9670989501c384a344c1d0145819367badf61`;
`rxfs_ops.h` SHA-256 is
`3f2f5f25acc4a98c295c0f536f424a2456a7ab8b473f08ada521b7e2ce5b82d7`.
Both hashes exactly match the same paths fetched from published `9f2f44cfd`.

This uses the **actual provider and cREXX toolchain**, not an extracted C
function. A temporary Level B source imports `rxfs`, accepts operation,
source and target as `main` arguments, calls the named `rxfs..` method, and
prints `RXFS_RESULT` plus the returned integer. It follows the in-tree
`lib/plugins/fs/rxfs_test.crexx` pattern. Its path is
`/tmp/crexx-rxfs-stable.kXNQj6/probe.crexx` (SHA-256
`5a11a5d46dd5e47560e8f7253c6918be07eba8740082ae1cfbb0286bb5b628ff`).
The temporary Python fixture driver (SHA-256
`7fc2d057d7c91d7819c0977612a2906816229bd1798dc9e36ec2395d76589dfc`)
created **one fresh directory per call**, set up ordinary files/directories,
valid relative links to a local victim, broken final links and occupied
targets, then ran the bytecode through `rxbvm`. It recorded the return code,
VM status and `lstat`-equivalent after-state of source, target and victim in
`results.jsonl`. There were 53 independent calls; every VM process exited 0
and every result marker parsed. No outside path was a fixture or mutation
target.

Preparation and execution commands, from the checkout unless a working
directory is stated:

```sh
cmake --build cmake-build-debug --target fs rxc rxas rxbvm --parallel 8
cd /tmp/crexx-rxfs-stable.kXNQj6
/Users/adrian/.codex/worktrees/beta3-core-baseline/CREXX/cmake-build-debug/bin/rxc -i /Users/adrian/.codex/worktrees/beta3-core-baseline/CREXX/cmake-build-debug/bin -n -o probe probe.crexx
/Users/adrian/.codex/worktrees/beta3-core-baseline/CREXX/cmake-build-debug/bin/rxas -n -o probe probe
python3 driver.py
```

For each case the driver invoked `rxbvm` with absolute paths to the
checkout's `bin` artifacts and to that case's fresh source/target. One exact
invocation was:

```sh
/Users/adrian/.codex/worktrees/beta3-core-baseline/CREXX/cmake-build-debug/bin/rxbvm /tmp/crexx-rxfs-stable.kXNQj6/probe /Users/adrian/.codex/worktrees/beta3-core-baseline/CREXX/cmake-build-debug/bin/library -a delete /tmp/crexx-rxfs-stable.kXNQj6/cases/01-delete-file-absent/source /tmp/crexx-rxfs-stable.kXNQj6/cases/01-delete-file-absent/target
```

Targeted build passed; it rebuilt `fs` and linked the dynamic provider.
Compilation and assembly passed. Provider module
`bin/providers/rxfs.rxplugin` SHA-256 is
`26a80a58428e139afe473d8707a884f7477366ef08e6eef54b38e6afe33d9b65`,
identical to `bin/rxfs.rxplugin`; temporary `probe.rxbin` SHA-256 is
`1b2e348cb913a69095f9b80e290a66130f9e50a4019a8d540d7265b960707b19`.
The linked `library.rxbin` SHA-256 is
`9809e3de86f04c3ce4e70f604258aff1e0fc3cc9a362491aee5462bbbaf2d11f`.

| Retained log | SHA-256 | Result |
| --- | --- | --- |
| `/tmp/crexx-rxfs-stable.kXNQj6/build.log` | `f3f20d2eacc97416629109a72bb861045b6ea9c6f30e258f693b0bacda5b49ed` | Targeted build PASS. |
| `/tmp/crexx-rxfs-stable.kXNQj6/compile.log` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` | `rxc` PASS, empty diagnostics. |
| `/tmp/crexx-rxfs-stable.kXNQj6/assemble.log` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` | `rxas` PASS, empty diagnostics. |
| `/tmp/crexx-rxfs-stable.kXNQj6/driver.log` | `d8b1e45097ee74382dfb214e49506cc8f69894c7e3e537ff521a0b422c554003` | `53 isolated provider invocations; all VM exits 0 and all results parsed`. |
| `/tmp/crexx-rxfs-stable.kXNQj6/results.jsonl` | `a6aae8b3b11f3c5f686346bf3158f53a09a0b7b4db4306ab9e246d490b97b3ca` | Per-case return, output and after-state. |
| `/tmp/crexx-rxfs-stable.kXNQj6/assertions.log` | `59f232069534844c72800eeae40a756c952ade86f72c236ee2c7c615c4625f1f` | PASS: all 53 records, D-02, overwrite/exclusion, victim integrity and copy/link inode distinctions. |

## Observed legacy statuses

`source` was a fresh entry for each row. “Link file/dir” points to a separate
local victim, which remained intact. `delete` removes an empty directory as
well as an ordinary file on this macOS host, despite its procedure name.

| Initial source | `delete(source)` | `rmdir(source)` | Relevant after-state |
| --- | ---: | ---: | --- |
| Ordinary file | 0 | -4 | Delete removed file; rmdir kept it. |
| Empty directory | 0 | 0 | Each operation removed its own fresh directory. |
| Nonempty directory | -8 | -8 | Directory and child remained. |
| Valid symlink to file | 0 | -4 | Delete removed only link; rmdir kept it; victim file remained. |
| Valid symlink to directory | 0 | -8 | Delete removed only link; rmdir kept it; victim directory remained. |
| Broken final symlink | **-4** | **-4** | Link remained in both cases. |
| Missing source | -4 | -4 | No entry created. |

`rename(source,target)` preserves the historical replacement behavior for
same-type occupied targets. A valid final symlink is moved *as a link*;
the victim remains intact. A broken final symlink is refused before the
pathname rename call.

| Initial source | Initial target | Result | After-state |
| --- | --- | ---: | --- |
| File | Absent | 0 | Source gone; target has original bytes. |
| File | Occupied file | **0** | Source gone; target's old bytes replaced by original bytes. |
| File | Occupied empty directory | -8 | Both entries unchanged. |
| File | Occupied broken symlink | **0** | Source gone; target becomes ordinary file with original bytes. |
| Empty directory | Absent | 0 | Source gone; target is empty directory. |
| Empty directory | Occupied empty directory | **0** | Source gone; target is empty directory. |
| Empty directory | Occupied nonempty directory | -8 | Both directories and target child remain. |
| Valid symlink to file | Absent | 0 | Source gone; target is same symlink; victim file intact. |
| Valid symlink to directory | Absent | 0 | Source gone; target is same symlink; victim directory intact. |
| Broken final symlink | Absent | **-4** | Source link remains; target absent. |
| Missing source | Absent | -4 | Target absent. |

## Observed newer transfer statuses

Each occupied target retained its original bytes, directory or broken-link
entry on failure. Each refused source remained in place; linked victims were
unmodified. For the successful file case, `copy` produced equal bytes with a
**different** inode, while `hardlink` produced equal bytes with the **same**
device and inode. A successful `move` removed the source name. Return values
below are `copy / hardlink / move`; `—` means that combination was not run.

| Initial source | Initial target | Copy | Hardlink | Move | After-state on success or failure |
| --- | --- | ---: | ---: | ---: | --- |
| Ordinary file | Absent | 0 | 0 | 0 | Copy/link keep source; move removes it; target has original bytes. |
| Ordinary file | Occupied file | -8 | -8 | **-8** | Source and target bytes unchanged. |
| Ordinary file | Occupied broken symlink | -8 | -8 | **-8** | Target remains broken symlink; source unchanged. |
| Empty directory | Absent | -8 | -8 | 0 | Move creates directory target and removes source. |
| Empty directory | Occupied empty directory | — | — | **-8** | Both directory entries remain. |
| Nonempty directory | Absent | -8 | -8 | 0 | Move transfers directory with child; source gone. |
| Valid symlink to file | Absent | -8 | -8 | -8 | Source link remains, target absent; victim file intact. |
| Valid symlink to directory | Absent | -8 | -8 | -8 | Source link remains, target absent; victim directory intact. |
| Broken final symlink | Absent | -8 | -8 | -8 | Source link remains; target absent. |
| Missing source | Absent | -8 | -8 | -8 | Target absent. |

This confirms the concrete D-02 compatibility issue: simply calling
`remove` or `rename` first would act on a broken final symlink that this
provider currently leaves intact with `-4`. It also confirms that legacy
`rename` can overwrite an occupied target, while new `move` does not. These
are macOS stable-path observations, **not** evidence that the documented
final-symlink promise survives concurrent substitution. The proposal's
Linux/Windows/error-injection and permission cases remain open; no alert
closure, design choice or broad QA follows from this receipt.
