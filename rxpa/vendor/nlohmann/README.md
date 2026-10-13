# Core JSON header

Unmodified nlohmann/json 3.12.0 single-header dependency, MIT licensed.
The header and license were extracted from the verified llama.cpp source archive
already pinned by this repository at 5266f24da75dc449bd56cbed7addb9c8e4a6a73e.
`provenance.json` records the archive and file SHA-256 values. The header hash
matches the retained Windows engineering helper exactly.

This local copy lets the generic crexx-provider-package build with
ENABLE_LLAMA=OFF and without a network fetch or inference engine dependency.
Its existing copier/validator and the llama loader still share package.hpp;
the platform/architecture macro names and package version remain unchanged.
The MIT license/provenance are installed in share/crexx/licenses/nlohmann-json.
