# cREXX Standard Libraries and Built-In Functions (BIFs)

The `crexx` toolchain implements its Standard Libraries and Built-In Functions (BIFs) using a hybrid approach. Some core functions are implemented natively in C via the **cREXX Plugin Architecture (RXPA)**, while many standard library functions (like those in Classic REXX) are actually implemented in cREXX itself.

Libraries are housed in the `lib/` directory, which is divided into domains like:
- `lib/rxfnsb/` (Classic REXX Built-In Functions for Level B)
- `lib/rxfnsg/` (Level G class-shaped general-purpose interfaces)
- `lib/rxfnsc/` (Level C standard library functions)
- `lib/rxmath/` (Math extensions)
- `lib/plugins/` (General-purpose extensions like `fileio`, `regex`, `strings`, `socket`, etc.)

`lib/rxfnsb/rexx/rxjson.crexx` contains the first JSON foundation library module
for Level B web-service and transport work. It is implemented in Rexx, ships in
`library.rxbin`, and intentionally exposes string-oriented helpers first:

- `jsonvalid(json)`
- `jsontype(json, path)`
- `jsonget(json, path)`
- `jsoncount(json, path)`
- `jsonmembers(json, path, names[])`
- `jsonquote(text)`
- `jsonunquote(json)`
- `jsonarray(values[])`
- `jsonobject(keys[], values[])`

Paths are Rexx-friendly and one-based for arrays, for example
`choices.1.message.content`. This is enough to build LLM-style request JSON and
extract common response fields without introducing a full object mapper yet.

For the full API contract, path syntax, examples, limits, and test location, see
`lib/rxfnsb/rexx/rxjson.md`.

`lib/rxfnsb/rexx/rxsocket.crexx` provides the Level B wrapper for the VM's core
TCP socket instructions. It ships in `library.rxbin` and exposes a small raw
socket API for loopback clients, servers, and future web-service work:

- `socketcreate()`, `socketclose(sock)`
- `socketconnect(sock, host, port)`
- `socketconnecttls(sock, host, port)`
- `socketbind(sock, host, port)`, `socketlisten(sock, backlog)`,
  `socketaccept(sock)`
- `socketsend(sock, text)`, `socketrecv(sock, maxbytes)`
- `socketsendb(sock, data)`, `socketrecvb(sock, maxbytes)`
- `sockettimeout(sock, milliseconds)`, `socketblocking(sock, enabled)`,
  `socketnodelay(sock, enabled)`, `socketkeepalive(sock, enabled)`
- `socketpending(sock)`, `socketshutdown(sock, how)`
- `socketpeer(sock)`, `socketlocal(sock)`, `socketstatus(sock)`,
  `socketerror(sock)`

The default library path is raw TCP and does not attempt HTTP parsing. Its core
value is deployment stability: VM opcodes use platform socket APIs directly,
avoiding dynamic `.rxplugin` discovery. Optional client TLS is exposed through
the same VM-managed handle model as `socketconnecttls(sock, host, port)`, which
connects and starts TLS before application bytes are exchanged. The instruction
exists in all builds; without a TLS backend it returns a negative socket status
rather than signalling. True STARTTLS remains a lower-level RXAS/VM instruction
for future protocol-specific libraries and is not exposed by the public Level B
`rxsocket` wrapper. Fresh CMake configurations select a TLS backend by platform:
`NETWORK` on Apple platforms, `OPENSSL` on non-Windows Unix-like platforms, and
`SCHANNEL` on Windows. `NETWORK` uses macOS Network.framework,
Security.framework, CoreFoundation.framework, and the system trust store,
`SCHANNEL` uses Windows SChannel/SSPI and the Windows trust store, and
`OPENSSL` uses OpenSSL with default verification paths and hostname checks.
`CREXX_ENABLE_TLS=OFF` can be used for dependency-minimal builds.
`CREXX_TLS_STATIC_OPENSSL=ON` asks CMake to prefer static OpenSSL libraries when
the OpenSSL backend is selected. For API details, status codes, and examples,
see
`lib/rxfnsb/rexx/rxsocket.md`.

The older OpenSSL-backed dynamic socket plugin is deprecated and no longer
builds by default. Developers who still need it can configure with
`CREXX_BUILD_LEGACY_SOCKET_PLUGIN=ON`; otherwise source builds avoid that
plugin's OpenSSL discovery and distribution burden.

`lib/rxfnsb/rexx/rxhttp.crexx` provides a reusable Level B HTTP client on top of
`rxsocket`. It builds request text with UTF-8 byte-counted `Content-Length`,
reads raw socket responses, decodes `Content-Length` and HTTP/1.1 chunked
bodies, preserves non-2xx response bodies for diagnostics, and exposes the last
raw response/body through the `httpclient` interface. It sends
`Accept-Encoding: identity` and `Connection: close`; compressed content remains
out of scope. A non-zero fourth factory argument enables TLS through
`socketconnecttls`. Callers can pass complete additional header lines through
`sendWithHeaders`, `postWithHeaders`, or `buildRequestWithHeaders`, which is
how hosted provider authentication is layered without duplicating HTTP framing.
See `lib/rxfnsb/rexx/rxhttp.md`.

`lib/rxfnsb/rexx/trace.crexx` provides the Level B trace/debugger internals used
by `rxdb` and by the `TRACE` certified compiler exit:

- `.tracecontroller`: breakpoint enable/disable, module/procedure helpers,
  source/ASM lookup, default runtime/debugger filtering, and shared
  structured trace-event metadata lookup
- `.tracecontext`: immutable per-event module/address/source/ASM/procedure
  snapshot
- `.trace_interrupt_raw`: internal register-mapped view of the VM interrupt
  object used by breakpoint handlers
- `_trace_set(mode)`, `_trace_set_from_env(allow_output)`,
  `_trace_set_format(format)`, `_trace_set_output(target)`,
  `_trace_current_mode()`, `_trace_mode_from_option(option)`,
  `_trace_needs_breakpoints(mode)`,
  `_trace_context_from_raw(raw)`, `_trace_should_trace_context(event)`,
  `_trace_should_emit_key(key)`, `_trace_capture_result_target(module, addr)`,
  `_trace_pending_parent_register(module, type, value)`,
  `_trace_supply_parent_value(value)`, `_trace_pending_result()`,
  `_trace_pending_prefix()`, and `_trace_clear_pending_result()`: the
  compiler-exit-facing runtime surface for setting trace mode/format/output,
  normalizing dynamic `TRACE VALUE` options, applying explicit `TRACE ENV`
  `CREXX_TRACE` / `CREXX_TRACE_TO` environment settings, coordinating simple
  `TRACE R`/`TRACE I` parent-frame value reads, and servicing `BREAKPOINT`
  events. `.tracecontroller` owns the trace-event metadata lookup and pending
  value state so other trace/debug users can share the same interpretation. The
  exit
  still emits caller-frame assembler to enable/disable breakpoints, install the
  handler, and perform the actual `metalinkpreg` read for a pending register,
  because VM signal tables and `metalinkpreg` are frame-sensitive. Register
  reads are driven only by `.traceevent` metadata; `.meta_reg` remains
  scope/register-placement metadata, not a value-change stream.
- `_trace_command_before(environment, command)` and
  `_trace_command_after(environment, command, rc, condition)`: ADDRESS dispatch
  hooks used by `TRACE C`, `TRACE E`, `TRACE F`, and quiet/default `TRACE N`.

Classic text and LLM trace formatting belong to the generated TRACE exit
handler. The shared runtime provides controller/filter helpers and output
plumbing; RXDB and other debugger UIs should make their own presentation and
stepping choices from the structured metadata.
`_trace_set_output` accepts `stdout`, `stderr`, or a file path; file targets are
opened in append mode per trace record.

The helpers rely on VM metadata instructions such as `metaloaddata`,
`metaloadinst`, `metadecodeinst`, and `metaloadedmodules`, so deployable linked
images that strip source/TRACE debug metadata may still provide ASM/module data
while source-line and trace-event lookup return empty. Debugger UI text and
menu rendering belong to `debugger/rxdb_gui.crexx`, not the library trace
internals.

`lib/rxfnsb/rexx/_address.crexx` owns the Rexx-side ADDRESS protocol. In
addition to command dispatch, redirects, sandboxes, and function calls,
`addressrequest.get_binding_value(name)` gives Rexx providers a simple way to
read scalar bindings auto-exposed from ADDRESS host-variable anchors such as
`:name` and `${name}`. Anchor interpretation remains provider-specific; the VM
only carries binding values and write-back updates.

`lib/rxfnsg/rexx/llm.crexx` contains the first Level G LLM integration surface.
Level G is layered on the Level B foundation: the module is `options levelg`,
builds into `rxfnsg.rxbin`, and imports the Level B `rxfnsb`, `rxjson`, and
`rxhttp` modules rather than duplicating their work. It exposes a
class-shaped interface in the `rxfnsg` namespace:

- `llm`: provider-selecting interface for the local Ollama default
- `ollama`: concrete local Ollama implementation over plain HTTP
- `openai`: concrete OpenAI Responses API implementation over HTTPS
- `anthropic`: concrete Anthropic Messages API implementation over HTTPS
- `gemini`: concrete Gemini `generateContent` implementation over HTTPS

The first provider posts JSON to a local Ollama `/api/generate` endpoint with
`stream:false`, using `rxhttp` for HTTP transport and `rxjson` for
request/response JSON. It keeps the last raw HTTP response plus decoded JSON
body available for diagnostics. Hosted providers are also Rexx implementations:
they use environment-variable API keys by default, build provider-specific JSON
request bodies and authentication headers, and send through `rxhttp` with TLS
enabled. `demos/llm/llm_address_environment.crexx` layers a Rexx ADDRESS
provider over these clients so scripts can use model-shaped environments such
as `ADDRESS LLM_GPT_4_1`, `ADDRESS CLAUDE_SONNET_4_5`, and
`ADDRESS GEMMA4_LATEST`; the provider caches by environment instance and routes
through a small driver registry of exact aliases and prefixes. See
`lib/rxfnsg/rexx/llm.md` and `demos/llm/`.

## 1. BIFs Implemented in cREXX (`lib/rxfnsb/rexx/`)

A significant portion of the Classic REXX Built-In Functions (such as `abs()`, `date()`, `length()`, `substr()`) are written entirely in cREXX. These are located in `lib/rxfnsb/rexx/`. 

This approach minimizes the VM footprint and demonstrates the capability of the cREXX compiler to handle system-level logic.

For repo-native Level B authoring patterns, argument signature examples, and
wayfinding to real `.crexx` examples, see `docs/ai-context/CREXX_LEVELB_AUTHORING.md`.

`lib/rxfnsb/rexx/fileio.crexx` exposes the sequential Level B text file BIFs
`linein`, `lineout`, `charin`, `charout`, and `lines`. These are UTF text
functions over `.string`: `linein` reads one line without its line terminator,
`lineout` writes text plus a newline, `charin` reads UTF-8 codepoints, and
`charout` writes text without adding a newline. They are not the binary byte
I/O surface. Future binary file BIFs should take and return `.binary` and use
the VM byte instructions (`freadb`, `fwriteb`, `freadbyte`, or `fwritebyte`)
rather than weakening the Level B `.string` UTF contract.

### Build And Debugging Rules

The Rexx BIF library build is a bootstrap build. `lib/rxfnsb/rexx/CMakeLists.txt`
compiles most Rexx BIF modules with `rxc -x --import-rxas`, which disables
certified compiler exits while building the library used by those exits. An
explicit `TRACE`, `PARSE`, `ADDRESS`, or other certified-exit statement added
directly to a BIF source file will fail with `#CERTIFIED_EXIT_DISABLED`.

Do not debug BIFs by adding `TRACE RESULTS` inside `lib/rxfnsb/rexx/abs.crexx`
or another library source file. Use one of these instead:

- a normal scratch program or functional test that imports `rxfnsb` and calls
  the BIF with compiler exits enabled;
- `TRACE UNSUPPRESS NAMESPACE rxfnsb` (or `rxfnsg`, `rxfnsl`, `rxfnsc`) when
  library frames should be visible despite the default system-namespace filter;
- `TRACE ASM` or `TRACE LLM` from the caller for lower-level metadata checks;
- `crexx -native --link-keep-source` or an unstripped `rxlink` image when
  debugging native/linked output, because stripped linked images drop
  `META_SOURCE_STEP` and `META_TRACE_EVENT`.

Useful focused checks for this area:

```sh
cmake --build cmake-build-debug --target testbifs
ctest --test-dir cmake-build-debug -R '^ts_.*_(noopt|opt)$' --output-on-failure
ctest --test-dir cmake-build-debug -R '^test_system$' --output-on-failure
ctest --test-dir cmake-build-debug \
  -R '^(trace_event_metadata|test_trace_|ts_trace_|rxlink_format_check|rxlink_rxdas_strip_smoke)' \
  --output-on-failure
```

The standard Level B array helpers live in `lib/rxfnsb/rexx` and are preferred
over the legacy `lib/plugins/arrays` RXPA plugin. Current array BIFs include
`arrayfind`, `arrayinsert`, `arraydelete`, `arraysort`, `arraycopy`,
`arraydrop`, `arrayhi`, `arraymove`, `arraydump`, `arrayformat`,
`arrayappend`, `arrayprepend`, `arraypop`, `arrayshift`, `arrayget`,
`arrayset`, `arraycontains`, `arrayindexof`, `arrayreverse`, and `arrayjoin`.
Insertion, deletion, movement, append, prepend, pop, shift, and gap-growing set
use VM bulk attribute instructions so the logical array pointer list can be
shifted without a Rexx-level per-element copy loop. Mutating array BIFs must
declare the array with `arg expose`.

Container naming is intentionally split by shape:

- Arrays/lists are ordered, one-based, duplicate-preserving sequences using
  `array[0]` as the high water mark. Classic code should use the `array*` BIFs;
  OO code should expose the same semantics through `ArrayList`/`LinkedList`
  methods such as `add`/`append`, `insert`, `remove`, `get`, `set`, `size`,
  `clear`, `contains`, and `indexOf`.
- Maps/stems are keyed containers. Do not overload `array*` for keyed lookup;
  use `stem*` or `map*` BIF names and class methods such as `put`, `get`,
  `containsKey`, `remove`, `keys`, and `values`.
- Sets are uniqueness containers. Use `add`, `contains`, `remove`, `size`,
  `clear`, and `toArray`/`fromArray` semantics consistently across classic and
  class-shaped APIs.

Imported Rexx BIF calls inline only when the imported artifact carries
`META_INLINE`. `rxlink` strips `META_INLINE` by default, so the standard-library
link explicitly uses `PRESERVE INLINE`. Release linked libraries also use
`STRIP SOURCE`, keeping inline bodies available to downstream optimisation while
dropping source-level debug metadata: `META_SOURCE_STEP` file/source-line
records and `META_TRACE_EVENT` semantic TRACE value records. Debug linked
libraries keep both inline bodies and source/TRACE metadata. Class-library hot
paths that rely on this should inspect generated `.rxas` when changing import
paths or link-strip policy.

`lib/plugins/arrays` is deprecated and retained only as a legacy plugin smoke
test. New Level B code should import `rxfnsb` and use the standard BIFs.

### Anatomy of a cREXX BIF
Functions written in cREXX follow standard language rules, utilizing namespaces and type enforcement:

```rexx
/* lib/rxfnsb/rexx/abs.crexx */
options levelb

namespace rxfnsb expose abs

abs: procedure = .string
  arg number = .string
  if left(number, 1) = '-' then number = substr(number, 2)
  return number
```

**Key Features:**
1. **Namespaces:** Functions must declare `namespace rxfnsb expose <function_name>` so they correctly bind into the Standard Library space that user scripts import.
2. **Inline Assembly (`assembler`)**: When low-level access is required (such as fetching the current system time in `date.crexx`), cREXX BIFs can drop down into inline bytecode using the `assembler` keyword.
3. **Compilation:** These `.crexx` files are compiled into `.rxbin` bytecodes during the build process and are packaged or shipped exactly like user-compiled binaries.
4. **Explicit Late Load:** `loadmodule(path) -> .int` wraps the VM's
   `METALOADMODULE` instruction. Use it when Rexx code deliberately loads a
   `.rxbin` or `.rxplugin` provider before calling imports supplied by that
   file. The VM relinks immediately after a successful load.
5. **ADDRESS Public Helpers:** `addressenv(name) -> .addressenvironment`
   returns the cached environment object, including `environment_name()` and
   `environment_id()` for the traditional `SYSTEM`/`PATH` environments and for
   Rexx/native providers. `addresscall(env, name, ...) -> .string` wraps the
   lower-level `_address_function(env, name, args[])` request object path.
   `_address_call(...)` and `_address_call_response(...)` remain internal
   compatibility spellings for code that needs the raw response object.

## 2. RXPA (cREXX Plugin Architecture)

For functions requiring native performance or access to C-level system libraries (like cryptography or sockets), `crexx` provides the RXPA framework. This macro-driven C API (defined in `rxpa/crexxpa.h` and `rxpa/rxpa.h`) allows developers to write REXX-callable functions without interacting directly with the VM's internal `stack_frame` or `value` structures.

Plugins can be compiled in two ways:
1. **Dynamic Plugins (`.rxplugin`)**: Recommended for user extensions. They are discovered and loaded at runtime using the same search paths as regular `.crexx` modules.
2. **Static Plugins**: Built directly into the `crexx` binaries. These are typically reserved for core Standard Libraries to guarantee they are always available.

## 3. Writing a Native Function

A native C function meant to be exposed to REXX is defined using the `PROCEDURE` macro. 

### Argument Access and Returns
The VM passes arguments as opaque handles mapped to internal VM registers. The RXPA headers provide macros to extract native C types from these registers and to write results back:
- `NUM_ARGS`: The count of arguments passed from REXX.
- `ARG(n)`: Retrieves the opaque handle for the *n*th argument.
- `GETINT()`, `GETFLOAT()`, `GETSTRING()`: Extracts the native C value from a register handle.
- `SETINT()`, `SETFLOAT()`, `SETSTRING()`: Writes a native C value into a target register.
- `SETNATIVEPAYLOAD()` / `GETNATIVEPAYLOAD()`: Attach or read a hidden
  native binary payload for object-shaped native values. Use this only with a
  clear copy/finalizer contract; ordinary Rexx code still sees an object value,
  not a C pointer.
- `RETURN`: The specific target register designated for the function's return value.

### Error Handling
Errors are thrown using the `RETURNSIGNAL` macro, which halts execution and raises a specific `RXSIGNAL_*` exception within the VM. Successful execution must conclude with `RESETSIGNAL`.

### Example Native Function

```c
#include "crexxpa.h"

// Example: Add two integers together
PROCEDURE(add_integers)
{
    int result;

    // 1. Validate argument count
    if (NUM_ARGS != 2) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "2 arguments expected")
    }

    // 2. Extract values and perform logic
    result = GETINT(ARG(0)) + GETINT(ARG(1));

    // 3. Set the return value
    SETINT(RETURN, result);

    // 4. Clear the signal state and exit
    RESETSIGNAL
}
```

## 4. Registering Functions with the VM

Once the C functions are written, they must be registered so the REXX compiler (`rxc`) and interpreter (`rxvm`) can map REXX namespace calls to the native C function pointers. 

This is accomplished using the `LOADFUNCS` mapping block, which binds the C function pointer to a REXX namespace, declares its return type, and defines its expected argument signature.

```c
// Publish functions to the cREXX compiler and VM
LOADFUNCS
//       C Function       REXX Namespace & Name      Opt  Return Type   Argument Signature
ADDPROC( add_integers,    "rxmath.add_integers",     "b", ".int",       "i1 = .int, i2 = .int" );
ADDPROC( string_concat,   "rxstr.string_concat",     "b", ".string",    "s1 = .string, s2 = .string" );
ENDLOADFUNCS
```

### Registration Breakdown:
1. **C Function**: The literal name of the C function defined by `PROCEDURE(...)`.
2. **REXX Namespace & Name**: How the function will be called from REXX code (e.g., `import rxmath; x = add_integers(1, 2)`).
3. **Option**: The target cREXX language level (`"b"` for Level B, `"c"` for Level C).
4. **Return Type**: A string literal dictating the exact cREXX type returned (`".int"`, `".string"`, `".void"`).
5. **Arguments**: The exact type signature expected by the compiler. It supports standard types, array syntax (`.int[]`), and reference passing (`expose`). 

During compilation, `rxc` parses this block to strictly enforce type safety when the REXX code invokes the native plugin. During execution, `rxvm` uses this block to dynamically link the `.rxplugin` shared library symbol into the execution space.

## 5. Declaring Native Classes and Interfaces

RXPA can also publish class/interface contract metadata to the compiler and VM.
Use this when a native or hybrid provider needs to expose the same class-shaped
contract that Rexx source would normally declare.

```c
LOADFUNCS
ADDINTERFACE("demo.environment");
ADDFACTORY("demo.environment", "*", ".environment", "name=.string");
ADDMETHOD("demo.environment", "describe", ".string", "");

ADDCLASS("demo.nativeenvironment");
ADDIMPLEMENTS("demo.nativeenvironment", "demo.environment");
ADDFACTORY("demo.nativeenvironment", "*", ".nativeenvironment", "name=.string");
ADDMETHOD("demo.nativeenvironment", "describe", ".string", "");

ADDPROC(make_env, "demo.make", "b", ".environment", "");
ENDLOADFUNCS
```

Rexx source factories omit return types (`*: factory`). RXPA declaration
macros still carry a return-type string because they emit low-level metadata
directly; use the owner contract type for that metadata until RXPA grows an
owner-derived factory helper.

Available declaration macros:
- `ADDCLASS(name)` and `ADDCLASSX(name, option, type)`
- `ADDINTERFACE(name)` and `ADDINTERFACEX(name, option, type)`
- `ADDIMPLEMENTS(class_name, interface_name)`
- `ADDFACTORY(owner, member, return_type, args)`
- `ADDMETHOD(owner, member, return_type, args)`
- `ADDDEFAULTMETHOD(owner, member, return_type, args)`
- `ADDMEMBER(owner, kind, member, return_type, args)` for the generic form

These declarations are consumed from both dynamic `.rxplugin` modules and
static `DECL_ONLY` declaration libraries. They make contracts visible for type
checking and runtime metadata discovery.

Declaration is not construction. `ADDCLASS`, `ADDINTERFACE`,
`ADDIMPLEMENTS`, and the member macros tell the compiler and VM that a contract
exists, but they do not by themselves run a factory or stamp class identity on
a return value. Existing RXPA return helpers such as `SETSTRING`, `SETINT`, and
array attribute helpers fill a return value slot; factory/class construction is
still a separate operation.

That means a native procedure can advertise a typed signature, for example:

```c
ADDPROC(make_env, "demo.make", "b", ".environment", "");
```

but the C body must still create or receive an object value that has the right
shape and class identity. For complete object creation today, use a small Rexx
factory/class shim to create the typed Rexx object, and let that object
delegate selected work to native C functions. A future RXPA helper should cover
the pure-C operation of constructing/stamping a typed object directly.

Ordering matters when a native procedure signature references a class or
interface type. Put the relevant `ADDCLASS`/`ADDINTERFACE` metadata before the
dependent `ADDPROC` so the compiler knows the type before it validates the
procedure signature:

```c
LOADFUNCS
ADDINTERFACE("demo.environment");
ADDMETHOD("demo.environment", "describe", ".string", "");

ADDPROC(make_env, "demo.make", "b", ".environment", "");
ENDLOADFUNCS
```

### Native payload ownership

When a native implementation needs to hide a C-side handle inside a Rexx object,
the preferred physical storage is the value's binary payload. Attach shared
static payload operations with `SETNATIVEPAYLOAD()` if the payload owns native
resources:

```c
static const rxpa_native_payload_ops env_payload_ops;

static void env_finalize(rxpa_attribute_value value) {
    EnvHandle **slot = (EnvHandle **)GETNATIVEPAYLOAD(value, NULL, NULL, NULL);
    if (slot && *slot) env_release(*slot);
}

static void env_copy(rxpa_attribute_value dest, rxpa_attribute_value source) {
    EnvHandle **slot = (EnvHandle **)GETNATIVEPAYLOAD(source, NULL, NULL, NULL);
    EnvHandle *copy = slot && *slot ? env_retain(*slot) : NULL;
    SETNATIVEPAYLOAD(dest, &copy, sizeof(copy), &env_payload_ops, 0);
}

static const rxpa_native_payload_ops env_payload_ops = {
    "demo.EnvHandle",
    env_copy,
    env_finalize
};
```

The ops object is provided by the native module and shared across instances;
the value stores only a pointer to it. The VM owns the per-value binary payload
buffer. `SETNATIVEPAYLOAD()` mallocs VM-owned storage, copies the supplied
payload bytes into it, and records the shared ops pointer and flags. Copy hooks
must install the destination payload through `SETNATIVEPAYLOAD()` rather than
storing externally allocated memory directly in the destination value. On
`clear_value()`, the finalizer runs before the VM frees the binary buffer; the
finalizer releases nested native resources but must not free the payload buffer
itself. On `move_value()`, the buffer and ops pointer move together. On
`copy_value()`, the VM calls the copy hook if set; if it is not set, the VM
byte-copies the payload and copies the ops pointer. That fallback is only safe
when the payload was deliberately designed to tolerate duplicate finalization,
such as a registry handle or refcounted pointer. Use
`RXVM_NATIVE_PAYLOAD_FLAG_BITCOPY_SAFE` to document that intent on such
payloads.
