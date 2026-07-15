# ooRexx installation proof

- Official release page: <https://www.oorexx.org/downloads.rsp>
- Official portable package: <https://sourceforge.net/projects/oorexx/files/oorexx/5.1.0/portable/oorexx-5.1.0-12973.macos.arm64.x86_64-portable-release.zip/download>
- Release/build: ooRexx 5.1.0 r12973, built 2025-05-02
- Package SHA-256: `7d81e2ad65cdd2155cbe27fe6d948d1937e72a0e10099a7b409c79315e870770`
- Published checksum: none found alongside the official portable download;
  the value above is the locally computed archive checksum
- Package cache: `/Users/adrian/.local/cache/oorexx/oorexx-5.1.0-12973.macos.arm64.x86_64-portable-release.zip`
- Installation: `/Users/adrian/.local/opt/oorexx/5.1.0-12973`
- License: Common Public License 1.0 and bundled redistribution terms
- Homebrew state: no `oorexx` formula or cask; `/opt/homebrew/bin/rexx`
  remains the Homebrew Regina 3.9.7 symlink

The installation is the official universal macOS portable build. It is
versioned, user-local, and does not change shell startup files or the Homebrew
Regina command. `install-proof.txt` retains the version, architecture, exact
invocation, and minimal program output.

`setupoorexx.sh` generated the portable environment scripts but reported that
its optional `JavaInfo4BSF4RexxInstallation.class` helper was absent. No Java
package is needed for these interpreter-only cells, and direct version and
program execution passed. The first proof draft called `version()` and ooRexx
resolved the repository's `VERSION` file as an external routine; the retained
proof uses the unambiguous `PARSE VERSION` language instruction.
