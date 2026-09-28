# CodeQL #226: controlled final-source substitution

Coordinator review, 28 September 2026, on macOS ARM64. This is a bounded
host probe of the unchanged `rxfs_transfer` function from published develop
`9f2f44cfd888d324858769809b0381e524850b4c`, not a native mainframe test or a
new full product regression. No production source or alert state changed.

The library reference promises that new transfer operations reject a final
symlink/reparse point. It assigns parent traversal and transactional
publication to the caller; it does not promise immutable source identity in
general. This probe therefore tests the explicit final-symlink promise, rather
than inventing a stronger atomic-identity requirement.

## Method and retained evidence

The probe extracts the complete `static int rxfs_transfer` definition directly
from `lib/plugins/fs/rxfs_ops.h`. Its source file SHA-256 is
`3f2f5f25acc4a98c295c0f536f424a2456a7ab8b473f08ada521b7e2ce5b82d7`.
A local macro wraps only `lstat`: immediately after a successful inspection
of an ordinary source file, the wrapper replaces that temporary path with a
symlink to a different temporary regular file. The actual copy, link and
rename system calls remain unchanged. This deterministic hook models another
process changing the final source between inspection and use; it is not a
claim about the probability of winning a live race.

All paths belong to the probe's fresh temporary directory. Each operation has
its own directory and an unchanged-source control. The victim file contains
`victim\n`, while the original source contains `original\n`. Compilation with
`cc -std=c99 -Wall -Wextra probe.c -o probe` succeeds without diagnostics.

Retained directory:
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-codeql226-qwn7eqww`

| File | SHA-256 |
| --- | --- |
| `probe.c` | `8bea303dd56737b12f32ec7f7367f0dd7950aed949e8269ed96afade5214c34e` |
| `build.log` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| `result.log` | `a3128160fda94ff61cb4402688f8d26828b9ef3607fa5c1929fd477cab06ff6c` |
| `identity.log` | `0c12480d081a7080a4643173e8fe713ed4728e21709211b4fd10582bd5d8c309` |

## Observed results

| Operation | Unchanged source | Source replaced with final symlink after `lstat` |
| --- | --- | --- |
| Copy | Returns 0; target contains original bytes. | Returns -8; no target. `O_NOFOLLOW` rejects the replacement. |
| Hardlink | Returns 0; target contains original bytes. | Returns 0; target is a regular-file hardlink to the **victim inode**, with victim bytes. On this macOS host, the link operation follows the substituted final symlink. |
| Move | Returns 0; target contains original bytes. | Returns 0; the target is the substituted symlink. |

All six probe processes exit normally, and the victim bytes remain unchanged.
The result confirms that the documented final-symlink rejection can be bypassed
for hardlink and move under controlled final-source substitution on this host.
It does not establish a Linux/Windows result, privileged execution, an
untrusted writer in an actual deployment, or mainframe applicability. The
copy result covers a substituted symlink only; regular-file/FIFO substitution
and classification of the opened descriptor remain separate review questions.

## Disposition

Keep #226 open. Prepare a platform-specific repair/design proposal that
preserves the documented final-symlink and destination non-overwrite contracts,
or return any necessary contract change to Adrian before implementation.
Neither another pathname check nor a cooperative fileguard establishes atomic
rejection. Do not weaken the contract or claim a complete repair from this
probe. No filesystem edit, suppression, dismissal or extra broad QA run is
part of this review.
