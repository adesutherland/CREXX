# Level L compiler product-stage repair

15 October 2026; macOS ARM64 Debug. Source baseline:
`f14d8a569a8ced9420fd27e8d6277db0617825db`. This is the bounded PKG-AC01–03
repair in [PLAN.md](../../PLAN.md), authorised after the provenance publication.
The first substantive push includes the build fix, permanent regression,
documentation and this evidence. Normal Build/CodeQL results are separate.

## Cause and repair

[STEP04 Build CREXX](https://github.com/adesutherland/CREXX/actions/runs/38763409728)
failed SDK installation on macOS ARM64, macOS Intel and Windows after the
essential/smoke tests passed. `cmake --install` could not find
`bin/llcompilers.rxbin`; [failed-log excerpts](hosted-failure.txt) retain the
actual diagnostic. The workflow selects `stage-product`, rather than CMake's
default `ALL` target. `stage-l-libraries` selected only `rxfnsl` and `levell_cli`.
The service, Classic adapter and two compiler commands were all omitted.

The stage now requires `llcompilers`, `llctarget`, `llrxc-c` and `llrxc-g`.
Their existing direct build dependencies remain authoritative; no workflow,
compiler, runtime, install optionality or library format changes are needed.

`llcompiler_product_inventory` is an essential CTest checking all four products
exist and are nonempty, with the configured executable suffix. Its sole prep
target is `stage-product`: adding the individual producers to test preparation
would conceal a missing dependency on clean CI builds. The test only inspects
files; it launches no nested build/compiler/VM workload.

## Decisive checks

1. Saved the four existing delivery outputs and their hashes, then removed
   them from the build's `bin/`. With the original stage selection,
   `cmake --build cmake-build-levell-integration --target stage-product
   --parallel 6` succeeded but recreated none. The new inventory regression
   failed with the missing service: [before.json](before.json),
   [build](before-build.log), [inventory](before-inventory.log).
2. Applied the dependency repair. The same stage-only build regenerated all
   four products in 4.373 seconds: [command](fixed-build.json),
   [log](fixed-build.log). All four SHA-256 hashes equal their saved qualified
   versions: [hashes](product-hashes.json). Timing is an observation only.
3. The registered essential test passed, 1/1:
   [CTest](ctest.log). The generated QA preparation graph includes
   `stage-product`; [graph and registration](graph.txt).
4. Ran the complete `cmake --install` to a new temporary prefix containing
   spaces, exactly the operation that failed in hosted SDK preparation:
   [command](installation.json), [log](install.log). It passed without building
   any individually named compiler target or using the developer install.
5. Thirteen commands against that installed prefix passed:
   [commands](smoke.json), [outputs](consumer-results.txt). Both installed
   C/G commands compiled the pipeline examples; installed RXAS, linker and
   `rxbvm` executed each with output `10`, `0`, `3`. An installed B compiler
   built the existing service consumer against copies of only the installed
   `llcompilers`, `rxfnsl` and `library` packages. It passed 50 service/backend
   assertions. Installed product hashes match the rebuilt products.

[inputs.json](inputs.json) pins the changed build/test inputs and confirms that
compiler/runtime source and the prior qualified binary products are unchanged.
The prior provenance functional checks are reused; this build-selection repair
does not justify another broad compiler, sanitizer or overnight matrix.

This proves the missing-product mechanism and temporary-prefix installation
locally. It does not claim a new Release SDK ZIP, signing/notarization,
cross-platform or hosted CI pass. The ordinary workflows provide those separate
delivery results, and the existing lightweight monitor moves to the repair
commit after publication. No automatic rerun is authorised.
