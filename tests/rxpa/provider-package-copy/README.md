# Published provider dependency scan

CI-F15's fixture builds two small shared-library roots in different directories,
with identical copies of one loader-relative dependency. The original package
writer scans those source locations and reports conflicting dependency paths.
The repaired writer scans the already-copied package files. The control verifies
their manifest hashes and separately requires a missing dependency to fail.

This standalone fixture builds no cREXX tools, llama.cpp engine or models. The
plugin CI job runs it before spending time on an engine build. Its files are
QA data and never enter release archives.

```sh
cmake -S tests/rxpa/provider-package-copy -B /tmp/package-copy -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/package-copy
ctest --test-dir /tmp/package-copy --output-on-failure
```

Use the selected MSVC compiler on Windows. Local Debug and Apple-ASan fixture
measurements each take 0.09 seconds for the packaging control; its serial
300-second CTest limit is a hang backstop. Sanitizer runs use `tools/asan-run.sh`;
the fixture libraries are scanned, not executed as a model/runtime test.

The manifest must list each published runtime dependency once. A declared file
and its discovered package copy have different source paths but the same
published identity. The control rejects duplicate metadata before a later
consumer can attempt to copy a dependency over its own hard link on Windows.
