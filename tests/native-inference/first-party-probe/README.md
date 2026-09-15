# First-party probe with an ordinary engine

This isolated QA project runs the existing `rxllama_backend_probe_cycle`
regression in Debug and ASan/LSan without building or instrumenting llama.cpp.
It serves SAN-009's supported-platform closure. It does not test models, GPU
inference quality or the complete public RXPA surface.

`run.py` requires a retained, SHA-verified release-layout provider archive and
the source headers for the exact llama.cpp pin recorded in
`lib/plugins/llama/CMakeLists.txt`. Verify the header source archive against that
file's SHA256 before extracting it. The runner verifies the provider files,
imports its existing engine libraries, builds only our bridge/SHA256/probe
translation units, and runs both configurations through `tools/asan-run.sh`.
It inspects sanitizer symbols and confirms the upstream file hashes remain
unchanged. Linux keeps leak detection enabled; Apple uses its documented
AddressSanitizer-only capability.

```sh
python3 tests/native-inference/first-party-probe/run.py \
  --archive /path/to/verified-provider-archive.zip \
  --sha256 THE_RETAINED_ARCHIVE_SHA256 \
  --source . --llama-source /path/to/verified-llama-source \
  --output /tmp/crexx-first-party-probe
```

Use a fresh output directory outside the source/evidence tree. Retain its root
logs/JSON, each build's `asan-logs`, `Testing/Temporary` and `CMakeCache.txt`.
The `providers` directories and binaries are private test copies and should not
be committed, released or installed. Run results still need the broader core
platform gate and the original-trigger evidence named in
`docs/SANITIZER-WORKLIST.md`; a probe pass alone is not full qualification.
