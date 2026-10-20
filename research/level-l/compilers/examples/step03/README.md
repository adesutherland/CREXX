# Generated frontend examples

These are actual STEP03 source-tree and verified-IR dumps from the six approved
[STEP01 inputs](../step01). They are regenerated using the built `parse-probe`
and `frontend-probe` Level B consumers, as described in the
[frontend manual](../../doc/frontends.md). The [walkthrough](../../doc/pipeline-walkthrough.md)
explains the same outputs. No optimisation pass or RXAS backend ran here.

The Classic IR views use the current C v9 context/entry ABI: generated
parameter setup and checked main/label dispatch lead into one source-unit body.
The C source trees retain COptions and its header Name expression. DO uses
normalized CDo/DoWhile and contextual while.truth. The pipeline calls
`length('α漢🙂')`: three codepoints occupy nine UTF-8 payload bytes. Its quoted
String spans bytes `139:150`; the IR preserves `CEB1E6BCA2F09F9982` as value
bytes. LENGTH's result is a runtime contract, not a byte-span calculation.

The current pipeline CArg owns ParseTemplates/ParseTemplate/ParseTarget rather
than scalar ArgTarget children. Its IR uses raw `parse.source` plus an immutable
UPPER packet for `parse.execute`; the activation's original argument is retained.
The [ARG contract](../../doc/architecture.md#proposed-arg-contract) defines
its source, schema and emission requirements. These tree/IR probes themselves do not
assemble or execute output.

| Input pair | Classic output | G output |
|---|---|---|
| Pipeline | [Tree](pipeline-c.tree), [IR](pipeline-c.ir) | [Tree](pipeline-g.tree), [IR](pipeline-g.ir) |
| Evaluation | [Tree](evaluation-c.tree), [IR](evaluation-c.ir) | [Tree](evaluation-g.tree), [IR](evaluation-g.ir) |
| Numbers | [Tree](numbers-c.tree), [IR](numbers-c.ir) | [Tree](numbers-g.tree), [IR](numbers-g.ir) |

The tree inspector prints kind, zero-based half-open byte range and exact source
excerpt. Nonterminal spans may include trivia before the next token. The schema
reference supplies field names; the tree walker follows schema field/list order.
The IR dump shows blocks, typed constants, values, named attributes and origin
IDs. Classic value constants use a byte codec, so `32` represents hex bytes for
source spelling `2`. Origin IDs refer to the module's retained exact source
records; use its structured view or lossless text/binary representation for the
complete source map.

[syntax-error-g.crexx](syntax-error-g.crexx) is deliberately invalid, using CRLF
and Greek alpha before a missing operand. Its [actual diagnostic](syntax-error-g.txt)
shows the independent byte and Unicode-column coordinates. The probe exits 1;
that refusal is expected, and no successful IR is emitted. The local Git
attributes preserve its literal CRLF bytes and retain the generated dumps
without trimming source trivia or printed framing.

The [frontend manual](../../doc/frontends.md) describes regeneration;
the [current plan](../../PLAN.md) tracks instruction status and remaining work.
