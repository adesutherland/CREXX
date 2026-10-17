# Experimental compilers using Level L

These independent compilers are research consumers of Level L. All grammar,
frontend, semantic and RXAS-emission code is Level B. Native `rxc` is the build
seed; source compilation requests never call it.

| Owner | Responsibility |
|---|---|
| `levelc/` | Classic grammar, token policy, frontend, semantic family, selector, service and CLI |
| `typed/` | Shared typed B/G parsing, binding, types, IR and RXAS selection; currently qualified for G |
| `levelg/` | G service and CLI over the typed core |
| `levelb/` | Explicit future B profile; not implemented or claimed self-hosted |
| `common/` | Checked scalar codecs/evaluation and compiler ABI interpretation |
| `levelc/runtime/` | Classic target adapter; separate from the compiler host |
| `tests/` | C/G regressions; mixed-language adapters exist only here |

Build explicitly from the repository root:

```sh
cmake -S . -B cmake-build-debug -DCREXX_BUILD_RESEARCH_COMPILERS=ON
cmake --build cmake-build-debug --target stage-product stage-research-compilers --parallel 8
```

Select one language with `-DCREXX_RESEARCH_LANGUAGES=C` or `G`; the default is
`C;G`. No B product is offered. Products are in
`cmake-build-debug/research/level-l/compilers/products/`.

Normal installation includes the SDK and Level L tools. Install research
products explicitly after installing that base:

```sh
cmake --install cmake-build-debug --prefix '/tmp/crexx review'
cmake --install cmake-build-debug --prefix '/tmp/crexx review' --component ResearchCompilers
```

The services are `llccompiler.rxbin` and `llgcompiler.rxbin`. The old combined
`llcompilers.rxbin` and `request(level,...)` API are retired. Each service exposes
`request(source, artifacts, filename, budget, nesting, bytes, registers, optimise)`;
arguments after source retain their defaults. See [backend](doc/backend.md).
Both commands preserve `-i`, `-o` and `-n`. Use installed packages with the
existing `rxas`, `rxlink` and VM for execution.

[The plan](PLAN.md) owns scope and unfinished criteria.
[The walkthrough](doc/pipeline-walkthrough.md) and maintained examples show
source, AST, IR and RXAS. C completion proceeds instruction by instruction;
INTERPRET and proper B are later work. Correct execution and emitted-code quality
are both acceptance requirements; native compiler output is only a control.
