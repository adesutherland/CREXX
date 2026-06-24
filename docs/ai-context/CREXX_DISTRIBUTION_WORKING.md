# CREXX Distribution Packaging Working Notes

Status: working packaging plan and implementation notes. Verify against
`.github/workflows/build.yml`, `scripts/`, and current release assets before
making claims about shipped behaviour.

## Current Direction

CREXX release distribution should keep portable archives and add native
installers/packages. The portable archives remain useful for CI, development
workflows, and users who do not want system installation. Native packages give
normal users a clearer installation and removal path.

The `dev-snapshot` prerelease is the proving ground for new packaging formats.
Once a package format is stable there, enable the same format for tagged
releases.

## Release Line And Tag Discipline

For the Release 1 beta line, use `develop` as the active mainline and the
Release 1 line as the source for versioned release tags. At beta 3 time that
release line is `master`; if the project later renames or splits it, the same
policy should apply to `release/1`.

Recommended beta publication sequence:

1. Finalize `VERSION`, README, release notes, install docs, security notes, and
   packaging notes on `develop`.
2. Open and merge a PR from `develop` to the Release 1 line.
3. Tag the resulting Release 1 line commit, for example `v1.0.0-beta.3`.
4. Let the tag-driven GitHub Actions workflow create the GitHub release and
   macOS/Linux/Windows CI assets.
5. Run the Windows signing script against the explicit tag, not the implicit
   "latest release" target:

   ```sh
   scripts/sign-latest-windows-release.sh --tag v1.0.0-beta.3 --dry-run
   scripts/sign-latest-windows-release.sh --tag v1.0.0-beta.3
   ```

Do not move a pushed release tag after assets have been published. If only the
GitHub Release body needs a signing-status or checksum clarification, edit the
release body as release metadata. If source-controlled docs, version fields, or
package contents are materially wrong, commit the fix on the release line and
cut a new tag instead of retargeting the old one.

## Asset Model

For each successful `develop` build, the `dev-snapshot` release is updated in
place:

- The Git tag remains `dev-snapshot`.
- The release title is timestamped, for example `CREXX Dev Snapshot
  20260514T104213Z`.
- Stable asset names are replaced by each successful build.
- Windows local signing may remove the unsigned Windows ZIP after the signed
  ZIP is uploaded and visible.

Current dev snapshot assets include:

- `CREXX-dev-snapshot-linux-x64.zip`
- `CREXX-dev-snapshot-linux-x64.deb`
- `CREXX-dev-snapshot-windows-x64-signed.zip` after the local Windows signing
  script has completed
- `CREXX-dev-snapshot-macos-arm64.zip`
- `CREXX-dev-snapshot-macos-arm64.pkg` when Apple signing and notarization are
  configured
- `CREXX-dev-snapshot-macos-x86_64.zip`
- `CREXX-dev-snapshot-macos-x86_64.pkg` when Apple signing and notarization are
  configured

## Linux Debian Package

The first native package is a Debian package for Linux x64. It is generated
from the same staged payload used by the portable Linux ZIP.

Package decisions:

- Debian package name: `crexx`
- Install root: `/opt/crexx`
- Command links: `/usr/bin/<tool>` symlinks to `/opt/crexx/bin/<tool>`
- Package version: derived from the CREXX display version, lower-cased and
  given Debian revision `-1`
- Payload includes `bin/`, `examples/`, `VERSION`, `BUILDINFO`, and release
  documentation

This first `.deb` is intentionally simple. It proves asset creation,
installation layout, symlink behaviour, and dev-snapshot replacement. It does
not yet run a full Debian dependency harvest with `dpkg-shlibdeps`; revisit
dependency metadata before enabling `.deb` artifacts for tagged releases.

The package helper is `scripts/package-linux-deb.sh`. Next hardening work before
promoting this beyond prototype status:

- install and uninstall the generated `.deb` in CI;
- run `crexx /opt/crexx/examples/hello.crexx` from the installed package;
- review shared-library dependency metadata, for example with
  `dpkg-shlibdeps`;
- add an `.rpm` package after the `.deb` install layout and asset replacement
  behaviour are stable.

## macOS Packages

The macOS workflow publishes ZIP assets as portable developer archives. When
Developer ID Installer and notarization secrets are configured, it also builds
`.pkg` packages from the already signed Mach-O payload, signs them with a
Developer ID Installer identity, notarizes them, staples them, verifies them
with `spctl --assess --type install`, and uploads them alongside the ZIPs. ZIPs
can be signed and submitted to notarization, but ZIPs are not the no-network
install solution for CREXX's standalone command-line tools. Maintainer setup
instructions for Apple certificates, notarization credentials, and GitHub
secrets live in `docs/packaging/macos-signing-notarization.md`.

Forks and repositories without Apple signing secrets should still build and
upload unsigned macOS ZIP assets. The workflow should log the skipped
signing/notarization state rather than failing on missing secrets. No macOS
`.pkg` asset should be uploaded unless package signing, notarization, stapling,
and installer assessment all succeed.

The signed, notarized, stapled `.pkg` is now the best end-user install
experience and should be the recommended macOS download. It supports the normal
Finder double-click Installer flow, installs under `/usr/local/crexx`, and
creates command symlinks in `/usr/local/bin`.

## Windows Packages

Beta 2 shipped the Windows ZIP path, with the maintainer signing flow producing
a signed ZIP after the CI asset exists. Beta 3 targets the first Windows
click-through installer while keeping the signed ZIP as the portable asset.

Preferred simple beta 3 flow: add a signed NSIS `setup.exe` after the ZIP
signing flow.

1. GitHub builds and publishes the unsigned Windows ZIP.
2. The local signing script downloads the ZIP.
3. The local signing script signs the Windows binaries/plugins.
4. The signed payload is repacked as a signed ZIP.
5. The NSIS installer is built from the signed payload.
6. The installer `.exe` itself is signed and uploaded.
7. The unsigned ZIP is removed after signed assets are visible.

The installer should:

- install CREXX into a normal Windows location, preferably per-user first to
  avoid unnecessary elevation;
- add the installed `bin` directory to PATH;
- register a normal Windows uninstaller;
- optionally create Start Menu shortcuts for a CREXX command prompt,
  documentation, and uninstall;
- keep the signed ZIP as the portable/CI asset.

WiX/MSI and `winget` remain good later targets, especially for enterprise-style
installation, but NSIS is the simplest next step that fits the current local
Certum/PKCS#11 signing model.
