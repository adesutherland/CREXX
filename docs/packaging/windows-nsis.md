# Windows NSIS Installer Trial

Status: beta 3 packaging spike. The current versioned release remains
`v1.0.0-beta.2`; beta 3 installer assets must not be described as released
until the beta 3 tag and assets exist.

The moving dev snapshot automatically publishes an unsigned NSIS installer and
portable Windows ZIP from the same staged payload. Windows CI checks silent
installation, reinstall, file hashes, tool versions, the installed hello example,
and uninstallation before publishing. Versioned-release installer publication
remains a maintainer operation.

Snapshot asset names are explicit:

- `CREXX-dev-snapshot-windows-x64-unsigned-setup.exe`: automatic tester installer.
- `CREXX-dev-snapshot-windows-x64.zip`: automatic portable payload.
- `CREXX-dev-snapshot-windows-x64-signed-setup.exe`: optional signed installer.
- `CREXX-dev-snapshot-windows-x64-signed.zip`: optional signed portable payload.

A new snapshot replaces the unsigned assets and removes previous signed assets
and the legacy ambiguous `CREXX-dev-snapshot-windows-x64-setup.exe`. Signed and
unsigned downloads coexist until the next snapshot. Release notes include the
source commit and checksums for the automatic assets; installed BUILDINFO and
VERSION identify the payload commit.

## Sign And Publish The Current Snapshot On macOS

Install `makensis`, `jsign`, `osslsigncode`, GitHub CLI and Python 3. Use the
existing Certum SimplySign login and `scripts/provider.macos.cfg` configuration.
Then run this one command from the repository:

```sh
scripts/sign-windows-dev-snapshot.sh
```

This downloads the unsigned ZIP by immutable GitHub asset ID, verifies its
SHA-256 and BUILDINFO against the tag, signs/verifies Windows executables,
libraries and native plugins, and creates the signed ZIP. It then builds NSIS
from that payload, signs private copies of the embedded NSIS helper DLLs, signs
the embedded uninstaller, and signs/verifies the final setup. Both signed
outputs are uploaded together through staged temporary assets. The unsigned
ZIP and installer remain available; `--delete-unsigned` is refused for snapshots.

No separate installer command or `--upload` is needed for this snapshot signing
entrypoint. `--dry-run` reports its selected source/options without signing or
uploading; `--keep-work` retains local output. PROVIDER, CERTUM_ALIAS and TSA_URL
still select the existing signing configuration.

Publication verifies the tag commit and source asset identity again before and
after promotion. If a new snapshot arrives while signing, the helper refuses
publication and removes only its own temporary/published asset IDs. Rerun on
the current snapshot. GitHub publication is a sequence of API operations, not
an atomic multi-asset transaction; failed runs can temporarily leave signed
assets absent, while the unsigned downloads remain available.

## Other Packaging Entrypoints

The packagers share `scripts/package-windows-installer-common.sh`:

- `scripts/package-latest-windows-installer.sh` selects a versioned release and
  signs by default.
- `scripts/package-windows-dev-snapshot-installer.sh` selects the snapshot and
  signs by default. Use this when only a local installer is wanted.
- `scripts/package-windows-nsis.sh` is unsigned by default.

All accept `--sign` or `--unsigned`. Signing includes the payload even when the
input ZIP is unsigned. Default output names end in `-signed-setup.exe` or
`-unsigned-setup.exe`; an explicit `--output` still controls the filename.
Packagers upload only with `--upload` and validate the selected release source.

For a local unsigned installer:

```sh
scripts/package-windows-nsis.sh \
  --zip /path/to/CREXX-dev-snapshot-windows-x64.zip \
  --unsigned --output-dir dist
```

For a versioned-release installer (use the intended explicit release tag):

```sh
scripts/package-latest-windows-installer.sh \
  --tag v1.0.0-beta.2 --output-dir dist --upload
```

The ZIP-only `scripts/sign-latest-windows-release.sh` retains its versioned
release behaviour; the snapshot entrypoint is the combined sign-and-publish
command. Neither path rebuilds Windows binaries on macOS.

## SmartScreen Reputation

The signed NSIS installer can still trigger the Microsoft Defender SmartScreen
"Windows protected your PC" prompt on early downloads. Authenticode signing and
timestamping prove the publisher identity and protect the installer from
tampering, but SmartScreen also uses reputation signals for the specific
download, signing certificate, and observed install/download volume.

For a new CREXX installer or a low-volume beta asset, testers may need to choose
**More info** and then **Run anyway**. This is expected while reputation builds.
It should improve as signed installer downloads and successful installs
accumulate. Keep the signed ZIP available as the portable fallback while this
installer reputation is still maturing.

## Installer Behavior

- Requests administrator elevation.
- Requires 64-bit Windows.
- Defaults to `%ProgramFiles%\CREXX`.
- Shows `packaging/windows/assets/crexx-wizard.bmp` on the welcome and finish
  pages. NSIS Modern UI expects this bitmap at `164x314`.
- Sets machine-level `CREXX_HOME` and `REXX_HOME` to the install directory.
- Adds the install `bin` directory to the machine `Path`.
- Broadcasts the Windows environment-change message after install and
  uninstall.
- Registers an uninstaller in Windows Apps & Features.
- Removes the installed files, `CREXX_HOME`, `REXX_HOME`, and the PATH entry on
  uninstall. Existing environment variables are only deleted when they still
  point at the install directory.

## Windows Smoke Test

After copying the generated `setup.exe` to a Windows x64 machine:

1. Launch the installer and keep the default destination.
2. Open a new terminal.
3. Confirm `CREXX_HOME` and `REXX_HOME` point to `C:\Program Files\CREXX`.
4. Confirm `Path` includes `C:\Program Files\CREXX\bin`.
5. Run:

```powershell
crexx --version
rxc --version
rxas --version
rxlink --version
rxvm --version
```

Then uninstall through Apps & Features or run:

```powershell
& 'C:\Program Files\CREXX\Uninstall.exe'
```

Open a new terminal and confirm the install directory, environment variables,
and PATH entry were removed.
