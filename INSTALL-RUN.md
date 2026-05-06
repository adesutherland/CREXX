# Installing And Running CREXX Release Packages

These instructions are for binary packages downloaded from the
[CREXX GitHub Releases](https://github.com/adesutherland/CREXX/releases) page.

Each package expands to a platform directory such as `CREXX-linux-x64`,
`CREXX-windows-x64`, `CREXX-macos-arm64`, or `CREXX-macos-x86_64`.

The main tools and runtime files are in `bin/`. The release package also
contains `README.md`, `LICENSE`, `SECURITY.md`, `VERSION`, this file, and a
small `examples/` directory.

You can run tools by using their full path, for example:

```sh
./bin/crexx examples/hello.rexx
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

Prefer the `windows-x64-signed` ZIP asset when it is present on the release.
The signed package contains Authenticode-signed Windows executables, libraries,
and native plugin binaries.

## Linux

Unzip the `linux-x64` archive to any destination and add the extracted package
`bin` directory to `PATH`.

If the executable bits are not preserved by your unzip tool, restore them with:

```sh
chmod +x bin/*
```

## macOS

Choose the package for your Mac:

- Apple Silicon: `macos-arm64`
- Intel: `macos-x86_64`

Unpack the ZIP with Finder or with `ditto`:

```sh
ditto -x -k CREXX-v1.0.0-beta.2-macos-arm64.zip "$HOME/CREXX"
cd "$HOME/CREXX/CREXX-macos-arm64"
```

The beta 2 macOS packages are Developer ID signed and notarized during the
release workflow. You should not need to remove quarantine attributes as a
normal installation step.

To verify the signature and signing identity:

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
bin/crexx examples/hello.rexx
```

Expected output:

```text
hello CREXX world!
```

After adding `bin/` to `PATH`, the same command can be run as:

```sh
crexx examples/hello.rexx
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
crexx examples/hello.rexx -native
```

Native executables and user-built native plugins may have platform-specific
runtime dependencies. Test them on the target platform before distributing
them.
