# Experimental compilers using Level L

These independent compiler services are written in Level B using Level L for
recognition, trees, IR, analysis and output. Native `rxc` builds the tools and
libraries; new C/G source compilation requests use their own frontends and
selectors. Existing RXAS, RXLINK and VM tools assemble, link and execute their
ahead-of-time output.

The active increment closes the agreed Classic C research profile using its
existing instruction/BIF panel and compact interaction checks, with known defects
and limitations recorded explicitly. [PLAN.md](PLAN.md) owns this bounded local
acceptance; it commissions no release, platform, installation, sanitizer or
performance campaign and no publication. New B/G work, self-hosting and default
compiler replacement are outside scope; existing shared/G specifications remain.

## Start here

- [Classic Rexx compiler user guide (Level C)](doc/levelc-user-guide.md):
  installation, a first program, syntax, built-in functions, Unicode/Latin-1
  compatibility, providers, INTERPRET, diagnostics and embedding.
- [Level C architecture](doc/levelc-architecture.md): one account of components,
  source and IR flow, runtime state, static/dynamic execution, ownership and
  exact boundary contracts.
- [Conformance and known limitations](doc/levelc-conformance.md): admitted
  instructions/BIFs, exact runtime bindings, accepted differences and known defects.
- [Authoritative plan](PLAN.md): stable acceptance, decisions and remaining work.
- [Level L user guide](../../../langtools/doc/user-guide.md): the reusable
  SDK/tools independently of these compiler consumers.
- [Frontend](doc/frontends.md), [IR](doc/trees-and-ir.md),
  [backend](doc/backend.md) and [walkthrough](doc/pipeline-walkthrough.md):
  implementation references and worked C/G examples. Earlier examples retain
  their stated historical profiles; use the current C guide for feature status.

## Build and install

From the repository root:

```sh
cmake -S . -B cmake-build-debug \
  -DCREXX_BUILD_RESEARCH_COMPILERS=ON \
  -DCREXX_RESEARCH_LANGUAGES=C
cmake --build cmake-build-debug --target stage-product stage-research-compilers --parallel 8
cmake --install cmake-build-debug --prefix '/tmp/crexx review'
cmake --install cmake-build-debug --prefix '/tmp/crexx review' --component ResearchCompilers
```

Select `G` for the separate G research product; the default is `C;G`. There is
no research B product. Build-tree products are under
`cmake-build-debug/research/level-l/compilers/products/`. The base stage and
installation supply the SDK/tools and ordinary runtime; the research stage and
component add the selected compiler products. Installation copies prepared
outputs and does not build missing products. The research stage includes the
selected C dynamic-runtime image and contract-provider package.

| Product | Purpose |
|---|---|
| `llrxc-c`, `llccompiler.rxbin` | Classic C command and reusable compile service. |
| `llctarget.rxbin` | Runtime adapter for generated C applications. |
| `llcdynamic.rxbin` | Optional same-C-frontend preparation and semantic IR execution. |
| `llrxc-g`, `llgcompiler.rxbin` | Separate G command and compile service over the typed core. |

The services expose `request(source, artifacts, filename, budget, nesting,
bytes, registers, optimise)` with defaults after source; C appends optional
provider bindings, routine export and host export. The commands retain `-i`,
`-o` and `-n`. The old combined `llcompilers.rxbin` and `request(level,...)`
API are retired. Follow the [C guide](doc/levelc-user-guide.md#compiler-options)
or the [G sections of the Level L guide](../../../langtools/doc/user-guide.md)
for their distinct interfaces.

## Research checks

The existing `^llcompiler_c_` CTest selector identifies the Classic panel; the
four supplemental C-only cells carry `levelc-interaction`. They vary compiler
build, semantic IR, assembler and application VM independently over maintained
small cases. A direct `check-dynamic.py --only authored-trace` uses the current
defaults as the TRACE control. The checker's default remains its full existing
43-case workload; selecting a compact panel does not change those oracles.

Reuse qualified baseline inputs and let the QA coordinator own preparation and
execution. The plan records known limitations and future unqualified work; local
C completion neither qualifies G nor switches the ordinary compiler.

## Implementation ownership

| Directory | Responsibility |
|---|---|
| `levelc/` | Classic grammar, token policy, frontend, semantic family, selector, service and CLI. |
| `levelc/runtime/` | Classic target adapter, independent of the compiler host. |
| `levelc/dynamic/` | Optional C IR execution adapter, source resolver, observations and aggregate budget ownership. |
| `typed/` | Shared typed B/G parsing, binding, types, IR and RXAS selection; currently qualified for its G profile. |
| `levelg/` | G service and CLI over the typed core. |
| `levelb/` | Explicit future B profile; not implemented or claimed self-hosted. |
| `common/` | Shared scalar codecs and compiler ABI/metadata interpretation. |
| `tests/` | Maintained component/consumer regressions; mixed-language adapters exist only here. |

Shared B owns Classic values, pools, activations, conditions, BIFs, Unicode and
host algorithms. C owns source policy and adaptation. Static C targets link
the ordinary runtime without a compiler or Level L. Dynamic targets add a
frontend/IR runtime that excludes the RXAS selector, assembler, linker and
native source frontends. The detailed contracts retain conservative code-quality
limits and later work; local functional delivery is not a performance or
production-replacement verdict.
