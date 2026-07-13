# Raw Typed Intrinsic, Reserved, And Documentation-Only BIF Inventory

Stage 3 reconciliation found names presented by the typed language reference that are not namespace-exposed `rxfnsb` procedures. These are separate from the independent `CBIF-` Classic contracts.

| ID | Name | Source fact | Observed implementation fact |
|---|---|---|---|
| `BIF-TYPED-ARG` | `ARG()` / `arg[]` | arguments and BIF references | Compiler/runtime pseudo-argument surface; not a library procedure. |
| `BIF-DOC-ADDRESS` | `ADDRESS()` | listed in the typed BIF table | No typed callable implementation found; the ADDRESS statement/environment surface is implemented. |
| `BIF-DOC-CONDITION` | `CONDITION()` | listed in the typed BIF table | No typed callable implementation found. |
| `BIF-DOC-JUSTIFY` | `JUSTIFY()` | listed in the typed SAA BIF table | No typed callable implementation found. |
| `BIF-DOC-QUEUED` | `QUEUED()` | listed in the typed BIF table | No typed callable implementation found. |
| `BIF-DOC-SOURCELINE` | `SOURCELINE()` | listed in the typed SAA BIF table | No typed callable implementation found. |
| `BIF-DOC-STORAGE` | `STORAGE()` | listed in the typed SAA BIF table | No typed callable implementation found. |
| `BIF-RESERVED-TRACE` | `TRACE()` | typed BIF reference explicitly calls the name reserved | No callable BIF; the implemented TRACE surface is the statement/compiler exit. |
