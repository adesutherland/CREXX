# Optional llama.rexx installers

The authoritative scope and outstanding qualification are CI-D04 / INST-AC-01–04
in `docs/planning/native-inference-ci.md`. Mac lifecycle is implemented; the
Windows wrapper awaits the backend coexistence choice. These are separate
add-ons to a matching core. Do not change core binaries or download models.

The build-side Python packager consumes **final**, qualified core and plugin
directories, including their post-signing manifests. It verifies every declared
hash and the source/platform/toolchain match before packaging. Recipients need
only the OS shell, not Python or an SDK. Example unsigned QA command:

```sh
python3 scripts/package-llama-installer.py \
  --core /path/to/core/CREXX-macos-arm64 \
  --plugin /path/to/plugin/CREXX-macos-arm64 \
  --unsigned-qa --output /tmp/llama-test.pkg
```

For release packaging use `--sign-identity` (and optionally `--keychain`) instead
of `--unsigned-qa`. Native code must already have valid Developer ID signatures;
an installer signature cannot legitimize unsigned payloads. The existing Mac
release job packages the verified signed inputs, then notarizes, staples and
assesses the final `.pkg` before adding it to the release assets. Portable ZIPs
remain available. Windows' eventual wrapper uses the maintained signing helper
contract for NSIS helper DLLs, embedded uninstaller and final executable.

`--prepare-only --output DIR` writes an inspectable payload plus OS helper for
explicit-path portable installation/testing. On Mac run
`sh DIR/manage.sh install /path/to/CREXX`; the helper supports `check` and
`remove` too. The native `.pkg` discovers the package receipt/default prefix and
environment locations; ambiguous discovery is an error. Its scripts refuse
non-startup target volumes rather than writing into the wrong volume.

Mac removal uses the installed `.llama-installer/manage.sh`. Only manifest-owned,
unchanged files are removed; core and models remain. Remove the plugin before
changing core releases, then install its new matching plugin. A normal interrupted
copy rolls back; an abrupt power loss can leave `.llama-install-lock` and a
`.llama-transaction.*` recovery directory. Preserve these and inspect the prior
files before clearing a stale lock. Never run two installers against one core.

The manual `llama-installer-qa.yml` reuses a successful Build run's artifacts and
does not compile cREXX, llama.cpp or CUDA. It exercises native Mac package install,
reinstall, provider smoke and removal on disposable ARM/Intel runners. Unsigned
lifecycle proof does not close signing or offline Gatekeeper acceptance. Real
offline first-install/run proof must use the final signed/stapled package on a
fresh Mac; do not simulate success by clearing quarantine or disabling Gatekeeper.
