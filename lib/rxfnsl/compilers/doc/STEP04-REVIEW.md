# STEP04 review: executable C/G backend

15 October 2026. **Approved and implemented; F4-STEP01–04 complete and
F4-AC01–06 verified locally.** The [qualification receipt](../evidence/step04-20261015/README.md)
records tests and limits. The [backend reference](backend.md) and
[actual emitted examples](../examples/step04/README.md) describe the delivered
implementation; the proposal and illustrative fragments below retain the review
context. Adrian authorised develop publication and switching the lightweight
CI monitor on 15 October 2026. Hosted results remain distinct from local evidence.
[PLAN.md](../PLAN.md) remains the programme authority. This refines STEP04 and
its existing POC-01–10 scope; it does not replace the full acceptance criteria.

## Vision and intended outcome

Turn the existing C/G source-to-IR services into two usable, separately selected
compilers. Both compile the admitted source profile through the same Level L IR
and RXAS backend, then execute through the existing assembler, linker and VMs.
Keep each language's observable behaviour, including their differences.

Every new compiler module is Level B. Grammars remain Level L specifications.
The backend is Level B code **using the existing Level L `llir.writer` emitter**
for chunked text, indentation, sections, origin IDs and output errors. Compiler
code chooses instructions, registers, literals, metadata and runtime calls.
It does not implement a competing text-output library.

The native compiler builds these B modules during bootstrap and can run as an
independent test control. It is not called by a new source compilation request.
Existing target runtime libraries remain permitted dependencies: Classic's
current runtime package still has G prerequisites, separately from the B-only
compiler host. No C compiler plugin, new C implementation or native-backend
delegation is proposed.

STEP04 emits correct unoptimised RXAS. Shared optimisation is STEP05; complete
PoC qualification is STEP06. This step neither enables INTERPRET nor changes
the default `rxc`/`rxc-c` routes.

## Proposed basic surface

Two commands, sharing one service and backend:

```sh
llrxc-g -i runtime-images -o demo.rxas demo-g.crexx
llrxc-c -i runtime-images -o demo.rxas demo-c.crexx
```

`-i` supplies an explicit directory of target runtime/import images; it does
not inject G or Classic modules into the compiler host. `-o` is the RXAS output
file. Assembling, linking and running are explicit subsequent toolchain stages.
Build/install these names separately without changing existing wrapper routing.

Proposed service, following the implemented frontend request convention:

```rexx
request = .llcompiler..request('g', source, artifacts, filename)
if request.status() <> 0 then say request.diagnostic()
else do
  output = request.output()  /* .llir..output, including origin-tagged chunks */
  say output.text()
end
```

The service owns exact source bytes, intermediate results and diagnostics. It
does not print, exit the process or invoke native compilation. On failure no
successful RXAS output is exposed. A successful output owns its storage and
remains usable after the request is released. The command validates first,
writes a temporary sibling file and replaces the destination only on success.
Existing frontend AST/IR inspection remains available; no new grammar syntax
or backend declaration language is needed.

## Backend structures and processing

1. **Check the admitted IR and runtime contracts.** Verify every operation,
   type, callable signature and target dependency. Resolve C/G operation
   meanings explicitly; unsupported operations produce located errors.
2. **Select target operations.** Traverse functions and blocks in deterministic
   order. Typed operations select suitable VM instructions. Classic operations
   use existing RexxValue/pool/numeric/activation runtime contracts, with B-only
   adapters where needed. Retain evaluation order and observable failures.
3. **Assign registers and call windows.** Initially give slots and temporary
   values stable registers within each function; avoid general liveness-based
   reuse. Reserve contiguous call windows separately. Specify argument/result
   copying versus aliasing, references, cleanup and recursion from the real ABI.
4. **Emit using Level L.** Emit procedure sections through `llir.writer`, then
   append them to the module output. Validate labels, call declarations,
   metadata, exact literal encodings and origin mapping before publication.

The proposed target representation is a small compiler-owned sequence of
records: instruction kind, ordered typed operands, labels/symbol references and
source origin. It retains virtual value/slot references until register
assignment. This is an emission plan, not another source AST or a new generic
Level L language. Its arrays/records, invariants and validators will be
documented alongside implementation. Source IR stays immutable and available
for inspection. A target instruction may represent one IR operation, or a
sequence may implement a runtime call.

Generic graph-lowering services currently admit pure straight-line fragments.
Do not pretend they already support arbitrary stateful CFG rewriting. This
backend can traverse verified blocks and select target instruction sequences
without rewriting the source graph. Any needed reusable CFG transformation
API would be presented separately before changing that public contract.

For example, the emission path uses the existing API:

```rexx
out = .llir..writer()
call out.line('main() .locals=2')
call out.indent()
call out.line('load r1,1', origin_id)
call out.line('itos r1', origin_id)
call out.line('say r1', origin_id)
call out.dedent()
output = out.finish()
```

This shows emitter use, not the complete procedure implementation. Production
selection creates operands from checked values and uses one RXAS quoting
routine; it does not interpolate arbitrary source bytes into instruction text.

## Small example: same spelling, different semantics

```rexx
/* Classic */
options levelc
say 01
```

```rexx
/* Typed G */
options levelg
say 01
```

Expected outputs are `01` for Classic and `1` for G. The existing STEP03
frontend was run on these exact sources for this review. Relevant actual IR
excerpts, with setup/return operations omitted:

```text
Classic:
  %5 = core.constant 3031 : crexx.c.value
  crexx.c.say %5

G:
  %2 = core.constant 1 : crexx.g.int64
  %3 = crexx.g.convert.int64.text %2
  crexx.g.say %3
```

`3031` is the Classic value codec's hexadecimal display of the bytes `01`, not
the number 3031. `%2` and `%5` are IR value IDs, not machine registers. The
backend assigns registers later. Both languages share the graph/container and
backend machinery; their registered operation contracts preserve these
different meanings.

The corresponding **illustrative RXAS body fragments** are:

```rxas
* Classic literal spelling is already text.
load r1,"01"
say r1
```

```rxas
* G has an integer value that must be converted for SAY.
load r1,1
itos r1
say r1
```

These are hand-authored target illustrations, not output from an implemented
backend or a claim that the complete Classic graph reduces to these two
instructions. The real backend must also emit the entry/activation contract,
pool/context setup, return behaviour, metadata and required cleanup. Ordinary
Classic arithmetic must preserve numeric context and RexxValue conversion;
it must not become a G integer addition merely because a small example agrees.

For this literal-only illustration, each fragment was wrapped in
`.globals=0`, `main() .locals=2`, `load r0,0` and `ret`. Both assembled with
`rxas -n` and printed the stated output on `rxbvm` and `rxtvm`. This checks the
RXAS spelling and demonstration only. It does not establish the unimplemented
source-to-RXAS path or any general optimisation equivalence.

The larger existing [paired pipeline](pipeline-walkthrough.md) is the next
execution acceptance control: arithmetic, assignment, branch, loop, local
routine/return, SAY and an actual imported LENGTH call. Its expected output is
`10`, `0`, `3`. The evaluation and numeric pairs retain intentional C/G
differences. The tiny example above does not narrow the admitted profile.

## Numbered acceptance criteria

All six criteria are verified for the admitted local STEP04 scope by the
[receipt](../evidence/step04-20261015/README.md). Full-programme and hosted
platform obligations remain open in PLAN.md.

1. **F4-AC01 — Complete admitted executable path.** Both separate compilers
   execute all POC-01–10 features through their existing frontend, checked IR,
   this backend and RXAS/RXLINK/VM. Source compilation never invokes the native
   compiler. Verify the six source controls plus focused cases with compiler
   executables unavailable to the source-compilation service.
2. **F4-AC02 — Correct target semantics and ABI.** Cover G's admitted scalar
   types/conversions and C's dynamic pool, exact literals, numeric state,
   argument presence/RESULT, calls and return/exit. Include nested loops,
   LEAVE/ITERATE, early returns, recursion, actual imports, argument order,
   alias/copy boundaries and errors on taken versus skipped paths. Compare
   explicit expected observations with independent existing-compiler controls.
3. **F4-AC03 — Checked registers and output.** Validate register bounds,
   call windows, labels, operand forms, imports/metadata, literal encodings and
   source identity. Run assembled/linked output on both concrete VMs, initially
   with assembler optimisation disabled to isolate backend behaviour; retain
   a normal assembler-mode check too.
4. **F4-AC04 — Simple B-only service and tools.** The proposed request/result
   and separate commands own their outputs and failures. All compiler host
   modules use B/L only and output construction uses `llir.writer`. Test
   installed independent commands, B-only bootstrap/build manifests and
   preservation of current compiler routes. Record target runtime dependencies
   separately rather than claiming the Classic runtime is already B-only.
5. **F4-AC05 — Diagnostics and atomic failure.** Retain frontend error/message
   coverage; add malformed IR, missing/wrong runtime contracts, unsupported
   lowering, output/register/resource limits and file-write failures. Runtime
   errors in admitted programs retain useful original source locations.
   Failed requests expose no successful output; failed file publication leaves
   the prior destination intact. Test successful output lifetime after release
   of its request and repeated independent requests.
6. **F4-AC06 — Maintained qualification and explanation.** Keep schemas,
   instruction selection, register/call algorithms, origin propagation,
   import/metadata handling and paired actual RXAS outputs current in the
   architecture and user guide. Pass product build and focused correctness
   tests, preserve both host compile modes/VMs and CI preparation dependencies,
   and state local versus hosted evidence. Broader unchanged evidence is reused.

## Numbered implementation steps

| Step | Work | Acceptance / dependency |
|---|---|---|
| F4-STEP01 | Inventory all emitted IR operation/type families and actual runtime ABI contracts; define target records and checked selection. | F4-AC01–03; approval of this review first. |
| F4-STEP02 | Implement scalar/control selection, stable registers, procedure/call windows, numeric/activation handling and target metadata in Level B. | F4-AC01–03/05; after the contract inventory. |
| F4-STEP03 | Implement the shared compile service, separate commands and Level L writer-based output/file publication. | F4-AC03–05; reuse the existing writer. |
| F4-STEP04 | Run paired end-to-end and adverse cases, installed/host-boundary checks; update the walkthrough, architecture, algorithms and guide. | F4-AC01–06; complete all earlier steps. |

Approval recorded 15 October 2026: this bounded unoptimised backend, the simple
request/command surface and the initial stable-register strategy. Any missing platform contract
requiring native C or a new language/public IR design returns for a decision;
it is not silently implemented as a fallback or removed from acceptance.
