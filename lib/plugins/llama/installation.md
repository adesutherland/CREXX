# Install llama.rexx

[Guide index](README.md) · [Models](models.md) · [Examples](examples/README.md)

The optional provider runs llama.cpp inside your cREXX program. Its package
supplies the inference libraries; model files are downloaded separately. There
is no inference server to start. Model provisioning and building need network
access unless their inputs are already available; inference uses local files.

This is the current development implementation. CPU and Metal have local macOS
evidence. Windows/Linux and CUDA/Vulkan recipes below are qualification recipes,
not claims that those platforms have passed. See [status](qualification.md).

## Using a binary package

The candidate release pipeline builds the provider into the downloadable cREXX
packages. Qualification is in progress; older releases do not acquire this
feature retrospectively. In a package containing `bin/rxllama.rxplugin` and
`bin/providers/rxllama.native.json`, no C/C++ build, llama.cpp installation or
inference server is needed to run ordinary cREXX programs. Keep the package's
`bin` directory intact, then follow [model provisioning](models.md) and the
[examples](examples/README.md).

Use `rxvm` as the VM entry point. It selects the preferred implementation for
the package's compiler/platform: a relative symlink on macOS/Linux and an
executable copy on Windows. Keep that entry point with the rest of `bin` when
moving or unpacking an installation.

| Package suffix | Included inference backends |
| --- | --- |
| `linux-x64`, `windows-x64` | CPU and Vulkan |
| `macos-arm64`, `macos-x86_64` | CPU and Metal |
| `linux-x64-cuda`, `windows-x64-cuda` | CPU and NVIDIA CUDA |

CUDA ZIPs are complete alternative installations, not overlays for another
package. In particular, Windows CUDA uses MSVC while the ordinary Windows
package uses MinGW; keep each package together. GPU users need a compatible
installed device driver, but not the CUDA/Vulkan build SDK. CPU fallback is
included. Runtime detection selects from the backends in the chosen package.
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

Metal defaults to ON on Apple builds. CPU support is included with every package.
To build a CPU-only package, add `-DCREXX_LLAMA_METAL=OFF` at configure time.
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
