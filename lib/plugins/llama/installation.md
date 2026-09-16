# Install llama.rexx

[Guide index](README.md) · [Models](models.md) · [Examples](examples/README.md)

The optional provider runs llama.cpp inside your cREXX program. Its package
supplies the inference libraries; model files are downloaded separately. There
is no inference server to start. Model provisioning and building need network
access unless their inputs are already available; inference uses local files.

Source builds keep `ENABLE_LLAMA=OFF` by default. Add `-DENABLE_LLAMA=ON` to
experiment with the provider; CPU is included, Metal defaults on for macOS,
and CUDA/Vulkan remain explicit SDK-dependent choices. The candidate binary
release pipeline enables and packages these dependencies for recipients.

This is the current development implementation. Both trained models have local
macOS CPU/Metal evidence. Generated-fixture package checks also pass on the
recorded Linux, Windows MinGW and Intel Mac candidates; wider and real-device
qualification remains open. See [status](qualification.md).

## Using a binary package

Separate plugin installers are implemented under CI-D04. Unsigned native
lifecycle checks pass on ARM/Intel Mac and Windows with both Vulkan/CUDA;
signing and final release acceptance remain open. The Mac `.pkg` checks the existing cREXX package
receipt (normally `/usr/local/crexx`) and any supplied `CREXX_HOME`/`REXX_HOME`
locations, then installs only into one exact matching core. Missing, ambiguous,
altered or incompatible installations stop before plugin copying. Core and
plugin must have the same release/commit, platform and toolchain. The installers
do not download models or change PATH or the preferred VM.

Close programs using the plugin before installing or removing it. The Mac
installer can be rerun for the same release. To remove an installer-managed
plugin while retaining the core and models, run:

```sh
sudo sh /usr/local/crexx/.llama-installer/manage.sh remove /usr/local/crexx
```

Use your actual core directory if different. Remove the plugin before upgrading
the core, then install the matching plugin release. Changed plugin files are
reported rather than silently deleted. Package receipts can remain after removal;
the installed file/ownership checks determine whether the plugin is present.

Release Mac plugin packages must have signed code, a signed installer and a
stapled notarization ticket. The ticket provides local notarization evidence for
offline installation; it does not disable Gatekeeper or guarantee macOS never
contacts Apple.

The Windows installers keep Vulkan and CUDA in separate backend directories.
Either installer supplies the same **native cREXX program**, `crexx-llama`:

```text
crexx-llama status
crexx-llama use vulkan
crexx-llama use cuda
```

Close programs using llama.rexx before switching; use an administrator terminal
for a system-wide installation. Installing a second backend preserves the active
choice unless you select its activation checkbox. Switching checks the matching
core and file hashes and restores the prior files if publication fails. Models,
PATH and core binaries remain unchanged. There is no PowerShell switcher,
first-run compilation, inference initialization or model download.

Remove each backend through its own Windows installed-app entry. Removing an
inactive variant leaves the active variant alone; removing the active one leaves
no active plugin until you explicitly select another installed variant. The
shared management program remains while either variant is installed. Previously
packaged native applications retain their own bundled dependencies. Portable ZIP
variants still share filenames: use only one ZIP variant per directory.
Installer qualification remains tracked in CI-D04; these instructions do not
claim a published/signed release.

The candidate release pipeline supplies a llama-free cREXX core and a separate
optional prebuilt `llama.rexx` package. Download the core and one plugin for the
exact same release/commit and platform, then extract both ZIPs into the same
parent folder so their shared `CREXX-<platform>` directory combines. Qualification is in progress; older releases do not acquire this
feature retrospectively. In the combined installation containing `bin/rxllama.rxplugin` and
`bin/providers/rxllama.native.json`, no C/C++ build, llama.cpp installation or
inference server is needed to run ordinary cREXX programs. Keep the package's
`bin` directory intact, then follow [model provisioning](models.md) and the
[examples](examples/README.md).

Use `rxvm` as the VM entry point. It selects the preferred implementation for
the package's compiler/platform: a relative symlink on macOS/Linux and an
executable copy on Windows. Keep that entry point with the rest of `bin` when
moving or unpacking an installation.

| Plugin platform/backend | Included inference backends |
| --- | --- |
| `linux-x64-vulkan`, `windows-x64-vulkan` | CPU and Vulkan |
| `macos-arm64-metal` | CPU and Metal |
| `macos-x86_64-cpu` | CPU only |
| `linux-x64-cuda`, `windows-x64-cuda` | CPU and NVIDIA CUDA |

Choose Vulkan as the general Windows/Linux option, or the separate CUDA plugin
for NVIDIA. The approved full-release policy includes prebuilt CUDA packages
for both platforms, even though users need only their chosen variant. Every
full release build, including beta, must build, package and smoke-test those
CUDA variants; valid compiler-cache reuse is allowed. Otherwise CUDA builds and
tests run only on explicit manual GitHub Actions requests. Ordinary push, PR and
development-snapshot CI does not select CUDA. Smoke means small package/load/
fixture checks within the selected CUDA build, not a separate routine test job.
CI-D03 is implemented in workflow selection and snapshot/release asset checks.
Manual dispatch defaults to `base`; choose `all` or a CUDA lane explicitly.
This policy does not claim that the candidate packages have already been released.

CUDA and Vulkan plugins use the same core for their platform. Windows uses
MSVC for the core and both plugin variants, with `rxvm` selecting `rxbvm`.
MinGW remains supported for source builds and has a separate core QA gate for
both VM variants; it does not produce a binary download. Install one
backend variant at a time and preserve the release/commit match. GPU users need a compatible
installed device driver, but not the CUDA/Vulkan build SDK. CPU fallback is
included. Runtime detection selects from the backends in the chosen package.
Intel Mac Metal is unsupported in this delivery: the pinned engine repeatedly
stalled during Metal compiler-service initialization on the Intel runner. The
Intel CPU package omits Metal, so startup does not enter that path. This does
not imply that all Intel Macs lack Metal hardware support. ARM Mac Metal is
included.
CUDA builds retain the pinned engine's portable architecture defaults for the
selected toolkit. Some devices compile the supplied PTX on first use, which can
add startup time; keep the model/session prepared for repeated work. Real-device
qualification remains separate from compiling and packaging those targets.

The release contains these guides, examples and dependency notices, but no
model weights. The tiny random-weight developer fixture is not a useful model
and is deliberately excluded. Download a supported BGE or Smol model using
the model guide. Creating a native executable with `crexx --native` still needs
a C toolchain on the author's machine; its recipient needs only that prepared
application's complete runtime package and model files.

## Building and installing on macOS or Linux

Start at the root of a cREXX checkout containing `lib/plugins/llama`. Install a
C/C++ toolchain, CMake 3.24 or newer, and Ninja. macOS needs the Apple developer
tools/SDK for Metal; Linux needs its normal C/C++ development tools. A native
cREXX executable also needs the platform C toolchain when it is built, but its
recipient needs only the resulting runtime package and local models.

Choose a writable install prefix. These commands enable source downloads at
build time and disable the unrelated editor parser integration to avoid its
optional source dependency. Keep a separate build directory from existing work.

```sh
crexx_prefix="$HOME/.local/crexx-llama"
cmake -S . -B build-llama -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$crexx_prefix" \
  -DENABLE_LLAMA=ON -DCREXX_ALLOW_NETWORK_DOWNLOADS=ON \
  -DENABLE_PARSER_MODE=OFF
cmake --build build-llama --target stage-c1-toolchain stage-product stage-optional \
  llama_provider_runtime_package crexx-provider-package --parallel 4
cmake --install build-llama --prefix "$crexx_prefix"
```

Metal defaults to ON on Apple source builds. On Intel Mac, add
`-DCREXX_LLAMA_METAL=OFF` to use the supported CPU-only configuration. CPU support
is included with every package. Intel Metal source experiments are unqualified.
For Linux CUDA or Vulkan, add **one** of `-DCREXX_LLAMA_CUDA=ON` or
`-DCREXX_LLAMA_VULKAN=ON` and install the matching build SDK/toolkit and device
driver. Backend options select what gets packaged; runtime detection can only
use those packaged backends. An installed GPU driver is still required on a
discrete-GPU target. Runtime selection is described in [the reference](reference.md).

CUDA builds also require `-DCREXX_LLAMA_CUDA_NOTICE=/absolute/path/to/notices.txt`
containing the NVIDIA redistributable license/notices for the packaged runtime
components. The CI SDK provisioner assembles this file from the pinned component
licenses. Its contents become part of `rxllama-NOTICES.txt`, so native application
packages retain them as well as the ordinary release ZIP.

Build `llama_provider_runtime_package` and `crexx-provider-package` explicitly:
`stage-optional` alone can leave an older adapter or missing dependency metadata
in an otherwise working install. Retain the install's `bin` directory intact,
including static archives used by `crexx --native`.

## Windows build recipe — pending Windows qualification

Use an x64 Visual Studio developer PowerShell with the C++ build tools, Windows
SDK, CMake 3.24+ and Ninja available. Using Ninja in that developer environment
avoids mixing single- and multi-configuration output paths. Begin in the cREXX
checkout. Do not overlap builds and tests in the same Windows build directory.

```powershell
$crexxPrefix = Join-Path $env:LOCALAPPDATA 'crexx-llama'
cmake -S . -B build-llama -G Ninja `
  -DCMAKE_BUILD_TYPE=Release "-DCMAKE_INSTALL_PREFIX=$crexxPrefix" `
  -DENABLE_LLAMA=ON -DCREXX_ALLOW_NETWORK_DOWNLOADS=ON `
  -DENABLE_PARSER_MODE=OFF -DCREXX_LLAMA_METAL=OFF
if ($LASTEXITCODE -ne 0) { throw 'Configure failed' }
cmake --build build-llama --target stage-c1-toolchain stage-product stage-optional `
  llama_provider_runtime_package crexx-provider-package --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
cmake --install build-llama --prefix $crexxPrefix
if ($LASTEXITCODE -ne 0) { throw 'Install failed' }
```

For NVIDIA CUDA, add `-DCREXX_LLAMA_CUDA=ON` to configure with a compatible CUDA
toolkit and driver installed, and supply `CREXX_LLAMA_CUDA_NOTICE` as above.
For Vulkan, add `-DCREXX_LLAMA_VULKAN=ON` with the
Vulkan SDK and a Vulkan-capable driver. A CPU-only build needs neither GPU SDK.
Do not assume an ordinary Windows CI runner supplies a real GPU. MSVC supplies
`rxbvm.exe` and the product `rxvm.exe`; a second VM executable is not required.
The pinned upstream [build notes](https://github.com/ggml-org/llama.cpp/blob/5266f24da75dc449bd56cbed7addb9c8e4a6a73e/docs/build.md)
describe backend prerequisites; cREXX uses the `CREXX_LLAMA_*` options above.

## Building with an already downloaded source archive

The engine is pinned to llama.cpp commit
`5266f24da75dc449bd56cbed7addb9c8e4a6a73e` and its in-tree GGML.
Download the [exact source archive](https://codeload.github.com/ggml-org/llama.cpp/tar.gz/5266f24da75dc449bd56cbed7addb9c8e4a6a73e)
on a connected machine. Expected SHA-256:

```text
2de0d87eda4696e9f6bbd771d4c623267f4e95856cce6f99793f91522f993e43
```

Add `-DCREXX_LLAMA_ARCHIVE=/absolute/path/to/archive.tar.gz` and
`-DCREXX_ALLOW_NETWORK_DOWNLOADS=OFF` to configure; keep
`-DENABLE_PARSER_MODE=OFF` unless its separate source is also provisioned.
CMake verifies the archive hash before extraction. This option provisions the
engine source, not model data. Retain the archive and cREXX checkout identity
with your build record. Do not replace the pinned engine with a newer system
llama.cpp library and call it the same package.

## What is installed and what to distribute

The install contains the toolchain and bytecode in `bin`, the RXPA provider and
its declared native/runtime manifests, and versioned inference dependencies in
the provider package locations. The runtime notice is `rxllama-NOTICES.txt`.
These guides and four examples are in `share/crexx/llama`.
Windows also places the small bridge/engine core DLLs and their runtime
dependencies beside the executables in `bin`, so plugin startup needs no SDK
on PATH. GPU libraries remain in `bin/providers`; verified backend loading
resolves their dependencies there. Keep the entire installed directory together.

Start with [model provisioning](models.md), then follow the complete
[example commands](examples/README.md). Use the chosen prefix's `bin/crexx`
explicitly if another cREXX install is on PATH. When building a native program,
set `CREXX_HOME` to this prefix for tool/library discovery.

`crexx --native` publishes the program with adjacent provider dependencies and
manifests. Distribute the executable **and its declared runtime files/notices**;
copying the executable alone is insufficient. Keep the generated directory
together, or copy the `runtime_files` entries from each `.native.json` along
with the executable and manifests. Paths in those manifests are relative.
Models stay in a separately chosen data directory. Installed-VM programs use
the installed provider package; they do not require a separate llama.cpp CLI.

GPU discovery reads verified package files from canonical locations, not
`GGML_BACKEND_PATH`. Dependency hashes and platform identities are checked.
On Windows, missing dependent DLLs may prevent process startup before a cREXX
diagnostic can be produced; check package completeness and the driver first.

## Installation troubleshooting

| Symptom | Action |
| --- | --- |
| `import llama` cannot resolve | Check that the selected cREXX prefix was built with `ENABLE_LLAMA=ON`, that both explicit packaging targets were built, and that the complete install was used. |
| Source-download configure error | Allow build-time downloads or provide the exact source archive with the expected hash. |
| Native package missing or stale | Rebuild both explicit provider packaging targets, reinstall to the intended prefix, then rebuild the native consumer into a clean output directory. |
| Package hash/identity error | Restore a complete matching package; do not edit manifests to bless mismatched files. |
| Unowned runtime dependency collision | Use a fresh application output directory or deliberately manage the old package; the helper will not overwrite unrelated files. |
| Required GPU unavailable | Confirm the backend was packaged, the driver/device is usable and the configured memory budget admits the model/context. Inspect device inventory and `selection`. |
| Windows cleanup cannot remove generated files | Finish/stop leftover build or VM processes, then retry sequentially in that build tree. |

## Maintainer signing of split Windows packages

The Build run retains a `llama-manager-<commit>-windows-x64` artifact shared by
both backend installers. Download it and the exact matching core/plugin ZIPs.
With SimplySign Desktop logged in, prepare signed archives and installers without
publishing or rebuilding the engine:

```sh
python3 scripts/sign-windows-packages.py \
  --core /path/CREXX-user-test-COMMIT-windows-x64.zip \
  --plugin /path/llama.rexx-user-test-COMMIT-windows-x64-vulkan.zip \
  --plugin /path/llama.rexx-user-test-COMMIT-windows-x64-cuda.zip \
  --manager /path/unpacked-manager --output /path/new-signed-output
```

The helper validates matching identities and original hashes, signs/verifies PE
payloads, refreshes provider and package manifests, preserves the selected
`rxvm.exe` copy, and signs the native manager, NSIS helpers, uninstallers and
installers. `signed-delivery.json` records input/output hashes. Omit the CUDA
argument for a Vulkan-only development snapshot. Recipient machines need none
of these packaging tools. Signing, native Windows consumer QA and publication
remain separately recorded actions.

The older release-asset signer defaults explicitly to the **core** ZIP; use
`--asset` to select a plugin ZIP. That single-asset operation does not build a
plugin installer. Use the paired-input helper above for installers so their
core hash expectations describe the same signed core users will install.
