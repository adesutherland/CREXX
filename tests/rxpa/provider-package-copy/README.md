# Published provider dependency scan

CI-F15's fixture builds a `.rxplugin` module and a small shared-library root in different directories,
with identical copies of one loader-relative dependency. The original package
writer scans those source locations and reports conflicting dependency paths.
The repaired writer scans the already-copied package files. The control verifies
their manifest hashes and separately requires a missing module dependency to
fail. Scanning only ordinary dylib/so/dll suffixes incorrectly accepts that
missing `.rxplugin` dependency.

`provider_package_file_links` calls the public RXPA helper with both LINK_TARGETS
and concrete LINK_FILES, then checks copied linker-file hashes and the module's
discovered runtime dependency. This covers INTERFACE SDK packages whose concrete
import libraries have no owning TARGET_LINKER_FILE, such as PkgConfig::GTK3.

This standalone fixture builds no cREXX tools, llama.cpp engine or models. The
plugin CI job runs it before spending time on an engine build. Its files are
QA data and never enter release archives.

```sh
cmake -S tests/rxpa/provider-package-copy -B /tmp/package-copy -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/package-copy
ctest --test-dir /tmp/package-copy --output-on-failure
```

Use the selected MSVC compiler on Windows. The 13 October Debug controls passed
2/2 in 0.14 s; the same controls with the maintained Apple ASan flags passed
2/2 in 0.20 s. The dependency-scan test remains serial with a 300-second hang
backstop; the generated-file/hash test starts no nested builds or tools and has
the same generous limit. Sanitizer runs use `tools/asan-run.sh`;
the fixture libraries are scanned, not executed as a model/runtime test.

The manifest must list each published runtime dependency once. A declared file
and its discovered package copy have different source paths but the same
published identity. The control rejects duplicate metadata before a later
consumer can attempt to copy a dependency over its own hard link on Windows.
