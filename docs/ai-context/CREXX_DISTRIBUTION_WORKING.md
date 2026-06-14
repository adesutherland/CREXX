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
- `CREXX-dev-snapshot-windows-x64.zip`
- `CREXX-dev-snapshot-macos-arm64.zip`
- `CREXX-dev-snapshot-macos-x86_64.zip`

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

The package helper is `scripts/package-linux-deb.sh`.

## macOS And Future Packages

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

Windows should add an MSI after the ZIP signing flow. Preferred flow:

1. GitHub builds and publishes the unsigned Windows ZIP.
2. The local signing script downloads the ZIP.
3. The local signing script signs the Windows binaries/plugins.
4. The signed payload is repacked as a signed ZIP.
5. The MSI is built from the signed payload.
6. The MSI itself is signed and uploaded.
7. The unsigned ZIP is removed after signed assets are visible.

Avoid building an MSI in GitHub, decomposing it locally, signing internals, and
recomposing it unless there is no practical alternative.

Linux `.rpm` should follow the `.deb` package once the installation layout and
asset replacement behaviour are proven.
