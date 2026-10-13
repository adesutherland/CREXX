# Core native package consumer

This model-free fixture builds a dynamic/static RXPA provider whose function
returns 7 through a real shared-library dependency. The production helper writes
its version-1 `.native.json`. Extracted core ZIP smoke copies the provider package
into a private clone of the core, then uses the core's generic metadata helper.
No fixture binaries enter the public ZIP.

The shared package validator is exercised with its existing fingerprint,
identity/hash/traversal/Unicode/idempotency/collision controls. Removing only
crexx-provider-package reproduces native packaging rc=2 after successful
bytecode linkage (the project driver exits 1). Restoring the helper must produce
an EXE whose DLL dependency is adjacent and whose output remains 7 after the
fixture build, producer prefix and intermediate program directories are removed.

The flat bin plugin/DLL copies permit native declaration inspection on Windows;
canonical archives and metadata remain under bin/providers. This uses no llama
engine, model or download during core build/test.
