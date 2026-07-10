# rxjson Parser Performance Work

Status: implementation in progress.

The previous `rxjson` implementation was deliberately small, but its algorithm
was a poor fit for production JSON use. It parsed the whole document to produce
string metadata such as `kind start after`, then split that metadata with
`word()`, and then rescanned for path traversal, counts, and member lists.
Character scanning used repeated `substr(json, p, 1)` calls.

The Release 1 performance direction is to keep the public helper API stable
while replacing the internals with a scanner-style parser:

- convert the input string to `.binary` once per public call;
- scan structural JSON bytes with `<at..u8>`;
- keep spans as zero-based byte offsets;
- track path matching during the validation pass;
- compare unescaped object keys with `<compare..binary>` rather than
  materializing candidate strings;
- materialize only the public result value, object member name, or escaped key
  that genuinely needs a `.string`;
- decode `\uXXXX` escapes with Unicode `d2c()` semantics instead of the old
  ASCII-only fallback.

This is not intended as a like-for-like experiment against the old algorithm.
The product goal is a fast, correct foundation parser behind `jsonvalid`,
`jsontype`, `jsonget`, `jsoncount`, `jsonmembers`, `jsonquote`,
`jsonunquote`, `jsonarray`, and `jsonobject`.

Open follow-up areas:

- measure repeated field extraction from the same large document and decide
  whether Release 1 needs a parsed/indexed JSON handle in addition to the
  existing string helper API;
- inspect generated RXAS for the scanner hot paths after select-assist work;
- consider a dedicated string-byte scanner intrinsic if the `.string` to
  `.binary` copy becomes visible in large-document profiles;
- decide how strict to make escaped surrogate validation in `jsonvalid`.

## First Debug Smoke Measurement

Local Debug build on macOS arm64, `rxjson_parser_compare`, 60 rows, 30
iterations, 4,394-byte payload:

| Operation | noopt us | opt us |
| --- | ---: | ---: |
| `jsonvalid(payload)` | 24,380 | 24,362 |
| `jsonget(payload, "choices.1.message.content")` | 28,583 | 25,092 |
| `jsonget(payload, "items.60.sku")` | 25,355 | 25,010 |
| `jsoncount(payload, "items")` | 48,273 | 47,708 |
| `jsonmembers(payload, "usage", names)` | 24,612 | 24,515 |

The generated RXAS for the scanner hot path contains `bgetu8` byte loads and
`bcmpb` binary compares. Remaining `substr` use is concentrated in `jsonquote`,
path parsing, and small path digit checks, not the JSON source scan.

`jsoncount` is expected to be higher in this first cut because it validates and
locates the target during the main pass, then rescans the located array/object
to count members. A parsed/indexed handle or per-call action folded into the
main pass is the next product-level algorithm question if repeated extraction
from one document remains important.

## First Release Smoke Measurement

Local Release build on the same host and benchmark shape:

| Operation | noopt us | opt us |
| --- | ---: | ---: |
| `jsonvalid(payload)` | 16,144 | 16,853 |
| `jsonget(payload, "choices.1.message.content")` | 16,909 | 16,939 |
| `jsonget(payload, "items.60.sku")` | 17,290 | 20,945 |
| `jsoncount(payload, "items")` | 33,263 | 35,215 |
| `jsonmembers(payload, "usage", names)` | 17,314 | 16,954 |

The opt/noopt spread is small in this benchmark. That is expected until the
compiler grows the planned select/multi-branch assist and can flatten the
scanner's byte-class decisions more aggressively.
