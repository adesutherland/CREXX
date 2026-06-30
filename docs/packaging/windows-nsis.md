# Windows NSIS Installer Trial

Status: beta 3 packaging spike. The current versioned release remains
`v1.0.0-beta.2`; beta 3 installer assets must not be described as released
until the beta 3 tag and assets exist.

CREXX's Windows installer trial uses NSIS rather than MSI/WiX. This is a local
post-build release helper, not a GitHub Actions build step. The installer is
built from the Windows x64 ZIP asset that the release flow already publishes:
the helper downloads the selected ZIP, unpacks its contents, validates the
expected `bin/*.exe` tools, and uses that packaged tree as the installer
baseline. It does not rebuild Windows artifacts on macOS.

For release-candidate work, sign/repack the Windows ZIP first with
`scripts/sign-latest-windows-release.sh` or
`scripts/sign-windows-dev-snapshot.sh`, then run the installer helper against
that signed ZIP. That keeps the binaries inside the installer Authenticode
signed before NSIS wraps them.

The packaging entrypoints share `scripts/package-windows-installer-common.sh`:

- `scripts/package-latest-windows-installer.sh` defaults to the latest
  versioned release, downloads the selected Windows ZIP, and signs the embedded
  uninstaller and final `setup.exe`.
- `scripts/package-windows-dev-snapshot-installer.sh` defaults to the moving
  `dev-snapshot` release, downloads the selected Windows ZIP, and signs the
  embedded uninstaller and final `setup.exe`.
- `scripts/package-windows-nsis.sh` is the local/generic entrypoint and remains
  unsigned by default unless `--sign` is passed.

## Build Locally On macOS

Install NSIS with Homebrew if `makensis` is not already available:

```sh
brew install makensis
```

Build and sign from the latest versioned release or an explicit release tag:

```sh
scripts/package-latest-windows-installer.sh \
  --tag v1.0.0-beta.2 \
  --asset CREXX-v1.0.0-beta.2-windows-x64-signed.zip \
  --output dist/CREXX-v1.0.0-beta.2-windows-x64-setup.exe
```

Build and sign from the interim dev snapshot:

```sh
scripts/package-windows-dev-snapshot-installer.sh \
  --output-dir dist
```

Build without signing from a local ZIP:

```sh
scripts/package-windows-nsis.sh \
  --zip /path/to/CREXX-v1.0.0-beta.2-windows-x64-signed.zip \
  --output dist/CREXX-v1.0.0-beta.2-windows-x64-setup.exe
```

The script unpacks the ZIP, detects the single top-level payload directory, and
passes that directory to `packaging/windows/crexx.nsi`. The installer copies the
payload contents directly under `%ProgramFiles%\CREXX`, so the installed tools
land in `%ProgramFiles%\CREXX\bin`.

When signing is enabled, the common script creates a temporary signing helper and
passes it to NSIS through `!uninstfinalize`. That signs the embedded
`Uninstall.exe` before it is written into the installer. After NSIS finishes,
the same helper signs the outer `setup.exe` and verifies it with
`osslsigncode`.

Use `--upload` to upload the generated installer to the selected GitHub release
with `gh release upload --clobber`. The upload target is the same release/tag
from which the ZIP was downloaded.

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
