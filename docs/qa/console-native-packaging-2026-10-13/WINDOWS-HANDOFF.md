# Windows native UI and signed core handoff

This correction supplies core console static closure and the RXPA dependency
inputs needed by the Windows GTK overlay. The Windows task owns GTK target/app
implementation and native Windows qualification. No release tag or signed
distribution was created by this Mac task.

The continuation also supplies `crexx-provider-package.exe` from llama-free core
RXPA/product/driver builds. Its existing validator/fingerprint/copier code is
unchanged and uses the exact pinned nlohmann header already used in the Windows
engineering helper (SHA-256 aaf127c04cb31c406e5b04a63f1ae89369fccde6d8fa7cdda1ed4f32dfc5de63).
The installed core/SDK now require this helper and its MIT notice. Replace the
temporary engineering-only helper with the matched product output after fetching
the correction. Partial-flat runtime scanning accepts only identical duplicate
library bytes, stages their transitive closure, and rejects differing hashes or
missing dependencies; the 44-DLL manual pre-staging workaround is not the intended
normal package path.

## Build and verify the matching core

Fetch the published develop correction into the isolated Windows worktree while
preserving the approved UI overlay. Build and test sequentially in that tree:

```text
cmake --build <build> --target stage-c1-toolchain stage-product stage-optional sqlite sqlite_static vector vector_static --parallel <jobs>
```

The C1 stage supplies public tool aliases as well as the selected VMs. Product
selection now explicitly includes console_static. Confirm the installed payload
contains all three entries below, and that both archive hashes match:

```text
bin/providers/rxconsole.rxplugin
bin/providers/rxconsole.lib
bin/providers/rxconsole_static.lib
```

Run the normal core ZIP and SDK package scripts with the Windows platform,
MSVC toolchain and actual producer commit. Core ZIP smoke now compiles the
headless console contract with --native and runs its EXE after relocation.
SDK inventory requires both the console plugin and canonical static archive.
Beta 3's signed Windows ZIP was independently inspected: its source is
ae1607b8e145174422cee7f3e73fbcc37a65226c and it has only the dynamic console
plugin. Do not mix a development static archive with that older SDK/core.

## Declare the GTK provider package

The existing version-1 native manifest now accepts concrete LINK_FILES and scans
declared .rxplugin modules for their shared-library dependencies. Use the real
GTK import/archive paths and DLL directory, for example:

```cmake
add_rxpa_provider_package(_ui_gtk_native
    LINK_FILES ${GTK3_LINK_LIBRARIES}
    RUNTIME_TARGETS _ui_gtk_native
    RUNTIME_SEARCH_DIRECTORIES "${GTK_RUNTIME_DLL_DIR}")
add_dependencies(ui_gtk
    _ui_gtk_native_static _ui_gtk_native_runtime_package)
```

GTK_RUNTIME_DLL_DIR means the actual SDK DLL directory, typically its bin
directory; GTK3_LIBRARY_DIRS commonly names the import-library directory.
PkgConfig::GTK3 is INTERFACE and has no TARGET_LINKER_FILE. Keep its normal
target_link_libraries use for compiling/linking the provider. LINK_FILES carries
its resolved concrete files into the package manifest instead. The helper's
runtime-package target is ALL, so ui_gtk needs the explicit dependency for a
selective product build. The static provider archive also needs that edge.

The native GTK EXE uses the packaged GTK runtime DLLs. Verify their declared
hashes and test the relocated app with producer/MSYS2/core directories absent
from PATH. Confirm the three exclusive country choices, integer spin range,
generation, Save/confirmation/write and cancellation in both console and GTK
surfaces. Check any GTK runtime resources used by the app as part of that
relocation proof. The helper changes do not claim native Windows app completion.

## Existing signed snapshot route

The ordinary route remains:

```sh
scripts/sign-windows-dev-snapshot.sh --repo adesutherland/CREXX --dry-run
# After native app qualification and verification of the selected source:
scripts/sign-windows-dev-snapshot.sh --repo adesutherland/CREXX --keep-work
```

It defaults to the moving dev-snapshot and signs/publishes the matching core
and selected llama ZIPs plus their installers together. It requires at least
one unsigned Windows llama ZIP (normally Vulkan); selecting --asset for a
plugin still includes its matching core. It does not accept an arbitrary GTK
app bundle as a snapshot input and does not rebuild Windows binaries on macOS.

Before the actual signing call, the printed commit must be the qualified source
cohort. If the moving tag now selects another commit, stop and reconcile the
proof instead of signing a different build. A successful matching Build must
have produced:

- CREXX-dev-snapshot-windows-x64.zip with the fixed console archives, matching
  BUILDINFO/VERSION and core-package.json hashes;
- the matching llama.rexx-dev-snapshot-windows-x64-vulkan.zip (or selected CUDA
  plugin), with its package identities and hashes;
- the unexpired llama-manager-<commit>-windows-x64 Actions artifact from that
  successful .github/workflows/build.yml run, including GitHub SHA-256 and
  package/bootstrap hashes. Retention is currently 14 days.

On macOS the route requires gh, Python 3, jsign, osslsigncode, makensis, unzip,
zip and file, plus the established Certum SimplySign login/configuration.
It verifies source ZIPs and identities before key use, refreshes manifest hashes
after Authenticode signing, signs payloads/helper DLLs/uninstallers/setups,
verifies outputs, and stages/promotes signed assets while retaining unsigned
downloads. It rechecks source/tag identities during publication and removes only
its own asset IDs if the snapshot moves. GitHub multi-asset publication is a
sequence of API operations, so it must not be described as atomic.

Read-only inspection on 13 October selected source
5c57df76f7c80c6ebee1a2288d939199a41103e2, manager artifact 11773080151 and
successful Build 38302906672. That is evidence for the existing route, not the
yet-to-be-published correction or native GTK proof. Wait for the matching new
cohort after both native app tests pass; do not cut/tag an interim release now.
