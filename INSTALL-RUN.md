# Installing And Running CREXX Release Packages

These instructions are for binary packages downloaded from the
[CREXX GitHub Releases](https://github.com/adesutherland/CREXX/releases) page.
Versioned releases are stable distribution points. The `CREXX Dev Snapshot`
pre-release is a moving interim build from the `develop` branch; its assets are
replaced by the next successful `develop` build.

ZIP packages expand to a platform directory such as `CREXX-linux-x64`,
`CREXX-windows-x64`, `CREXX-macos-arm64`, or `CREXX-macos-x86_64`. macOS
`.pkg` packages use the standard macOS Installer flow instead.

The main tools and runtime files are in `bin/`. The release package also
contains `README.md`, `LICENSE`, `SECURITY.md`, `VERSION`, `BUILDINFO`, this
file, and a small `examples/` directory.

`VERSION` contains the exact build identity reported by the packaged tools.
`BUILDINFO` includes the base version, build channel, timestamp, and source
commit used to produce the package.

The next inference-enabled delivery is being qualified as a small core download
plus a separate optional `llama.rexx` plugin download. The core works on its own.
Choose a plugin for the exact same release/commit and platform, and extract it
into the same platform directory. Windows uses one MSVC core with `rxvm`
selecting `rxbvm`; either the Vulkan or CUDA plugin uses that same base.
Linux offers Vulkan or CUDA; Mac uses Metal. Every plugin includes CPU fallback.
These candidate packages are not yet a published release.

The plugin supplies `bin/rxllama.rxplugin`, its engine and dependencies under
`bin/providers`, plus `share/crexx/llama` guides and examples. Keep those files
together. Download a supported model separately; running programs needs no
C/C++ build, separate llama.cpp installation or build SDK. GPU use requires a
compatible driver. Install one matching backend variant, and do not combine
different releases or toolchains. Older releases without the provider do not
acquire this feature automatically.

You can run tools by using their full path, for example:

```sh
./bin/crexx examples/hello.crexx
```

For day-to-day use, add the package `bin/` directory to your `PATH`. The
`crexx` driver finds its packaged runtime files relative to its own location.

## Windows

Download the `windows-x64` ZIP archive and unblock it before extracting:

1. Right-click the downloaded ZIP file.
2. Choose **Properties**.
3. If Windows shows a security message saying the file came from another
   computer, check **Unblock**.
4. Apply the change, then extract the ZIP.

Add the extracted package `bin` directory to your user or system `PATH`, or run
the tools by their full path.

The moving dev snapshot provides an automatic installer:
`CREXX-dev-snapshot-windows-x64-unsigned-setup.exe`. It installs into
`C:\Program Files\CREXX`, sets `CREXX_HOME` and `REXX_HOME`, adds `bin` to the
machine PATH, and registers an uninstaller. Open a new terminal after installing.
Unsigned applications may show unknown-publisher/SmartScreen warnings or be
blocked by Windows security policy.

When present, prefer `CREXX-dev-snapshot-windows-x64-signed-setup.exe` for
installation or `CREXX-dev-snapshot-windows-x64-signed.zip` for portable use.
The maintainer's `scripts/sign-windows-dev-snapshot.sh` signs the complete
payload and installer and publishes both. Unsigned downloads remain available.
Each new snapshot replaces the automatic assets and removes the previous signed
assets and legacy installers, so old code is not offered as the current build.
Check the release's commit and installed `BUILDINFO`/`VERSION` for build identity.

For versioned releases, prefer a signed Windows ZIP when it is present. The
versioned-release ZIP signing helper may remove the corresponding unsigned ZIP.

## Linux

Unzip the `linux-x64` archive to any destination and add the extracted package
`bin` directory to `PATH`.

If the executable bits are not preserved by your unzip tool, restore them with:

```sh
chmod +x bin/*
```

The moving dev snapshot also publishes a prototype Debian package:
`CREXX-dev-snapshot-linux-x64.deb`. Install it with:

```sh
sudo apt install ./CREXX-dev-snapshot-linux-x64.deb
```

The Debian package installs CREXX under `/opt/crexx` and creates command
symlinks in `/usr/bin`. Remove it with:

```sh
sudo apt remove crexx
```

## macOS

Choose the package for your Mac:

- Apple Silicon: `macos-arm64`
- Intel: `macos-x86_64`

### Recommended `.pkg` Install

When a `.pkg` asset is available, prefer it for normal installation. The `.pkg`
is signed, notarized, and stapled so Gatekeeper can validate it locally after
download. It installs CREXX under `/usr/local/crexx` and creates command
symlinks in `/usr/local/bin`.

Install with Finder:

1. Download the matching `.pkg` file.
2. Double-click it.
3. Follow the macOS Installer prompts. macOS may ask for an administrator
   password because the package installs into `/usr/local`.

This is the expected graphical install path for end users.

Optional checks before installing:

```sh
pkgutil --check-signature CREXX-v1.0.0-beta.3-macos-arm64.pkg
spctl --assess --type install --verbose=4 CREXX-v1.0.0-beta.3-macos-arm64.pkg
```

For scripted installs, use Terminal:

```sh
sudo installer -pkg CREXX-v1.0.0-beta.3-macos-arm64.pkg -target /
```

Use the matching `macos-x86_64.pkg` filename on Intel Macs.

After installation, run the included hello world example:

```sh
crexx /usr/local/crexx/examples/hello.crexx
```

Remove the installed files manually if needed:

```sh
sudo find /usr/local/bin -type l -lname '/usr/local/crexx/bin/*' -exec rm -f {} +
sudo rm -rf /usr/local/crexx
sudo pkgutil --forget org.crexx.crexx
```

### Portable ZIP Install

The ZIP remains available as a portable archive for CI, testing, and users who
do not want a system install. Unpack it with Finder or with `ditto`:

```sh
ditto -x -k CREXX-v1.0.0-beta.3-macos-arm64.zip "$HOME/CREXX"
cd "$HOME/CREXX/CREXX-macos-arm64"
```

The macOS ZIP packages are Developer ID signed and submitted to Apple
notarization during the release workflow. They are still portable ZIP archives,
not stapled installer packages.

To verify the ZIP payload signature and signing identity:

```sh
codesign --verify --strict --verbose=2 bin/crexx
codesign -dv --verbose=4 bin/crexx 2>&1 | egrep 'Authority|TeamIdentifier|Runtime'
```

Other tools and plugins under `bin/` can be checked with `codesign --verify`.

For these ZIP-based command-line packages, `spctl --assess --type execute` may
reject an individual `bin/` executable with "the code is valid but does not
seem to be an app". That message does not mean the CREXX signature is invalid;
it reflects that the file is a bare command-line executable rather than an app
bundle or installer package.

If a downloaded package is still blocked by local macOS policy after you have
verified that it came from the CREXX release page and has the expected
Developer ID signature, you can remove quarantine from the extracted package:

```sh
xattr -dr com.apple.quarantine "$HOME/CREXX/CREXX-macos-arm64"
```

Use the matching extracted directory name if you installed the Intel package.

## Verifying The Installation

From the extracted package directory, run the included hello world example:

```sh
bin/crexx examples/hello.crexx
```

Expected output:

```text
hello CREXX world!
```

After adding `bin/` to `PATH`, the same command can be run as:

```sh
crexx examples/hello.crexx
```

For more detail while learning the toolchain, use `-verbose1` through
`-verbose4`.

## Compiling To A Native Executable

The `crexx -native` flow packages a CREXX program as a native executable for
the current operating system and CPU architecture. This requires a local C
compiler.

On Linux, install the usual development tools for your distribution, for
example:

```sh
sudo apt install build-essential
```

On Windows, install the MSYS2 GNU C compiler environment.

On macOS, install the Xcode Command Line Tools. Running `clang` or `gcc` in a
Terminal window usually prompts macOS to install them if they are missing.

Then run:

```sh
crexx examples/hello.crexx -native
```

Native executables and user-built native plugins may have platform-specific
runtime dependencies. Test them on the target platform before distributing
them.
