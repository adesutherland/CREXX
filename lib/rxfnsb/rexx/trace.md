# Level B trace runtime

`trace.crexx` is the shared Level B runtime used by the certified TRACE exit,
RXDB, and ADDRESS command tracing. It is not the Classic `TRACE()` built-in
function; that separate Level C contract is documented in
`lib/rxfnsc/trace.md`.

## Public objects

### `trace_interrupt_raw`

This is the VM breakpoint-signal transport view. Its register-backed fields are
`code` (register 1), `module` (2), `address` (3), and `name` (4). Their order
and register mappings are ABI, not implementation detail. The four accessor
methods return the corresponding typed value.

### `tracecontext`

The factory accepts typed `module` and `addr` integers plus optional string
`mode`, integer `signal_code`, and string `signal_name`. Accessors expose the
signal identity, module/address, mode, source file/line/column/text, formatted
source line, closest source, decoded ASM line, and procedure name.

Metadata is cached per context. Direct controller contexts prepare their
content before return. Breakpoint contexts first resolve the procedure needed
for filtering; rejected events do not scan source or decode an instruction.
Accepted REXX contexts load source data, while ASM/LLM contexts also decode the
instruction. Repeated accessors do not rescan metadata.

### `tracecontroller`

The controller groups these public operations:

- mode and breakpoint control: `mode`, `set_mode`, `toggle_mode`,
  `enable_breakpoints`, and `disable_breakpoints`;
- module policy: `set_first_client_module`, `first_client_module`,
  `set_latest_module_only`, `latest_module_only`, `set_include_runtime`, and
  `include_runtime`;
- namespace policy: reset, suppress, unsuppress, add/remove, cache clearing,
  containment, component matching, and `namespace_is_suppressed`;
- event handling: `context`, `context_from_interrupt`, `should_trace`, and
  `should_trace_module`;
- metadata/status queries: exact/closest source, ASM line, procedure name,
  loaded module count/name, module loading, loaded procedure count/name/id, and
  procedure search;
- trace-result coordination: clear/capture target, pending result/module/
  prefix/type/register/constant, supplied result/value, and parent-value
  supply.

Module and procedure indices are `.int`. Invalid metadata indices return the
documented empty-string or zero status. `load_module` retains the VM's module
number/non-positive status protocol.

## Namespace helpers

The 30 exported `_trace_*` helpers are the compiler-exit-facing surface. They
cover runtime initialization, mode/environment/format/output selection,
namespace controls, event filtering, result-register coordination, escaping,
line output, and ADDRESS command-before/after records. Their argument and return
metadata is typed; counts, addresses, modules, return codes, masks, and
registers are integers.

Supported runtime modes include the Classic `A`, `C`, `E`, `F`, `I`, `L`, `N`,
`O`/`OFF`, and `R` families plus cREXX `REXX`, `ASM`, and `LLM`. Full-word left
prefixes are normalized once. Signed non-zero settings select Normal and zero
selects Off. An invalid `_trace_set` mode raises `INVALID_ARGUMENTS` and leaves
the previous state intact.

Output targets are `stdout`, `stderr`, or an append-mode file. An OS open
failure raises `NOTREADY`; it is not printed and ignored. Environment-driven
invalid settings deliberately retain their documented policy: turn tracing off
and emit a trace diagnostic.

Namespace filters store normalized components. Matching accepts exact dot,
slash, or backslash-delimited components and rejects arbitrary substrings, so
`rxfnsb.trace` matches `rxfnsb`, but `myrxfnsbhelper` does not. Stored filters
are not renormalized for each event.

`_trace_escape_text` scans each Unicode code point once. It escapes backslash,
quote, tab, newline, form feed, carriage return, and remaining C0 controls,
while appending ordinary code points directly.

## Example

```rexx
options levelb
import rxfnsb

controller = .tracecontroller()
call controller.reset_namespace_filters()
call controller.unsuppress_namespace("my.library")
call _trace_set("results")
say _trace_current_mode()       /* R */
call _trace_set("off")
```

`ts_trace.crexx` exercises this example and the mode, filter, escaping, signal,
metadata, status, and cached-context boundaries in optimized and unoptimized
forms. TRACE statement lowering and presentation remain owned by the certified
compiler exit and are outside this Level B selector contract.
